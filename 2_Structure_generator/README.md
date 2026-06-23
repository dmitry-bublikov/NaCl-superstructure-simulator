**Read this in: [Русский](README_ru.md)**

# Program 1: `find-local-balance.exe`

## Purpose
Search for stoichiometric compositions that satisfy the local valence balance condition.

## Input data
Stoichiometry files: `stoichiometry_N.txt` (where `N` is the file number consisting of two digits: the first is cation component, the second is anion component).

## Input parameters
- File number
- Anion charge (0, 1, 2, 3)
- Allowed deviation from local balance (if no deviation - 0)

## Output
- File: `local_balance_N_B_D.txt`
  - `N` - file number (cation/anion component)
  - `B` - local balance (anion charge × 6)
  - `D` - allowed deviation from local balance

## Algorithm
1. Read input file with stoichiometries
2. For each stoichiometry:
   - Determine charges and quantities of cations
   - Iterate through all possible cation combinations across 32 positions
   - Check local balance for each octahedron
   - Check stoichiometric constraints
3. Write found stoichiometries to output file

## Running in cmd or powershell
find-local-balance.exe
The program will prompt:

text
Enter stoichiometry file number (e.g., 71, 41, etc.): 31
Enter charge of anion for local balance (e.g., 0, 1, 2, 3): 2
Enter the deviation from the local balance: 0
Usage example
bash
# Input file: stoichiometry_31.txt
# Output file: local_balance_31_12_0.txt
Program 2: gen-structures.exe
Purpose
Generate all possible superstructure variants for a given stoichiometry.

Input data
Stoichiometry file: for_generator_X_Y_Z.txt (file format is the same as local_balance_31_12_0.txt, but contains a single stoichiometry)

X - stoichiometry number

Y - local balance

Z - allowed deviation

Input parameters
File name in format X_Y_Z

Output
File: local_balance_X_Y_Z.txt

Contains all found cation combinations across 32 positions

Algorithm
Read input file with one stoichiometry

Determine charges and quantities of cations

Full iteration through all cation combinations across 32 positions

Check local balance for 32 octahedra

Check stoichiometric constraints

Write all found variants to output file

Running in cmd or powershell
gen-structures.exe
The program will prompt:

Enter stoichiometry file number (e.g., 31_12_0, etc.): 31_12_0
Usage example
# Input file: for_generator_31_12_0.txt
# Output file: local_balance_31_12_0.txt
Input file structure
Stoichiometry file format
A8+3 B4+2 C2+1 | ...
Where:

A8+3 - cation A with quantity 8 and charge +3

B4+2 - cation B with quantity 4 and charge +2

C2+1 - cation C with quantity 2 and charge +1

File format for gen-structures
text
A8+3 B4+2 C2+1 | ...
Same format, but used for a single specific stoichiometry.

Output data structure
For find-local-balance
text
[original stoichiometry line]
Contains only lines that satisfy local balance.

For gen-structures
z1 z2 z3 z4 z5 z6 z7 z8 z9 z10 z11 z12 z13 z14 z15 z16 z17 z18 z19 z20 z21 z22 z23 z24 z25 z26 z27 z28 z29 z30 z31 z32
Where z1...z32 are cation charges in 32 positions (indices 1-8 correspond to cations A-H and vacancy V).

Complete workflow
Step 1: Generate stoichiometries for different compositions

Step 2: find-local-balance - search for compositions satisfying local balance

Step 3: gen-structures - build superstructure variants

Notes
Both programs use the same local balance checking algorithm

find-local-balance processes multiple stoichiometries, gen-structures processes one

Both programs display execution time and number of found variants

A file with the corresponding number is required for proper operation

The program currently operates with a componentity of 1 for the anion

Compilation - Windows (MSVC)

```cl find-local-balance.cpp /O2 /Oi /Ot /EHsc /Fe:find-local-balance.exe```

```cl gen-structures.cpp /O2 /Oi /Ot /EHsc /Fe:gen-structures.exe```