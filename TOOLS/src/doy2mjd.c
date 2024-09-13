/*********************************************
**	DOY2MJD : Convert Modified Julian Data	**
**			 to Day Of Year.				**
**											**
**	CAUTION	: Available between				**
**				1901 and 2099				**
**	AUTHOUR	: KAMENO Seiji					**
**	CREATED	: 1994/7/25						**
**********************************************/

#define	MJD_1901		15384
#define	DAY_PER_YEAR	365
#define	DAY_PER_4YEAR	1461

long	doy2mjd(year, doy, mjd_ptr)
	long	year;		/* INPUT : Calender Year */
	long	doy;		/* INPUT : Day Of Year */
	long	*mjd_ptr;	/* OUTPUT: Modified Julian Date (Pointer) */
{

	if((year < 1901) || (year >2099)){
		return(-1);
	}
/*----------------------------- YEAR & DOY -> MJD ----------------------*/

	*mjd_ptr= (year - 1901)/4 * DAY_PER_4YEAR 			/* Days From 1901/1/0 */
			+ (year - 1901)%4 * DAY_PER_YEAR
			+ doy + MJD_1901;

	return(*mjd_ptr);
}
