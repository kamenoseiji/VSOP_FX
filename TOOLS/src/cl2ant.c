/*********************************************************
**	CL2ANT.C	: SUFFIX for Antenna from Closure Num	**
**														**
**	FUNCTION : Input Closure Number and Returns Three	**
**				Antenna numbers which form the Closure	**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/2/8									**
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long	cl2ant(clnum, ant1, ant2, ant3)

	long	clnum;				/* INPUT : NUMBER OF Triangle */
	long	*ant1;				/* OUTPUT: Pointer of NUMBER OF ANTENNA a */
	long	*ant2;				/* OUTPUT: Pointer of NUMBER OF ANTENNA b */
	long	*ant3;				/* OUTPUT: Pointer of NUMBER OF ANTENNA c */
{
	long	i;

	if(clnum == 0){
		i = 1;
	} else {
		i = (long)exp(log((double)(clnum*6))/3.0) + 1;
	}
	while( (i*(i + 1)*(i-1)/6) > clnum){
		i--;
	}
	*ant1 = i + 1; 
	bl2ant((clnum - i*(i+1)*(i-1)/6), ant2, ant3);

	return(0);
}
