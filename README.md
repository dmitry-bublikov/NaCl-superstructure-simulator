**Read this in: [Русский](README_ru.md)**

# NaCl-type Superstructure Simulator

## Purpose

The program implements a combinatorial approach to modeling NaCl-type
superstructures. The algorithm is based on enumerating all possible
occupancy variants of the cation sublattice in a cubic fragment (CF32/CF108),
followed by verification of global electroneutrality and local valence
balance (Pauling's rule). As a result, stoichiometric compositions and
superstructure models are generated, which can be compared with real
complex oxides.

## Overview

The program consists of several documented modules:

1. **`Stoichiometry_generator`** — generates stoichiometries considering global valence balance for CF32/CF108
2. **`Structure_generator`** — generates structures based on found stoichiometries considering local valence balance
3. **`Filter_g_compatible`** — filters out geometrically compatible structures
4. **`Filter_interpenetrating`** — filters out interpenetrating structures
5. **`Structure_import_to_CIF_format`** — imports structure to CIF format for DIAMOND/VESTA programs
6. **`Reduce_cell_for_DFT`** — CF32 primitive cell reduction for DFT
7. **`Standard_cell_for_Crystallography`** — standard crystallographic cell builder

---

## Citation

> [!IMPORTANT]
> If you use this software in scientific research, please cite both the original publication and the Zenodo software release.

This software accompanies the publication:

D. N. Bublikov,
"Combinatorial approach to modeling NaCl-type superstructures",
Crystallography Reports **56**, 89-100 (2011).
DOI: [10.1134/S106377451101007X](https://doi.org/10.1134/S106377451101007X)

If you use this code in your research, please cite both the original article and the version of the software deposited in Zenodo:

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.20820466.svg)](https://doi.org/10.5281/zenodo.20820466)

---

## System Requirements

- **Operating System:** Windows 10/11
- **Compiler:** Microsoft Visual Studio 2022 (or newer)
- **RAM:** 4 GB minimum (8 GB recommended for CF108 mode)
- **Disk Space:** ~500 MB for output files

---

## Compilation

The program is written in standard C++ and compiled using Microsoft Visual Studio.

**Compilation Environment:**
- Visual Studio 2022 Developer Command Prompt v17.14.25
- Microsoft (R) C/C++ Optimizing Compiler Version 19.44.35222

**Runtime Environment:** CMD/PowerShell (Windows 10/11)

**Example compilation command:**
```cl Stoichiometry_84.cpp /O2 /Oi /Ot /EHsc /Fe:stoichiometry_84.exe```

**Compiler flags explanation:**

- `/O2` — Maximum speed optimization

- `/Oi` — Generate intrinsic functions

- `/Ot` — Favor speed over code size

- `/EHsc` — Enable C++ exception handling

- `/Fe` — Specify output executable name

## License and Usage
This software is provided for academic and research purposes. When using the program in scientific publications, please cite the original article:

D. N. Bublikov,
"Combinatorial approach to modeling NaCl-type superstructures",
Crystallography Reports 56, 89-100 (2011).
DOI: [10.1134/S106377451101007X](https://doi.org/10.1134/S106377451101007X)

## Contact

> [!TIP]
> Questions, bug reports and suggestions are welcome.
>
> 📧 **dmitrybublikov77@gmail.com**