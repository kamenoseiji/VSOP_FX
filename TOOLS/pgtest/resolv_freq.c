/*************************************************
**	PGTEST : Demonstration Program for PGPLOT	**	
**												**
**	AUTHOR : KAMENO Seiji						**	
**************************************************/

#include <stdio.h>
#include <math.h>
#include "cpgplot.h"
#define	PI2		6.28318530717959
#define	NDATA	512

MAIN__(argc, argv)
	int		argc;			/* Number of Arguments			*/
	char	**argv;			/* Pointer of Arguments			*/
{
	float	xmin, xmax;		/* Window Minimum and Maximum	*/
	float	ymin, ymax;		/* Window Minimum and Maximum	*/
	float	phase[NDATA];	/* X-Axis Data 					*/
	float	volt1[NDATA];	/* Y-Axis Data					*/
	float	volt2[NDATA];	/* Y-Axis Data					*/
	float	volt3[NDATA];	/* Y-Axis Data					*/
	float	volt4[NDATA];	/* Y-Axis Data					*/
	float	amp;			/* Amplitude 					*/
	int		phase_index;	/* Index for Phase				*/
	int		plot_index;		/* Index for Plot Refresh		*/
	int		err_code;		/* Error Code					*/

	/*-------- CHECK FOR INPUT ARGUMENTS --------*/
	if(argc < 3){
		printf("USAGE : pgtest [device] [plot number]!!\n");
		printf("  [device] : /xw  -> X-Window \n");
		printf("             /xd  -> pgdisp Window (run pgdisp first !!)\n");
		printf("             /ps  -> PostScript File (landscape)\n");
		printf("             /vps -> PostScript File (portrait)\n");
		exit(0);
	}
	if( atoi(argv[2]) < 1 ){
		printf("WARNING : [plot number] must be >= 1 !!\n");
		exit(0);
	}

	/*-------- OPEN PGPLOT DEVICE --------*/
	cpgbeg(1, argv[1], 1, 1);				/* OPEN PGPLOT DEVICE			*/
#ifdef HIDOI
	cpgscrn(0, "DarkSlateGray", &err_code);	/* COLOR DEFINISHON				*/
	cpgscrn(1, "White", &err_code);			/* COLOR DEFINISHON				*/
	cpgscrn(2, "SlateGray", &err_code);		/* COLOR DEFINISHON				*/
	cpgscrn(3, "Yellow", &err_code);		/* COLOR DEFINISHON				*/
	cpgeras();
#endif

	/*-------- CREATE X-AXIS DATA --------*/
	xmin	=  0.0;	xmax	= 3.0;
	ymin	= -3.0;	ymax	= 3.0;

	cpgslw(4.0);
	cpgsch(1.5);

	/*-------- PLOT THE FIRST PANEL --------*/
	cpgsvp(0.10, 0.90, 0.15, 0.85);				/* PANEL # 1		*/
	cpglab("Frequency [GHz]", "Angular Size [mas]", "");
	cpgswin(xmin, xmax, ymin, ymax);	/* SET LOCAL COORDINATE			*/
	cpgbox("BCLNTS", 0.0, 0, "BCNLTS", 0.0, 0);	/* WINDOW FRAME		*/


	cpgend();
	return(0);
}

#ifdef HIDOI
plot_data( left, right, bottom, top, ndata, xdata_ptr, ydata_ptr )
	float	left;				/* WORLD CORRD. for LEFT END	*/
	float	right;				/* WORLD CORRD. for RIGHT END	*/
	float	bottom;				/* WORLD CORRD. for BOTTOM END	*/
	float	top;				/* WORLD CORRD. for TOP END		*/
	int		ndata;				/* NUMBER OF DATA				*/
	float	*xdata_ptr;			/* POINTER OF X-AXIS DATA		*/
	float	*ydata_ptr;			/* POINTER OF Y-AXIS DATA		*/
{
	cpgswin(left, right, bottom, top);	/* SET LOCAL COORDINATE			*/
	cpgsci(2);							/* SET COLOR TO BACKGROUND		*/
	cpgrect(left, right, bottom, top);	/* ERASE THE PREVIOUS LINE		*/
	cpgsci(0);							/* SET COLOR TO WHITE			*/
	cpgbox("G", PI2/8, 1, "G", 0.2, 1);	/* PLOT GRID					*/
	cpgsci(1);							/* SET COLOR TO WHITE			*/
	cpgbox("BCNTS", 1.0, 10, "BCNTSV", 1.0, 10);	/* WINDOW FRAME		*/
	cpgsci(3);							/* SET COLOR TO GREEN			*/
	cpgline( ndata, xdata_ptr, ydata_ptr );	/* PLOT THE DATA			*/
	cpgsci(1);							/* SET COLOR TO WHITE			*/

	return;
}
#endif
