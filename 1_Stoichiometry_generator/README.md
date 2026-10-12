**Read this in: [Русский](README_ru.md)**

# Stoichiometry_* — Stoichiometry generator for CF32/CF108 parent-cell catalogues

The `Stoichiometry_*` programs enumerate globally electroneutral stoichiometries for NaCl-type cation and anion sublattices.

The modernized version separates two concepts:

- **CF32 / CF108** — the selected parent-cell catalogue/context.
- **N** — the actual number of cation sites and anion sites used in the stoichiometric enumeration.

The calculation is performed for the selected **N**, not automatically only for 32 or 108 sites.

---

## Module naming convention

Each source/executable corresponds to a component configuration `XY`:

- `X` = number of cation components.
- `Y` = number of anion components.

Examples: `84`, `73`, `52`, `11`.

The complete set contains 32 modules: 1–8 cation components × 1–4 anion components.

---

## CF catalogue and site count N

At startup select the parent catalogue:

```text
Enter your choice (1 - CF32 or 2 - CF108):
```

Then select `N` from the values available for that catalogue.

### CF32

```text
4 (1)  5 (1)  6 (3)  7 (2)  8 (5)  9 (3)  10 (1)
12 (4)  13 (1)  14 (1)  16 (1)  20 (1)  32 (1)
```

The current CF32 catalogue contains **25 maximal parent cells**, grouped into **13 distinct values of `N`**. The number in parentheses indicates how many maximal parent cells have the corresponding value of `N`. The value **`N = 32` corresponds to the original cubic CF32 fragment**.

### CF108

```text
7 (1)   10 (1)  11 (1)  12 (3)  13 (2)  14 (2)
15 (7)  16 (11) 17 (3)  18 (14) 19 (8)  20 (27)
21 (10) 22 (16) 23 (7)  24 (35) 25 (7)  26 (16)
27 (10) 28 (21) 29 (3)  30 (21) 31 (3)  32 (17)
33 (2)  34 (7)  35 (5)  36 (19) 37 (1)  38 (4)
39 (2)  40 (8)  42 (6)  43 (1)  44 (3)  45 (1)
46 (1)  48 (11) 50 (3)  52 (2)  54 (1)  55 (1)
60 (4)  62 (1)  63 (1)  72 (3)  78 (1)  108 (1)
```

The current CF108 catalogue contains **335 maximal parent cells**, grouped into **48 distinct values of `N`**. The number in parentheses indicates how many maximal parent cells have the corresponding value of `N`. The value **`N = 108` corresponds to the original cubic CF108 fragment**.

At this stage the program asks only for `N`, because **global stoichiometry depends on the number of sites `N`, not on the particular cell shape**. Stoichiometries are therefore generated once for each distinct `N` and can later be reused for all parent-cell forms with the same `N`.

If an entered value is not present in the selected catalogue, the program rejects it and prints the valid list again.

> **Important:** `CF32` and `CF108` are catalogue identifiers anchored to the original cubic CF32 and CF108 fragments. They no longer mean that the calculation must use only `N = 32` or `N = 108`.
>
> For example, `CF32, N = 6` means 6 cation sites and 6 anion sites are used in the stoichiometric calculation, while CF32 identifies the catalogue from which that `N` was selected.

---

## Available charges

### Cations

```text
0 1 2 3 4 5 6 7
```

- `0` = vacancy.
- `1–7` = formal positive charge states.

### Anions

```text
0 1 2 3
```

- `0` = vacancy.
- `1–3` are stored as absolute values; physical charges are `-1`, `-2`, `-3`.

---

## Component selection

For reduced-component modules, charge states can be selected manually.

Input is a space-separated list terminated by `-1`.

Example for module `74`:

```text
0 1 2 3 4 5 6 -1
```

Example for module `64`:

```text
1 2 3 4 5 6 -1
```

Example for a 3-component anion subsystem:

```text
0 1 2 -1
```

Full-component modules may omit the corresponding selection prompt because the complete charge set is used automatically.

---

## Full module matrix

| Anion components | Modules |
|---|---|
| 4 | `14 24 34 44 54 64 74 84` |
| 3 | `13 23 33 43 53 63 73 83` |
| 2 | `12 22 32 42 52 62 72 82` |
| 1 | `11 21 31 41 51 61 71 81` |

`comp-N` means that `N` distinct charge states are selected from the corresponding full charge set.

---

## Enumeration rules

For the selected `N`:

- total cation-site count = `N`;
- total anion-site count = `N`;
- every selected component has a positive stoichiometric coefficient;
- vacancies have charge `0`;
- only globally electroneutral stoichiometries are retained.

Electroneutrality condition:

```text
sum(cation_count × cation_charge)
=
sum(anion_count × |anion_charge|)
```

This module enumerates **global stoichiometries only**. It does not itself test a specific atomic ordering, local coordination environment, or local charge-balance criterion.

---

## Output filename

The modernized filename includes the module, CF catalogue, and `N`:

```text
stoichiometry_XY_CF_N.txt
```

Examples:

```text
stoichiometry_73_32_8.txt
stoichiometry_34_32_20.txt
stoichiometry_24_108_54.txt
stoichiometry_84_108_108.txt
```

This prevents results obtained for different catalogues or different `N` values from being mixed.

---

## Output format

Each valid stoichiometry is written on a separate line.

Example:

```text
A5+1 B1+7 | X6-2 ch=12 1
```

General interpretation:

| Token | Meaning |
|---|---|
| `A5+1` | 5 sites occupied by a cation component with charge +1 |
| `B1+7` | 1 site occupied by a cation component with charge +7 |
| `V2 0` | 2 vacancies |
| `|` | cation/anion separator |
| `X6-2` | 6 sites occupied by an anion component with charge -2 |
| `ch=12` | common absolute cation/anion charge sum |
| final integer | sequential number of the stoichiometry |

For reduced-component modules, the letters `A, B, C, ...` and `X, Y, Z` identify the charge components in the ordering used by that module. The explicit printed `+n` / `-n` value is authoritative.

---

## Program workflow

1. Run the required `Stoichiometry_XY` module.
2. Select `CF32` or `CF108`.
3. Select `N` from the displayed list.
4. Select cation charges if the module provides this option.
5. Select anion charges if the module provides this option.
6. Select output mode: do not save / save.
7. Enumerate stoichiometric coefficient distributions for `N`.
8. Enumerate the relevant charge-component assignments.
9. Check global electroneutrality.
10. Save valid results to `stoichiometry_XY_CF_N.txt` if requested.
11. Print execution time and `Total found`.

---

## Example session

```text
> stoichiometry_73.exe

Enter your choice (1 - CF32 or 2 - CF108): 1
Selected: CF32 (32 sites)

Available N for CF32: 4 5 6 7 8 9 10 12 13 14 16 20 32
Enter number of sites N for the selected parent cell: 8
Selected parent cell: CF32, N = 8

Enter your choice (1 - Generate all cations or 2 - Selected cations): 1
Generate all cations

Enter your choice (1 - Generate all anions or 2 - Selected anions): 1
Generate all anions

Enter your choice (1 - Do not save to file, or 2 - Save to file): 1
Selected: Do not save to file

PROGRAM START: ...
...
Total found: ...
```

> [!WARNING]
> Output files can become very large. It is recommended to run without saving first to estimate `Total found`.
>
> Output size depends strongly on `N`, the module, and the selected charge states.

---

## Reference performance

These values are retained from the previous documentation as reference benchmarks for the full-size cases:

| Catalogue / N | Components | Approx. time | Total found |
|---|---|---:|---:|
| CF32 / N=32 | 8 cations + 4 anions | ~6 s | 8,452,591 |
| CF32 / N=32 | 7 cations + 3 anions | ~6 s | 19,678,072 |
| CF108 / N=108 | 3 cations + 4 anions | ~1 min | 78,318,473 |
| CF108 / N=108 | 4 cations + 3 anions | ~5 min | 343,875,345 |

Actual performance depends on processor, `N`, component count, and selected charges.

---

## Features

- Up to 8 cation components.
- Up to 4 anion components.
- Cation and anion vacancies (`charge = 0`).
- CF32 and CF108 parent-cell catalogues.
- Runtime selection of `N`.
- 13 available `N` values for CF32.
- 48 available `N` values for CF108.
- Manual charge selection for reduced-component modules.
- Canonical ordering to avoid duplicate stoichiometric representations.
- Complete global electroneutrality enumeration for the selected module and `N`.
- Output filenames containing module, CF catalogue, and `N`.

---

## Limitations

- Maximum cation components: 8.
- Maximum anion components: 4.
- `N` must belong to the predefined list for the selected CF catalogue.
- The current `N` lists correspond to the maximal-parent catalogues used by the program.
- Anion charges are stored as absolute values.
- This stage checks global electroneutrality, not local charge balance or structural realizability.
- Large `N` can produce very large output files.

---

## Error handling

| Message | Action |
|---|---|
| `ERROR: Invalid choice!` | Enter one of the listed menu values |
| `ERROR: N must be an integer.` | Enter an integer |
| `ERROR: N = ... is not in the maximal-parent catalogue for CF...` | Select one of the displayed values |
| `Invalid charge: X, ignored` | Enter a charge in the allowed range |
| `Total found: 0` | No valid electroneutral stoichiometries were found |

---

## Compilation — Windows (MSVC)

Example for one module:

```bat
cl Stoichiometry_74.cpp /O2 /Oi /Ot /EHsc /Fe:stoichiometry_74.exe
```

Use the same options for the other `Stoichiometry_*.cpp` modules.
