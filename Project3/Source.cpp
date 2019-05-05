#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <stdio.h>

#define M_PI   3.14159265358979323846264338327950288

int	NowYear = 2019;		// 2019 - 2024
int	NowMonth = 0;		// 0 - 11

float	NowPrecip;		// inches of rain per month
float	NowTemp ;		// temperature this month
float	NowHeight = 5.;		// grain height in inches
int	NowNumDeer =1;		// number of deer in the current population
int NowPeople = 2;



const float GRAIN_GROWS_PER_MONTH = 8.0;
const float ONE_DEER_EATS_PER_MONTH = 0.5;
const float ONE_PEOPLE_EATS_PER_MONTH = 0.3;


const float AVG_PRECIP_PER_MONTH = 6.0;	// average
const float AMP_PRECIP_PER_MONTH = 6.0;	// plus or minus
const float RANDOM_PRECIP = 2.0;	// plus or minus noise


const float AVG_TEMP = 50.0;	// average
const float AMP_TEMP = 20.0;	// plus or minus
const float RANDOM_TEMP = 10.0;	// plus or minus noise

const float MIDTEMP = 40.0;
const float MIDPRECIP = 10.0;


omp_lock_t	Lock;
int		NumInThreadTeam;
int		NumAtBarrier;
int		NumGone;
void	InitBarrier(int);
void	WaitBarrier();

float Ranf( float, float);
int Ranf( int, int);

void GrainDeer();
void Grain();
void Watcher();
void People();

float
SQR(float x)
{
	return x * x;
}

int main(int argc, char *argv[])
{
	omp_init_lock(&Lock);
	omp_set_num_threads(4);	// same as # of sections
	InitBarrier(4);
	float ang = (30.*(float)NowMonth + 15.) * (M_PI / 180.);
	float temp = AVG_TEMP - AMP_TEMP * cos(ang);
	unsigned int seed = 0;
	float random_temp = Ranf(-RANDOM_TEMP, RANDOM_TEMP);
	//fprintf(stdout, "The rand is %5.2lf, temp ios %5.2lf\n", random_temp, temp);
	NowTemp = temp + random_temp;
	float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
	NowPrecip = precip + Ranf(-RANDOM_PRECIP, RANDOM_PRECIP);
#pragma omp parallel sections
	{
#pragma omp section
		{
			GrainDeer();
		}

#pragma omp section
		{
			Grain();
		}

#pragma omp section
		{
			Watcher();
		}

#pragma omp section
		{
			People();	// your own
		}

	}       // implied barrier -- all functions must return in order
			// to allow any of them to get past here
}


void Grain() {
	while (NowYear < 2025)
	{
		float tempFactor = exp(-SQR((NowTemp - MIDTEMP) / 10.));
		float precipFactor = exp(-SQR((NowPrecip - MIDPRECIP) / 10.));
		float temp_height = NowHeight + tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
		temp_height -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;
		temp_height -= (float)NowPeople * ONE_PEOPLE_EATS_PER_MONTH;
		if (temp_height < 0.0) {
			temp_height = 0.0;
		}
		WaitBarrier();
		NowHeight = temp_height;
		WaitBarrier();
		WaitBarrier(); 
	}
}

void GrainDeer() {
	while (NowYear < 2025)
	{
		int temp_deer = NowNumDeer;
		if (temp_deer <= int(NowHeight)) {
			temp_deer += 1;
		}
		else {
			temp_deer -= 1;
		}
		if (temp_deer > 3 * NowPeople) {
			temp_deer -= 1;
		}

		if (temp_deer < 0)
			temp_deer = 0;
		WaitBarrier();
		NowNumDeer = temp_deer;	
		WaitBarrier();
		WaitBarrier();
	}
}

void Watcher() {
	while (NowYear < 2025)
	{
		WaitBarrier();
		WaitBarrier();
		float temp_c = (5. / 9.) * (NowTemp - 32);
		float percip_c = NowPrecip * 2.54;
		float height_c = NowHeight * 2.54;
		fprintf(stdout,  "%5.2lf\t %5.2lf \t  %5.2lf \t %d \t %d \t %d\n", temp_c, percip_c, height_c, NowNumDeer, NowPeople, NowYear);
		NowMonth += 1;
		if (NowMonth >= 12) {
			NowMonth = 0;
			NowYear += 1;
		}
		float ang = (30.*(float)NowMonth + 15.) * (M_PI / 180.);
		float temp = AVG_TEMP - AMP_TEMP * cos(ang);
		unsigned int seed = 0;
		float random_temp = Ranf(-RANDOM_TEMP, RANDOM_TEMP);
		//fprintf(stdout, "The rand is %5.2lf, temp ios %5.2lf\n", random_temp, temp);
		NowTemp = temp + random_temp;
		float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
		NowPrecip = precip + Ranf(-RANDOM_PRECIP, RANDOM_PRECIP);
		if (NowPrecip < 0.)
			NowPrecip = 0.;
		WaitBarrier();
	}
}

void People(){
while (NowYear < 2025)
{
	int tempPeople = NowPeople;
	if (NowNumDeer > 2 * NowPeople || NowHeight > (float)3 * NowPeople) {
		tempPeople += 1;
	}
	else {
		tempPeople -= 1;
	}
	if (tempPeople < 0) {
		tempPeople = 0;
	}
	WaitBarrier();
	NowPeople = tempPeople;
	WaitBarrier();
	WaitBarrier();
}
}









void
InitBarrier(int n)
{
	NumInThreadTeam = n;
	NumAtBarrier = 0;
	omp_init_lock(&Lock);
}


void
WaitBarrier()
{
	omp_set_lock(&Lock);
	{
		NumAtBarrier++;
		if (NumAtBarrier == NumInThreadTeam)
		{
			NumGone = 0;
			NumAtBarrier = 0;
			// let all other threads get back to what they were doing
			// before this one unlocks, knowing that they might immediately
			// call WaitBarrier( ) again:
			while (NumGone != NumInThreadTeam - 1);
			omp_unset_lock(&Lock);
			return;
		}
	}
	omp_unset_lock(&Lock);

	while (NumAtBarrier != 0);	// this waits for the nth thread to arrive
	//{
//		fprintf(stdout, "test");
//	}
#pragma omp atomic
	NumGone++;			// this flags how many threads have returned
}


float
Ranf( float low, float high)
{
	float r = (float)rand();              // 0 - RAND_MAX

	return(low + r * (high - low) / (float)RAND_MAX);
}


int
Ranf( int ilow, int ihigh)
{
	float low = (float)ilow;
	float high = (float)ihigh + 0.9999f;

	return (int)(Ranf(low, high));}