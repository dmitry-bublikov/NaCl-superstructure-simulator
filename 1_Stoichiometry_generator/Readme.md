**Read this in: [Русский](1_Stoichiometry_generator/README_ru.md)**

# Stoichiometry_* — Stoichiometry generator for CF32/108

# Module naming convention

Each executable file corresponds to a specific component configuration.

**Format:** `XY` (two digits)

- First digit (`X`) = number of cation components
- Second digit (`Y`) = number of anion components

### Examples:
- `84` — 8 cation components, 4 anion components
- `73` — 7 cation components, 3 anion components
- `52` — 5 cation components, 2 anion components
- `11` — 1 cation component, 1 anion component

---

# Calculation modes

The program supports two calculation modes depending on the number of sites in the supercell:

| Mode  | Number of sites |
|-------|-----------------|
| CF32  | 32              |
| CF108 | 108             |

When starting, the user selects the desired mode:
Enter your choice:
1 - CF32
2 - CF108

text

---

# Available charges

## Cations
- Full set: `0 1 2 3 4 5 6 7`
  - `0` = vacancy
  - `1-7` = cation charge states

## Anions
- Full set: `0 1 2 3`
  - `0` = vacancy
  - `1-3` = anion charge states (absolute values; actual charges: `-1`, `-2`, `-3`)

---

# Component selection

For systems with fewer than the maximum number of components, the user manually selects the desired charge states.

**Input format:** list of charges separated by spaces, terminated by `-1`

### Example: 7-component cation system (module 74)

Possible input:
0 1 2 3 4 5 6 -1

text
or
1 2 3 4 5 6 7 -1

text

### Example: 6-component cation system (module 64)

Possible input:
0 1 2 3 4 5 -1

text
or
1 2 3 4 5 6 -1

text

### Example: 3-component anion system (module 83)

Possible input:
0 1 2 -1

text
or
1 2 3 -1

text

---

# Full list of modules

## Number of anion components = 4

| Module | Cation charges | Anion charges |
|--------|----------------|---------------|
| 84     | 0..7           | 0..3          |
| 74     | comp-7         | 0..3          |
| 64     | comp-6         | 0..3          |
| 54     | comp-5         | 0..3          |
| 44     | comp-4         | 0..3          |
| 34     | comp-3         | 0..3          |
| 24     | comp-2         | 0..3          |
| 14     | comp-1         | 0..3          |

## Number of anion components = 3

| Module | Cation charges | Anion charges |
|--------|----------------|---------------|
| 83     | 0..7           | comp-3        |
| 73     | comp-7         | comp-3        |
| 63     | comp-6         | comp-3        |
| 53     | comp-5         | comp-3        |
| 43     | comp-4         | comp-3        |
| 33     | comp-3         | comp-3        |
| 23     | comp-2         | comp-3        |
| 13     | comp-1         | comp-3        |

## Number of anion components = 2

| Module | Cation charges | Anion charges |
|--------|----------------|---------------|
| 82     | 0..7           | comp-2        |
| 72     | comp-7         | comp-2        |
| 62     | comp-6         | comp-2        |
| 52     | comp-5         | comp-2        |
| 42     | comp-4         | comp-2        |
| 32     | comp-3         | comp-2        |
| 22     | comp-2         | comp-2        |
| 12     | comp-1         | comp-2        |

## Number of anion components = 1

| Module | Cation charges | Anion charges |
|--------|----------------|---------------|
| 81     | 0..7           | comp-1        |
| 71     | comp-7         | comp-1        |
| 61     | comp-6         | comp-1        |
| 51     | comp-5         | comp-1        |
| 41     | comp-4         | comp-1        |
| 31     | comp-3         | comp-1        |
| 21     | comp-2         | comp-1        |
| 11     | comp-1         | comp-1        |

---

# Meaning of comp-N

The notation `comp-N` means that `N` charge states are selected from the full set of charges.

### Example for cations:
`comp-5` can be selected as:
0 1 2 3 4

text
or
1 2 3 4 5

text
or any other valid set of five different charges from `0` to `7`.

### Example for anions:
`comp-3` can be selected as:
0 1 2

text
or
1 2 3

text
or any other valid set of three different charges from `0` to `3`.

---

# Output format

The program generates all valid stoichiometric configurations that satisfy the electroneutrality condition.

**Output file:** `stoichiometry_XX.txt` (where `XX` corresponds to the module number)

### Example output (module 73, CF32):
A5+1 B5+2 C5+3 D4+4 E4+5 F4+6 V5 0| X27-3 Y4-2 Z1-1 ch=90 1
A5+1 B5+2 C5+3 D4+4 E4+5 F4+6 V5 0| X28-3 Y2-2 Z2-1 ch=90 2
A5+1 B5+2 C5+3 D4+4 E4+5 F4+6 V5 0| X28-3 Y3-2 V1 0 ch=90 3
A5+1 B5+2 C4+3 D5+4 E4+5 F4+6 V5 0| X28-3 Y3-2 Z1-1 ch=91 4

text

### Output format explanation:

| Element | Description |
|---------|-------------|
| `A5+1`  | 5 atoms of cation with charge +1 |
| `B5+2`  | 5 atoms of cation with charge +2 |
| `V5+0`  | 5 vacancies (charge 0) |
| `\|`    | Separator between cations and anions |
| `X27-3` | 27 atoms of anion with charge -1 |
| `Y4-2`  | 4 atoms of anion with charge -2 |
| `Z1-1`  | 1 atom of anion with charge -3 |
| `ch=90` | Total absolute charge (balance check) |
| `1`     | Stoichiometry sequence number |

---

# Cation symbols

| Symbol | Charge |
|--------|-------|
| A      | +1    |
| B      | +2    |
| C      | +3    |
| D      | +4    |
| E      | +5    |
| F      | +6    |
| G      | +7    |
| V      | 0 (vacancy) |

---

# Anion symbols

| Symbol | Charge |
|--------|-------|
| X      | -1    |
| Y      | -2    |
| Z      | -3    |
| V      | 0 (vacancy) |

---

# Program workflow

1. Start the program
2. Select calculation mode (CF32 or CF108)
3. Select component configuration (module number)
   - Select cation charges (manually or all)
   - Select anion charges (manually or all)
4. Select output mode (save to file or display only)
5. Generate all permutations of cation and anion charges
6. Generate distributions of stoichiometric coefficients
7. Check electroneutrality condition
8. Output valid stoichiometries to file (if selected)
9. Display statistics (execution time, total found)

---

## Example session

> stoichiometry_73.exe

Enter your choice (1 - CF32 or 2 - CF108): 1

Selected: CF32 (32 sites)

Enter your choice (1 - Generate all cations or 2 - Selected cations): 1

Generate all cations

Enter your choice (1 - Generate all anions or 2 - Selected anions): 1

Generate all anions

Enter your choice (1 - Do not save to file, or 2 - Save to file): 1

Selected: Do not save to file

PROGRAM START: 2026-06-04 09:39:21

Processing st_c1 = 27 from 29 var = 4974541  [00:00:01]

Processing st_c1 = 28 from 29 var = 5010373  [00:00:01]

Processing st_c1 = 29 from 29 var = 5027558  [00:00:01]

Time: 1.468 seconds

Total found: 5033065

## Important: The data file can take up a lot of space! It is recommended to first run the calculation without saving to a file to estimate the file size (one var is approximately 100 bytes). The directory contains a Results.xlsx file with an estimate of the number of var for all component configurations.

Performance (CPU — i7-1360P)

Mode	Components	Time	Output size

CF32	8 cations + 4 anions	~6 seconds	8,452,591

CF32	7 cations + 3 anions	~6 seconds	19,678,072

CF108	3 cations + 4 anions	~1 minute	78,318,473

CF108	4 cations + 3 anions	~5 minutes	343,875,345

Note: Actual performance depends on the processor and the selected component configuration.

---

# Features

Support for up to 8 cation components

Support for up to 4 anion components

Support for vacancies (charge 0)

Support for CF32 and CF108 supercell sizes

Elimination of duplicate configurations through canonical ordering

Ability to select charges for configurations with reduced component count

Full enumeration of electroneutral stoichiometries

Limitations
Maximum number of cation components: 8

Maximum number of anion components: 4

Supported site counts: only 32 (CF32) or 108 (CF108)

Anion charges are stored as absolute values; actual charges are negative

Output file can become very large (several GB for CF108 mode)

Error handling
Error	Solution
ERROR: Invalid choice!	Restart and enter 1 or 2
ERROR: Cannot create file!	Check disk space and permissions
Invalid charge: X, ignored	Enter charges from 0 to max_charge
Total found: 0	Check component configuration

---

Compilation - Windows (MSVC)

```cl Stoichiometry_*.cpp /O2 /Oi /Ot /EHsc /Fe:stoichiometry_*.exe```