/*********************************************************
**	ANT2CL.C	: SUFFIX for Closure					**
**														**
**	FUNCTION : Input Three Numbers of Antenna which form**
**				a Triangle and Returns Closure number	**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/2/8									**
*********************************************************/

#include <stdio.h>
#include <stdlib.h>

long	ant2cl(ant1, ant2, ant3)

	long	ant1;				/* INPUT : NUMBER OF ANTENNA a */
	long	ant2;				/* INPUT : NUMBER OF ANTENNA b */
	long	ant3;				/* INPUT : NUMBER OF ANTENNA c */
{
	long	i, j, k;
	/*-------- SORT --------*/
	if(ant1>ant2){
		if(ant2>ant3){
			i=ant1; j=ant2; k=ant3;
		} else {
			if(ant1>ant3){
				i=ant1; j=ant3; k=ant2;
			} else {
				i=ant3; j=ant1; k=ant2;
			}
		}
	} else {
		if(ant1>ant3){
			i=ant2; j=ant1; k=ant3;
		} else {
			if(ant2>ant3){
				i=ant2; j=ant3; k=ant1;
			} else {
				i=ant3; j=ant2; k=ant1;
			}
		}
	}

	return(i*(i-1)*(i-2)/6 + j*(j-1)/2 + k);
}
