/*************************************************
**	DHMS2SOY : Convert DOY, HH, MM, SS to		**
**				Second of Year.					**
**	AUTHOUR	: KAMENO Seiji						**
**	CREATED	: 1994/11/1							**
**************************************************/

#include <stdio.h>
long dhms2soy(doy, hh, mm, ss, soy_ptr)

	unsigned long	doy;			/* [INPUT]  Pointer of Day of Year	*/
	unsigned long	hh;				/* [INPUT]  Pointer of Hour (UTC)	*/
	unsigned long	mm;				/* [INPUT]  Pointer of Minute		*/
	unsigned long	ss;				/* [INPUT]  Pointer of Second		*/
	unsigned long	*soy_ptr;		/* [OUTPUT] Pointer of Second of Year*/
{

	*soy_ptr	= (((doy - 1)*24 + hh)*60 + mm)*60 + ss;
/*
--------------------------------------- Verification
*/
	doy	= (*soy_ptr)/86400 + 1;				/* Day Number */
	hh	= ((*soy_ptr)%86400)/3600;			/* Hour */
	mm	= ((*soy_ptr)%3600)/60;				/* Minute */
	ss	= (*soy_ptr)%60;					/* Second */
/*
--------------------------------------- Ending
*/
	return(0);
}
