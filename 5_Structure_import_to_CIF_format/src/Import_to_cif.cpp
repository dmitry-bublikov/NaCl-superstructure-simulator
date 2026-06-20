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
// Description: Greatest common divisor (Euclidean algorithm)
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
// Description: Stores all properties of a cation
//===========================================================================
struct CationProperties {
    std::string symbol;
    int charge;
    double atomic_weight;
    double radius;
};

//===========================================================================
// STRUCT: AnionProperties
// Description: Stores properties of anions (oxygen)
//===========================================================================
struct AnionProperties {
    std::string symbol;
    int charge;
    double atomic_weight;
    double radius;
};

//===========================================================================
// STRUCT: AnionSite
// Description: Stores all CIF parameters for an anion site
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
// FUNCTION: load_anion_properties
// Description: Loads anion properties from file
// File format: "<code> <symbol> <charge> <atomic_weight> <radius>"
// Example: "1 Cl 1 35.453 1.81"  (charge is absolute value, stored as negative)
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
    double radius;
    
    while (file >> code >> symbol >> charge >> atomic_weight >> radius) {
        AnionProperties props;
        props.symbol = symbol;
        props.charge = -charge;  // Store as negative number
        props.atomic_weight = atomic_weight;
        props.radius = radius;
        anions[code] = props;
    }
    
    return anions;
}

//===========================================================================
// FUNCTION: get_anion_symbol
// Description: Returns element symbol for a given anion code
//===========================================================================
std::string get_anion_symbol(int code, const std::map<int, AnionProperties>& anions) {
    std::map<int, AnionProperties>::const_iterator it = anions.find(code);
    if (it != anions.end()) {
        return it->second.symbol;
    }
    return "X";
}

//===========================================================================
// FUNCTION: get_anion_properties_by_code
// Description: Returns anion properties for a given code
//===========================================================================
AnionProperties get_anion_properties_by_code(int code, const std::map<int, AnionProperties>& anions) {
    std::map<int, AnionProperties>::const_iterator it = anions.find(code);
    if (it != anions.end()) {
        return it->second;
    }
    // Return default value
    AnionProperties default_props;
    default_props.symbol = "O";
    default_props.charge = -2;
    default_props.atomic_weight = 15.9990;
    default_props.radius = 0.73;
    return default_props;
}

//===========================================================================
// STRUCT: CationSite
// Description: Stores all CIF parameters for a cation site
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
// Description: Loads cation properties from file
// File format: "<code> <symbol> <charge> <atomic_weight> <radius>"
// Example: "1 Na 1 22.9898 1.02"
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
    double radius;
    
    while (file >> code >> symbol >> charge >> atomic_weight >> radius) {
        CationProperties props;
        props.symbol = symbol;
        props.charge = charge;
        props.atomic_weight = atomic_weight;
        props.radius = radius;
        cations[code] = props;
    }
    
    return cations;
}

//===========================================================================
// FUNCTION: load_anion_sites
// Description: Loads anion sites with all CIF parameters from file
// File format: "<label> <x> <y> <z> <occupancy> <mult> <wyckoff> <H> <calc> <disp> <u_iso>"
// Example: "1 0.0 0.0 0.0 1.000 1 a ? d ? ?"
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
// FUNCTION: load_ordering
// Description: Loads 32 cation type codes from file
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
// Description: Loads cation sites with all CIF parameters from file
// File format: "<label> <x> <y> <z> <occupancy> <mult> <wyckoff> <H> <calc> <disp> <u_iso>"
// Example: "1 0.2500 0.2500 0.2500 1.000 1 a ? d ? ?"
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
// FUNCTION: load_anion_positions
// Description: Loads 32 anion positions with fractional coordinates
//===========================================================================
std::vector<std::tuple<double, double, double>> load_anion_positions(const std::string& filename) {
    std::vector<std::tuple<double, double, double>> positions;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open " << filename << std::endl;
        return positions;
    }
    
    int label;
    double x, y, z;
    while (file >> label >> x >> y >> z) {
        positions.push_back({x, y, z});
    }
    
    if (positions.size() != 32) {
        std::cerr << "Warning: Expected 32 anion positions, got " << positions.size() << std::endl;
    }
    
    return positions;
}

//===========================================================================
// FUNCTION: get_cation_symbol
// Description: Returns element symbol for a given type code
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
// Description: Returns charge for a given type code
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
// Description: Returns atomic weight for a given type code
//===========================================================================
double get_cation_weight(int code, const std::map<int, CationProperties>& cations) {
    std::map<int, CationProperties>::const_iterator it = cations.find(code);
    if (it != cations.end()) {
        return it->second.atomic_weight;
    }
    return 0.0;
}

//===========================================================================
// FUNCTION: get_cation_radius
// Description: Returns radius for a given type code
//===========================================================================
double get_cation_radius(int code, const std::map<int, CationProperties>& cations) {
    std::map<int, CationProperties>::const_iterator it = cations.find(code);
    if (it != cations.end()) {
        return it->second.radius;
    }
    return 0.0;
}

//===========================================================================
// FUNCTION: calculate_formula
// Description: Calculates chemical formula with:
//              1. Cations sorted by charge (ascending: Na⁺, Mg²⁺, Fe³⁺)
//              2. Vacancies [] after cations, before oxygen
//              3. Oxygen (anions) last
//              4. Reduced coefficients (gcd)
//===========================================================================
//===========================================================================
// FUNCTION: calculate_formula
// Description: Calculates chemical formula with anions from file
//===========================================================================
std::string calculate_formula(const std::vector<int>& ordering, 
                              const std::map<int, CationProperties>& cations,
                              const AnionProperties& anion_props,
                              int anion_count) {
    std::map<int, int> charge_counts;
    std::map<int, std::string> charge_symbol;
    int vacancy_count = 0;
    
    // Collect cation data
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
    
    // Add anion (will be last)
    charge_counts[anion_props.charge] = anion_count;
    charge_symbol[anion_props.charge] = anion_props.symbol;
    
    // Add vacancies (will be before anion)
    if (vacancy_count > 0) {
        charge_counts[0] = vacancy_count;
    }
    
    // Sort charges in ascending order
    std::vector<int> charges;
    for (std::map<int, int>::iterator it = charge_counts.begin(); it != charge_counts.end(); ++it) {
        charges.push_back(it->first);
    }
    std::sort(charges.begin(), charges.end());
    
    // Find GCD for reduction
    int g = 0;
    for (std::map<int, int>::iterator it = charge_counts.begin(); it != charge_counts.end(); ++it) {
        if (it->second > 0) {
            g = (g == 0) ? it->second : gcd(g, it->second);
        }
    }
    if (g < 1) g = 1;
    
    // Build formula
    std::string formula;
    
    // 1. Cations (positive charges)
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
    
    // 2. Vacancies
    if (vacancy_count > 0) {
        int count = charge_counts[0] / g;
        if (count > 1) {
            formula += "[]" + std::to_string(count);
        } else {
            formula += "[]";
        }
    }
    
    // 3. Anion (negative charge) — always last
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
// Description: Calculates molecular weight from ordering
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
    // Add anion weight
    weight += anion_count * anion_props.atomic_weight;
    return weight;
}

//===========================================================================
// FUNCTION: get_anion_properties
// Description: Returns properties of oxygen anion
//===========================================================================
AnionProperties get_anion_properties() {
    AnionProperties o;
    o.symbol = "O";
    o.charge = -2;
    o.atomic_weight = 15.9990;
    o.radius = 0.73;
    return o;
}

//===========================================================================
// MAIN FUNCTION
//===========================================================================
int main() {
    //=======================================================================
    // 1. LOAD DATA FROM FILES
    //=======================================================================
    
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
    
    // Load cation sequence
    std::vector<int> ordering = load_ordering("ordering.txt");
    if (ordering.size() != 32) {
        std::cerr << "Error: Invalid ordering file" << std::endl;
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
    
    // Anion code (select the desired anion)
    int anion_code = 1;  // if anion is number 1 from types_anions.txt
    	// Or can ask the user:
    // std::cout << "Enter anion code (1-Cl, 2-O, 3-N): ";
    // std::cin >> anion_code;
    
    AnionProperties anion_props = get_anion_properties_by_code(anion_code, anions);
    
    //=======================================================================
    // 2. GENERATE FULL CIF FILE
    //=======================================================================
    
    std::ofstream cif("output.cif");
    cif << std::fixed << std::setprecision(4);
    
    // --- CIF Header ---
    cif << "data_CF32\n";
    cif << "_audit_creation_method 'Cleaned and symmetry-consistent CIF'\n";
    cif << "_audit_creation_date\t2026-05-04\n";
    cif << "_audit_update_record\t2026-05-04\n";
    
    std::string formula = calculate_formula(ordering, cations, anion_props, 32);
    cif << "_chemical_formula_sum\t'" << formula << "'\n";
    
    double weight = calculate_molecular_weight(ordering, cations, anion_props, 32);
    cif << "_chemical_formula_weight\t" << std::fixed << std::setprecision(3) << weight << "\n";
    
    // Cell parameters
    cif << "_cell_length_a\t8.0000\n";
    cif << "_cell_length_b\t8.0000\n";
    cif << "_cell_length_c\t8.0000\n";
    cif << "_cell_angle_alpha\t90.000\n";
    cif << "_cell_angle_beta\t90.000\n";
    cif << "_cell_angle_gamma\t90.000\n";
    cif << "_cell_volume\t512.0\n";
    
    // Symmetry
    cif << "_symmetry_cell_setting\ttriclinic\n";
    cif << "_symmetry_int_tables_number\t1\n";
    cif << "_symmetry_space_group_name_H-M\t'P 1'\n";
    cif << "_symmetry_space_group_name_Hall\t'P_1'\n\n";
    
    // Symmetry operations
    cif << "loop_\n";
    cif << "_symmetry_equiv_pos_site_id\n";
    cif << "_symmetry_equiv_pos_as_xyz\n";
    cif << "1 x,y,z\n\n";
    
    // Atom types
    std::map<std::string, bool> added_types;
    
    cif << "loop_\n";
    cif << "_atom_type_symbol\n";
    cif << "_atom_type_oxidation_number\n";
    cif << "_atom_type_radius_bond\n";
    
    // Add cation types
    for (int i = 0; i < (int)ordering.size(); i++) {
        int code = ordering[i];
        std::string sym = get_cation_symbol(code, cations);
        if (!added_types[sym]) {
            int charge = get_cation_charge(code, cations);
            double radius = get_cation_radius(code, cations);
            cif << sym << "\t" << charge << "\t" << radius << "\n";
            added_types[sym] = true;
        }
    }
    
    if (!added_types[anion_props.symbol]) {
        cif << anion_props.symbol << "\t" << anion_props.charge << "\t" << anion_props.radius << "\n";
        added_types[anion_props.symbol] = true;
    }
    cif << "\n";
    
    // Atom sites loop
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
    
    // Write cations (skip vacancies - code 0, but keep original numbering)
    for (std::vector<CationSite>::iterator it = cation_sites.begin();
         it != cation_sites.end(); ++it) {
    
        int code = ordering[it->label - 1];
    
        // Check: if code == 0 (vacancy) - skip entry
        if (code == 0) {
            continue;  // Move to next position, but label C remains unchanged
        }
    
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
    
    // Write anions
    for (std::vector<AnionSite>::iterator it = anion_sites.begin();
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
    
    std::cout << "Full CIF file generated: output.cif\n";
    std::cout << "Loaded " << cations.size() << " cation types\n";
    std::cout << "Loaded " << anions.size() << " anion types\n";
    std::cout << "Loaded 32 cation sites\n";
    std::cout << "Loaded 32 anion positions\n";
    std::cout << "Chemical formula: " << formula << "\n";
    std::cout << "Molecular weight: " << std::fixed << std::setprecision(3) << weight << "\n";
    
    return 0;
}