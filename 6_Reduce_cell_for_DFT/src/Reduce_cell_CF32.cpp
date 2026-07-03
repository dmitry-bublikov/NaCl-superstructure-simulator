//===========================================================================
// PROGRAM: Reduce CF32 Cell (Chemical Translation Cell Reducer)
// Developed by Dmitry Bublikov, 2026
//===========================================================================
//
// Purpose:
//   Searches for hidden translational symmetry in CF32 NaCl-derived
//   superstructures and constructs the smallest translationally
//   equivalent chemical unit cell.
//
// Algorithm:
//   Read CIF
//       ↓
//   Build 4×4×4 cation lattice
//       ↓
//   Test all 63 non-zero translations
//       ↓
//   Find independent translations
//       ↓
//   Construct the smallest chemical cell
//       ↓
//   Write reduced CIF
//
// Features:
//   - CF32-specific implementation
//   - no external libraries
//   - integer 4×4×4 lattice representation
//   - exhaustive search over all 63 non-zero translations
//   - automatic construction of the primitive chemical cell
//   - writes a reduced CIF file
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

//===========================================================================
// CONSTANTS
//===========================================================================
const int GRID_SIZE = 4;
const int GRID_VOLUME = 64;
const double PI_VALUE = 3.14159265358979323846;

//===========================================================================
// STRUCT: Int3
//===========================================================================
struct Int3 {
    int x;
    int y;
    int z;
};

//===========================================================================
// STRUCT: Vec3
//===========================================================================
struct Vec3 {
    double x;
    double y;
    double z;
};

//===========================================================================
// STRUCT: AtomType
//===========================================================================
struct AtomType {
    std::string symbol;
    int oxidation;
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
    double formula_weight;
    std::vector<AtomType> atom_types;
    std::vector<AtomSite> atoms;
};

//===========================================================================
// STRUCT: Matrix3i
//===========================================================================
struct Matrix3i {
    int m[3][3];
};

//===========================================================================
// STRUCT: Matrix3d
//===========================================================================
struct Matrix3d {
    double m[3][3];
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
            (s[0] == '"' && s[s.size() - 1] == '"')) {
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
    return (double)value / 4.0;
}

//===========================================================================
// FUNCTION: int3_equal
//===========================================================================
bool int3_equal(const Int3& a, const Int3& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

//===========================================================================
// FUNCTION: int3_less
//===========================================================================
bool int3_less(const Int3& a, const Int3& b) {
    if (a.x != b.x) return a.x < b.x;
    if (a.y != b.y) return a.y < b.y;
    return a.z < b.z;
}

//===========================================================================
// FUNCTION: int3_key
//===========================================================================
int int3_key(const Int3& v) {
    return mod4(v.x) * 16 + mod4(v.y) * 4 + mod4(v.z);
}

//===========================================================================
// FUNCTION: add_mod4
//===========================================================================
Int3 add_mod4(const Int3& a, const Int3& b) {
    Int3 r;
    r.x = mod4(a.x + b.x);
    r.y = mod4(a.y + b.y);
    r.z = mod4(a.z + b.z);
    return r;
}

//===========================================================================
// FUNCTION: contains_translation
//===========================================================================
bool contains_translation(const std::vector<Int3>& translations, const Int3& t) {
    for (size_t i = 0; i < translations.size(); i++) {
        if (int3_equal(translations[i], t)) return true;
    }
    return false;
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
    if (symbol == "O" || symbol == "F" || symbol == "Cl" || symbol == "Br" || symbol == "I" || symbol == "N") return false;
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
    cell.formula_weight = 0.0;
    
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
        else if (starts_with(line, "_chemical_formula_weight")) cell.formula_weight = parse_number_after_key(line);
    }
    
    // Parse loops
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
                if ((int)tokens.size() > idx_symbol && idx_symbol >= 0) {
                    AtomType t;
                    t.symbol = remove_quotes(tokens[idx_symbol]);
                    t.oxidation = 0;
                    if (idx_ox >= 0 && (int)tokens.size() > idx_ox) {
                        t.oxidation = atoi(remove_quotes(tokens[idx_ox]).c_str());
                    }
                    cell.atom_types.push_back(t);
                    oxidation_map[t.symbol] = t.oxidation;
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
                std::cerr << "Error: Invalid _atom_site loop" << std::endl;
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
std::map<std::string, int> build_type_map(const std::vector<AtomSite>& atoms) {
    std::map<std::string, int> type_map;
    int next_type = 1;
    
    for (size_t i = 0; i < atoms.size(); i++) {
        if (!atoms[i].is_cation) continue;
        if (type_map.find(atoms[i].symbol) == type_map.end()) {
            type_map[atoms[i].symbol] = next_type;
            next_type++;
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
                lattice[x][y][z] = 0;
    
    int cation_count = 0;
    for (size_t i = 0; i < cell.atoms.size(); i++) {
        if (!cell.atoms[i].is_cation) continue;
        int x = cell.atoms[i].ix;
        int y = cell.atoms[i].iy;
        int z = cell.atoms[i].iz;
        
        if (lattice[x][y][z] != 0) {
            std::cerr << "Error: Duplicate cation position at "
                      << x << " " << y << " " << z << std::endl;
            return false;
        }
        
        std::map<std::string, int>::const_iterator it = type_map.find(cell.atoms[i].symbol);
        if (it == type_map.end()) return false;
        lattice[x][y][z] = it->second;
        cation_count++;
    }
    
    if (cation_count != 32) {
        std::cerr << "Warning: Expected 32 cations, found " << cation_count << std::endl;
    }
    
    return true;
}

//===========================================================================
// FUNCTION: check_translation
//
// Every possible translation of the CF32 cation ordering is tested.
//
// Since all cation coordinates are multiples of 1/4,
// every valid translation must also belong to the same
// 4×4×4 fractional lattice.
//
// Therefore only
//
//      4×4×4 − 1 = 63
//
// non-zero translations have to be checked.
//===========================================================================
bool check_translation(const int lattice[GRID_SIZE][GRID_SIZE][GRID_SIZE], int dx, int dy, int dz) {
// Oxygen atoms are not analysed.
//
// All CF32 structures have exactly the same oxygen
// sublattice. Only the cation ordering determines
// translational symmetry of the chemical structure.
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            for (int z = 0; z < GRID_SIZE; z++) {
                int nx = (x + dx) & 3;
                int ny = (y + dy) & 3;
                int nz = (z + dz) & 3;
                if (lattice[x][y][z] != lattice[nx][ny][nz]) {
                    return false;
                }
            }
        }
    }
    return true;
}

//===========================================================================
// FUNCTION: find_translations
//===========================================================================
std::vector<Int3> find_translations(const int lattice[GRID_SIZE][GRID_SIZE][GRID_SIZE]) {
    std::vector<Int3> translations;
    
    Int3 zero;
    zero.x = 0;
    zero.y = 0;
    zero.z = 0;
    translations.push_back(zero);
    
    for (int dx = 0; dx < GRID_SIZE; dx++) {
        for (int dy = 0; dy < GRID_SIZE; dy++) {
            for (int dz = 0; dz < GRID_SIZE; dz++) {
                if (dx == 0 && dy == 0 && dz == 0) continue;
                if (check_translation(lattice, dx, dy, dz)) {
                    Int3 t;
                    t.x = dx;
                    t.y = dy;
                    t.z = dz;
                    translations.push_back(t);
                }
            }
        }
    }
    
    std::sort(translations.begin(), translations.end(), int3_less);
    return translations;
}

//===========================================================================
// FUNCTION: determinant3
//===========================================================================
int determinant3(const Int3& a, const Int3& b, const Int3& c) {
    return a.x * (b.y * c.z - b.z * c.y)
         - a.y * (b.x * c.z - b.z * c.x)
         + a.z * (b.x * c.y - b.y * c.x);
}

//===========================================================================
// FUNCTION: vector_norm2
//===========================================================================
int vector_norm2(const Int3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

//===========================================================================
// FUNCTION: vector_is_in_translation_lattice
//===========================================================================
bool vector_is_in_translation_lattice(const Int3& v, const std::set<int>& translation_keys) {
    Int3 r;
    r.x = mod4(v.x);
    r.y = mod4(v.y);
    r.z = mod4(v.z);
    return translation_keys.find(int3_key(r)) != translation_keys.end();
}

//===========================================================================
// FUNCTION: build_candidate_vectors
//===========================================================================
std::vector<Int3> build_candidate_vectors(const std::vector<Int3>& translations) {
    std::set<int> translation_keys;
    for (size_t i = 0; i < translations.size(); i++) {
        translation_keys.insert(int3_key(translations[i]));
    }
    
    std::vector<Int3> candidates;
    for (int x = 0; x <= 4; x++) {
        for (int y = 0; y <= 4; y++) {
            for (int z = 0; z <= 4; z++) {
                if (x == 0 && y == 0 && z == 0) continue;
                Int3 v;
                v.x = x;
                v.y = y;
                v.z = z;
                if (vector_is_in_translation_lattice(v, translation_keys)) {
                    candidates.push_back(v);
                }
            }
        }
    }
    
    std::sort(candidates.begin(), candidates.end(),
        [](const Int3& a, const Int3& b) {
            int na = vector_norm2(a);
            int nb = vector_norm2(b);
            if (na != nb) return na < nb;
            if (a.x != b.x) return a.x < b.x;
            if (a.y != b.y) return a.y < b.y;
            return a.z < b.z;
        });
    
    return candidates;
}

//===========================================================================
// FUNCTION: create_default_basis (prototype)
//===========================================================================
Matrix3i create_default_basis();

//===========================================================================
// FUNCTION: find_primitive_basis
//===========================================================================
bool find_primitive_basis(const std::vector<Int3>& translations, Matrix3i& basis) {
    int group_size = (int)translations.size();
    if (group_size <= 0) return false;
    
    if (group_size == 1) {
        basis = create_default_basis();
        return true;
    }
    
    int target_det = GRID_VOLUME / group_size;
    std::vector<Int3> candidates = build_candidate_vectors(translations);
    
    int best_score = 2147483647;
    bool found = false;
    Int3 best_a, best_b, best_c;
    
    for (size_t i = 0; i < candidates.size(); i++) {
        for (size_t j = i + 1; j < candidates.size(); j++) {
            for (size_t k = j + 1; k < candidates.size(); k++) {
                int det = determinant3(candidates[i], candidates[j], candidates[k]);
                if (det == 0) continue;
                if (std::abs(det) != target_det) continue;
                
                int score = vector_norm2(candidates[i]) +
                            vector_norm2(candidates[j]) +
                            vector_norm2(candidates[k]);
                
                if (!found || score < best_score) {
                    found = true;
                    best_score = score;
                    best_a = candidates[i];
                    best_b = candidates[j];
                    best_c = candidates[k];
                }
            }
        }
    }
    
    if (!found) return false;
    
    if (determinant3(best_a, best_b, best_c) < 0) {
        Int3 temp = best_b;
        best_b = best_c;
        best_c = temp;
    }
    
    basis.m[0][0] = best_a.x; basis.m[1][0] = best_a.y; basis.m[2][0] = best_a.z;
    basis.m[0][1] = best_b.x; basis.m[1][1] = best_b.y; basis.m[2][1] = best_b.z;
    basis.m[0][2] = best_c.x; basis.m[1][2] = best_c.y; basis.m[2][2] = best_c.z;
    
    return true;
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
// FUNCTION: multiply_cell_basis
//===========================================================================
Matrix3d multiply_cell_basis(const Matrix3d& original, const Matrix3i& basis) {
    Matrix3d result;
    for (int col = 0; col < 3; col++) {
        for (int row = 0; row < 3; row++) {
            result.m[row][col] = 0.0;
            for (int k = 0; k < 3; k++) {
                result.m[row][col] += original.m[row][k] * ((double)basis.m[k][col] / 4.0);
            }
        }
    }
    return result;
}

//===========================================================================
// FUNCTION: dot_vec
//===========================================================================
double dot_vec(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
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
// FUNCTION: determinant_matrix_int
//===========================================================================
int determinant_matrix_int(const Matrix3i& b) {
    Int3 a, c, d;
    a.x = b.m[0][0]; a.y = b.m[1][0]; a.z = b.m[2][0];
    c.x = b.m[0][1]; c.y = b.m[1][1]; c.z = b.m[2][1];
    d.x = b.m[0][2]; d.y = b.m[1][2]; d.z = b.m[2][2];
    return determinant3(a, c, d);
}

//===========================================================================
// FUNCTION: inverse_matrix3
//===========================================================================
bool inverse_matrix3(const Matrix3i& b, double inv[3][3]) {
    double m00 = (double)b.m[0][0], m01 = (double)b.m[0][1], m02 = (double)b.m[0][2];
    double m10 = (double)b.m[1][0], m11 = (double)b.m[1][1], m12 = (double)b.m[1][2];
    double m20 = (double)b.m[2][0], m21 = (double)b.m[2][1], m22 = (double)b.m[2][2];
    
    double det = m00 * (m11 * m22 - m12 * m21)
               - m01 * (m10 * m22 - m12 * m20)
               + m02 * (m10 * m21 - m11 * m20);
    
    if (std::fabs(det) < 1e-12) return false;
    
    inv[0][0] =  (m11 * m22 - m12 * m21) / det;
    inv[0][1] = -(m01 * m22 - m02 * m21) / det;
    inv[0][2] =  (m01 * m12 - m02 * m11) / det;
    inv[1][0] = -(m10 * m22 - m12 * m20) / det;
    inv[1][1] =  (m00 * m22 - m02 * m20) / det;
    inv[1][2] = -(m00 * m12 - m02 * m10) / det;
    inv[2][0] =  (m10 * m21 - m11 * m20) / det;
    inv[2][1] = -(m00 * m21 - m01 * m20) / det;
    inv[2][2] =  (m00 * m11 - m01 * m10) / det;
    
    return true;
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
// FUNCTION: transform_to_new_fractional
//===========================================================================
Vec3 transform_to_new_fractional(const Matrix3i& basis, const Int3& p) {
    double inv[3][3];
    Vec3 r;
    r.x = r.y = r.z = 0.0;
    
    if (!inverse_matrix3(basis, inv)) {
        return r;
    }
    
    double v[3];
    v[0] = (double)p.x;
    v[1] = (double)p.y;
    v[2] = (double)p.z;
    
    r.x = inv[0][0] * v[0] + inv[0][1] * v[1] + inv[0][2] * v[2];
    r.y = inv[1][0] * v[0] + inv[1][1] * v[1] + inv[1][2] * v[2];
    r.z = inv[2][0] * v[0] + inv[2][1] * v[1] + inv[2][2] * v[2];
    
    r.x = normalize_fraction(r.x);
    r.y = normalize_fraction(r.y);
    r.z = normalize_fraction(r.z);
    return r;
}

//===========================================================================
// FUNCTION: atom_position_key
//===========================================================================
std::string atom_position_key(const AtomSite& atom) {
    std::ostringstream out;
    out << atom.symbol << "_" << atom.ix << "_" << atom.iy << "_" << atom.iz;
    return out.str();
}

//===========================================================================
// FUNCTION: find_representative_atoms
//===========================================================================
std::vector<AtomSite> find_representative_atoms(const CellData& cell,
                                                const std::vector<Int3>& translations) {
    std::set<std::string> visited;
    std::vector<AtomSite> representatives;
    
    for (size_t i = 0; i < cell.atoms.size(); i++) {
        std::string key = atom_position_key(cell.atoms[i]);
        if (visited.find(key) != visited.end()) continue;
        
        representatives.push_back(cell.atoms[i]);
        
        for (size_t t = 0; t < translations.size(); t++) {
            AtomSite moved = cell.atoms[i];
            moved.ix = mod4(cell.atoms[i].ix + translations[t].x);
            moved.iy = mod4(cell.atoms[i].iy + translations[t].y);
            moved.iz = mod4(cell.atoms[i].iz + translations[t].z);
            visited.insert(atom_position_key(moved));
        }
    }
    
    return representatives;
}

//===========================================================================
// FUNCTION: gcd_int
//===========================================================================
int gcd_int(int a, int b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

//===========================================================================
// FUNCTION: calculate_formula_from_atoms
//===========================================================================
std::string calculate_formula_from_atoms(const std::vector<AtomSite>& atoms) {
    std::map<std::string, int> counts;
    for (size_t i = 0; i < atoms.size(); i++) {
        counts[atoms[i].symbol]++;
    }
    
    int g = 0;
    for (std::map<std::string, int>::iterator it = counts.begin(); it != counts.end(); ++it) {
        if (it->second > 0) {
            g = (g == 0) ? it->second : gcd_int(g, it->second);
        }
    }
    if (g < 1) g = 1;
    
    std::vector<std::string> order;
    if (counts.find("Na") != counts.end()) order.push_back("Na");
    if (counts.find("Li") != counts.end()) order.push_back("Li");
    if (counts.find("Mg") != counts.end()) order.push_back("Mg");
    if (counts.find("Fe") != counts.end()) order.push_back("Fe");
    
    for (std::map<std::string, int>::iterator it = counts.begin(); it != counts.end(); ++it) {
        if (it->first == "O" || it->first == "Na" || it->first == "Li" || it->first == "Mg" || it->first == "Fe") continue;
        order.push_back(it->first);
    }
    if (counts.find("O") != counts.end()) order.push_back("O");
    
    std::string formula;
    for (size_t i = 0; i < order.size(); i++) {
        std::string sym = order[i];
        int count = counts[sym] / g;
        formula += sym;
        if (count > 1) formula += std::to_string(count);
    }
    return formula;
}

//===========================================================================
// FUNCTION: write_reduced_cif
//===========================================================================
bool write_reduced_cif(const std::string& filename,
                       const CellData& cell,
                       const std::vector<AtomSite>& representatives,
                       const std::vector<Int3>& translations,
                       const Matrix3i& basis) {
    Matrix3d original = original_cell_vectors(cell);
    Matrix3d reduced = multiply_cell_basis(original, basis);
    
    Vec3 va = matrix_column(reduced, 0);
    Vec3 vb = matrix_column(reduced, 1);
    Vec3 vc = matrix_column(reduced, 2);
    
    double new_a = length_vec(va);
    double new_b = length_vec(vb);
    double new_c = length_vec(vc);
    double new_alpha = angle_between(vb, vc);
    double new_beta  = angle_between(va, vc);
    double new_gamma = angle_between(va, vb);
    double new_volume = cell.a * cell.b * cell.c *
                        std::sqrt(1.0 - std::cos(cell.alpha * PI_VALUE / 180.0) * std::cos(cell.alpha * PI_VALUE / 180.0)
                                      - std::cos(cell.beta  * PI_VALUE / 180.0) * std::cos(cell.beta  * PI_VALUE / 180.0)
                                      - std::cos(cell.gamma * PI_VALUE / 180.0) * std::cos(cell.gamma * PI_VALUE / 180.0)
                                      + 2.0 * std::cos(cell.alpha * PI_VALUE / 180.0) * std::cos(cell.beta * PI_VALUE / 180.0) * std::cos(cell.gamma * PI_VALUE / 180.0));
    new_volume /= (double)translations.size();
    
    std::ofstream cif(filename.c_str());
    if (!cif.is_open()) {
        std::cerr << "Error: Cannot write " << filename << std::endl;
        return false;
    }
    
    std::string formula = calculate_formula_from_atoms(representatives);
    
    cif << std::fixed << std::setprecision(6);
    cif << "data_CF32_reduced\n";
    cif << "_audit_creation_method 'Reduced by Reduce_cell.cpp'\n";
    cif << "_chemical_formula_sum\t'" << formula << "'\n";
    cif << "_cell_length_a\t" << new_a << "\n";
    cif << "_cell_length_b\t" << new_b << "\n";
    cif << "_cell_length_c\t" << new_c << "\n";
    cif << "_cell_angle_alpha\t" << new_alpha << "\n";
    cif << "_cell_angle_beta\t" << new_beta << "\n";
    cif << "_cell_angle_gamma\t" << new_gamma << "\n";
    cif << "_cell_volume\t" << new_volume << "\n";
    cif << "_symmetry_cell_setting\ttriclinic\n";
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
        for (size_t i = 0; i < representatives.size(); i++) {
            if (!written_types[representatives[i].symbol]) {
                cif << representatives[i].symbol << "\t?\t?\n";
                written_types[representatives[i].symbol] = true;
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
    for (size_t i = 0; i < representatives.size(); i++) {
        Int3 p;
        p.x = representatives[i].ix;
        p.y = representatives[i].iy;
        p.z = representatives[i].iz;
        Vec3 f = transform_to_new_fractional(basis, p);
        
        label_count[representatives[i].symbol]++;
        std::string label = representatives[i].symbol + std::to_string(label_count[representatives[i].symbol]);
        
        cif << label << " "
            << representatives[i].symbol << "  "
            << std::fixed << std::setprecision(6)
            << f.x << "  " << f.y << "  " << f.z << "  "
            << "1.000 1 a ? d ? ?\n";
    }
    
    cif.close();
    return true;
}

//===========================================================================
// FUNCTION: print_translation
//===========================================================================
void print_translation(const Int3& t) {
    std::cout << "(" << t.x << " " << t.y << " " << t.z << ")";
    std::cout << " = (" << std::fixed << std::setprecision(2)
              << grid_to_coord(t.x) << " "
              << grid_to_coord(t.y) << " "
              << grid_to_coord(t.z) << ")";
}

//===========================================================================
// FUNCTION: print_basis
//===========================================================================
void print_basis(const Matrix3i& basis) {
    std::cout << "Primitive basis vectors in original fractional coordinates:" << std::endl;
    for (int col = 0; col < 3; col++) {
        std::cout << "  v" << (col + 1) << " = ("
                  << std::fixed << std::setprecision(4)
                  << ((double)basis.m[0][col] / 4.0) << " "
                  << ((double)basis.m[1][col] / 4.0) << " "
                  << ((double)basis.m[2][col] / 4.0) << ")" << std::endl;
    }
}

//===========================================================================
// FUNCTION: print_report
//===========================================================================
void print_report(const CellData& cell,
                  const std::vector<Int3>& translations,
                  const std::vector<AtomSite>& representatives,
                  const Matrix3i& basis,
                  const std::string& output_file) {
    int cations = 0;
    int anions = 0;
    for (size_t i = 0; i < cell.atoms.size(); i++) {
        if (cell.atoms[i].is_cation) cations++;
        else anions++;
    }
    
    std::cout << "//============================================================" << std::endl;
    std::cout << "// CF32 CELL REDUCTION REPORT" << std::endl;
    std::cout << "//============================================================" << std::endl;
    std::cout << "Input atoms:       " << cell.atoms.size() << std::endl;
    std::cout << "Input cations:     " << cations << std::endl;
    std::cout << "Input anions:      " << anions << std::endl;
    std::cout << "Translations:      " << translations.size() << std::endl;
    std::cout << "Reduction factor:  " << translations.size() << std::endl;
    std::cout << "Output atoms:      " << representatives.size() << std::endl;
    std::cout << "Output file:       " << output_file << std::endl;
    std::cout << std::endl;
    
    std::cout << "Translations preserving cation coloring:" << std::endl;
    for (size_t i = 0; i < translations.size(); i++) {
        std::cout << "  ";
        print_translation(translations[i]);
        if (i == 0) std::cout << "  identity";
        std::cout << std::endl;
    }
    std::cout << std::endl;
    
    print_basis(basis);
    std::cout << std::endl;
    
    if (translations.size() == 1) {
        std::cout << "Status: PRIMITIVE CF32 CELL" << std::endl;
    } else {
        std::cout << "Status: REDUCED BY FACTOR " << translations.size() << std::endl;
    }
    std::cout << "//============================================================" << std::endl;
}

//===========================================================================
// FUNCTION: create_default_basis
//===========================================================================
Matrix3i create_default_basis() {
    Matrix3i basis;
    basis.m[0][0] = 4; basis.m[1][0] = 0; basis.m[2][0] = 0;
    basis.m[0][1] = 0; basis.m[1][1] = 4; basis.m[2][1] = 0;
    basis.m[0][2] = 0; basis.m[1][2] = 0; basis.m[2][2] = 4;
    return basis;
}

//===========================================================================
// FUNCTION: make_reduced_filename
//
// After all valid translations have been found,
// the independent translation subgroup is used
// to construct the smallest equivalent chemical cell.
//
// The resulting CIF can be directly used by
// VESTA, spglib, pymatgen or DFT codes.
//===========================================================================
std::string make_reduced_filename(const std::string& input_file) {
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

    return path + "Reduced_" + name + ".cif";
}

//===========================================================================
// MAIN FUNCTION
//===========================================================================
int main(int argc, char* argv[]) {
std::string input_file = "output.cif";

if (argc >= 2) {
    input_file = argv[1];
}

std::string output_file = make_reduced_filename(input_file);
    
    CellData cell = load_cif(input_file);
    if (cell.atoms.empty()) {
        std::cerr << "Error: No atoms loaded from CIF" << std::endl;
        return 1;
    }
    
    std::map<std::string, int> type_map = build_type_map(cell.atoms);
    // lattice[x][y][z]
    //
    // x,y,z:
    //
    //      0 -> 0.00
    //      1 -> 0.25
    //      2 -> 0.50
    //      3 -> 0.75
    //
    // value:
    //
    //      0 = empty
    //      1 = Na
    //      2 = Fe
    //      3 = Mg
    int lattice[GRID_SIZE][GRID_SIZE][GRID_SIZE];
    
    if (!build_lattice(cell, type_map, lattice)) {
        std::cerr << "Error: Cannot build CF32 lattice" << std::endl;
        return 1;
    }
    
    std::vector<Int3> translations = find_translations(lattice);
    
    Matrix3i basis = create_default_basis();
    if (!find_primitive_basis(translations, basis)) {
        std::cerr << "Warning: Cannot construct primitive basis; using original cell" << std::endl;
        basis = create_default_basis();
        translations.clear();
        Int3 zero;
        zero.x = zero.y = zero.z = 0;
        translations.push_back(zero);
    }
    
    std::vector<AtomSite> representatives = find_representative_atoms(cell, translations);
    
    if (!write_reduced_cif(output_file, cell, representatives, translations, basis)) {
        std::cerr << "Error: Cannot write reduced CIF" << std::endl;
        return 1;
    }
    
    print_report(cell, translations, representatives, basis, output_file);
    return 0;
}