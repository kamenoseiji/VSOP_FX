/*****************************************************
**	DIRSEND : Send DIR-1000 CommandsThrough GP-IB	**
**													**
**	FUNCTION: DIRSEND sends commands for DIR-1000	**
**				through GP-IB and TCU. The Action	**
**				timing should be defined by TSS_ACT	**
**				or SOY_ACT. The both == 0 means	 	**
**				instant command.					**
**	AUTHOUR	: KAMENO Seiji							**
**	CREATED	: 1994/11/1								**
******************************************************/

#include <stdio.h>
#include <sys/ugpib.h> 
#include <sys/types.h> 
#define	WAIT	100

int dirsend(board, addr, dircmd, tss_act, soy_act)
	int			board;				/* GP-IB Board ID			*/
	Addr4882_t	addr;				/* GP-IB Address of TCU		*/
	int			tss_act;			/* TSS-ID of the ACTION		*/
	int			soy_act;			/* Second of Year of the ACTION */
	char		*dircmd;			/* DIR-1000 Command			*/
{
	char	gpcmd[256];			/* GP-IB Command */
	long	doy;				/* Day of Year */
	long	hh;					/* Hour */
	long	mm;					/* Minute */
	long	ss;					/* Second */
	short	result;				/* GP-IB SRQ Status Byte */
/*
--------------------------------------- Command Setting
*/
	sprintf(gpcmd, "RECCOM=%s\r", dircmd);
	Send(board, addr, gpcmd, strlen(gpcmd), NLend); fsleep(WAIT);
	ReadStatusByte(board, addr, &result); DevClear(board, addr);
	fsleep(WAIT);
/*
--------------------------------------- ACTION Timing
*/
	/*-------- Immediate ACTION --------*/
	if((soy_act == 0) && (tss_act == 0)){
		sprintf(gpcmd, "TSSACT,SENDCM\r");

	/*-------- UTCACT ACTION --------*/
	} else if(tss_act == 0){
		soy2dhms(soy_act, &doy, &hh, &mm, &ss);
		sprintf(gpcmd, "UTCACT=%02d%02d%02d,SENDCM\r", hh, mm, ss);

	/*-------- TSSACT ACTION --------*/
	} else {
		sprintf(gpcmd, "TSSACT=%07d,SENDCM\r", tss_act);
	}

	/*-------- Send ACTION --------*/
	Send(board, addr, gpcmd, strlen(gpcmd), NLend); fsleep(WAIT);
	ReadStatusByte(board, addr, &result); DevClear(board, addr);
	fsleep(WAIT);
/*
--------------------------------------- RECANS
*/
	sprintf(gpcmd, "RECANS\r\n");
	Send(board, addr, gpcmd, strlen(gpcmd), NLend); fsleep(WAIT);
	ReadStatusByte(board, addr, &result); DevClear(board, addr);
	fsleep(WAIT);
/*
--------------------------------------- Ending
*/
	return(0);
}
