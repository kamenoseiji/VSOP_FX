/*****************************************************
**	TCUCOPY_UTCSET : Set UTC befor CORR Start Time 	**
**	AUTHOUR	: KAMENO Seiji							**
**	CREATED	: 1994/11/1								**
******************************************************/
#include "fxsync.inc"
#include <stdio.h>
#include <unistd.h>
#include <sys/ugpib.h>

#define	WAIT		100			/* Wait Timer [msec] */

unsigned int tcu_fxstart(BRD, TCU, fxstart_tss )

	int			BRD;				/* GP-IB Board ID			*/
	Addr4882_t	TCU;				/* Device I/O of TCU		*/
	int			fxstart_tss;		/* TSSID at FXSTART			*/
{
	char		cmd[256];
	short		result;
/*
--------------------------------------- UTCSET to TCU
*/
	memset( cmd, 0, 256 );
	sprintf(cmd, "TSSACT=%07d\r", fxstart_tss );
	Send(BRD, TCU, cmd, strlen(cmd), NLend); fsleep(WAIT);
	printf("MASTER [%d bytes] : %s\n", strlen(cmd), cmd);
	ReadStatusByte(BRD, TCU, &result);	DevClear(BRD, TCU);

	memset( cmd, 0, 256 );
	sprintf(cmd, "ACTION=FXSTRT\r");
	Send(BRD, TCU, cmd, strlen(cmd), NLend); fsleep(WAIT);
	printf("MASTER [%d bytes] : %s\n", strlen(cmd), cmd);
	ReadStatusByte(BRD, TCU, &result);	DevClear(BRD, TCU);
/*
--------------------------------------- Ending
*/
	return(1);
}
