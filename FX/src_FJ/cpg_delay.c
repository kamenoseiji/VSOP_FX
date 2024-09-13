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

cpg_delay( obs_ptr, obj_ptr, refant_id, fcal_ptr_ptr, stn_ptr, atm_ptr,
			mjd_min, mjd_max, rate_min, rate_max,
			delay_min, delay_max, acc_min, acc_max ) 

	struct	header		*obs_ptr;		/* Pointer of Obs Headder */
	struct	head_obj	*obj_ptr;		/* Pointer of Obj Headder */
	struct	head_stn	*stn_ptr;		/* Pointer of Station Headder */
	int		refant_id;					/* Refant ID in CFS */
	struct	fcal_data	**fcal_ptr_ptr;	/* Pointer of CLOCK data */
	double	*atm_ptr;					/* Pointer of Atmospheric Parameter */
	double	mjd_min,	mjd_max;		/* Max and Min of MJD */
	double	rate_min,	rate_max;		/* Max and Min of Rate */
	double	delay_min,	delay_max;		/* Max and Min of Delay */
	double	acc_min,	acc_max;		/* Max and Min of Acceleration */
{
	struct	head_obj	*first_obj_ptr;	/* Pointer of Obj Headder */
	struct	head_stn	*first_stn_ptr;	/* Pointer of Station Headder */
	struct	fcal_data	**first_fcal_ptr;/* Pointer of CLOCK data */
	struct	fcal_data	*fcal_ptr;		/* Pointer of CLOCK data */
	int		first_flag;
	int		stn_index;					/* Index for Station */
	int		stn_num;					/* Number of Station */
	int		time_index;					/* Index for Time */
	double	radius, radius_ref;			/* Earth Radius */
	double	sin_phi, sin_phi_ref;		/* sin(latitude) */
	double	cos_phi, cos_phi_ref;		/* cos(latitude) */
	double	lambda, lambda_ref;			/* Longitude */
	double	sin_el, sin_el_ref;			/* sin(EL) */
	double	dsecz, dsecz_ref;			/* d[secz]/dt */
	double	gmst;						/* Greenwich Mean Sidereal Time */
	float	model_rate;					/* Delay Rate derived from model */
	float	x_data,	y_data, y_bottom, y_top;
	float	x_min,	x_max,	y_min,	y_max;
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
		if( stn_ptr->stn_index == refant_id){
			radius_ref  = sqrt( stn_ptr->stn_pos[0]*stn_ptr->stn_pos[0]
							+	stn_ptr->stn_pos[1]*stn_ptr->stn_pos[1]
							+	stn_ptr->stn_pos[2]*stn_ptr->stn_pos[2] );
			sin_phi_ref = stn_ptr->stn_pos[2] / radius_ref;
			cos_phi_ref = sqrt( 1.0 - sin_phi_ref*sin_phi_ref);
			lambda_ref  = atan2(stn_ptr->stn_pos[1]/(radius_ref*cos_phi_ref),
								stn_ptr->stn_pos[0]/(radius_ref*cos_phi_ref));
		} else if(stn_ptr->acorr_index != -1){
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

	sprintf( pg_text, "TIME from MJD %d", (int)mjd_min); 
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

		/*-------- STATION POSITION in SPHERICAL COORDINATE --------*/
		radius	= sqrt( stn_ptr->stn_pos[0]*stn_ptr->stn_pos[0]
				+		stn_ptr->stn_pos[1]*stn_ptr->stn_pos[1]
				+		stn_ptr->stn_pos[2]*stn_ptr->stn_pos[2] );
		sin_phi	= stn_ptr->stn_pos[2] / radius;
		cos_phi	= sqrt(1.0 - sin_phi*sin_phi);
		lambda	= atan2(stn_ptr->stn_pos[1]/(radius*cos_phi),
						stn_ptr->stn_pos[0]/(radius*cos_phi) );

		fcal_ptr	= *fcal_ptr_ptr;
		cpgsci(stn_index + 2);

		first_flag = 1;
		/*-------- LOOP FOR DATA (TIME) --------*/
		while( fcal_ptr != NULL ){

			/*-------- SEARCE FOR CURRENT OBJ in OBJ LIST --------*/
			obj_ptr = first_obj_ptr;
			while(obj_ptr != NULL){
				if(strstr(fcal_ptr->objnam, obj_ptr->obj_name) != NULL){
					break;
				}
				obj_ptr = obj_ptr->next_obj_ptr;
			}

			/*-------- CALC. CURRENT ELEVATION --------*/
			mjd2gmst(fcal_ptr->mjd, obs_ptr->ut1utc, &gmst);

			gst2el(gmst, -lambda_ref, atan2(sin_phi_ref, cos_phi_ref),
				obj_ptr->obj_pos[0]/RADDEG,
				obj_ptr->obj_pos[1]/RADDEG,
				&sin_el_ref );

			gst2el(gmst, -lambda, atan2(sin_phi, cos_phi),
				obj_ptr->obj_pos[0]/RADDEG,
				obj_ptr->obj_pos[1]/RADDEG,
				&sin_el );

			model_rate	= (float)
				( atm_ptr[stn_index+1]/sin_el + atm_ptr[stn_num+stn_index+1]
				- atm_ptr[0]/sin_el_ref );

			x_data	= (float)( SECDAY*(fcal_ptr->mjd - (long)mjd_min) );
			y_data	= (float)fcal_ptr->delay;
			y_bottom= (float)( fcal_ptr->delay - fcal_ptr->delay_err );
			y_top	= (float)( fcal_ptr->delay + fcal_ptr->delay_err );

			cpgpt( 1, &x_data, &y_data, 17);
			cpgerry( 1, &x_data, &y_bottom, &y_top, 1.0);

/* */
			if(first_flag == 1){
				cpgmove( x_data,  y_data );
				prev_x	= x_data;	prev_y	= y_data;
				first_flag = 0;
			} else {
				cpgmove( prev_x, prev_y );
				cpgdraw( x_data,  y_data );
				prev_x	= x_data;	prev_y	= y_data;
			}
/* */

/*
			cpgpt( 1, &x_data, &model_rate, 2);

			if(first_flag == 1){
				cpgmove( x_data,  model_rate );
				prev_x	= x_data;	prev_y	= model_rate;
				first_flag = 0;
			} else {
				cpgmove( prev_x, prev_y );
				cpgdraw( x_data,  model_rate );
				prev_x	= x_data;	prev_y	= model_rate;
			}
*/



			fcal_ptr = fcal_ptr->next_fcal_ptr;
		}

/* 
		cpgmove( x_min,  delay_0 + x_min*delay_1 );
		cpgdraw( x_max,  delay_0 + x_max*delay_1 );
*/

		cpgsch(0.7);
		cpgtext(x_max*0.9 + x_min*0.1,
				y_data,
				stn_ptr->stn_name);
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

		/*-------- STATION POSITION in SPHERICAL COORDINATE --------*/
		radius	= sqrt( stn_ptr->stn_pos[0]*stn_ptr->stn_pos[0]
				+		stn_ptr->stn_pos[1]*stn_ptr->stn_pos[1]
				+		stn_ptr->stn_pos[2]*stn_ptr->stn_pos[2] );
		sin_phi	= stn_ptr->stn_pos[2] / radius;
		cos_phi	= sqrt(1.0 - sin_phi*sin_phi);
		lambda	= atan2(stn_ptr->stn_pos[1]/(radius*cos_phi),
						stn_ptr->stn_pos[0]/(radius*cos_phi) );

		cpgsci(stn_index + 2);
		time_index	= 0;
		fcal_ptr	= *fcal_ptr_ptr;
		first_flag	= 1;
		/*-------- LOOP FOR DATA (TIME) --------*/
		while( fcal_ptr != NULL ){

			/*-------- SEARCE FOR CURRENT OBJ in OBJ LIST --------*/
			obj_ptr = first_obj_ptr;
			while(obj_ptr != NULL){
				if(strstr(fcal_ptr->objnam, obj_ptr->obj_name) != NULL){
					break;
				}
				obj_ptr = obj_ptr->next_obj_ptr;
			}

			/*-------- CALC. CURRENT ELEVATION --------*/
			mjd2gmst(fcal_ptr->mjd, obs_ptr->ut1utc, &gmst);

			gst2el(gmst, -lambda_ref, atan2(sin_phi_ref, cos_phi_ref),
				obj_ptr->obj_pos[0]/RADDEG,
				obj_ptr->obj_pos[1]/RADDEG,
				&sin_el_ref );

			gst2el(gmst, -lambda, atan2(sin_phi, cos_phi),
				obj_ptr->obj_pos[0]/RADDEG,
				obj_ptr->obj_pos[1]/RADDEG,
				&sin_el );

			gst2dsecz( obs_ptr->degpdy, gmst, -lambda_ref,
				atan2(sin_phi_ref, cos_phi_ref),
				obj_ptr->obj_pos[0]/RADDEG,
				obj_ptr->obj_pos[1]/RADDEG,
				sin_el_ref,
				&dsecz_ref );

			gst2dsecz( obs_ptr->degpdy, gmst, -lambda,
				atan2(sin_phi, cos_phi),
				obj_ptr->obj_pos[0]/RADDEG,
				obj_ptr->obj_pos[1]/RADDEG,
				sin_el,
				&dsecz );

			model_rate	= (float)
				( atm_ptr[stn_index+1]*dsecz + atm_ptr[2*stn_num+stn_index+1]
				- atm_ptr[0]*dsecz_ref );

			x_data	= (float)(SECDAY*(fcal_ptr->mjd - (long)mjd_min));
			y_data	= (float)fcal_ptr->rate;
			y_bottom= (float)( fcal_ptr->rate - fcal_ptr->rate_err);
			y_top	= (float)( fcal_ptr->rate + fcal_ptr->rate_err);

			cpgpt( 1, &x_data, &y_data, 17);
			cpgerry( 1, &x_data, &y_bottom, &y_top, 1.0);

/* */
			if(first_flag == 1){
				cpgmove( x_data,  y_data );
				prev_x	= x_data;	prev_y	= y_data;
				first_flag = 0;
			} else {
				cpgmove( prev_x, prev_y );
				cpgdraw( x_data,  y_data );
				prev_x	= x_data;	prev_y	= y_data;
			}
/* */
/*
			cpgpt( 1, &x_data, &model_rate, 2);

			if(first_flag == 1){
				cpgmove( x_data,  model_rate );
				prev_x	= x_data;	prev_y	= model_rate;
				first_flag = 0;
			} else {
				cpgmove( prev_x, prev_y );
				cpgdraw( x_data,  model_rate );
				prev_x	= x_data;	prev_y	= model_rate;
			}
*/


/*
			if(time_index == 0){
				cpgsch(0.7);
				cpgtext(x_max*0.9 + x_min*0.1, y_data,
						stn_ptr->stn_name);
				cpgsch(1.0);
			}
*/

			time_index++;
			fcal_ptr = fcal_ptr->next_fcal_ptr;
		}
		cpgsch(0.7);
		cpgtext(x_max*0.9 + x_min*0.1,
				y_data,
				stn_ptr->stn_name);
		cpgsch(1.0);


		fcal_ptr_ptr++;
		stn_index++;
		stn_ptr = stn_ptr->next_stn_ptr;
	}
	fcal_ptr_ptr = first_fcal_ptr;

	return;
}
