**Read this in: [Русский](README_ru.md)**

# Import_to_cif — Structure import to CIF format

## Purpose
The program generates a CIF file (Crystallographic Information File) based on user-defined data for cations, anions, and their positions in the crystal lattice.

---

## How it works
The program reads from text files:
1. Cation properties (symbol, charge, atomic weight, radius)
2. Anion properties (symbol, charge, atomic weight, radius)
3. Sequence of 32 cations (their codes)
4. Coordinates and CIF parameters for 32 cation sites
5. Coordinates and CIF parameters for 32 anion sites

Based on this data, the program generates a CIF file containing the complete crystal structure.

---

## Input files

### 1. `types_cations.txt` — cation types
File containing the mapping of numeric codes to cation properties.

**Format:**
<code> <symbol> <charge> <atomic_weight>

**Example:**
0 V 0 0
1 Na 1 22.9898
2 Mg 2 24.3050
3 Fe 3 55.8450

**Note:** Code `0` is reserved for vacancies.

---

### 2. `types_anions.txt` — anion types
File containing the mapping of numeric codes to anion properties.

**Format:**
<code> <symbol> <charge> <atomic_weight>

**Example:**
1 O 2 15.999
2 N 3 14.007
3 Cl 1 35.453 

**Note:** Charge is specified as an absolute value (without the minus sign). Only anion number one is taken into account.

---

### 3. `ordering.txt` — cation sequence
File containing 32 numbers (cation codes) that determine the cation type at each position.

**Format:** 32 numbers separated by spaces (can be in one or multiple lines).

**Example (single line):**
1 2 2 2 2 3 2 2 2 2 2 2 2 2 2 2 2 2 3 2 1 2 3 2 1 2 2 2 2 2 1 3

---

### 4. `cation_positions.txt` — cation positions
File containing 32 cation positions with coordinates and CIF parameters.

**Format:**
<label> <x> <y> <z> <occupancy> <multiplicity> <Wyckoff> <H> <calc> <disp> <u_iso>

**Example:**
1  0.5000  0.5000  0.2500  1.000 1 a ? d ? ?
2  0.2500  0.2500  0.2500  1.000 1 a ? d ? ?

---

### 5. `anion_positions.txt` — anion positions
File containing 32 anion positions with coordinates and CIF parameters.

**Format:**
<label> <x> <y> <z> <occupancy> <multiplicity> <Wyckoff> <H> <calc> <disp> <u_iso>

**Example:**
1  0.0000  0.0000  0.0000  1.000 1 a ? d ? ?
2  0.0000  0.2500  0.2500  1.000 1 a ? d ? ?

---

## Output

### `output.cif` — CIF file
Contains the complete crystal structure in CIF format:
- Header with unit cell parameters
- Atom types with charges and radii
- 32 cations (C1...C32) with coordinates and parameters
- 32 anions with coordinates and parameters

---

## Algorithm
1. Load cation properties from `types_cations.txt`
2. Load anion properties from `types_anions.txt`
3. Load sequence of 32 codes from `ordering.txt`
4. Load 32 cation positions from `cation_positions.txt`
5. Load 32 anion positions from `anion_positions.txt`
6. Select anion by code (set in the program)
7. For each cation position:
   - Determine cation code from `ordering.txt`
   - Convert code to element symbol
   - Write to CIF with coordinates from `cation_positions.txt`
8. Write 32 anions with coordinates from `anion_positions.txt`
9. Calculate chemical formula with charge-based sorting and coefficient reduction
10. Save CIF file

---

## Running the program
Import_to_cif.exe
The program automatically loads all files and generates output.cif.

Example

# Input files:

types_cations.txt     # 0 V, 1 Na, 2 Mg, 3 Fe
types_anions.txt      # 1 O, 2 N, 3 Cl
ordering.txt          # 1 2 3 4 1 2 3 4 ...
cation_positions.txt  # 1 0.2500 0.2500 0.2500 ...
anion_positions.txt   # 1 0.0000 0.0000 0.0000 ...

# Output file:

output.cif            # Full structure in CIF format
Anion selection
By default, the program uses the anion with code 1 (first line in types_anions.txt).

To change the anion, edit the following line in Import_to_cif.cpp:

cpp
int anion_code = 1;  // code from types_anions.txt
Features
Vacancies (code 0): automatically skipped when writing to CIF

Cation numbering: original numbering is preserved (C1, C2, C4... when vacancies are present)

Chemical formula: cations sorted by charge (ascending), vacancies shown as [], anion last

Coefficient reduction: automatic reduction to the greatest common divisor

Currently, the program outputs all structures in a cubic primitive unit cell; the mechanism for finding the smallest (primitive) cell has not yet been implemented

Program messages
Full CIF file generated: output.cif — file successfully created

Loaded N cation types — N cation types loaded

Loaded N anion types — N anion types loaded

Loaded 32 cation sites — 32 cation positions loaded

Loaded 32 anion positions — 32 anion positions loaded

Chemical formula: ... — calculated chemical formula

Molecular weight: ... — calculated molecular weight

Error: Cannot open <filename> — failed to open file

Limitations
Number of positions is fixed: 32 cations and 32 anions

All coordinates must be fractional (from 0.0 to 1.0)

Code 0 is reserved for vacancies

---

Compilation - Windows (MSVC)

```cl Import_to_cif.cpp /O2 /Oi /Ot /EHsc /Fe:import_to_cif.exe```