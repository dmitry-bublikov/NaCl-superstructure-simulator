#define _CRT_SECURE_NO_WARNINGS
//=============================================================================
// PROGRAM: CF32 local-valence-balance tolerance solver, optimized + mixed anions
// VERSION 4 EXPERIMENTAL: exact finite-field histogram and prefix pruning
// Derived from VERSION 2; scientific acceptance and fallback paths retained.
// At D=0 with all octahedron sums constrained, Aq=6a implies Aq=0 mod 2,3.
// Offline exhaustive enumeration of the kernels gives necessary residue-count
// tables (13 histograms mod 2; 39 mod 3). Reverse elimination also gives exact
// prefix congruences. A site-transitivity anchor removes equivalent searches.
// Reproduce generated tables/source: python experiment_build.py
// Validate geometry symmetries: python experiments/verify_symmetry.py
// Details, correctness proof and measurements: experiments/REPORT.md
// CF32, NaCl-derived structure
// Developed from Dmitry Bublikov's Find-local-balance / CF32 generator
//
// Geometry convention:
//   anion_i = cation_i + (-0.2500, 0, 0), periodic modulo 1.
//
// Physical / algorithmic rule used in this VERSION 1:
//   - for every occupied anion center with charge z_A < 0:
//         abs(sum of 6 neighboring cation charges + 6*z_A) <= delta
//     equivalently, target cation sum = 6*abs(z_A):
//         X^-  -> 6
//         Y^2- -> 12
//         Z^3- -> 18
//   - if the ANION CENTER is a vacancy (charge 0), this octahedron is skipped;
//   - cation-centered octahedra are NOT used as a strict filter in this version.
//
// Input:
//   stoichiometry_<number>.txt
// Example line:
//   A5+1 B5+2 C5+3 D5+4 E3+5 F3+6 G1+7 V5 0| Z29-3 Y1-2 X1-1 V1 0 ch=90 1
//
// Supports:
//   cation components: A..G and V (up to 8 components total)
//   anion components : X, Y, Z and V (1..4 components total)
//
// Output:
//   local_balance_<number>_<delta>.txt
// Each input stoichiometry is written once if at least one complete CF32
// cation+anion assignment satisfying the criterion is found.
//
// Designed for Microsoft C/C++ Optimizing Compiler 19.44.35222.
// Dispatches automatically:
//   * uniform occupied anion sublattice + D>0 -> specialized tolerance DFS
//     with CF32 symmetry, incremental stoichiometry and complementary-pair pruning;
//   * one occupied anion charge class + vacancies, D>0 -> reduced capacity solver;
//   * uniform occupied anion sublattice + D=0 -> exact 10-variable fast path;
//   * fully mixed X/Y/Z/V -> general cation-only CSP + exact Hall-capacity feasibility.
// Multicore streaming input/output: a bounded producer/worker queue keeps RAM
// usage independent of input-file size.  Independent stoichiometry lines are
// solved in parallel with std::thread.  Every accepted line is written and
// fflush()'ed immediately by the worker that found it.
// The shared per-problem cache is compact, hard-capped and sharded to reduce
// lock contention between worker threads.
// NOTE: with more than one worker, accepted output lines can appear in a
// different order from the input file; each line itself is unchanged.
// Recommended compile command:
//   cl Find-local-balance-32-32-a.cpp /O2 /Oi /Ot /EHsc /std:c++17 /Fe:find-local-balance-32-32-a.exe
//=============================================================================

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
#include <sstream>
#include <string>
#include <initializer_list>
#include <array>
#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>

using namespace std;

static const int CF_SITES = 32;
static const int MAX_COMPONENTS = 8;
static const uint64_t PROGRESS_INTERVAL = 1000000ULL;
// The CF32 problem key is larger than the uint64_t CF108 histogram key, so use
// a conservative cap.  The cache accelerates repeated stoichiometries but is
// never allowed to grow with the full input size.
static const size_t MAX_PROBLEM_CACHE_ENTRIES = 250000ULL;
static const size_t PROBLEM_CACHE_SHARDS = 64ULL;
static const size_t MAX_CACHE_ENTRIES_PER_SHARD =
    (MAX_PROBLEM_CACHE_ENTRIES + PROBLEM_CACHE_SHARDS - 1) / PROBLEM_CACHE_SHARDS;

inline bool check_anion_local_balance(int cation_neighbor_sum,
                                      int anion_center_charge,
                                      int delta)
{
    // A vacancy in the center has no local-balance equation in VERSION 1.
    if (anion_center_charge == 0)
        return true;

    return abs(cation_neighbor_sum + 6 * anion_center_charge) <= delta;
}

bool within_stoichiometry(initializer_list<int> assigned,
                          const int stoich[MAX_COMPONENTS + 1],
                          int num_components)
{
    int count[MAX_COMPONENTS + 1] = {0};
    for (initializer_list<int>::const_iterator it = assigned.begin();
         it != assigned.end(); ++it)
    {
        const int component = *it;
        if (component < 1 || component > num_components)
            return false;

        count[component]++;
        if (count[component] > stoich[component])
            return false;
    }
    return true;
}

bool valid_component_symbol(char symbol, bool cation_side)
{
    if (cation_side)
        return (symbol >= 'A' && symbol <= 'G') || symbol == 'V';

    return symbol == 'X' || symbol == 'Y' || symbol == 'Z' || symbol == 'V';
}

bool parse_components(const string& side,
                      bool cation_side,
                      int charge[MAX_COMPONENTS + 1],
                      int stoich[MAX_COMPONENTS + 1],
                      int& num_components)
{
    for (int i = 0; i <= MAX_COMPONENTS; ++i)
    {
        charge[i] = 0;
        stoich[i] = 0;
    }
    num_components = 0;

    istringstream input(side);
    string token;

    while (input >> token)
    {
        if (token.empty())
            continue;

        char symbol = token[0];
        if (symbol >= 'a' && symbol <= 'z')
            symbol = static_cast<char>(symbol - 'a' + 'A');

        // Ignore metadata such as ch=90 and the line/variant number.
        if (!valid_component_symbol(symbol, cation_side))
            continue;

        size_t first_digit = string::npos;
        for (size_t i = 1; i < token.size(); ++i)
        {
            if (isdigit(static_cast<unsigned char>(token[i])))
            {
                first_digit = i;
                break;
            }
        }
        if (first_digit == string::npos)
            return false;

        size_t after_stoich = first_digit;
        while (after_stoich < token.size() &&
               isdigit(static_cast<unsigned char>(token[after_stoich])))
        {
            ++after_stoich;
        }

        if (num_components >= MAX_COMPONENTS)
            return false;

        const int st = atoi(token.substr(first_digit,
                                         after_stoich - first_digit).c_str());
        int z = 0;

        if (after_stoich < token.size() &&
            (token[after_stoich] == '+' || token[after_stoich] == '-'))
        {
            z = atoi(token.substr(after_stoich).c_str());
        }
        else
        {
            // Vacancy format: Vn 0
            string charge_token;
            if (!(input >> charge_token))
                return false;
            z = atoi(charge_token.c_str());
        }

        if (cation_side)
        {
            if (z < 0 || z > 7)
                return false;
        }
        else
        {
            if (z > 0 || z < -3)
                return false;
        }

        ++num_components;
        stoich[num_components] = st;
        charge[num_components] = z;
    }

    return num_components > 0;
}

int stoichiometry_sum(const int stoich[MAX_COMPONENTS + 1],
                      int num_components)
{
    int sum = 0;
    for (int i = 1; i <= num_components; ++i)
        sum += stoich[i];
    return sum;
}


//=============================================================================
// FAST ANION-CENTERED SOLVER
//
// The original VERSION 1 explicitly nested assignments of 32 cation sites and
// 32 anion sites.  This version keeps exactly the same physical criterion but
// eliminates the anion-site permutation loops.
//
// For a partial cation assignment each anion-centered octahedron has a small
// domain of anion charge classes that can still be compatible with it.  The
// domains are checked against the exact anion stoichiometry by the capacitated
// Hall condition (only 4 anion charge classes exist: 0, -1, -2, -3).
//
// Cation stoichiometry is maintained incrementally.  Possible sums for the
// still-unassigned neighbours of every octahedron are computed exactly by a
// tiny bounded subset-sum DP (at most 6 sites, charge 0..7).
//=============================================================================

namespace {

static const int CATION_CHARGE_CLASSES = 8; // 0..7
static const int ANION_CHARGE_CLASSES  = 4; // magnitude 0..3; 0 == vacancy
static const int OCT_SIZE = 6;
static const int N_OCT = 32;

// OCTAHEDRA[a-1] contains the six zero-based cation sites surrounding anion a.
// This is the same -0.2500 x-shift geometry as VERSION 1.
static const int OCTAHEDRA[N_OCT][OCT_SIZE] = {
    { 0,  1,  6,  9, 16, 22}, // anion 1
    { 1,  3, 22, 23, 24, 25}, // anion 2
    { 1,  2,  6, 13, 19, 25}, // anion 3
    { 0,  1,  2,  3,  4,  5}, // anion 4
    { 1,  4,  9, 10, 19, 23}, // anion 5
    { 1,  5, 10, 13, 16, 24}, // anion 6
    { 6,  7, 22, 25, 26, 30}, // anion 7
    { 0,  2,  6,  7, 17, 18}, // anion 8
    { 0,  4,  8,  9, 15, 18}, // anion 9
    { 8,  9, 22, 23, 27, 30}, // anion 10
    {10, 11, 23, 24, 27, 31}, // anion 11
    { 4,  5, 10, 11, 15, 20}, // anion 12
    { 2,  5, 12, 13, 17, 20}, // anion 13
    {12, 13, 24, 25, 26, 31}, // anion 14
    { 0,  5, 14, 15, 16, 17}, // anion 15
    { 9, 10, 15, 16, 27, 28}, // anion 16
    {14, 16, 22, 24, 26, 27}, // anion 17
    { 6, 13, 16, 17, 26, 28}, // anion 18
    { 6,  9, 18, 19, 28, 30}, // anion 19
    {19, 21, 23, 25, 30, 31}, // anion 20
    {10, 13, 19, 20, 28, 31}, // anion 21
    { 2,  4, 18, 19, 20, 21}, // anion 22
    { 0,  3,  7,  8, 14, 22}, // anion 23
    { 3,  4,  8, 11, 21, 23}, // anion 24
    { 3,  5, 11, 12, 14, 24}, // anion 25
    { 2,  3,  7, 12, 21, 25}, // anion 26
    { 7, 12, 14, 17, 26, 29}, // anion 27
    { 8, 11, 14, 15, 27, 29}, // anion 28
    {26, 27, 28, 29, 30, 31}, // anion 29
    {15, 17, 18, 20, 28, 29}, // anion 30
    { 7,  8, 18, 21, 29, 30}, // anion 31
    {11, 12, 20, 21, 29, 31}  // anion 32
};


// Cation-site order inherited from the original staged generator.  It was
// designed so that complete octahedra become available as early as possible.
static const int SITE_ORDER[CF_SITES] = {
     0,  1,  2,  3,  4,  5,
     6,  7, 17, 18,
     8,  9, 15,
    16, 22,
    14,
    12, 13, 20,
    10, 11,
    24,
    19, 25,
    21, 23,
    26, 27,
    28, 29, 30, 31
};


//=============================================================================
// SPECIALIZED FAST SOLVER FOR ONE OCCUPIED ANION CHARGE CLASS, D > 0
//
// This is the optimized successor path for Find-local-balance-32-32-a.cpp.
// It preserves the fast, low-overhead D>0 DFS: exact remaining stoichiometry,
// incremental octahedron sums, a safe CF32 symmetry break and the rigorous
// complementary-pair inequality.  The hybrid dispatcher keeps this path for
// uniform anions instead of paying the cost of general Hall-domain matching.
//=============================================================================

class FastUniformToleranceSolver
{
public:
    FastUniformToleranceSolver(
        const int cation_count[CATION_CHARGE_CLASSES],
        int anion_magnitude,
        int delta)
        : balance_(6 * anion_magnitude),
          low_(balance_ - delta),
          high_(balance_ + delta),
          delta_(delta),
          anchor_charge_(-1)
    {
        for (int q = 0; q < CATION_CHARGE_CLASSES; ++q)
        {
            remaining_[q] = cation_count[q];
            if (anchor_charge_ < 0 && cation_count[q] > 0)
                anchor_charge_ = q;
        }

        for (int s = 0; s < CF_SITES; ++s)
        {
            site_charge_[s] = -1;
            site_degree_[s] = 0;
        }

        for (int o = 0; o < N_OCT; ++o)
        {
            oct_sum_[o] = 0;
            oct_unassigned_[o] = OCT_SIZE;
            for (int k = 0; k < OCT_SIZE; ++k)
            {
                const int site = OCTAHEDRA[o][k];
                site_to_oct_[site][site_degree_[site]++] = o;
            }
        }

        pair_low_ = ceil_div3(balance_ - 11 * delta_);
        pair_high_ = floor_div3(balance_ + 11 * delta_);
        if (pair_low_ < 0) pair_low_ = 0;
        if (pair_high_ > 14) pair_high_ = 14;
    }

    bool solve()
    {
        if (delta_ <= 0)
            return false;

        int total_charge = 0;
        for (int q = 0; q < CATION_CHARGE_CLASSES; ++q)
            total_charge += q * remaining_[q];

        const long long all_octa_sum =
            static_cast<long long>(OCT_SIZE) * total_charge;
        if (all_octa_sum < static_cast<long long>(N_OCT) * low_ ||
            all_octa_sum > static_cast<long long>(N_OCT) * high_)
            return false;

        return dfs(0);
    }

private:
    static constexpr int PARTNER[CF_SITES] = {
        31,29,27,28,26,30,11,10,13,12,7,6,9,8,19,25,
        21,23,24,14,22,16,20,17,18,15,4,2,3,1,5,0
    };

    int balance_;
    int low_;
    int high_;
    int delta_;
    int pair_low_ = 0;
    int pair_high_ = 14;
    int anchor_charge_;

    int remaining_[CATION_CHARGE_CLASSES]{};
    signed char site_charge_[CF_SITES]{};
    int oct_sum_[N_OCT]{};
    unsigned char oct_unassigned_[N_OCT]{};
    int site_to_oct_[CF_SITES][OCT_SIZE]{};
    unsigned char site_degree_[CF_SITES]{};

    static int floor_div3(int n)
    {
        return n >= 0 ? n / 3 : -((-n + 2) / 3);
    }

    static int ceil_div3(int n)
    {
        return -floor_div3(-n);
    }

    void make_bounds(int min_add[OCT_SIZE + 1],
                     int max_add[OCT_SIZE + 1]) const
    {
        for (int k = 0; k <= OCT_SIZE; ++k)
        {
            min_add[k] = 0;
            max_add[k] = 0;
        }

        int used = 0;
        int sum = 0;
        for (int q = 0; q < CATION_CHARGE_CLASSES && used < OCT_SIZE; ++q)
        {
            const int c = remaining_[q];
            for (int k = 0; k < c && used < OCT_SIZE; ++k)
            {
                sum += q;
                ++used;
                min_add[used] = sum;
            }
        }

        used = 0;
        sum = 0;
        for (int q = CATION_CHARGE_CLASSES - 1;
             q >= 0 && used < OCT_SIZE; --q)
        {
            const int c = remaining_[q];
            for (int k = 0; k < c && used < OCT_SIZE; ++k)
            {
                sum += q;
                ++used;
                max_add[used] = sum;
            }
        }
    }

    bool adjacent_feasible(int site) const
    {
        int min_add[OCT_SIZE + 1];
        int max_add[OCT_SIZE + 1];
        make_bounds(min_add, max_add);

        for (int k = 0; k < site_degree_[site]; ++k)
        {
            const int o = site_to_oct_[site][k];
            const int sum = oct_sum_[o];
            const int slots = static_cast<int>(oct_unassigned_[o]);

            if (slots == 0)
            {
                if (sum < low_ || sum > high_)
                    return false;
            }
            else
            {
                if (sum + min_add[slots] > high_)
                    return false;
                if (sum + max_add[slots] < low_)
                    return false;
            }
        }

        const int partner = PARTNER[site];
        const int qsite = static_cast<int>(site_charge_[site]);
        const int qpartner = static_cast<int>(site_charge_[partner]);

        if (qpartner >= 0)
        {
            const int pair_sum = qsite + qpartner;
            if (pair_sum < pair_low_ || pair_sum > pair_high_)
                return false;
        }
        else
        {
            int minq = -1;
            int maxq = -1;

            for (int q = 0; q < CATION_CHARGE_CLASSES; ++q)
            {
                if (remaining_[q] > 0)
                {
                    minq = q;
                    break;
                }
            }
            for (int q = CATION_CHARGE_CLASSES - 1; q >= 0; --q)
            {
                if (remaining_[q] > 0)
                {
                    maxq = q;
                    break;
                }
            }

            if (minq >= 0 &&
                (qsite + minq > pair_high_ ||
                 qsite + maxq < pair_low_))
                return false;
        }

        return true;
    }

    bool dfs(int depth)
    {
        if (depth == CF_SITES)
            return true;

        const int site = SITE_ORDER[depth];

        double desired = 0.0;
        for (int k = 0; k < site_degree_[site]; ++k)
        {
            const int o = site_to_oct_[site][k];
            desired +=
                static_cast<double>(balance_ - oct_sum_[o]) /
                static_cast<double>(oct_unassigned_[o]);
        }
        desired /= static_cast<double>(site_degree_[site]);

        pair<double, int> candidates[CATION_CHARGE_CLASSES];
        int nc = 0;

        for (int q = 0; q < CATION_CHARGE_CLASSES; ++q)
        {
            if (remaining_[q] <= 0)
                continue;
            if (depth == 0 && q != anchor_charge_)
                continue;
            if (depth >= 2 && depth <= 5)
            {
                const int previous = SITE_ORDER[depth - 1];
                if (q < site_charge_[previous])
                    continue;
            }

            candidates[nc++] =
                make_pair(abs(static_cast<double>(q) - desired), q);
        }

        for (int i = 1; i < nc; ++i)
        {
            const pair<double, int> key = candidates[i];
            int j = i - 1;
            while (j >= 0 &&
                   (candidates[j].first > key.first ||
                    (candidates[j].first == key.first &&
                     candidates[j].second > key.second)))
            {
                candidates[j + 1] = candidates[j];
                --j;
            }
            candidates[j + 1] = key;
        }

        for (int i = 0; i < nc; ++i)
        {
            const int q = candidates[i].second;

            --remaining_[q];
            site_charge_[site] = static_cast<signed char>(q);
            for (int k = 0; k < site_degree_[site]; ++k)
            {
                const int o = site_to_oct_[site][k];
                oct_sum_[o] += q;
                --oct_unassigned_[o];
            }

            if (adjacent_feasible(site) && dfs(depth + 1))
                return true;

            for (int k = 0; k < site_degree_[site]; ++k)
            {
                const int o = site_to_oct_[site][k];
                oct_sum_[o] -= q;
                ++oct_unassigned_[o];
            }
            site_charge_[site] = -1;
            ++remaining_[q];
        }

        return false;
    }
};

constexpr int FastUniformToleranceSolver::PARTNER[CF_SITES];


//=============================================================================
// SPECIALIZED D>0 SOLVER FOR ONE OCCUPIED ANION CHARGE + ANION VACANCIES
//
// With only one occupied anion class, Hall matching collapses to one simple
// capacity condition: at least N occupied-anion octahedra must still be able
// to reach the target interval; every other octahedron may be a vacancy.
// This avoids the 15-subset Hall loop of the general mixed-anion solver.
//=============================================================================

class FastSingleAnionVacancySolver
{
public:
    FastSingleAnionVacancySolver(
        const int cation_count[CATION_CHARGE_CLASSES],
        int anion_magnitude,
        int occupied_count,
        int delta)
        : target_(6 * anion_magnitude),
          occupied_needed_(occupied_count),
          delta_(delta),
          assigned_sites_(0),
          anchor_charge_(-1)
    {
        int rarest = CF_SITES + 1;
        for (int q = 0; q < CATION_CHARGE_CLASSES; ++q)
        {
            remaining_cation_[q] = cation_count[q];
            if (cation_count[q] > 0 && cation_count[q] < rarest)
            {
                rarest = cation_count[q];
                anchor_charge_ = q;
            }
        }

        for (int s = 0; s < CF_SITES; ++s)
        {
            site_charge_[s] = -1;
            site_degree_[s] = 0;
        }

        for (int o = 0; o < N_OCT; ++o)
        {
            oct_sum_[o] = 0;
            oct_unassigned_[o] = OCT_SIZE;
            for (int k = 0; k < OCT_SIZE; ++k)
            {
                const int site = OCTAHEDRA[o][k];
                site_to_oct_[site][site_degree_[site]++] = o;
            }
        }
    }

    bool solve()
    {
        if (occupied_needed_ <= 0)
            return true;
        if (occupied_needed_ > N_OCT)
            return false;
        return search();
    }

private:
    int target_;
    int occupied_needed_;
    int delta_;
    int assigned_sites_;
    int anchor_charge_;

    int remaining_cation_[CATION_CHARGE_CLASSES]{};
    signed char site_charge_[CF_SITES]{};
    int oct_sum_[N_OCT]{};
    unsigned char oct_unassigned_[N_OCT]{};
    int site_to_oct_[CF_SITES][OCT_SIZE]{};
    unsigned char site_degree_[CF_SITES]{};

    void build_reachable_sums(uint64_t reachable[OCT_SIZE + 1]) const
    {
        for (int k = 0; k <= OCT_SIZE; ++k)
            reachable[k] = 0;
        reachable[0] = uint64_t(1);

        for (int q = 0; q < CATION_CHARGE_CLASSES; ++q)
        {
            const int copies = min(remaining_cation_[q], OCT_SIZE);
            for (int copy = 0; copy < copies; ++copy)
            {
                for (int k = OCT_SIZE - 1; k >= 0; --k)
                {
                    if (reachable[k] != 0)
                        reachable[k + 1] |= (reachable[k] << q);
                }
            }
        }
    }

    static bool interval_reachable(uint64_t sums, int low, int high)
    {
        if (low < 0) low = 0;
        if (high > 42) high = 42;
        if (low > high)
            return false;

        for (int value = low; value <= high; ++value)
        {
            if ((sums & (uint64_t(1) << value)) != 0)
                return true;
        }
        return false;
    }

    bool occupied_capacity_feasible() const
    {
        uint64_t reachable[OCT_SIZE + 1];
        build_reachable_sums(reachable);

        int support = 0;
        for (int o = 0; o < N_OCT; ++o)
        {
            const int missing = static_cast<int>(oct_unassigned_[o]);
            const int low_add = target_ - delta_ - oct_sum_[o];
            const int high_add = target_ + delta_ - oct_sum_[o];

            bool can_be_occupied = false;
            if (missing == 0)
            {
                can_be_occupied =
                    (oct_sum_[o] >= target_ - delta_ &&
                     oct_sum_[o] <= target_ + delta_);
            }
            else
            {
                can_be_occupied =
                    interval_reachable(reachable[missing], low_add, high_add);
            }

            if (can_be_occupied)
            {
                ++support;
                if (support >= occupied_needed_)
                    return true;
            }
        }

        return false;
    }

    void assign_site(int site, int charge)
    {
        site_charge_[site] = static_cast<signed char>(charge);
        --remaining_cation_[charge];
        ++assigned_sites_;

        for (int k = 0; k < site_degree_[site]; ++k)
        {
            const int o = site_to_oct_[site][k];
            oct_sum_[o] += charge;
            --oct_unassigned_[o];
        }
    }

    void unassign_site(int site, int charge)
    {
        for (int k = 0; k < site_degree_[site]; ++k)
        {
            const int o = site_to_oct_[site][k];
            oct_sum_[o] -= charge;
            ++oct_unassigned_[o];
        }

        --assigned_sites_;
        ++remaining_cation_[charge];
        site_charge_[site] = -1;
    }

    bool search()
    {
        if (!occupied_capacity_feasible())
            return false;

        if (assigned_sites_ == CF_SITES)
            return true;

        const int site = SITE_ORDER[assigned_sites_];

        int candidates[CATION_CHARGE_CLASSES];
        int nc = 0;

        for (int q = 0; q < CATION_CHARGE_CLASSES; ++q)
        {
            if (remaining_cation_[q] <= 0)
                continue;

            // Safe because the whole cation sublattice is site-transitive and
            // the anion occupied/vacancy pattern is itself free to transform.
            if (assigned_sites_ == 0 && q != anchor_charge_)
                continue;

            candidates[nc++] = q;
        }

        // Scarcer charge classes first.
        for (int i = 1; i < nc; ++i)
        {
            const int value = candidates[i];
            int j = i - 1;
            while (j >= 0 &&
                   (remaining_cation_[candidates[j]] > remaining_cation_[value] ||
                    (remaining_cation_[candidates[j]] == remaining_cation_[value] &&
                     candidates[j] > value)))
            {
                candidates[j + 1] = candidates[j];
                --j;
            }
            candidates[j + 1] = value;
        }

        for (int i = 0; i < nc; ++i)
        {
            const int q = candidates[i];
            assign_site(site, q);

            if (search())
                return true;

            unassign_site(site, q);
        }

        return false;
    }
};


// Exact necessary congruences A*q=0 modulo 2 and modulo 3.
// Rows are derived by invertible row operations over each prime field,
// eliminating columns in reverse SITE_ORDER. A row is tested as soon as
// its last variable is assigned. No assumption about anion ordering is used.
static const int MOD_ROWS[][34] = {
    {2,31,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,0,1,0,0,0,1},
    {2,30,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,1,0},
    {2,29,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0},
    {2,28,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0},
    {2,27,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,0,0,0,0},
    {2,25,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0},
    {2,24,0,1,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,0,1,0,0,0,0,0,0,0},
    {2,23,0,0,0,1,1,0,0,0,0,0,0,1,0,0,1,0,0,0,0,1,0,1,1,1,0,0,0,0,0,0,0,0},
    {2,21,0,1,0,1,0,0,0,1,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0},
    {2,20,0,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0},
    {2,18,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {2,15,1,0,0,0,0,1,0,0,1,0,0,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {2,14,1,1,0,0,0,0,1,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {2,12,0,0,1,0,1,0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {2,9,1,0,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {2,5,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {3,31,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,0,1,0,0,0,1},
    {3,30,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,1,0},
    {3,29,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0},
    {3,28,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0},
    {3,27,0,0,0,0,0,0,2,2,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,2,0,1,2,1,0,0,0,0},
    {3,26,0,0,0,0,0,0,2,2,0,0,1,1,0,2,2,2,0,0,0,0,0,2,0,2,0,1,1,0,0,0,0,0},
    {3,25,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0},
    {3,24,0,0,0,0,0,0,2,2,0,0,0,0,0,1,0,1,0,0,0,2,2,0,1,0,1,0,0,0,0,0,0,0},
    {3,23,0,0,0,1,2,0,0,0,0,0,0,2,0,0,1,0,0,0,0,2,0,1,2,1,0,0,0,0,0,0,0,0},
    {3,22,0,2,0,1,1,0,0,2,1,0,1,1,2,0,1,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0},
    {3,21,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0},
    {3,20,0,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0},
    {3,19,0,2,0,2,1,2,0,1,0,2,0,1,1,2,2,2,0,2,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
    {3,18,0,0,0,0,1,1,2,2,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {3,17,0,0,0,1,0,1,2,0,0,2,1,1,0,0,0,1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {3,15,1,0,0,0,0,1,0,0,1,0,0,0,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {3,14,1,1,0,0,0,0,1,0,0,0,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {3,13,2,2,0,1,0,2,2,1,2,0,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {3,12,0,0,2,0,1,0,2,2,2,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {3,9,1,0,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {3,5,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const bool MOD2_COUNTS[33] = {true,false,false,false,false,false,true,false,true,false,true,false,true,false,true,false,true,false,true,false,true,false,true,false,true,false,true,false,false,false,false,false,true};

static const bool MOD3_COUNTS[33][33] = {
{true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,true,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,true,false,false,false,false,false,false,true,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,true,false,false,false,true,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,true,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,true,true,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,true,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,true,false,true,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,true,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,true,false,false,false,false,true,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{true,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false},
{true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false}};

class FastAnionCenteredSolver
{
public:
    FastAnionCenteredSolver(const int cation_count[CATION_CHARGE_CLASSES],
                            const int anion_count[ANION_CHARGE_CLASSES],
                            int delta,
                            bool vacancy_must_be_zero)
        : delta_(delta),
          vacancy_must_be_zero_(vacancy_must_be_zero),
          assigned_sites_(0)
    {
        for (int q = 0; q < CATION_CHARGE_CLASSES; ++q)
            remaining_cation_[q] = cation_count[q];

        for (int a = 0; a < ANION_CHARGE_CLASSES; ++a)
            anion_capacity_[a] = anion_count[a];

        for (int s = 0; s < CF_SITES; ++s)
        {
            site_charge_[s] = -1;
            site_degree_[s] = 0;
        }

        for (int o = 0; o < N_OCT; ++o)
        {
            oct_sum_[o] = 0;
            oct_unassigned_[o] = OCT_SIZE;
            for (int k = 0; k < OCT_SIZE; ++k)
            {
                const int s = OCTAHEDRA[o][k];
                site_to_oct_[s][site_degree_[s]++] = o;
            }
        }
    }

    bool solve()
    {
        // With negative delta, the original abs(error) <= delta test can never
        // accept an occupied anion.  Preserve that behavior exactly.
        if (delta_ < 0)
            return anion_capacity_[0] == CF_SITES;

        use_modular_ = delta_ == 0 &&
            (anion_capacity_[0] == 0 || vacancy_must_be_zero_);
        if (use_modular_) {
            int r2=0,r31=0,r32=0;
            for (int q=0;q<8;++q) {
                if(q%2) r2+=remaining_cation_[q];
                if(q%3==1) r31+=remaining_cation_[q];
                if(q%3==2) r32+=remaining_cation_[q];
            }
            if(!MOD2_COUNTS[r2] || !MOD3_COUNTS[r31][r32]) return false;
        }
        int anchor = -1;
        for (int q=0; q<8; ++q)
            if (remaining_cation_[q]>0 && (anchor<0 || remaining_cation_[q]<remaining_cation_[anchor])) anchor=q;
        if (anchor<0) return false;
        // CF32 hypergraph is site-transitive; translate any occurrence of
        // the selected charge to SITE_ORDER[0], translating anions with it.
        // Verified explicit automorphisms are in experiments/symmetry_maps.json.
        assign_site(SITE_ORDER[0], anchor);
        return search();
    }

private:
    bool use_modular_ = false;
    int delta_;
    bool vacancy_must_be_zero_;
    int assigned_sites_;

    int remaining_cation_[CATION_CHARGE_CLASSES];
    int anion_capacity_[ANION_CHARGE_CLASSES];

    signed char site_charge_[CF_SITES];
    int oct_sum_[N_OCT];
    unsigned char oct_unassigned_[N_OCT];

    int site_to_oct_[CF_SITES][OCT_SIZE];
    unsigned char site_degree_[CF_SITES];

    // reachable[k]: bit s is 1 iff exactly k still-unassigned cation sites can
    // be filled from the remaining global stoichiometry with charge sum s.
    void build_reachable_sums(uint64_t reachable[OCT_SIZE + 1]) const
    {
        for (int k = 0; k <= OCT_SIZE; ++k)
            reachable[k] = 0;
        reachable[0] = uint64_t(1);

        for (int q = 0; q < CATION_CHARGE_CLASSES; ++q)
        {
            const int copies = (remaining_cation_[q] < OCT_SIZE)
                             ? remaining_cation_[q]
                             : OCT_SIZE;

            for (int copy = 0; copy < copies; ++copy)
            {
                for (int k = OCT_SIZE - 1; k >= 0; --k)
                {
                    if (reachable[k] != 0)
                        reachable[k + 1] |= (reachable[k] << q);
                }
            }
        }
    }

    bool interval_reachable(uint64_t sums, int low, int high) const
    {
        if (low < 0) low = 0;
        if (high > 42) high = 42; // 6 * max cation charge 7
        if (low > high) return false;

        for (int s = low; s <= high; ++s)
            if (sums & (uint64_t(1) << s))
                return true;
        return false;
    }

    unsigned char anion_domain_for_oct(int o,
                                        const uint64_t reachable[OCT_SIZE + 1]) const
    {
        unsigned char domain = 0;

        const int missing = static_cast<int>(oct_unassigned_[o]);
        const uint64_t possible_add = reachable[missing];

        // A vacancy has no local equation in VERSION 1.  In the important
        // exact charge-balanced case, however, summing all occupied equations
        // proves that every vacancy-centered octahedron must have total cation
        // charge zero.  Use that derived condition as a safe extra pruning rule.
        if (anion_capacity_[0] > 0)
        {
            if (!vacancy_must_be_zero_ ||
                (oct_sum_[o] == 0 && (possible_add & uint64_t(1)) != 0))
            {
                domain |= 1u;
            }
        }

        for (int magnitude = 1; magnitude <= 3; ++magnitude)
        {
            if (anion_capacity_[magnitude] <= 0)
                continue;

            const int target = 6 * magnitude;
            const int low_add  = target - delta_ - oct_sum_[o];
            const int high_add = target + delta_ - oct_sum_[o];

            if (interval_reachable(possible_add, low_add, high_add))
                domain |= static_cast<unsigned char>(1u << magnitude);
        }

        return domain;
    }

    // Generalized Hall test for assigning 32 octahedra to up to four anion
    // classes with exact capacities.  For every class subset S, the number of
    // octahedra whose entire domain lies inside S must not exceed capacity(S).
    bool anion_capacity_feasible(unsigned char domains[N_OCT]) const
    {
        uint64_t reachable[OCT_SIZE + 1];
        build_reachable_sums(reachable);

        for (int o = 0; o < N_OCT; ++o)
        {
            domains[o] = anion_domain_for_oct(o, reachable);
            if (domains[o] == 0)
                return false;
        }

        for (unsigned int subset = 1; subset < 16; ++subset)
        {
            int capacity = 0;
            for (int a = 0; a < ANION_CHARGE_CLASSES; ++a)
                if (subset & (1u << a))
                    capacity += anion_capacity_[a];

            int forced_inside = 0;
            for (int o = 0; o < N_OCT; ++o)
            {
                const unsigned int d = domains[o];
                if ((d & ~subset) == 0)
                {
                    ++forced_inside;
                    if (forced_inside > capacity)
                        return false;
                }
            }
        }

        return true;
    }

    int choose_next_site(const unsigned char domains[N_OCT]) const
    {
        (void)domains;
        if (assigned_sites_ < 0 || assigned_sites_ >= CF_SITES)
            return -1;
        return SITE_ORDER[assigned_sites_];
    }

    void assign_site(int site, int charge)
    {
        site_charge_[site] = static_cast<signed char>(charge);
        --remaining_cation_[charge];
        ++assigned_sites_;

        for (int j = 0; j < site_degree_[site]; ++j)
        {
            const int o = site_to_oct_[site][j];
            oct_sum_[o] += charge;
            --oct_unassigned_[o];
        }
    }

    void unassign_site(int site, int charge)
    {
        for (int j = 0; j < site_degree_[site]; ++j)
        {
            const int o = site_to_oct_[site][j];
            oct_sum_[o] -= charge;
            ++oct_unassigned_[o];
        }

        --assigned_sites_;
        ++remaining_cation_[charge];
        site_charge_[site] = -1;
    }

    bool search()
    {
        unsigned char domains[N_OCT];
        if (!anion_capacity_feasible(domains))
            return false;

        if (assigned_sites_ == CF_SITES)
            return true;

        const int site = choose_next_site(domains);
        if (site < 0)
            return false;

        // Try scarcer charge classes first.  This tends to expose impossible
        // stoichiometric branches early while still preserving completeness.
        int candidates[CATION_CHARGE_CLASSES];
        int candidate_count = 0;
        for (int q = 0; q < CATION_CHARGE_CLASSES; ++q)
            if (remaining_cation_[q] > 0)
                candidates[candidate_count++] = q;

        for (int i = 1; i < candidate_count; ++i)
        {
            const int value = candidates[i];
            int j = i - 1;
            while (j >= 0 &&
                   (remaining_cation_[candidates[j]] > remaining_cation_[value] ||
                    (remaining_cation_[candidates[j]] == remaining_cation_[value] &&
                     candidates[j] > value)))
            {
                candidates[j + 1] = candidates[j];
                --j;
            }
            candidates[j + 1] = value;
        }

        // Restrict only the candidate residues implied by completed linear
        // combinations of exact octahedron equations. All other candidates
        // retain the original exhaustive search and exact Hall test.
        unsigned allowed = 255u;
        if (use_modular_) {
            for (const auto& row : MOD_ROWS) {
                if (row[1] != assigned_sites_) continue;
                int sum = 0;
                for (int j = 0; j < assigned_sites_; ++j)
                    sum += row[j+2] * site_charge_[SITE_ORDER[j]];
                for (int q = 0; q < 8; ++q)
                    if ((sum + q) % row[0] != 0) allowed &= ~(1u << q);
            }
        }
        for (int i = 0; i < candidate_count; ++i)
        {
            const int q = candidates[i];
            if (!(allowed & (1u << q))) continue;
            assign_site(site, q);

            if (search())
                return true;

            unassign_site(site, q);
        }

        return false;
    }
};


static int popcount32(uint32_t x)
{
    int n = 0;
    while (x)
    {
        x &= (x - 1);
        ++n;
    }
    return n;
}

// Exact minimum number of distinct cation sites covered by v vacancy-centered
// octahedra for v <= 4.  This is tiny (at most C(32,4)=35960 combinations) and
// is cached.  For larger v, return the conservative lower bound 6.
static int minimum_zero_sites_for_vacancies(int v)
{
    if (v <= 0) return 0;
    if (v > 4) return 6;

    // Function-local static initialization is thread-safe in C++11 and later.
    // Compute the four small geometry constants exactly once, then only read
    // them from all worker threads.
    static const array<int, 5> cache = []() {
        array<int, 5> result{};
        result[0] = 0;
        result[1] = 6;

        uint32_t oct_mask[N_OCT];
        for (int o = 0; o < N_OCT; ++o)
        {
            uint32_t m = 0;
            for (int k = 0; k < OCT_SIZE; ++k)
                m |= (uint32_t(1) << OCTAHEDRA[o][k]);
            oct_mask[o] = m;
        }

        int best2 = CF_SITES + 1;
        for (int a = 0; a < N_OCT; ++a)
        for (int b = a + 1; b < N_OCT; ++b)
            best2 = min(best2, popcount32(oct_mask[a] | oct_mask[b]));
        result[2] = best2;

        int best3 = CF_SITES + 1;
        for (int a = 0; a < N_OCT; ++a)
        for (int b = a + 1; b < N_OCT; ++b)
        for (int c = b + 1; c < N_OCT; ++c)
            best3 = min(best3, popcount32(oct_mask[a] | oct_mask[b] | oct_mask[c]));
        result[3] = best3;

        int best4 = CF_SITES + 1;
        for (int a = 0; a < N_OCT; ++a)
        for (int b = a + 1; b < N_OCT; ++b)
        for (int c = b + 1; c < N_OCT; ++c)
        for (int d = c + 1; d < N_OCT; ++d)
            best4 = min(best4, popcount32(oct_mask[a] | oct_mask[b] |
                                         oct_mask[c] | oct_mask[d]));
        result[4] = best4;

        return result;
    }();

    return cache[static_cast<size_t>(v)];
}

static uint64_t histogram_key_from_counts(const int count[8])
{
    uint64_t key = 0;
    for (int q = 0; q < 8; ++q)
        key |= (static_cast<uint64_t>(count[q]) << (6 * q));
    return key;
}

static uint64_t histogram_add_pair(uint64_t key, int q, int k)
{
    key += (uint64_t(1) << (6 * q));
    key += (uint64_t(1) << (6 * (k - q)));
    return key;
}

// Specialized exact solver used when all 32 anions have the same nonzero
// charge.  It is the same 10-variable CF32 reduction used by
// Find-local-balance-32-32-a.cpp, retained here as a fast path.
static bool uniform_exact_fast_path(const int cation_count[8], int magnitude)
{
    const int balance = 6 * magnitude;
    const int k = balance / 3; // 2, 4 or 6

    unsigned int domain_mask = 0;
    for (int q = 0; q < 8; ++q)
        if (cation_count[q] > 0)
            domain_mask |= (1u << q);

    // Necessary complementary-pair histogram condition.
    for (int q = 0; q < 8; ++q)
    {
        if (cation_count[q] == 0) continue;
        const int other = k - q;
        if (other < 0 || other > 7 || cation_count[other] == 0)
            return false;
        if (q == other)
        {
            if (cation_count[q] & 1) return false;
        }
        else if (cation_count[q] != cation_count[other])
        {
            return false;
        }
    }

    const int cache_id = (k << 8) | static_cast<int>(domain_mask);

    // This cache is tiny (only a limited number of k/domain combinations), but
    // it is shared by all workers.  Store immutable sets behind shared_ptr so a
    // worker can release the mutex immediately after obtaining the cached set.
    typedef unordered_set<uint64_t> FeasibleSet;
    static unordered_map<int, shared_ptr<const FeasibleSet> > feasible_cache;
    static mutex feasible_cache_mutex;

    shared_ptr<const FeasibleSet> cached_set;
    {
        lock_guard<mutex> lock(feasible_cache_mutex);
        const auto it = feasible_cache.find(cache_id);
        if (it != feasible_cache.end())
            cached_set = it->second;
    }

    if (!cached_set)
    {
        vector<int> D;
        bool domain_ok = true;
        for (int q = 0; q < 8; ++q)
        {
            if (domain_mask & (1u << q))
            {
                const int other = k - q;
                if (other < 0 || other > 7 ||
                    (domain_mask & (1u << other)) == 0)
                {
                    domain_ok = false;
                    break;
                }
                D.push_back(q);
            }
        }

        shared_ptr<FeasibleSet> generated(new FeasibleSet());

        if (domain_ok)
        {
            generated->reserve(4096);

            for (size_t ff = 0; ff < D.size(); ++ff)
            for (size_t gg = 0; gg < D.size(); ++gg)
            for (size_t hh = 0; hh < D.size(); ++hh)
            for (size_t ii = 0; ii < D.size(); ++ii)
            for (size_t jj = 0; jj < D.size(); ++jj)
            {
                const int f=D[ff], g=D[gg], h=D[hh], i=D[ii], j=D[jj];
                const int p5 = -2*k + f+g+h+i+j;
                if (p5 < 0 || p5 > 7 || (domain_mask & (1u << p5)) == 0) continue;

                for (size_t cc = 0; cc < D.size(); ++cc)
                for (size_t ee = 0; ee < D.size(); ++ee)
                for (size_t dd = 0; dd < D.size(); ++dd)
                {
                    const int c=D[cc], e=D[ee], d=D[dd];
                    const int p21 = c+d+e-g-h;
                    if (p21 < 0 || p21 > 7 || (domain_mask & (1u << p21)) == 0) continue;

                    for (size_t bb = 0; bb < D.size(); ++bb)
                    {
                        const int b=D[bb];
                        const int p15 = -2*k+b+c+e+i+j;
                        if (p15 < 0 || p15 > 7 || (domain_mask & (1u << p15)) == 0) continue;

                        for (size_t aa = 0; aa < D.size(); ++aa)
                        {
                            const int a=D[aa];
                            const int p8 = k+a-b+d-h-i;
                            if (p8 < 0 || p8 > 7 || (domain_mask & (1u << p8)) == 0) continue;
                            const int p10 = k+a+d+e-f-g-h-i;
                            if (p10 < 0 || p10 > 7 || (domain_mask & (1u << p10)) == 0) continue;
                            const int p7 = -2*k-a+b+c+f+h+i+j;
                            if (p7 < 0 || p7 > 7 || (domain_mask & (1u << p7)) == 0) continue;

                            uint64_t key = 0;
                            key = histogram_add_pair(key, j,   k);
                            key = histogram_add_pair(key, h,   k);
                            key = histogram_add_pair(key, f,   k);
                            key = histogram_add_pair(key, g,   k);
                            key = histogram_add_pair(key, p5,  k);
                            key = histogram_add_pair(key, i,   k);
                            key = histogram_add_pair(key, p7,  k);
                            key = histogram_add_pair(key, p8,  k);
                            key = histogram_add_pair(key, a,   k);
                            key = histogram_add_pair(key, p10, k);
                            key = histogram_add_pair(key, p15, k);
                            key = histogram_add_pair(key, e,   k);
                            key = histogram_add_pair(key, b,   k);
                            key = histogram_add_pair(key, c,   k);
                            key = histogram_add_pair(key, d,   k);
                            key = histogram_add_pair(key, p21, k);
                            generated->insert(key);
                        }
                    }
                }
            }
        }

        {
            lock_guard<mutex> lock(feasible_cache_mutex);
            const auto inserted =
                feasible_cache.emplace(cache_id,
                    static_pointer_cast<const FeasibleSet>(generated));
            cached_set = inserted.first->second;
        }
    }

    return cached_set->find(histogram_key_from_counts(cation_count)) !=
           cached_set->end();
}

struct ProblemCacheKey
{
    uint64_t cation_counts = 0; // 8 classes x 6 bits = 48 bits
    uint32_t anion_counts = 0;  // 4 classes x 6 bits = 24 bits
    int delta = 0;

    bool operator==(const ProblemCacheKey& other) const
    {
        return cation_counts == other.cation_counts &&
               anion_counts == other.anion_counts &&
               delta == other.delta;
    }
};

struct ProblemCacheKeyHash
{
    size_t operator()(const ProblemCacheKey& key) const
    {
        uint64_t x = key.cation_counts;
        x ^= static_cast<uint64_t>(key.anion_counts) * 0x9E3779B185EBCA87ULL;
        x ^= static_cast<uint64_t>(static_cast<uint32_t>(key.delta)) * 0xC2B2AE3D27D4EB4FULL;

        // MurmurHash3-style 64-bit finalizer.
        x ^= x >> 33;
        x *= 0xff51afd7ed558ccdULL;
        x ^= x >> 33;
        x *= 0xc4ceb9fe1a85ec53ULL;
        x ^= x >> 33;
        return static_cast<size_t>(x);
    }
};

static ProblemCacheKey make_problem_cache_key(
    const int cation_count[CATION_CHARGE_CLASSES],
    const int anion_count[ANION_CHARGE_CLASSES],
    int delta)
{
    ProblemCacheKey key;
    for (int q = 0; q < CATION_CHARGE_CLASSES; ++q)
        key.cation_counts |= (static_cast<uint64_t>(cation_count[q]) << (6 * q));
    for (int a = 0; a < ANION_CHARGE_CLASSES; ++a)
        key.anion_counts |= (static_cast<uint32_t>(anion_count[a]) << (6 * a));
    key.delta = delta;
    return key;
}

struct ProblemCacheShard
{
    mutex guard;
    unordered_map<ProblemCacheKey, bool, ProblemCacheKeyHash> values;

    ProblemCacheShard()
    {
        values.reserve(MAX_CACHE_ENTRIES_PER_SHARD);
        values.max_load_factor(0.80f);
    }
};

struct ProblemCacheState
{
    array<ProblemCacheShard, PROBLEM_CACHE_SHARDS> shards;
    atomic<uint64_t> hits{0};
    atomic<uint64_t> solver_calls{0};
    atomic<uint64_t> insertions_skipped{0};
    atomic<size_t> entries{0};

    bool lookup(const ProblemCacheKey& key, bool& value)
    {
        const size_t hash_value = ProblemCacheKeyHash{}(key);
        ProblemCacheShard& shard =
            shards[hash_value % PROBLEM_CACHE_SHARDS];

        lock_guard<mutex> lock(shard.guard);
        const auto it = shard.values.find(key);
        if (it == shard.values.end())
            return false;

        value = it->second;
        hits.fetch_add(1, memory_order_relaxed);
        return true;
    }

    void store(const ProblemCacheKey& key, bool value)
    {
        const size_t hash_value = ProblemCacheKeyHash{}(key);
        ProblemCacheShard& shard =
            shards[hash_value % PROBLEM_CACHE_SHARDS];

        lock_guard<mutex> lock(shard.guard);

        // Another worker may have solved and inserted the same problem while
        // this worker was computing it.
        if (shard.values.find(key) != shard.values.end())
            return;

        if (shard.values.size() < MAX_CACHE_ENTRIES_PER_SHARD)
        {
            shard.values.emplace(key, value);
            entries.fetch_add(1, memory_order_relaxed);
        }
        else
        {
            insertions_skipped.fetch_add(1, memory_order_relaxed);
        }
    }

    size_t size() const
    {
        return entries.load(memory_order_relaxed);
    }
};

static ProblemCacheState& problem_cache_state()
{
    static ProblemCacheState state;
    return state;
}

} // namespace

static atomic<uint64_t> g_path_uniform_d0{0};
static atomic<uint64_t> g_path_uniform_dpos{0};
static atomic<uint64_t> g_path_single_anion_vacancy{0};
static atomic<uint64_t> g_path_all_anion_vacancy{0};
static atomic<uint64_t> g_path_general_mixed{0};

bool find_anion_centered_variant(
    const int cation_charge[MAX_COMPONENTS + 1],
    const int cation_stoich[MAX_COMPONENTS + 1],
    int col_c,
    const int anion_charge[MAX_COMPONENTS + 1],
    const int anion_stoich[MAX_COMPONENTS + 1],
    int col_a,
    int delta)
{
    // Local-balance feasibility depends only on charge classes and their exact
    // multiplicities, not on the A/B/C or X/Y/Z labels themselves.  Aggregate
    // equal-charge components to remove redundant permutations safely.
    int cation_count[CATION_CHARGE_CLASSES] = {0};
    int anion_count[ANION_CHARGE_CLASSES] = {0};

    for (int i = 1; i <= col_c; ++i)
    {
        const int q = cation_charge[i];
        if (q < 0 || q >= CATION_CHARGE_CLASSES)
            return false;
        cation_count[q] += cation_stoich[i];
    }

    for (int i = 1; i <= col_a; ++i)
    {
        const int z = anion_charge[i];
        if (z > 0 || z < -3)
            return false;
        anion_count[-z] += anion_stoich[i];
    }

    int total_c = 0;
    for (int q = 0; q < CATION_CHARGE_CLASSES; ++q)
        total_c += cation_count[q];

    int total_a = 0;
    for (int a = 0; a < ANION_CHARGE_CLASSES; ++a)
        total_a += anion_count[a];

    if (total_c != CF_SITES || total_a != CF_SITES)
        return false;

    int cation_charge_sum = 0;
    for (int q = 0; q < CATION_CHARGE_CLASSES; ++q)
        cation_charge_sum += q * cation_count[q];

    int anion_abs_charge_sum = 0;
    for (int a = 1; a < ANION_CHARGE_CLASSES; ++a)
        anion_abs_charge_sum += a * anion_count[a];

    // If all 32 anions are identical and D=0, use the same rank-22 / 10-free-
    // variable reduction as the dedicated exact CF32 solver.
    if (delta == 0 && anion_count[0] == 0)
    {
        for (int magnitude = 1; magnitude <= 3; ++magnitude)
        {
            if (anion_count[magnitude] == CF_SITES)
            {
                g_path_uniform_d0.fetch_add(1, memory_order_relaxed);
                return uniform_exact_fast_path(cation_count, magnitude);
            }
        }
    }

    // If all 32 anions are identical and D>0, avoid the general Hall-domain
    // machinery and use the specialized CF32 tolerance solver.
    if (delta > 0 && anion_count[0] == 0)
    {
        for (int magnitude = 1; magnitude <= 3; ++magnitude)
        {
            if (anion_count[magnitude] == CF_SITES)
            {
                g_path_uniform_dpos.fetch_add(1, memory_order_relaxed);
                FastUniformToleranceSolver solver(
                    cation_count, magnitude, delta);
                return solver.solve();
            }
        }
    }

    // All anion positions vacant: there is no anion-centered local equation.
    if (anion_count[0] == CF_SITES)
    {
        g_path_all_anion_vacancy.fetch_add(1, memory_order_relaxed);
        return true;
    }

    // D>0 with exactly one occupied anion charge class plus vacancies:
    // use the cheaper one-capacity solver instead of full 4-class Hall tests.
    if (delta > 0 && anion_count[0] > 0)
    {
        int occupied_classes = 0;
        int occupied_magnitude = 0;
        int occupied_count = 0;

        for (int magnitude = 1; magnitude <= 3; ++magnitude)
        {
            if (anion_count[magnitude] > 0)
            {
                ++occupied_classes;
                occupied_magnitude = magnitude;
                occupied_count = anion_count[magnitude];
            }
        }

        if (occupied_classes == 1)
        {
            g_path_single_anion_vacancy.fetch_add(1, memory_order_relaxed);
            FastSingleAnionVacancySolver solver(
                cation_count,
                occupied_magnitude,
                occupied_count,
                delta);
            return solver.solve();
        }
    }

    bool vacancy_must_be_zero = false;

    // For D=0, summing all 32 octahedral cation sums gives 6 times the total
    // cation charge.  Occupied anions account for their exact target sums; the
    // remainder must be the sum around vacancy-centered octahedra.
    if (delta == 0)
    {
        const long long vacancy_sum_required =
            6LL * static_cast<long long>(cation_charge_sum - anion_abs_charge_sum);
        const int vacancy_count = anion_count[0];

        if (vacancy_count == 0)
        {
            if (vacancy_sum_required != 0)
                return false;
        }
        else
        {
            int min_q = 8;
            int max_q = -1;
            for (int q = 0; q < CATION_CHARGE_CLASSES; ++q)
            {
                if (cation_count[q] > 0)
                {
                    min_q = min(min_q, q);
                    max_q = max(max_q, q);
                }
            }

            const long long min_possible =
                6LL * vacancy_count * static_cast<long long>(min_q);
            const long long max_possible =
                6LL * vacancy_count * static_cast<long long>(max_q);

            if (vacancy_sum_required < min_possible ||
                vacancy_sum_required > max_possible)
                return false;

            if (vacancy_sum_required == 0)
            {
                vacancy_must_be_zero = true;

                // Every vacancy octahedron must consist entirely of q=0 sites.
                // For up to four vacancies compute the exact minimum union size
                // of such octahedra and reject impossible stoichiometries now.
                const int required_zero_sites =
                    minimum_zero_sites_for_vacancies(vacancy_count);
                if (cation_count[0] < required_zero_sites)
                    return false;
            }
        }
    }

    // Cheap global necessary condition when there are no anion vacancies:
    // every cation site belongs to six anion-centered octahedra.
    if (anion_count[0] == 0 && delta >= 0)
    {
        const long long global_error =
            6LL * (static_cast<long long>(cation_charge_sum) -
                   static_cast<long long>(anion_abs_charge_sum));
        const long long max_error = 32LL * static_cast<long long>(delta);

        if (global_error < -max_error || global_error > max_error)
            return false;
    }

    g_path_general_mixed.fetch_add(1, memory_order_relaxed);

    // Repeated stoichiometries/variants with the same charge histogram are
    // mathematically identical for this filter. Reuse results, but keep the
    // cache bounded so RAM cannot grow with an arbitrarily large input file.
    ProblemCacheState& cache_state = problem_cache_state();
    const ProblemCacheKey key =
        make_problem_cache_key(cation_count, anion_count, delta);

    bool cached_result = false;
    if (cache_state.lookup(key, cached_result))
        return cached_result;

    FastAnionCenteredSolver solver(
        cation_count, anion_count, delta, vacancy_must_be_zero);
    const bool result = solver.solve();
    cache_state.solver_calls.fetch_add(1, memory_order_relaxed);

    cache_state.store(key, result);
    return result;
}

struct WorkItem
{
    uint64_t input_line = 0;
    string original_line;
    array<int, MAX_COMPONENTS + 1> cation_charge{};
    array<int, MAX_COMPONENTS + 1> cation_stoich{};
    array<int, MAX_COMPONENTS + 1> anion_charge{};
    array<int, MAX_COMPONENTS + 1> anion_stoich{};
    int col_c = 0;
    int col_a = 0;
};

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    int file_number = 0;
    int delta = 0;

    cout << "Enter stoichiometry file number (e.g. 84): ";
    cin >> file_number;
    cout << "Enter allowed deviation delta (e.g. 0): ";
    cin >> delta;

    if (!cin)
    {
        cerr << "Error: Invalid input." << endl;
        return 1;
    }

    char filename[160];
    char output_filename[200];

    sprintf(filename, "stoichiometry_%d_32_32.txt", file_number);
    sprintf(output_filename,
            "local_balance_%d_%d.txt",
            file_number,
            delta);

    FILE* Fpdat = fopen(filename, "r");
    if (Fpdat == NULL)
    {
        cout << "Error: Cannot open file " << filename << endl;
        return 1;
    }

    FILE* Fout = fopen(output_filename, "w");
    if (Fout == NULL)
    {
        cout << "Error: Cannot create file " << output_filename << endl;
        fclose(Fpdat);
        return 1;
    }

    vector<char> input_io_buffer(1u << 20);
    vector<char> output_io_buffer(1u << 20);
    setvbuf(Fpdat, input_io_buffer.data(), _IOFBF, input_io_buffer.size());
    setvbuf(Fout, output_io_buffer.data(), _IOFBF, output_io_buffer.size());

    unsigned int thread_count = thread::hardware_concurrency();
    if (thread_count == 0)
        thread_count = 1;

    // Keep the queue deliberately small.  The producer blocks when workers fall
    // behind, therefore RAM usage is bounded independently of input-file size.
    const size_t max_pending_jobs =
        max<size_t>(256, static_cast<size_t>(thread_count) * 32ULL);

    deque<WorkItem> work_queue;
    mutex queue_mutex;
    condition_variable queue_not_empty;
    condition_variable queue_not_full;
    bool producer_done = false;

    mutex output_mutex;
    mutex console_mutex;

    atomic<uint64_t> jobs_completed{0};
    atomic<uint64_t> found{0};

    const auto start_time = chrono::steady_clock::now();
    uint64_t lines_read = 0;
    uint64_t lines_valid = 0;
    uint64_t lines_invalid = 0;
    uint64_t parse_warnings = 0;

    cout << "======================================================" << endl;
    cout << "CF32 V4 EXACT MODULAR PRUNING: uniform + mixed anions" << endl;
    cout << "Worker threads: " << thread_count << endl;
    cout << "Max pending jobs: " << max_pending_jobs << endl;
    cout << "Input file is streamed; it is NOT stored in RAM" << endl;
    cout << "Accepted lines are written + fflush() immediately" << endl;
    cout << "With >1 worker, output-line order may differ from input order" << endl;
    cout << "General mixed-path cache cap: about " << MAX_PROBLEM_CACHE_ENTRIES << endl;
    cout << "Problem-cache shards: " << PROBLEM_CACHE_SHARDS << endl;
    cout << "Progress interval: " << PROGRESS_INTERVAL << " input records" << endl;
    cout << "======================================================" << endl;

    auto worker = [&]()
    {
        for (;;)
        {
            WorkItem job;

            {
                unique_lock<mutex> lock(queue_mutex);
                queue_not_empty.wait(lock, [&]() {
                    return producer_done || !work_queue.empty();
                });

                if (work_queue.empty())
                {
                    if (producer_done)
                        break;
                    continue;
                }

                job = std::move(work_queue.front());
                work_queue.pop_front();
                queue_not_full.notify_one();
            }

            const bool ok = find_anion_centered_variant(
                job.cation_charge.data(),
                job.cation_stoich.data(),
                job.col_c,
                job.anion_charge.data(),
                job.anion_stoich.data(),
                job.col_a,
                delta);

            if (ok)
            {
                // Serialize only the very short physical output operation.
                // Flush BEFORE printing "Found", preserving the safety property
                // of the reviewed single-thread streaming version.
                lock_guard<mutex> out_lock(output_mutex);

                fputs(job.original_line.c_str(), Fout);
                if (!job.original_line.empty() &&
                    job.original_line[job.original_line.size() - 1] != '\n')
                {
                    fputc('\n', Fout);
                }
                fflush(Fout);

                const uint64_t found_now =
                    found.fetch_add(1, memory_order_relaxed) + 1;

                lock_guard<mutex> console_lock(console_mutex);
                cout << "Found variant for input line " << job.input_line
                     << " (total found " << found_now << ")" << endl;
                cout.flush();
            }

            jobs_completed.fetch_add(1, memory_order_relaxed);
        }
    };

    vector<thread> workers;
    workers.reserve(thread_count);
    for (unsigned int i = 0; i < thread_count; ++i)
        workers.emplace_back(worker);

    char line_buffer[4096];

    while (fgets(line_buffer, sizeof(line_buffer), Fpdat) != NULL)
    {
        ++lines_read;
        const string original_line(line_buffer);

        bool only_space = true;
        for (size_t i = 0; i < original_line.size(); ++i)
        {
            if (!isspace(static_cast<unsigned char>(original_line[i])))
            {
                only_space = false;
                break;
            }
        }
        if (only_space)
            continue;

        const size_t separator = original_line.find('|');
        if (separator == string::npos)
        {
            ++lines_invalid;
            ++parse_warnings;
            if (parse_warnings <= 20)
            {
                lock_guard<mutex> console_lock(console_mutex);
                cout << "Warning: line " << lines_read
                     << " has no | separator; skipped." << endl;
            }
            else if (parse_warnings == 21)
            {
                lock_guard<mutex> console_lock(console_mutex);
                cout << "Warning: further parse warnings are suppressed." << endl;
            }
            continue;
        }

        const string cation_side = original_line.substr(0, separator);
        const string anion_side = original_line.substr(separator + 1);

        WorkItem job;
        job.input_line = lines_read;
        job.original_line = original_line;

        if (!parse_components(cation_side,
                              true,
                              job.cation_charge.data(),
                              job.cation_stoich.data(),
                              job.col_c))
        {
            ++lines_invalid;
            ++parse_warnings;
            if (parse_warnings <= 20)
            {
                lock_guard<mutex> console_lock(console_mutex);
                cout << "Warning: cannot parse cations in line "
                     << lines_read << "; skipped." << endl;
            }
            else if (parse_warnings == 21)
            {
                lock_guard<mutex> console_lock(console_mutex);
                cout << "Warning: further parse warnings are suppressed." << endl;
            }
            continue;
        }

        if (!parse_components(anion_side,
                              false,
                              job.anion_charge.data(),
                              job.anion_stoich.data(),
                              job.col_a))
        {
            ++lines_invalid;
            ++parse_warnings;
            if (parse_warnings <= 20)
            {
                lock_guard<mutex> console_lock(console_mutex);
                cout << "Warning: cannot parse anions in line "
                     << lines_read << "; skipped." << endl;
            }
            else if (parse_warnings == 21)
            {
                lock_guard<mutex> console_lock(console_mutex);
                cout << "Warning: further parse warnings are suppressed." << endl;
            }
            continue;
        }

        if (job.col_a < 1 || job.col_a > 4)
        {
            {
                lock_guard<mutex> console_lock(console_mutex);
                cout << "Warning: line " << lines_read
                     << " has " << job.col_a
                     << " anion components; expected 1..4; skipped." << endl;
            }
            ++lines_invalid;
            continue;
        }

        if (stoichiometry_sum(job.cation_stoich.data(), job.col_c) != CF_SITES)
        {
            {
                lock_guard<mutex> console_lock(console_mutex);
                cout << "Warning: cation stoichiometry in line " << lines_read
                     << " does not sum to 32; skipped." << endl;
            }
            ++lines_invalid;
            continue;
        }

        if (stoichiometry_sum(job.anion_stoich.data(), job.col_a) != CF_SITES)
        {
            {
                lock_guard<mutex> console_lock(console_mutex);
                cout << "Warning: anion stoichiometry in line " << lines_read
                     << " does not sum to 32; skipped." << endl;
            }
            ++lines_invalid;
            continue;
        }

        ++lines_valid;

        {
            unique_lock<mutex> lock(queue_mutex);
            queue_not_full.wait(lock, [&]() {
                return work_queue.size() < max_pending_jobs;
            });
            work_queue.push_back(std::move(job));
        }
        queue_not_empty.notify_one();

        if ((lines_read % PROGRESS_INTERVAL) == 0)
        {
            const auto now_tp = chrono::steady_clock::now();
            const double elapsed =
                chrono::duration<double>(now_tp - start_time).count();
            ProblemCacheState& cache_state = problem_cache_state();

            size_t queued_now = 0;
            {
                lock_guard<mutex> lock(queue_mutex);
                queued_now = work_queue.size();
            }

            lock_guard<mutex> console_lock(console_mutex);
            cout << "[progress] read=" << lines_read
                 << " valid=" << lines_valid
                 << " completed=" << jobs_completed.load(memory_order_relaxed)
                 << " queued=" << queued_now
                 << " found=" << found.load(memory_order_relaxed)
                 << " solver_calls=" << cache_state.solver_calls.load(memory_order_relaxed)
                 << " cache_hits=" << cache_state.hits.load(memory_order_relaxed)
                 << " cache_size=" << cache_state.size()
                 << " elapsed=" << elapsed << " s" << endl;
            cout.flush();
        }
    }

    fclose(Fpdat);

    {
        lock_guard<mutex> lock(queue_mutex);
        producer_done = true;
    }
    queue_not_empty.notify_all();

    for (thread& t : workers)
        t.join();

    // Every accepted record was already fflush()'ed by its worker.  This final
    // flush/close is still performed for a clean normal shutdown.
    fflush(Fout);
    fclose(Fout);

    const auto finish_time = chrono::steady_clock::now();
    const double elapsed =
        chrono::duration<double>(finish_time - start_time).count();

    ProblemCacheState& cache_state = problem_cache_state();

    cout << "======================================================" << endl;
    cout << "Time: " << elapsed << " seconds" << endl;
    cout << "Worker threads: " << thread_count << endl;
    cout << "Input lines read: " << lines_read << endl;
    cout << "Valid stoichiometry lines: " << lines_valid << endl;
    cout << "Invalid/skipped lines: " << lines_invalid << endl;
    cout << "Jobs completed: " << jobs_completed.load(memory_order_relaxed) << endl;
    cout << "Total found: " << found.load(memory_order_relaxed) << endl;
    cout << "General Hall solver calls: "
         << cache_state.solver_calls.load(memory_order_relaxed) << endl;
    cout << "Cache hits: "
         << cache_state.hits.load(memory_order_relaxed) << endl;
    cout << "Cached problems: " << cache_state.size() << endl;
    cout << "Cache insertions skipped after shard caps: "
         << cache_state.insertions_skipped.load(memory_order_relaxed) << endl;
    cout << "Path uniform D=0: "
         << g_path_uniform_d0.load(memory_order_relaxed) << endl;
    cout << "Path uniform D>0: "
         << g_path_uniform_dpos.load(memory_order_relaxed) << endl;
    cout << "Path one anion type + V, D>0: "
         << g_path_single_anion_vacancy.load(memory_order_relaxed) << endl;
    cout << "Path all-anion vacancies: "
         << g_path_all_anion_vacancy.load(memory_order_relaxed) << endl;
    cout << "Path general mixed: "
         << g_path_general_mixed.load(memory_order_relaxed) << endl;
    cout << "Parse warnings: " << parse_warnings << endl;
    cout << "Output: " << output_filename << endl;
    cout << "NOTE: multicore output order may differ from input order." << endl;

    return 0;
}
