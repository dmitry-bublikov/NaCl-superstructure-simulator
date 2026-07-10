**Read this in: [Русский](README_ru.md)**

# Cf_g_compat.cpp — Filter for geometrically compatible structures

## Purpose
The program is designed to remove duplicate crystal structures that are **geometrically compatible** (equivalent up to rotations of symmetry axes).

## How it works
The program checks structure equivalence taking into account:
- Rotations around axis 1 (4 variants)
- Rotations around axis 2 (4 variants)
- Rotations around axis 3 (4 variants)
- Inverted variants (reversed axes)

## Algorithm
1. Read input file with structures
2. First structure is automatically saved as unique
3. For each new structure:
   - Check equivalence considering rotations of axis 1
   - Check equivalence considering rotations of axis 2
   - Check equivalence considering rotations of axis 3
4. If the structure is not equivalent to any saved structure — it is added
5. Write all unique structures to output file

## Input
- File: `structuries_X.txt` (where `X` is e.g. `31_12_0`)
- Contains sets of 32 integers (cation charges at positions)

## Output
- File: `structuries_X_f1.txt`
- Contains only unique structures (no duplicates)

## Running the program
```text
Cf_g_compat.exe

The program will prompt:
Enter stoichiometry file number (e.g., 31_12_0, etc.): 31_12_0
```

## Example

**Input file:** structuries_31_12_0.txt
**Output file:** structuries_31_12_0_f1.txt

## Limitations
Maximum number of unique structures: 5000 (defined in #define A 5000)

Supports up to 32 cation positions

## Compilation — Windows (MSVC)

```cl Cf_g_compat.cpp /O2 /Oi /Ot /EHsc /Fe:cf_g_compat.exe```