#include <omp.h>
#include <stdio.h>
#include <math.h>

#define NUMT1	         1
#define NUMT4		     4
#define ARRAYSIZE       1000000	// you decide
#define NUMTRIES        200	// you decide

float A[ARRAYSIZE];
float B[ARRAYSIZE];
float C[ARRAYSIZE];

int
main()
{
#ifndef _OPENMP
	fprintf(stderr, "OpenMP is not supported here -- sorry.\n");
	return 1;
#endif

	omp_set_num_threads(NUMT1);
	fprintf(stderr, "Using %d threads\n", NUMT1);

	double maxMegaMults1 = 0.;

	for (int t = 0; t < NUMTRIES; t++)
	{
		double time0 = omp_get_wtime();

#pragma omp parallel for
		for (int i = 0; i < ARRAYSIZE; i++)
		{
			C[i] = A[i] * B[i];
		}

		double time1 = omp_get_wtime();
		double megaMults = (double)ARRAYSIZE / (time1 - time0) / 1000000.;
		if (megaMults > maxMegaMults1)
			maxMegaMults1 = megaMults;
	}

	printf("The thread number is %d, Peak Performance = %8.2lf MegaMults/Sec\n", NUMT1, maxMegaMults1);
	// note: %lf stands for "long float", which is how printf prints a "double"
	//        %d stands for "decimal integer", not "double"



	omp_set_num_threads(NUMT4);
	fprintf(stderr, "Using %d threads\n", NUMT4);

	double maxMegaMults4 = 0.;

	for (int t = 0; t < NUMTRIES; t++)
	{
		double time0 = omp_get_wtime();

#pragma omp parallel for
		for (int i = 0; i < ARRAYSIZE; i++)
		{
			C[i] = A[i] * B[i];
		}

		double time1 = omp_get_wtime();
		double megaMults = (double)ARRAYSIZE / (time1 - time0) / 1000000.;
		if (megaMults > maxMegaMults4)
			maxMegaMults4 = megaMults;
	}

	printf("The thread number is %d, Peak Performance = %8.2lf MegaMults/Sec\n", NUMT4, maxMegaMults4);

	float speedup = maxMegaMults4 / maxMegaMults1;
	printf("The speed up should be %8.2lf\n", speedup);
	float Fp = (4. / 3.)*(1. - (1. / speedup));
	printf("The parallel Fraction is %8.2lf\n", Fp);

	return 0;
}