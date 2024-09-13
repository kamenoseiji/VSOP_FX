/*********************************************************
**	CPG_DELAY.C :	Plot Antenna-Based Delay Data		**
**														**
**	AUTHOR  : KAMENO Seiji								**
**	CREATED : 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"
#define	RADDEG 57.29577951308232087721
#define SECDAY 86400

cpg_spline_delay( obs_ptr, obj_ptr, stn_list, refant_id, fcal_ptr_ptr, stn_ptr,
			node_num, time_node_ptr, delay_coeff_ptr, rate_coeff_ptr,
			mjd_min, mjd_max, rate_min, rate_max,
			delay_min, delay_max, acc_min, acc_max,
			mjd_epoch, clk_ofs, clk_rat, clk_acc ) 

	struct	header		*obs_ptr;		/* Pointer of Obs Headder			*/
	struct	head_obj	*obj_ptr;		/* Pointer of Obj Headder			*/
	char	stn_list[][16];				/* Pointer of Station Name List		*/
	int		refant_id;					/* Refant ID in CFS					*/
	struct	fcal_data	**fcal_ptr_ptr;	/* Pointer of CLOCK data			*/
	struct	head_stn	*stn_ptr;		/* Pointer of Station Headder		*/
	int		*node_num;					/* Number of Node for Station		*/	
	int		*time_node_ptr;				/* Pointer of Time Node for Station	*/	
	int		*delay_coeff_ptr;			/* Pointer of Delay Spline Coeff.	*/	
	int		*rate_coeff_ptr;			/* Pointer of Rate Spline Coeff.	*/	
	double	mjd_min,	mjd_max;		/* Max and Min of MJD				*/
	double	rate_min,	rate_max;		/* Max and Min of Rate				*/
	double	delay_min,	delay_max;		/* Max and Min of Delay				*/
	double	acc_min,	acc_max;		/* Max and Min of Acceleration		*/
	double	mjd_epoch;					/* MJD of the Clock Epoch			*/
	double	clk_ofs;					/* Clock Offset [microsec]			*/
	double	clk_rat;					/* Clock rate [microsec/sec]		*/
	double	clk_acc;					/* Clock accel [microsec/sec/sec]	*/
{
	struct	head_obj	*first_obj_ptr;	/* Pointer of Obj Headder			*/
	struct	head_stn	*first_stn_ptr;	/* Pointer of Station Headder		*/
	struct	fcal_data	**first_fcal_ptr;/* Pointer of CLOCK data			*/
	struct	fcal_data	*fcal_ptr;		/* Pointer of CLOCK data			*/
	int		first_flag;
	int		stn_index;					/* Index for Station				*/
	int		stn_num;					/* Number of Station				*/
	int		time_index;					/* Index for Time					*/
	int		node_index;					/* Index for Spline Node			*/
	double	current_time;				/* Current Time for Spline			*/
	double	current_delay;				/* Current Delay by Spline			*/
	double	current_rate;				/* Current Rate by Spline			*/
	int		year, doy, hh, mm;			/* Time Variable					*/
	double	ss;							/* Time Variable					*/
	double	ref_sec;					/* Seconds from Clock Epoch			*/
	double	delta_ofs;					/* Clock Offset						*/
	double	delta_rat;					/* Clock Offset						*/
	double	delta_acc;					/* Clock Offset						*/

	/*-------- SSL2 VARIABLE --------*/
	int		spline_dim;					/* Spline Dimension					*/
	int		icon;						/* Condition Code					*/
	int		isw;						/* Control Code						*/
	double	vw[4];						/* Working Area						*/

	float	model_rate;					/* Delay Rate derived from model	*/
	float	x_data,	y_data, y_bottom, y_top;
	float	x_min,	x_max,	y_min,	y_max;
	float	time_min,	time_max;
	float	prev_x, prev_y;
	double	x_incr,	y_incr;
	char	pg_text[64];
/*
------------------------ SAVE INITIAL POINTER
*/
	first_obj_ptr	= obj_ptr;
	first_stn_ptr	= stn_ptr;
	first_fcal_ptr	= fcal_ptr_ptr;
/*
----------------------------------------------- SEARCH FOR REFANT
*/
	stn_index = 0;
	stn_ptr = first_stn_ptr;
	while( stn_ptr != NULL){
		if((stn_ptr->stn_index != refant_id) && (stn_ptr->acorr_index != -1)){
			stn_index++;
		}
		stn_ptr = stn_ptr->next_stn_ptr;
	}
	stn_num	= stn_index;
/*
----------------------------------------------- PREPARE PGPLOT DEVICE
*/
	cpgsvp( 0.10, 0.95, 0.10, 0.475 );
	x_min	= 1.2*SECDAY*(float)(mjd_min - (long)mjd_min)
			- 0.2*SECDAY*(float)(mjd_max - (long)mjd_min);
	x_max	= 1.2*SECDAY*(float)(mjd_max - (long)mjd_min)
			- 0.2*SECDAY*(float)(mjd_min - (long)mjd_min);
	y_min	= 1.2*(float)delay_min - 0.2*(float)delay_max;
	y_max	= 1.2*(float)delay_max - 0.2*(float)delay_min;

	cpg_incr( (x_max - x_min), &x_incr );
	cpg_incr( (y_max - y_min), &y_incr );

	cpgswin( x_min, x_max, y_min, y_max);
	cpgsci(14); cpgrect( x_min, x_max, y_min, y_max);
	cpgsci(0);	cpgbox( "G", 0.0, 0,
						"G", 0.0, 0 );
	cpgsci(13);	cpgtbox( "BCNTSZH", 0.0, 0,
						"BCNTS", 0.0, 0 );

	fmjd2doy(mjd_min, &year, &doy, &hh, &mm, &ss);
	sprintf( pg_text, "TIME from %04d %03d day", year, doy); 
	cpglab(pg_text, "DELAY [\\gmsec]", ""); 
/*
----------------------------------------------- LOOP FOR DELAY DATA
*/
	stn_index	= 0;
	stn_ptr	= first_stn_ptr;

	/*-------- LOOP FOR STATION --------*/
	while( *fcal_ptr_ptr != NULL ){


		/*-------- AVOID REFANT --------*/
		if( stn_ptr->stn_index == refant_id ){
			sprintf(pg_text, "REFANT : %s", stn_ptr->stn_name);
			cpgtext( x_min*0.25+x_max*0.75, y_min*0.95+y_max*0.05, pg_text);
			stn_ptr = stn_ptr->next_stn_ptr;
		}

		fcal_ptr	= *fcal_ptr_ptr;
		cpgsci(stn_index + 2);

		first_flag = 1;
		time_min = 1.0e17;	time_max = -1.0e17;
		/*-------- LOOP F-CAL FOR DATA (TIME) --------*/
		cpgbbuf();
		while( fcal_ptr != NULL ){

			/*-------- SEARCE FOR CURRENT OBJ in OBJ LIST --------*/
			obj_ptr = first_obj_ptr;
			while(obj_ptr != NULL){
				if(strstr(fcal_ptr->objnam, obj_ptr->obj_name) != NULL){
					break;
				}
				obj_ptr = obj_ptr->next_obj_ptr;
			}


			ref_sec = SECDAY* (fcal_ptr->mjd - mjd_epoch);
			delta_rat = clk_acc* ref_sec + clk_rat;
			delta_ofs = 0.5* clk_acc* ref_sec* ref_sec
					  + clk_rat* ref_sec + clk_ofs;

			/*-------- DISPLAY DELAY AND RATE DATA --------*/
			fmjd2doy(fcal_ptr->mjd, &year, &doy, &hh, &mm, &ss);
			printf("%04d%03d%02d%02d%02d : STN[%d] DELAY=%8.4lf +/- %6.2lf  RATE=%8.4lf +/- %6.2lf\n",
				year, doy, hh, mm, (int)ss, stn_ptr->stn_index,
				1000.0*(fcal_ptr->delay + delta_ofs),
				1000.0*fcal_ptr->delay_err,
				1.0e6*(fcal_ptr->rate + delta_rat),
				1.0e6*fcal_ptr->rate_err);


			x_data	= (float)( SECDAY*(fcal_ptr->mjd - (long)mjd_min) );
			y_data	= (float)fcal_ptr->delay;
			y_bottom= (float)( fcal_ptr->delay - fcal_ptr->delay_err );
			y_top	= (float)( fcal_ptr->delay + fcal_ptr->delay_err );

			if( x_data < time_min ){ time_min = x_data; }
			if( x_data > time_max ){ time_max = x_data; }

			cpgpt( 1, &x_data, &y_data, 17);
			cpgerry( 1, &x_data, &y_bottom, &y_top, 1.0);

			fcal_ptr = fcal_ptr->next_fcal_ptr;
		} /*-------- END of LOOP FOR F-CAL DATA (TIME) --------*/
		cpgebuf();

		/*-------- DISPLAY SPLINE DATA (TIME) --------*/
		current_time = (double)time_min;
		node_index = 0;	spline_dim = 3;
		cpgbbuf();
		while( current_time < (double)time_max){

			isw = 0;
			dbsf1_( &spline_dim, time_node_ptr[stn_index], &node_num[stn_index],
				delay_coeff_ptr[stn_index], &isw, &current_time, &node_index,
				&current_delay, vw, &icon);

			if( current_time == (double)time_min ){
				cpgmove( (float)current_time, (float)current_delay );
			} else {
				cpgdraw( (float)current_time, (float)current_delay );
			}

			current_time += 1.0;
		} /*-------- END of SPLINE DATA (TIME) --------*/
		cpgebuf();

		cpgsch(0.7);
		cpgtext(x_max*0.9 + x_min*0.1, y_data, stn_list[stn_index]);
		cpgsch(1.0);

		fcal_ptr_ptr++;
		stn_index++;
		stn_ptr	= stn_ptr->next_stn_ptr;
	}
	fcal_ptr_ptr = first_fcal_ptr;
/*
----------------------------------------------- PREPARE PGPLOT FOR RATE DATA
*/
	cpgsvp( 0.10, 0.95, 0.525, 0.90 );

	y_min	= 1.2*(float)rate_min - 0.2*(float)rate_max;
	y_max	= 1.2*(float)rate_max - 0.2*(float)rate_min;
	cpg_incr( (y_max - y_min), &y_incr );

	cpgswin( x_min, x_max, y_min, y_max);
	cpgsci(14); cpgrect( x_min, x_max, y_min, y_max);
	cpgsci(0);	cpgbox( "G", 0.0, 0,
						"G", 0.0, 0 );
	cpgsci(13);	cpgtbox( "BCTSZH", 0.0, 0,
						"BCNTS", 0.0, 0 );
	cpglab("", "RATE [\\gmsec/sec]", "RESULTS FROM FRINGE SEARCH"); 
	sprintf(pg_text, "EXPER : %s\0", obs_ptr->obscode);
	cpgtext(0.25* x_min + 0.75* x_max, -0.05* y_min + 1.05* y_max, pg_text);

/*
----------------------------------------------- LOOP FOR RATE DATA
*/
	stn_index = 0;
	stn_ptr	= first_stn_ptr;

	/*-------- LOOP FOR STATION --------*/
	while( *fcal_ptr_ptr != NULL ){

		/*-------- AVOID REFANT --------*/
		if( stn_ptr->stn_index == refant_id ){
			sprintf(pg_text, "REFANT : %s", stn_ptr->stn_name);
			cpgtext( x_min*0.25+x_max*0.75, y_min*0.95+y_max*0.05, pg_text);
			stn_ptr = stn_ptr->next_stn_ptr;
		}

		cpgsci(stn_index + 2);
		time_index	= 0;	time_min = 1.0e17;	time_max = -1.0e17;
		fcal_ptr	= *fcal_ptr_ptr;
		first_flag	= 1;
		/*-------- LOOP FOR DATA (TIME) --------*/
		cpgbbuf();
		while( fcal_ptr != NULL ){

			/*-------- SEARCE FOR CURRENT OBJ in OBJ LIST --------*/
			obj_ptr = first_obj_ptr;
			while(obj_ptr != NULL){
				if(strstr(fcal_ptr->objnam, obj_ptr->obj_name) != NULL){
					break;
				}
				obj_ptr = obj_ptr->next_obj_ptr;
			}


			x_data	= (float)(SECDAY*(fcal_ptr->mjd - (long)mjd_min));
			y_data	= (float)fcal_ptr->rate;
			y_bottom= (float)( fcal_ptr->rate - fcal_ptr->rate_err);
			y_top	= (float)( fcal_ptr->rate + fcal_ptr->rate_err);

			if( x_data < time_min ){ time_min = x_data; }
			if( x_data > time_max ){ time_max = x_data; }

			cpgpt( 1, &x_data, &y_data, 17);
			cpgerry( 1, &x_data, &y_bottom, &y_top, 1.0);

			time_index++;
			fcal_ptr = fcal_ptr->next_fcal_ptr;
		} /*-------- LOOP FOR DATA (TIME) --------*/
		cpgebuf();

		/*-------- DISPLAY SPLINE DATA (TIME) --------*/
		current_time = (double)time_min;
		node_index = 0;	spline_dim = 3;
		cpgbbuf();
		while( current_time < (double)time_max ){

			isw = 0;
			dbsf1_( &spline_dim, time_node_ptr[stn_index], &node_num[stn_index],
				rate_coeff_ptr[stn_index], &isw, &current_time, &node_index,
				&current_rate, vw, &icon);

			if( current_time == (double)time_min ){
				cpgmove( (float)current_time, (float)current_rate );
			} else {
				cpgdraw( (float)current_time, (float)current_rate );
			}

			current_time += 1.0;
		} /*-------- END of SPLINE DATA (TIME) --------*/
		cpgebuf();

		cpgsch(0.7);
		cpgtext(x_max*0.9 + x_min*0.1, y_data, stn_list[stn_index]);
		cpgsch(1.0);

		fcal_ptr_ptr++;
		stn_index++;
		stn_ptr = stn_ptr->next_stn_ptr;
	}
	fcal_ptr_ptr = first_fcal_ptr;

	return;
}
