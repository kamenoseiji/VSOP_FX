/*********************************************
**	MJD2GMST : Convert MJD to Greenwidge	**
**				Mean Sidereal Time			**
**											**
**	CAUTION	: Use IAU Definition in 1984	**
**	AUTHOUR	: KAMENO Seiji					**
**	CREATED	: 1994/7/25						**
**********************************************/

#define	FACT0	24110.54841	
#define	FACT1	8640184.812866
#define	FACT2	0.093104
#define	FACT3	0.0000062
#define MJD_EPOCH	51544.5				/* MJD at 2000 1/1 12:00:00 UT */
#define TU_UNIT		36525.0				/* Tu Unit */
#define SEC_PER_DAY	86400.0 
#define PI2			6.283185307179586476925286766559; 

double	mjd2gmst(mjd, ut1utc, gmst_ptr)
	double	mjd;		/* INPUT : Modified Julian Date */
	double	ut1utc;		/* INPUT : UT1 - UTC [sec] */
	double	*gmst_ptr;	/* OUTPUT: Greenwidge Mean Sidereal Time [rad] */
{
	double	tu;			/* From 2000 1/1 12:00:00 UT */
	double	ut1;
	double	gmst;

	tu			= ( mjd - MJD_EPOCH ) / TU_UNIT;
	ut1			= ( mjd - (long)mjd ) * SEC_PER_DAY + ut1utc;
	gmst		= (ut1 + FACT0 + ((FACT3*tu + FACT2)*tu + FACT1)*tu)
				/ SEC_PER_DAY;
	gmst		= gmst - (long)gmst;
	if(gmst < 0.0){	gmst = gmst + 1.0;}
	*gmst_ptr	= gmst * PI2; 

	return(*gmst_ptr);
}
