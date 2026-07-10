**Read this in: [Русский](README_ru.md)**

# Mirror Structure Comparison

## Overview

**Mirror_structure_comparison** compares two CF32s with each other and determines whether they are:

- identical;
- equivalent by translation;
- equivalent by proper rotation;
- mirror-related (enantiomorphic);
- different.

If a mirror pair is detected, the program automatically generates a publication-ready visualization CIF:

`Mirror_paper_<StructureA>_vs_<StructureB>.cif`

## Scientific basis

The comparison is performed on the **4×4×4 CF32 cation lattice**.

The algorithm systematically tests:

- all lattice translations;
- all proper symmetry operations (det = +1);
- all improper symmetry operations (det = −1).

Only the cation sublattice is compared because the oxygen framework is identical for CF32 structures.

## Recommended workflow

Compare structures **after running 5_Structure_import_to_CIF_format**.

should be compared.

```
Original CIF
      ↓
Reduce_cell
      ↓
Reduced CIF
      ↓
Mirror_structure_comparison
```

## In subsequent calculations, we use only the filtered cells and only the left mirrored cell.

## Compilation

```bat
cl Mirror_structure_comparison.cpp /O2 /Oi /Ot /EHsc /Fe:mirror_structure_comparison.exe
```

## Usage

```text
mirror_structure_comparison.exe Reduced_A.cif Reduced_B.cif
```

## Output

The program reports whether the structures are translation-equivalent, rotation-equivalent, mirror/enantiomorphic, or different.

For mirror-related pairs it additionally creates:

`Mirror_paper_<StructureA>_vs_<StructureB>.cif`

## Suggested repository folder

`8_Mirror_structure_comparison`

---