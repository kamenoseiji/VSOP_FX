/*****************************************************
**	J2000TOB1950.C : Convert (RA, DEC) in J2000.0	**
**						To B1950.0					**
**													**
**	AUTHOUR	: KAMENO Seiji							**
**	CREATED	: 1994/7/25								**
*****************************************************/

#include	<math.h>
#define	PI2	6.283185307179586476925286766559

/**********************************************
**	The Followings Are Matrix Which Converts **
**	B1950.0 -> J2000.0. See "TENTAI-NO-ICHI- **
**	KEISAN" (NAGASAWA KO, 1985) Published 	 **
**	by Chijin-Shokan, p.238.				 **
**********************************************/

#define		XX	 0.99992567813234756180
#define		XY	 0.01118206092933668461
#define		XZ	 0.00485794786277498909
#define		YX	-0.01118206097070957930
#define		YY	 0.99993747846619240871
#define		YZ	-0.00002714738683769079
#define		ZX	-0.00485794773736179623
#define		ZY	-0.00002717648739713432
#define		ZZ	 0.99998819976615449523

long	j2000tob1950(ra_2000, dec_2000, ra_1950_ptr, dec_1950_ptr)
	double	ra_2000;		/* INPUT : Right Ascension [J2000, rad] */
	double	dec_2000;		/* INPUT : Declination [J2000, rad] */
	double	*ra_1950_ptr;	/* OUTPUT: Right Ascension [B1950, rad] */
	double	*dec_1950_ptr;	/* OUTPUT: Declination [B1950, rad] */
{
	double	x_1950;			/* X-Coordinate in B1950 */
	double	y_1950;			/* Y-Coordinate in B1950 */
	double	z_1950;			/* Z-Coordinate in B1950 */
	double	x_2000;			/* X-Coordinate in J2000 */
	double	y_2000;			/* Y-Coordinate in J2000 */
	double	z_2000;			/* Z-Coordinate in J2000 */

	/*-------- CONVERT (RA,DEC) -> (X,Y,Z) --------*/
	x_2000	= cos(ra_2000) * cos(dec_2000);
	y_2000	= sin(ra_2000) * cos(dec_2000);
	z_2000	= sin(dec_2000);

	/*-------- CONVERT (J2000.0) -> (B1950.0) --------*/
	x_1950	= XX*x_2000	+ XY*y_2000	+ XZ*z_2000;
	y_1950	= YX*x_2000	+ YY*y_2000	+ YZ*z_2000;
	z_1950	= ZX*x_2000	+ ZY*y_2000	+ ZZ*z_2000;

	/*-------- CONVERT (X,Y,Z) -> (RA,DEC) --------*/
	*ra_1950_ptr	= atan2(y_1950, x_1950);
	*dec_1950_ptr	= asin(z_1950);

	if(*ra_1950_ptr < 0.0){
		*ra_1950_ptr	+= PI2;
	}

	return(0);
}
