/*********************************************************
**	ANT2BL.C	: SUFFIX for Baseline					**
**														**
**	FUNCTION : Input Two Numbers of Antenna which form	**
**				a Baseline and Returns Baseline number	**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/2/8									**
*********************************************************/

#include <stdio.h>
#include <stdlib.h>

long	ant2bl(ant1, ant2)

	long	ant1;				/* INPUT : NUMBER OF ANTENNA a*/
	long	ant2;				/* INPUT : NUMBER OF ANTENNA b*/
{
	if(ant1 > ant2){
		return( ant1*(ant1 - 1)/2 + ant2 );
	} else {
		return( ant2*(ant2 - 1)/2 + ant1 );
	}
}
