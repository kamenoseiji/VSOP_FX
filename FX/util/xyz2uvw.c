/*****************************************************
**	XYZ2UVW : Convert Station Position (x, y, z)	**
**			 to (u, v, w).							**
**													**
**	AUTHOUR	: KAMENO Seiji							**
**	CREATED	: 1994/7/25								**
*****************************************************/
#include	<math.h>

long	xyz2uvw( x, y, z, ha, dec, lambda, u_ptr, v_ptr, w_ptr )
	double	x;			/* INPUT : Station Position [m] */
	double	y;			/* INPUT : Station Position [m] */
	double	z;			/* INPUT : Station Position [m] */
	double	ha;			/* INPUT : Hour Angle of the Source [rad] */
	double	dec;		/* INPUT : Declination of the Source [rad] */
	double	lambda;		/* INPUT : Wavelength [m] */
	double	*u_ptr;		/* OUTPUT: Pointer of u [wavelength] */
	double	*v_ptr;		/* OUTPUT: Pointer of v [wavelength] */
	double	*w_ptr;		/* OUTPUT: Pointer of w [wavelength] */
{
	double	cs_ha;		/* cosine( hour angle ) */
	double	sn_ha;		/* sine( hour angle ) */
	double	cs_dec;		/* cosine( declination ) */
	double	sn_dec;		/* sine( declination ) */

	cs_ha = cos(ha);	sn_ha = sin(ha);
	cs_dec= cos(dec);	sn_dec= sin(dec);

	*u_ptr =( sn_ha*x			+ cs_ha*y)/lambda;
	*v_ptr =(-sn_dec*cs_ha*x	+ sn_dec*sn_ha*y	+ cs_dec*z)/lambda;
	*w_ptr =( cs_dec*cs_ha*x	- cs_dec*sn_ha*y	+ sn_dec*z)/lambda;

	return(0);
}
