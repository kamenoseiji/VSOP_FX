/*************************************************
**	SOY2MDHMS : Convert Second of Year to		**
**	CREATED	: 1994/11/1							**
**************************************************/

#include "drudge.inc"

int soy2mdhms(year, soy, mdhms_ptr)

	unsigned long	year;			/* [INPUT]  Year					*/
	unsigned long	soy;			/* [INPUT]  Second of Year			*/
	struct mdhms	*mdhms_ptr;		/* [OUTPUT] Pointer of Second		*/
{
	int		leap;					/* 1 -> leap year, 0 -> usual year	*/
	int		doy;					/* Day of Year						*/
	int		mon_idx;				/* Month Number						*/

	static int	month_day[12] = {
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	static int	leap_day[12] = {
		31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	leap = 0;
	if( (year%100 != 0) && (year%4 == 0) ){	leap = 1;}
	if( year%400 == 0 ){ leap = 1;}

	doy = soy/86400 + 1;				/* Day Number */
	mon_idx = 0;
	if( leap == 1 ){						/* In Case of Leap Year		*/
		while( doy > leap_day[mon_idx] ){
			doy -= leap_day[mon_idx];
			mon_idx ++;
		}
	} else {								/* In Case of Usual Year	*/
		while( doy > month_day[mon_idx] ){
			doy -= month_day[mon_idx];
			mon_idx ++;
		}
	}

	mdhms_ptr->mon		= mon_idx + 1;			/* Month	*/
	mdhms_ptr->day		= doy;					/* Day		*/
	mdhms_ptr->hour		= (soy%86400)/3600;		/* Hour		*/
	mdhms_ptr->min		= (soy%3600)/60;		/* Minute	*/
	mdhms_ptr->sec		= soy%60;				/* Second	*/
/*
--------------------------------------- Ending
*/
	return(0);
}
