#include <stdio.h>
#include <math.h>
#include "cpgplot.h"
#include "delaydata.inc"
#define	TOTALLAG	512
#define	NSPEC	16
#define	NSS		1
#define	NCPP	256	
#define	MAX_SS	16
#define	MAX_ANT 5
#define	MAX_BL	MAX_ANT*(MAX_ANT-1)/2
#define	MAX_VAR	2*(MAX_BL*MAX_SS + MAX_ANT - 1)

MAIN__(argc, argv)
	long	argc;				/* Number of Arguments */
	char	**argv;				/* Pointer of Arguments */
{
	long	start;
	long	start_integ;
	double	rf[MAX_SS];			/* Observing Frequency [MHz] */
	double	freq_incr[MAX_SS];	/* Frequency increment [MHz] */
	double	fsample;			/* Sampling Freq. [MHz] */
	double	cpp_len;			/* Duration of Compressed PP [sec] */
	float	vis_r[MAX_BL*MAX_SS*NCPP*NSPEC];
	float	vis_i[MAX_BL*MAX_SS*NCPP*NSPEC];
	float	*vr_ptr[MAX_BL];
	float	*vi_ptr[MAX_BL];
	float	*vr_bl_ptr;
	float	*vi_bl_ptr;
	float	vis_amp[NCPP*2][NSPEC*2];
	double	vis_max;
	long	ncpp;
	struct	delay_data	delay_ptr;
	long	blnum;
	long	antnum;
	long	icon;				/* CONDITION CODE */
	long	bl_index;
	long	ss_index;
	long	time_index;
	long	spec_index;
	long	delay_index;
	long	rate_index;
	long	spec_num[NSS];
	long	var_num;
	float	vis_snr[MAX_BL];
	float	gff_result[MAX_VAR];
	float	gff_err[MAX_VAR];
	float	afact;
	double	coeff[5];
	double	bl_delay[MAX_BL];
	double	bl_rate[MAX_BL];
	double	bl_delay_err[MAX_BL];
	double	bl_rate_err[MAX_BL];
	double	ant_delay[MAX_ANT];
	double	ant_rate[MAX_ANT];
	double	ant_delay_err[MAX_ANT];
	double	ant_rate_err[MAX_ANT];
	long	i, j;
	float	xaxis[NSS];
	float	ydata[NSS];
	float	y_ul[NSS];
	float	y_ll[NSS];
/*
-------------------------------- OUTPUT FILE
*/
	if(argc < 5){
		printf("USAGE : gfftest [START] [INTEG] [PATH1] [NUM1] [PATH2] [NUM2] ... !!\n");
		exit(0);
	}
	blnum = (argc-3)/2;
	antnum	= (1+(long)sqrt(1.0 + 8*blnum))/2;
	printf("%d BASELINEs, %d ANTENNAE\n", blnum, antnum);
/*
-------------------------------- INPUT PARAMETERS
*/
	start	= 3600*(atoi(argv[1])/10000)
			+ 60*((atoi(argv[1])/100)%100)
			+ atoi(argv[1])%100;

	bl_index=0;

		/*-------- INITIALIZE VISIBILITY AMP --------*/
		for(time_index=0; time_index<NCPP*2; time_index++){
			for(spec_index=0; spec_index<NSPEC*2; spec_index++){
				vis_amp[time_index][spec_index] = 0.0;
			}
		}

		vr_ptr[bl_index] = &vis_r[bl_index*MAX_SS*NCPP*NSPEC];
		vi_ptr[bl_index] = &vis_i[bl_index*MAX_SS*NCPP*NSPEC];

		vr_bl_ptr	= vr_ptr[bl_index];
		vi_bl_ptr	= vi_ptr[bl_index];

		for(ss_index=0; ss_index<NSS; ss_index++){

			icon = blvis(
				argv[2*bl_index+3],	/* DIRECTORY POINTER */
				atoi(argv[2*bl_index+4]),	/* FILE NUMBER */
				ss_index+1,			/* Sub-Stream NUMBER */
				start,				/* START TIME */
				atoi(argv[2]),		/* INTEGRATION TIME */
				2*NSPEC,			/* LAG NUMBER */
				vr_bl_ptr,
				vi_bl_ptr,
				&ncpp,				/* CPP Number */
				&fsample,			/* Sampling Freq. [MHz] */
				&cpp_len,			/* CPP Length [sec] */
				&rf[ss_index],		/* Obs. Freq. [MHz] */
				&start_integ,		/* START TIME for INTEG */
				&delay_ptr
			);
			if(icon == -1){ exit(0); }

			spec_num[ss_index]	= NSPEC;
			freq_incr[ss_index]	= fsample/NSPEC/2;

			#ifdef DEBUG
			printf("SS=%d READ %d CPP DATA. \n",ss_index+1, ncpp);
			#endif
				icon = coarse_mult(
					vr_bl_ptr,
					vi_bl_ptr,
					NSPEC,				/* SPECTRUM POINT */
					NSPEC,				/* SPECTRUM DIMENSION */
					rf[ss_index],		/* INITIAL FREQUENCY [MHz] */
					freq_incr[ss_index],/* FREQUENCY INCREMENT [MHz] */
					ncpp,				/* TIME POINT */
					cpp_len,			/* TIME INCREMENT */
					vis_amp
				);

			vr_bl_ptr += ncpp*NSPEC;
			vi_bl_ptr += ncpp*NSPEC;
		}

		afact	= 1.0/(float)(ncpp * NSPEC * NSS);

		vis_max = 0.0;
		for(time_index=0; time_index<NCPP*2; time_index++){
			for(spec_index=0; spec_index<NSPEC*2; spec_index++){
				vis_amp[time_index][spec_index] *= afact;
				if( vis_amp[time_index][spec_index] > vis_max ){
					vis_max = vis_amp[time_index][spec_index];
					delay_index	= spec_index;
					rate_index	= time_index;
				}
			}
		}

		sqr_fit((double)(delay_index-NSPEC-1),
				(double)(rate_index-pow2round(ncpp)),
				(double)vis_amp[rate_index][delay_index-1],

				(double)(delay_index-NSPEC),
				(double)(rate_index-pow2round(ncpp)),
				(double)vis_amp[rate_index][delay_index],

				(double)(delay_index-NSPEC+1),
				(double)(rate_index-pow2round(ncpp)),
				(double)vis_amp[rate_index][delay_index+1],

				(double)(delay_index-NSPEC),
				(double)(rate_index-pow2round(ncpp)-1),
				(double)vis_amp[rate_index-1][delay_index],

				(double)(delay_index-NSPEC),
				(double)(rate_index-pow2round(ncpp)+1),
				(double)vis_amp[rate_index+1][delay_index],
				coeff );

		bl_delay[bl_index]	= -coeff[2]/(coeff[0]*4*fsample);
		bl_rate[bl_index]	= -1.0e6*coeff[3]
							/ (coeff[1]*4*rf[0]*pow2round(ncpp)*cpp_len);
		vis_max	= -(coeff[2]*coeff[2]/coeff[0] + coeff[3]*coeff[3]/coeff[1])/4
				+ coeff[4];
		vis_snr[bl_index]= vis_max * sqrt(5.0e5*fsample*ncpp*cpp_len*sqrt(NSS));
		bl_delay_err[bl_index] = 1.0/(fsample * vis_snr[bl_index]);
		bl_rate_err[bl_index] = 1.0e6/(rf[0]*ncpp*cpp_len*vis_snr[bl_index]);

		printf("FMAX = %10.4e SNR = %5.1f DELAY = %lf, RATE = %lf\n",
		vis_max, vis_snr[bl_index], bl_delay[bl_index], bl_rate[bl_index]);

		cpgbeg(1, "/xd", 1, 1);
		cpgenv(-cos(0.5), sin(0.5), 0.0, 1.5, 0, -2);
		cpgbbuf();
		cpgbird( vis_amp, pow2round(ncpp)*2, NSPEC*2, NSPEC*2, vis_max,
				0.5, 0.3, 1);
		cpgebuf();
		cpgend();

	return(0);
}
