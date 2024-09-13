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

MAIN__(argc, argv)
	int		argc;			/* Number of Arguments			*/
	char	**argv;			/* Pointer of Arguments			*/
{
	float	xmin, xmax;		/* Window Minimum and Maximum	*/
	float	ymin, ymax;		/* Window Minimum and Maximum	*/
	float	x_data[NDATA];	/* X-DATA to plot				*/
	float	y_data[NDATA];	/* Y-DATA to plot				*/
	double	theta;			/* Weight for <HH>				*/
	double	phase;
	double	delay;
	double	power;
	int		data_index;		/* Index Number for LAG			*/
	int		err_code;		/* Error Code					*/

	/*-------- CHECK FOR INPUT ARGUMENTS --------*/
	if(argc < 3){
		printf("USAGE : beam [device] [BL]!!\n");
		printf("  [device] : /xw  -> X-Window \n");
		printf("             /xd  -> pgdisp Window (run pgdisp first !!)\n");
		printf("             /ps  -> PostScript File (landscape)\n");
		printf("             /vps -> PostScript File (portrait)\n");
		exit(0);
	}

	/*-------- MAKE SPECTRUM --------*/
	for(data_index=0; data_index<NDATA; data_index++){

		theta	= (double)data_index * PI2/NDATA - M_PI;
		delay	= atof(argv[2]) * sin(theta);
		phase	= PI2 * delay;

		power	= 0.5 + 0.5 * cos(phase);
		power	= power * power;

		x_data[data_index] = (float)power*sin(theta);
		y_data[data_index] =-(float)power*cos(theta);
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
	xmin = -1.0;	xmax = 1.0;
	ymin = -1.0;		ymax = 1.0; 

	cpgsvp(0.10, 0.90, 0.10, 0.90);				/* PANEL # 1		*/
	cpglab("BEAM PATTERN", "", "");
/*
	cpglab("BASELINE ANGLE", "GAIN", argv[1]);
*/

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
	int		data_index;
	float	theta;
	double	x_incr, y_incr;

	cpg_incr( (double)(right-left), &x_incr);
	cpg_incr( (double)(top-bottom), &y_incr);
	cpgwnad(left, right, bottom, top);	/* SET LOCAL COORDINATE			*/

#ifdef HIDOI
	cpgsci(2);							/* SET COLOR TO BACKGROUND		*/
	cpgrect(left, right, bottom, top);	/* ERASE THE PREVIOUS LINE		*/
	cpgsci(4);							/* SET COLOR TO WHITE			*/
	cpgbox(	"G", (float)x_incr, 1,
			"G", (float)y_incr, 1);		/* PLOT GRID					*/
	cpgsci(1);							/* SET COLOR TO WHITE			*/
	cpgbox(	"BCNTS", (float)x_incr, 10,
			"BCNTSV",(float)y_incr, 10);/* WINDOW FRAME					*/
#endif
	cpgsci(2);							/* SET COLOR TO BACKGROUND		*/
	cpgsfs(1);							/* FILL SOLID					*/
	cpgcirc(0.0, 0.0, 1.0);
	cpgsci(4);							/* SET COLOR TO WHITE			*/
	cpgsfs(2);							/* FILL OUTLINE					*/
	cpgcirc(0.0, 0.0, 1.0);

	for(data_index=0; data_index<36; data_index++){
		theta = PI2 * (float)data_index/36.0;
		cpgmove(0.0, 0.0);
		cpgdraw(sin(theta), -cos(theta));
	}
	for(data_index=0; data_index<10; data_index++){
		cpgcirc(0.0, 0.0, (float)(data_index)/10.0 );
	}

	cpgsci(3);							/* SET COLOR TO GREEN			*/
	cpgslw(10.0);
	cpgline( ndata, xdata_ptr, ydata_ptr );	/* PLOT THE DATA			*/
	cpgsci(1);							/* SET COLOR TO WHITE			*/

	return;
}
