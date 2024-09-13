#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cpgplot.h"
#define PI 		3.1415926535897932384626433832795
#define PI2 	6.2831853071795864769252867665590
#define MAS2RAD 4.84813681109535993589e-9
#define DEG2RAD 1.74532925199432957692e-2
#define FWHM2SIGMA 1.177410022515474691
#define NDATA 1000

MAIN__(argc, argv)
	long	argc;		/* Number of Arguments */
	char	**argv;		/* Pointer of Arguments */
{
	double	gaus_comp[6];	/* Gaussian Component Parameter */
	double	uv[2];			/* U and V */
	double	vis[2];			/* Visibility Amplitude and Phase */
	double	total_vis[2];	/* Total Visibility Real and Imag */
	long	ncomp;			/* Number of Gaussian Components */
	long	comp_index;		/* Index of Gaussian Components */
	long	i;

	/*-------- CHECK FOR INPUT PARAMETER --------*/
	if(argc < 9){
		printf("USAGE : visiv [u] [v] [Flux] [Radius] [Theta] [Maj] [Ratio] [PA] ... !!\n");
		exit(0);
	}

	ncomp	= (argc - 3)/6;			/* Number of Components */
	uv[0] = atof(argv[1]) * 1.0e6; 	/* UV in [Mega Wavelength] */
	uv[1] = atof(argv[2]) * 1.0e6; 	/* UV in [Mega Wavelength] */
	total_vis[0]	= 0.0;
	total_vis[1]	= 0.0;
/*
---------------------------------------- INPUT PARAMETERS
*/
	for( comp_index=0; comp_index<ncomp; comp_index++){

		for(i=0; i<6; i++){
			gaus_comp[i] = atof(argv[6*comp_index + i+3]);
			printf(" GAUSS [%d] = %f\n", i, gaus_comp[i]);
		}

		gaus_comp[1] *= MAS2RAD;				/* milliarcsec -> rad */
		gaus_comp[2] *= DEG2RAD;				/* degree -> rad */
		gaus_comp[3] *= (MAS2RAD / FWHM2SIGMA);	/* mas, FWHM -> rad, sigma */
		gaus_comp[5] *= DEG2RAD;				/* degree -> rad */
/*
---------------------------------------- CALC VISIBILITY 
*/
		(void)comp2vis( gaus_comp, uv, vis ); 

		total_vis[0] += vis[0] * cos( vis[1] );
		total_vis[1] += vis[0] * sin( vis[1] );
	}

	printf("REAL = %f, IMAG=%f\n", total_vis[0], total_vis[1]);
	return(0);
}
