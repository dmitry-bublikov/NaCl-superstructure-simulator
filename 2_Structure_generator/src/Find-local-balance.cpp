//===========================================================================
// PROGRAM: Optimized Stoichiometry Generator (SORTED OUTPUT)
// Developed by Dmitry Bublikov in 2026
//===========================================================================
// CF32

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>

using namespace std;

inline bool check_balance(int sum, int balance, int delta)
{
    return abs(sum - balance) <= delta;
}

int main(int argc, char **argv)
{

int local_balance=12; // target local balance
int delta=0;          // allowed deviation

// Maximum allowed quantities of cations for each charge (limits)
int max_ch0=0, max_ch1=0, max_ch2=0, max_ch3=0, max_ch4=0, max_ch5=0, max_ch6=0, max_ch7=0;
// Current counts of cations in the tested combination
int count_ch0=0, count_ch1=0, count_ch2=0, count_ch3=0, count_ch4=0, count_ch5=0, count_ch6=0, count_ch7=0;
int zk[10], var_current=0, var_found=0, col=0;
// Cationic positions
int p1=0, p2=0, p3=0, p4=0, p5=0, p6=0, p7=0, p8=0, p9=0, p10=0,
p11=0, p12=0, p13=0, p14=0, p15=0, p16=0, p17=0, p18=0, p19=0, p20=0,
p21=0, p22=0, p23=0, p24=0, p25=0, p26=0, p27=0, p28=0, p29=0, p30=0,
p31=0, p32=0;

// ********************************************** INPUT FILE **************************************
      int file_number;
      char filename[100];
      
      // Ask user for file number
      cout << "Enter stoichiometry file number (e.g., 71, 41, etc.): ";
      cin >> file_number;
      cout << "Enter charge of anion for local balance (e.g., 0, 1, 2, 3): ";
      cin >> local_balance; local_balance *= 6;
      cout << "Enter the deviation from the local balance: ";
      cin >> delta;

      // Form the filename
      sprintf(filename, "stoichiometry_%d.txt", file_number);
      
      // Open the file
      FILE *Fpdat = fopen(filename, "r");
      
      if (Fpdat == NULL) {
          cout << "Error: Cannot open file " << filename << endl;
          return 1;
      }
// ********************************************** OUT FILE ***************************************
      char local_balance_filename[100];
      sprintf(local_balance_filename,
        "local_balance_%d_%d_%d.txt",
        file_number,
        local_balance,
        delta);

      FILE *Fout = fopen(local_balance_filename, "w");
// ************************************************************************************************

// ************************ Program startup time **************************************************
clock_t start_time = clock();
time_t start_time_date = time(nullptr);
struct tm* start_tm = localtime(&start_time_date);
cout << "==============================================" << endl;
cout << "PROGRAM START: ";
printf("%04d-%02d-%02d %02d:%02d:%02d\n",
       start_tm->tm_year + 1900, start_tm->tm_mon + 1, start_tm->tm_mday, start_tm->tm_hour, start_tm->tm_min, start_tm->tm_sec);
cout << "==============================================" << endl;
// ************************************************************************************************

while (!feof(Fpdat)) {

max_ch0=0; max_ch1=0; max_ch2=0; max_ch3=0; max_ch4=0; max_ch5=0; max_ch6=0; max_ch7=0;

// ############################# Read data in required format ##################################
// ############################# Declare arrays instead of separate variables ##################
int ch_c[9] = {0};  // ch_c[1]..ch_c[8] - cation charges
int st_c[9] = {0};  // st_c[1]..st_c[8] - stoichiometry
int num_components = 0;  // actual number of components

// ############################# Read data in required format  #################################
char line[256];
char original_line[256];
int stoh, charge;
int comp_num = 1;  // current component number (1-8)

// Initialize all components with value 100 (unused)
for (int i = 1; i <= 8; i++) {
    ch_c[i] = 100;
    st_c[i] = 100;
}

if (fgets(line, sizeof(line), Fpdat) == NULL) break;

strcpy(original_line, line);  // сохраняем исходную строку

// Trim the string up to the '|' character
for (int k = 0; k < 256; k++) {
    if (line[k] == '|') {
        line[k] = '\0';
        break;
    }
}

// Parse components (A, B, C, D, E, F, G, V)
char *token = strtok(line, " ");

while (token != NULL && comp_num <= 8) {
    // Check if it's a cation (A-G or V)
    if (strlen(token) > 0 && 
        ((token[0] >= 'A' && token[0] <= 'G') || token[0] == 'V')) {
        
        // Parse stoichiometry and charge
        char *sign_pos = strpbrk(token, "+-");
        if (sign_pos != NULL) {
            // Format: A11+2 (stoichiometry and charge together)
            sscanf(token, "%*c%d%*c%d", &stoh, &charge);
        } else {
            // Format: V11 0 (charge as separate token)
            sscanf(token, "%*c%d", &stoh);
            token = strtok(NULL, " ");
            if (token != NULL) {
                sscanf(token, "%d", &charge);
            } else {
                charge = 0;
            }
        }
        
        // Save to arrays
        ch_c[comp_num] = charge;   // charge
        st_c[comp_num] = stoh;     // stoichiometry
        comp_num++;
        num_components++;
    }
    token = strtok(NULL, " ");
}

// Set col equal to the actual number of components
col = num_components;
// ###############################################################################################

// This block becomes very simple:
for (int i = 1; i <= 8; i++) {
    zk[i] = ch_c[i];
    if (ch_c[i] >= 0 && ch_c[i] <= 7) {
        switch (ch_c[i]) {
            case 0: max_ch0 = st_c[i]; break;
            case 1: max_ch1 = st_c[i]; break;
            case 2: max_ch2 = st_c[i]; break;
            case 3: max_ch3 = st_c[i]; break;
            case 4: max_ch4 = st_c[i]; break;
            case 5: max_ch5 = st_c[i]; break;
            case 6: max_ch6 = st_c[i]; break;
            case 7: max_ch7 = st_c[i]; break;
        }
    }
}
var_current++;

for (p1=1; p1<=col; p1++)            // Octahedron 1
for (p2=1; p2<=col; p2++)
for (p3=1; p3<=col; p3++)
for (p4=1; p4<=col; p4++)
for (p5=1; p5<=col; p5++)
for (p6=1; p6<=col; p6++)
{

  if (!check_balance(zk[p1]+zk[p2]+zk[p3]+zk[p4]+zk[p5]+zk[p6], local_balance, delta)) continue;

  count_ch0=0; count_ch1=0; count_ch2=0; count_ch3=0; count_ch4=0; count_ch5=0; count_ch6=0; count_ch7=0;

switch (zk[p1]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p2]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p3]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p4]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p5]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p6]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
            
if ( count_ch0>max_ch0 || count_ch1>max_ch1 || count_ch2>max_ch2 || count_ch3>max_ch3 || count_ch4>max_ch4 || count_ch5>max_ch5 || count_ch6>max_ch6 || count_ch7>max_ch7 ) continue;

for (p7=1; p7<=col; p7++)            // Octahedron 2
for (p8=1; p8<=col; p8++)
for (p18=1; p18<=col; p18++)
for (p19=1; p19<=col; p19++)
{
  if (!check_balance(zk[p3]+zk[p18]+zk[p1]+zk[p7]+zk[p19]+zk[p8], local_balance, delta)) continue;

for (p9=1; p9<=col; p9++)            // Octahedron 3
for (p10=1; p10<=col; p10++)
for (p16=1; p16<=col; p16++)
{
  if (!check_balance(zk[p5]+zk[p16]+zk[p1]+zk[p9]+zk[p19]+zk[p10], local_balance, delta)) continue;

                               {
  count_ch0=0; count_ch1=0; count_ch2=0; count_ch3=0; count_ch4=0; count_ch5=0; count_ch6=0; count_ch7=0;

switch (zk[p1]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p2]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p3]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p4]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p5]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p6]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p7]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p8]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p9]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p10]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p16]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p18]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p19]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}

if ( count_ch0>max_ch0 || count_ch1>max_ch1 || count_ch2>max_ch2 || count_ch3>max_ch3 || count_ch4>max_ch4 || count_ch5>max_ch5 || count_ch6>max_ch6 || count_ch7>max_ch7 ) continue;
                          }

for (p17=1; p17<=col; p17++)         // Octahedron 4
for (p23=1; p23<=col; p23++)
{ 
  if (!check_balance(zk[p2]+zk[p17]+zk[p1]+zk[p10]+zk[p23]+zk[p7], local_balance, delta)) continue;

for (p15=1; p15<=col; p15++)         // Octahedron 5
{ 
  if (!check_balance(zk[p4]+zk[p15]+zk[p1]+zk[p8]+zk[p23]+zk[p9], local_balance, delta) || !check_balance(zk[p1]+zk[p6]+zk[p15]+zk[p16]+zk[p17]+zk[p18], local_balance, delta)) continue;


for (p13=1; p13<=col; p13++)         // Octahedron 6 
for (p14=1; p14<=col; p14++)
for (p21=1; p21<=col; p21++)
{
  if (!check_balance(zk[p3]+zk[p18]+zk[p6]+zk[p14]+zk[p21]+zk[p13], local_balance, delta)) continue;

for (p11=1; p11<=col; p11++)          // Octahedron 7
for (p12=1; p12<=col; p12++)
{
  if (!check_balance(zk[p5]+zk[p16]+zk[p6]+zk[p12]+zk[p21]+zk[p11], local_balance, delta)) continue;


for (p25=1; p25<=col; p25++)          // Octahedron 8
{
  if (!check_balance(zk[p2]+zk[p17]+zk[p6]+zk[p11]+zk[p25]+zk[p14], local_balance, delta) || !check_balance(zk[p4]+zk[p15]+zk[p6]+zk[p13]+zk[p25]+zk[p12], local_balance, delta)) continue;

  count_ch0=0; count_ch1=0; count_ch2=0; count_ch3=0; count_ch4=0; count_ch5=0; count_ch6=0; count_ch7=0;

switch (zk[p1]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p2]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p3]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p4]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p5]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p6]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p7]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p8]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p9]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p10]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p11]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p12]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p13]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p14]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p15]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p16]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p17]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p18]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p19]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p21]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p23]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p25]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}

if ( count_ch0>max_ch0 || count_ch1>max_ch1 || count_ch2>max_ch2 || count_ch3>max_ch3 || count_ch4>max_ch4 || count_ch5>max_ch5 || count_ch6>max_ch6 || count_ch7>max_ch7 ) continue;

for (p20=1; p20<=col; p20++)          // Octahedron 9
for (p26=1; p26<=col; p26++)
{
  if (!check_balance(zk[p7]+zk[p14]+zk[p2]+zk[p26]+zk[p20]+zk[p3], local_balance, delta)) continue;


for (p22=1; p22<=col; p22++)          // Octahedron 10
{

  if (!check_balance(zk[p8]+zk[p13]+zk[p3]+zk[p22]+zk[p26]+zk[p4], local_balance, delta) || !check_balance(zk[p3]+zk[p5]+zk[p19]+zk[p20]+zk[p21]+zk[p22], local_balance, delta)) continue;


for (p24=1; p24<=col; p24++)          // Octahedron 11
{
  if (!check_balance(zk[p2]+zk[p4]+zk[p23]+zk[p24]+zk[p25]+zk[p26], local_balance, delta) || 
      !check_balance(zk[p9]+zk[p12]+zk[p4]+zk[p24]+zk[p22]+zk[p5], local_balance, delta)  || !check_balance(zk[p10]+zk[p11]+zk[p2]+zk[p5]+zk[p20]+zk[p24], local_balance, delta)) continue;


for (p27=1; p27<=col; p27++)          // Octahedron 12
for (p28=1; p28<=col; p28++)
{
  if (!check_balance(zk[p15]+zk[p17]+zk[p23]+zk[p27]+zk[p25]+zk[p28], local_balance, delta)) continue;          


for (p29=1; p29<=col; p29++)           // Octahedron 13
{
  if (!check_balance(zk[p7]+zk[p14]+zk[p17]+zk[p27]+zk[p29]+zk[p18], local_balance, delta) || !check_balance(zk[p10]+zk[p11]+zk[p16]+zk[p29]+zk[p28]+zk[p17], local_balance, delta)) continue;  


for (p30=1; p30<=col; p30++)           // Octahedron 14
{
  if (!check_balance(zk[p16]+zk[p18]+zk[p19]+zk[p29]+zk[p21]+zk[p30], local_balance, delta) || !check_balance(zk[p8]+zk[p13]+zk[p15]+zk[p18]+zk[p30]+zk[p27], local_balance, delta) ||
      !check_balance(zk[p9]+zk[p12]+zk[p15]+zk[p28]+zk[p30]+zk[p16], local_balance, delta)) continue;  

for (p31=1; p31<=col; p31++)           // Octahedron 15
{
  if (!check_balance(zk[p7]+zk[p10]+zk[p19]+zk[p20]+zk[p31]+zk[p29], local_balance, delta) || !check_balance(zk[p7]+zk[p8]+zk[p23]+zk[p26]+zk[p31]+zk[p27], local_balance, delta) ||
      !check_balance(zk[p9]+zk[p10]+zk[p23]+zk[p28]+zk[p31]+zk[p24], local_balance, delta) || !check_balance(zk[p8]+zk[p9]+zk[p19]+zk[p30]+zk[p31]+zk[p22], local_balance, delta)) continue;


for (p32=1; p32<=col; p32++)           // Octahedron 16
{

  count_ch0=0; count_ch1=0; count_ch2=0; count_ch3=0; count_ch4=0; count_ch5=0; count_ch6=0; count_ch7=0;

switch (zk[p1]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p2]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p3]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p4]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p5]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p6]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p7]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p8]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p9]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
            case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p10]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p11]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p12]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p13]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p14]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p15]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p16]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p17]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p18]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p19]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p20]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p21]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p22]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p23]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p24]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p25]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p26]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p27]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p28]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p29]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p30]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p31]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}
switch (zk[p32]){case 0: count_ch0++; break; case 1: count_ch1++; break; case 2: count_ch2++; break; case 3: count_ch3++; break;
             case 4: count_ch4++; break; case 5: count_ch5++; break; case 6: count_ch6++; break; case 7: count_ch7++;}

if ( count_ch0>max_ch0 || count_ch1>max_ch1 || count_ch2>max_ch2 || count_ch3>max_ch3 || count_ch4>max_ch4 || count_ch5>max_ch5 || count_ch6>max_ch6 || count_ch7>max_ch7 ) continue;

  if (!check_balance(zk[p20]+zk[p22]+zk[p24]+zk[p31]+zk[p26]+zk[p32], local_balance, delta) || !check_balance(zk[p12]+zk[p13]+zk[p21]+zk[p22]+zk[p32]+zk[p30], local_balance, delta) ||
      !check_balance(zk[p11]+zk[p14]+zk[p20]+zk[p21]+zk[p29]+zk[p32], local_balance, delta) || !check_balance(zk[p13]+zk[p14]+zk[p25]+zk[p27]+zk[p32]+zk[p26], local_balance, delta) ||
      !check_balance(zk[p11]+zk[p12]+zk[p24]+zk[p32]+zk[p28]+zk[p25], local_balance, delta) || !check_balance(zk[p27]+zk[p28]+zk[p29]+zk[p31]+zk[p30]+zk[p32], local_balance, delta)) continue;

  var_found++;    
  printf("Found variant for local valence balance %d\n", var_current);
  fprintf(Fout, "%s", original_line);
  goto next_line; 

} } } } } } } } } } } } } } } }
next_line:
  continue;
}
fclose(Fpdat);
fclose(Fout);

float tt;
tt = (float)(clock() - start_time) / CLOCKS_PER_SEC;
cout << "Time: " << tt << " seconds" << endl;
cout << "Total found: " << (var_found) << endl;
    return 0;      
}