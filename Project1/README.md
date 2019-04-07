# Introduction
Monte Carlo simulation is used to determine the range of outcomes for a series of parameters, each of which has a probability distribution showing how likely each option is to happen. In this project, you will take a scenario and develop a Monte Carlo simulation of it, determining how likely a particular output is to happen 
# The Scenario
![alt text](https://github.com/gyb1325/CS575_Parallel_Programming/tree/master/Project1/img/img1.jpg "Logo Title Text 1")
A laser is pointed at a circle (circle, in this case). The circle is defined by a center point (xc,yc) and a radius (r). The beam comes out at a 45° angle. It bounces off the circle. Underneath, even with the laser origin, is an infinite plate. Given all of this, does the beam hit the plate?  

Normally this would be a pretty straightforward geometric calculation, but the circle is randomly changing location and size. So now, the laser beam might hit the plate or it might not, depending on the values of (xc, yc, r ). OK, since it is not certain, what is the probability that it hits the plate? This is a job for multicore Monte Carlo simulation!  

Because of the variability, the beam could miss the circle entirely (A). The circle might totally engulf the laser pointer (B). It could bounce off the circle and miss the plate entirely (C). Or, it could bounce off the circle and actually hit the plate (D).  

So, now the question is "What is the probability that the beam hits the plate?".  
1.The Range are 

| Variable | Range     |
|----------|-----------|
| xc       | -1 0- 1.0 |
| yc       | 0 0 - 2.0 |
| r        | 0.5 - 2.0 |

2.Run this for some combinations of trials and threads. Do timing for each combination. Like we talked about in the Project Notes, run each experiment some number of tries, NUMTRIES, and record just the peak performance.
Do a table and two graphs showing performance versus trials and threads.  

Chosing one of the runs (the one with the maximum number of trials would be good), tell me what you think the actual probability is.  

Compute Fp, the Parallel Fraction, for this computation.  

# The Algorithm
```cpp
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

// setting the number of threads:
#ifndef NUMT
#define NUMT		1
#endif

// setting the number of trials in the monte carlo simulation:
#ifndef NUMTRIALS
#define NUMTRIALS	1000000
#endif

// how many tries to discover the maximum performance:
#ifndef NUMTRIES
#define NUMTRIES	10
#endif

// ranges for the random numbers:
const float XCMIN =	-1.0;
const float XCMAX =	 1.0;
const float YCMIN =	 0.0;
const float YCMAX =	 2.0;
const float RMIN  =	 0.5;
const float RMAX  =	 2.0;

// function prototypes:
float		Ranf( float, float );
int		Ranf( int, int );
void		TimeOfDaySeed( );

// main program:
int
main( int argc, char *argv[ ] )
{
#ifndef _OPENMP
	fprintf( stderr, "No OpenMP support!\n" );
	return 1;
#endif

	TimeOfDaySeed( );		// seed the random number generator

	omp_set_num_threads( NUMT );	// set the number of threads to use in the for-loop:`
	
	// better to define these here so that the rand() calls don't get into the thread timing:
	float *xcs = new float [NUMTRIALS];
	float *ycs = new float [NUMTRIALS];
	float * rs = new float [NUMTRIALS];

        // fill the random-value arrays:
        for( int n = 0; n < NUMTRIALS; n++ )
        {       
                xcs[n] = Ranf( XCMIN, XCMAX );
                ycs[n] = Ranf( YCMIN, YCMAX );
                rs[n] = Ranf(  RMIN,  RMAX ); 
        }       

        // get ready to record the maximum performance and the probability:
        float maxPerformance = 0.;      // must be declared outside the NUMTRIES loop
        float currentProb;              // must be declared outside the NUMTRIES loop

        // looking for the maximum performance:
        for( int t = 0; t < NUMTRIES; t++ )
        {
                double time0 = omp_get_wtime( );

                int numHits = 0;
		#pragma omp parallel for default(none) shared(xcs,ycs,rs) reduction(+:numHits)
		for( int n = 0; n < NUMTRIALS; n++ )
		{
			// randomize the location and radius of the circle:
			float xc = xcs[n];
			float yc = ycs[n];
			float  r =  rs[n];

			// solve for the intersection using the quadratic formula:
			float a = 2.;
			float b = -2.*( xc + yc );
			float c = xc*xc + yc*yc - r*r;
			float d = b*b - 4.*a*c;
If d is less than 0., then the circle was completely missed. (Case A) Continue on to the next trial in the for-loop.


			// hits the circle:
			// get the first intersection:
			d = sqrt( d );
			float t1 = (-b + d ) / ( 2.*a );	// time to intersect the circle
			float t2 = (-b - d ) / ( 2.*a );	// time to intersect the circle
			float tmin = t1 < t2 ? t1 : t2;		// only care about the first intersection
If tmin is less than 0., then the circle completely engulfs the laser pointer. (Case B) Continue on to the next trial in the for-loop.


			// where does it intersect the circle?
			float xcir = tmin;
			float ycir = tmin;

			// get the unitized normal vector at the point of intersection:
			float nx = xcir - xc;
			float ny = ycir - yc;
			float n = sqrt( nx*nx + ny*ny );
			nx /= n;	// unit vector
			ny /= n;	// unit vector

			// get the unitized incoming vector:
			float inx = xcir - 0.;
			float iny = ycir - 0.;
			float in = sqrt( inx*inx + iny*iny );
			inx /= in;	// unit vector
			iny /= in;	// unit vector

			// get the outgoing (bounced) vector:
			float dot = inx*nx + iny*ny;
			float outx = inx - 2.*nx*dot;	// angle of reflection = angle of incidence`
			float outy = iny - 2.*ny*dot;	// angle of reflection = angle of incidence`

			// find out if it hits the infinite plate:
			float t = ( 0. - ycir ) / outy;
If t is less than 0., then the reflected beam went up instead of down. Continue on to the next trial in the for-loop.

Otherwise, this beam hit the infinite plate. (Case D) Increment the number of hits and continue on to the next trial in the for-loop.


		}
		double time1 = omp_get_wtime( );
		double megaTrialsPerSecond = (double)NUMTRIALS / ( time1 - time0 ) / 1000000.;
		if( megaTrialsPerSecond > maxPerformance )
			maxPerformance = megaTrialsPerSecond;
		currentProb = (float)numHits/(float)NUMTRIALS;
	}
```
Print out: (1) the number of threads, (2) the number of trials, (3) the probability of hitting the plate, and (4) the MegaTrialsPerSecond. Printing this as a single line with tabs between the numbers is nice so that you can import these lines right into Excel.  
# Helper Functions:
To choose a random number between two floats or two ints, use:  
```cpp
#include <stdlib.h>

float
Ranf( float low, float high )
{
        float r = (float) rand();               // 0 - RAND_MAX
        float t = r  /  (float) RAND_MAX;       // 0. - 1.

        return   low  +  t * ( high - low );
}

int
Ranf( int ilow, int ihigh )
{
        float low = (float)ilow;
        float high = ceil( (float)ihigh );

        return (int) Ranf(low,high);
}

void
TimeOfDaySeed( )
{
	struct tm y2k = { 0 };
	y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;

	time_t  timer;
	time( &timer );
	double seconds = difftime( timer, mktime(&y2k) );
	unsigned int seed = (unsigned int)( 1000.*seconds );    // milliseconds
	srand( seed );
}
```