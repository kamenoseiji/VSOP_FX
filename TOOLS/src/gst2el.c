/*********************************************
**	GST2EL : Calculate EL at the			**
**			 Specified LST.					**
**											**
**	AUTHOUR	: KAMENO Seiji					**
**	CREATED	: 1994/7/25						**
**********************************************/

#include <math.h>

double	gst2el( gst, lambda, phi, ra, dec, sin_el_ptr )

	double	gst;		/* INPUT : Greenwich Sidereal Time [rad] */
	double	lambda;		/* INPUT : Longitude [rad] */
	double	phi;		/* INPUT : Latitude [rad] */
	double	ra;			/* INPUT : Right Ascension [rad] */
	double	dec;		/* INPUT : Declination [rad] */
	double	*sin_el_ptr;/* OUTPUT: Pointer of sin(EL) */
{
	*sin_el_ptr = sin(phi)*sin(dec) + cos(phi)*cos(dec)*cos(gst - lambda - ra);
	return(*sin_el_ptr);
}
