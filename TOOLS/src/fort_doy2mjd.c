/*************************************************
**	FORT_DOY2MJD : Convert Modified Julian Data	**
**			 to Day Of Year.					**
**	This is For FORTRAN							**
**												**
**	CAUTION	: Available between					**
**				1901 and 2099					**
**	AUTHOUR	: KAMENO Seiji						**
**	CREATED	: 1994/7/25							**
**************************************************/

#define	MJD_1901		15384
#define	DAY_PER_YEAR	365
#define	DAY_PER_4YEAR	1461

long	fort_doy2mjd_(year_ptr, doy_ptr, mjd_ptr)
	long	*year_ptr;		/* INPUT : Calender Year */
	long	*doy_ptr;		/* INPUT : Day Of Year */
	long	*mjd_ptr;		/* OUTPUT: Modified Julian Date (Pointer) */
{
	if((*year_ptr < 1901) || (*year_ptr >2099)){
		return(-1);
	}
/*----------------------------- YEAR & DOY -> MJD ----------------------*/

	*mjd_ptr= (*year_ptr - 1901)/4 * DAY_PER_4YEAR 		/* Days From 1901/1/0 */
			+ (*year_ptr - 1901)%4 * DAY_PER_YEAR
			+ *doy_ptr + MJD_1901;

	return(*mjd_ptr);
}
