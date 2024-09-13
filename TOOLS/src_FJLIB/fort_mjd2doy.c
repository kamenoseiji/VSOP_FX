/*************************************************
**	FORT_MJD2DOY : Convert Modified Julian Data	**
**			 to Day Of Year.					**
**												**
**	CAUTION	: Available between					**
**				1901 and 2099					**
**	AUTHOUR	: KAMENO Seiji						**
**	CREATED	: 1994/7/25							**
**************************************************/

#define	MJD_1901		15384
#define	DAY_PER_YEAR	365
#define	DAY_PER_4YEAR	1461

long	fort_mjd2doy_(mjd_ptr, year_ptr, doy_ptr)
	long	*mjd_ptr;	/* INPUT : Modified Julian Date (Pointer) */
	long	*year_ptr;	/* OUTPUT:Calender Year (Pointer) */
	long	*doy_ptr;	/* OUTPUT:Day Of Year (Pointer) */
{
	long	day_per_year;
	long	day;						/* Days from 1990/1/0 */

	/*-------- Illegal MJD --------*/
	if((*mjd_ptr < MJD_1901) || (*mjd_ptr > 88433)){
		return(-1);
	}

/*----------------------------- MJD -> YEAR & DOY ----------------------*/
	day_per_year= DAY_PER_YEAR;
	day			= *mjd_ptr - MJD_1901;				/* Days From 1901/1/0 */

	*doy_ptr	= (day % 1461) % day_per_year;		/* Day of Year */
	*year_ptr	= 1901 + 4*(day / 1461)				/* Calender Year */
				+ (day % 1461) / day_per_year;

	if( ((day-1)%1461)/day_per_year == 3 ){			/* Leap Year */
		*doy_ptr	= day % 1461 - 3*day_per_year;	/* Day of Year */
		day_per_year		= 366;
	}

	if( *doy_ptr == 0){								/* Final Day in Year */
		*doy_ptr = day_per_year;
		*year_ptr= *year_ptr - 1;
	}

	return(*doy_ptr);
}
