#include <stdio.h>
#include <math.h>
#include "cpgplot.h"
#include "delaydata.inc"
#define	TOTALLAG	512
#define	NSPEC	16
#define	NBBC	16
#define	NCPP	128	
#define	MAX_ANT 10
#define	MAX_BL	MAX_ANT*(MAX_ANT-1)/2

MAIN__(argc, argv)
	long	argc;				/* Number of Arguments */
	char	**argv;				/* Pointer of Arguments */
{
	long	start;
	long	start_integ;
	double	rf;					/* Observing Frequency */
	double	fsample;			/* Sampling Freq. [MHz] */
	double	cpp_len;			/* Duration of Compressed PP [sec] */
	float	vis_r[MAX_BL][NCPP][NSPEC];
	float	vis_i[MAX_BL][NCPP][NSPEC];
	long	ncpp;
	struct	delay_data	delay_ptr;
	double	delay_bl[MAX_BL];
	double	delay_bl_err[MAX_BL];
	double	rate_bl[MAX_BL];
	double	rate_bl_err[MAX_BL];
	double	bl_weight[MAX_BL];
	double	vis_amp[MAX_BL];
	double	vis_snr[MAX_BL];
	double	vis_phs[MAX_BL];
	double	ant_delay[MAX_ANT];
	double	ant_delay_err[MAX_ANT];
	double	ant_rate[MAX_ANT];
	double	ant_rate_err[MAX_ANT];
	double	ant_acc[MAX_ANT];
	long	blnum;
	long	antnum;
	long	icon;				/* CONDITION CODE */
	long	i;
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

	for(i=0; i<blnum; i++){
		icon = blvis(
			argv[2*i+3],		/* DIRECTORY POINTER */
			atoi(argv[2*i+4]),	/* FILE NUMBER */
			1,					/* BBC NUMBER */
			start,				/* START TIME */
			atoi(argv[2]),		/* INTEGRATION TIME */
			2*NSPEC,			/* LAG NUMBER */
			vis_r[i],			/* VISIBILITY (real) */
			vis_i[i],			/* VISIBILITY (imag) */
			&ncpp,				/* CPP Number */
			&fsample,			/* Sampling Freq. [MHz] */
			&cpp_len,			/* CPP Length [sec] */
			&rf,				/* Obs. Freq. [MHz] */
			&start_integ,		/* START TIME for INTEG */
			&delay_ptr
		);
		if(icon == -1){ exit(0); }
		printf("READ %d CPP DATA. \n", ncpp);
		icon = coarse(
			vis_r[i],			/* VISIBILITY (real) */
			vis_i[i],			/* VISIBILITY (imag) */
			NSPEC,				/* SPECTRUM POINT */
			NSPEC,				/* SPECTRUM DIMENSION */
			rf,					/* INITIAL FREQUENCY [MHz] */
			fsample/NSPEC/2,	/* FREQUENCY INCREMENT [MHz] */
			ncpp,				/* TIME POINT */
			cpp_len,			/* TIME INCREMENT */
			&vis_amp[i],		/* VISIBILITY AMP */
			&vis_phs[i],		/* VISIBILITY PHASE */
			&vis_snr[i],		/* VISIBILITY SNR */
			&delay_bl[i],		/* BASELINE DELAY [microsec]*/
			&delay_bl_err[i],	/* BASELINE DELAY ERROR */
			&rate_bl[i],		/* BASELINE DELAY RATE [picosec/sec] */
			&rate_bl_err[i]		/* BASELINE DELAY RATE ERROR */
		);
	}

	closure_solve( antnum, delay_bl, delay_bl_err, ant_delay, ant_delay_err);
	closure_solve( antnum, rate_bl, rate_bl_err, ant_rate, ant_rate_err);

	for(i=0; i<blnum; i++){
		vis_phs[i] = -vis_phs[i];
		bl_weight[i]= vis_snr[i];
	}

	for(i=0; i<antnum; i++){
		ant_delay[i]	= -ant_delay[i];
		ant_rate[i]		= -ant_rate[i];
		ant_acc[i]		= 0.0;
	}

	printf("-------- INITIAL PARAMETER --------\n");
	for(i=0; i<blnum; i++){
		printf("VIS AMP and PHS for BL %d = %12.4e  %6.2lf\n",
				i, vis_amp[i], vis_phs[i]);
	}
	for(i=0; i<antnum; i++){
		printf("ANT %d :  DELAY=%9.5lf, RATE=%9.5lf, ACC=%9.5lf\n",
				i, ant_delay[i], ant_rate[i], ant_acc[i]);
	}

	icon = gff(
			vis_r,				/* VISIBILITY (real) */
			vis_i,				/* VISIBILITY (imag) */
			NSPEC,				/* SPECTRUM POINT */
			NSPEC,				/* SPECTRUM DIMENSION */
			rf,					/* INITIAL FREQUENCY [MHz] */
			fsample/NSPEC/2,	/* FREQUENCY INCREMENT [MHz] */
			ncpp,				/* TIME POINT */
			NCPP,				/* TIME DIMENSION */
			cpp_len,			/* TIME INCREMENT */
			antnum,				/* ANTENNA NUMBER */
			bl_weight,			/* BASELINE WEIGHT */
			vis_amp,			/* VISIBILITY AMP */
			vis_phs,			/* VISIBILITY PHASE [rad] */
			ant_delay,			/* ANTENNA DELAY */
			ant_rate 			/* ANTENNA DELAY RATE */
		);

	#ifdef DEBUG
	icon = gff_acc(
			vis_r,				/* VISIBILITY (real) */
			vis_i,				/* VISIBILITY (imag) */
			NSPEC,				/* SPECTRUM POINT */
			NSPEC,				/* SPECTRUM DIMENSION */
			rf,					/* INITIAL FREQUENCY [MHz] */
			fsample/NSPEC/2,	/* FREQUENCY INCREMENT [MHz] */
			ncpp,				/* TIME POINT */
			NCPP,				/* TIME DIMENSION */
			cpp_len,			/* TIME INCREMENT */
			antnum,				/* ANTENNA NUMBER */
			bl_weight,			/* BASELINE WEIGHT */
			vis_amp,			/* VISIBILITY AMP */
			vis_phs,			/* VISIBILITY PHASE [rad] */
			ant_delay,			/* ANTENNA DELAY */
			ant_rate, 			/* ANTENNA DELAY RATE */
			ant_rate 			/* ANTENNA DELAY ACCELERATION RATE */
		);
	#endif

	printf("-------- RESULT --------\n");
	if(icon != 0){
		printf("WARNING :  ICON = %d\n", icon);
	}
	for(i=0; i<blnum; i++){
		printf("VIS AMP and PHS for BL %d = %12.4e  %6.2lf\n",
				i, vis_amp[i], vis_phs[i]);
	}
	for(i=0; i<antnum; i++){
		printf("ANT %d :  DELAY=%9.5lf, RATE=%9.5lf, ACC=%9.5lf\n",
				i, ant_delay[i], ant_rate[i], ant_acc[i]);
	}

	return(0);
}
