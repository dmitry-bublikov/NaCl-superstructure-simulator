#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Parent_cells_CF108.cpp
// MODULE 0: Parent Cells
// Builds the maximal parent-cell catalogue used by the subsequent stoichiometry modules.
//
// Enumerates translation superlattices of the fcc cation sublattice subject to BOTH:
//   (1) index N <= nmax (default 108 cation sites), and
//   (2) at least one basis parallelepiped can be translated wholly inside an
//       axis-aligned B x B x B parent-cubic box (default B=3 = CF108 extent).
//
// IMPORTANT: this is a GEOMETRIC-envelope catalogue. It is broader than the set
// of periodic subcells of the fixed cubic CF108 translation lattice.
//
// Parent fcc primitive basis (in conventional cubic coordinates, units of a0):
//   f1 = (0, 1/2, 1/2)
//   f2 = (1/2, 0, 1/2)
//   f3 = (1/2, 1/2, 0)
//
// Internally Cartesian vectors are stored in doubled coordinates (U,V,W)=2*(x,y,z),
// so all arithmetic is integer. A supercell basis fits in a BxBxB conventional-cubic
// box iff, after translation of the parallelepiped,
//   sum_i |U_i| <= 2B, sum_i |V_i| <= 2B, sum_i |W_i| <= 2B.
//
// Each translation lattice is canonicalized by row Hermite normal form (HNF) in
// primitive-fcc integer coordinates. Lattices related by the 48 signed-permutation
// symmetries of the cubic parent are identified as equivalent.
//
// C++17, no external dependencies.

using i64 = long long;
using Vec3 = std::array<i64, 3>;
using Mat3 = std::array<std::array<i64, 3>, 3>;

struct MatKey {
    std::array<i64, 9> a{};
    bool operator==(const MatKey& o) const { return a == o.a; }
    bool operator<(const MatKey& o) const { return a < o.a; }
};

struct MatKeyHash {
    std::size_t operator()(const MatKey& k) const noexcept {
        std::size_t h = 1469598103934665603ull;
        for (i64 x : k.a) {
            std::uint64_t v = static_cast<std::uint64_t>(x) ^ (static_cast<std::uint64_t>(x) >> 33);
            h ^= static_cast<std::size_t>(v + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2));
            h *= 1099511628211ull;
        }
        return h;
    }
};

struct CandidateVec {
    Vec3 cart2{}; // doubled conventional cubic coordinates
    Vec3 prim{};  // integer coordinates in primitive fcc basis
};

struct BasisRecord {
    std::array<Vec3, 3> prim{};
    std::array<Vec3, 3> cart2{};
    int span2[3]{}; // doubled bounding-box spans
    i64 sqsum2 = 0; // sum of squared doubled Cartesian lengths
};

struct CellClass {
    Mat3 representativeHnf{};
    MatKey orbitKey{};
    BasisRecord basis{};
    i64 index = 0;
};

static i64 det3(const Mat3& A) {
    return A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1])
         - A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0])
         + A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);
}

static Mat3 matMul(const Mat3& A, const Mat3& B) {
    Mat3 C{};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

static MatKey keyOf(const Mat3& A) {
    MatKey k;
    int p = 0;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            k.a[p++] = A[i][j];
    return k;
}

static Mat3 matOf(const MatKey& k) {
    Mat3 A{};
    int p = 0;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            A[i][j] = k.a[p++];
    return A;
}

struct EG { i64 g, x, y; };

static EG egcdPos(i64 a, i64 b) {
    // a,b >= 0
    if (b == 0) return {a, 1, 0};
    EG t = egcdPos(b, a % b);
    return {t.g, t.y, t.x - (a / b) * t.y};
}

static EG egcdSigned(i64 a, i64 b) {
    i64 aa = std::llabs(a), bb = std::llabs(b);
    EG e = egcdPos(aa, bb);
    if (a < 0) e.x = -e.x;
    if (b < 0) e.y = -e.y;
    return e;
}

static i64 floorDiv(i64 a, i64 p) {
    // p > 0
    i64 q = a / p;
    i64 r = a % p;
    if (r < 0) --q;
    return q;
}

static Mat3 rowHnf(Mat3 A) {
    // Full-rank 3x3 integer matrix. Only unimodular row operations are used.
    // Canonical upper-triangular row HNF:
    // positive diagonal, and 0 <= A[i][j] < A[j][j] for i<j.
    for (int k = 0; k < 3; ++k) {
        int pivot = -1;
        for (int r = k; r < 3; ++r) {
            if (A[r][k] != 0) { pivot = r; break; }
        }
        if (pivot < 0) {
            throw std::runtime_error("rowHnf: rank-deficient matrix");
        }
        if (pivot != k) std::swap(A[pivot], A[k]);

        for (int r = k + 1; r < 3; ++r) {
            if (A[r][k] == 0) continue;
            i64 a = A[k][k];
            i64 b = A[r][k];
            EG e = egcdSigned(a, b);
            if (e.g == 0) continue;

            auto rowK = A[k];
            auto rowR = A[r];
            for (int c = 0; c < 3; ++c) {
                A[k][c] = e.x * rowK[c] + e.y * rowR[c];
                A[r][c] = (-b / e.g) * rowK[c] + (a / e.g) * rowR[c];
            }
        }

        if (A[k][k] < 0) {
            for (int c = 0; c < 3; ++c) A[k][c] = -A[k][c];
        }
        i64 p = A[k][k];
        if (p <= 0) throw std::runtime_error("rowHnf: non-positive pivot");

        for (int r = 0; r < k; ++r) {
            i64 q = floorDiv(A[r][k], p);
            for (int c = 0; c < 3; ++c) A[r][c] -= q * A[k][c];
            // A[r][k] is now in [0,p)
        }
    }
    return A;
}

static Vec3 cart2ToPrim(const Vec3& d) {
    // d=(U,V,W)=2*(x,y,z), with U+V+W even for an fcc vector.
    // U=n2+n3, V=n1+n3, W=n1+n2.
    return {
        (d[1] + d[2] - d[0]) / 2,
        (d[0] + d[2] - d[1]) / 2,
        (d[0] + d[1] - d[2]) / 2
    };
}

static bool canonicalVectorSign(const Vec3& d) {
    for (int k = 0; k < 3; ++k) {
        if (d[k] > 0) return true;
        if (d[k] < 0) return false;
    }
    return false; // zero vector
}

static bool vecLess(const CandidateVec& a, const CandidateVec& b) {
    i64 la = a.cart2[0]*a.cart2[0] + a.cart2[1]*a.cart2[1] + a.cart2[2]*a.cart2[2];
    i64 lb = b.cart2[0]*b.cart2[0] + b.cart2[1]*b.cart2[1] + b.cart2[2]*b.cart2[2];
    if (la != lb) return la < lb;
    return a.cart2 < b.cart2;
}

static std::vector<CandidateVec> makeCandidates(int boxCells) {
    int lim = 2 * boxCells;
    std::vector<CandidateVec> v;
    for (int U = -lim; U <= lim; ++U) {
        for (int V = -lim; V <= lim; ++V) {
            for (int W = -lim; W <= lim; ++W) {
                if (U == 0 && V == 0 && W == 0) continue;
                if (((U + V + W) & 1) != 0) continue; // not an fcc lattice vector
                Vec3 d{U,V,W};
                if (!canonicalVectorSign(d)) continue; // identify v and -v
                Vec3 n = cart2ToPrim(d);
                v.push_back({d,n});
            }
        }
    }
    std::sort(v.begin(), v.end(), vecLess);
    return v;
}

static std::vector<Mat3> makeCubicSymmetryActionsPrimitive() {
    // Return 48 integer matrices P such that n' = n P in primitive coordinates.
    const int perms[6][3] = {
        {0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}
    };

    std::vector<Mat3> actions;
    actions.reserve(48);

    // Primitive basis rows in doubled Cartesian coords.
    const Vec3 f[3] = {{0,1,1},{1,0,1},{1,1,0}};

    for (const auto& perm : perms) {
        for (int sx : {-1,1}) for (int sy : {-1,1}) for (int sz : {-1,1}) {
            int s[3] = {sx,sy,sz};
            Mat3 P{};
            for (int i = 0; i < 3; ++i) {
                Vec3 d{};
                // Row-vector Cartesian action corresponding to a signed permutation:
                // d'_j = s_j * d_{perm[j]}.
                for (int j = 0; j < 3; ++j) d[j] = s[j] * f[i][perm[j]];
                Vec3 np = cart2ToPrim(d);
                P[i] = np;
            }
            if (std::llabs(det3(P)) != 1) {
                throw std::runtime_error("Internal error: cubic action is not unimodular");
            }
            actions.push_back(P);
        }
    }

    // Deduplicate defensively.
    std::vector<Mat3> uniq;
    std::unordered_set<MatKey, MatKeyHash> seen;
    for (const auto& P : actions) {
        MatKey k = keyOf(P);
        if (seen.insert(k).second) uniq.push_back(P);
    }
    if (uniq.size() != 48) throw std::runtime_error("Expected 48 cubic symmetry actions");
    return uniq;
}

static MatKey orbitCanonicalKey(const Mat3& H, const std::vector<Mat3>& actions) {
    bool first = true;
    MatKey best{};
    for (const Mat3& P : actions) {
        Mat3 T = matMul(H, P); // row lattice transformed by parent symmetry
        Mat3 K = rowHnf(T);
        MatKey key = keyOf(K);
        if (first || key < best) { best = key; first = false; }
    }
    return best;
}

// H is a canonical upper-triangular row HNF. Test whether row vector v
// belongs to the translation lattice generated by the rows of H.
static bool containsVector(const Mat3& H, const Vec3& v) {
    const i64 a = H[0][0], b = H[0][1], c = H[0][2];
    const i64 d = H[1][1], e = H[1][2], f = H[2][2];
    if (a <= 0 || d <= 0 || f <= 0) return false;

    if (v[0] % a != 0) return false;
    const i64 x0 = v[0] / a;
    const i64 r1 = v[1] - x0 * b;
    if (r1 % d != 0) return false;
    const i64 x1 = r1 / d;
    const i64 r2 = v[2] - x0 * c - x1 * e;
    return (r2 % f) == 0;
}

// Return true when class B is a proper translation supercell/container of class A
// for at least one relative cubic-parent orientation P:
//
//     L_B P subseteq L_A
//
// This is equivalent to (B P) A^{-1} being an integer matrix.
static bool covers(const CellClass& A, const CellClass& B,
                   const std::vector<Mat3>& actions) {
    if (B.index <= A.index) return false;
    if (B.index % A.index != 0) return false;

    for (const Mat3& P : actions) {
        const Mat3 BP = matMul(B.representativeHnf, P);
        bool ok = true;
        for (const Vec3& row : BP) {
            if (!containsVector(A.representativeHnf, row)) {
                ok = false;
                break;
            }
        }
        if (ok) return true;
    }
    return false;
}

static BasisRecord makeBasisRecord(const CandidateVec& a, const CandidateVec& b, const CandidateVec& c) {
    BasisRecord r;
    r.prim = {a.prim,b.prim,c.prim};
    r.cart2 = {a.cart2,b.cart2,c.cart2};
    for (int k = 0; k < 3; ++k) {
        r.span2[k] = static_cast<int>(std::llabs(a.cart2[k]) + std::llabs(b.cart2[k]) + std::llabs(c.cart2[k]));
    }
    for (const auto& d : r.cart2)
        r.sqsum2 += d[0]*d[0] + d[1]*d[1] + d[2]*d[2];
    return r;
}

static std::array<i64, 8> basisScore(const BasisRecord& r) {
    int maxSpan = std::max({r.span2[0], r.span2[1], r.span2[2]});
    int sumSpan = r.span2[0] + r.span2[1] + r.span2[2];
    // Deterministic tie breakers from absolute Cartesian data.
    std::array<i64,8> s{};
    s[0] = maxSpan;
    s[1] = sumSpan;
    s[2] = r.sqsum2;
    s[3] = r.span2[0]; s[4] = r.span2[1]; s[5] = r.span2[2];
    s[6] = std::llabs(r.cart2[0][0]) + std::llabs(r.cart2[1][1]) + std::llabs(r.cart2[2][2]);
    s[7] = 0;
    return s;
}

static bool betterBasis(const BasisRecord& a, const BasisRecord& b) {
    return basisScore(a) < basisScore(b);
}

static std::string fmtHalf(i64 x2) {
    std::ostringstream os;
    if (x2 % 2 == 0) os << (x2 / 2);
    else os << std::fixed << std::setprecision(1) << (static_cast<double>(x2) / 2.0);
    return os.str();
}

static double lengthCart(const Vec3& d2) {
    return 0.5 * std::sqrt(static_cast<double>(d2[0]*d2[0] + d2[1]*d2[1] + d2[2]*d2[2]));
}

static double angleDeg(const Vec3& a2, const Vec3& b2) {
    double da = std::sqrt(static_cast<double>(a2[0]*a2[0] + a2[1]*a2[1] + a2[2]*a2[2]));
    double db = std::sqrt(static_cast<double>(b2[0]*b2[0] + b2[1]*b2[1] + b2[2]*b2[2]));
    double dot = static_cast<double>(a2[0]*b2[0] + a2[1]*b2[1] + a2[2]*b2[2]);
    double c = dot / (da * db);
    c = std::max(-1.0, std::min(1.0, c));
    return std::acos(c) * 180.0 / 3.14159265358979323846;
}

static void printMat(std::ostream& os, const Mat3& A) {
    os << "[";
    for (int i = 0; i < 3; ++i) {
        if (i) os << "; ";
        os << A[i][0] << " " << A[i][1] << " " << A[i][2];
    }
    os << "]";
}

static void printBasis(std::ostream& os, const BasisRecord& b) {
    os << "  primitive_fcc_basis_rows: [";
    for (int i = 0; i < 3; ++i) {
        if (i) os << "; ";
        os << b.prim[i][0] << " " << b.prim[i][1] << " " << b.prim[i][2];
    }
    os << "]\n";

    os << "  cartesian_basis_a0: [";
    for (int i = 0; i < 3; ++i) {
        if (i) os << "; ";
        os << fmtHalf(b.cart2[i][0]) << " " << fmtHalf(b.cart2[i][1]) << " " << fmtHalf(b.cart2[i][2]);
    }
    os << "]\n";

    os << "  bbox_span_a0: " << fmtHalf(b.span2[0]) << " " << fmtHalf(b.span2[1]) << " " << fmtHalf(b.span2[2]) << "\n";
    double la = lengthCart(b.cart2[0]), lb = lengthCart(b.cart2[1]), lc = lengthCart(b.cart2[2]);
    double alpha = angleDeg(b.cart2[1], b.cart2[2]);
    double beta  = angleDeg(b.cart2[0], b.cart2[2]);
    double gamma = angleDeg(b.cart2[0], b.cart2[1]);
    os << std::fixed << std::setprecision(6);
    os << "  metric_a0: a=" << la << " b=" << lb << " c=" << lc
       << " alpha=" << alpha << " beta=" << beta << " gamma=" << gamma << "\n";
}

static void usage(const char* argv0) {
    std::cout
        << "Usage: " << argv0 << " [--box B] [--nmax N] [--output FILE]\n"
        << "\n"
        << "  --box B     Maximum axis-aligned extent in conventional parent NaCl cells.\n"
        << "              Default B=3, i.e. the spatial boundary of CF108 (3x3x3).\n"
        << "  --nmax N    Keep envelope cells with at most N cation sites.\n"
        << "              Default N=4*B^3; for B=3 this is 108.\n"
        << "  --output    Output maximal-parent catalogue.\n"
        << "              Default: CF108_parents.txt\n"
        << "\n"
        << "The program writes ONE final catalogue. No separate summary file is produced;\n"
        << "the counts and N-distribution are included in the catalogue header.\n";
}

int main(int argc, char** argv) {
    try {
        int box = 3;
        int nmax = -1;
        std::string outputName;
        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];
            if (a == "--help" || a == "-h") { usage(argv[0]); return 0; }
            else if (a == "--box" && i + 1 < argc) box = std::stoi(argv[++i]);
            else if (a == "--nmax" && i + 1 < argc) nmax = std::stoi(argv[++i]);
            else if (a == "--output" && i + 1 < argc) outputName = argv[++i];
            else { std::cerr << "Unknown/incomplete argument: " << a << "\n"; usage(argv[0]); return 2; }
        }
        if (box <= 0) throw std::runtime_error("--box must be positive");
        if (nmax < 0) nmax = 4 * box * box * box;
        if (nmax <= 0) throw std::runtime_error("--nmax must be positive");
        if (outputName.empty()) {
            outputName = (box == 3 && nmax == 108)
                ? "CF108_parents.txt"
                : ("box" + std::to_string(box) + "_N" + std::to_string(nmax) + "_maximal_parents.txt");
        }

        const int lim2 = 2 * box;
        std::cout << "Parent: fcc cation sublattice of NaCl\n";
        std::cout << "Geometric boundary: " << box << " x " << box << " x " << box << " parent conventional cells\n";
        std::cout << "Maximum cation sites/index: " << nmax << "\n";

        auto cand = makeCandidates(box);
        auto sym = makeCubicSymmetryActionsPrimitive();
        std::cout << "Candidate basis vectors (v and -v identified): " << cand.size() << "\n";
        std::cout << "Parent cubic symmetry actions: " << sym.size() << "\n";

        std::unordered_map<MatKey, BasisRecord, MatKeyHash> raw;
        raw.reserve(200000);

        std::uint64_t triplesBBox = 0;
        std::uint64_t triplesIndependent = 0;
        std::uint64_t triplesIndependentBoxAllIndex = 0;
        std::uint64_t triplesRejectedByNmax = 0;
        i64 maxIndexInsideBox = 0;

        const std::size_t M = cand.size();
        for (std::size_t i = 0; i < M; ++i) {
            const auto& A = cand[i];
            for (std::size_t j = i + 1; j < M; ++j) {
                const auto& B = cand[j];
                int s01[3] = {
                    static_cast<int>(std::llabs(A.cart2[0]) + std::llabs(B.cart2[0])),
                    static_cast<int>(std::llabs(A.cart2[1]) + std::llabs(B.cart2[1])),
                    static_cast<int>(std::llabs(A.cart2[2]) + std::llabs(B.cart2[2]))
                };
                if (s01[0] > lim2 || s01[1] > lim2 || s01[2] > lim2) continue;

                for (std::size_t k = j + 1; k < M; ++k) {
                    const auto& C = cand[k];
                    if (s01[0] + std::llabs(C.cart2[0]) > lim2 ||
                        s01[1] + std::llabs(C.cart2[1]) > lim2 ||
                        s01[2] + std::llabs(C.cart2[2]) > lim2) continue;
                    ++triplesBBox;

                    Mat3 Bmat{{A.prim, B.prim, C.prim}};
                    i64 d = std::llabs(det3(Bmat));
                    if (d == 0) continue;
                    ++triplesIndependentBoxAllIndex;
                    if (d > maxIndexInsideBox) maxIndexInsideBox = d;
                    if (d > nmax) { ++triplesRejectedByNmax; continue; }
                    ++triplesIndependent;

                    Mat3 H = rowHnf(Bmat);
                    if (std::llabs(det3(H)) != d) throw std::runtime_error("HNF determinant mismatch");
                    MatKey hk = keyOf(H);
                    BasisRecord br = makeBasisRecord(A,B,C);
                    auto it = raw.find(hk);
                    if (it == raw.end()) raw.emplace(hk, br);
                    else if (betterBasis(br, it->second)) it->second = br;
                }
            }
        }

        std::cout << "Basis triples satisfying box bound: " << triplesBBox << "\n";
        std::cout << "Independent basis triples inside box (before N limit): " << triplesIndependentBoxAllIndex << "\n";
        std::cout << "Maximum index among independent box-compatible triples: " << maxIndexInsideBox << "\n";
        std::cout << "Independent triples rejected only by N > nmax: " << triplesRejectedByNmax << "\n";
        std::cout << "Independent basis triples satisfying BOTH filters: " << triplesIndependent << "\n";
        std::cout << "Distinct translation lattices before parent-symmetry reduction: " << raw.size() << "\n";

        // Sort raw entries for deterministic class representative selection.
        std::vector<std::pair<MatKey,BasisRecord>> rawVec;
        rawVec.reserve(raw.size());
        for (const auto& kv : raw) rawVec.push_back(kv);
        std::sort(rawVec.begin(), rawVec.end(), [](const auto& x, const auto& y){ return x.first < y.first; });

        std::unordered_map<MatKey, CellClass, MatKeyHash> classes;
        classes.reserve(raw.size());

        for (const auto& kv : rawVec) {
            Mat3 H = matOf(kv.first);
            i64 idx = std::llabs(det3(H));
            MatKey ok = orbitCanonicalKey(H, sym);
            auto it = classes.find(ok);
            if (it == classes.end()) {
                CellClass cc;
                cc.representativeHnf = H;
                cc.orbitKey = ok;
                cc.basis = kv.second;
                cc.index = idx;
                classes.emplace(ok, cc);
            } else if (betterBasis(kv.second, it->second.basis)) {
                it->second.representativeHnf = H;
                it->second.basis = kv.second;
            }
        }

        std::vector<CellClass> out;
        out.reserve(classes.size());
        for (const auto& kv : classes) out.push_back(kv.second);
        std::sort(out.begin(), out.end(), [](const CellClass& a, const CellClass& b){
            if (a.index != b.index) return a.index < b.index;
            MatKey ka = keyOf(a.representativeHnf), kb = keyOf(b.representativeHnf);
            return ka < kb;
        });

        std::cout << "Symmetry-inequivalent translation lattices: " << out.size() << "\n";

        // Determine maximal parent/container classes among the symmetry-inequivalent
        // geometric-envelope catalogue.
        std::vector<bool> isCovered(out.size(), false);
        std::vector<int> coveredBy(out.size(), -1);
        for (std::size_t i = 0; i < out.size(); ++i) {
            for (std::size_t j = 0; j < out.size(); ++j) {
                if (i == j) continue;
                if (covers(out[i], out[j], sym)) {
                    isCovered[i] = true;
                    coveredBy[i] = static_cast<int>(j);
                    break;
                }
            }
        }

        std::vector<std::size_t> maximalIds;
        for (std::size_t i = 0; i < out.size(); ++i)
            if (!isCovered[i]) maximalIds.push_back(i);

        std::vector<std::uint64_t> maximalCountByN(static_cast<std::size_t>(nmax) + 1, 0);
        for (std::size_t i : maximalIds) {
            const i64 N = out[i].index;
            if (N >= 1 && N <= nmax) ++maximalCountByN[static_cast<std::size_t>(N)];
        }

        std::ofstream os(outputName);
        if (!os) throw std::runtime_error("Cannot open " + outputName);
        os << "# MODULE 0 - CF108 maximal parent cells\n";
        os << "# ================================================\n";
        os << "#\n";
        os << "# Selection rule:\n";
        os << "#   1) translation lattice has at least one basis parallelepiped fitting\n";
        os << "#      inside the " << box << "x" << box << "x" << box << " parent-cubic geometric envelope;\n";
        os << "#   2) index N <= " << nmax << ";\n";
        os << "#   3) classes related by the 48 cubic-parent signed-permutation symmetries\n";
        os << "#      are identified;\n";
        os << "#   4) a class A is removed when another class B can contain every A-periodic\n";
        os << "#      ordering for some relative cubic symmetry P: L_B P subseteq L_A.\n";
        os << "#\n";
        os << "# This is the maximal-parent catalogue used by the stoichiometry workflow.\n";
        os << "# It is NOT the strict-periodic-subcell catalogue of the fixed cubic CF108 cell.\n";
        os << "#\n";
        os << "# Starting geometric-envelope classes = " << out.size() << "\n";
        os << "# Maximal parent/container classes = " << maximalIds.size() << "\n";
        os << "# Covered/redundant classes = " << (out.size() - maximalIds.size()) << "\n";
        os << "#\n";
        os << "# Distribution of maximal classes by cation-site index N:\n";
        for (int N = 1; N <= nmax; ++N) {
            if (maximalCountByN[static_cast<std::size_t>(N)] != 0)
                os << "#   N=" << std::setw(2) << N << " : "
                   << maximalCountByN[static_cast<std::size_t>(N)] << "\n";
        }
        os << "#\n";
        os << "# original_catalogue_cell is the CELL number in the full symmetry-inequivalent\n";
        os << "# geometric-envelope catalogue before maximal-containment reduction.\n\n";

        std::size_t mid = 0;
        for (std::size_t idx : maximalIds) {
            const CellClass& c = out[idx];
            os << "# MAXIMAL_PARENT " << std::setw(2) << std::setfill('0') << (++mid)
               << " / " << maximalIds.size() << std::setfill(' ') << "\n";
            os << "# original_catalogue_cell = " << (idx + 1) << "\n";
            os << "CELL " << (idx + 1) << "\n";
            os << "  index_cation_sites: " << c.index << "\n";
            os << "  representative_row_HNF: "; printMat(os, c.representativeHnf); os << "\n";
            os << "  cubic_orbit_key_HNF: "; printMat(os, matOf(c.orbitKey)); os << "\n";
            printBasis(os, c.basis);
            os << "\n";
        }

        std::cout << "Maximal parent cells: " << maximalIds.size() << "\n";
        std::cout << "Covered/redundant classes: " << (out.size() - maximalIds.size()) << "\n";
        std::cout << "Wrote: " << outputName << "\n";
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}
