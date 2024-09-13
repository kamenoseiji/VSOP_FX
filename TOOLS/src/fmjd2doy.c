/*********************************************
**	FMJD2DOY : Convert Modified Julian Data	**
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

long	fmjd2doy(mjd, year_ptr, doy_ptr, hh_ptr, mm_ptr, ss_ptr)
	double	mjd;		/* INPUT : Modified Julian Date (Pointer) */
	long	*year_ptr;	/* OUTPUT:Calender Year (Pointer) */
	long	*doy_ptr;	/* OUTPUT:Day Of Year (Pointer) */
	long	*hh_ptr;	/* OUTPUT:Hour (Pointer) */
	long	*mm_ptr;	/* OUTPUT:Minute (Pointer) */
	double	*ss_ptr;	/* OUTPUT:Second (Pointer) */
{
	long	day_per_year;
	long	day;						/* Days from 1990/1/0 */

	/*-------- Illegal MJD --------*/
	if((mjd < MJD_1901) || (mjd > 88433)){
		return(-1);
	}

/*----------------------------- MJD -> YEAR & DOY ----------------------*/
	day_per_year= DAY_PER_YEAR;
	day			= (long)mjd - MJD_1901;				/* Days From 1901/1/0 */

	*doy_ptr	= (day % 1461) % day_per_year;		/* Day of Year */
	*year_ptr	= 1901 + 4*(day / 1461)				/* Calender Year */
				+ (day % 1461) / day_per_year;

	if( ((day-1)%1461)/day_per_year == 3 ){			/* Leap Year */
		*doy_ptr	= day % 1461 - 3*day_per_year;	/* Day of Year */
		day_per_year		= 366;
	}

	if( *doy_ptr == 0){								/* Final Day in Year */
		*doy_ptr = day_per_year;
		*year_ptr --;
	}

	*ss_ptr	= ( mjd - (long)mjd ) * SEC_PER_DAY;
	*mm_ptr	= ((long)(*ss_ptr)/60) % 60;
	*hh_ptr	= ((long)(*ss_ptr))/3600;
	*ss_ptr -= ( *hh_ptr * 60 + *mm_ptr )*60; 

	return(*doy_ptr);
}
