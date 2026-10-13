Version History

a2v-1 - took lab 5 and started working off of it. Attempting to put an hour a day into this assignment, though overwhelmed by projects. Most of the time was truly spent just getting MPI to load on my desktop at home. All is well now, will put some solid effort into this tomorrow. (A note from the future: this did not happen.)
Timestamp: 4:49pm March 25.

*Note* Did not use this version, changed the version number to -1. Reason why: is it took me a long time to finish the final project, and by the time I was done, I knew that my MPI code for the project worked well, so I wanted to use that as my baseline and build from that code to save some time.

a2v0 - the baseline MPI code for the final project.
Timestamp: ‎7:00 pm April ‎2.

a2v1 - churned through a lot of the unneeded MPI code from the project, and set up some basic structure for the assignment requirements.
Timestamp: ‎8:30pm April ‎2.

a2v2 - set up serialBaseline() and main(). Spent quite a lot of time debugging to get this to work.
Timestamp: 4:30pm April 3.

a2v3 - created mode 1 and debugged successfully. created mode 2 and failed in debugging, leading to silent crash after silent crash. I only included one photo of the silent crash, to save my sanity. 
Timestamp: 6:15pm April 3.

a2v4 - finally fixed the structure/architecture design for mode 2. Everything seems to be working now, and I seem to be under the tol limit, so I will call this version "integration.c" for submission.
Timestamp: 8:15pm April 3.

Compile command: gcc -O3 integration.c -o integration -I"C:\Program Files (x86)\Microsoft SDKs\MPI\Include" -L"C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64" -lmsmpi -lm

NOTE: This compile command is used because my environment variables aren't set correctly, or something (my other ones do - my computer just doesn't like MPI). your compile command will likely be different, especially if you are using Linux.

Example typical compile command: mpicc -fopenmp integration.c -lm -o integration

Example run commands: Windows - mpiexec -n 4 integration 1 0 2 1e-8
                      Linux   - mpirun -np 4 ./integration 4 0 2 1e-6

Machine specifications are included in the report file.

MPI implementation used: Microsoft MPI Startup Program [Version 10.1.12498.52]