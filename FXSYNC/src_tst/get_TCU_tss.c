/*****************************************************
**	get_TCU_tss : Read TSS ID for Specified TCU 	**
**	AUTHOUR	: KAMENO Seiji							**
**	CREATED	: 1994/11/1								**
******************************************************/

#include "fxsync.inc"
#include <stdio.h>
#include <unistd.h>
#include <sys/ugpib.h>
unsigned int get_TCU_tss(BRD, TCU, tss_ptr, frc_ptr )
	int			BRD;		/* GP-IB board ID		*/
	Addr4882_t	TCU;		/* Device I/O of TCU	*/
	int			*tss_ptr;	/* Pointer of TSS ID	*/
	int			*frc_ptr;	/* Pointer of Fraction	*/
{
	char			cmd[64];
	short			result;
/*
--------------------------------------- UTCSET to TCU
*/
	memset(cmd, 0, 64);
	sprintf(cmd, "UTCACT,TSSSET?\r\n");
	Send(BRD, TCU, cmd, strlen(cmd), NLend); fsleep(100);
	Receive(BRD, TCU, cmd, 16, STOPend); fsleep(100);

	if( cmd[0] != 'B' ){
		sscanf(cmd, "%07d%*1s%d", tss_ptr, frc_ptr);
		ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);
		return(1);
	} else {
		*tss_ptr = -1;	*frc_ptr = -1;
		ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);
		return(-1);
	}
/*
--------------------------------------- Ending
*/
}
