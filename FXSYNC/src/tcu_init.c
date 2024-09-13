/*********************************************************
**	TCUSYNC_GPINI.C : Initialize GPIB in TCUSYNC.C		**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/11/1									**
**********************************************************/

#include <sys/ugpib.h>

int tcu_init(BRD,  TCU, time_base, rec_rate )
	int			BRD;				/* GPIB BOARD ID			*/
	Addr4882_t	TCU;				/* GPIB DEVICE of TCU		*/
	int			time_base;			/* Time Base in MHz			*/
	int			rec_rate;			/* Rec Rate in MHz			*/
{
	char	cmd[16];				/* GP-IB Command to TCU		*/
	short	result;					/* Serial Poll Status Byte	*/
/*
-------------------------------------------- SEND INITIAL PARAMETERS
*/
	sprintf(cmd, "TIBASE=%02dM\r", time_base);
		Send(BRD, TCU,  cmd, strlen(cmd), NLend);
		fsleep(100); ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);

	sprintf(cmd, "RECCLK=%02dM\r", rec_rate);
		Send(BRD, TCU, cmd, strlen(cmd), NLend);
		fsleep(100); ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);

	sprintf(cmd, "UT1PPS=R\r" );
		Send(BRD, TCU, cmd, strlen(cmd), NLend);
		fsleep(100); ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);

	dirsend(BRD, TCU, "2893023C00", 0, 0);	/* Stand-by Off Time = 60 sec	*/
	dirsend(BRD, TCU, "28900183", 0, 0);	/* REC = 'REFIN'				*/
	dirsend(BRD, TCU, "289201FF", 0, 0);	/* REC Disable					*/
/*
-------------------------------------------- RETURN
*/
	return(0);
}
