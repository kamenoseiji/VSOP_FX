/*****************************************************
**	FORT_DOY2FMJD : Convert Modified Julian Data	**
**			 to Day Of Year.						**
**													**
**	CAUTION	: Available between						**
**				1901 and 2099						**
**	AUTHOUR	: KAMENO Seiji							**
**	CREATED	: 1994/7/25								**
******************************************************/

#define	MJD_1901		15384
#define	DAY_PER_YEAR	365
#define	DAY_PER_4YEAR	1461
#define	SEC_PER_DAY		86400

double	fort_doy2fmjd_(year_ptr, doy_ptr, hh_ptr, mm_ptr, ss_ptr, mjd_ptr)
	long	*year_ptr;		/* INPUT : Calender Year */
	long	*doy_ptr;		/* INPUT : Day Of Year */
	long	*hh_ptr;		/* INPUT : hour */
	long	*mm_ptr;		/* INPUT : Minute */
	double	*ss_ptr;		/* INPUT : Second */
	double	*mjd_ptr;		/* OUTPUT: Modified Julian Date (Pointer) */
{
	long	mjd;

	if((*year_ptr < 1901) || (*year_ptr >2099)){
		return(-1);
	}
/*----------------------------- YEAR & DOY -> MJD ----------------------*/

	mjd		= (*year_ptr - 1901)/4 * DAY_PER_4YEAR 		/* Days From 1901/1/0 */
			+ (*year_ptr - 1901)%4 * DAY_PER_YEAR
			+ *doy_ptr + MJD_1901;

	*mjd_ptr= ( (double)(((*hh_ptr)*60 + (*mm_ptr))*60)
				+ (*ss_ptr) ) / SEC_PER_DAY;
	*mjd_ptr += (double)mjd;
	return(*mjd_ptr);
}
