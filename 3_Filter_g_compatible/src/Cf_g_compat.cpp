//===========================================================================
// PROGRAM: Optimized Stoichiometry Generator (SORTED OUTPUT)
// Developed by Dmitry Bublikov in 2026
//===========================================================================

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>
#define A 5000 // Maximum number of unique structures

using namespace std;

int main(int argc, char **argv)
{
 
int i, ii=1, check; // i, ii - loop counters; flag - indicates if structures are equivalent
// Сationic positions found
int ap1[A], ap2[A], ap3[A], ap4[A], ap5[A], ap6[A], ap7[A], ap8[A], ap9[A], ap10[A],
ap11[A], ap12[A], ap13[A], ap14[A], ap15[A], ap16[A], ap17[A], ap18[A], ap19[A],
ap20[A], ap21[A], ap22[A], ap23[A], ap24[A], ap25[A], ap26[A], ap27[A], ap28[A],
ap29[A], ap30[A], ap31[A], ap32[A];
// Current cationic positions
int p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17,
p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32;


// ********************************************** INPUT FILE **************************************
      char file_number[100];
      char filename[100];

      // Ask user for file number
      cout << "Enter stoichiometry file number (e.g., 31_12_0, etc.): ";
      cin >> file_number;

      // Form the filename
      sprintf(filename, "structuries_%s.txt", file_number);
      
      // Open the file
      FILE *Fpdat = fopen(filename, "r");
      
      if (Fpdat == NULL) {
          cout << "Error: Cannot open file " << filename << endl;
          return 1;
      }
// ********************************************** OUT FILE **************************************
      char structuries_filename[100];
      sprintf(structuries_filename,
        "structuries_%s_f1.txt",
        file_number);

      FILE *Fout = fopen(structuries_filename, "w");

fscanf(Fpdat,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
&ap1[1],&ap2[1],&ap3[1],&ap4[1],&ap5[1],&ap6[1],&ap7[1],&ap8[1],&ap9[1],&ap10[1],&ap11[1],&ap12[1],&ap13[1],
&ap14[1],&ap15[1],&ap16[1],&ap17[1],&ap18[1],&ap19[1],&ap20[1],&ap21[1],&ap22[1],&ap23[1],&ap24[1],&ap25[1],
&ap26[1],&ap27[1],&ap28[1],&ap29[1],&ap30[1],&ap31[1],&ap32[1]);

while (fscanf(Fpdat, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
              &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10,
              &p11, &p12, &p13, &p14, &p15, &p16, &p17, &p18, &p19, &p20,
              &p21, &p22, &p23, &p24, &p25, &p26, &p27, &p28, &p29, &p30,
              &p31, &p32) == 32) 
{
check=0;

for (i=1; i<=ii; i++)
if (
                                                     // CF32 1-axis
 (ap1[i]==p1 &&                                      
 ap2[i]==p2 && ap5[i]==p5 && ap4[i]==p4 && ap3[i]==p3 &&
 ap6[i]==p6 &&

 ap7[i]==p7 && ap10[i]==p10 && ap9[i]==p9 && ap8[i]==p8 &&
 ap14[i]==p14 && ap11[i]==p11 && ap12[i]==p12 && ap13[i]==p13 &&

 ap17[i]==p17 && ap16[i]==p16 && ap15[i]==p15 && ap18[i]==p18 &&
 ap23[i]==p23 && ap19[i]==p19 && ap23[i]==p23 && ap19[i]==p19 &&
 ap26[i]==p26 && ap24[i]==p24 && ap20[i]==p20 && ap22[i]==p22 &&
                                                 ap24[i]==p24 && ap26[i]==p26 && ap22[i]==p22 && ap20[i]==p20 &&
 ap25[i]==p25 && ap21[i]==p21 && ap25[i]==p25 && ap21[i]==p21 &&

 ap27[i]==p27 && ap28[i]==p28 && ap29[i]==p29 && ap30[i]==p30 &&
                                                 ap28[i]==p28 && ap27[i]==p27 && ap30[i]==p30 && ap29[i]==p29 &&
 ap31[i]==p31 &&
 ap32[i]==p32 )

||

 (ap1[i]==p1 &&
 ap2[i]==p5 && ap5[i]==p4 && ap4[i]==p3 && ap3[i]==p2 &&
 ap6[i]==p6 &&

 ap7[i]==p10 && ap10[i]==p9 && ap9[i]==p8 && ap8[i]==p7 &&
 ap14[i]==p11 && ap11[i]==p12 && ap12[i]==p13 && ap13[i]==p14 &&

 ap17[i]==p16 && ap16[i]==p15 && ap15[i]==p18 && ap18[i]==p17 &&
 ap23[i]==p19 && ap19[i]==p23 && ap23[i]==p19 && ap19[i]==p23 &&
 ap26[i]==p20 && ap24[i]==p22 && ap20[i]==p24 && ap22[i]==p26 &&
                                                 ap24[i]==p22 && ap26[i]==p20 && ap22[i]==p26 && ap20[i]==p24 &&
 ap25[i]==p21 && ap21[i]==p25 && ap25[i]==p21 && ap21[i]==p25 &&

 ap27[i]==p29 && ap28[i]==p30 && ap29[i]==p28 && ap30[i]==p27 &&
                                                 ap28[i]==p30 && ap27[i]==p29 && ap30[i]==p27 && ap29[i]==p28 &&
 ap31[i]==p31 &&
 ap32[i]==p32 )

 ||

 (ap1[i]==p1 &&
 ap2[i]==p4 && ap5[i]==p3 && ap4[i]==p2 && ap3[i]==p5 &&
 ap6[i]==p6 &&

 ap7[i]==p9 && ap10[i]==p8 && ap9[i]==p7 && ap8[i]==p10 &&
 ap14[i]==p12 && ap11[i]==p13 && ap12[i]==p14 && ap13[i]==p11 &&

 ap17[i]==p15 && ap16[i]==p18 && ap15[i]==p17 && ap18[i]==p16 &&
 ap23[i]==p23 && ap19[i]==p19 && ap23[i]==p23 && ap19[i]==p19 &&
 ap26[i]==p24 && ap24[i]==p26 && ap20[i]==p22 && ap22[i]==p20 &&
                                                 ap24[i]==p26 && ap26[i]==p24 && ap22[i]==p20 && ap20[i]==p22 &&
 ap25[i]==p25 && ap21[i]==p21 && ap25[i]==p25 && ap21[i]==p21 &&

 ap27[i]==p28 && ap28[i]==p27 && ap29[i]==p30 && ap30[i]==p29 &&
                                                 ap28[i]==p27 && ap27[i]==p28 && ap30[i]==p29 && ap29[i]==p30 &&
 ap31[i]==p31 &&
 ap32[i]==p32 )

  ||

 (ap1[i]==p1 &&
 ap2[i]==p3 && ap5[i]==p2 && ap4[i]==p5 && ap3[i]==p4 &&
 ap6[i]==p6 &&

 ap7[i]==p8 && ap10[i]==p7 && ap9[i]==p10 && ap8[i]==p9 &&
 ap14[i]==p13 && ap11[i]==p14 && ap12[i]==p11 && ap13[i]==p12 &&

 ap17[i]==p18 && ap16[i]==p17 && ap15[i]==p16 && ap18[i]==p15 &&
 ap23[i]==p19 && ap19[i]==p23 && ap23[i]==p19 && ap19[i]==p23 &&
 ap26[i]==p22 && ap24[i]==p20 && ap20[i]==p26 && ap22[i]==p24 &&
                                                 ap24[i]==p20 && ap26[i]==p22 && ap22[i]==p24 && ap20[i]==p26 &&
 ap25[i]==p21 && ap21[i]==p25 && ap25[i]==p21 && ap21[i]==p25 &&

 ap27[i]==p30 && ap28[i]==p29 && ap29[i]==p27 && ap30[i]==p28 &&
                                                 ap28[i]==p29 && ap27[i]==p30 && ap30[i]==p28 && ap29[i]==p27 &&
 ap31[i]==p31 &&
 ap32[i]==p32 )

  ||
                                                      // CF32 1-axis (inverted)
 (ap1[i]==p6 &&
 ap2[i]==p2 && ap5[i]==p3 && ap4[i]==p4 && ap3[i]==p5 &&
 ap6[i]==p1 &&

 ap7[i]==p11 && ap10[i]==p14 && ap9[i]==p13 && ap8[i]==p12 &&
 ap14[i]==p10 && ap11[i]==p7 && ap12[i]==p8 && ap13[i]==p9 &&

 ap17[i]==p17 && ap16[i]==p18 && ap15[i]==p15 && ap18[i]==p16 &&
 ap23[i]==p25 && ap19[i]==p21 && ap23[i]==p25 && ap19[i]==p21 &&
 ap26[i]==p24 && ap24[i]==p26 && ap20[i]==p20 && ap22[i]==p22 &&
                                                 ap24[i]==p26 && ap26[i]==p24 && ap22[i]==p22 && ap20[i]==p20 &&
 ap25[i]==p23 && ap21[i]==p19 && ap25[i]==p23 && ap21[i]==p19 &&

 ap27[i]==p28 && ap28[i]==p27 && ap29[i]==p29 && ap30[i]==p30 &&
                                                 ap28[i]==p27 && ap27[i]==p28 && ap30[i]==p30 && ap29[i]==p29 &&
 ap31[i]==p32 &&
 ap32[i]==p31 )

  ||

 (ap1[i]==p6 &&
 ap2[i]==p3 && ap5[i]==p4 && ap4[i]==p5 && ap3[i]==p2 &&
 ap6[i]==p1 &&

 ap7[i]==p14 && ap10[i]==p13 && ap9[i]==p12 && ap8[i]==p11 &&
 ap14[i]==p7 && ap11[i]==p8 && ap12[i]==p9 && ap13[i]==p10 &&

 ap17[i]==p18 && ap16[i]==p15 && ap15[i]==p16 && ap18[i]==p17 &&
 ap23[i]==p21 && ap19[i]==p25 && ap23[i]==p21 && ap19[i]==p25 &&
 ap26[i]==p20 && ap24[i]==p22 && ap20[i]==p26 && ap22[i]==p24 &&
                                                 ap24[i]==p22 && ap26[i]==p20 && ap22[i]==p24 && ap20[i]==p26 &&
 ap25[i]==p19 && ap21[i]==p23 && ap25[i]==p19 && ap21[i]==p23 &&

 ap27[i]==p29 && ap28[i]==p30 && ap29[i]==p27 && ap30[i]==p28 &&
                                                 ap28[i]==p30 && ap27[i]==p29 && ap30[i]==p28 && ap29[i]==p27 &&
 ap31[i]==p32 &&
 ap32[i]==p31 )
               
  ||

 (ap1[i]==p6 &&
 ap2[i]==p4 && ap5[i]==p5 && ap4[i]==p2 && ap3[i]==p3 &&
 ap6[i]==p1 &&

 ap7[i]==p13 && ap10[i]==p12 && ap9[i]==p11 && ap8[i]==p14 &&
 ap14[i]==p8 && ap11[i]==p9 && ap12[i]==p10 && ap13[i]==p7 &&

 ap17[i]==p15 && ap16[i]==p16 && ap15[i]==p17 && ap18[i]==p18 &&
 ap23[i]==p25 && ap19[i]==p21 && ap23[i]==p25 && ap19[i]==p21 &&
 ap26[i]==p26 && ap24[i]==p24 && ap20[i]==p22 && ap22[i]==p20 &&
                                                 ap24[i]==p24 && ap26[i]==p26 && ap22[i]==p20 && ap20[i]==p22 &&
 ap25[i]==p23 && ap21[i]==p19 && ap25[i]==p23 && ap21[i]==p19 &&

 ap27[i]==p27 && ap28[i]==p28 && ap29[i]==p30 && ap30[i]==p29 &&
                                                 ap28[i]==p28 && ap27[i]==p27 && ap30[i]==p29 && ap29[i]==p30 &&
 ap31[i]==p32 &&
 ap32[i]==p31 )

  ||

 (ap1[i]==p6 &&
 ap2[i]==p5 && ap5[i]==p2 && ap4[i]==p3 && ap3[i]==p4 &&
 ap6[i]==p1 &&

 ap7[i]==p12 && ap10[i]==p11 && ap9[i]==p14 && ap8[i]==p13 &&
 ap14[i]==p9 && ap11[i]==p10 && ap12[i]==p7 && ap13[i]==p8 &&

 ap17[i]==p16 && ap16[i]==p17 && ap15[i]==p18 && ap18[i]==p15 &&
 ap23[i]==p21 && ap19[i]==p25 && ap23[i]==p21 && ap19[i]==p25 &&
 ap26[i]==p22 && ap24[i]==p20 && ap20[i]==p24 && ap22[i]==p26 &&
                                                 ap24[i]==p20 && ap26[i]==p22 && ap22[i]==p26 && ap20[i]==p24 &&
 ap25[i]==p19 && ap21[i]==p23 && ap25[i]==p19 && ap21[i]==p23 &&

 ap27[i]==p30 && ap28[i]==p29 && ap29[i]==p28 && ap30[i]==p27 &&
                                                 ap28[i]==p29 && ap27[i]==p30 && ap30[i]==p27 && ap29[i]==p28 &&
 ap31[i]==p32 &&
 ap32[i]==p31 )

  ||
                                                // CF32 2-axis
 (ap1[i]==p2 &&
 ap2[i]==p1 && ap5[i]==p3 && ap4[i]==p6 && ap3[i]==p5 &&
 ap6[i]==p4 &&

 ap7[i]==p10 && ap10[i]==p7 && ap9[i]==p14 && ap8[i]==p11 &&
 ap14[i]==p9 && ap11[i]==p8 && ap12[i]==p13 && ap13[i]==p12 &&

 ap17[i]==p23 && ap16[i]==p26 && ap15[i]==p25 && ap18[i]==p24 &&
 ap23[i]==p17 && ap19[i]==p20 && ap23[i]==p17 && ap19[i]==p20 &&
 ap26[i]==p16 && ap24[i]==p18 && ap20[i]==p19 && ap22[i]==p21 &&
                                                 ap24[i]==p18 && ap26[i]==p16 && ap22[i]==p21 && ap20[i]==p19 &&
 ap25[i]==p15 && ap21[i]==p22 && ap25[i]==p15 && ap21[i]==p22 &&

 ap27[i]==p28 && ap28[i]==p27 && ap29[i]==p31 && ap30[i]==p32 &&
                                                 ap28[i]==p27 && ap27[i]==p28 && ap30[i]==p32 && ap29[i]==p31 &&
 ap31[i]==p29 &&
 ap32[i]==p30 )

  ||

 (ap1[i]==p2 &&
 ap2[i]==p3 && ap5[i]==p6 && ap4[i]==p5 && ap3[i]==p1 &&
 ap6[i]==p4 &&

 ap7[i]==p7 && ap10[i]==p14 && ap9[i]==p11 && ap8[i]==p10 &&
 ap14[i]==p8 && ap11[i]==p13 && ap12[i]==p12 && ap13[i]==p9 &&

 ap17[i]==p26 && ap16[i]==p25 && ap15[i]==p24 && ap18[i]==p23 &&
 ap23[i]==p20 && ap19[i]==p17 && ap23[i]==p20 && ap19[i]==p17 &&
 ap26[i]==p19 && ap24[i]==p21 && ap20[i]==p18 && ap22[i]==p16 &&
                                                 ap24[i]==p21 && ap26[i]==p19 && ap22[i]==p16 && ap20[i]==p18 &&
 ap25[i]==p22 && ap21[i]==p15 && ap25[i]==p22 && ap21[i]==p15 &&

 ap27[i]==p31 && ap28[i]==p32 && ap29[i]==p27 && ap30[i]==p28 &&
                                                 ap28[i]==p32 && ap27[i]==p31 && ap30[i]==p28 && ap29[i]==p27 &&
 ap31[i]==p29 &&
 ap32[i]==p30 )
             
  ||                                     
                   
 (ap1[i]==p2 &&
 ap2[i]==p6 && ap5[i]==p5 && ap4[i]==p1 && ap3[i]==p3 &&
 ap6[i]==p4 &&

 ap7[i]==p14 && ap10[i]==p11 && ap9[i]==p10 && ap8[i]==p7 &&
 ap14[i]==p13 && ap11[i]==p12 && ap12[i]==p9 && ap13[i]==p8 &&

 ap17[i]==p25 && ap16[i]==p24 && ap15[i]==p23 && ap18[i]==p26 &&
 ap23[i]==p17 && ap19[i]==p20 && ap23[i]==p17 && ap19[i]==p20 &&
 ap26[i]==p18 && ap24[i]==p16 && ap20[i]==p21 && ap22[i]==p19 &&
                                                 ap24[i]==p16 && ap26[i]==p18 && ap22[i]==p19 && ap20[i]==p21 &&
 ap25[i]==p15 && ap21[i]==p22 && ap25[i]==p15 && ap21[i]==p22 &&

 ap27[i]==p27 && ap28[i]==p28 && ap29[i]==p32 && ap30[i]==p31 &&
                                                 ap28[i]==p28 && ap27[i]==p27 && ap30[i]==p31 && ap29[i]==p32 &&
 ap31[i]==p29 &&
 ap32[i]==p30 )


  ||

 (ap1[i]==p2 &&
 ap2[i]==p5 && ap5[i]==p1 && ap4[i]==p3 && ap3[i]==p6 &&
 ap6[i]==p4 &&

 ap7[i]==p11 && ap10[i]==p10 && ap9[i]==p7 && ap8[i]==p14 &&
 ap14[i]==p12 && ap11[i]==p9 && ap12[i]==p8 && ap13[i]==p13 &&

 ap17[i]==p24 && ap16[i]==p23 && ap15[i]==p26 && ap18[i]==p25 &&
 ap23[i]==p20 && ap19[i]==p17 && ap23[i]==p20 && ap19[i]==p17 &&
 ap26[i]==p21 && ap24[i]==p19 && ap20[i]==p16 && ap22[i]==p18 &&
                                                 ap24[i]==p19 && ap26[i]==p21 && ap22[i]==p18 && ap20[i]==p16 &&
 ap25[i]==p22 && ap21[i]==p15 && ap25[i]==p22 && ap21[i]==p15 &&

 ap27[i]==p32 && ap28[i]==p31 && ap29[i]==p28 && ap30[i]==p27 &&
                                                 ap28[i]==p31 && ap27[i]==p32 && ap30[i]==p27 && ap29[i]==p28 &&
 ap31[i]==p29 &&
 ap32[i]==p30 )
                                              // CF32 2-axis (inverted)
  ||                                     
        
 (ap1[i]==p4 &&
 ap2[i]==p1 && ap5[i]==p5 && ap4[i]==p6 && ap3[i]==p3 &&
 ap6[i]==p2 &&

 ap7[i]==p8 && ap10[i]==p9 && ap9[i]==p12 && ap8[i]==p13 &&
 ap14[i]==p7 && ap11[i]==p10 && ap12[i]==p11 && ap13[i]==p14 &&

 ap17[i]==p23 && ap16[i]==p24 && ap15[i]==p25 && ap18[i]==p26 &&
 ap23[i]==p15 && ap19[i]==p22 && ap23[i]==p15 && ap19[i]==p22 &&
 ap26[i]==p18 && ap24[i]==p16 && ap20[i]==p19 && ap22[i]==p21 &&
                                                 ap24[i]==p16 && ap26[i]==p18 && ap22[i]==p21 && ap20[i]==p19 &&
 ap25[i]==p17 && ap21[i]==p20 && ap25[i]==p17 && ap21[i]==p20 &&

 ap27[i]==p27 && ap28[i]==p28 && ap29[i]==p31 && ap30[i]==p32 &&
                                                 ap28[i]==p28 && ap27[i]==p27 && ap30[i]==p32 && ap29[i]==p31 &&
 ap31[i]==p30 &&
 ap32[i]==p29 )
        
  ||

 (ap1[i]==p4 &&
 ap2[i]==p5 && ap5[i]==p6 && ap4[i]==p3 && ap3[i]==p1 &&
 ap6[i]==p2 &&

 ap7[i]==p9 && ap10[i]==p12 && ap9[i]==p13 && ap8[i]==p8 &&
 ap14[i]==p10 && ap11[i]==p11 && ap12[i]==p14 && ap13[i]==p7 &&

 ap17[i]==p24 && ap16[i]==p25 && ap15[i]==p26 && ap18[i]==p23 &&
 ap23[i]==p22 && ap19[i]==p15 && ap23[i]==p22 && ap19[i]==p15 &&
 ap26[i]==p19 && ap24[i]==p21 && ap20[i]==p16 && ap22[i]==p18 &&
                                                 ap24[i]==p21 && ap26[i]==p19 && ap22[i]==p18 && ap20[i]==p16 &&
 ap25[i]==p20 && ap21[i]==p17 && ap25[i]==p20 && ap21[i]==p17 &&

 ap27[i]==p31 && ap28[i]==p32 && ap29[i]==p28 && ap30[i]==p27 &&
                                                 ap28[i]==p32 && ap27[i]==p31 && ap30[i]==p27 && ap29[i]==p28 &&
 ap31[i]==p30 &&
 ap32[i]==p29 )
              
  ||

 (ap1[i]==p4 &&
 ap2[i]==p6 && ap5[i]==p3 && ap4[i]==p1 && ap3[i]==p5 &&
 ap6[i]==p2 &&

 ap7[i]==p12 && ap10[i]==p13 && ap9[i]==p8 && ap8[i]==p9 &&
 ap14[i]==p11 && ap11[i]==p14 && ap12[i]==p7 && ap13[i]==p10 &&

 ap17[i]==p25 && ap16[i]==p26 && ap15[i]==p23 && ap18[i]==p24 &&
 ap23[i]==p15 && ap19[i]==p22 && ap23[i]==p15 && ap19[i]==p22 &&
 ap26[i]==p16 && ap24[i]==p18 && ap20[i]==p21 && ap22[i]==p19 &&
                                                 ap24[i]==p18 && ap26[i]==p16 && ap22[i]==p19 && ap20[i]==p21 &&
 ap25[i]==p17 && ap21[i]==p20 && ap25[i]==p17 && ap21[i]==p20 &&

 ap27[i]==p28 && ap28[i]==p27 && ap29[i]==p32 && ap30[i]==p31 &&
                                                 ap28[i]==p27 && ap27[i]==p28 && ap30[i]==p31 && ap29[i]==p32 &&
 ap31[i]==p30 &&
 ap32[i]==p29 )

  ||

 (ap1[i]==p4 &&
 ap2[i]==p3 && ap5[i]==p1 && ap4[i]==p5 && ap3[i]==p6 &&
 ap6[i]==p2 &&

 ap7[i]==p13 && ap10[i]==p8 && ap9[i]==p9 && ap8[i]==p12 &&
 ap14[i]==p14 && ap11[i]==p7 && ap12[i]==p10 && ap13[i]==p11 &&

 ap17[i]==p26 && ap16[i]==p23 && ap15[i]==p24 && ap18[i]==p25 &&
 ap23[i]==p22 && ap19[i]==p15 && ap23[i]==p22 && ap19[i]==p15 &&
 ap26[i]==p21 && ap24[i]==p19 && ap20[i]==p18 && ap22[i]==p16 &&
                                                 ap24[i]==p19 && ap26[i]==p21 && ap22[i]==p16 && ap20[i]==p18 &&
 ap25[i]==p20 && ap21[i]==p17 && ap25[i]==p20 && ap21[i]==p17 &&

 ap27[i]==p32 && ap28[i]==p31 && ap29[i]==p27 && ap30[i]==p28 &&
                                                 ap28[i]==p31 && ap27[i]==p32 && ap30[i]==p28 && ap29[i]==p27 &&
 ap31[i]==p30 &&
 ap32[i]==p29 )
                                              // CF32 3-axis
   ||

 (ap1[i]==p3 &&
 ap2[i]==p1 && ap5[i]==p4 && ap4[i]==p6 && ap3[i]==p2 &&
 ap6[i]==p5 &&

 ap7[i]==p7 && ap10[i]==p8 && ap9[i]==p13 && ap8[i]==p14 &&
 ap14[i]==p10 && ap11[i]==p9 && ap12[i]==p12 && ap13[i]==p11 &&

 ap17[i]==p19 && ap16[i]==p22 && ap15[i]==p21 && ap18[i]==p20 &&
 ap23[i]==p18 && ap19[i]==p26 && ap23[i]==p18 && ap19[i]==p26 &&
 ap26[i]==p17 && ap24[i]==p15 && ap20[i]==p23 && ap22[i]==p25 &&
                                                 ap24[i]==p15 && ap26[i]==p17 && ap22[i]==p25 && ap20[i]==p23 &&
 ap25[i]==p16 && ap21[i]==p24 && ap25[i]==p16 && ap21[i]==p24 &&

 ap27[i]==p29 && ap28[i]==p30 && ap29[i]==p31 && ap30[i]==p32 &&
                                                 ap28[i]==p30 && ap27[i]==p29 && ap30[i]==p32 && ap29[i]==p31 &&
 ap31[i]==p27 &&
 ap32[i]==p28 )
         
   ||

 (ap1[i]==p3 &&
 ap2[i]==p4 && ap5[i]==p6 && ap4[i]==p2 && ap3[i]==p1 &&
 ap6[i]==p5 &&

 ap7[i]==p8 && ap10[i]==p13 && ap9[i]==p14 && ap8[i]==p7 &&
 ap14[i]==p9 && ap11[i]==p12 && ap12[i]==p11 && ap13[i]==p10 &&

 ap17[i]==p22 && ap16[i]==p21 && ap15[i]==p20 && ap18[i]==p19 &&
 ap23[i]==p26 && ap19[i]==p18 && ap23[i]==p26 && ap19[i]==p18 &&
 ap26[i]==p23 && ap24[i]==p25 && ap20[i]==p15 && ap22[i]==p17 &&
                                                 ap24[i]==p25 && ap26[i]==p23 && ap22[i]==p17 && ap20[i]==p15 &&
 ap25[i]==p24 && ap21[i]==p16 && ap25[i]==p24 && ap21[i]==p16 &&

 ap27[i]==p31 && ap28[i]==p32 && ap29[i]==p30 && ap30[i]==p29 &&
                                                 ap28[i]==p32 && ap27[i]==p31 && ap30[i]==p29 && ap29[i]==p30 &&
 ap31[i]==p27 &&
 ap32[i]==p28 )
                   
   ||

 (ap1[i]==p3 &&
 ap2[i]==p6 && ap5[i]==p2 && ap4[i]==p1 && ap3[i]==p4 &&
 ap6[i]==p5 &&

 ap7[i]==p13 && ap10[i]==p14 && ap9[i]==p7 && ap8[i]==p8 &&
 ap14[i]==p12 && ap11[i]==p11 && ap12[i]==p10 && ap13[i]==p9 &&

 ap17[i]==p21 && ap16[i]==p20 && ap15[i]==p19 && ap18[i]==p22 &&
 ap23[i]==p18 && ap19[i]==p26 && ap23[i]==p18 && ap19[i]==p26 &&
 ap26[i]==p15 && ap24[i]==p17 && ap20[i]==p25 && ap22[i]==p23 &&
                                                 ap24[i]==p17 && ap26[i]==p15 && ap22[i]==p23 && ap20[i]==p25 &&
 ap25[i]==p16 && ap21[i]==p24 && ap25[i]==p16 && ap21[i]==p24 &&

 ap27[i]==p30 && ap28[i]==p29 && ap29[i]==p32 && ap30[i]==p31 &&
                                                 ap28[i]==p29 && ap27[i]==p30 && ap30[i]==p31 && ap29[i]==p32 &&
 ap31[i]==p27 &&
 ap32[i]==p28 )
             
   ||

 (ap1[i]==p3 &&
 ap2[i]==p2 && ap5[i]==p1 && ap4[i]==p4 && ap3[i]==p6 &&
 ap6[i]==p5 &&

 ap7[i]==p14 && ap10[i]==p7 && ap9[i]==p8 && ap8[i]==p13 &&
 ap14[i]==p11 && ap11[i]==p10 && ap12[i]==p9 && ap13[i]==p12 &&

 ap17[i]==p20 && ap16[i]==p19 && ap15[i]==p22 && ap18[i]==p21 &&
 ap23[i]==p26 && ap19[i]==p18 && ap23[i]==p26 && ap19[i]==p18 &&
 ap26[i]==p25 && ap24[i]==p23 && ap20[i]==p17 && ap22[i]==p15 &&
                                                 ap24[i]==p23 && ap26[i]==p25 && ap22[i]==p15 && ap20[i]==p17 &&
 ap25[i]==p24 && ap21[i]==p16 && ap25[i]==p24 && ap21[i]==p16 &&

 ap27[i]==p32 && ap28[i]==p31 && ap29[i]==p29 && ap30[i]==p30 &&
                                                 ap28[i]==p31 && ap27[i]==p32 && ap30[i]==p30 && ap29[i]==p29 &&
 ap31[i]==p27 &&
 ap32[i]==p28 )

   ||
                                              // CF32 3-axis (inverted)
 (ap1[i]==p5 &&
 ap2[i]==p1 && ap5[i]==p2 && ap4[i]==p6 && ap3[i]==p4 &&
 ap6[i]==p3 &&

 ap7[i]==p9 && ap10[i]==p10 && ap9[i]==p11 && ap8[i]==p12 &&
 ap14[i]==p8 && ap11[i]==p7 && ap12[i]==p14 && ap13[i]==p13 &&

 ap17[i]==p19 && ap16[i]==p20 && ap15[i]==p21 && ap18[i]==p22 &&
 ap23[i]==p16 && ap19[i]==p24 && ap23[i]==p16 && ap19[i]==p24 &&
 ap26[i]==p15 && ap24[i]==p17 && ap20[i]==p23 && ap22[i]==p25 &&
                                                 ap24[i]==p17 && ap26[i]==p15 && ap22[i]==p25 && ap20[i]==p23 &&
 ap25[i]==p18 && ap21[i]==p26 && ap25[i]==p18 && ap21[i]==p26 &&

 ap27[i]==p30 && ap28[i]==p29 && ap29[i]==p31 && ap30[i]==p32 &&
                                                 ap28[i]==p29 && ap27[i]==p30 && ap30[i]==p32 && ap29[i]==p31 &&
 ap31[i]==p28 &&
 ap32[i]==p27 )

   ||

 (ap1[i]==p5 &&
 ap2[i]==p2 && ap5[i]==p6 && ap4[i]==p4 && ap3[i]==p1 &&
 ap6[i]==p3 &&

 ap7[i]==p10 && ap10[i]==p11 && ap9[i]==p12 && ap8[i]==p9 &&
 ap14[i]==p7 && ap11[i]==p14 && ap12[i]==p13 && ap13[i]==p8 &&

 ap17[i]==p20 && ap16[i]==p21 && ap15[i]==p22 && ap18[i]==p19 &&
 ap23[i]==p24 && ap19[i]==p16 && ap23[i]==p24 && ap19[i]==p16 &&
 ap26[i]==p23 && ap24[i]==p25 && ap20[i]==p17 && ap22[i]==p15 &&
                                                 ap24[i]==p25 && ap26[i]==p23 && ap22[i]==p15 && ap20[i]==p17 &&
 ap25[i]==p26 && ap21[i]==p18 && ap25[i]==p26 && ap21[i]==p18 &&

 ap27[i]==p31 && ap28[i]==p32 && ap29[i]==p29 && ap30[i]==p30 &&
                                                 ap28[i]==p32 && ap27[i]==p31 && ap30[i]==p30 && ap29[i]==p29 &&
 ap31[i]==p28 &&
 ap32[i]==p27 )

   ||

 (ap1[i]==p5 &&
 ap2[i]==p6 && ap5[i]==p4 && ap4[i]==p1 && ap3[i]==p2 &&
 ap6[i]==p3 &&

 ap7[i]==p11 && ap10[i]==p12 && ap9[i]==p9 && ap8[i]==p10 &&
 ap14[i]==p14 && ap11[i]==p13 && ap12[i]==p8 && ap13[i]==p7 &&

 ap17[i]==p21 && ap16[i]==p22 && ap15[i]==p19 && ap18[i]==p20 &&
 ap23[i]==p16 && ap19[i]==p24 && ap23[i]==p16 && ap19[i]==p24 &&
 ap26[i]==p17 && ap24[i]==p15 && ap20[i]==p25 && ap22[i]==p23 &&
                                                 ap24[i]==p15 && ap26[i]==p17 && ap22[i]==p23 && ap20[i]==p25 &&
 ap25[i]==p18 && ap21[i]==p26 && ap25[i]==p18 && ap21[i]==p26 &&

 ap27[i]==p29 && ap28[i]==p30 && ap29[i]==p32 && ap30[i]==p31 &&
                                                 ap28[i]==p30 && ap27[i]==p29 && ap30[i]==p31 && ap29[i]==p32 &&
 ap31[i]==p28 &&
 ap32[i]==p27 )

   ||

 (ap1[i]==p5 &&
 ap2[i]==p4 && ap5[i]==p1 && ap4[i]==p2 && ap3[i]==p6 &&
 ap6[i]==p3 &&

 ap7[i]==p12 && ap10[i]==p9 && ap9[i]==p10 && ap8[i]==p11 &&
 ap14[i]==p13 && ap11[i]==p8 && ap12[i]==p7 && ap13[i]==p14 &&

 ap17[i]==p22 && ap16[i]==p19 && ap15[i]==p20 && ap18[i]==p21 &&
 ap23[i]==p24 && ap19[i]==p16 && ap23[i]==p24 && ap19[i]==p16 &&
 ap26[i]==p25 && ap24[i]==p23 && ap20[i]==p15 && ap22[i]==p17 &&
                                                 ap24[i]==p23 && ap26[i]==p25 && ap22[i]==p17 && ap20[i]==p15 &&
 ap25[i]==p26 && ap21[i]==p18 && ap25[i]==p26 && ap21[i]==p18 &&

 ap27[i]==p32 && ap28[i]==p31 && ap29[i]==p30 && ap30[i]==p29 &&
                                                 ap28[i]==p31 && ap27[i]==p32 && ap30[i]==p29 && ap29[i]==p30 &&
 ap31[i]==p28 &&
 ap32[i]==p27 )    

      
 )  {check=1; break;} else {}

if (check==0)   { ii++;
ap1[ii]=p1; ap2[ii]=p2; ap3[ii]=p3; ap4[ii]=p4; ap5[ii]=p5; ap6[ii]=p6; ap7[ii]=p7; ap8[ii]=p8; ap9[ii]=p9;
ap10[ii]=p10; ap11[ii]=p11; ap12[ii]=p12; ap13[ii]=p13; ap14[ii]=p14; ap15[ii]=p15;
ap16[ii]=p16; ap17[ii]=p17; ap18[ii]=p18; ap19[ii]=p19; ap20[ii]=p20; ap21[ii]=p21;
ap22[ii]=p22; ap23[ii]=p23; ap24[ii]=p24; ap25[ii]=p25; ap26[ii]=p26; ap27[ii]=p27;
ap28[ii]=p28; ap29[ii]=p29; ap30[ii]=p30; ap31[ii]=p31; ap32[ii]=p32;  }

}                         

fclose(Fpdat);

for (i=1; i<=ii; i++)
{   fprintf(Fout,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
ap1[i], ap2[i], ap3[i], ap4[i], ap5[i], ap6[i], ap7[i], ap8[i], ap9[i], ap10[i], ap11[i], ap12[i], ap13[i],
ap14[i], ap15[i], ap16[i], ap17[i], ap18[i], ap19[i], ap20[i], ap21[i], ap22[i], ap23[i], ap24[i], ap25[i],
ap26[i], ap27[i], ap28[i], ap29[i], ap30[i], ap31[i], ap32[i]);  }

fclose(Fout);
   return 0;
}              