/*********************************************************
**	CPG_GAIN.C :	Plot Antenna-Based Delay Data		**
**														**
**	AUTHOR  : KAMENO Seiji								**
**	CREATED : 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"
#define MAX_ANT 10
#define MAX_DATA 100
#define SECDAY 86400
#define	RADDEG	57.29577951308232087721

cpg_gain( obs_ptr, obj_ptr, stn_num, gcal_ptr_ptr,	first_stn_ptr,
			mjd_min, mjd_max, gain_min, gain_max, gain_limit)

	struct	header		*obs_ptr;		/* Pointer of Obs Header */
	struct	head_obj	*obj_ptr;		/* Pointer of Obs Header */
	struct	gcal_data	**gcal_ptr_ptr;	/* Pointer of GAIN data */
	struct	head_stn	*first_stn_ptr;	/* Pointer of Station Headder */
	double	mjd_min,	mjd_max;		/* Max and Min of MJD */
	double	gain_min,	gain_max;		/* Max and Min of gain */
	double	gain_limit;					/* Acceptable gain Limit		*/
{
	struct	head_obj	*first_obj_ptr;	/* First Pointer of Obs Header	*/
	struct	gcal_data	**first_gcal_ptr;/* Pointer of GAIN data		*/
	struct	gcal_data	*gcal_ptr;		/* Pointer of GAIN data			*/
	struct	head_stn	*stn_ptr;		/* Pointer of Station Headder	*/
	int		stn_index;
	int		data_index;
	double	radius;						/* Earth Radius					*/
	double	sin_phi, cos_phi;			/* sin(latitude), cos(latitude)	*/
	double	sin_el;						/* sin(EL)						*/
	double	lambda;						/* Longitude					*/
	double	gmst;						/* Greenwidge Mean Sidereal Time*/
	double	sefd_prm[3*MAX_ANT];
	double	solint;						/* Solution Interval [MJD]		*/
	double	ae[MAX_ANT];				/* Effective Aperture [m^2]		*/
	double	trx[MAX_ANT];				/* Receiver Temperature [K]		*/
	double	sefd_coeff[1024];			/* Spline Coefficient			*/
	double	time_node[1024];			/* Node Point of Time [MJD]		*/
	int		node_num[1024];				/* Number of Node Points		*/
	int		current_src;				/* Current Source ID			*/
	double	current_mjd;				/* MJD in Model SEFD Plot		*/
	double	current_sefd[MAX_ANT];		/* SEFD Value from the Model	*/
	int		year, doy, hour, min;		/* Time Data					*/
	double	sec;
	double	x_incr,	y_incr;
	float	x_data,	y_data, y_bottom, y_top;
	float	x_min,	x_max,	y_min,	y_max;
	char	pg_text[64];

	first_obj_ptr	= obj_ptr;
	first_gcal_ptr	= gcal_ptr_ptr;
/*
------------------------  GAIN MODEL VERSUS ELEVATION
	atmgain_solve( obs_ptr, obj_ptr, gcal_ptr_ptr, first_stn_ptr, sefd_prm);
*/


	obj_ptr			= first_obj_ptr;
	gcal_ptr_ptr	= first_gcal_ptr;
	stn_ptr			= first_stn_ptr;

	solint	= 300.0/SECDAY;
	ae[0]	= 1050.0;	trx[0]	= 188.0;
	ae[1]	= 220.0;	trx[1]	= 163.1;
	ae[2]	= 40.0;		trx[2]	= 160.0;
	ae[3]	= 12.0;		trx[3]	= 120.0;
/*
	atmgain_solve( obs_ptr, obj_ptr, gcal_ptr_ptr, stn_ptr,
				solint, trx, ae, sefd_coeff, time_node, node_num);
------------------------ PLOT LABEL
*/
	cpgsvp( 0.0, 1.0, 0.0, 1.0 );
	cpgswin( 0.0, 1.0, 0.0, 1.0 );
	cpgsci(1);	cpgsch(1.0);

	cpgtext( 0.45, 0.975, "GAIN PLOT");
	cpgsvp( 0.1, 0.5, 0.1, 1.0 );

/*
	cpgswin( 0.0, 1.0, 0.0, 1.0 );
	cpglab("SEC Z", "SEFD [Jy]", "");
*/

	sprintf( pg_text, "MJD - %d", (int)mjd_min);
	cpgswin( 0.0, 1.0, 0.0, 1.0 );
	cpglab(pg_text, "SEFD [Jy]", "");

	cpgsvp( 0.58, 1.0, 0.1, 1.0 );
	cpgswin( 0.0, 1.0, 0.0, 1.0 );
	cpglab(pg_text, "PHASE [rad]", "");

	cpgsch(0.7);
	sprintf(pg_text, "EXPER    :  %s\0", obs_ptr->obscode);
	cpgtext(0.4, 0.980, pg_text);
	sprintf(pg_text, "SOURCE   :  %s\0", obj_ptr->obj_name);
	cpgtext(0.4, 0.962, pg_text);
	gcal_ptr	= *gcal_ptr_ptr;
	fmjd2doy( gcal_ptr->mjd, &year, &doy, &hour, &min, &sec);
	sprintf(pg_text, "OBS DATE : %04d DOY=%03d", year, doy);
	cpgtext(0.4, 0.944, pg_text);
/*
------------------------ STATION LOOP
*/
	stn_index	= 0;
	stn_ptr	= first_stn_ptr;
	while( *gcal_ptr_ptr != NULL ){

		radius	= sqrt( stn_ptr->stn_pos[0]*stn_ptr->stn_pos[0]
					+	stn_ptr->stn_pos[1]*stn_ptr->stn_pos[1]
					+	stn_ptr->stn_pos[2]*stn_ptr->stn_pos[2]);
		sin_phi		= stn_ptr->stn_pos[2] / radius;
		cos_phi		= sqrt(1.0 - sin_phi*sin_phi);
		lambda		= atan2(stn_ptr->stn_pos[1]/(radius*cos_phi),
							stn_ptr->stn_pos[0]/(radius*cos_phi) );

		gcal_ptr	= *gcal_ptr_ptr;

		/*-------- PEAK SEARCH --------*/
		gain_max	= -999.0;
		while( gcal_ptr != NULL ){

			if(	(gcal_ptr->weight > 0.0001) &&
				(gcal_ptr->real < gain_limit) &&
				(gcal_ptr->weight/gcal_ptr->real < 0.2) &&
				(gcal_ptr->real > gain_max) ){

				gain_max	= gcal_ptr->real;
			}
			gcal_ptr = gcal_ptr->next_gcal_ptr;
		}

		/*-------- PLOT FRAME --------*/
		cpgsvp( 0.10, 0.475,
			0.10 + 0.85*(float)stn_index/((float)stn_num),
			0.05 + 0.85*(float)(stn_index + 1)/((float)stn_num) );

		x_min	= 1.2*SECDAY*(float)(mjd_min - (int)mjd_min)
				- 0.2*SECDAY*(float)(mjd_max - (long)mjd_min);
		x_max	= 1.2*SECDAY*(float)(mjd_max - (int)mjd_min)
				- 0.2*SECDAY*(float)(mjd_min - (long)mjd_min);
		y_min	= 0.0; y_max	= 1.2*(float)gain_max;

		cpgsch(0.75);
		cpgswin( x_min, x_max, y_min, y_max);
		cpgsci(14); cpgrect( x_min, x_max, y_min, y_max);
		cpgsci(0);	cpgbox( "G", 0.0, 0,
							"G", 0.0, 0 );
		cpgsci(13);	cpgtbox( "BCNTSZH", 0.0, 0,
							"BCNTS", 0.0, 0 );
		cpgsch(1.0);

		/*-------- PLOT DATA --------*/
		gcal_ptr	= *gcal_ptr_ptr;

		cpgsci(stn_index + 2);
		printf("-------- %s SEFD --------\n", stn_ptr->stn_name);
		while( gcal_ptr != NULL ){

			/*-------- SEARCH CURRENT OBJECT from OBJECT LIST --------*/ 
			obj_ptr	= first_obj_ptr;
			while( obj_ptr != NULL ){
				if(strstr(gcal_ptr->objnam, obj_ptr->obj_name) != NULL){
					break;
				}
				obj_ptr = obj_ptr->next_obj_ptr;
			}

			/*-------- CALCULATE CURRENT ELEVATION --------*/
			mjd2gmst( gcal_ptr->mjd, obs_ptr->ut1utc, &gmst);
			gst2el( gmst, -lambda, atan2(sin_phi, cos_phi),
				obj_ptr->obj_pos[0]/RADDEG,
				obj_ptr->obj_pos[1]/RADDEG,
				&sin_el );

			#ifdef DEBUG
			#endif
			fmjd2doy( gcal_ptr->mjd, &year, &doy, &hour, &min, &sec);
			printf("%03d %02d:%02d:%02d  %lf  +/-  %lf\n",
				doy, hour, min, (int)sec, 
				gcal_ptr->real, gcal_ptr->weight);
			#ifdef DEBUG
			#endif

			x_data	= SECDAY*(float)(gcal_ptr->mjd - (long)mjd_min);
/*
			x_data	= (float)(1.0/sin_el);
*/
			y_data	= (float)gcal_ptr->real;
			cpgpt( 1, &x_data, &y_data, 17);
			gcal_ptr = gcal_ptr->next_gcal_ptr;
		}

		/*-------- PLOT SEFD MODEL --------*/

#ifdef HIDOI
		for(data_index=0; data_index<MAX_DATA; data_index++){

			obj_ptr			= first_obj_ptr;

			x_data	= x_min + data_index * (x_max - x_min)/MAX_DATA;

			current_mjd	= (double)x_data + (int)mjd_min;
			current_src	= 3;

			atmgain_get( obs_ptr, obj_ptr, first_stn_ptr,
				trx, ae, sefd_coeff, time_node, node_num,
				current_mjd, current_src, current_sefd);
/*

			printf(" CALCULATED SEFD = %lf\n", current_sefd[stn_index]);
*/

			y_data	= (float)current_sefd[stn_index];




/*
			y_data	= (float)(sefd_prm[stn_index*3]
					* exp(sefd_prm[stn_index*3 + 2] * (double)x_data)
					+ sefd_prm[stn_index*3 + 1]);
*/

			if( data_index == 0){
				cpgmove( x_data,  y_data );
			} else {
				cpgdraw( x_data,  y_data );
			}
		}
#endif

/*
		cpgtext(x_min*0.975 + x_max*0.025,
				gain_0 + x_min*gain_1 + 0.03*(y_max - y_min),
				stn_ptr->stn_name);
*/
		cpgtext(x_min*0.975 + x_max*0.025,
				y_max*0.8 + y_min*0.2,
				stn_ptr->stn_name);

		gcal_ptr_ptr++;
		stn_index++;
		stn_ptr	= stn_ptr->next_stn_ptr;
	}

/*
------------------------ PLOT PHASE
*/
	gcal_ptr_ptr = first_gcal_ptr;
	stn_index	= 0;
/*
------------------------ STATION LOOP
*/
	stn_ptr	= first_stn_ptr;
	while( *gcal_ptr_ptr != NULL ){
		gcal_ptr	= *gcal_ptr_ptr;

		/*-------- PLOT FRAME --------*/
		cpgsvp( 0.575, 0.95,
			0.10 + 0.85*(float)stn_index/((float)stn_num),
			0.05 + 0.85*(float)(stn_index + 1)/((float)stn_num) );

		x_min	= 1.2*SECDAY*(float)(mjd_min - (int)mjd_min)
				- 0.2*SECDAY*(float)(mjd_max - (long)mjd_min);
		x_max	= 1.2*SECDAY*(float)(mjd_max - (int)mjd_min)
				- 0.2*SECDAY*(float)(mjd_min - (long)mjd_min);
		y_min	= -M_PI;
		y_max	= M_PI;

/*
		x_incr	= exp(M_LN10*((int)(log10(x_max - x_min) - 1.5)));
		y_incr	= exp(M_LN10*((int)(log10(y_max - y_min) - 0.5)));

		cpg_incr( (x_max - x_min), &x_incr);
		cpg_incr( (y_max - y_min), &y_incr);
*/

		cpgsch(0.75);
		cpgswin( x_min, x_max, y_min, y_max);
		cpgsci(14); cpgrect( x_min, x_max, y_min, y_max);
		cpgsci(0);	cpgbox( "G", 0.0, 0,
							"G", 0.0, 0 );
		cpgsci(13);	cpgtbox( "BCNTSZH", 0.0, 0,
							"BCNTS", 0.0, 0 );
		cpgsch(1.0);

		/*-------- PLOT PHASE DATA --------*/
		cpgsci(stn_index + 2);
		while( gcal_ptr != NULL ){
			x_data	= SECDAY*(float)(gcal_ptr->mjd - (long)mjd_min);
			y_data	= (float)gcal_ptr->phase;

			cpgpt( 1, &x_data, &y_data, 17);
			gcal_ptr = gcal_ptr->next_gcal_ptr;
		}

		cpgmove( x_min, 0.0 );
		cpgdraw( x_max, 0.0 );

		cpgtext(x_min*0.975 + x_max*0.025,
				y_max*0.8 + y_min*0.2,
				stn_ptr->stn_name);

		gcal_ptr_ptr++;
		stn_index++;
		stn_ptr	= stn_ptr->next_stn_ptr;
	}

	return;
}
