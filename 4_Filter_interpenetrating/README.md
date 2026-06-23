**Read this in: [Русский](README_ru.md)**

# Cf_interpen.cpp — Filter for interpenetrating structures

## Purpose
The program is designed to remove duplicate crystal structures that are **interpenetrating** (equivalent up to permutations of positions within octahedra).

## How it works
The program checks structure equivalence taking into account:
- Octahedron rotations (6 variants for each octahedron)
- Inversions (inverted variants)
- Permutations of cation positions within octahedra

## Algorithm
1. Read input file with structures
2. First structure is automatically saved as unique
3. For each new structure:
   - Check equivalence to octahedron 1 (CF32-1)
   - Check equivalence to the remaining 31 octahedra (CF32-2 ... CF32-32)
4. If the structure is not equivalent to any saved structure — it is added
5. Write all unique structures to output file

## Input
- File: `structuries_X_f1.txt` (where `X` is e.g. `31_12_0`)
- Contains sets of 32 integers (cation charges at positions)

## Output
- File: `structuries_X_f2.txt`
- Contains only unique structures (no duplicates)

## Running the program
Cf_interpen.exe
The program will prompt:

Enter stoichiometry file number (e.g., 31_12_0, etc.): 31_12_0
Example
# Input file: structuries_31_12_0_f1.txt
# Output file: structuries_31_12_0_f2.txt
Limitations
Maximum number of unique structures: 5000 (defined in #define A 5000)

Supports up to 32 cation positions

Compilation - Windows (MSVC)

```cl Cf_interpen.cpp /O2 /Oi /Ot /EHsc /Fe:cf_interpen.exe```