/*************************************************
**	PGTEST : Demonstration Program for PGPLOT	**	
**												**
**	AUTHOR : KAMENO Seiji						**	
**************************************************/

#include <stdio.h>
#include <math.h>
#include "cpgplot.h"
#define	PI2		6.28318530717959
#define	NLAG	256
#define A		-1.50081977550763021826
#define B		1.078151260504201680686
#define C		1.009166436154484495286
#define D		1.135073779795686719636

MAIN__(argc, argv)
	int		argc;			/* Number of Arguments			*/
	char	**argv;			/* Pointer of Arguments			*/
{
	float	xmin, xmax;		/* Window Minimum and Maximum	*/
	float	ymin, ymax;		/* Window Minimum and Maximum	*/
	float	*volt;			/* Voltage Data					*/
	float	mean;			/* Mean for Random				*/
	float	sigma;			/* Standard Dispersion			*/
	float	rr[2*NLAG];		/* Correlation Function (real)	*/
	float	rr2[2*NLAG];	/* Correlation Function (real)	*/
	float	ri[2*NLAG];		/* Correlation Function (imag)	*/
	float	v1, v2;
	float	x_data[NLAG];	/* LAG 							*/
	float	amp[NLAG];		/* Amplitude 					*/
	float	amp2[NLAG];		/* Amplitude 					*/
	float	ratio[NLAG];	/* Amplitude 					*/
	float	amp_max;		/* Amplitude MAX				*/
	float	hh_weight;		/* Weight for <HH>				*/
	float	weight[4][4];	/* Weight Table					*/
	float	afact;
	float	psi;
	float	ep;
	float	rho;
	int		nfft;			/* Number of FFT Point			*/
	int		ndim;			/* FFT Dimension				*/
	int		isn;			/* FFT Direction				*/
	int		icon;			/* Condition Code				*/
	int		sample_num;		/* Number of Samples			*/
	int		index;			/* Index Number for Random		*/
	int		lag_index;		/* Index Number for LAG			*/
	int		sample_index;	/* Index Number for Sample		*/
	int		err_code;		/* Error Code					*/
	int		index2;

	/*-------- CHECK FOR INPUT ARGUMENTS --------*/
	if(argc < 3){
		printf("USAGE : spectest [device] [HH_WEIGHT] [sample number]!!\n");
		printf("  [device] : /xw  -> X-Window \n");
		printf("             /xd  -> pgdisp Window (run pgdisp first !!)\n");
		printf("             /ps  -> PostScript File (landscape)\n");
		printf("             /vps -> PostScript File (portrait)\n");
		exit(0);
	}
	hh_weight	= (float)atof( argv[2] );

	sample_num = atoi(argv[3]);
	psi			= (float)erf(sqrt(0.5));	
	ep			= (float)exp(-0.5);
	
	afact = 1.0/( psi + hh_weight*hh_weight*(1.0 - psi) );
	if( sample_num < NLAG ){
		printf("WARNING : [sample number] must be >= %d !!\n", NLAG);
		exit(0);
	}

	/*-------- WEIGHT TABLE --------*/
#ifdef XFCORR
	weight[0][0]	= hh_weight;
	weight[0][1]	= 1.0;
	weight[0][2]	=-1.0;
	weight[0][3]	=-hh_weight;
	weight[1][0]	= 1.0;
	weight[1][1]	= 0.0;
	weight[1][2]	= 0.0;
	weight[1][3]	=-1.0;
	weight[2][0]	=-1.0;
	weight[2][1]	= 0.0;
	weight[2][2]	= 0.0;
	weight[2][3]	= 1.0;
	weight[3][0]	=-hh_weight;
	weight[3][1]	=-1.0;
	weight[3][2]	= 1.0;
	weight[3][3]	= hh_weight;
#endif

	weight[0][0]	= hh_weight*hh_weight;
	weight[0][1]	= hh_weight;
	weight[0][2]	=-hh_weight;
	weight[0][3]	=-hh_weight*hh_weight;
	weight[1][0]	= hh_weight;
	weight[1][1]	= 1.0;
	weight[1][2]	=-1.0;
	weight[1][3]	=-hh_weight;
	weight[2][0]	=-hh_weight;
	weight[2][1]	=-1.0;
	weight[2][2]	= 1.0;
	weight[2][3]	= hh_weight;
	weight[3][0]	=-hh_weight*hh_weight;
	weight[3][1]	=-hh_weight;
	weight[3][2]	= hh_weight;
	weight[3][3]	= hh_weight*hh_weight;

	/*-------- MAKE SAMPLES --------*/
	nfft = 2*NLAG;	index = 10293;
	mean = 0.0;		sigma = 1.0;
	volt = (float *)malloc( sample_num*sizeof(float) );
	rann1_( &mean, &sigma, &index, volt, &sample_num, &icon);

	/*-------- MAKE BANDPASS --------*/
	for(sample_index=0; sample_index<sample_num-4; sample_index++){
		volt[sample_index] += 0.300*volt[sample_index+1];
		volt[sample_index] -= 0.150*volt[sample_index+2];
		volt[sample_index] += 0.050*volt[sample_index+3];
		volt[sample_index] -= 0.012*volt[sample_index+4];
	}

#ifdef DEBUG
	for(sample_index=0; sample_index<sample_num; sample_index++){

		index = (int)(volt[sample_index] + 2.0);
		if(index < 0){	index = 0;}
		if(index > 3){	index = 3;}

		printf("FLOAT = %f,  INT = %f\n",
			volt[sample_index], weight[index][3] );
	}
#endif

	/*-------- MAKE CORRELATION FUNCTION --------*/
	for(lag_index=0; lag_index<NLAG; lag_index++){
		rr[lag_index] = 0.0;
		for(sample_index=0; sample_index<sample_num-NLAG; sample_index++){
			v1 = volt[sample_index];
			v2 = volt[sample_index+lag_index];
			rr[lag_index] += v1*v2;

			index = (int)(v1 + 2.0);
			if(index < 0){	index = 0;}
			if(index > 3){	index = 3;}

			index2 = (int)(v2 + 2.0);
			if(index2 < 0){	index2 = 0;}
			if(index2 > 3){	index2 = 3;}

			rr2[lag_index] += weight[index][index2];

		}
		rr[lag_index] /= (float)(sample_num - NLAG);
		rr2[lag_index] /= (float)(sample_num - NLAG);
	}

	/*-------- IMAG PART --------*/
	for(lag_index=0; lag_index<2*NLAG; lag_index++){
		ri[lag_index] = 0.0;
	}

	/*-------- EXPANDED LAGS --------*/
	for(lag_index=1; lag_index<NLAG; lag_index++){
		rr[2*NLAG - lag_index] = rr[lag_index];
		rr2[2*NLAG - lag_index] = rr2[lag_index];
	}
	rr[NLAG] = 0.0;
	rr2[NLAG] = 0.0;

	/*-------- VAN VLECK CORRECTION --------*/
	for(lag_index=0; lag_index<2*NLAG; lag_index++){
		rho	= afact*rr2[lag_index];
		if(rho > 0.7){
			rr2[lag_index] = A*(rho - B)*(rho - B) + C;
		} else {
			rr2[lag_index] = D*rho;
		}
	}

	/*-------- MAKE SPECTRUM --------*/
	nfft = 2*NLAG;	ndim = 1;	isn = 1; amp_max = -9999.0;
	cft_(rr, ri, &nfft, &ndim, &isn, &icon);
	for(lag_index=0; lag_index<NLAG; lag_index++){
		amp[lag_index] = sqrt(rr[lag_index]*rr[lag_index]
						+ ri[lag_index]*ri[lag_index]);
		if(amp[lag_index] > amp_max){
			amp_max = amp[lag_index];
		}
		x_data[lag_index] = (float)lag_index;
	}


	/*-------- OPEN PGPLOT DEVICE --------*/
	cpgbeg(1, argv[1], 1, 1);			/* OPEN PGPLOT DEVICE			*/
	cpgscrn(0, "White", &err_code);		/* COLOR DEFINISHON				*/
	cpgscrn(1, "Black", &err_code);		/* COLOR DEFINISHON				*/
	cpgscrn(2, "ivory", &err_code);		/* COLOR DEFINISHON				*/
	cpgscrn(3, "Black", &err_code);		/* COLOR DEFINISHON				*/
	cpgscrn(4, "Yellow", &err_code);		/* COLOR DEFINISHON				*/
	cpgeras();

	/*-------- CREATE X-AXIS DATA --------*/
	xmin	= 0.0;	xmax	= NLAG;
	ymin	= 0.0;	ymax	= 1.2*amp_max; 

	cpgsvp(0.10, 0.90, 0.07, 0.90);				/* PANEL # 1		*/
	cpglab("SPECTRUM [POINT]", "AMPLITUDE", argv[1]);	/* AXIS LABEL		*/

	/*-------- PLOT THE FIRST PANEL --------*/
	cpgsvp(0.10, 0.90, 0.05, 0.30);				/* PANEL # 1		*/
	plot_data(xmin, xmax, ymin, ymax, NLAG, x_data, amp);


	/*-------- IMAG PART --------*/
	for(lag_index=0; lag_index<2*NLAG; lag_index++){
		ri[lag_index] = 0.0;
	}
	/*-------- MAKE SPECTRUM --------*/
	nfft = 2*NLAG;	ndim = 1;	isn = 1; amp_max = -9999.0;
	cft_(rr2, ri, &nfft, &ndim, &isn, &icon);
	for(lag_index=0; lag_index<NLAG; lag_index++){
		amp2[lag_index] = sqrt(rr2[lag_index]*rr2[lag_index]
						+ ri[lag_index]*ri[lag_index]);
		if(amp2[lag_index] > amp_max){
			amp_max = amp2[lag_index];
		}
	}

	/*-------- CREATE X-AXIS DATA --------*/
	xmin	= 0.0;	xmax	= NLAG;
	ymin	= 0.0;	ymax	= 1.2*amp_max; 

	/*-------- PLOT THE FIRST PANEL --------*/
	cpgsvp(0.10, 0.90, 0.35, 0.60);				/* PANEL # 1		*/
	plot_data(xmin, xmax, ymin, ymax, NLAG, x_data, amp2);


	amp_max = -9999.0;
	for(lag_index=0; lag_index<NLAG; lag_index++){
		ratio[lag_index] = amp2[lag_index]/amp[lag_index];
		if(ratio[lag_index] > amp_max){
			amp_max = ratio[lag_index];
		}
	}

	/*-------- CREATE X-AXIS DATA --------*/
	xmin	= 0.0;	xmax	= NLAG;
	ymin	= 0.0;	ymax	= 1.2*amp_max; 

	/*-------- PLOT THE FIRST PANEL --------*/
	cpgsvp(0.10, 0.90, 0.65, 0.90);				/* PANEL # 1		*/
	plot_data(xmin, xmax, ymin, ymax, NLAG, x_data, ratio);


	cpgend();
	return(0);
}

plot_data( left, right, bottom, top, ndata, xdata_ptr, ydata_ptr )
	float	left;				/* WORLD CORRD. for LEFT END	*/
	float	right;				/* WORLD CORRD. for RIGHT END	*/
	float	bottom;				/* WORLD CORRD. for BOTTOM END	*/
	float	top;				/* WORLD CORRD. for TOP END		*/
	int		ndata;				/* NUMBER OF DATA				*/
	float	*xdata_ptr;			/* POINTER OF X-AXIS DATA		*/
	float	*ydata_ptr;			/* POINTER OF Y-AXIS DATA		*/
{
	double	x_incr, y_incr;

	cpg_incr( (double)(right-left), &x_incr);
	cpg_incr( (double)(top-bottom), &y_incr);
	cpgswin(left, right, bottom, top);	/* SET LOCAL COORDINATE			*/
	cpgsci(2);							/* SET COLOR TO BACKGROUND		*/
	cpgrect(left, right, bottom, top);	/* ERASE THE PREVIOUS LINE		*/
	cpgsci(4);							/* SET COLOR TO WHITE			*/
	cpgbox(	"G", (float)x_incr, 1,
			"G", (float)y_incr, 1);		/* PLOT GRID					*/
	cpgsci(1);							/* SET COLOR TO WHITE			*/
	cpgbox(	"BCNTS", (float)x_incr, 10,
			"BCNTSV",(float)y_incr, 10);/* WINDOW FRAME					*/
	cpgsci(3);							/* SET COLOR TO GREEN			*/
	cpgline( ndata, xdata_ptr, ydata_ptr );	/* PLOT THE DATA			*/
	cpgsci(1);							/* SET COLOR TO WHITE			*/

	return;
}
