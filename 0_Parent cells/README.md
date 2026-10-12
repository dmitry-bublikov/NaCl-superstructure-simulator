**Read this in: [Русский](README_ru.md)**

# Module 0 — Parent Cells

`Module 0` builds the catalogues of maximal parent translation cells used by the subsequent stoichiometry and structure-enumeration modules.

It contains two programs:

| Program | Default geometric envelope | Maximum index | Output |
|---|---:|---:|---|
| `Parent_cells_CF32.cpp` | `2 x 2 x 2` parent cubic cells | `N = 32` | `CF32_parents.txt` |
| `Parent_cells_CF108.cpp` | `3 x 3 x 3` parent cubic cells | `N = 108` | `CF108_parents.txt` |

Here `N` is the number of cation sites in the translation cell. For a fully occupied NaCl-type model, the corresponding anion sublattice also contains `N` sites.

---

## Purpose

The module searches for translation-cell forms of the fcc cation sublattice that can be represented within the selected CF geometric envelope. Equivalent cells are merged by cubic parent symmetry, after which redundant cells are removed by a containment test.

The final result is a compact catalogue of **maximal parent cells**. These cells are used as the structural parent catalogue for later stages of the program.

The global stoichiometry stage does not need to repeat a calculation for every cell shape. Cells with the same `N` share the same global stoichiometric search space, so stoichiometries can be generated once per distinct `N` and then reused for all parent cells having that index.

---

## Default results

### CF32

The CF32 calculation gives:

```text
Starting symmetry-inequivalent geometric-envelope classes: 53
Maximal parent cells: 25
Covered/redundant classes: 28
```

Distribution of the 25 maximal parent cells by `N`:

```text
4 (1)  5 (1)  6 (3)  7 (2)  8 (5)  9 (3)  10 (1)
12 (4)  13 (1)  14 (1)  16 (1)  20 (1)  32 (1)
```

There are 25 parent cells grouped into 13 distinct values of `N`.

### CF108

The CF108 calculation gives:

```text
Starting symmetry-inequivalent geometric-envelope classes: 616
Maximal parent cells: 335
Covered/redundant classes: 281
```

Distribution of the 335 maximal parent cells by `N`:

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

There are 335 parent cells grouped into 48 distinct values of `N`.

The number in parentheses is the number of maximal parent cells having that value of `N`.

---

## Selection rule

A candidate translation cell is retained in the starting catalogue when:

1. it has at least one basis parallelepiped that fits inside the selected axis-aligned parent-cubic envelope;
2. its index satisfies `N <= Nmax`;
3. cells related by one of the 48 signed-permutation symmetries of the cubic parent are treated as equivalent.

The resulting symmetry-inequivalent cells are then compared by translation-lattice containment. A cell is removed if another cell in the same catalogue is a proper translation supercell that can contain every periodic ordering of the smaller cell for at least one relative cubic orientation.

The cells that are not covered by another catalogue member form the final maximal-parent set.

---

## Hermite normal form (HNF)

The programs represent translation lattices in the primitive fcc basis and convert each integer basis matrix to **Hermite normal form (HNF)**.

HNF is a canonical form for an integer lattice basis. Different basis matrices can describe exactly the same translation lattice; converting them to HNF provides a deterministic representation that makes duplicate detection possible.

For a full-rank `3 x 3` HNF matrix `H`:

```text
N = |det(H)|
```

is the lattice index and therefore the number of cation sites in the corresponding translation cell.

HNF is not, by itself, the geometric search algorithm. The search first enumerates admissible fcc basis-vector triples; HNF is then used to canonicalize the translation lattice represented by each triple.

---

## Output format

Each program writes one catalogue file. No separate summary file is produced; the summary and the `N` distribution are included in the header of the catalogue itself.

Typical entry:

```text
# MAXIMAL_PARENT 01 / 25
# original_catalogue_cell = 7
CELL 7
  index_cation_sites: 4
  representative_row_HNF: [1 0 0; 0 1 0; 0 0 4]
  cubic_orbit_key_HNF: [1 0 0; 0 1 0; 0 0 4]
  primitive_fcc_basis_rows: [...]
  cartesian_basis_a0: [...]
  bbox_span_a0: ...
  metric_a0: a=... b=... c=... alpha=... beta=... gamma=...
```

### Field meanings

| Field | Meaning |
|---|---|
| `MAXIMAL_PARENT` | sequential number inside the final maximal-parent catalogue |
| `original_catalogue_cell` | cell number before maximal-containment reduction |
| `index_cation_sites` | index `N`, equal to the number of cation sites |
| `representative_row_HNF` | representative translation lattice in row HNF |
| `cubic_orbit_key_HNF` | canonical key of the orbit under the 48 cubic symmetries |
| `primitive_fcc_basis_rows` | selected compact basis in primitive-fcc integer coordinates |
| `cartesian_basis_a0` | the same basis in conventional Cartesian coordinates, in units of `a0` |
| `bbox_span_a0` | axis-aligned span of the displayed basis parallelepiped |
| `metric_a0` | lengths and inter-vector angles of the displayed basis |

---

## Command-line options

Both programs support:

```text
--box B
--nmax N
--output FILE
```

Default CF32 run:

```bat
Parent_cells_CF32.exe
```

Default CF108 run:

```bat
Parent_cells_CF108.exe
```

Custom example:

```bat
Parent_cells_CF32.exe --box 2 --nmax 32 --output CF32_parents.txt
```

---

## Compilation — Windows (MSVC)

```bat
cl Parent_cells_CF32.cpp /O2 /Oi /Ot /EHsc /Fe:parent_cells_cf32.exe
cl Parent_cells_CF108.cpp /O2 /Oi /Ot /EHsc /Fe:parent_cells_cf108.exe
```

---

## Position in the workflow

```text
Module 0: Parent Cells
        |
        v
Stoichiometry modules
        |
        v
Local-balance / configuration generation
        |
        v
Crystallographic filtering and standardization
```

Module 0 defines the parent-cell catalogue and the available `N` values before chemical stoichiometries and atomic occupations are enumerated.

For a more detailed mathematical and algorithmic description, see [DOCUMENTATION.md](DOCUMENTATION.md).
