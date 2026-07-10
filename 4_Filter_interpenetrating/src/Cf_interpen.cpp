//===========================================================================
// PROGRAM: CF Interpenetration Filter v3
// Developed by Dmitry Bublikov in 2026
//===========================================================================
//
// Purpose:
//   Strict filtering of equivalent CF32 variants directly in p1...p32
//   representation.
//
// Input:
//   structuries_<number>_f1.txt
//
// Output:
//   structuries_<number>_f2.txt
//
// Main idea:
//   A structure is removed only if the full 32-position coloring is equivalent
//   to an already saved representative by:
//
//        24 proper cubic rotations
//      x 32 CF32-compatible translations
//      x full comparison of all 32 cation positions
//
// Compilation:
//   cl Cf_interpen_v3.cpp /O2 /Oi /Ot /EHsc /Fe:cf_interpen_v3.exe
//===========================================================================

#include <cstdio>
#include <iostream>
#include <vector>
#include <array>

using namespace std;

static const int POSITION_COUNT = 32;
static const int GRID_SIZE = 4;

typedef array<int, POSITION_COUNT> Structure;

struct Vec3 {
    int x;
    int y;
    int z;
};

struct Rot3 {
    int m[3][3];
};

// CF32 cation positions in integer coordinates on the 4x4x4 grid.
// Same numbering as in gen_us_32.cpp.
static const Vec3 POS[POSITION_COUNT] = {
    {2,2,3}, {2,1,2}, {1,2,2}, {2,3,2},
    {3,2,2}, {2,2,1}, {1,1,3}, {1,3,3},
    {3,3,3}, {3,1,3}, {3,1,1}, {3,3,1},
    {1,3,1}, {1,1,1}, {2,3,0}, {3,2,0},
    {2,1,0}, {1,2,0}, {0,2,3}, {0,1,2},
    {0,2,1}, {0,3,2}, {2,0,3}, {3,0,2},
    {2,0,1}, {1,0,2}, {1,0,0}, {3,0,0},
    {0,1,0}, {0,3,0}, {0,0,3}, {0,0,1}
};

//===========================================================================
// index_from_coord
//===========================================================================
int index_from_coord(int x, int y, int z)
{
    x = (x % GRID_SIZE + GRID_SIZE) % GRID_SIZE;
    y = (y % GRID_SIZE + GRID_SIZE) % GRID_SIZE;
    z = (z % GRID_SIZE + GRID_SIZE) % GRID_SIZE;

    for (int i = 0; i < POSITION_COUNT; i++) {
        if (POS[i].x == x && POS[i].y == y && POS[i].z == z) {
            return i;
        }
    }

    return -1;
}

//===========================================================================
// determinant
//===========================================================================
int determinant(const Rot3& r)
{
    int a = r.m[0][0], b = r.m[0][1], c = r.m[0][2];
    int d = r.m[1][0], e = r.m[1][1], f = r.m[1][2];
    int g = r.m[2][0], h = r.m[2][1], i = r.m[2][2];

    return a * (e * i - f * h)
         - b * (d * i - f * g)
         + c * (d * h - e * g);
}

//===========================================================================
// generate_rotations
//
// Generates all 24 proper rotations of the cube.
//===========================================================================
vector<Rot3> generate_rotations()
{
    vector<Rot3> rotations;

    int perm[6][3] = {
        {0,1,2}, {0,2,1}, {1,0,2},
        {1,2,0}, {2,0,1}, {2,1,0}
    };

    int signs[8][3] = {
        { 1, 1, 1}, { 1, 1,-1}, { 1,-1, 1}, { 1,-1,-1},
        {-1, 1, 1}, {-1, 1,-1}, {-1,-1, 1}, {-1,-1,-1}
    };

    for (int p = 0; p < 6; p++) {
        for (int s = 0; s < 8; s++) {
            Rot3 r;

            for (int row = 0; row < 3; row++) {
                for (int col = 0; col < 3; col++) {
                    r.m[row][col] = 0;
                }
            }

            for (int row = 0; row < 3; row++) {
                r.m[row][perm[p][row]] = signs[s][row];
            }

            if (determinant(r) == 1) {
                rotations.push_back(r);
            }
        }
    }

    return rotations;
}

//===========================================================================
// transform_index
//
// Applies q = M * p + t  (mod 4).
//===========================================================================
int transform_index(int index, const Rot3& r, int tx, int ty, int tz)
{
    const Vec3& p = POS[index];

    int x = r.m[0][0] * p.x + r.m[0][1] * p.y + r.m[0][2] * p.z + tx;
    int y = r.m[1][0] * p.x + r.m[1][1] * p.y + r.m[1][2] * p.z + ty;
    int z = r.m[2][0] * p.x + r.m[2][1] * p.y + r.m[2][2] * p.z + tz;

    return index_from_coord(x, y, z);
}

//===========================================================================
// read_structure
//===========================================================================
bool read_structure(FILE* fp, Structure& s)
{
    int values[POSITION_COUNT];

    int n = fscanf(fp,
        "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
        &values[0],  &values[1],  &values[2],  &values[3],
        &values[4],  &values[5],  &values[6],  &values[7],
        &values[8],  &values[9],  &values[10], &values[11],
        &values[12], &values[13], &values[14], &values[15],
        &values[16], &values[17], &values[18], &values[19],
        &values[20], &values[21], &values[22], &values[23],
        &values[24], &values[25], &values[26], &values[27],
        &values[28], &values[29], &values[30], &values[31]);

    if (n != POSITION_COUNT) return false;

    for (int i = 0; i < POSITION_COUNT; i++) {
        s[i] = values[i];
    }

    return true;
}

//===========================================================================
// write_structure
//===========================================================================
void write_structure(FILE* fp, const Structure& s)
{
    for (int i = 0; i < POSITION_COUNT; i++) {
        if (i > 0) fprintf(fp, " ");
        fprintf(fp, "%d", s[i]);
    }
    fprintf(fp, "\n");
}

//===========================================================================
// match_operation
//
// representative[p] must be equal to current[q],
// where q = M*p + t.
//===========================================================================
bool match_operation(const Structure& representative,
                     const Structure& current,
                     const Rot3& r,
                     int tx,
                     int ty,
                     int tz)
{
    for (int p = 0; p < POSITION_COUNT; p++) {
        int q = transform_index(p, r, tx, ty, tz);

        if (q < 0) {
            return false;
        }

        if (representative[p] != current[q]) {
            return false;
        }
    }

    return true;
}

//===========================================================================
// is_equivalent
//===========================================================================
bool is_equivalent(const Structure& representative,
                   const Structure& current,
                   const vector<Rot3>& rotations)
{
    for (size_t r = 0; r < rotations.size(); r++) {
        for (int tx = 0; tx < GRID_SIZE; tx++) {
            for (int ty = 0; ty < GRID_SIZE; ty++) {
                for (int tz = 0; tz < GRID_SIZE; tz++) {

                    // Only 32 CF32-compatible translations.
                    if (((tx + ty + tz) & 1) != 0) continue;

                    if (match_operation(representative, current,
                                        rotations[r], tx, ty, tz)) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

//===========================================================================
// is_duplicate
//===========================================================================
bool is_duplicate(const vector<Structure>& representatives,
                  const Structure& current,
                  const vector<Rot3>& rotations)
{
    for (size_t i = 0; i < representatives.size(); i++) {
        if (is_equivalent(representatives[i], current, rotations)) {
            return true;
        }
    }

    return false;
}

//===========================================================================
// main
//===========================================================================
int main(int argc, char** argv)
{
    char file_number[100];
    char input_filename[256];
    char output_filename[256];

    cout << "Enter stoichiometry file number (e.g., 31_12_0, etc.): ";
    cin >> file_number;

    sprintf(input_filename, "structuries_%s_f1.txt", file_number);
    sprintf(output_filename, "structuries_%s_f2.txt", file_number);

    FILE* input = fopen(input_filename, "r");
    if (input == NULL) {
        cout << "Error: Cannot open file " << input_filename << endl;
        return 1;
    }

    vector<Rot3> rotations = generate_rotations();

    vector<Structure> representatives;
    Structure current;

    int input_count = 0;
    int removed_count = 0;

    while (read_structure(input, current)) {
        input_count++;

        if (!is_duplicate(representatives, current, rotations)) {
            representatives.push_back(current);
        } else {
            removed_count++;
        }
    }

    fclose(input);

    FILE* output = fopen(output_filename, "w");
    if (output == NULL) {
        cout << "Error: Cannot create file " << output_filename << endl;
        return 1;
    }

    for (size_t i = 0; i < representatives.size(); i++) {
        write_structure(output, representatives[i]);
    }

    fclose(output);

    cout << "//============================================================" << endl;
    cout << "// CF32 INTERPENETRATION FILTER v3 REPORT" << endl;
    cout << "//============================================================" << endl;
    cout << "Input structures:       " << input_count << endl;
    cout << "Output structures:      " << representatives.size() << endl;
    cout << "Removed structures:     " << removed_count << endl;
    cout << "Rotations used:         " << rotations.size() << endl;
    cout << "Translations per rot.:  32" << endl;
    cout << "Total operations:       " << rotations.size() * 32 << endl;
    cout << "Input file:             " << input_filename << endl;
    cout << "Output file:            " << output_filename << endl;
    cout << "Status: STRICT ROTATION + TRANSLATION CF32 FILTER" << endl;
    cout << "//============================================================" << endl;

    return 0;
}
