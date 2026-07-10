//===========================================================================
// PROGRAM: Compare CF32 Structures
// Developed by Dmitry Bublikov, 2026
//===========================================================================
//
// Purpose:
//   Compares two CF32 NaCl-derived CIF structures on the cation sublattice.
//   The program checks whether the two structures are:
//      1) identical by translation only,
//      2) equivalent by proper rotation + translation,
//      3) equivalent only by improper operation + translation,
//         i.e. mirror/enantiomorphic equivalents,
//      4) different.
//
//   If a mirror relationship is found, the program also writes a special
//   visualization CIF file. In this file the first structure is placed on
//   the left side, and the second structure is transformed and placed on
//   the right side as its mirror image. A mirror plane is located at x=1/2
//   of the visualization cell.
//
// Notes:
//   - Designed for CF32 structures with fractional coordinates 0, 1/4, 1/2,
//     3/4 on the cation sublattice.
//   - The comparison test uses only cations.
//   - Oxygen atoms are included in the visualization CIF, but do not affect
//     the equivalence test.
//   - The program does not determine a full space group.
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
const int GRID_SIZE = 4;
const int EMPTY_SITE = 0;

// Gap between the two complete cells in paper-view CIF, in units of one CF32 cell.
const double PAPER_GAP_IN_CELL_UNITS = 0.50;

//===========================================================================
// STRUCT: Int3
//===========================================================================
struct Int3 {
    int x;
    int y;
    int z;
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
    int ix;
    int iy;
    int iz;
    bool is_cation;
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
    std::vector<AtomType> atom_types;
    std::vector<AtomSite> atoms;
};

//===========================================================================
// STRUCT: MatchResult
//===========================================================================
struct MatchResult {
    bool found;
    bool translation_only;
    bool proper;
    bool improper;
    Matrix3i op;
    Int3 shift;
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
// FUNCTION: mod4
//===========================================================================
int mod4(int value) {
    int r = value % GRID_SIZE;
    if (r < 0) r += GRID_SIZE;
    return r;
}

//===========================================================================
// FUNCTION: coord_to_grid
//===========================================================================
int coord_to_grid(double value) {
    int v = (int)std::floor(value * 4.0 + 0.5);
    return mod4(v);
}

//===========================================================================
// FUNCTION: grid_to_coord
//===========================================================================
double grid_to_coord(int value) {
    return (double)mod4(value) / 4.0;
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
// FUNCTION: int3_equal
//===========================================================================
bool int3_equal(const Int3& a, const Int3& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

//===========================================================================
// FUNCTION: int3_key
//===========================================================================
int int3_key(const Int3& v) {
    return mod4(v.x) * 16 + mod4(v.y) * 4 + mod4(v.z);
}

//===========================================================================
// FUNCTION: symbol_is_cation
//===========================================================================
bool symbol_is_cation(const std::string& symbol,
                      const std::map<std::string, int>& oxidation_map,
                      const std::string& label) {
    std::map<std::string, int>::const_iterator it = oxidation_map.find(symbol);
    if (it != oxidation_map.end()) {
        return it->second > 0;
    }

    if (!label.empty() && (label[0] == 'C' || label[0] == 'M')) return true;
    if (symbol == "O" || symbol == "F" || symbol == "Cl" ||
        symbol == "Br" || symbol == "I" || symbol == "N") return false;

    return true;
}

//===========================================================================
// FUNCTION: load_cif
//===========================================================================
CellData load_cif(const std::string& filename) {
    CellData cell;
    cell.a = 8.0;
    cell.b = 8.0;
    cell.c = 8.0;
    cell.alpha = 90.0;
    cell.beta = 90.0;
    cell.gamma = 90.0;

    std::vector<std::string> lines = read_all_lines(filename);
    std::map<std::string, int> oxidation_map;

    for (size_t i = 0; i < lines.size(); i++) {
        std::string line = trim(lines[i]);
        if (line.empty()) continue;

        if (starts_with(line, "_cell_length_a")) cell.a = parse_number_after_key(line);
        else if (starts_with(line, "_cell_length_b")) cell.b = parse_number_after_key(line);
        else if (starts_with(line, "_cell_length_c")) cell.c = parse_number_after_key(line);
        else if (starts_with(line, "_cell_angle_alpha")) cell.alpha = parse_number_after_key(line);
        else if (starts_with(line, "_cell_angle_beta")) cell.beta = parse_number_after_key(line);
        else if (starts_with(line, "_cell_angle_gamma")) cell.gamma = parse_number_after_key(line);
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
                        oxidation_map[t.symbol] = atoi(t.oxidation.c_str());
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
                    atom.ix = coord_to_grid(atom.fx);
                    atom.iy = coord_to_grid(atom.fy);
                    atom.iz = coord_to_grid(atom.fz);
                    atom.is_cation = symbol_is_cation(atom.symbol, oxidation_map, atom.label);
                    cell.atoms.push_back(atom);
                }
                j++;
            }
        }
    }

    return cell;
}

//===========================================================================
// FUNCTION: build_type_map
//===========================================================================
std::map<std::string, int> build_type_map(const std::vector<AtomSite>& a,
                                          const std::vector<AtomSite>& b) {
    std::map<std::string, int> type_map;
    int next_type = 1;

    for (size_t i = 0; i < a.size(); i++) {
        if (!a[i].is_cation) continue;
        if (type_map.find(a[i].symbol) == type_map.end()) {
            type_map[a[i].symbol] = next_type++;
        }
    }

    for (size_t i = 0; i < b.size(); i++) {
        if (!b[i].is_cation) continue;
        if (type_map.find(b[i].symbol) == type_map.end()) {
            type_map[b[i].symbol] = next_type++;
        }
    }

    return type_map;
}

//===========================================================================
// FUNCTION: build_lattice
//===========================================================================
bool build_lattice(const CellData& cell,
                   const std::map<std::string, int>& type_map,
                   int lattice[GRID_SIZE][GRID_SIZE][GRID_SIZE]) {
    for (int x = 0; x < GRID_SIZE; x++)
        for (int y = 0; y < GRID_SIZE; y++)
            for (int z = 0; z < GRID_SIZE; z++)
                lattice[x][y][z] = EMPTY_SITE;

    int cation_count = 0;

    for (size_t i = 0; i < cell.atoms.size(); i++) {
        if (!cell.atoms[i].is_cation) continue;

        int x = cell.atoms[i].ix;
        int y = cell.atoms[i].iy;
        int z = cell.atoms[i].iz;

        if (lattice[x][y][z] != EMPTY_SITE) {
            std::cerr << "Error: duplicate cation position at "
                      << x << " " << y << " " << z << std::endl;
            return false;
        }

        std::map<std::string, int>::const_iterator it = type_map.find(cell.atoms[i].symbol);
        if (it == type_map.end()) return false;

        lattice[x][y][z] = it->second;
        cation_count++;
    }

    if (cation_count != 32) {
        std::cerr << "Warning: expected 32 cations, found "
                  << cation_count << std::endl;
    }

    return true;
}

//===========================================================================
// FUNCTION: matrix_det
//===========================================================================
int matrix_det(const Matrix3i& m) {
    return m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1])
         - m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0])
         + m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
}

//===========================================================================
// FUNCTION: identity_matrix
//===========================================================================
Matrix3i identity_matrix() {
    Matrix3i m;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            m.m[i][j] = (i == j) ? 1 : 0;
    return m;
}

//===========================================================================
// FUNCTION: is_identity
//===========================================================================
bool is_identity(const Matrix3i& m) {
    Matrix3i id = identity_matrix();
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (m.m[i][j] != id.m[i][j]) return false;
    return true;
}

//===========================================================================
// FUNCTION: apply_operation_grid
//===========================================================================
Int3 apply_operation_grid(const Matrix3i& op, const Int3& p, const Int3& shift) {
    Int3 r;

    r.x = op.m[0][0] * p.x + op.m[0][1] * p.y + op.m[0][2] * p.z + shift.x;
    r.y = op.m[1][0] * p.x + op.m[1][1] * p.y + op.m[1][2] * p.z + shift.y;
    r.z = op.m[2][0] * p.x + op.m[2][1] * p.y + op.m[2][2] * p.z + shift.z;

    r.x = mod4(r.x);
    r.y = mod4(r.y);
    r.z = mod4(r.z);

    return r;
}

//===========================================================================
// FUNCTION: apply_inverse_operation_grid
//
// For signed permutation matrices, inverse(op) = transpose(op).
// This function returns p such that:
//
//      q = op * p + shift
//
//===========================================================================
Int3 apply_inverse_operation_grid(const Matrix3i& op, const Int3& q, const Int3& shift) {
    Int3 s;
    s.x = mod4(q.x - shift.x);
    s.y = mod4(q.y - shift.y);
    s.z = mod4(q.z - shift.z);

    Int3 p;
    p.x = op.m[0][0] * s.x + op.m[1][0] * s.y + op.m[2][0] * s.z;
    p.y = op.m[0][1] * s.x + op.m[1][1] * s.y + op.m[2][1] * s.z;
    p.z = op.m[0][2] * s.x + op.m[1][2] * s.y + op.m[2][2] * s.z;

    p.x = mod4(p.x);
    p.y = mod4(p.y);
    p.z = mod4(p.z);

    return p;
}

//===========================================================================
// FUNCTION: generate_signed_permutation_matrices
//===========================================================================
std::vector<Matrix3i> generate_signed_permutation_matrices() {
    std::vector<Matrix3i> result;

    int perm[3] = {0, 1, 2};

    do {
        for (int sx = -1; sx <= 1; sx += 2) {
            for (int sy = -1; sy <= 1; sy += 2) {
                for (int sz = -1; sz <= 1; sz += 2) {
                    Matrix3i m;

                    for (int i = 0; i < 3; i++)
                        for (int j = 0; j < 3; j++)
                            m.m[i][j] = 0;

                    m.m[0][perm[0]] = sx;
                    m.m[1][perm[1]] = sy;
                    m.m[2][perm[2]] = sz;

                    result.push_back(m);
                }
            }
        }
    } while (std::next_permutation(perm, perm + 3));

    return result;
}

//===========================================================================
// FUNCTION: check_operation
//===========================================================================
bool check_operation(const int lattice_a[GRID_SIZE][GRID_SIZE][GRID_SIZE],
                     const int lattice_b[GRID_SIZE][GRID_SIZE][GRID_SIZE],
                     const Matrix3i& op,
                     const Int3& shift) {
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            for (int z = 0; z < GRID_SIZE; z++) {
                Int3 p;
                p.x = x;
                p.y = y;
                p.z = z;

                Int3 q = apply_operation_grid(op, p, shift);

                if (lattice_a[x][y][z] != lattice_b[q.x][q.y][q.z]) {
                    return false;
                }
            }
        }
    }

    return true;
}

//===========================================================================
// FUNCTION: find_match
//===========================================================================
MatchResult find_match(const int lattice_a[GRID_SIZE][GRID_SIZE][GRID_SIZE],
                       const int lattice_b[GRID_SIZE][GRID_SIZE][GRID_SIZE],
                       int required_det) {
    MatchResult result;
    result.found = false;
    result.translation_only = false;
    result.proper = false;
    result.improper = false;
    result.op = identity_matrix();
    result.shift.x = result.shift.y = result.shift.z = 0;

    std::vector<Matrix3i> operations = generate_signed_permutation_matrices();

    for (size_t i = 0; i < operations.size(); i++) {
        int det = matrix_det(operations[i]);
        if (required_det != 0 && det != required_det) continue;

        for (int dx = 0; dx < GRID_SIZE; dx++) {
            for (int dy = 0; dy < GRID_SIZE; dy++) {
                for (int dz = 0; dz < GRID_SIZE; dz++) {
                    Int3 shift;
                    shift.x = dx;
                    shift.y = dy;
                    shift.z = dz;

                    if (check_operation(lattice_a, lattice_b, operations[i], shift)) {
                        result.found = true;
                        result.op = operations[i];
                        result.shift = shift;
                        result.translation_only = is_identity(operations[i]);
                        result.proper = (det == 1);
                        result.improper = (det == -1);
                        return result;
                    }
                }
            }
        }
    }

    return result;
}

//===========================================================================
// FUNCTION: make_paper_filename
//===========================================================================
std::string make_paper_filename(const std::string& file_a, const std::string& file_b) {
    size_t slash_a = file_a.find_last_of("\\/");
    size_t dot_a = file_a.find_last_of('.');
    std::string name_a = (slash_a == std::string::npos) ? file_a : file_a.substr(slash_a + 1);
    if (dot_a != std::string::npos && (slash_a == std::string::npos || dot_a > slash_a)) {
        name_a = name_a.substr(0, name_a.find_last_of('.'));
    }

    size_t slash_b = file_b.find_last_of("\\/");
    size_t dot_b = file_b.find_last_of('.');
    std::string name_b = (slash_b == std::string::npos) ? file_b : file_b.substr(slash_b + 1);
    if (dot_b != std::string::npos && (slash_b == std::string::npos || dot_b > slash_b)) {
        name_b = name_b.substr(0, name_b.find_last_of('.'));
    }

    std::string path = "";
    if (slash_a != std::string::npos) {
        path = file_a.substr(0, slash_a + 1);
    }

    return path + "Mirror_paper_" + name_a + "_vs_" + name_b + ".cif";
}

//===========================================================================
// FUNCTION: print_matrix
//===========================================================================
void print_matrix(const Matrix3i& op) {
    for (int row = 0; row < 3; row++) {
        std::cout << "  "
                  << std::setw(3) << op.m[row][0] << " "
                  << std::setw(3) << op.m[row][1] << " "
                  << std::setw(3) << op.m[row][2] << std::endl;
    }
}

//===========================================================================
// FUNCTION: print_operation_formula
//===========================================================================
void print_operation_formula(const MatchResult& match) {
    std::cout << "Operation in CF32 integer coordinates:" << std::endl;
    std::cout << "  q = M * p + t  (mod 4)" << std::endl;
    std::cout << "M:" << std::endl;
    print_matrix(match.op);
    std::cout << "t = (" << match.shift.x << " "
              << match.shift.y << " "
              << match.shift.z << ")" << std::endl;
    std::cout << std::endl;

    std::cout << "Translation in fractional coordinates:" << std::endl;
    std::cout << "  t = ("
              << std::fixed << std::setprecision(2)
              << grid_to_coord(match.shift.x) << " "
              << grid_to_coord(match.shift.y) << " "
              << grid_to_coord(match.shift.z) << ")" << std::endl;
}

//===========================================================================
// FUNCTION: write_paper_visualization_cif
//
// This file is intended for VESTA / publication figures.
// It contains two complete cells with a gap:
//
//   | left full cell | gap / mirror plane | right full cell |
//
// The right side is not compressed into a half-cell, so no boundary layer
// is visually lost along the a axis.
//
//===========================================================================
bool write_paper_visualization_cif(const std::string& filename,
                                   const CellData& cell_a,
                                   const CellData& cell_b,
                                   const MatchResult& mirror_match) {
    std::ofstream cif(filename.c_str());

    if (!cif.is_open()) {
        std::cerr << "Error: cannot write " << filename << std::endl;
        return false;
    }

    double scale_x = 2.0 + PAPER_GAP_IN_CELL_UNITS;
    double a_vis = scale_x * cell_a.a;
    double b_vis = cell_a.b;
    double c_vis = cell_a.c;

    double mirror_x = (1.0 + PAPER_GAP_IN_CELL_UNITS / 2.0) / scale_x;
    double right_offset = 1.0 + PAPER_GAP_IN_CELL_UNITS;

    cif << std::fixed << std::setprecision(6);
    cif << "data_mirror_paper_visualization\n";
    cif << "_audit_creation_method 'Generated by Compare_CF32_structures.cpp'\n";
    cif << "_chemical_formula_sum 'mirror comparison view'\n";
    cif << "_cell_length_a " << a_vis << "\n";
    cif << "_cell_length_b " << b_vis << "\n";
    cif << "_cell_length_c " << c_vis << "\n";
    cif << "_cell_angle_alpha " << cell_a.alpha << "\n";
    cif << "_cell_angle_beta " << cell_a.beta << "\n";
    cif << "_cell_angle_gamma " << cell_a.gamma << "\n";
    cif << "_cell_volume " << (a_vis * b_vis * c_vis) << "\n";
    cif << "_symmetry_cell_setting triclinic\n";
    cif << "_symmetry_int_tables_number 1\n";
    cif << "_symmetry_space_group_name_H-M 'P 1'\n";
    cif << "_symmetry_space_group_name_Hall 'P_1'\n\n";

    cif << "loop_\n";
    cif << "_symmetry_equiv_pos_site_id\n";
    cif << "_symmetry_equiv_pos_as_xyz\n";
    cif << "1 x,y,z\n\n";

    std::map<std::string, bool> written_types;

    cif << "loop_\n";
    cif << "_atom_type_symbol\n";
    cif << "_atom_type_oxidation_number\n";
    cif << "_atom_type_radius_bond\n";

    for (size_t i = 0; i < cell_a.atom_types.size(); i++) {
        if (!written_types[cell_a.atom_types[i].symbol]) {
            cif << cell_a.atom_types[i].symbol << "\t"
                << cell_a.atom_types[i].oxidation << "\t?\n";
            written_types[cell_a.atom_types[i].symbol] = true;
        }
    }

    for (size_t i = 0; i < cell_b.atom_types.size(); i++) {
        if (!written_types[cell_b.atom_types[i].symbol]) {
            cif << cell_b.atom_types[i].symbol << "\t"
                << cell_b.atom_types[i].oxidation << "\t?\n";
            written_types[cell_b.atom_types[i].symbol] = true;
        }
    }

    if (!written_types["X"]) {
        cif << "X\t0\t?\n";
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

    // Left complete cell: original structure A.
    // For publication-style visualization we explicitly duplicate atoms
    // located at x = 0 onto the right boundary of the left cell.
    // This makes the displayed cube look complete in non-periodic figures.
    for (size_t i = 0; i < cell_a.atoms.size(); i++) {
        const AtomSite& atom = cell_a.atoms[i];

        double local_x = normalize_fraction(atom.fx);
        double y = normalize_fraction(atom.fy);
        double z = normalize_fraction(atom.fz);

        double x = local_x / scale_x;

        label_count[atom.symbol]++;
        std::string label = "L_" + atom.symbol + std::to_string(label_count[atom.symbol]);

        cif << label << " "
            << atom.symbol << "  "
            << x << "  " << y << "  " << z << "  "
            << "1.000 1 a ? d ? ?\n";

        if (atom.ix == 0) {
            double x_boundary = 1.0 / scale_x;

            label_count[atom.symbol]++;
            std::string label_boundary = "L_" + atom.symbol + std::to_string(label_count[atom.symbol]);

            cif << label_boundary << " "
                << atom.symbol << "  "
                << x_boundary << "  " << y << "  " << z << "  "
                << "1.000 1 a ? d ? ?\n";
        }
    }

    // Right complete cell:
    //   1) take atoms from B;
    //   2) transform them back to A-like coordinates using inverse mirror op;
    //   3) reflect across the central mirror plane in the paper-view cell.
    //
    // The formula x = right_offset + (1 - px) places a full mirrored copy
    // to the right of the mirror region without losing the boundary layer.
    for (size_t i = 0; i < cell_b.atoms.size(); i++) {
        const AtomSite& atom = cell_b.atoms[i];

        Int3 q;
        q.x = atom.ix;
        q.y = atom.iy;
        q.z = atom.iz;

        Int3 p = apply_inverse_operation_grid(mirror_match.op, q, mirror_match.shift);

        double px = grid_to_coord(p.x);
        double py = grid_to_coord(p.y);
        double pz = grid_to_coord(p.z);

        double x_unscaled = right_offset + (1.0 - px);
        double x = x_unscaled / scale_x;
        double y = py;
        double z = pz;

        label_count[atom.symbol]++;
        std::string label = "R_" + atom.symbol + std::to_string(label_count[atom.symbol]);

        cif << label << " "
            << atom.symbol << "  "
            << x << "  " << y << "  " << z << "  "
            << "1.000 1 a ? d ? ?\n";

        // The mirrored copy also needs an explicit boundary layer.
        // Atoms with px = 0 represent the periodic boundary of the original
        // cell; after reflection their duplicate is placed at the left
        // boundary of the right cell.
        if (p.x == 0) {
            double x_boundary = right_offset / scale_x;

            label_count[atom.symbol]++;
            std::string label_boundary = "R_" + atom.symbol + std::to_string(label_count[atom.symbol]);

            cif << label_boundary << " "
                << atom.symbol << "  "
                << x_boundary << "  " << y << "  " << z << "  "
                << "1.000 1 a ? d ? ?\n";
        }
    }

    // Dummy X atoms mark the mirror plane. In VESTA these can be colored
    // differently, hidden, or used as guides for drawing a transparent plane.
    cif << "Mirror1 X  " << mirror_x << "  0.000000  0.000000  1.000 1 a ? d ? ?\n";
    cif << "Mirror2 X  " << mirror_x << "  1.000000  0.000000  1.000 1 a ? d ? ?\n";
    cif << "Mirror3 X  " << mirror_x << "  0.000000  1.000000  1.000 1 a ? d ? ?\n";
    cif << "Mirror4 X  " << mirror_x << "  1.000000  1.000000  1.000 1 a ? d ? ?\n";
    cif << "Mirror5 X  " << mirror_x << "  0.500000  0.500000  1.000 1 a ? d ? ?\n";

    cif.close();
    return true;
}

//===========================================================================
// FUNCTION: print_report
//===========================================================================
void print_report(const CellData& cell_a,
                  const CellData& cell_b,
                  const MatchResult& direct,
                  const MatchResult& proper,
                  const MatchResult& improper,
                  const std::string& paper_file) {
    int cations_a = 0;
    int cations_b = 0;

    for (size_t i = 0; i < cell_a.atoms.size(); i++) {
        if (cell_a.atoms[i].is_cation) cations_a++;
    }

    for (size_t i = 0; i < cell_b.atoms.size(); i++) {
        if (cell_b.atoms[i].is_cation) cations_b++;
    }

    std::cout << "//============================================================" << std::endl;
    std::cout << "// CF32 STRUCTURE COMPARISON REPORT" << std::endl;
    std::cout << "//============================================================" << std::endl;
    std::cout << "Structure A atoms:     " << cell_a.atoms.size() << std::endl;
    std::cout << "Structure A cations:   " << cations_a << std::endl;
    std::cout << "Structure B atoms:     " << cell_b.atoms.size() << std::endl;
    std::cout << "Structure B cations:   " << cations_b << std::endl;
    std::cout << std::endl;

    if (direct.found) {
        std::cout << "Result: TRANSLATION-EQUIVALENT" << std::endl;
        print_operation_formula(direct);
    } else if (proper.found) {
        std::cout << "Result: ROTATION-EQUIVALENT" << std::endl;
        std::cout << "The structures are equivalent by a proper operation (det = +1)." << std::endl;
        print_operation_formula(proper);
    } else if (improper.found) {
        std::cout << "Result: MIRROR / ENANTIOMORPHIC PAIR" << std::endl;
        std::cout << "The structures are equivalent only by an improper operation (det = -1)." << std::endl;
        std::cout << "This means that they are mirror-related on the CF32 cation lattice." << std::endl;
        std::cout << std::endl;
        print_operation_formula(improper);
        std::cout << std::endl;
        std::cout << "Paper visualization CIF: " << paper_file << std::endl;
        std::cout << "The visualization contains two complete cells separated by a gap." << std::endl;
    } else {
        std::cout << "Result: DIFFERENT" << std::endl;
        std::cout << "No translation, proper rotation, or improper mirror relation was found." << std::endl;
    }

    std::cout << "//============================================================" << std::endl;
}

//===========================================================================
// MAIN FUNCTION
//===========================================================================
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage:" << std::endl;
        std::cout << "  compare_CF32.exe structure_A.cif structure_B.cif" << std::endl;
        return 0;
    }

    std::string file_a = argv[1];
    std::string file_b = argv[2];

    CellData cell_a = load_cif(file_a);
    CellData cell_b = load_cif(file_b);

    if (cell_a.atoms.empty()) {
        std::cerr << "Error: no atoms loaded from " << file_a << std::endl;
        return 1;
    }

    if (cell_b.atoms.empty()) {
        std::cerr << "Error: no atoms loaded from " << file_b << std::endl;
        return 1;
    }

    std::map<std::string, int> type_map = build_type_map(cell_a.atoms, cell_b.atoms);

    int lattice_a[GRID_SIZE][GRID_SIZE][GRID_SIZE];
    int lattice_b[GRID_SIZE][GRID_SIZE][GRID_SIZE];

    if (!build_lattice(cell_a, type_map, lattice_a)) {
        std::cerr << "Error: cannot build cation lattice for structure A" << std::endl;
        return 1;
    }

    if (!build_lattice(cell_b, type_map, lattice_b)) {
        std::cerr << "Error: cannot build cation lattice for structure B" << std::endl;
        return 1;
    }

    Matrix3i identity = identity_matrix();

    MatchResult direct;
    direct.found = false;
    direct.translation_only = false;
    direct.proper = false;
    direct.improper = false;
    direct.op = identity;

    for (int dx = 0; dx < GRID_SIZE && !direct.found; dx++) {
        for (int dy = 0; dy < GRID_SIZE && !direct.found; dy++) {
            for (int dz = 0; dz < GRID_SIZE && !direct.found; dz++) {
                Int3 shift;
                shift.x = dx;
                shift.y = dy;
                shift.z = dz;

                if (check_operation(lattice_a, lattice_b, identity, shift)) {
                    direct.found = true;
                    direct.translation_only = true;
                    direct.proper = true;
                    direct.improper = false;
                    direct.shift = shift;
                }
            }
        }
    }

    MatchResult proper = find_match(lattice_a, lattice_b, +1);
    MatchResult improper = find_match(lattice_a, lattice_b, -1);

    std::string paper_file = "";
    if (!direct.found && !proper.found && improper.found) {
        paper_file = make_paper_filename(file_a, file_b);
        write_paper_visualization_cif(paper_file, cell_a, cell_b, improper);
    }

    print_report(cell_a, cell_b, direct, proper, improper, paper_file);

    return 0;
}
