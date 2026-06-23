**Read this in: [Русский](README_ru.md)**

# NaCl-TYPE SUPERSTRUCTURE SIMULATOR

## PURPOSE

Generation of all locally and globally valence-balanced configurations with automatic exclusion of solutions allowing a smaller unit cell.

## Overview

The program consists of several documented modules:

1. **`Stoichiometry_generator`** — generates stoichiometries considering global valence balance for CF32/CF108
2. **`Structure_generator`** — generates structures based on found stoichiometries considering local valence balance
3. **`Filter_g_compatible`** — filters out geometrically compatible structures
4. **`Filter_interpenetrating`** — filters out interpenetrating structures
5. **`Structure_import_to_CIF_format`** — imports structure to CIF format for DIAMOND/VESTA programs

---

## CITATION

This software accompanies the publication:

D. N. Bublikov,
"Combinatorial approach to modeling NaCl-type superstructures",
Crystallography Reports **56**, 89-100 (2011).
DOI: [10.1134/S106377451101007X](https://doi.org/10.1134/S106377451101007X)

If you use this code in your research, please cite both the original article and the version of the software deposited in Zenodo:

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.XXXXXXX.svg)](https://doi.org/10.5281/zenodo.XXXXXXX)

*(Replace `10.5281/zenodo.1234567` with the actual DOI obtained upon deposition in Zenodo).*

---

## SYSTEM REQUIREMENTS

- **Operating System:** Windows 10/11
- **Compiler:** Microsoft Visual Studio 2022 (or newer)
- **RAM:** 4 GB minimum (8 GB recommended for CF108 mode)
- **Disk Space:** ~500 MB for output files

---

## COMPILATION

The program is written in standard C++ and compiled using Microsoft Visual Studio.

**Compilation Environment:**
- Visual Studio 2022 Developer Command Prompt v17.14.25
- Microsoft (R) C/C++ Optimizing Compiler Version 19.44.35222

**Runtime Environment:** CMD/PowerShell (Windows 10/11)

**Example compilation command:**
```cl Stoichiometry_84.cpp /O2 /Oi /Ot /EHsc /Fe:stoichiometry_84.exe```

Compiler flags explanation:

/O2 — Maximum speed optimization

/Oi — Generate intrinsic functions

/Ot — Favor speed over code size

/EHsc — Enable C++ exception handling

/Fe — Specify output executable name

LICENSE AND USAGE
This software is provided for academic and research purposes. When using the program in scientific publications, please cite the original article:

D. N. Bublikov,
"Combinatorial approach to modeling NaCl-type superstructures",
Crystallography Reports 56, 89-100 (2011).
DOI: 10.1134/S106377451101007X

CONTACT
For questions, bug reports, or suggestions, please contact the author:
📧 dmitrybublikov77@gmail.com