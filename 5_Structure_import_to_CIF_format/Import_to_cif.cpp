#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// --- твоя последовательность (32 позиции) ---
std::vector<int> ordering = {
    1,2,2,2,2,3,2,2,
    2,2,2,2,2,2,2,2,
    2,2,3,2,1,2,3,2,
    1,2,2,2,2,2,1,3
};

// соответствие типов
std::string get_symbol(int t) {
    if (t == 1) return "Na";
    if (t == 2) return "Mg";
    if (t == 3) return "Fe";
    return "X";
}

int main() {

    std::ofstream cif("output.cif");

    cif << "data_CF32\n";
    cif << "_symmetry_space_group_name_H-M   'P1'\n";
    cif << "_cell_length_a   8.0\n";
    cif << "_cell_length_b   8.0\n";
    cif << "_cell_length_c   8.0\n";
    cif << "_cell_angle_alpha 90\n";
    cif << "_cell_angle_beta  90\n";
    cif << "_cell_angle_gamma 90\n\n";

    cif << "loop_\n";
    cif << "_atom_site_label\n";
    cif << "_atom_site_type_symbol\n";
    cif << "_atom_site_fract_x\n";
    cif << "_atom_site_fract_y\n";
    cif << "_atom_site_fract_z\n\n";

    // FCC базис для катионов
    std::vector<std::vector<double>> base = {
        {0,0,0},
        {0,0.5,0.5},
        {0.5,0,0.5},
        {0.5,0.5,0}
    };

    int idx = 0;

    // --- КАТИОНЫ (2x2x2 суперячейка) ---
    for (int i = 0; i < 2; i++)
    for (int j = 0; j < 2; j++)
    for (int k = 0; k < 2; k++)
    {
        for (auto &b : base) {

            double x = (b[0] + i) / 2.0;
            double y = (b[1] + j) / 2.0;
            double z = (b[2] + k) / 2.0;

            std::string sym = get_symbol(ordering[idx]);

            cif << "C" << idx+1 << " "
                << sym << " "
                << x << " "
                << y << " "
                << z << "\n";

            idx++;
        }
    }

    // --- АНИОНЫ (сдвиг (0.5,0.5,0.5)) ---
    int o_idx = 1;

    for (int i = 0; i < 2; i++)
    for (int j = 0; j < 2; j++)
    for (int k = 0; k < 2; k++)
    {
        for (auto &b : base) {

            double x = (b[0] + i + 0.5) / 2.0;
            double y = (b[1] + j + 0.5) / 2.0;
            double z = (b[2] + k + 0.5) / 2.0;

            // периодичность
            if (x >= 1) x -= 1;
            if (y >= 1) y -= 1;
            if (z >= 1) z -= 1;

            cif << "O" << o_idx++ << " O "
                << x << " "
                << y << " "
                << z << "\n";
        }
    }

    cif.close();

    std::cout << "CIF file generated: output.cif\n";

    return 0;
}