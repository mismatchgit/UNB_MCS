Compile Command: gcc -O3 -fopenmp a1v5.c -lm -o a1

Run Command Format: ./a1 [N value] [Mode Number]

Machine Specs Used: UNB Lab Computers

N values Used: 512, 1024, 2048.

Mode Numbers:
	0	serial
	1	openMP
	2	openMPCollapse
	3	synchronizationComparison
	4	taskBasedParrellelism
	5	SIMDOnly
	6	threadsAndSIMD


Version history:
a1v1: Formatting construction.
a1v2: Some logic implemented as required in the assignment document, some formatting applied for the modes.
a1v3: More internal logic created for modes 0-2. Some formatting applied in other modes. Checksum verification found to be incorrect as a separate method, and was included within the modes themselves.
a1v4: Rough logic created for most modes (0-5). Updates needed to correct the logic and debugging required.
a1v5: (Nearly) final code version. This code was sent over to the UNB labs after debugging, and was used to run the code. Many logic errors were fixed in this edition. Some reformatting for missed assignment requirements was required. Some missing include statements were added, and matrix 
a1v6: Code which was run during/after the times were recorded. Some print statements were reformatted, and the scheduler has changed from a1v5 (due to running that scheduler).