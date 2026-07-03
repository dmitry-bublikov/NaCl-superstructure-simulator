//===========================================================================
// PROGRAM: Standard CF32 Cell (Standard Crystallographic Cell Builder)
// Developed by Dmitry Bublikov, 2026
//===========================================================================
//
// Purpose:
//   Converts a reduced primitive chemical cell into a more conventional
//   crystallographic cell when such a representation is possible.
//
// Typical workflow:
//
//   phase_6.cif
//       |
//       v
//   Reduce_cell_CF32.exe phase_6.cif
//       |
//       v
//   Reduced_phase_6.cif
//       |
//       v
//   Standard_cell_CF32.exe Reduced_phase_6.cif
//       |
//       v
//   Standard_phase_6.cif
//
// Algorithm:
//   Read reduced CIF
//       |
//       v
//   Build primitive cell vectors
//       |
//       v
//   Enumerate small integer supercell transformations
//       |
//       v
//   Evaluate metric symmetry of each candidate cell
//       |
//       v
//   Select the best conventional-like crystallographic cell
//       |
//       v
//   Expand atoms into the selected standard cell
//       |
//       v
//   Write standard CIF
//
// Notes:
//   - This program does not search CF32 cation translations.
//   - That task belongs to Reduce_cell_CF32.
//   - This module works after reduction, using the reduced primitive CIF.
//   - It does not determine a full space group.
//   - It only searches for a more conventional crystallographic cell metric.
//
//===========================================================================

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <cstdlib>

//===========================================================================
// CONSTANTS
//===========================================================================
const double PI_VALUE = 3.14159265358979323846;
const double EPS_VALUE = 1e-7;

//===========================================================================
// STRUCT: Vec3
//===========================================================================
struct Vec3 {
    double x;
    double y;
    double z;
};

//===========================================================================
// STRUCT: Matrix3d
//===========================================================================
struct Matrix3d {
    double m[3][3];
};

//===========================================================================
// STRUCT: Matrix3i
//===========================================================================
struct Matrix3i {
    int m[3][3];
};

//===========================================================================
// STRUCT: AtomType
//===========================================================================
struct AtomType {
    std::string symbol;
    std::string oxidation;
};

//===========================================================================
// STRUCT: AtomSite
//===========================================================================
struct AtomSite {
    std::string label;
    std::string symbol;
    double fx;
    double fy;
    double fz;
};

//===========================================================================
// STRUCT: CellData
//===========================================================================
struct CellData {
    double a;
    double b;
    double c;
    double alpha;
    double beta;
    double gamma;
    std::string formula;
    double volume;
    std::vector<AtomType> atom_types;
    std::vector<AtomSite> atoms;
};

//===========================================================================
// STRUCT: CellMetric
//===========================================================================
struct CellMetric {
    double a;
    double b;
    double c;
    double alpha;
    double beta;
    double gamma;
    double volume;
    std::string crystal_system;
    int system_rank;
    double penalty;
};

//===========================================================================
// STRUCT: CandidateCell
//===========================================================================
struct CandidateCell {
    Matrix3i transform;
    Matrix3d vectors;
    CellMetric metric;
    int determinant;
    double score;
};

//===========================================================================
// FUNCTION: trim
//===========================================================================
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

//===========================================================================
// FUNCTION: starts_with
//===========================================================================
bool starts_with(const std::string& s, const std::string& prefix) {
    if (s.size() < prefix.size()) return false;
    return s.compare(0, prefix.size(), prefix) == 0;
}

//===========================================================================
// FUNCTION: remove_quotes
//===========================================================================
std::string remove_quotes(const std::string& s) {
    if (s.size() >= 2) {
        if ((s[0] == '\'' && s[s.size() - 1] == '\'') ||
            (s[0] == '"'  && s[s.size() - 1] == '"')) {
            return s.substr(1, s.size() - 2);
        }
    }
    return s;
}

//===========================================================================
// FUNCTION: split_tokens
//===========================================================================
std::vector<std::string> split_tokens(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    bool in_quote = false;
    char quote_char = 0;

    for (size_t i = 0; i < line.size(); i++) {
        char ch = line[i];

        if ((ch == '\'' || ch == '"') && !in_quote) {
            in_quote = true;
            quote_char = ch;
            token += ch;
        } else if (ch == quote_char && in_quote) {
            in_quote = false;
            token += ch;
        } else if ((ch == ' ' || ch == '\t') && !in_quote) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += ch;
        }
    }

    if (!token.empty()) tokens.push_back(token);
    return tokens;
}

//===========================================================================
// FUNCTION: read_all_lines
//===========================================================================
std::vector<std::string> read_all_lines(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream file(filename.c_str());

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        return lines;
    }

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    return lines;
}

//===========================================================================
// FUNCTION: parse_number_after_key
//===========================================================================
double parse_number_after_key(const std::string& line) {
    std::vector<std::string> tokens = split_tokens(line);
    if (tokens.size() < 2) return 0.0;
    return atof(remove_quotes(tokens[1]).c_str());
}

//===========================================================================
// FUNCTION: parse_string_after_key
//===========================================================================
std::string parse_string_after_key(const std::string& line) {
    std::vector<std::string> tokens = split_tokens(line);
    if (tokens.size() < 2) return "";
    return remove_quotes(tokens[1]);
}

//===========================================================================
// FUNCTION: normalize_fraction
//===========================================================================
double normalize_fraction(double value) {
    double r = value - std::floor(value);
    if (r < 0.0) r += 1.0;
    if (r >= 1.0 - 1e-10) r = 0.0;
    return r;
}

//===========================================================================
// FUNCTION: almost_equal
//===========================================================================
bool almost_equal(double a, double b, double tolerance) {
    return std::fabs(a - b) <= tolerance;
}

//===========================================================================
// FUNCTION: dot_vec
//===========================================================================
double dot_vec(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

//===========================================================================
// FUNCTION: cross_vec
//===========================================================================
Vec3 cross_vec(const Vec3& a, const Vec3& b) {
    Vec3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

//===========================================================================
// FUNCTION: length_vec
//===========================================================================
double length_vec(const Vec3& a) {
    return std::sqrt(dot_vec(a, a));
}

//===========================================================================
// FUNCTION: matrix_column
//===========================================================================
Vec3 matrix_column(const Matrix3d& m, int col) {
    Vec3 v;
    v.x = m.m[0][col];
    v.y = m.m[1][col];
    v.z = m.m[2][col];
    return v;
}

//===========================================================================
// FUNCTION: set_matrix_column
//===========================================================================
void set_matrix_column(Matrix3d& m, int col, const Vec3& v) {
    m.m[0][col] = v.x;
    m.m[1][col] = v.y;
    m.m[2][col] = v.z;
}

//===========================================================================
// FUNCTION: angle_between
//===========================================================================
double angle_between(const Vec3& a, const Vec3& b) {
    double la = length_vec(a);
    double lb = length_vec(b);
    if (la <= 0.0 || lb <= 0.0) return 90.0;

    double c = dot_vec(a, b) / (la * lb);
    if (c > 1.0) c = 1.0;
    if (c < -1.0) c = -1.0;

    return std::acos(c) * 180.0 / PI_VALUE;
}

//===========================================================================
// FUNCTION: matrix_determinant_double
//===========================================================================
double matrix_determinant_double(const Matrix3d& m) {
    return m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1])
         - m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0])
         + m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
}

//===========================================================================
// FUNCTION: matrix_determinant_int
//===========================================================================
int matrix_determinant_int(const Matrix3i& m) {
    return m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1])
         - m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0])
         + m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
}

//===========================================================================
// FUNCTION: inverse_matrix3d
//===========================================================================
bool inverse_matrix3d(const Matrix3d& a, Matrix3d& inv) {
    double det = matrix_determinant_double(a);
    if (std::fabs(det) < 1e-12) return false;

    inv.m[0][0] =  (a.m[1][1] * a.m[2][2] - a.m[1][2] * a.m[2][1]) / det;
    inv.m[0][1] = -(a.m[0][1] * a.m[2][2] - a.m[0][2] * a.m[2][1]) / det;
    inv.m[0][2] =  (a.m[0][1] * a.m[1][2] - a.m[0][2] * a.m[1][1]) / det;

    inv.m[1][0] = -(a.m[1][0] * a.m[2][2] - a.m[1][2] * a.m[2][0]) / det;
    inv.m[1][1] =  (a.m[0][0] * a.m[2][2] - a.m[0][2] * a.m[2][0]) / det;
    inv.m[1][2] = -(a.m[0][0] * a.m[1][2] - a.m[0][2] * a.m[1][0]) / det;

    inv.m[2][0] =  (a.m[1][0] * a.m[2][1] - a.m[1][1] * a.m[2][0]) / det;
    inv.m[2][1] = -(a.m[0][0] * a.m[2][1] - a.m[0][1] * a.m[2][0]) / det;
    inv.m[2][2] =  (a.m[0][0] * a.m[1][1] - a.m[0][1] * a.m[1][0]) / det;

    return true;
}

//===========================================================================
// FUNCTION: multiply_matrix_vector
//===========================================================================
Vec3 multiply_matrix_vector(const Matrix3d& m, const Vec3& v) {
    Vec3 r;
    r.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z;
    r.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z;
    r.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z;
    return r;
}

//===========================================================================
// FUNCTION: original_cell_vectors
//===========================================================================
Matrix3d original_cell_vectors(const CellData& cell) {
    Matrix3d v;

    double alpha = cell.alpha * PI_VALUE / 180.0;
    double beta  = cell.beta  * PI_VALUE / 180.0;
    double gamma = cell.gamma * PI_VALUE / 180.0;

    double cos_alpha = std::cos(alpha);
    double cos_beta  = std::cos(beta);
    double cos_gamma = std::cos(gamma);
    double sin_gamma = std::sin(gamma);

    v.m[0][0] = cell.a;
    v.m[1][0] = 0.0;
    v.m[2][0] = 0.0;

    v.m[0][1] = cell.b * cos_gamma;
    v.m[1][1] = cell.b * sin_gamma;
    v.m[2][1] = 0.0;

    v.m[0][2] = cell.c * cos_beta;
    v.m[1][2] = cell.c * (cos_alpha - cos_beta * cos_gamma) / sin_gamma;

    double z2 = cell.c * cell.c - v.m[0][2] * v.m[0][2] - v.m[1][2] * v.m[1][2];
    if (z2 < 0.0 && z2 > -1e-8) z2 = 0.0;
    v.m[2][2] = std::sqrt(z2);

    return v;
}

//===========================================================================
// FUNCTION: multiply_cell_transform
//===========================================================================
Matrix3d multiply_cell_transform(const Matrix3d& primitive, const Matrix3i& transform) {
    Matrix3d result;

    for (int col = 0; col < 3; col++) {
        for (int row = 0; row < 3; row++) {
            result.m[row][col] = 0.0;
            for (int k = 0; k < 3; k++) {
                result.m[row][col] += primitive.m[row][k] * (double)transform.m[k][col];
            }
        }
    }

    return result;
}

//===========================================================================
// FUNCTION: calculate_cell_metric
//===========================================================================
CellMetric calculate_cell_metric(const Matrix3d& vectors) {
    CellMetric metric;

    Vec3 va = matrix_column(vectors, 0);
    Vec3 vb = matrix_column(vectors, 1);
    Vec3 vc = matrix_column(vectors, 2);

    metric.a = length_vec(va);
    metric.b = length_vec(vb);
    metric.c = length_vec(vc);

    metric.alpha = angle_between(vb, vc);
    metric.beta  = angle_between(va, vc);
    metric.gamma = angle_between(va, vb);

    metric.volume = std::fabs(matrix_determinant_double(vectors));

    double len_tol = 1e-4;
    double ang_tol = 1e-3;

    bool ab_eq = almost_equal(metric.a, metric.b, len_tol);
    bool ac_eq = almost_equal(metric.a, metric.c, len_tol);
    bool bc_eq = almost_equal(metric.b, metric.c, len_tol);

    bool alpha90 = almost_equal(metric.alpha, 90.0, ang_tol);
    bool beta90  = almost_equal(metric.beta,  90.0, ang_tol);
    bool gamma90 = almost_equal(metric.gamma, 90.0, ang_tol);

    bool alpha60  = almost_equal(metric.alpha, 60.0,  ang_tol);
    bool beta60   = almost_equal(metric.beta,  60.0,  ang_tol);
    bool gamma60  = almost_equal(metric.gamma, 60.0,  ang_tol);
    bool alpha120 = almost_equal(metric.alpha, 120.0, ang_tol);
    bool beta120  = almost_equal(metric.beta,  120.0, ang_tol);
    bool gamma120 = almost_equal(metric.gamma, 120.0, ang_tol);

    metric.crystal_system = "triclinic";
    metric.system_rank = 1;

    if (alpha90 && beta90 && gamma90 && ab_eq && ac_eq) {
        metric.crystal_system = "cubic";
        metric.system_rank = 7;
    } else if (alpha90 && beta90 && gamma90 && (ab_eq || ac_eq || bc_eq)) {
        metric.crystal_system = "tetragonal";
        metric.system_rank = 6;
    } else if (alpha90 && beta90 && gamma90) {
        metric.crystal_system = "orthorhombic";
        metric.system_rank = 5;
    } else if (ab_eq && ac_eq && bc_eq &&
               almost_equal(metric.alpha, metric.beta, ang_tol) &&
               almost_equal(metric.beta, metric.gamma, ang_tol)) {
        metric.crystal_system = "rhombohedral";
        metric.system_rank = 4;
    } else if (alpha90 && beta90 && (gamma60 || gamma120) && ab_eq) {
        metric.crystal_system = "hexagonal";
        metric.system_rank = 4;
    } else if ((alpha90 && beta90) || (alpha90 && gamma90) || (beta90 && gamma90)) {
        metric.crystal_system = "monoclinic";
        metric.system_rank = 2;
    }

    double angle_penalty = std::fabs(metric.alpha - 90.0) +
                           std::fabs(metric.beta  - 90.0) +
                           std::fabs(metric.gamma - 90.0);

    if (metric.crystal_system == "hexagonal") {
        angle_penalty = std::fabs(metric.alpha - 90.0) +
                        std::fabs(metric.beta  - 90.0) +
                        std::min(std::fabs(metric.gamma - 60.0),
                                 std::fabs(metric.gamma - 120.0));
    }

    double length_penalty = 0.0;
    if (metric.crystal_system == "cubic" || metric.crystal_system == "rhombohedral") {
        length_penalty = std::fabs(metric.a - metric.b) +
                         std::fabs(metric.a - metric.c) +
                         std::fabs(metric.b - metric.c);
    } else if (metric.crystal_system == "tetragonal" ||
               metric.crystal_system == "hexagonal") {
        double p1 = std::fabs(metric.a - metric.b);
        double p2 = std::fabs(metric.a - metric.c);
        double p3 = std::fabs(metric.b - metric.c);
        length_penalty = std::min(p1, std::min(p2, p3));
    }

    metric.penalty = angle_penalty * 100.0 + length_penalty * 10.0;

    return metric;
}

//===========================================================================
// FUNCTION: load_cif
//===========================================================================
CellData load_cif(const std::string& filename) {
    CellData cell;
    cell.a = 1.0;
    cell.b = 1.0;
    cell.c = 1.0;
    cell.alpha = 90.0;
    cell.beta = 90.0;
    cell.gamma = 90.0;
    cell.volume = 0.0;

    std::vector<std::string> lines = read_all_lines(filename);

    for (size_t i = 0; i < lines.size(); i++) {
        std::string line = trim(lines[i]);
        if (line.empty()) continue;

        if (starts_with(line, "_cell_length_a")) cell.a = parse_number_after_key(line);
        else if (starts_with(line, "_cell_length_b")) cell.b = parse_number_after_key(line);
        else if (starts_with(line, "_cell_length_c")) cell.c = parse_number_after_key(line);
        else if (starts_with(line, "_cell_angle_alpha")) cell.alpha = parse_number_after_key(line);
        else if (starts_with(line, "_cell_angle_beta")) cell.beta = parse_number_after_key(line);
        else if (starts_with(line, "_cell_angle_gamma")) cell.gamma = parse_number_after_key(line);
        else if (starts_with(line, "_cell_volume")) cell.volume = parse_number_after_key(line);
        else if (starts_with(line, "_chemical_formula_sum")) cell.formula = parse_string_after_key(line);
    }

    for (size_t i = 0; i < lines.size(); i++) {
        std::string line = trim(lines[i]);
        if (line != "loop_") continue;

        std::vector<std::string> headers;
        size_t j = i + 1;

        while (j < lines.size()) {
            std::string h = trim(lines[j]);
            if (h.empty()) { j++; continue; }
            if (!starts_with(h, "_")) break;
            headers.push_back(h);
            j++;
        }

        bool is_atom_type_loop = false;
        bool is_atom_site_loop = false;

        for (size_t h = 0; h < headers.size(); h++) {
            if (headers[h] == "_atom_type_symbol") is_atom_type_loop = true;
            if (headers[h] == "_atom_site_label") is_atom_site_loop = true;
        }

        if (is_atom_type_loop) {
            int idx_symbol = -1;
            int idx_ox = -1;

            for (size_t h = 0; h < headers.size(); h++) {
                if (headers[h] == "_atom_type_symbol") idx_symbol = (int)h;
                if (headers[h] == "_atom_type_oxidation_number") idx_ox = (int)h;
            }

            while (j < lines.size()) {
                std::string data = trim(lines[j]);
                if (data.empty()) { j++; continue; }
                if (data == "loop_" || starts_with(data, "_") || starts_with(data, "data_")) break;

                std::vector<std::string> tokens = split_tokens(data);
                if (idx_symbol >= 0 && (int)tokens.size() > idx_symbol) {
                    AtomType t;
                    t.symbol = remove_quotes(tokens[idx_symbol]);
                    t.oxidation = "?";
                    if (idx_ox >= 0 && (int)tokens.size() > idx_ox) {
                        t.oxidation = remove_quotes(tokens[idx_ox]);
                    }
                    cell.atom_types.push_back(t);
                }
                j++;
            }
        }

        if (is_atom_site_loop) {
            int idx_label = -1;
            int idx_symbol = -1;
            int idx_x = -1;
            int idx_y = -1;
            int idx_z = -1;

            for (size_t h = 0; h < headers.size(); h++) {
                if (headers[h] == "_atom_site_label") idx_label = (int)h;
                else if (headers[h] == "_atom_site_type_symbol") idx_symbol = (int)h;
                else if (headers[h] == "_atom_site_fract_x") idx_x = (int)h;
                else if (headers[h] == "_atom_site_fract_y") idx_y = (int)h;
                else if (headers[h] == "_atom_site_fract_z") idx_z = (int)h;
            }

            if (idx_label < 0 || idx_symbol < 0 || idx_x < 0 || idx_y < 0 || idx_z < 0) {
                continue;
            }

            while (j < lines.size()) {
                std::string data = trim(lines[j]);
                if (data.empty()) { j++; continue; }
                if (data == "loop_" || starts_with(data, "_") || starts_with(data, "data_")) break;

                std::vector<std::string> tokens = split_tokens(data);
                if ((int)tokens.size() > idx_z) {
                    AtomSite atom;
                    atom.label = remove_quotes(tokens[idx_label]);
                    atom.symbol = remove_quotes(tokens[idx_symbol]);
                    atom.fx = atof(remove_quotes(tokens[idx_x]).c_str());
                    atom.fy = atof(remove_quotes(tokens[idx_y]).c_str());
                    atom.fz = atof(remove_quotes(tokens[idx_z]).c_str());
                    cell.atoms.push_back(atom);
                }
                j++;
            }
        }
    }

    return cell;
}

//===========================================================================
// FUNCTION: build_transform
//===========================================================================
Matrix3i build_transform(int a1, int a2, int a3,
                         int b1, int b2, int b3,
                         int c1, int c2, int c3) {
    Matrix3i t;

    t.m[0][0] = a1; t.m[1][0] = a2; t.m[2][0] = a3;
    t.m[0][1] = b1; t.m[1][1] = b2; t.m[2][1] = b3;
    t.m[0][2] = c1; t.m[1][2] = c2; t.m[2][2] = c3;

    return t;
}

//===========================================================================
// FUNCTION: matrix_int_key
//===========================================================================
std::string matrix_int_key(const Matrix3i& m) {
    std::ostringstream out;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            out << m.m[i][j] << "_";
        }
    }
    return out.str();
}

//===========================================================================
// FUNCTION: calculate_score
//===========================================================================
double calculate_score(const CandidateCell& candidate) {
    double score = 0.0;

    // Main goal: prefer the highest Bravais-like metric symmetry.
    score += candidate.metric.system_rank * 1000000.0;

    // Prefer smaller conventional supercells if metric quality is comparable.
    score -= candidate.determinant * 10000.0;

    // Prefer cells closer to exact metric symmetry.
    score -= candidate.metric.penalty;

    // Prefer shorter vectors inside the same class.
    score -= (candidate.metric.a + candidate.metric.b + candidate.metric.c);

    return score;
}

//===========================================================================
// FUNCTION: enumerate_candidate_cells
//===========================================================================
std::vector<CandidateCell> enumerate_candidate_cells(const Matrix3d& primitive_vectors) {
    std::vector<CandidateCell> candidates;
    std::set<std::string> seen;

    // Coefficients from -2 to 2 are sufficient for CF32 reduced cells.
    // The determinant is limited to 1..4 to avoid returning unnecessarily
    // large supercells such as the original unreduced CF32 cube.
    for (int a1 = -2; a1 <= 2; a1++)
    for (int a2 = -2; a2 <= 2; a2++)
    for (int a3 = -2; a3 <= 2; a3++)
    for (int b1 = -2; b1 <= 2; b1++)
    for (int b2 = -2; b2 <= 2; b2++)
    for (int b3 = -2; b3 <= 2; b3++)
    for (int c1 = -2; c1 <= 2; c1++)
    for (int c2 = -2; c2 <= 2; c2++)
    for (int c3 = -2; c3 <= 2; c3++) {
        Matrix3i t = build_transform(a1, a2, a3, b1, b2, b3, c1, c2, c3);
        int det = matrix_determinant_int(t);

        if (det <= 0) continue;
        if (det > 4) continue;

        std::string key = matrix_int_key(t);
        if (seen.find(key) != seen.end()) continue;
        seen.insert(key);

        Matrix3d vectors = multiply_cell_transform(primitive_vectors, t);
        CellMetric metric = calculate_cell_metric(vectors);

        CandidateCell candidate;
        candidate.transform = t;
        candidate.vectors = vectors;
        candidate.metric = metric;
        candidate.determinant = det;
        candidate.score = calculate_score(candidate);

        candidates.push_back(candidate);
    }

    std::sort(candidates.begin(), candidates.end(),
        [](const CandidateCell& a, const CandidateCell& b) {
            if (std::fabs(a.score - b.score) > 1e-9) return a.score > b.score;
            return a.determinant < b.determinant;
        });

    return candidates;
}

//===========================================================================
// FUNCTION: find_best_standard_cell
//===========================================================================
bool find_best_standard_cell(const Matrix3d& primitive_vectors, CandidateCell& best) {
    std::vector<CandidateCell> candidates = enumerate_candidate_cells(primitive_vectors);
    if (candidates.empty()) return false;

    // The best candidate according to metric symmetry scoring is selected.
    best = candidates[0];
    return true;
}

//===========================================================================
// FUNCTION: position_key
//===========================================================================
std::string position_key(const AtomSite& atom) {
    int ix = (int)std::floor(normalize_fraction(atom.fx) * 1000000.0 + 0.5);
    int iy = (int)std::floor(normalize_fraction(atom.fy) * 1000000.0 + 0.5);
    int iz = (int)std::floor(normalize_fraction(atom.fz) * 1000000.0 + 0.5);

    std::ostringstream out;
    out << atom.symbol << "_" << ix << "_" << iy << "_" << iz;
    return out.str();
}

//===========================================================================
// FUNCTION: expand_atoms_to_standard_cell
//===========================================================================
std::vector<AtomSite> expand_atoms_to_standard_cell(const CellData& cell,
                                                    const Matrix3i& transform) {
    std::vector<AtomSite> result;
    std::set<std::string> used;

    Matrix3d transform_double;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            transform_double.m[i][j] = (double)transform.m[i][j];
        }
    }

    Matrix3d inverse_transform;
    if (!inverse_matrix3d(transform_double, inverse_transform)) {
        return result;
    }

    int search_range = 4;

    for (size_t i = 0; i < cell.atoms.size(); i++) {
        for (int nx = -search_range; nx <= search_range; nx++) {
            for (int ny = -search_range; ny <= search_range; ny++) {
                for (int nz = -search_range; nz <= search_range; nz++) {
                    Vec3 old_fractional;
                    old_fractional.x = cell.atoms[i].fx + (double)nx;
                    old_fractional.y = cell.atoms[i].fy + (double)ny;
                    old_fractional.z = cell.atoms[i].fz + (double)nz;

                    Vec3 new_fractional = multiply_matrix_vector(inverse_transform, old_fractional);

                    if (new_fractional.x >= -1e-8 && new_fractional.x < 1.0 - 1e-8 &&
                        new_fractional.y >= -1e-8 && new_fractional.y < 1.0 - 1e-8 &&
                        new_fractional.z >= -1e-8 && new_fractional.z < 1.0 - 1e-8) {

                        AtomSite atom;
                        atom.symbol = cell.atoms[i].symbol;
                        atom.fx = normalize_fraction(new_fractional.x);
                        atom.fy = normalize_fraction(new_fractional.y);
                        atom.fz = normalize_fraction(new_fractional.z);
                        atom.label = atom.symbol;

                        std::string key = position_key(atom);
                        if (used.find(key) == used.end()) {
                            used.insert(key);
                            result.push_back(atom);
                        }
                    }
                }
            }
        }
    }

    std::sort(result.begin(), result.end(),
        [](const AtomSite& a, const AtomSite& b) {
            if (a.symbol != b.symbol) return a.symbol < b.symbol;
            if (std::fabs(a.fx - b.fx) > 1e-8) return a.fx < b.fx;
            if (std::fabs(a.fy - b.fy) > 1e-8) return a.fy < b.fy;
            return a.fz < b.fz;
        });

    return result;
}

//===========================================================================
// FUNCTION: crystal_system_to_setting
//===========================================================================
std::string crystal_system_to_setting(const std::string& system) {
    if (system == "cubic") return "cubic";
    if (system == "tetragonal") return "tetragonal";
    if (system == "orthorhombic") return "orthorhombic";
    if (system == "hexagonal") return "hexagonal";
    if (system == "rhombohedral") return "rhombohedral";
    if (system == "monoclinic") return "monoclinic";
    return "triclinic";
}

//===========================================================================
// FUNCTION: make_standard_filename
//===========================================================================
std::string make_standard_filename(const std::string& input_file) {
    size_t slash = input_file.find_last_of("\\/");
    size_t dot = input_file.find_last_of('.');

    std::string path;
    std::string name;

    if (slash == std::string::npos) {
        path = "";
        name = (dot == std::string::npos) ? input_file : input_file.substr(0, dot);
    } else {
        path = input_file.substr(0, slash + 1);
        if (dot == std::string::npos || dot < slash) {
            name = input_file.substr(slash + 1);
        } else {
            name = input_file.substr(slash + 1, dot - slash - 1);
        }
    }

    if (starts_with(name, "Reduced_")) {
        name = name.substr(8);
    }

    return path + "Standard_" + name + ".cif";
}

//===========================================================================
// FUNCTION: calculate_formula_from_atoms
//===========================================================================
std::string calculate_formula_from_atoms(const std::vector<AtomSite>& atoms) {
    std::map<std::string, int> counts;
    for (size_t i = 0; i < atoms.size(); i++) {
        counts[atoms[i].symbol]++;
    }

    std::vector<std::string> order;
    if (counts.find("Na") != counts.end()) order.push_back("Na");
    if (counts.find("Li") != counts.end()) order.push_back("Li");
    if (counts.find("Mg") != counts.end()) order.push_back("Mg");
    if (counts.find("Fe") != counts.end()) order.push_back("Fe");

    for (std::map<std::string, int>::iterator it = counts.begin(); it != counts.end(); ++it) {
        if (it->first == "Na" || it->first == "Li" ||
            it->first == "Mg" || it->first == "Fe" ||
            it->first == "O") continue;
        order.push_back(it->first);
    }

    if (counts.find("O") != counts.end()) order.push_back("O");

    std::string formula;
    for (size_t i = 0; i < order.size(); i++) {
        std::string sym = order[i];
        formula += sym;
        if (counts[sym] > 1) formula += std::to_string(counts[sym]);
    }

    return formula;
}

//===========================================================================
// FUNCTION: write_standard_cif
//===========================================================================
bool write_standard_cif(const std::string& filename,
                        const CellData& cell,
                        const CandidateCell& standard,
                        const std::vector<AtomSite>& atoms) {
    std::ofstream cif(filename.c_str());

    if (!cif.is_open()) {
        std::cerr << "Error: Cannot write " << filename << std::endl;
        return false;
    }

    std::string formula = calculate_formula_from_atoms(atoms);

    cif << std::fixed << std::setprecision(6);
    cif << "data_standard_cell\n";
    cif << "_audit_creation_method 'Standardized by Standard_cell_CF32.cpp'\n";
    cif << "_chemical_formula_sum\t'" << formula << "'\n";
    cif << "_cell_length_a\t" << standard.metric.a << "\n";
    cif << "_cell_length_b\t" << standard.metric.b << "\n";
    cif << "_cell_length_c\t" << standard.metric.c << "\n";
    cif << "_cell_angle_alpha\t" << standard.metric.alpha << "\n";
    cif << "_cell_angle_beta\t" << standard.metric.beta << "\n";
    cif << "_cell_angle_gamma\t" << standard.metric.gamma << "\n";
    cif << "_cell_volume\t" << standard.metric.volume << "\n";
    cif << "_symmetry_cell_setting\t" << crystal_system_to_setting(standard.metric.crystal_system) << "\n";
    cif << "_symmetry_int_tables_number\t1\n";
    cif << "_symmetry_space_group_name_H-M\t'P 1'\n";
    cif << "_symmetry_space_group_name_Hall\t'P_1'\n\n";

    cif << "loop_\n";
    cif << "_symmetry_equiv_pos_site_id\n";
    cif << "_symmetry_equiv_pos_as_xyz\n";
    cif << "1 x,y,z\n\n";

    std::map<std::string, bool> written_types;

    cif << "loop_\n";
    cif << "_atom_type_symbol\n";
    cif << "_atom_type_oxidation_number\n";
    cif << "_atom_type_radius_bond\n";

    for (size_t i = 0; i < cell.atom_types.size(); i++) {
        if (!written_types[cell.atom_types[i].symbol]) {
            cif << cell.atom_types[i].symbol << "\t" << cell.atom_types[i].oxidation << "\t?\n";
            written_types[cell.atom_types[i].symbol] = true;
        }
    }

    if (cell.atom_types.empty()) {
        for (size_t i = 0; i < atoms.size(); i++) {
            if (!written_types[atoms[i].symbol]) {
                cif << atoms[i].symbol << "\t?\t?\n";
                written_types[atoms[i].symbol] = true;
            }
        }
    }

    cif << "\n";

    cif << "loop_\n";
    cif << "_atom_site_label\n";
    cif << "_atom_site_type_symbol\n";
    cif << "_atom_site_fract_x\n";
    cif << "_atom_site_fract_y\n";
    cif << "_atom_site_fract_z\n";
    cif << "_atom_site_occupancy\n";
    cif << "_atom_site_symmetry_multiplicity\n";
    cif << "_atom_site_Wyckoff_symbol\n";
    cif << "_atom_site_attached_hydrogens\n";
    cif << "_atom_site_calc_flag\n";
    cif << "_atom_site_thermal_displace_type\n";
    cif << "_atom_site_u_iso_or_equiv\n";

    std::map<std::string, int> label_count;

    for (size_t i = 0; i < atoms.size(); i++) {
        label_count[atoms[i].symbol]++;
        std::string label = atoms[i].symbol + std::to_string(label_count[atoms[i].symbol]);

        cif << label << " "
            << atoms[i].symbol << "  "
            << atoms[i].fx << "  "
            << atoms[i].fy << "  "
            << atoms[i].fz << "  "
            << "1.000 1 a ? d ? ?\n";
    }

    cif.close();
    return true;
}

//===========================================================================
// FUNCTION: print_transform
//===========================================================================
void print_transform(const Matrix3i& t) {
    std::cout << "Transformation matrix (columns are new basis vectors):" << std::endl;
    for (int row = 0; row < 3; row++) {
        std::cout << "  "
                  << std::setw(3) << t.m[row][0] << " "
                  << std::setw(3) << t.m[row][1] << " "
                  << std::setw(3) << t.m[row][2] << std::endl;
    }
}


//===========================================================================
// FUNCTION: print_selection_reason
//===========================================================================
void print_selection_reason(const CandidateCell& standard) {
    std::cout << "Selection reason:" << std::endl;

    std::cout << "  - Candidate cells were generated by small integer transformations" << std::endl;
    std::cout << "    of the reduced primitive cell." << std::endl;

    std::cout << "  - The selected cell has the highest detected metric symmetry" << std::endl;
    std::cout << "    among the tested candidates: " << standard.metric.crystal_system << "." << std::endl;

    if (standard.metric.crystal_system == "cubic") {
        std::cout << "  - The metric is cubic because a = b = c and all angles" << std::endl;
        std::cout << "    are close to 90 degrees." << std::endl;
    } else if (standard.metric.crystal_system == "tetragonal") {
        std::cout << "  - The metric is tetragonal because two cell lengths are equal" << std::endl;
        std::cout << "    and all angles are close to 90 degrees." << std::endl;
    } else if (standard.metric.crystal_system == "orthorhombic") {
        std::cout << "  - The metric is orthorhombic because all angles are close" << std::endl;
        std::cout << "    to 90 degrees." << std::endl;
    } else if (standard.metric.crystal_system == "hexagonal") {
        std::cout << "  - The metric is hexagonal because two cell lengths are equal," << std::endl;
        std::cout << "    two angles are close to 90 degrees, and one angle is" << std::endl;
        std::cout << "    close to 60 or 120 degrees." << std::endl;
    } else if (standard.metric.crystal_system == "rhombohedral") {
        std::cout << "  - The metric is rhombohedral because all cell lengths are" << std::endl;
        std::cout << "    equal and all angles are equal." << std::endl;
    } else if (standard.metric.crystal_system == "monoclinic") {
        std::cout << "  - The metric is monoclinic because two angles are close" << std::endl;
        std::cout << "    to 90 degrees." << std::endl;
    } else {
        std::cout << "  - No higher metric symmetry was detected, so the best" << std::endl;
        std::cout << "    available representation remains triclinic." << std::endl;
    }

    std::cout << "  - The supercell index is " << standard.determinant << ", so the" << std::endl;
    std::cout << "    standard cell contains " << standard.determinant << " primitive cell(s)." << std::endl;

    std::cout << "  - Within the same metric class, the program prefers smaller" << std::endl;
    std::cout << "    supercells and shorter basis vectors." << std::endl;

    std::cout << "  - This is a metric standardization only; the program does not" << std::endl;
    std::cout << "    determine the full space group or Wyckoff positions." << std::endl;
}

//===========================================================================
// FUNCTION: print_report
//===========================================================================
void print_report(const CellData& cell,
                  const CandidateCell& standard,
                  const std::vector<AtomSite>& atoms,
                  const std::string& output_file) {
    std::cout << "//============================================================" << std::endl;
    std::cout << "// STANDARD CELL REPORT" << std::endl;
    std::cout << "//============================================================" << std::endl;
    std::cout << "Input atoms:       " << cell.atoms.size() << std::endl;
    std::cout << "Output atoms:      " << atoms.size() << std::endl;
    std::cout << "Supercell index:   " << standard.determinant << std::endl;
    std::cout << "Crystal system:    " << standard.metric.crystal_system << std::endl;
    std::cout << "Output file:       " << output_file << std::endl;
    std::cout << std::endl;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Cell parameters:" << std::endl;
    std::cout << "  a     = " << standard.metric.a << std::endl;
    std::cout << "  b     = " << standard.metric.b << std::endl;
    std::cout << "  c     = " << standard.metric.c << std::endl;
    std::cout << "  alpha = " << standard.metric.alpha << std::endl;
    std::cout << "  beta  = " << standard.metric.beta << std::endl;
    std::cout << "  gamma = " << standard.metric.gamma << std::endl;
    std::cout << "  V     = " << standard.metric.volume << std::endl;
    std::cout << std::endl;

    print_transform(standard.transform);
    std::cout << std::endl;

    print_selection_reason(standard);

    std::cout << "//============================================================" << std::endl;
}

//===========================================================================
// MAIN FUNCTION
//===========================================================================
int main(int argc, char* argv[]) {
    std::string input_file = "Reduced_output.cif";

    if (argc >= 2) {
        input_file = argv[1];
    }

    std::string output_file = make_standard_filename(input_file);

    CellData cell = load_cif(input_file);

    if (cell.atoms.empty()) {
        std::cerr << "Error: No atoms loaded from CIF" << std::endl;
        return 1;
    }

    Matrix3d primitive_vectors = original_cell_vectors(cell);

    CandidateCell standard;
    if (!find_best_standard_cell(primitive_vectors, standard)) {
        std::cerr << "Error: Cannot construct standard cell" << std::endl;
        return 1;
    }

    std::vector<AtomSite> standard_atoms = expand_atoms_to_standard_cell(cell, standard.transform);

    if (standard_atoms.empty()) {
        std::cerr << "Error: Cannot transform atoms to standard cell" << std::endl;
        return 1;
    }

    if (!write_standard_cif(output_file, cell, standard, standard_atoms)) {
        std::cerr << "Error: Cannot write standard CIF" << std::endl;
        return 1;
    }

    print_report(cell, standard, standard_atoms, output_file);

    return 0;
}
