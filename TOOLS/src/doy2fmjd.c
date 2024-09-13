/*********************************************
**	DOY2FMJD : Convert Modified Julian Data	**
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
#define	SEC_PER_DAY		86400

double	doy2fmjd(year, doy, hh, mm, ss, mjd_ptr)
	long	year;		/* INPUT : Calender Year */
	long	doy;		/* INPUT : Day Of Year */
	long	hh;			/* INPUT : hour */
	long	mm;			/* INPUT : Minute */
	double	ss;			/* INPUT : Second */
	double	*mjd_ptr;	/* OUTPUT: Modified Julian Date (Pointer) */
{
	long	mjd;

	if((year < 1901) || (year >2099)){
		return(-1);
	}
/*----------------------------- YEAR & DOY -> MJD ----------------------*/

	mjd		= (year - 1901)/4 * DAY_PER_4YEAR 			/* Days From 1901/1/0 */
			+ (year - 1901)%4 * DAY_PER_YEAR
			+ doy + MJD_1901;

	*mjd_ptr= ( (double)((hh*60 + mm)*60) + ss ) / SEC_PER_DAY;
	*mjd_ptr += (double)mjd;

	return(*mjd_ptr);
}
