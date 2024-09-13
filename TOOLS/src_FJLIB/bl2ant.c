/*********************************************************
**	BL2ANT.C	: SUFFIX for Antenna from Baseline Num	**
**														**
**	FUNCTION : Input Baseline Number and Returns Two	**
**				Antenna numbers which form the Baseline	**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/2/8									**
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long	bl2ant(blnum, ant1, ant2)

	long	blnum;				/* INPUT : NUMBER OF Baseline */
	long	*ant1;				/* OUTPUT: Pointer of NUMBER OF ANTENNA a */
	long	*ant2;				/* OUTPUT: Pointer of NUMBER OF ANTENNA b */
{
	long	i;

	i = (long)(sqrt((double)(blnum*2)));
	while( (i*(i + 1)/2) > blnum){
		i--;
	}
	*ant1 = i+1; 
	*ant2 = blnum - i*(i+1)/2;

	return(0);
}
