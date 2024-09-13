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
	cpgscrn(0, "DarkSlateGray", &err_code);	/* COLOR DEFINISHON				*/
	cpgscrn(1, "White", &err_code);			/* COLOR DEFINISHON				*/
	cpgscrn(2, "SlateGray", &err_code);		/* COLOR DEFINISHON				*/
	cpgscrn(3, "Yellow", &err_code);		/* COLOR DEFINISHON				*/
	cpgeras();

	/*-------- CREATE X-AXIS DATA --------*/
	xmin	= 0.0;	xmax	= PI2;
	ymin	= -1.2;	ymax	= 1.2;
	for( phase_index=0; phase_index<NDATA; phase_index++){ 
		phase[phase_index]	= xmin
							+ PI2 * (xmax - xmin) * (float)phase_index/NDATA;
	}

	/*-------- LOOP FOR PLOT (REFRESH) --------*/
	for( plot_index=0; plot_index<atoi(argv[2]); plot_index++){

		cpgbbuf();							/* BEGIN OF PLOT BUFFERING		*/

		/*-------- CREATE Y-AXIS DATA --------*/
		amp	= (float)cos( (double)plot_index*0.1 );
		for( phase_index=0; phase_index<NDATA; phase_index++){ 
			volt1[phase_index] = amp*(float)sin( (double)phase[phase_index]/2 );
			volt2[phase_index] = amp*(float)sin( (double)phase[phase_index] );
			volt3[phase_index] = amp*(float)sin( (double)phase[phase_index]*2 );
			volt4[phase_index] = amp*(float)sin( (double)phase[phase_index]*4 );
		}

		/*-------- PLOT THE FIRST PANEL --------*/
		cpgsvp(0.10, 0.40, 0.10, 0.40);				/* PANEL # 1		*/
		cpglab("Phase [rad]", "Voltage", "");		/* AXIS LABEL		*/
		plot_data(0.0, PI2, -1.2, 1.2, NDATA, phase, volt1);

		/*-------- PLOT THE SECOND PANEL --------*/
		cpgsvp(0.60, 0.90, 0.10, 0.40);				/* PANEL # 2		*/
		cpglab("Phase [rad]", "Voltage", "");		/* AXIS LABEL		*/
		plot_data(0.0, PI2, -1.2, 1.2, NDATA, phase, volt2);

		/*-------- PLOT THE THIRD PANEL --------*/
		cpgsvp(0.10, 0.40, 0.60, 0.90);				/* PANEL # 3		*/
		cpglab("Phase [rad]", "Voltage", "");		/* AXIS LABEL		*/
		plot_data(0.0, PI2, -1.2, 1.2, NDATA, phase, volt3);

		/*-------- PLOT THE FOURTH PANEL --------*/
		cpgsvp(0.60, 0.90, 0.60, 0.90);				/* PANEL # 4		*/
		cpglab("Phase [rad]", "Voltage", "");		/* AXIS LABEL		*/
		plot_data(0.0, PI2, -1.2, 1.2, NDATA, phase, volt4);

		cpgebuf();							/* PLOT THE CONTENTS OF BUFFER	*/

	}/*-------- END OF PLOT LOOP --------*/

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
