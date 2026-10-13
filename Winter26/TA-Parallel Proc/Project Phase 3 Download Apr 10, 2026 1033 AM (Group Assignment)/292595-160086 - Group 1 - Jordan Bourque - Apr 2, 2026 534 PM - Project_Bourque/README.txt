**Compilation on my computer (Windows 11) did not handle the pathings/environment variables properly for MPI, specifically. 
For this reason, I used a different compilation code for MPI, however other codes might need to be used if system environment variables work, or a different OS is used.**

System information can be found in the project document writeup .pdf file.

~~Code Structure Development~~
Description: General structure of the project creation. Phase3v4 was used to create SerialProject1, OMPProject1, and MPIProject 1.

Phase3v0: General construction of base requirements of the project. Most of the code was either copied from earlier labs or created simple structures and layouts.
	  This was used as my Phase 2 submission, just copied over (hence why version 0).
Phase3v1: Created the structures for imageContrast() and imageBlur(). Created a global value for alpha for imageBlur to use.
Phase3v2: Fixed some casting errors in 3v1. Fixed imageBlur. Most of the general structures are complete here.
Phase3v3: Looking back, seems to be identical to 3v2 - I may have overwritten 3v2 before saving as 3v3.
Phase3v4: Cleaned up main, included better statements and formatting. Included getting time (although incorrectly).

Compilation: gcc Phase3v4.c -o Phase3v4
Execution:   ./Phase3v4 <height> <width>

Note: Height and width are backwards here, a problem I found only near the end of development.

~~Serial Code Development~~
Description: Runs the image-editing software (Image Blur, Image Contrast Adjustment, and Image Upscale) in serial/sequential method on one thread/process. Used as the baseline to compare OMP and MPI versions of the program for execution time analysis.

SerialProject1: Took out the improper OMP timing method.
SerialProject2: Included proper clock_gettime() method. Cleaned up some print statements.
SerialProject3: Created a case 3 (to run all the methods sequentially) and fixed the print statement to be the same as OMPProject3 and MPIProject3. Fixed %.9f formatting. 
                I created this version after I had completed OMPProject3 (but started OMPProject 1 after SerialProject2 was completed).

Compilation: gcc SerialProject3.c -o SerialProject3
Execution:   ./Phase3v3 <width> <height> <runs> <mode>

~~Open MP Code Development~~
Description: Uses OpenMP, omp parallel collapse(2) and schedule(static), to process portions of the total image-editing workload as independent threads.

OMPProject1: Copied directly from Phase3v4 (likely should have called this OMPProject0).
OMPProject2: Integrated timing in OMP and created better print statements.
OMPProject3: Created a case 3 (to run all the methods sequentially) and fixed the print statement to be the same as SerialProject3 and MPIProject3. Fixed %.9f formatting.

Compilation: gcc -O3 -fopenmp OMPProject3.c -o OMPProject3
Execution:   ./OMPProject3 <width> <height> <mode> <runs> <threads>

~~MPI Code Development~~
Description: Uses MPI, Scatter and Gather, to process portions of the total image-editing workload as independent processes.

MPIProject1: Created some basic starting structures for communication in MPI.
MPIProject2: Finished a correct implementation of scatter and gather for modes 0 and 1. Mode 2 would not have worked well, though I did not think to test all modes at the 
	     time. I also did not realize that I needed a mode 3 yet, which changed everything for version 3.
MPIProject3: Spent a massive amount of time restructuring and getting MPI to work properly for case 3. I likely should have included more debug statements and versions for  
             this, but due to frustration and tunnel vision - I did not. 

Compilation: gcc -O3 MPIProject3.c -o MPIProject3 -I"C:\Program Files (x86)\Microsoft SDKs\MPI\Include" -L"C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64" -lmsmpi
Execution:   mpiexec -n <processes> MPIProject3 <width> <height> <mode> <runs>