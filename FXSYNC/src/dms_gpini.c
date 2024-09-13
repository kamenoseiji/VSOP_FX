/*********************************************************
**	DMS_GPINI.C : Initialize GPIB in dms_ptr 			**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/11/1									**
**********************************************************/

#include <sys/ugpib.h>

int dms_gpini( dms_addr,	dms_ptr )
	int		dms_addr;			/* GPIB ADDRESS of dms_ptr	*/
	int		*dms_ptr;			/* GPIB DEVICE of dms_ptr	*/
{
	char	cmd[8];
/*
-------------------------------------------- OPEN GPIB ADDRESS
*/

	/*-------- GP-IB CONTROLLER --------*/
	sprintf(cmd, "dev%d", dms_addr);
	*dms_ptr=ibfind(cmd);						/* Get Master-tcu_ptr Address */
	if( *dms_ptr < 0 ) {
		printf("cannot open GP-IB on the dms_ptr.\n");
		return(-1);
	}
/*
-------------------------------------------- CLEAR GP-IB BUFFER
*/
	ibclr(*dms_ptr);							/* CLEAR COMM BUFFER */
	ibtmo(*dms_ptr, T1s);						/* SET TIME OUT = 1 sec */
/*
-------------------------------------------- RETURN
*/
	return(1);
}
