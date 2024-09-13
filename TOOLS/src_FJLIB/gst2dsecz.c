/*********************************************
**	GST2DSECZ : Calculate EL at the			**
**			 Specified LST.					**
**											**
**	AUTHOUR	: KAMENO Seiji					**
**	CREATED	: 1994/7/25						**
**********************************************/

#include <math.h>
#define	RADDEG	0.02617993877991494365		/* 2PI*360/86400 */

double	gst2dsecz( degday, gst, lambda, phi, ra, dec, sin_el, dsecz_ptr )

	double	degday;		/* INPUT : Degree Per Day */
	double	gst;		/* INPUT : Greenwich Sidereal Time [rad] */
	double	lambda;		/* INPUT : Longitude [rad] */
	double	phi;		/* INPUT : Latitude [rad] */
	double	ra;			/* INPUT : Right Ascension [rad] */
	double	dec;		/* INPUT : Declination [rad] */
	double	sin_el;		/* INPUT : sin(EL) */
	double	*dsecz_ptr;	/* OUTPUT: Pointer of d[secz]/dt */
{
	*dsecz_ptr	= RADDEG*cos(dec)*cos(phi)*sin(gst - lambda - ra)
				/ (sin_el*sin_el*degday);

	return(*dsecz_ptr);
}
