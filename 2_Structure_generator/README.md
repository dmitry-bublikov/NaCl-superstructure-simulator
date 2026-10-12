# Local Valence-Balance Search for CF32 and CF108

**Русская версия:** [README_ru.md](README_ru.md)

These programs filter stoichiometric compositions of NaCl-derived structures for which **at least one assignment of ions to lattice sites exists** that satisfies the selected local valence-balance criterion. A separate CF32 generator enumerates concrete cation-charge assignments for a restricted workflow.

## 1. Programs and scope

| Source file | Structure | Balance center | Supported operation |
| --- | --- | --- | --- |
| `Find-local-balance-32-32-a.cpp` | CF32: 32 cation + 32 anion sites | **Anion-centered** | Mixed `X/Y/Z/V` anions, exact search, `D = 0` and `D > 0` |
| `Find-local-balance-32-32-c.cpp` | CF32: 32 cation + 32 anion sites | **Cation-centered** | Mixed cations and cation vacancies, exact search, `D = 0` and `D > 0` |
| `Find-local-balance-108-108-a.cpp` | CF108: 108 cation + 108 anion sites | **Anion-centered** | **One uniform anion charge only**; exact balance `D = 0` |
| `Gen-structures.cpp` | CF32: 32 cation sites | **Enumerates assignments for one stoichiometry** | Used **only with `Find-local-balance-32-32-a.cpp` for component-count codes `11, 21, 31, 41, 51, 61, 71, 81`** (1–8 cation components and exactly 1 anion component); one uniform target balance |

**Important:** the CF32 `-a` and `-c` programs test **different** criteria. A stoichiometry passing both independent filters does **not** by itself show that a *single common assignment* satisfies both simultaneously: each filter may have found a different assignment.

The three filters write **the original lines of accepted stoichiometries**, not atomic coordinates or all superstructures. `Gen-structures.cpp` is the separate enumerator for the limited use case above.

## 2. Mathematical criteria

The NaCl-type neighborhood contains six nearest neighbors on the opposite sublattice. `D` (also called `delta`) is the maximum absolute deviation. `D = 0` requires exact local balance.

**Anion-centered balance** (`Find-local-balance-32-32-a.cpp`, `Find-local-balance-108-108-a.cpp`):

\[
\left|\sum_{j \in N(a)} q_j + 6z_a\right|\leq D,
\]

where `q_j` are neighboring positive cation charges and `z_a` is the negative central anion charge. Equivalently, the target sum is `6 × |z_a|`: `6`, `12`, and `18` for `X⁻`, `Y²⁻`, and `Z³⁻`. For CF32, an **anion vacancy center** has no local equation and is skipped.

**Cation-centered balance** (`Find-local-balance-32-32-c.cpp`):

\[
\left|\sum_{j \in N(c)} |z_j| - 6q_c\right|\leq D,
\]

where `|z_j|` are the magnitudes of six neighboring anion charges and `q_c` is the central cation charge. A **cation vacancy center** has no local equation and is skipped. This is the dual neighborhood problem, but it is **not the same acceptance criterion** as anion-centered balance.

## 3. Input files

### CF32: `Find-local-balance-32-32-a.cpp` and `Find-local-balance-32-32-c.cpp`

Both programs read:

```text
stoichiometry_N_32_32.txt
```

`N` is the component-count code entered at startup (e.g., `11`, `31`, or `84`). The **first digit** indicates the number of cation components and the **second digit** the number of anion components: `31` means 3 cation and 1 anion component; `84` means 8 cation and 4 anion components. The executable uses this code to construct the input filename.

Example input line:

```text
A32+2 | Y32-2 ch=64 1
```

`A32+2` specifies 32 cations of charge `+2`; `Y32-2` specifies 32 anions of charge `-2`. `ch=64` and the trailing number are existing dataset metadata. Supported cation component labels are `A`–`G` and vacancy `V` (charges `0…+7`); supported anion labels are `X`, `Y`, `Z`, and `V` (charges `0, -1, -2, -3`). The multiplicities must sum to **exactly 32 cation sites and 32 anion sites**. For example, a vacancy can be written `V4 0`.

### CF108: `Find-local-balance-108-108-a.cpp`

This program expects a **different filename**:

```text
stoichiometry_N.txt
```

The cation portion must describe **108 sites**; components `A`–`G` and `V` with charges `0…+7` are supported. **Different cation components must have distinct charges.** Example:

```text
A108+2 | Y108-2 ch=216 1
```

The program parses **only the cation part before `|`**. The absolute uniform anion charge (`0`, `1`, `2`, or `3`) is entered **interactively**. Users must therefore ensure that the selected charge is consistent with the prepared data: mixed anion charge targets are **not supported**.

## 4. Build on Windows (Visual Studio / MSVC)

Open **Developer PowerShell for Visual Studio 2022** or the **x64 Native Tools Command Prompt**, then change into your project directory. Each command creates a separate executable:

```powershell
cl Find-local-balance-32-32-c.cpp /O2 /Oi /Ot /EHsc /std:c++17 /Fe:find-local-balance-32-32-c.exe
cl Find-local-balance-32-32-a.cpp /O2 /Oi /Ot /EHsc /std:c++17 /Fe:find-local-balance-32-32-a.exe
cl Find-local-balance-108-108-a.cpp /O2 /Oi /Ot /EHsc /std:c++17 /Fe:find-local-balance-108-108-a.exe
cl Gen-structures.cpp /O2 /Oi /Ot /EHsc /std:c++17 /Fe:Gen-structures.exe
```

Place the input file in the **current working directory** from which the executable is launched.

## 5. Running the CF32 filters

### 5.1. Anion-centered — `find-local-balance-32-32-a.exe`

```powershell
.\find-local-balance-32-32-a.exe
```

Sample interactive input:

```text
Enter stoichiometry file number (e.g. 84): 11
Enter allowed deviation delta (e.g. 0): 0
```

This reads `stoichiometry_11_32_32.txt` and writes **`local_balance_11_0.txt`**. Each anion component's charge is read from the stoichiometry line; the program **does not ask for a separate anion charge**.

For suitable `D = 0` problems, the implementation uses necessary modular constraints (`mod 2` and `mod 3`) and symmetry reduction. Specialized paths handle certain `D > 0` cases, while general mixed-anion cases retain an exact capacity-constrained search.

### 5.2. Cation-centered — `find-local-balance-32-32-c.exe`

```powershell
.\find-local-balance-32-32-c.exe
```

Sample interactive input:

```text
Enter stoichiometry file number (e.g. 84): 11
Enter allowed deviation delta (e.g. 0): 0
```

Input: `stoichiometry_11_32_32.txt`. Output: **`local_balance_11_0.txt`**. Here, anion magnitudes are assigned to sites and constraints are tested around cation centers. For `D = 0`, the program uses necessary modular equations for the **transposed** CF32 incidence matrix. Certain `D > 0` cases also have specialized fast paths.

**Warning: output filename collision.** The `-a` and `-c` filters both use **`local_balance_N_D.txt`**, opened for writing. Running both for the same `N` and `D` in one directory will **overwrite the first result** unless it has been saved. Use separate working directories or rename each file before running the next program, for example:

```powershell
# Immediately after -a, before running -c
Rename-Item local_balance_11_0.txt local_balance_11_0_anion_centered.txt
# After -c completes
Rename-Item local_balance_11_0.txt local_balance_11_0_cation_centered.txt
```

Both CF32 executables are multithreaded. Accepted output lines may appear in a different order than the input; compare result files **without relying on line order**.

## 6. Running CF108 — `find-local-balance-108-108-a.exe`

```powershell
.\find-local-balance-108-108-a.exe
```

Example interaction:

```text
Enter stoichiometry file number (e.g., 31, 71): 11
Enter absolute UNIFORM anion charge for local balance (0, 1, 2, 3): 2
```

This reads `stoichiometry_11.txt` and writes **`local_balance_cf108_11_12_0.txt`**, with `12 = 6 × 2`. There is no `D` input: the program always uses **`D = 0`**.

The exact CF108 solver reduces 108 cation sites to **27 quartets**, with an identical charge within each quartet, leaving **12 independent variables** (the octahedral incidence matrix has rank 96). Consequently, **each cation-charge multiplicity must be divisible by four**. Global charge balance is checked as well. These are **necessary, not sufficient** conditions: the exact solver still checks any remaining candidates.

CF108 processes input **one line at a time** and uses a bounded cache, including for very large stoichiometry files. It **does not** support cation-centered local balance or mixed anion-charge targets.

## 7. `Gen-structures.cpp`: enumerating CF32 assignments

**Strict scope:** `Gen-structures.cpp` is used **only after `Find-local-balance-32-32-a.cpp`, for component-count codes `11, 21, 31, 41, 51, 61, 71, 81`**. These are **not sequential stoichiometry record numbers**: the first digit represents the number of cation components (**1–8**) and the second digit the number of anion components (**exactly 1**). It is **not** a downstream step for either `Find-local-balance-32-32-c.cpp` or `Find-local-balance-108-108-a.cpp`.

The generator uses **one fixed anion-centered target `B`**. It therefore cannot directly represent mixed anion charges for which different anion centers have different target sums. For **one** chosen stoichiometry, it enumerates **all qualifying assignments** of charges to 32 cation sites, rather than merely reporting whether one exists.

### Preparing an input file

1. Run `find-local-balance-32-32-a.exe` with the appropriate `stoichiometry_N_32_32.txt`, where **`N ∈ {11, 21, 31, 41, 51, 61, 71, 81}`** (one anion component).
2. Select **one** accepted line with a uniform anion charge from `local_balance_N_D.txt`.
3. Save that line in a new **`for_generator_N_B_D.txt`** file, where `B = 6 × |z_A|` and `D` is the deviation used by the filter. This step is **manual**; the generator does not automatically consume the filter's output.

For instance, a uniform `Y²⁻` stoichiometry `A32+2 | Y32-2 ch=64 1` under exact balance uses:

```text
for_generator_11_12_0.txt
```

File contents (a single line):

```text
A32+2 | Y32-2 ch=64 1
```

### Execution and output

```powershell
.\Gen-structures.exe
```

Enter the **filename suffix**, without `for_generator_` or `.txt`:

```text
Enter stoichiometry file number (e.g., 31_12_0, etc.): 11_12_0
```

The program reads `for_generator_11_12_0.txt` and writes **`structuries_11_12_0.txt`**. The spelling `structuries` is **intentional here because it is exactly what the source code writes**; it is not `structures` or `local_balance`.

Each output line contains **32 integer cation charges**, corresponding to positions `p1 … p32` in the generator's built-in site order. These represent **charge assignments**, not atomic coordinates. Exhaustive enumeration may take considerable time and produce large output files.

## 8. Filename reference

| Program | Input | Interactive parameters | Output |
| --- | --- | --- | --- |
| CF32 `-a` | `stoichiometry_N_32_32.txt` | `N`, `D` | `local_balance_N_D.txt` |
| CF32 `-c` | `stoichiometry_N_32_32.txt` | `N`, `D` | `local_balance_N_D.txt` |
| CF108 `-a` | `stoichiometry_N.txt` | `N`, `|z_A|` | `local_balance_cf108_N_B_0.txt`, `B = 6 × |z_A|` |
| `Gen-structures` (CF32 `N = 11, 21, 31, 41, 51, 61, 71, 81`; after `-a` only) | `for_generator_N_B_D.txt` | `N_B_D` | `structuries_N_B_D.txt` |

## 9. Notes

- An accepted line means **at least one valid assignment exists** under the implemented local-balance model. It is **not proof of thermodynamic stability**.
- `D = 0` requires exact balance; `D > 0` allows a specified deviation in the CF32 programs. Only `D = 0` is implemented for CF108.
- Keep the outputs of the two CF32 filters separate; because the programs are multithreaded, result ordering may vary between runs.
- The filenames documented here follow the **actual patterns in the supplied source files**. In particular, CF108's `stoichiometry_N.txt` is not the same as CF32's `stoichiometry_N_32_32.txt`.
