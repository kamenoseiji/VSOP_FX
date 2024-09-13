/*****************************************************
**	B1950TOJ2000.C : Convert (RA, DEC) in B1950.0	**
**						To J2000.0					**
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

#define		XX	 0.9999256782
#define		XY	-0.0111820610
#define		XZ	-0.0048579477
#define		YX	 0.0111820609
#define		YY	 0.9999374784
#define		YZ	-0.0000271765
#define		ZX	 0.0048579479
#define		ZY	-0.0000271474
#define		ZZ	 0.9999881997

long	b1950toj2000(ra_1950, dec_1950, ra_2000_ptr, dec_2000_ptr)
	double	ra_1950;		/* INPUT : Right Ascension [B1950, rad] */
	double	dec_1950;		/* INPUT : Declination [B1950, rad] */
	double	*ra_2000_ptr;	/* OUTPUT: Right Ascension [J2000, rad] */
	double	*dec_2000_ptr;	/* OUTPUT: Declination [J2000, rad] */
{
	double	x_1950;			/* X-Coordinate in B1950 */
	double	y_1950;			/* Y-Coordinate in B1950 */
	double	z_1950;			/* Z-Coordinate in B1950 */
	double	x_2000;			/* X-Coordinate in J2000 */
	double	y_2000;			/* Y-Coordinate in J2000 */
	double	z_2000;			/* Z-Coordinate in J2000 */

	/*-------- CONVERT (RA,DEC) -> (X,Y,Z) --------*/
	x_1950	= cos(ra_1950) * cos(dec_1950);
	y_1950	= sin(ra_1950) * cos(dec_1950);
	z_1950	= sin(dec_1950);

	/*-------- CONVERT (B1950.0) -> (J2000.0) --------*/
	x_2000	= XX*x_1950	+ XY*y_1950	+ XZ*z_1950;
	y_2000	= YX*x_1950	+ YY*y_1950	+ YZ*z_1950;
	z_2000	= ZX*x_1950	+ ZY*y_1950	+ ZZ*z_1950;

	/*-------- CONVERT (X,Y,Z) -> (RA,DEC) --------*/
	*ra_2000_ptr	= atan2(y_2000, x_2000);
	*dec_2000_ptr	= asin(z_2000);

	if(*ra_2000_ptr < 0.0){
		*ra_2000_ptr	+= PI2;
	}

	return(0);
}
