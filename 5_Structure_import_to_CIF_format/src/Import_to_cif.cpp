//===========================================================================
// PROGRAM: Import to CIF format (Full CIF Generator)
// Developed by Dmitry Bublikov in 2026
//===========================================================================

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <tuple>
#include <map>
#include <sstream>
#include <iomanip>
#include <algorithm>

//===========================================================================
// FUNCTION: gcd
//===========================================================================
int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

//===========================================================================
// STRUCT: CationProperties
//===========================================================================
struct CationProperties {
    std::string symbol;
    int charge;
    double atomic_weight;
};

//===========================================================================
// STRUCT: AnionProperties
//===========================================================================
struct AnionProperties {
    std::string symbol;
    int charge;
    double atomic_weight;
};

//===========================================================================
// STRUCT: AnionSite
//===========================================================================
struct AnionSite {
    int label;
    double x, y, z;
    double occupancy;
    int multiplicity;
    std::string wyckoff;
    std::string hydrogens;
    std::string calc_flag;
    std::string displace_type;
    std::string u_iso;
};

//===========================================================================
// STRUCT: CationSite
//===========================================================================
struct CationSite {
    int label;
    double x, y, z;
    double occupancy;
    int multiplicity;
    std::string wyckoff;
    std::string hydrogens;
    std::string calc_flag;
    std::string displace_type;
    std::string u_iso;
};

//===========================================================================
// FUNCTION: load_cation_properties
//===========================================================================
std::map<int, CationProperties> load_cation_properties(const std::string& filename) {
    std::map<int, CationProperties> cations;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        return cations;
    }
    
    int code;
    std::string symbol;
    int charge;
    double atomic_weight;
    
    while (file >> code >> symbol >> charge >> atomic_weight) {
        CationProperties props;
        props.symbol = symbol;
        props.charge = charge;
        props.atomic_weight = atomic_weight;
        cations[code] = props;
    }
    
    return cations;
}

//===========================================================================
// FUNCTION: load_anion_properties
//===========================================================================
std::map<int, AnionProperties> load_anion_properties(const std::string& filename) {
    std::map<int, AnionProperties> anions;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        return anions;
    }
    
    int code;
    std::string symbol;
    int charge;
    double atomic_weight;
    
    while (file >> code >> symbol >> charge >> atomic_weight) {
        AnionProperties props;
        props.symbol = symbol;
        props.charge = -charge;
        props.atomic_weight = atomic_weight;
        anions[code] = props;
    }
    
    return anions;
}

//===========================================================================
// FUNCTION: get_anion_properties_by_code
//===========================================================================
AnionProperties get_anion_properties_by_code(int code, const std::map<int, AnionProperties>& anions) {
    std::map<int, AnionProperties>::const_iterator it = anions.find(code);
    if (it != anions.end()) {
        return it->second;
    }
    AnionProperties default_props;
    default_props.symbol = "O";
    default_props.charge = -2;
    default_props.atomic_weight = 15.9990;
    return default_props;
}

//===========================================================================
// FUNCTION: load_ordering
//===========================================================================
std::vector<int> load_ordering(const std::string& filename) {
    std::vector<int> ordering;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        return ordering;
    }
    
    int value;
    while (file >> value) {
        ordering.push_back(value);
    }
    
    if (ordering.size() != 32) {
        std::cerr << "Warning: Expected 32 values, got " << ordering.size() << std::endl;
    }
    
    return ordering;
}

//===========================================================================
// FUNCTION: load_cation_sites
//===========================================================================
std::vector<CationSite> load_cation_sites(const std::string& filename) {
    std::vector<CationSite> sites;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        return sites;
    }
    
    CationSite site;
    while (file >> site.label 
           >> site.x >> site.y >> site.z 
           >> site.occupancy >> site.multiplicity 
           >> site.wyckoff >> site.hydrogens 
           >> site.calc_flag >> site.displace_type 
           >> site.u_iso) {
        sites.push_back(site);
    }
    
    if (sites.size() != 32) {
        std::cerr << "Warning: Expected 32 sites, got " << sites.size() << std::endl;
    }
    
    return sites;
}

//===========================================================================
// FUNCTION: load_anion_sites
//===========================================================================
std::vector<AnionSite> load_anion_sites(const std::string& filename) {
    std::vector<AnionSite> sites;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        return sites;
    }
    
    AnionSite site;
    while (file >> site.label 
           >> site.x >> site.y >> site.z 
           >> site.occupancy >> site.multiplicity 
           >> site.wyckoff >> site.hydrogens 
           >> site.calc_flag >> site.displace_type 
           >> site.u_iso) {
        sites.push_back(site);
    }
    
    if (sites.size() != 32) {
        std::cerr << "Warning: Expected 32 anion sites, got " << sites.size() << std::endl;
    }
    
    return sites;
}

//===========================================================================
// FUNCTION: get_cation_symbol
//===========================================================================
std::string get_cation_symbol(int code, const std::map<int, CationProperties>& cations) {
    std::map<int, CationProperties>::const_iterator it = cations.find(code);
    if (it != cations.end()) {
        return it->second.symbol;
    }
    return "X";
}

//===========================================================================
// FUNCTION: get_cation_charge
//===========================================================================
int get_cation_charge(int code, const std::map<int, CationProperties>& cations) {
    std::map<int, CationProperties>::const_iterator it = cations.find(code);
    if (it != cations.end()) {
        return it->second.charge;
    }
    return 0;
}

//===========================================================================
// FUNCTION: get_cation_weight
//===========================================================================
double get_cation_weight(int code, const std::map<int, CationProperties>& cations) {
    std::map<int, CationProperties>::const_iterator it = cations.find(code);
    if (it != cations.end()) {
        return it->second.atomic_weight;
    }
    return 0.0;
}

//===========================================================================
// FUNCTION: calculate_formula
//===========================================================================
std::string calculate_formula(const std::vector<int>& ordering, 
                              const std::map<int, CationProperties>& cations,
                              const AnionProperties& anion_props,
                              int anion_count) {
    std::map<int, int> charge_counts;
    std::map<int, std::string> charge_symbol;
    int vacancy_count = 0;
    
    for (int i = 0; i < (int)ordering.size(); i++) {
        int code = ordering[i];
        if (code == 0) {
            vacancy_count++;
        } else {
            std::string sym = get_cation_symbol(code, cations);
            int charge = get_cation_charge(code, cations);
            if (sym != "X" && sym != "V") {
                charge_counts[charge]++;
                charge_symbol[charge] = sym;
            }
        }
    }
    
    charge_counts[anion_props.charge] = anion_count;
    charge_symbol[anion_props.charge] = anion_props.symbol;
    
    if (vacancy_count > 0) {
        charge_counts[0] = vacancy_count;
    }
    
    std::vector<int> charges;
    for (std::map<int, int>::iterator it = charge_counts.begin(); it != charge_counts.end(); ++it) {
        charges.push_back(it->first);
    }
    std::sort(charges.begin(), charges.end());
    
    int g = 0;
    for (std::map<int, int>::iterator it = charge_counts.begin(); it != charge_counts.end(); ++it) {
        if (it->second > 0) {
            g = (g == 0) ? it->second : gcd(g, it->second);
        }
    }
    if (g < 1) g = 1;
    
    std::string formula;
    
    for (std::vector<int>::iterator it = charges.begin(); it != charges.end(); ++it) {
        int charge = *it;
        if (charge > 0) {
            std::string sym = charge_symbol[charge];
            int count = charge_counts[charge] / g;
            if (count > 1) {
                formula += sym + std::to_string(count);
            } else {
                formula += sym;
            }
        }
    }
    
    if (vacancy_count > 0) {
        int count = charge_counts[0] / g;
        if (count > 1) {
            formula += "[]" + std::to_string(count);
        } else {
            formula += "[]";
        }
    }
    
    for (std::vector<int>::iterator it = charges.begin(); it != charges.end(); ++it) {
        int charge = *it;
        if (charge < 0) {
            int count = charge_counts[charge] / g;
            std::string sym = charge_symbol[charge];
            if (count > 1) {
                formula += sym + std::to_string(count);
            } else {
                formula += sym;
            }
            break;
        }
    }
    
    return formula;
}

//===========================================================================
// FUNCTION: calculate_molecular_weight
//===========================================================================
double calculate_molecular_weight(const std::vector<int>& ordering,
                                  const std::map<int, CationProperties>& cations,
                                  const AnionProperties& anion_props,
                                  int anion_count) {
    double weight = 0;
    for (int i = 0; i < (int)ordering.size(); i++) {
        int code = ordering[i];
        weight += get_cation_weight(code, cations);
    }
    weight += anion_count * anion_props.atomic_weight;
    return weight;
}

//===========================================================================
// FUNCTION: parse_ordering_line
//===========================================================================
std::vector<int> parse_ordering_line(const std::string& line) {
    std::vector<int> ordering;
    std::stringstream ss(line);

    int value;
    while (ss >> value) {
        ordering.push_back(value);
    }

    return ordering;
}

//===========================================================================
// FUNCTION: generate_cif
//===========================================================================
bool generate_cif(const std::vector<int>& ordering,
                  const std::map<int, CationProperties>& cations,
                  const std::map<int, AnionProperties>& anions,
                  const std::vector<CationSite>& cation_sites,
                  const std::vector<AnionSite>& anion_sites,
                  const std::string& output_filename,
                  int phase_number) {
    if (ordering.size() != 32) {
        std::cerr << "Error: Invalid ordering for phase " << phase_number
                  << " (expected 32 values, got " << ordering.size() << ")" << std::endl;
        return false;
    }

    // Anion code
    int anion_code = 1;
    AnionProperties anion_props = get_anion_properties_by_code(anion_code, anions);

    // Generate CIF
    std::ofstream cif(output_filename.c_str());
    if (!cif.is_open()) {
        std::cerr << "Error: Cannot create " << output_filename << std::endl;
        return false;
    }

    cif << std::fixed << std::setprecision(4);

    cif << "data_CF32_phase_" << phase_number << "\n";
    cif << "_audit_creation_method 'Cleaned and symmetry-consistent CIF'\n";
    cif << "_audit_creation_date\t2026-05-04\n";
    cif << "_audit_update_record\t2026-05-04\n";

    std::string formula = calculate_formula(ordering, cations, anion_props, 32);
    cif << "_chemical_formula_sum\t'" << formula << "'\n";

    double weight = calculate_molecular_weight(ordering, cations, anion_props, 32);
    cif << "_chemical_formula_weight\t" << std::fixed << std::setprecision(3) << weight << "\n";

    cif << "_cell_length_a\t8.0000\n";
    cif << "_cell_length_b\t8.0000\n";
    cif << "_cell_length_c\t8.0000\n";
    cif << "_cell_angle_alpha\t90.000\n";
    cif << "_cell_angle_beta\t90.000\n";
    cif << "_cell_angle_gamma\t90.000\n";
    cif << "_cell_volume\t512.0\n";

    cif << "_symmetry_cell_setting\ttriclinic\n";
    cif << "_symmetry_int_tables_number\t1\n";
    cif << "_symmetry_space_group_name_H-M\t'P 1'\n";
    cif << "_symmetry_space_group_name_Hall\t'P_1'\n\n";

    cif << "loop_\n";
    cif << "_symmetry_equiv_pos_site_id\n";
    cif << "_symmetry_equiv_pos_as_xyz\n";
    cif << "1 x,y,z\n\n";

    std::map<std::string, bool> added_types;

    cif << "loop_\n";
    cif << "_atom_type_symbol\n";
    cif << "_atom_type_oxidation_number\n";
    cif << "_atom_type_radius_bond\n";

    for (int i = 0; i < (int)ordering.size(); i++) {
        int code = ordering[i];
        if (code == 0) continue;

        std::string sym = get_cation_symbol(code, cations);
        if (!added_types[sym]) {
            int charge = get_cation_charge(code, cations);
            cif << sym << "\t" << charge << "\t?\n";
            added_types[sym] = true;
        }
    }

    if (!added_types[anion_props.symbol]) {
        cif << anion_props.symbol << "\t" << anion_props.charge << "\t?\n";
        added_types[anion_props.symbol] = true;
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

    for (std::vector<CationSite>::const_iterator it = cation_sites.begin();
         it != cation_sites.end(); ++it) {
        int index = it->label - 1;
        if (index < 0 || index >= (int)ordering.size()) {
            std::cerr << "Warning: Invalid cation site label " << it->label
                      << " in phase " << phase_number << std::endl;
            continue;
        }

        int code = ordering[index];
        if (code == 0) continue;

        std::string sym = get_cation_symbol(code, cations);
        cif << "C" << it->label << " "
            << sym << "  "
            << std::fixed << std::setprecision(4) << it->x << "  "
            << it->y << "  "
            << it->z << "  "
            << std::fixed << std::setprecision(3) << it->occupancy << " "
            << it->multiplicity << " "
            << it->wyckoff << " "
            << it->hydrogens << " "
            << it->calc_flag << " "
            << it->displace_type << " "
            << it->u_iso << "\n";
    }

    for (std::vector<AnionSite>::const_iterator it = anion_sites.begin();
         it != anion_sites.end(); ++it) {
        cif << anion_props.symbol << it->label << " "
            << anion_props.symbol << "  "
            << std::fixed << std::setprecision(4) << it->x << "  "
            << it->y << "  "
            << it->z << "  "
            << std::fixed << std::setprecision(3) << it->occupancy << " "
            << it->multiplicity << " "
            << it->wyckoff << " "
            << it->hydrogens << " "
            << it->calc_flag << " "
            << it->displace_type << " "
            << it->u_iso << "\n";
    }

    cif.close();

    std::cout << "Generated: " << output_filename << "\n";
    std::cout << "Chemical formula: " << formula << "\n";
    std::cout << "Molecular weight: " << std::fixed << std::setprecision(3) << weight << "\n\n";

    return true;
}

//===========================================================================
// MAIN FUNCTION
//===========================================================================
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " input_file.txt" << std::endl;
        std::cerr << "Each line of input_file.txt must contain 32 integer values." << std::endl;
        return 1;
    }

    std::string input_filename = argv[1];

    // Load cations
    std::map<int, CationProperties> cations = load_cation_properties("types_cations.txt");
    if (cations.empty()) {
        std::cerr << "Error: No cation types loaded" << std::endl;
        return 1;
    }

    // Load anions
    std::map<int, AnionProperties> anions = load_anion_properties("types_anions.txt");
    if (anions.empty()) {
        std::cerr << "Error: No anion types loaded" << std::endl;
        return 1;
    }

    // Load cation positions
    std::vector<CationSite> cation_sites = load_cation_sites("cation_positions.txt");
    if (cation_sites.size() != 32) {
        std::cerr << "Error: Invalid cation sites file (expected 32 sites)" << std::endl;
        return 1;
    }

    // Load anion positions
    std::vector<AnionSite> anion_sites = load_anion_sites("anion_positions.txt");
    if (anion_sites.size() != 32) {
        std::cerr << "Error: Invalid anion sites file (expected 32 sites)" << std::endl;
        return 1;
    }

    std::ifstream input_file(input_filename.c_str());
    if (!input_file.is_open()) {
        std::cerr << "Error: Cannot open " << input_filename << std::endl;
        return 1;
    }

    std::string line;
    int line_number = 0;
    int generated_count = 0;

    while (std::getline(input_file, line)) {
        line_number++;

        if (line.empty()) {
            std::cerr << "Warning: Line " << line_number << " is empty. Skipped." << std::endl;
            continue;
        }

        std::vector<int> ordering = parse_ordering_line(line);
        if (ordering.size() != 32) {
            std::cerr << "Warning: Line " << line_number
                      << " skipped. Expected 32 values, got "
                      << ordering.size() << "." << std::endl;
            continue;
        }

        std::string output_filename = "phase_" + std::to_string(line_number) + ".cif";

        if (generate_cif(ordering, cations, anions, cation_sites, anion_sites,
                         output_filename, line_number)) {
            generated_count++;
        }
    }

    input_file.close();

    std::cout << "Loaded " << cations.size() << " cation types\n";
    std::cout << "Loaded " << anions.size() << " anion types\n";
    std::cout << "Loaded 32 cation sites\n";
    std::cout << "Loaded 32 anion positions\n";
    std::cout << "Generated " << generated_count << " CIF files\n";

    return 0;
}
