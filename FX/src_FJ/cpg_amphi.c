/*********************************************************
**	CPG_AMPHI.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <cpgplot.h>
#include <math.h>
#define	NYWIN	4
#define	SECDAY	86400

int	cpg_amphi( obs_name, obj_name, stn_num, stn_list,
		time_num, start_mjd, stop_mjd, time_list, ss_num,
		amp_ptr, phs_ptr, amp_err, phs_err )

	char	*obs_name;				/* Observation Name				*/
	char	*obj_name;				/* Object Name					*/
	int		stn_num;				/* Number of Stations			*/
	char	stn_list[][16];			/* Station Lists				*/
	int		time_num;				/* Number of Time Data			*/
	double	start_mjd;				/* Integ Start [MJD ]			*/
	double	stop_mjd;				/* Integ Stop [MJD ]			*/
	float	*time_list;				/* Time Data [Sec of Day]		*/
	int		ss_num;					/* Number of Sub-Stream			*/
	int		*amp_ptr;				/* Pointer of Visibility Data	*/
	int		*phs_ptr;				/* Pointer of Visibility Data	*/
	int		*amp_err;				/* Pointer of Visibility Data	*/
	int		*phs_err;				/* Pointer of Visibility Data	*/
{
	/*-------- PGPLOT FRAME VARIABLE --------*/
	float	xmin, xmax, ymin, ymax;	/* Corner of the Frame			*/
	char	pg_device[32];			/* PGPLOT Device Name			*/
	float	win_top, win_bottom;	/* World Coord. of Window		*/
	float	ywin_incr;				/* W-Axis Increment				*/

	/*-------- INDEX --------*/
	int		bl_index;				/* Baseline Index				*/
	int		ss_index;				/* Sub-Stream Index				*/
	int		time_index;				/* Index for time				*/
	int		win_index;				/* Window Index					*/
	int		err_code;

	/*-------- IDINTIFIER --------*/
	int		ant1, ant2;				/* Antenna Pair					*/

	/*-------- TOTAL NUMBER --------*/
	int		bl_num;					/* Baseline Number				*/

	/*-------- General Variables --------*/
	float	vis_max;				/* Visibility Amplitude Max		*/
	char	text[32];				/* Text to Plot					*/
	float	*vis_amp_ptr;			/* Visibility Amplitude			*/
	float	*vis_phs_ptr;			/* Visibility Phase				*/
	float	*amp_err_ptr;			/* Visibility Phase				*/
	float	*phs_err_ptr;			/* Visibility Phase				*/
	float	*vis_top_ptr;			/* Visibility Amplitude			*/
	float	*vis_btm_ptr;			/* Visibility Amplitude			*/

	bl_num = (stn_num* (stn_num - 1))/2;
	vis_top_ptr = (float *)malloc(time_num* sizeof(float) );
	vis_btm_ptr = (float *)malloc(time_num* sizeof(float) );


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

	bl_index = 0;
	ss_index = 0;
	for(bl_index=0; bl_index<bl_num; bl_index++){
		bl2ant(bl_index, &ant2, &ant1);
		win_index = bl_index % NYWIN;
		win_bottom	= 0.05 + ywin_incr* win_index ;
		win_top		= 0.05 + ywin_incr* (win_index + 1) ;
		if(win_index == 0){
			if( bl_index != 0){ cpgpage();}
			cpgsvp( 0.0, 1.0, 0.0, 1.0 );	cpgswin(0.0, 1.0, 0.0, 1.0 );
			cpgsci(1); cpgsch(0.5);
			sprintf( text, "EXPER :  %s\0", obs_name );
			cpgtext( 0.65, 0.990, text );
			sprintf( text, "SOURCE : %s\0", obj_name );
			cpgtext( 0.65, 0.975, text );
		}

		/*-------- MAX and MIN --------*/
		for(ss_index=0; ss_index<ss_num; ss_index++){
			vis_amp_ptr = (float *)amp_ptr[bl_index* ss_num + ss_index];
			vis_max = 0.0;
			for(time_index=0; time_index<time_num; time_index++){
				if( vis_amp_ptr[time_index] > vis_max ){
					vis_max = vis_amp_ptr[time_index];
				}
			}
		}
		cpgsci(1);
		cpgsvp( 0.1, 0.45, win_bottom, win_top );
		ymin = 0.0; ymax = vis_max* 1.5;
		cpgswin( xmin, xmax, ymin, ymax );
		if(win_index == 0){ cpgtbox( "BCNTSZH", 0.0, 0, "BCNTS", 0.0, 0);
		} else { 			cpgtbox( "BCTSZH",  0.0, 0, "BCNTS", 0.0, 0); }
		sprintf( text, "%-9s- %-9s\0", stn_list[ant1], stn_list[ant2]);
		cpgtext( xmin*0.4+xmax*0.6, ymin*0.1+ymax*0.9, text );

		/*-------- PLOT AMPLITUDE --------*/
		for(ss_index=0; ss_index<ss_num; ss_index++){
			vis_amp_ptr = (float *)amp_ptr[bl_index* ss_num + ss_index];
			amp_err_ptr = (float *)amp_err[bl_index* ss_num + ss_index];

			vis_max = 0.0;
			for(time_index=0; time_index<time_num; time_index++){
				vis_top_ptr[time_index] = vis_amp_ptr[time_index]
										+ amp_err_ptr[time_index];
				vis_btm_ptr[time_index] = vis_amp_ptr[time_index]
										- amp_err_ptr[time_index];

				if( vis_amp_ptr[time_index] > vis_max ){
					vis_max = vis_amp_ptr[time_index];
				}
			}
			cpgsci( ss_index%8 + 3);
			cpgpt( time_num, time_list, vis_amp_ptr, 17);
			cpgerry( time_num, time_list, vis_top_ptr, vis_btm_ptr, 0.0);
		}

		/*-------- PLOT PHASE --------*/
		cpgsci(1);
		cpgsvp( 0.55, 0.90, win_bottom, win_top );
		ymin = -M_PI; ymax = M_PI;
		cpgswin( xmin, xmax, ymin, ymax );

		if(win_index == 0){ cpgtbox( "BCNTSZH", 0.0, 0, "BCNTS", 0.0, 0);
		} else { 			cpgtbox( "BCTSZH",  0.0, 0, "BCNTS", 0.0, 0); }
		cpgtext( xmin*0.4+xmax*0.6, ymin*0.1+ymax*0.9, text );

		for(ss_index=0; ss_index<ss_num; ss_index++){
			vis_phs_ptr = (float *)phs_ptr[bl_index* ss_num + ss_index];
			phs_err_ptr = (float *)phs_err[bl_index* ss_num + ss_index];
			for(time_index=0; time_index<time_num; time_index++){
				vis_top_ptr[time_index] = vis_phs_ptr[time_index]
										+ phs_err_ptr[time_index];
				vis_btm_ptr[time_index] = vis_phs_ptr[time_index]
										- phs_err_ptr[time_index];
			}
			cpgsci( ss_index%8 + 3);
			cpgpt( time_num, time_list, vis_phs_ptr, 17);
			cpgerry( time_num, time_list, vis_top_ptr, vis_btm_ptr, 0.0);
		}
	}
	free(vis_top_ptr);
	free(vis_btm_ptr);
	return(0);
}
