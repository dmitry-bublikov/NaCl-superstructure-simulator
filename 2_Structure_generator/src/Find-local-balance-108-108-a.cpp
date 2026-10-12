//===========================================================================
// PROGRAM: Find-exact-local-balance_cf108
// Exact local-valence-balance solver for the CF108 cation sublattice.
// Streaming edition for multi-million-line stoichiometry files.
//
// Geometry
// --------
// CF108 uses the 6 x 6 x 6 quarter-grid (step 0.2500, internal period 1.5000)
// with 108 cation sites and 108 anion-centered octahedra.
//
// For a uniform anion charge |z_A|, every anion-centered octahedron obeys
//
//     p_i1 + p_i2 + ... + p_i6 = B,   B = 6 * |z_A|.
//
// Exact CF108 reduction
// ---------------------
// For the supplied CF108 numbering, the 108 x 108 octahedral coefficient
// matrix has rank 96, hence the exact solution space has dimension 12.
//
// Moreover, exact balance forces the 108 cation sites into 27 quartets.
// All four sites in a quartet always have the same charge. Consequently,
// every cation stoichiometric count must be divisible by 4.
//
// The 12 independent quartet charges are:
//
//   a: (p6, p44, p73, p100)
//   b: (p23,p43, p71, p98)
//   c: (p10,p80, p85, p102)
//   d: (p33,p42, p70, p99)
//   e: (p3, p40, p76, p97)
//   f: (p34,p41, p72, p101)
//   g: (p24,p38, p67, p92)
//   h: (p12,p28, p69, p90)
//   i: (p45,p53, p54, p108)
//   j: (p15,p25, p59, p62)
//   k: (p22,p26, p49, p56)
//   l: (p16,p78, p86, p105)
//
// The remaining 15 quartet charges are exact affine functions of a...l.
// The program backtracks only over these 12 independent quartet charges,
// while enforcing the requested 27-group stoichiometric histogram.
//
// Supported input
// ---------------
// * CF108: exactly 108 cation sites per stoichiometry.
// * Cation charges: integer 0...7.
// * Different cation components must have different charges.
// * Vacancy is charge 0 (for example: V12 0).
// * Up to 8 cation components (A...G plus V).
// * Uniform anion charge supplied interactively (absolute charge 0...3).
// * Exact local balance only: D = 0.
// * Mixed-anion local targets are NOT handled by this program.
// * Streaming input: only one stoichiometry line is retained at a time.
// * Bounded histogram cache prevents RAM growth with input-file size.
//
// MSVC 19.44 / Visual Studio 2022
// ------------------------------
//   cl Find-exact-local-balance_cf108.cpp /O2 /Oi /Ot /EHsc /std:c++17 /Fe:Find-exact-local-balance_cf108_streaming.exe
//===========================================================================

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

namespace {

constexpr int N_SITES = 108;
constexpr int N_GROUPS = 27;
constexpr int GROUP_SIZE = 4;
constexpr int N_FREE = 12;
constexpr int N_DEP = 15;
constexpr int MAX_COMPONENTS = 8;
constexpr int MIN_CHARGE = 0;
constexpr int MAX_CHARGE = 7;
constexpr uint64_t PROGRESS_INTERVAL = 1000000ULL;
constexpr size_t MAX_CACHE_ENTRIES = 1000000ULL;

struct Component {
    char label = '?';
    int count = 0;
    int charge = 0;
};

struct ParsedLine {
    vector<Component> components;
    bool ok = false;
    string error;
};

struct Record {
    string original_line;
    vector<Component> components;
    array<int, 8> counts{};
    uint64_t histogram_key = 0;
    bool valid_input = false;
    bool found = false;
};

struct DepExpr {
    array<int8_t, N_FREE> coeff{};
    int8_t z_factor = 0;  // x = z_factor*z + sum(coeff[j]*u[j])
};

// Independent-variable names:
// 0=a, 1=b, 2=c, 3=d, 4=e, 5=f,
// 6=g, 7=h, 8=i, 9=j, 10=k, 11=l.
//
// 15 dependent quartets, in the same order as DEP_QUARTETS below.
static constexpr array<DepExpr, N_DEP> DEP = {{
    {{{ 0, 1, 0, 0,-1, 0, 0, 1, 0, 0, 1,-1}},  0}, // p1
    {{{ 0,-1, 0, 0, 0,-1,-1, 0, 0,-1,-1, 0}},  6}, // p2
    {{{-1, 0, 1,-1, 0, 0, 1, 0, 0, 0, 0, 1}},  0}, // p4
    {{{ 0, 0,-1, 1, 0, 1, 0,-1, 0, 1, 0, 0}},  0}, // p5
    {{{ 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1}}, -3}, // p7
    {{{ 0,-1,-1, 0, 0, 0,-1,-1,-1, 0, 0, 0}},  6}, // p8
    {{{ 1, 0, 0, 0, 1, 0, 0, 0, 1,-1,-1, 0}},  0}, // p9
    {{{-1, 0, 0,-1, 0, 0, 0, 0,-1, 0, 1, 0}},  3}, // p11
    {{{ 0, 0, 0, 1,-1, 0,-1, 0, 0, 0, 0,-1}},  3}, // p13
    {{{ 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0}}, -3}, // p14
    {{{ 0,-1,-1, 0, 1, 0, 0,-1, 0, 0, 0, 0}},  3}, // p17
    {{{ 0, 0, 1, 0, 0,-1, 0, 0, 0,-1,-1, 0}},  3}, // p18
    {{{-1,-1, 0,-1, 0,-1, 0, 0,-1, 0, 0, 0}},  6}, // p19
    {{{ 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}}, -3}, // p20
    {{{ 0, 0, 0, 0,-1,-1,-1, 0,-1, 0, 0,-1}},  6}  // p35
}};

// Four actual CF108 site numbers represented by each dependent quartet.
static constexpr int DEP_QUARTETS[N_DEP][4] = {
    { 1, 88, 89,106},
    { 2, 52, 61, 64},
    { 4, 51, 60, 65},
    { 5, 81, 87,104},
    { 7, 39, 74, 95},
    { 8, 31, 75, 94},
    { 9, 79, 84,103},
    {11, 36, 68, 91},
    {13, 27, 30, 32},
    {14, 29, 47, 55},
    {17, 50, 58, 63},
    {18, 21, 77, 96},
    {19, 82, 83,107},
    {20, 46, 48, 57},
    {35, 37, 66, 93}
};

// Search order chosen to complete the 4/5-variable dependent expressions early:
// i, b, f, g, c, h, e, l, j, k, a, d.
static constexpr array<int, N_FREE> SEARCH_ORDER = {
    8, 1, 5, 6, 2, 7, 4, 11, 9, 10, 0, 3
};

static bool is_space_char(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

static tm local_tm_from_time(time_t value)
{
    tm result{};
#ifdef _MSC_VER
    localtime_s(&result, &value);
#else
    localtime_r(&value, &result);
#endif
    return result;
}

static ParsedLine parse_stoichiometry_line(const string& input)
{
    ParsedLine result;

    string cation_part = input;
    const size_t pipe_pos = cation_part.find('|');
    if (pipe_pos != string::npos) {
        cation_part.resize(pipe_pos);
    }

    vector<string> tokens;
    tokens.reserve(16);
    result.components.reserve(MAX_COMPONENTS);
    string token;

    for (char ch : cation_part) {
        if (is_space_char(ch)) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token.push_back(ch);
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }

    for (size_t i = 0;
         i < tokens.size() && result.components.size() < MAX_COMPONENTS;
         ++i) {

        const string& t = tokens[i];
        if (t.empty()) {
            continue;
        }

        const char label = t[0];
        if (!((label >= 'A' && label <= 'G') || label == 'V')) {
            continue;
        }

        size_t pos = 1;
        if (pos >= t.size() || t[pos] < '0' || t[pos] > '9') {
            result.error = "Cannot read stoichiometric count from token: " + t;
            return result;
        }

        int count = 0;
        while (pos < t.size() && t[pos] >= '0' && t[pos] <= '9') {
            count = count * 10 + (t[pos] - '0');
            ++pos;
        }

        int charge = 0;

        if (pos < t.size() && (t[pos] == '+' || t[pos] == '-')) {
            const int sign = (t[pos] == '-') ? -1 : 1;
            ++pos;

            if (pos >= t.size() || t[pos] < '0' || t[pos] > '9') {
                result.error = "Cannot read charge from token: " + t;
                return result;
            }

            while (pos < t.size() && t[pos] >= '0' && t[pos] <= '9') {
                charge = charge * 10 + (t[pos] - '0');
                ++pos;
            }
            charge *= sign;
        } else {
            if (i + 1 >= tokens.size()) {
                result.error = "Missing separate charge after token: " + t;
                return result;
            }

            try {
                charge = stoi(tokens[++i]);
            } catch (...) {
                result.error = "Cannot read separate charge after token: " + t;
                return result;
            }
        }

        if (charge < MIN_CHARGE || charge > MAX_CHARGE) {
            result.error = "Cation charge is outside supported range 0...7";
            return result;
        }

        if (count < 0 || count > N_SITES) {
            result.error = "Stoichiometric count is outside supported range 0...108";
            return result;
        }

        result.components.push_back(Component{label, count, charge});
    }

    if (result.components.empty()) {
        result.error = "No cation components found";
        return result;
    }

    for (size_t i = 0; i < result.components.size(); ++i) {
        for (size_t j = i + 1; j < result.components.size(); ++j) {
            if (result.components[i].charge == result.components[j].charge) {
                result.error = "Two cation components have the same charge";
                return result;
            }
        }
    }

    int total_count = 0;
    for (const Component& component : result.components) {
        total_count += component.count;
    }

    if (total_count != N_SITES) {
        result.error = "Exact CF108 mode requires exactly 108 cation sites";
        return result;
    }

    result.ok = true;
    return result;
}

// 7 bits per charge count: 8 * 7 = 56 bits.
// CF108 counts are <=108, so 7 bits are sufficient.
static uint64_t make_histogram_key(const array<int,8>& counts)
{
    uint64_t key = 0;
    for (int q = 0; q <= 7; ++q) {
        key |= static_cast<uint64_t>(counts[q]) << (7 * q);
    }
    return key;
}

static array<int,8> make_counts(const vector<Component>& components)
{
    array<int,8> counts{};
    for (const Component& c : components) {
        counts[c.charge] += c.count;
    }
    return counts;
}

static bool all_counts_divisible_by_four(const array<int,8>& counts)
{
    for (int q = 0; q <= 7; ++q) {
        if ((counts[q] % GROUP_SIZE) != 0) {
            return false;
        }
    }
    return true;
}

static int total_cation_charge(const array<int,8>& counts)
{
    int total = 0;
    for (int q = 0; q <= 7; ++q) {
        total += q * counts[q];
    }
    return total;
}

class ExactCF108Solver {
public:
    ExactCF108Solver(const array<int,8>& full_counts, int anion_charge)
        : z_(anion_charge)
    {
        for (int q = 0; q <= 7; ++q) {
            remaining_[q] = full_counts[q] / GROUP_SIZE;
            if (remaining_[q] > 0) {
                domain_mask_ |= static_cast<uint16_t>(1u << q);
            }
        }

        values_.fill(-1);
        completed_.fill(false);

        for (int ci = 0; ci < N_DEP; ++ci) {
            int last_depth = -1;
            for (int j = 0; j < N_FREE; ++j) {
                if (DEP[ci].coeff[j] == 0) {
                    continue;
                }
                for (int d = 0; d < N_FREE; ++d) {
                    if (SEARCH_ORDER[d] == j) {
                        if (d > last_depth) last_depth = d;
                        break;
                    }
                }
            }
            completion_depth_[ci] = last_depth;
        }
    }

    bool solve()
    {
        // 27 quartet slots must be consumed.
        int groups = 0;
        for (int q = 0; q <= 7; ++q) groups += remaining_[q];
        if (groups != N_GROUPS) return false;

        return dfs(0);
    }

    uint64_t visited_nodes() const { return visited_nodes_; }

private:
    int z_ = 0;
    uint16_t domain_mask_ = 0;
    array<int,8> remaining_{};
    array<int,N_FREE> values_{};
    array<bool,N_DEP> completed_{};
    array<int,N_DEP> completion_depth_{};
    uint64_t visited_nodes_ = 0;

    int eval_dep(int ci) const
    {
        int value = DEP[ci].z_factor * z_;
        for (int j = 0; j < N_FREE; ++j) {
            const int c = DEP[ci].coeff[j];
            if (c != 0) {
                value += c * values_[j];
            }
        }
        return value;
    }

    // Exact residual-sum feasibility for one not-yet-complete dependent
    // expression. It ignores multiplicities among the future independent
    // variables, so it is a safe necessary test (never rejects a real solution).
    bool dep_can_still_reach_available_charge(int ci) const
    {
        int partial = DEP[ci].z_factor * z_;

        // Possible residual sums lie within [-35,+35].
        constexpr int OFFSET = 40;
        constexpr int SIZE = 81;
        array<uint8_t,SIZE> possible{};
        possible[OFFSET] = 1;

        for (int j = 0; j < N_FREE; ++j) {
            const int coeff = DEP[ci].coeff[j];
            if (coeff == 0) continue;

            if (values_[j] >= 0) {
                partial += coeff * values_[j];
                continue;
            }

            array<uint8_t,SIZE> next{};
            for (int s = -35; s <= 35; ++s) {
                if (!possible[s + OFFSET]) continue;

                for (int q = 0; q <= 7; ++q) {
                    if (remaining_[q] <= 0) continue;
                    const int ns = s + coeff * q;
                    if (ns >= -35 && ns <= 35) {
                        next[ns + OFFSET] = 1;
                    }
                }
            }
            possible = next;
        }

        for (int s = -35; s <= 35; ++s) {
            if (!possible[s + OFFSET]) continue;
            const int value = partial + s;
            if (value >= 0 && value <= 7 && remaining_[value] > 0) {
                return true;
            }
        }
        return false;
    }

    bool forward_check() const
    {
        for (int ci = 0; ci < N_DEP; ++ci) {
            if (completed_[ci]) continue;
            if (!dep_can_still_reach_available_charge(ci)) {
                return false;
            }
        }
        return true;
    }

    bool dfs(int depth)
    {
        ++visited_nodes_;

        if (depth == N_FREE) {
            for (int q = 0; q <= 7; ++q) {
                if (remaining_[q] != 0) return false;
            }
            return true;
        }

        const int var = SEARCH_ORDER[depth];

        // Try only charges still needed by the target histogram.
        for (int q = 0; q <= 7; ++q) {
            if ((domain_mask_ & static_cast<uint16_t>(1u << q)) == 0) continue;
            if (remaining_[q] <= 0) continue;

            values_[var] = q;
            --remaining_[q];

            bool ok = true;
            array<int,N_DEP> newly_completed{};
            int n_new = 0;

            // Evaluate each dependent quartet exactly at the first depth at
            // which all of its independent variables have been assigned.
            for (int ci = 0; ci < N_DEP; ++ci) {
                if (completion_depth_[ci] != depth) continue;

                const int value = eval_dep(ci);
                if (value < 0 || value > 7 || remaining_[value] <= 0) {
                    ok = false;
                    break;
                }

                --remaining_[value];
                completed_[ci] = true;
                newly_completed[n_new++] = ci;
            }

            if (ok && forward_check()) {
                if (dfs(depth + 1)) {
                    // No need to restore on a successful path because solve()
                    // returns immediately.
                    return true;
                }
            }

            // Restore dependent groups completed at this depth.
            for (int r = n_new - 1; r >= 0; --r) {
                const int ci = newly_completed[r];
                const int value = eval_dep(ci);
                ++remaining_[value];
                completed_[ci] = false;
            }

            ++remaining_[q];
            values_[var] = -1;
        }

        return false;
    }
};

} // namespace

int main()
{
    int file_number = 0;
    int anion_charge = 0;

    cout << "Exact CF108 local-balance solver (STREAMING, D = 0)\n";
    cout << "Enter stoichiometry file number (e.g., 31, 71): ";
    cin >> file_number;

    cout << "Enter absolute UNIFORM anion charge for local balance (0, 1, 2, 3): ";
    cin >> anion_charge;

    if (!cin) {
        cerr << "Error: Invalid input.\n";
        return 1;
    }

    if (anion_charge < 0 || anion_charge > 3) {
        cerr << "Error: Supported absolute anion charge is 0...3.\n";
        return 1;
    }

    const int local_balance = anion_charge * 6;

    char input_filename[100];
    snprintf(input_filename, sizeof(input_filename),
             "stoichiometry_%d.txt", file_number);

    FILE* input = fopen(input_filename, "r");
    if (input == nullptr) {
        cerr << "Error: Cannot open file " << input_filename << '\n';
        return 1;
    }

    char output_filename[120];
    snprintf(output_filename, sizeof(output_filename),
             "local_balance_cf108_%d_%d_0.txt",
             file_number, local_balance);

    FILE* output = fopen(output_filename, "w");
    if (output == nullptr) {
        cerr << "Error: Cannot create file " << output_filename << '\n';
        fclose(input);
        return 1;
    }

    // Large stdio buffers improve throughput on multi-million-line files.
    // Their lifetime extends until both FILE streams are closed.
    vector<char> input_io_buffer(1u << 20);
    vector<char> output_io_buffer(1u << 20);
    setvbuf(input, input_io_buffer.data(), _IOFBF, input_io_buffer.size());
    setvbuf(output, output_io_buffer.data(), _IOFBF, output_io_buffer.size());

    const auto start = chrono::steady_clock::now();
    const time_t now = time(nullptr);
    const tm start_tm = local_tm_from_time(now);

    cout << "==============================================\n";
    cout << "PROGRAM START: ";
    printf("%04d-%02d-%02d %02d:%02d:%02d\n",
           start_tm.tm_year + 1900,
           start_tm.tm_mon + 1,
           start_tm.tm_mday,
           start_tm.tm_hour,
           start_tm.tm_min,
           start_tm.tm_sec);
    cout << "CF108 exact mode: D = 0, B = " << local_balance << '\n';
    cout << "Reduction: 108 sites -> 27 quartets -> 12 independent groups\n";
    cout << "Streaming mode: input lines are NOT stored in RAM\n";
    cout << "Progress interval: " << PROGRESS_INTERVAL << " input records\n";
    cout << "Cache cap: " << MAX_CACHE_ENTRIES << " solved histograms\n";
    cout << "==============================================\n";

    // Cache only histograms that survive the cheap filters.  Unlike the old
    // implementation, this cache is NOT reserved to the size of the input
    // file and it has a hard cap, keeping memory bounded on huge files.
    unordered_map<uint64_t, bool> cache;
    cache.reserve(65536);
    cache.max_load_factor(0.80f);

    uint64_t input_record_number = 0;
    uint64_t valid_record_number = 0;
    uint64_t found_count = 0;
    uint64_t rejected_not_multiple4 = 0;
    uint64_t rejected_global_charge = 0;
    uint64_t passed_global_charge = 0;
    uint64_t passed_multiple4 = 0;
    uint64_t solver_calls = 0;
    uint64_t cache_hits = 0;
    uint64_t cache_insertions_skipped = 0;
    uint64_t total_search_nodes = 0;
    uint64_t parse_warnings = 0;

    char buffer[4096];

    while (fgets(buffer, sizeof(buffer), input) != nullptr) {
        string original_line(buffer);

        bool only_spaces = true;
        for (char ch : original_line) {
            if (!is_space_char(ch)) {
                only_spaces = false;
                break;
            }
        }
        if (only_spaces) {
            continue;
        }

        ++input_record_number;

        const ParsedLine parsed = parse_stoichiometry_line(original_line);
        if (!parsed.ok) {
            ++parse_warnings;
            // Do not flood the console if a huge file contains many bad lines.
            if (parse_warnings <= 20) {
                cerr << "Warning: input record " << input_record_number
                     << " skipped: " << parsed.error << '\n';
            } else if (parse_warnings == 21) {
                cerr << "Warning: further parse warnings are suppressed.\n";
            }
            continue;
        }

        ++valid_record_number;
        const array<int,8> counts = make_counts(parsed.components);

        // Summing all 108 octahedral equations gives
        // sum(cation charges) = 108 * |z_A|.
        if (total_cation_charge(counts) != N_SITES * anion_charge) {
            ++rejected_global_charge;
        } else {
            ++passed_global_charge;

            // Exact CF108 geometry forces 27 equal-charge quartets.
            if (!all_counts_divisible_by_four(counts)) {
                ++rejected_not_multiple4;
            } else {
                ++passed_multiple4;

                const uint64_t histogram_key = make_histogram_key(counts);
                bool found = false;

                const auto cached = cache.find(histogram_key);
                if (cached != cache.end()) {
                    ++cache_hits;
                    found = cached->second;
                } else {
                    ExactCF108Solver solver(counts, anion_charge);
                    found = solver.solve();
                    ++solver_calls;
                    total_search_nodes += solver.visited_nodes();

                    if (cache.size() < MAX_CACHE_ENTRIES) {
                        cache.emplace(histogram_key, found);
                    } else {
                        ++cache_insertions_skipped;
                    }
                }

                if (found) {
                    ++found_count;

                    // Write and flush BEFORE reporting the hit on the console.
                    // Therefore, once "Found variant..." is visible, this line
                    // has already been handed from stdio to the operating system.
                    fputs(original_line.c_str(), output);
                    if (!original_line.empty() &&
                        original_line.back() != '\n') {
                        fputc('\n', output);
                    }
                    fflush(output);

                    cout << "Found variant for exact CF108 local valence balance "
                         << valid_record_number << '\n';
                    cout.flush();
                }
            }
        }

        if ((input_record_number % PROGRESS_INTERVAL) == 0) {
            const auto now_tp = chrono::steady_clock::now();
            const double elapsed =
                chrono::duration<double>(now_tp - start).count();

            cout << "[progress] processed=" << input_record_number
                 << " valid=" << valid_record_number
                 << " global_ok=" << passed_global_charge
                 << " multiple4_ok=" << passed_multiple4
                 << " solver_calls=" << solver_calls
                 << " cache_hits=" << cache_hits
                 << " found=" << found_count
                 << " cache_size=" << cache.size()
                 << " elapsed=" << elapsed << " s\n";
            cout.flush();
            fflush(output);
        }
    }

    fclose(input);
    fflush(output);
    fclose(output);

    const auto finish = chrono::steady_clock::now();
    const double seconds = chrono::duration<double>(finish - start).count();

    cout << "==============================================\n";
    cout << "Time: " << seconds << " seconds\n";
    cout << "Input records processed: " << input_record_number << '\n';
    cout << "Valid records: " << valid_record_number << '\n';
    cout << "Total found: " << found_count << '\n';
    cout << "Rejected by global-charge test: " << rejected_global_charge << '\n';
    cout << "Passed global-charge test: " << passed_global_charge << '\n';
    cout << "Rejected because a count is not divisible by 4: "
         << rejected_not_multiple4 << '\n';
    cout << "Passed divisible-by-4 test: " << passed_multiple4 << '\n';
    cout << "Solver calls: " << solver_calls << '\n';
    cout << "Cache hits: " << cache_hits << '\n';
    cout << "Cached histograms: " << cache.size() << '\n';
    cout << "Cache insertions skipped after cap: "
         << cache_insertions_skipped << '\n';
    cout << "Search nodes visited: " << total_search_nodes << '\n';
    cout << "Parse warnings: " << parse_warnings << '\n';
    cout << "Output: " << output_filename << '\n';
    cout << "==============================================\n";

    return 0;
}
