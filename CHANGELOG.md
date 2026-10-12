# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Uploaded the first version of the program.

## [v1.0.0] - 2026-06-23

### Added
- Modules: 6_Mirror_structure_comparison, 7_Reduce_cell_for_DFT and 8_Standard_cell_for_Crystallography.
- Updated: 4_Filter_interpenetrating (the filtration was incomplete) and 5_Structure_import_to_CIF_format (working with a list)
- Documentation revision and unification of style.
- Fixed the file 5_Structure_import_to_CIF_format/example_files/NaFeO2/phase_3.cif.
- Updated: 1_Stoichiometry_generator/Results.xlsx
- Fixed date for release v1.0.0

## [v1.1.0] - 2026-07-10
### Added
- fix the readme files

## [v2.0.0] - 2026-10-12
### Added
- Module: 0_Parent cells (dentification of parent translational cells for CF32/CF108, which are used by subsequent stoichiometry and structure generation modules)
- Updated 1_Stoichiometry_generator: The stoichiometry generator now includes parent-cell catalogs for CF32/CF108
- Updated 2_Structure_generator: The Find-local-balance.cpp program has been removed (it supported only compositions with one anion component, corresponding to component ratios 11–81). It has been replaced by Find-local-balance-32-32-a.cpp, which is several orders of magnitude faster and supports all anion component combinations.
A similar program, Find-local-balance-32-32-c.cpp, has also been added, but it evaluates local valence balance at cation sites instead of anion sites.
Additionally, Find-local-balance-108-108-a.cpp has been introduced to analyze anion-centered local valence balance for CF108 structures with component ratios ranging from 11 to 81.