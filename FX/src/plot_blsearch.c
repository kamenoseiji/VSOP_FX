/*********************************************************
**	PLOT_BLSEARCH.C	: PLOT Fringe Search Function		**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"

plot_blsearch( pg_dev, visamp_ptr, col_level, wind_x, wind_y, x_incr, y_incr,
		obs_ptr, first_obj_ptr, bl_info,
		integ_time, obj_name, start_mjd,
		bl_delay, bl_rate, vismax, vis_snr )

	char	*pg_dev;					/* PGPLOT Device					*/
	float	*visamp_ptr;				/* Pointer of Visibility Amplitude	*/
	float	col_level;					/* Color (GRAY) Level				*/
	int		wind_x;						/* Window Range (X, Y)				*/
	int		wind_y;						/* Window Range (X, Y)				*/
	double	x_incr;						/* X-Axis Increment				*/
	double	y_incr;						/* Y-Axis Increment				*/
	struct	header		*obs_ptr;		/* OBS HEADDER						*/
	struct	head_obj	*first_obj_ptr;	/* First Pointer of Objects Header	*/
	struct	bl_list		*bl_info;		/* Baseline Information				*/
	double	integ_time;					/* Integration Time [sec]			*/
	char	*obj_name;					/* Object Name						*/
	double	start_mjd;					/* Start Time [MJD]					*/
	double	bl_delay;					/* Residual Delay (BL-Based)		*/
	double	bl_rate;					/* Residual Delay Rate (BL-Based)	*/
	float	vismax;						/* Maximum Visibility Amp			*/
	float	vis_snr;					/* SNR of Visivility				*/
{
	/*-------- STRUCT for CFS --------*/
	struct	head_obj	*obj_ptr;		/* Pointer of Objects Header		*/

	/*-------- ID Number --------*/
	int		obj_id;						/* OBJECT ID						*/

	/*-------- CHARACTERS --------*/
	char	pg_text[64];				/* Text to Plot						*/

	/*-------- GENERAL VARIABLES --------*/
	double	sec;						/* Second							*/
	int		year, doy;					/* YEAR and Day of Year				*/
	int		hour, min;					/* Hour and Minutre					*/ 
/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	/*-------- SHOW FRINGE-SEARCH FUNCTION (GRAY SCALE) --------*/
	cpgbeg(1, pg_dev, 1, 1);
	cpgpseud(visamp_ptr,wind_x, wind_y, wind_y, x_incr, y_incr, col_level );

	/*-------- WINDOW FRAME POSITION --------*/
	cpgsvp(0.77, 0.95, 0.1, 0.9);
	cpgswin(0.0, 1.0, 0.0, 1.0);
	cpgsch(0.7);

	/*-------- TYPE EXPERIMENT NAME --------*/
	sprintf(pg_text, "EXPER:  %s", obs_ptr->obscode);
	memset( &pg_text[16], 0, 1);
	cpgtext(0.1, 1.0, pg_text);

	/*-------- TYPE EXPERIMENT NAME --------*/
	obj_ptr = first_obj_ptr;
	objct_id( obj_ptr, obj_name, &obj_id );
	sprintf(pg_text, "SOURCE: %s", obj_ptr->obj_name);
		cpgtext(0.1, 0.95, pg_text);

	/*-------- TYPE START and STOP TIME --------*/
	fmjd2doy( start_mjd, &year, &doy, &hour, &min, &sec);
	sprintf(pg_text, "START:  %03d %02d:%02d:%02d",
		doy, hour, min, (int)sec, integ_time);
	cpgtext(0.1, 0.90, pg_text);
	sprintf(pg_text, "INTEG:  %7.3lf sec.", integ_time);
	cpgtext(0.1, 0.85, pg_text);

	/*-------- TYPE ANTENNA PAIR --------*/
	sprintf(pg_text, "ANT:    %s - %s",
		bl_info->stn_name1,
		bl_info->stn_name2 );
		cpgtext(0.1, 0.80, pg_text);

	/*-------- TYPE VISIBILITY AMPLITUDE, DELAY, RATE --------*/
	sprintf(pg_text, "PEAK = %9.3e", vismax);
	cpgtext(0.1, 0.75, pg_text);

	sprintf(pg_text, "    at delay=%7.3lf \\gmsec", bl_delay);
	cpgtext(0.1, 0.72, pg_text);

	sprintf(pg_text,"       rate =%7.3lf psec/sec",bl_rate* 1.0e6);
	cpgtext(0.1, 0.69, pg_text);

	sprintf(pg_text, "SNR = %7.2f", vis_snr);
	cpgtext(0.1, 0.65, pg_text);

	cpgend();
	return(0);
}
