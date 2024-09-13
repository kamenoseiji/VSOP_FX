/*********************************************************
**	CALC_EL.C	: Calculate Elevation			 		**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>
#include "obshead.inc"
#define RADDEG  57.29577951308232087721

int	calc_el( obs_ptr, obj_ptr, stn_ptr, mjd, sin_el, dsecz )
	struct	header		*obs_ptr;		/* Pointer of OBS HEADDER			*/
	struct	head_obj	*obj_ptr;		/* First Pointer of Objects Header	*/
	struct	head_stn	*stn_ptr;		/* First Pointer of Station Header	*/
	double	mjd;						/* Time [MJD]						*/
	double	*sin_el;					/* Pointer of sin(EL)				*/
	double	*dsecz;						/* Pointer of d[sec z]/dt			*/
{

	double	radius;		/* Earth Radius */
	double	sin_phi;	/* sin(latitude) */
	double	cos_phi;	/* cos(latitude) */
	double	lambda;		/* Longitude */
	double	gmst;		/* Greenwich Mean Sidereal Time */

	radius	= sqrt( stn_ptr->stn_pos[0]*stn_ptr->stn_pos[0]
			+	stn_ptr->stn_pos[1]*stn_ptr->stn_pos[1]
			+	stn_ptr->stn_pos[2]*stn_ptr->stn_pos[2] );
	sin_phi	= stn_ptr->stn_pos[2] / radius;
	cos_phi	= sqrt(1.0 - sin_phi*sin_phi);
	lambda	= atan2(stn_ptr->stn_pos[1]/(radius*cos_phi),
					stn_ptr->stn_pos[0]/(radius*cos_phi) );

	/*-------- CALC. CURRENT ELEVATION --------*/
	mjd2gmst(mjd, obs_ptr->ut1utc, &gmst);

	#ifdef DEBUG
	printf("SOURCE=%s : RA=%16.12lf DEC=%16.12lf\n",
		obj_ptr->obj_name,
		obj_ptr->obj_pos[0]/RADDEG, obj_ptr->obj_pos[1]/RADDEG );
	#endif

	gst2el(gmst, -lambda, atan2(sin_phi, cos_phi),
		obj_ptr->obj_pos[0]/RADDEG,
		obj_ptr->obj_pos[1]/RADDEG,
		sin_el);

	gst2dsecz( obs_ptr->degpdy, gmst, -lambda,
		atan2(sin_phi, cos_phi),
		obj_ptr->obj_pos[0]/RADDEG,
		obj_ptr->obj_pos[1]/RADDEG,
		*sin_el, dsecz);

	return(0);
}
