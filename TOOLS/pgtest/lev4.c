/*************************************************
**	PGTEST : Demonstration Program for PGPLOT	**	
**												**
**	AUTHOR : KAMENO Seiji						**	
**************************************************/

#include <stdio.h>
#include <math.h>
#include "cpgplot.h"
#define	PI2		6.28318530717959
#define	NDATA	1024
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
	float	x_data[NDATA];	/* X-DATA to plot				*/
	float	y_data[NDATA];	/* Y-DATA to plot				*/
	double	hh_weight;		/* Weight for <HH>				*/
	double	afact;
	double	psi;
	double	rho;
	double	rho4;
	int		data_index;		/* Index Number for LAG			*/
	int		err_code;		/* Error Code					*/

	/*-------- CHECK FOR INPUT ARGUMENTS --------*/
	if(argc < 3){
		printf("USAGE : lev4 [device] [HH_WEIGHT] !!\n");
		printf("  [device] : /xw  -> X-Window \n");
		printf("             /xd  -> pgdisp Window (run pgdisp first !!)\n");
		printf("             /ps  -> PostScript File (landscape)\n");
		printf("             /vps -> PostScript File (portrait)\n");
		exit(0);
	}
	hh_weight	= (double)atof( argv[2] );
	psi			= (double)erf(sqrt(0.5));	
	
	afact = 1.0/(M_PI*( psi + hh_weight*hh_weight*(1.0 - psi) ));


	/*-------- MAKE SPECTRUM --------*/
	for(data_index=0; data_index<NDATA; data_index++){

		rho = (double)data_index/NDATA;
		lev4corr_( &rho, &rho4 );
		rho4	*= afact;
/*
		printf("RHO4 = %lf\n", rho4);
*/

		x_data[data_index] = (float)rho4;
		y_data[data_index] = (float)rho;
	}


	/*-------- OPEN PGPLOT DEVICE --------*/
	cpgbeg(1, argv[1], 1, 1);			/* OPEN PGPLOT DEVICE			*/
	cpgscrn(0, "White", &err_code);		/* COLOR DEFINISHON				*/
	cpgscrn(1, "Black", &err_code);		/* COLOR DEFINISHON				*/
	cpgscrn(2, "ivory", &err_code);		/* COLOR DEFINISHON				*/
	cpgscrn(3, "Blue",	&err_code);		/* COLOR DEFINISHON				*/
	cpgscrn(4, "Yellow", &err_code);		/* COLOR DEFINISHON				*/
	cpgeras();

	/*-------- CREATE X-AXIS DATA --------*/
	xmin	= 0.0;	xmax	= 1.0;
	ymin	= 0.0;	ymax	= 1.0; 

	cpgsvp(0.10, 0.90, 0.10, 0.90);				/* PANEL # 1		*/
	cpglab("QUANTIZED CORR. COEFF.", "TRUE CORR. COEFF", argv[1]);

	/*-------- PLOT THE FIRST PANEL --------*/
	cpgsvp(0.10, 0.90, 0.10, 0.90);				/* PANEL # 1		*/
	plot_data(xmin, xmax, ymin, ymax, NDATA, x_data, y_data);


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
	cpgwnad(left, right, bottom, top);	/* SET LOCAL COORDINATE			*/
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

	return(0);
}
