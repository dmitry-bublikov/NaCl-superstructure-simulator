===============================================================================
                        STOICHIOMETRY GENERATOR
                            DOCUMENTATION
===============================================================================

OVERVIEW
===============================================================================
Stoichiometry Generator is a C++ program designed to generate all possible
stoichiometric combinations of multicomponent systems that satisfy the condition
of charge neutrality.

For every generated configuration, the program verifies:

    Σ(number of cations × cation charge) = Σ(number of anions × anion charge)

Only electrically neutral configurations are retained in the output.


CITATION
===============================================================================
This software accompanies the publication:

D. N. Bublikov,
"Combinatorial Approach to the Simulation of Superstructures of the NaCl Type",
Crystallography Reports 56, 89-100 (2011).
DOI: 10.1134/S106377451101007X


SYSTEM REQUIREMENTS
===============================================================================
- Operating System: Windows 10/11
- Compiler: Microsoft Visual Studio 2022 (or later)
- RAM: 4 GB minimum (8 GB recommended for CF108 mode)
- Disk Space: ~500 MB for output files


COMPILATION
===============================================================================
The program is written in standard C++ and compiled using Microsoft Visual Studio.

Compilation environment:
    Visual Studio 2022 Developer Command Prompt v17.14.25
    Microsoft (R) C/C++ Optimizing Compiler Version 19.44.35222

Example compilation command:

    cl Stoichiometry_84.cpp /O2 /Oi /Ot /EHsc /Fe:stoichiometry_84.exe

Compiler flags explained:
    /O2    - Maximum speed optimization
    /Oi    - Generate intrinsic functions
    /Ot    - Favor fast code over small code
    /EHsc  - Enable C++ exception handling
    /Fe    - Specify output executable name


MODULE NAMING CONVENTION
===============================================================================
Each executable corresponds to a specific component configuration.

Format: XY (two digits)

    First digit (X) = number of cation components
    Second digit (Y) = number of anion components

Examples:
    84 - 8 cation components, 4 anion components
    73 - 7 cation components, 3 anion components
    52 - 5 cation components, 2 anion components
    11 - 1 cation component, 1 anion component


CALCULATION MODES
===============================================================================
The program supports two calculation modes based on the number of sites:

    Mode    | Number of Sites| 
    --------|----------------|
    CF32    | 32             | 
    CF108   | 108            | 

At startup, the user selects the desired mode:

    Enter your choice:
    1 - CF32
    2 - CF108


AVAILABLE CHARGES
===============================================================================
CATIONS:
    Full set: 0 1 2 3 4 5 6 7
    0 = vacancy
    1-7 = cation charge states

ANIONS:
    Full set: 0 1 2 3
    0 = vacancy
    1-3 = anion charge states (absolute values; actual charges are -1, -2, -3)


COMPONENT SELECTION
===============================================================================
For systems with fewer than the maximum number of components, the user manually
selects the desired charge states.

Input format: space-separated list of charges, terminated by -1

Example: 7-Component Cation System (Module 74)

    Possible inputs:
        0 1 2 3 4 5 6 -1
    or
        1 2 3 4 5 6 7 -1

Example: 6-Component Cation System (Module 64)

    Possible inputs:
        0 1 2 3 4 5 -1
    or
        1 2 3 4 5 6 -1

Example: 3-Component Anion System (Module 83)

    Possible inputs:
        0 1 2 -1
    or
        1 2 3 -1


COMPLETE MODULE LIST
===============================================================================
ANION COMPONENT COUNT = 4
    Module | Cation Charges | Anion Charges
    --------|----------------|---------------
    84     | 0..7           | 0..3
    74     | comp-7         | 0..3
    64     | comp-6         | 0..3
    54     | comp-5         | 0..3
    44     | comp-4         | 0..3
    34     | comp-3         | 0..3
    24     | comp-2         | 0..3
    14     | comp-1         | 0..3

ANION COMPONENT COUNT = 3
    Module | Cation Charges | Anion Charges
    --------|----------------|---------------
    83     | 0..7           | comp-3
    73     | comp-7         | comp-3
    63     | comp-6         | comp-3
    53     | comp-5         | comp-3
    43     | comp-4         | comp-3
    33     | comp-3         | comp-3
    23     | comp-2         | comp-3
    13     | comp-1         | comp-3

ANION COMPONENT COUNT = 2
    Module | Cation Charges | Anion Charges
    --------|----------------|---------------
    82     | 0..7           | comp-2
    72     | comp-7         | comp-2
    62     | comp-6         | comp-2
    52     | comp-5         | comp-2
    42     | comp-4         | comp-2
    32     | comp-3         | comp-2
    22     | comp-2         | comp-2
    12     | comp-1         | comp-2

ANION COMPONENT COUNT = 1
    Module | Cation Charges | Anion Charges
    --------|----------------|---------------
    81     | 0..7           | comp-1
    71     | comp-7         | comp-1
    61     | comp-6         | comp-1
    51     | comp-5         | comp-1
    41     | comp-4         | comp-1
    31     | comp-3         | comp-1
    21     | comp-2         | comp-1
    11     | comp-1         | comp-1


MEANING OF comp-N
===============================================================================
The notation comp-N means that N charge states are selected from the complete
charge set.

Example: comp-5 for cations may be chosen as:
    0 1 2 3 4
or
    1 2 3 4 5
or any other valid set containing five distinct charge states from 0..7.

Example: comp-3 for anions may be chosen as:
    0 1 2
or
    1 2 3
or any other valid set containing three distinct charge states from 0..3.


OUTPUT FORMAT
===============================================================================
The program generates all valid stoichiometric configurations satisfying the
charge neutrality condition.

Output file: stoichiometry_XX.txt (where XX corresponds to the module number)

Example Output (Module 73, CF32):

A5+1 B5+2 C5+3 D4+4 E4+5 F4+6 V5 0| X27-3 Y4-2 Z1-1 ch=90 1
A5+1 B5+2 C5+3 D4+4 E4+5 F4+6 V5 0| X28-3 Y2-2 Z2-1 ch=90 2
A5+1 B5+2 C5+3 D4+4 E4+5 F4+6 V5 0| X28-3 Y3-2 V1 0 ch=90 3
A5+1 B5+2 C4+3 D5+4 E4+5 F4+6 V5 0| X28-3 Y3-2 Z1-1 ch=91 4

Output Format Explained:
    A5+1      - 5 atoms of cation with charge +1
    B5+2      - 5 atoms of cation with charge +2
    V5+0      - 5 vacancies (charge 0)
    |         - Separator between cations and anions
    X27-3     - 27 atoms of anion with charge -3
    Y4-2      - 4 atoms of anion with charge -2
    Z1-1      - 1 atoms of anion with charge -1
    ch=90     - Total absolute charge (balance check)
    1         - Sequential stoichiometry number

CATION SYMBOLS
===============================================================================
    Symbol | Charge
    --------|-------
    A      | +1
    B      | +2
    C      | +3
    D      | +4
    E      | +5
    F      | +6
    G      | +7
    V      | 0 (vacancy)


ANION SYMBOLS
===============================================================================
    Symbol | Charge
    --------|-------
    X      | -1
    Y      | -2
    Z      | -3
    V      | 0 (vacancy)


PROGRAM WORKFLOW
===============================================================================
    1. Start
    2. Select calculation mode (CF32 or CF108)
    3. Select component configuration (module number)
       - Choose cation charges (manually or all)
       - Choose anion charges (manually or all)
    4. Select output mode (save to file or console only)
    5. Generate all permutations of cation and anion charges
    6. Generate stoichiometric coefficient distributions
    7. Check charge neutrality
    8. Output valid stoichiometries to file (if selected)
    9. Display statistics (execution time, total found)


EXAMPLE SESSION
===============================================================================
> stoichiometry_73.exe

Enter your choice (1 - CF32 or 2 - CF108): 1
Selected: CF32 (32 sites)
Enter your choice (1 - Generate all cations or 2 - Selected cations): 1
Generate all cations
Enter your choice (1 - Generate all anions or 2 - Selected anions): 1
Generate all anions
Enter your choice (1 - Do not save to file, or 2 - Save to file): 1
Selected: Do not save to file
==============================================
PROGRAM START: 2026-06-04 09:39:21
==============================================
Processing st_c1 = 27 from 29 var = 4974541  [00:00:01]
Processing st_c1 = 28 from 29 var = 5010373  [00:00:01]
Processing st_c1 = 29 from 29 var = 5027558  [00:00:01]
Time: 1.468 seconds
Total found: 5033065

Please note that the data file may take up a lot of space!!!

PERFORMANCE (CPU - i7-1360P)
===============================================================================
    Mode    | Components              | Time        | Output Size
    --------|-------------------------|-------------|-------------
    CF32    | 8 cations + 4 anions    | ~6 seconds  | 8452591
    CF32    | 7 cations + 3 anions    | ~6 seconds  | 19678072
    CF108   | 8 cations + 4 anions    | ~5 minutes  | 78318473
    CF108   | 7 cations + 3 anions    | ~5 minutes  | 343875345

Note: Actual performance depends on CPU and selected component configuration.
It is recommended to first run the calculation without saving to a file in order
to estimate the file size (one var is approximately 100 bytes).

FEATURES
===============================================================================
- Supports up to 8 cation components
- Supports up to 4 anion components
- Supports vacancies (charge 0)
- Supports both CF32 and CF108 supercell sizes
- Eliminates duplicate configurations through canonical ordering
- Optional charge selection for reduced component configurations
- Optimized compilation with MSVC /O2 /Oi /Ot flags
- Exhaustive enumeration of charge-balanced stoichiometries


LIMITATIONS
===============================================================================
- Maximum cation components: 8
- Maximum anion components: 4
- Supported site counts: 32 (CF32) or 108 (CF108) only
- Anion charges are stored as absolute values; actual charges are negative
- File output can become very large (several GB for CF108 mode)


ERROR HANDLING
===============================================================================
    Error                                      | Solution
    -------------------------------------------|--------------------------------
    ERROR: Invalid choice!                     | Restart and enter 1 or 2
    ERROR: Cannot create file!                 | Check disk space and permissions
    Invalid charge: X, ignored                 | Enter charges from 0 to max_charge
    Total found: 0                             | Check component configuration


LICENSE AND USAGE
===============================================================================
This software is provided for academic and research purposes. When using this
program in scientific publications, please cite the original paper:

    D. N. Bublikov,
    "Combinatorial Approach to the Simulation of Superstructures of the NaCl Type",
    Crystallography Reports 56, 89-100 (2011).


CONTACT
===============================================================================
For questions, bug reports, or suggestions, please contact the author.


===============================================================================
Version: 1.0
Last Updated: June 2026
Author: Dmitry Bublikov
===============================================================================