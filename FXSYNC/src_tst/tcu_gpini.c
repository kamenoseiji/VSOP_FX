/*********************************************************
**	TCU_GPINI.C : Initialize GPIB in tcu_ptrSYNC.C		**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/11/1									**
**********************************************************/

#include <sys/ugpib.h>

int tcu_gpini( tcu_addr,	tcu_ptr )
	int		tcu_addr;			/* GPIB ADDRESS of tcu_ptr */
	int		*tcu_ptr;			/* GPIB DEVICE of tcu_ptr */
{
	char	cmd[8];
/*
-------------------------------------------- OPEN GPIB ADDRESS
*/

	/*-------- GP-IB CONTROLLER --------*/
	sprintf(cmd, "dev%d", tcu_addr);
	*tcu_ptr=ibfind(cmd);						/* Get Master-tcu_ptr Address */
	if( *tcu_ptr < 0 ) {
		printf("cannot open GP-IB on the tcu_ptr.\n");
		return(-1);
	}
/*
-------------------------------------------- CLEAR GP-IB BUFFER
*/
	ibclr(*tcu_ptr);							/* CLEAR COMM BUFFER */
	ibtmo(*tcu_ptr, T1s);						/* SET TIME OUT = 1 sec */
/*
-------------------------------------------- RETURN
*/
	return(1);
}
