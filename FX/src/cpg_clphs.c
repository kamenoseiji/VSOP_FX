/*********************************************************
**	CPG_CLPHS.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <cpgplot.h>
#include <math.h>
#define	NYWIN	4
#define	SECDAY	86400

int	cpg_clphs( obs_name, obj_name, stn_num, stn_list,
		time_num, start_mjd, stop_mjd, time_list, ss_num,
		clphs_ptr, clerr_ptr )

	char	*obs_name;				/* Observation Name				*/
	char	*obj_name;				/* Object Name					*/
	int		stn_num;				/* Number of Stations			*/
	char	stn_list[][16];			/* Station Lists				*/
	int		time_num;				/* Number of Time Data			*/
	double	start_mjd;				/* Integ Start [MJD ]			*/
	double	stop_mjd;				/* Integ Stop [MJD ]			*/
	float	*time_list;				/* Time Data [Sec of Day]		*/
	int		ss_num;					/* Number of Sub-Stream			*/
	int		*clphs_ptr;				/* Pointer of Visibility Data	*/
	int		*clerr_ptr;				/* Pointer of Visibility Data	*/
{
	/*-------- PGPLOT FRAME VARIABLE --------*/
	float	xmin, xmax, ymin, ymax;	/* Corner of the Frame			*/
	char	pg_device[32];			/* PGPLOT Device Name			*/
	float	win_top, win_bottom;	/* World Coord. of Window		*/
	float	ywin_incr;				/* W-Axis Increment				*/

	/*-------- INDEX --------*/
	int		cl_index;				/* Closure Index				*/
	int		ss_index;				/* Sub-Stream Index				*/
	int		time_index;				/* Index for time				*/
	int		win_index;				/* Window Index					*/
	int		err_code;

	/*-------- IDINTIFIER --------*/
	int		ant1, ant2, ant3;		/* Antenna Pair					*/

	/*-------- TOTAL NUMBER --------*/
	int		cl_num;					/* Closure Number				*/

	/*-------- General Variables --------*/
	char	text[32];				/* Text to Plot					*/
	float	*clphs;					/* Visibility Phase				*/
	float	*clerr;					/* Visibility Phase				*/
	float	*cl_top;				/* Visibility Amplitude			*/
	float	*cl_btm;				/* Visibility Amplitude			*/

	cl_num = (stn_num* (stn_num - 1)* (stn_num - 2))/6;
	cl_top = (float *)malloc(time_num* sizeof(float) );
	cl_btm = (float *)malloc(time_num* sizeof(float) );

	err_code	= 32;
	cpgqinf("FILE", pg_device, &err_code);

	err_code = 0;
	/*-------- OPEN PGPLOT DEVICE --------*/
	if( strstr( pg_device, "cps") != NULL ){
		cpgscrn(0, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(1, "Black", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(2, "ivory", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(3, "Blue", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(4, "Green", &err_code);			/* COLOR DEFINISHON */

	} else if( strstr( pg_device, "ps") == NULL ){
		cpgscrn(0, "DarkSlateGray", &err_code);	/* COLOR DEFINISHON */
		cpgscrn(1, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(2, "SlateGray", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(3, "Yellow", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(4, "Cyan", &err_code);			/* COLOR DEFINISHON */
	} else {
		cpgscrn(0, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(1, "Black", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(2, "Gray", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(3, "Black", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(4, "Black", &err_code);			/* COLOR DEFINISHON */
	}

	cpgeras();

	ywin_incr = 0.9 / NYWIN;

	xmin = (float)( (start_mjd*1.1 - stop_mjd*0.1 - (int)start_mjd)* SECDAY);
	xmax = (float)( (stop_mjd*1.1 - start_mjd*0.1 - (int)start_mjd)* SECDAY);

	ss_index = 0;
	for(cl_index=0; cl_index<cl_num; cl_index++){
		cl2ant(cl_index, &ant3, &ant2, &ant1);
		win_index = cl_index % NYWIN;
		win_bottom	= 0.05 + ywin_incr* win_index ;
		win_top		= 0.05 + ywin_incr* (win_index + 1) ;
		if(cl_index % (NYWIN*2) == 0){
			if( cl_index != 0 ){ cpgpage();}
			cpgsvp( 0.0, 1.0, 0.0, 1.0 ); cpgswin(0.0, 1.0, 0.0, 1.0 );
			cpgsci(1); cpgsch(0.5);
			sprintf( text, "EXPER :  %s\0", obs_name );
			cpgtext( 0.65, 0.990, text );
			sprintf( text, "SOURCE : %s\0", obj_name );
			cpgtext( 0.65, 0.975, text );
		}
		cpgbbuf();
		cpgsci(1);
		if( cl_index % (NYWIN*2) < NYWIN ){
			cpgsvp( 0.10, 0.45, win_bottom, win_top );
		} else {
			cpgsvp( 0.55, 0.90, win_bottom, win_top );
		}
		ymin = -M_PI; ymax = M_PI;
		cpgswin( xmin, xmax, ymin, ymax );

		if(win_index == 0){ cpgtbox( "BCNTSZH", 0.0, 0, "BCNTS", 0.0, 0);
		} else { 			cpgtbox( "BCTSZH",  0.0, 0, "BCNTS", 0.0, 0); }
		sprintf(text, "%-9s- %-9s- %-9s\0",
				stn_list[ant1], stn_list[ant2], stn_list[ant3]);
		cpgtext( xmin*0.9+xmax*0.1, ymin*0.1+ymax*0.9, text );

		for(ss_index=0; ss_index<ss_num; ss_index++){

			clphs = (float *)clphs_ptr[cl_index* ss_num + ss_index];
			clerr = (float *)clerr_ptr[cl_index* ss_num + ss_index];
			/*-------- PLOT PHASE --------*/
			for(time_index=0; time_index<time_num; time_index++){
				cl_top[time_index] = clphs[time_index] + clerr[time_index];
				cl_btm[time_index] = clphs[time_index] - clerr[time_index];
			}

			cpgsci( ss_index%8 + 3);
			cpgpt( time_num, time_list, clphs, 17);
			cpgerry( time_num, time_list, cl_top, cl_btm, 0.0);
		}

		cpgebuf();
	}
	free(cl_top);
	free(cl_btm);
	return(0);
}
