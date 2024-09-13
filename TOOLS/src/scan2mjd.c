/*********************************************************
**	read_aos_log: Read AOS Log File 					**
**														**
**	FUNCTION: Open Drudge File and Read.				**
**				Data File.								**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#define		SCAN_FMT	"%04d%02d%02d%02d%02d%02d %04d%02d%02d%02d%02d%02d"

int	scan2mjd(
	char	*scan_buf,					/* 1-line buffer		*/
	double	*start_mjd,					/* MJD					*/
	double	*stop_mjd)					/* MJD					*/
{
	int		mon_date[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
	int		mon_leap[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
	int		start_year;					/* STRT YEAR				*/
	int		start_mon;					/* STRT MONTH				*/
	int		start_day;					/* STRT DATE				*/
	int		start_hour;					/* STRT HOUR				*/
	int		start_min;					/* STRT MINUTE				*/
	int		start_sec;					/* STRT SECOND 				*/
	int		start_doy;					/* STRT Day of YEAR			*/
	int		start_soy;					/* STRT Second of YEAR		*/
	int		stop_year;					/* STOP YEAR				*/
	int		stop_mon;					/* STOP MONTH				*/
	int		stop_day;					/* STOP DATE				*/
	int		stop_hour;					/* STOP HOUR				*/
	int		stop_min;					/* STOP MINUTE				*/
	int		stop_sec;					/* STOP SECOND 				*/
	int		stop_doy;					/* STOP Day of YEAR			*/
	int		stop_soy;					/* STOP Second of YEAR		*/
/*
------------------------------------------- PARSE the LINE
*/
	sscanf(scan_buf, SCAN_FMT,
		&start_year, &start_mon, &start_day, &start_hour, &start_min, &start_sec,
		&stop_year,  &stop_mon,  &stop_day,  &stop_hour,  &stop_min,  &stop_sec);
/*
------------------------------------------- MONTH/DATE -> DAY OF YEAR
*/
	if( start_year%4 == 0){
		start_doy = mon_leap[start_mon - 1] + start_day;
		stop_doy  = mon_leap[stop_mon - 1]  + stop_day;
	} else {
		start_doy = mon_date[start_mon - 1] + start_day;
		stop_doy  = mon_date[stop_mon - 1]  + stop_day;
	}
/*
------------------------------------------- JST -> UT
*/

	/*-------- DHMS -> SOY --------*/
	dhms2soy( start_doy, start_hour, start_min, start_sec, &start_soy);
	dhms2soy( stop_doy,  stop_hour,  stop_min,  stop_sec,  &stop_soy);

	/*-------- TIME DIFFERENCE -------*/
	start_soy -= 32400;			/* JST -> UT	*/
	start_soy -= 4;				/* Time Offset	*/
	stop_soy -= 32400;			/* JST -> UT	*/

	/*-------- SOY -> DHMS --------*/
	soy2dhms( start_soy, &start_doy, &start_hour, &start_min, &start_sec);
	soy2dhms( stop_soy,  &stop_doy,  &stop_hour,  &stop_min,  &stop_sec);

	/*-------- DHMS -> MJD --------*/
	doy2fmjd( start_year, start_doy, start_hour, start_min, (double)start_sec, start_mjd);
	doy2fmjd( stop_year,  stop_doy,  stop_hour,  stop_min,  (double)stop_sec,  stop_mjd);

	return(0);
}
