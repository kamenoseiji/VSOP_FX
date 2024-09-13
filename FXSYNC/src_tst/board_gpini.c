/*********************************************************
**	BOARD_GPINI.C : Initialize GPIB Controller			**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/11/1									**
**********************************************************/

#include <sys/ugpib.h>

int board_gpini( board_ptr )
	int		*board_ptr;				/* GPIB DEVICE of CONTROLLER */
{
/*
-------------------------------------------- OPEN GPIB ADDRESS
*/
	/*-------- GP-IB CONTROLLER --------*/
	*board_ptr=ibfind("gpib0");					/* Get Board Address */
	if( *board_ptr < 0 ){
		printf("cannot open GP-IB board.\n");	
		return(-1);
	}

	ibsic(*board_ptr);							/* Interface Clear */
	ibsre(*board_ptr, 1);						/* Remote Enable */
/*
-------------------------------------------- CLEAR GP-IB BUFFER
*/
	ibclr(*board_ptr);							/* CLEAR COMM BUFFER */
	ibeot(*board_ptr,1);	ibclr(*board_ptr);	/* END OF TRANSMISSION */
	ibeos(*board_ptr,0x1482);ibclr(*board_ptr);	/* END OF STRING */
	ibtmo(*board_ptr, T1s);						/* SET TIME OUT = 1 sec */
/*
-------------------------------------------- RETURN
*/
	return(1);
}
