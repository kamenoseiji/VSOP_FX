/*********************************************************
**	TAPE_REWIND.C : Tape Rewind to the Top				** 
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/11/1									**
**********************************************************/

#include "fxsync.inc"
#include <stdio.h>
#include <unistd.h>
#include <sys/ugpib.h>
#define TRUE_MASK   (1 << pb_id)
#define FALSE_MASK  ~(1 << pb_id)
/*
---------------------------------------------------- LOAD PARAMETERS
*/
tape_rewind(fxsync_ptr, BRD, TCU, pb_id)
	struct shared_mem1 *fxsync_ptr;  /* Pointer of Shared Memory			*/
	int			BRD;		/* GP-IB Board ID							*/
	Addr4882_t	TCU;		/* GP-IB Address of TCU						*/
	int			pb_id;		/* Plaback ID								*/
{
	/*-------- STATUS  --------*/
	int		istat;			/* DIR-1000 TTP-SENSE Status				*/
	char	dir_stat[32];	/* DIR-1000 TTP-SENSE Status				*/
	short	result;			/* SRQ Status Byte							*/
/*
---------------------------------------------------- TAPE REWIND
*/
	dirsend(BRD, TCU, "2020", 0, 0);		/* SEND REWIND COMMAND	*/
	while(1){
		dirsend(BRD, TCU, "3001", 0, 0);	/* SEND TTP-SENSE COMMAND	*/
		Receive(BRD, TCU, dir_stat, 20, STOPend);
		ReadStatusByte(BRD, TCU, &result ); DevClear(BRD, TCU);
		sscanf(dir_stat, "%8X", &istat);

		if( (istat & STANDBY_STAT) == STANDBY_STAT){	break;}
		if(fxsync_ptr->validity == FINISH){ return(-1); }
		if((fxsync_ptr->rewind_done & TRUE_MASK) != 0){ return(0); }
		sleep(2);
	}
	fxsync_ptr->rewind_done |= TRUE_MASK;

	dirsend(BRD, TCU, "2001", 0, 0); sleep(6);	/* SEND PLAY COMMAND	*/
	dirsend(BRD, TCU, "2000", 0, 0); sleep(2);	/* SEND STOP COMMAND	*/

	return(0);
}
