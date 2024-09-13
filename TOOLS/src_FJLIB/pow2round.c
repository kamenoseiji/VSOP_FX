/*****************************************************************
**	POW2ROUND.C		CALCULATE THE MINIMUM NUMBER OF POWERS OF 2	**
**					WHICH IS LARGER THAN THE GIVEN NUMBER		**
**	FUNCITON:	INPUT	NATURAL NUMBER K ( > 0) 				**
**				OUTPUT	2^n > K (n is natural number)			**
**	AUTHOR:		KAMENO Seiji									**
**	CREATE:		1995/12/21										**
*****************************************************************/

long	pow2round( k )
	long	k;					/* INPUT NUMBER */
{
	long	i;					/* RESULT */

	/*-------- CHECK FOR INPUT NUMBER --------*/
	if(k < 1){	return(-1);}	/* INVALID INPUT */

	i = 1;						/* INITIALIZE RESULT */

	/*-------- LOOP FOR POWERS OF TWO --------*/
	while(i < k){
		i = i << 1;
	}
	return(i);
}
