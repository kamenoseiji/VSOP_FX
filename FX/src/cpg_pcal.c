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
#define	RADDEG	57.29577951308232087721

cpg_gain( obs_ptr, obj_ptr, stn_num, gcal_ptr_ptr,	first_stn_ptr,
			mjd_min, mjd_max, gain_min, gain_max)

	struct	header		*obs_ptr;		/* Pointer of Obs Header */
	struct	head_obj	*obj_ptr;		/* Pointer of Obs Header */
	struct	gcal_data	**gcal_ptr_ptr;	/* Pointer of GAIN data */
	struct	head_stn	*first_stn_ptr;	/* Pointer of Station Headder */
	double	mjd_min,	mjd_max;		/* Max and Min of MJD */
	double	gain_min,	gain_max;		/* Max and Min of gain */
{
	struct	head_obj	*first_obj_ptr;	/* First Pointer of Obs Header */
	struct	gcal_data	**first_gcal_ptr;/* Pointer of GAIN data */
	struct	gcal_data	*gcal_ptr;		/* Pointer of GAIN data */
	struct	head_stn	*stn_ptr;		/* Pointer of Station Headder */
	int		stn_index;
	int		data_index;
	double	radius;						/* Earth Radius */
	double	sin_phi, cos_phi;			/* sin(latitude), cos(latitude) */
	double	sin_el;						/* sin(EL) */
	double	lambda;						/* Longitude */
	double	gmst;						/* Greenwidge Mean Sidereal Time */
	double	sefd_prm[3*MAX_ANT];
	double	x_incr,	y_incr;

	float	x_data,	y_data, y_bottom, y_top;
	float	x_min,	x_max,	y_min,	y_max;
	char	pg_text[64];
/*
------------------------  GAIN MODEL VERSUS ELEVATION
	atmgain_solve( obs_ptr, obj_ptr, gcal_ptr_ptr, first_stn_ptr, sefd_prm);
*/
/*
------------------------ PLOT LABEL
*/
	cpgsvp( 0.0, 1.0, 0.0, 1.0 );
	cpgswin( 0.0, 1.0, 0.0, 1.0 );
	cpgsci(1);	cpgsch(1.0);
	cpgtext( 0.45, 0.975, "GAIN PLOT");

/*
	sprintf( pg_text, "MJD - %d", (int)mjd_min);
	cpgtext( 0.45, 0.025, pg_text);
*/

	cpgsvp( 0.1, 0.5, 0.1, 1.0 );
	cpgswin( 0.0, 1.0, 0.0, 1.0 );
	cpglab("SEC Z", "SEFD [Jy]", "");

	cpgsvp( 0.58, 1.0, 0.1, 1.0 );
	cpgswin( 0.0, 1.0, 0.0, 1.0 );
	sprintf( pg_text, "MJD - %d", (int)mjd_min);
	cpglab(pg_text, "PHASE [rad]", "");
/*
------------------------ PLOT GAIN
*/
	first_obj_ptr	= obj_ptr;
	first_gcal_ptr	= gcal_ptr_ptr;
	stn_index	= 0;
/*
------------------------ STATION LOOP
*/
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

			if( gcal_ptr->real > gain_max ){
				gain_max	= gcal_ptr->real;
			}
			gcal_ptr = gcal_ptr->next_gcal_ptr;
		}

		/*-------- PLOT FRAME --------*/
		cpgsvp( 0.10, 0.475,
			0.10 + 0.9*(float)stn_index/((float)stn_num),
			0.05 + 0.9*(float)(stn_index + 1)/((float)stn_num) );

		x_min	= 0.0; x_max	= 6.0;
		y_min	= 0.0; y_max	= 1.2*(float)gain_max;

		cpg_incr( (x_max - x_min), &x_incr);
		cpg_incr( (y_max - y_min), &y_incr);

		cpgsch(0.75);
		cpgswin( x_min, x_max, y_min, y_max);
		cpgsci(14); cpgrect( x_min, x_max, y_min, y_max);
		cpgsci(0);	cpgbox( "G", x_incr, 1,
							"G", y_incr, 1 );
		cpgsci(13);	cpgbox( "BCNTS", x_incr, 10,
							"BCNTS", y_incr*2, 5 );
		cpgsch(1.0);

		/*-------- PLOT DATA --------*/
		gcal_ptr	= *gcal_ptr_ptr;

		cpgsci(stn_index + 2);
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
			printf("%lf %lf  %lf  %lf\n",
				gcal_ptr->mjd, 
				gcal_ptr->real,
				RADDEG*asin(sin_el),
				gcal_ptr->weight );
			#ifdef DEBUG
			#endif

/*
			x_data	= (float)(gcal_ptr->mjd - (long)mjd_min);
*/
			x_data	= (float)(1.0/sin_el);
			y_data	= (float)gcal_ptr->real;
			cpgpt( 1, &x_data, &y_data, 17);
			gcal_ptr = gcal_ptr->next_gcal_ptr;
		}

		/*-------- PLOT SEFD MODEL --------*/

#ifdef HIDOI
		for(data_index=0; data_index<MAX_DATA; data_index++){
			x_data	= x_min + data_index * (x_max - x_min)/MAX_DATA;
			y_data	= (float)(sefd_prm[stn_index*3]
					* exp(sefd_prm[stn_index*3 + 2] * (double)x_data)
					+ sefd_prm[stn_index*3 + 1]);

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
			0.10 + 0.9*(float)stn_index/((float)stn_num),
			0.05 + 0.9*(float)(stn_index + 1)/((float)stn_num) );

		x_min	= 1.2*(float)(mjd_min - (long)mjd_min)
				- 0.2*(float)(mjd_max - (long)mjd_min);
		x_max	= 1.2*(float)(mjd_max - (long)mjd_min)
				- 0.2*(float)(mjd_min - (long)mjd_min);
		y_min	= -M_PI;
		y_max	= M_PI;

/*
		x_incr	= exp(M_LN10*((int)(log10(x_max - x_min) - 1.5)));
		y_incr	= exp(M_LN10*((int)(log10(y_max - y_min) - 0.5)));
*/

		cpg_incr( (x_max - x_min), &x_incr);
		cpg_incr( (y_max - y_min), &y_incr);

		cpgsch(0.75);
		cpgswin( x_min, x_max, y_min, y_max);
		cpgsci(14); cpgrect( x_min, x_max, y_min, y_max);
		cpgsci(0);	cpgbox( "G", x_incr, 1,
							"G", y_incr, 1 );
		cpgsci(13);	cpgbox( "BCNTS", x_incr, 10,
							"BCNTS", y_incr*2, 5 );
		cpgsch(1.0);

		/*-------- PLOT PHASE DATA --------*/
		cpgsci(stn_index + 2);
		while( gcal_ptr != NULL ){
			x_data	= (float)(gcal_ptr->mjd - (long)mjd_min);
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
