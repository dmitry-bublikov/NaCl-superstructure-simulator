**Read this in: [Русский](README_ru.md)**

# Reduce_cell_CF32 — CF32 primitive cell reduction for DFT

## Purpose

`Reduce_cell_CF32` analyzes CF32 NaCl-type superstructure CIF files and searches for the smallest chemically equivalent translation cell.

The program is intended to remove redundant translational repetition before further calculations, especially before DFT optimization.

In other words, it converts a possibly oversized CF32 representation into the smallest primitive chemical cell that preserves the same cation ordering.

---

## Scientific idea

The program uses the fact that CF32 NaCl-derived superstructures are represented on a fixed fractional grid:

```text
0
1/4
1/2
3/4
```

Therefore, all possible hidden translations of the cation ordering must also belong to the same 4×4×4 grid.

The program checks all possible non-zero translations:

```text
4 × 4 × 4 − 1 = 63 translations
```

A translation is accepted only if every cation is mapped onto a cation of the same chemical species.

The oxygen sublattice is not used for the reduction test because all CF32 NaCl-derived structures share the same oxygen framework. The chemical ordering is determined by the cation sublattice.

---

## What the program does

The program:

1. Reads a CF32 CIF file.
2. Extracts all atoms and identifies cations.
3. Builds an integer 4×4×4 cation lattice.
4. Searches all 63 possible non-zero translations.
5. Finds the full translation group preserving the cation coloring.
6. Constructs a primitive chemical cell.
7. Selects representative atoms.
8. Writes a reduced CIF file.

---

## Input

A CIF file containing a CF32 NaCl-derived superstructure.

Example:

```text
phase_6.cif
```

The expected structure contains:

```text
32 cations
32 anions
```

The program can still run if the atom count differs, but it will print a warning.

---

## Running the program

```bat
reduce_cell_cf32.exe phase_6.cif
```

The output file is created automatically in the same directory.

For example:

```text
phase_6.cif
```

becomes

```text
Reduced_phase_6.cif
```

---

## Compilation — Windows (MSVC)

Use the Visual Studio 2022 Developer Command Prompt:

```bat
cl Reduce_cell_CF32.cpp /O2 /Oi /Ot /EHsc /Fe:reduce_cell_cf32.exe
```

Tested with:

```text
Microsoft C/C++ Optimizing Compiler Version 19.44.35222
Visual Studio 2022 Developer Command Prompt v17.14.25
```

---

## Output

The output file contains the reduced primitive chemical cell.

Example:

```text
Reduced_phase_6.cif
```

The output CIF includes:

- new cell parameters;
- reduced cell volume;
- representative atoms only;
- recalculated fractional coordinates;
- chemical formula based on the reduced atom set.

---

## Example report

The program prints a report similar to:

```text
//============================================================
// CF32 CELL REDUCTION REPORT
//============================================================
Input atoms:       64
Input cations:     32
Input anions:      32
Translations:      4
Reduction factor:  4
Output atoms:      16
Output file:       Reduced_phase_6.cif

Translations preserving cation coloring:
  (0 0 0) = (0.00 0.00 0.00)  identity
  (0 2 2) = (0.00 0.50 0.50)
  ...

Primitive basis vectors in original fractional coordinates:
  v1 = (...)
  v2 = (...)
  v3 = (...)

Status: REDUCED BY FACTOR 4
//============================================================
```

---

## Interpretation of the result

The reduced CIF is the **primitive chemical representation** of the cation ordering.

This does not always mean that it is the most visually familiar crystallographic cell.

For example:

- MgO is usually described using a cubic F-centered conventional cell.
- Its primitive cell is rhombohedral and contains fewer atoms.
- Both descriptions are correct.

The purpose of `Reduce_cell_CF32` is to find the smallest primitive chemical cell, not necessarily the conventional crystallographic cell.

---

## Recommended workflow

```text
Generated CF32 CIF
        |
        v
Reduce_cell_CF32
        |
        v
Reduced CIF
        |
        v
DFT relaxation
        |
        v
spglib / crystallographic standardization
        |
        v
comparison with published structures
```

The reduced CIF is recommended as the input for DFT calculations because it contains the minimum number of atoms required to represent the same chemical ordering.

---

## Difference from Standard_cell_CF32

`Reduce_cell_CF32`:

```text
Finds the smallest primitive chemical cell.
```

`Standard_cell_CF32`:

```text
Attempts to build a more conventional crystallographic representation.
```

Both files may describe the same crystal structure, but they are intended for different purposes.

---

## Limitations

This module is designed specifically for CF32 NaCl-derived superstructures.

It assumes:

- fixed 4×4×4 fractional grid;
- cation coordinates are multiples of 1/4;
- NaCl-type anion framework;
- cation ordering is the relevant chemical information.

The current version is not intended for CF108 or larger superstructures.

---

## Notes

The program does not determine the full space group.

It only determines translational equivalence of the cation ordering.

For final crystallographic assignment after DFT relaxation, use specialized tools such as spglib, VESTA, FINDSYM, pymatgen, or other crystallographic software.

---