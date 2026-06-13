//===========================================================================
// PROGRAM: Optimized Stoichiometry Generator (SORTED OUTPUT)
// Developed by Dmitry Bublikov in 2026
//===========================================================================
// stoichiometry_22

#include <ctime>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

using namespace std;

const int COMP_C = 2;         // number of cation components (including vacancy)
const int COMP_A = 2;         // number of anion components (including vacancy)

vector<int> select_charges(const char* type, int max_charge) {
    vector<int> selected;
    
    cout << "\n=== " << type << " selection ===" << endl;
    cout << "Available charges: ";
    for (int i = 0; i <= max_charge; i++) {
        cout << i << " ";
    }
    cout << endl;
    
    cout << "Enter charges to include (0-" << max_charge << ", separated by space, end with -1):" << endl;
    int ch;
    while (cin >> ch && ch != -1) {
        if (ch >= 0 && ch <= max_charge) {
            selected.push_back(ch);
        } else {
            cout << "Invalid charge: " << ch << ", ignored" << endl;
        }
    }
    
    // Sort the selected charges
    sort(selected.begin(), selected.end());
    
    cout << "Selected " << type << " charges: ";
    for (int ch : selected) cout << ch << " ";
    cout << endl;
    
    return selected;
}

int main(int argc, char **argv)
{
    int st_c1, st_c2;     // stoichiometry for each cation
    uint8_t ch_c1, ch_c2; // cation charges
    int st_a1, st_a2;      // stoichiometry for each anion
    uint8_t ch_a1, ch_a2;  // anion charges
    long long var = 0;  // total number of stoichiometries
    int cf, choice, write_file=0, select_cation=0, select_anion=0;

// **************************************** Input parameters ****************************************
    cout << "Enter your choice (1 - CF32 or 2 - CF108): ";
    cin >> choice;
    if (choice == 1) { cf = 32; cout << "Selected: CF32 (32 sites)" << endl; } 
    else if (choice == 2) { cf = 108; cout << "Selected: CF108 (108 sites)" << endl; } 
    else {
        cout << "ERROR: Invalid choice! Please enter 1 or 2." << endl;
        cout << "Press Enter to exit..." << endl;
        cin.get();
        cin.get();
        return 1;
    }

    vector<int> selected_cations;
    cout << "Enter your choice (1 - Generate all cations or 2 - Selected cations): ";
    cin >> choice;
    if (choice == 1) { select_cation = 1; cout << "Generate all cations" << endl; } 
    else if (choice == 2) { select_cation = 2; cout << "Selected cations" << endl; 
    selected_cations = select_charges("CATIONS", 7); } 
    else {
        cout << "ERROR: Invalid choice! Please enter 1 or 2." << endl;
        cout << "Press Enter to exit..." << endl;
        cin.get();
        cin.get();
        return 1;
    }

    vector<int> selected_anions;
    cout << "Enter your choice (1 - Generate all anions or 2 - Selected anions): ";
    cin >> choice;
    if (choice == 1) { select_anion = 1; cout << "Generate all anions" << endl; } 
    else if (choice == 2) { select_anion = 2; cout << "Selected anions" << endl; 
    selected_anions = select_charges("ANIONS", 3); } 
    else {
        cout << "ERROR: Invalid choice! Please enter 1 or 2." << endl;
        cout << "Press Enter to exit..." << endl;
        cin.get();
        cin.get();
        return 1;
    }

    cout << "Enter your choice (1 - Do not save to file, or 2 - Save to file): ";
    cin >> choice;
    if (choice == 1) { write_file=1; cout << "Selected: Do not save to file" << endl; } 
    else if (choice == 2) { write_file=2; cout << "Selected: Save to file" << endl; } 
    else {
        cout << "ERROR: Invalid choice! Please enter 1 or 2." << endl;
        cout << "Press Enter to exit..." << endl;
        cin.get();
        cin.get();
        return 1;
    }

clock_t start_time = clock();
// ************************ Program startup time ***************************
time_t start_time_date = time(nullptr);
struct tm* start_tm = localtime(&start_time_date);
cout << "==============================================" << endl;
cout << "PROGRAM START: ";
printf("%04d-%02d-%02d %02d:%02d:%02d\n",
       start_tm->tm_year + 1900, start_tm->tm_mon + 1, start_tm->tm_mday, start_tm->tm_hour, start_tm->tm_min, start_tm->tm_sec);
cout << "==============================================" << endl;
// **************************************************************************************************

    // CREATE THE ARRAY need_cation_charge
    bool need_cation_charge[8] = {false};
    for (int ch : selected_cations) {
        if (ch >= 0 && ch <= 7) need_cation_charge[ch] = true;
    }

    // CREATE THE ARRAY need_anion_charge
     bool need_anion_charge[4] = {false};
     for (int ch : selected_anions) {
         if (ch >= 0 && ch <= 3) need_anion_charge[ch] = true;
    }

    int diff_c=cf - (COMP_C - 1), diff_a=cf - (COMP_A - 1), max_ch_c=(cf-1)*3+2;
    static uint8_t charges_c[40320][COMP_C];  // 8!=40320 7 different cations + 1 vacantion
    static uint8_t charges_a[24][COMP_A];    // 4!=24 for 3 different anions + 1 vacantion
    FILE* fp;
    string filename = "stoichiometry_" + to_string(COMP_C) + to_string(COMP_A) + ".txt";
    fp = fopen(filename.c_str(), "w");
    
    // *** All values ​​for the current calculation system (charges for cations!!!) ***
    int charge_count_c = 0;
    int all_charges_c[] = {0, 1, 2, 3, 4, 5, 6, 7};
    int combo_c[COMP_C];
    for (int skip1 = 0; skip1 < 8; skip1++)
    for (int skip2 = skip1 + 1; skip2 < 8; skip2++)
    for (int skip3 = skip2 + 1; skip3 < 8; skip3++)
    for (int skip4 = skip3 + 1; skip4 < 8; skip4++)
    for (int skip5 = skip4 + 1; skip5 < 8; skip5++)
    for (int skip6 = skip5 + 1; skip6 < 8; skip6++) {
        int idx = 0;
        for (int i = 0; i < 8; i++)
            if (i != skip1 && i != skip2 && i != skip3 && i != skip4 && i != skip5 && i != skip6)
               combo_c[idx++] = all_charges_c[i];
        do {
            for (int k = 0; k < COMP_C; k++)
                charges_c[charge_count_c][k] = combo_c[k];
            charge_count_c++;
        } while (next_permutation(combo_c, combo_c + COMP_C));
    }
    // ********************************************************************************

    // *** All values ​​for the current calculation system (charges for anions!!!) ***
    int charge_count_a = 0;
    int all_charges_a[] = {0, 1, 2, 3};
    int combo_a[COMP_A];
    for (int skip1 = 0; skip1 < 4; skip1++)
    for (int skip2 = skip1 + 1; skip2 < 4; skip2++) {
        int idx = 0;
        for (int i = 0; i < 4; i++)
            if (i != skip1 && i != skip2)
                combo_a[idx++] = all_charges_a[i];
        do {
            for (int k = 0; k < COMP_A; k++)
                charges_a[charge_count_a][k] = combo_a[k];
            charge_count_a++;
        } while (next_permutation(combo_a, combo_a + COMP_A));
    }
    // *******************************************************************************
    
    // ******************* Generation of stoichiometries for cations ***********************
      for (st_c1 = 1; st_c1 <= diff_c; st_c1++)
    {
          double seconds = (double)(clock() - start_time) / CLOCKS_PER_SEC;
          int total_seconds = (int)seconds;
          int hours = total_seconds / 3600;
          int minutes = (total_seconds % 3600) / 60;
          int secs = total_seconds % 60;
        printf("Processing st_c1 = %d from %d var = %lld  [%02d:%02d:%02d]\n", st_c1, diff_c, var, hours, minutes, secs);

            st_c2 = cf - st_c1;
            if (st_c2 < 1 || st_c2 > st_c1) continue;
            
            for (int c = 0; c < charge_count_c; c++)
            {
                ch_c1 = charges_c[c][0];
                ch_c2 = charges_c[c][1];

                // less charge for equal quantities
                if (st_c1 == st_c2 && ch_c1 > ch_c2) continue;

                // For selected cations
                if (select_cation==2) {
                bool all_valid_c = true;
                int charges_c[] = {ch_c1, ch_c2};
                for (int i = 0; i < COMP_C; i++) {
                    if (!need_cation_charge[charges_c[i]]) {
                        all_valid_c = false;
                        break;
                        }
                    }
                if (!all_valid_c) continue;
                }

                // total charge for cations
                 int sum_c = st_c1*ch_c1 + st_c2*ch_c2;
                 if (sum_c > max_ch_c) continue;

    // ******************* Generation of stoichiometries for anions ***********************
    for (st_a1 = 1; st_a1 <= diff_a; st_a1++)
    {

            st_a2 = cf - st_a1;
            if (st_a2 < 1 || st_a2 > st_a1) continue;
            
            for (int c = 0; c < charge_count_a; c++)
            {
                ch_a1 = charges_a[c][0];
                ch_a2 = charges_a[c][1];

                // less charge for equal quantities
                if (st_a1 == st_a2 && ch_a1 > ch_a2) continue;

                // For selected anions
                if (select_anion == 2) {
                bool all_valid_a = true;
                int charges_a[] = {ch_a1, ch_a2};
                for (int i = 0; i < COMP_A; i++) {
                    if (!need_anion_charge[charges_a[i]]) {
                        all_valid_a = false;
                        break;
                        }
                    }
                if (!all_valid_a) continue;
                }                

                // total charge for anions
                 int sum_a = st_a1*ch_a1 + st_a2*ch_a2;

                 if (sum_a == sum_c)
                  {  
                     var++;
// **************************************    Save date to file   ************************************
if (write_file == 2)
{
    // Step 1: Collect data into the charge_to_count_c array directly
int charge_to_count_c[8] = {0};  // indices 0..7
charge_to_count_c[ch_c1] = st_c1;
charge_to_count_c[ch_c2] = st_c2;

    // Step 2: Create vector of present charges
    vector<pair<int, int>> present_charges_c;
    for (int charge = 1; charge <= 7; charge++) {
        if (charge_to_count_c[charge] > 0) {
            present_charges_c.push_back({charge, charge_to_count_c[charge]});
        }
    }
    sort(present_charges_c.begin(), present_charges_c.end());
    // Output with fixed symbols A, B, C, D, E, F, G
    char fixed_symbols_c[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G'};
    for (size_t i = 0; i < present_charges_c.size() && i < COMP_C; i++) {
        fprintf(fp, "%c%d+%d ", fixed_symbols_c[i], present_charges_c[i].second, present_charges_c[i].first);
    }

// Step 3: Output the vacancy (charge 0) at the end
if (charge_to_count_c[0] > 0) {
    fprintf(fp, "V%d 0 ", charge_to_count_c[0]);
}
fprintf(fp, "| ");

// Step 4: Collect data into the charge_to_count_a array directly
int charge_to_count_a[4] = {0};  // indices 0..3
charge_to_count_a[ch_a1] = st_a1;
charge_to_count_a[ch_a2] = st_a2;

    // Step 5: Collect all present charges (1..3) and sort them in DESCENDING order
    vector<pair<int, int>> present_charges_a; // (charge, count)
    for (int charge = 1; charge <= 3; charge++) {
        if (charge_to_count_a[charge] > 0) {
            present_charges_a.push_back({charge, charge_to_count_a[charge]});
        }
    }
    // Sort by charge in DESCENDING order (3,2,1)
    sort(present_charges_a.begin(), present_charges_a.end(), greater<pair<int, int>>());
    
    // Step 6: Output with fixed symbols X, Y, Z
    char fixed_symbols_a[] = {'X', 'Y', 'Z'};
    for (size_t i = 0; i < present_charges_a.size() && i < COMP_A; i++) {
        fprintf(fp, "%c%d-%d ", fixed_symbols_a[i], present_charges_a[i].second, present_charges_a[i].first);
    }

    // Step 7: Output the vacancy (charge 0) at the end
    if (charge_to_count_a[0] > 0) {
        fprintf(fp, "V%d 0 ", charge_to_count_a[0], sum_a, var);
    }
fprintf(fp, "ch=%d %lld\n", sum_a, var);
}
// **************************************    End save date to file   ********************************                   
                            }
                        }  // end for (int ca...)
                }  // end for (st_a1...)
                // ==================== END OF ANION CYCLES ====================
            }  // end for (int c...)
    }  // end for (st_c1...)
    // ==================== END OF CATION CYCLES ====================
    
    fclose(fp);
    
    float tt;
    tt = (float)(clock() - start_time) / CLOCKS_PER_SEC;
    cout << "Time: " << tt << " seconds" << endl;
    cout << "Total found: " << (var) << endl;
    
    cin.get();
    return 0;
}