/*************************************************
**	SOY2DHMS : Convert Second of Year to		**
**				DOY, HH, MM, SS					**
**	AUTHOUR	: KAMENO Seiji						**
**	CREATED	: 1994/11/1							**
**************************************************/

#include <stdio.h>
long soy2dhms(sc, doy_ptr, hh_ptr, mm_ptr, ss_ptr)

	unsigned long	sc;				/* [INPUT]  Second of Year*/
	unsigned long	*doy_ptr;		/* [OUTPUT] Pointer of Day of Year	*/
	unsigned long	*hh_ptr;		/* [OUTPUT] Pointer of Hour (UTC)	*/
	unsigned long	*mm_ptr;		/* [OUTPUT] Pointer of Minute		*/
	unsigned long	*ss_ptr;		/* [OUTPUT] Pointer of Second		*/
{
	*doy_ptr= sc/86400 + 1;				/* Day Number */
	*hh_ptr	= (sc%86400)/3600;			/* Hour */
	*mm_ptr	= (sc%3600)/60;				/* Minute */
	*ss_ptr	= sc%60;					/* Second */
/*
--------------------------------------- Verification
*/
	if(sc != 
		(((*doy_ptr - 1)*24 + (*hh_ptr))*60 + (*mm_ptr))*60 + (*ss_ptr)){
		return(-1);
	}
/*
--------------------------------------- Ending
*/
	return(0);
}
