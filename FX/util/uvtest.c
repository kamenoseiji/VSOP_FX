#include <stdio.h>
#include <math.h>
#include "cpgplot.h"
#include "delaydata.inc"
#define	MAX_TIME 43200
#define	MAX_ANT 4
#define	MAX_BL	MAX_ANT*(MAX_ANT-1)/2

MAIN__(argc, argv)
	long	argc;				/* Number of Arguments */
	char	**argv;				/* Pointer of Arguments */
{
	double	x[MAX_ANT], y[MAX_ANT], z[MAX_ANT];
	double	u[MAX_ANT], v[MAX_ANT], w[MAX_ANT];
	double	ra;					/* Right Ascension [rad] */
	double	dec;				/* Declination [rad] */
	double	ha;					/* Hour Angle [rad] */
	double	gmst;				/* Greenwidge Sidereal Time [rad] */
	double	ut1utc;				/* UT1 - UTC [sec] */
	double	mjd;				/* Modified Julian Date */
	double	lambda;				/* Wavelength [m] */
	double	time_incr;			/* Time Increment [sec] */
	long	time_index;			/* Index of Time */
	long	ant_num;			/* Number of Antenna */
	long	bl_num;				/* Number of Baseline */
	long	ant_index;			/* Index of Antenna */
	long	ant1, ant2;			/* Antenna Index for Baseline */
	long	bl_index;			/* Baseline Index */
	float	u_bl;
	float	v_bl;

	/*----- KASHIMA34 ------ KAGOSHIMA ----- MIZUSAWA ------ NOBEYAMA ----*/
x[0]=-3997649.2400;	x[1]=-3537007.8900;	x[2]=-3857236.0300;	x[3]=-3871023.4900;
y[0]= 3276690.8100;	y[1]= 4140258.0200;	y[2]= 3108803.3100;	y[3]= 3428106.8000;
z[0]= 3724278.8900;	z[1]= 3309951.0700;	z[2]= 4003883.1200;	z[3]= 3724039.5000;

	ra = 4.5;
	dec= 0.5;
	lambda = 0.01;
	ut1utc = 27.0;
	time_incr = 240.0;
	ant_num	= 4;
	bl_num	= ant_num * (ant_num - 1) / 2;

	cpgbeg( 1, "/xd", 1, 1);
	cpgenv( 2.0e8, -2.0e8, -2.0e8, 2.0e8, 1, 1);
/*
	cpgbbuf();
*/
	for(time_index=0; time_index<MAX_TIME/time_incr; time_index++){
		mjd = 51544.0 + (double)(time_index)*time_incr/86400;
		mjd2gmst(mjd, ut1utc, &gmst);
		ha	= gmst - ra;
		for(ant_index=0; ant_index<ant_num; ant_index++){
			xyz2uvw(x[ant_index],	y[ant_index],	z[ant_index],
					ha,				dec,			lambda,
					&u[ant_index],	&v[ant_index],	&w[ant_index]);
		}

		for(bl_index=0; bl_index<bl_num; bl_index++){
			bl2ant(bl_index, &ant1, &ant2);
			u_bl	= u[ant1] - u[ant2];
			v_bl	= v[ant1] - v[ant2];
			cpgpt(1, &u_bl, &v_bl, 1);
			u_bl	*= -1;
			v_bl	*= -1;
			cpgpt(1, &u_bl, &v_bl, 1);
		}
	}
/*
	cpgebuf();
*/
	cpgend();
	return(0);
}
