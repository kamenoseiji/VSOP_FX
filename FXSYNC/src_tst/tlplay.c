/*********************************************************
**	TLPLAY.C : Tape Synchronization Using TCU			** 
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/11/1									**
**********************************************************/

#include "fxsync.inc"
#include <stdio.h>
#include <unistd.h>
#include <sys/ugpib.h>

#define	LOOP_WAIT	500
#define	WAIT		100
#define	TLRUN_OFS	400
#define	SYNC_OFS	100
#define	TSS_MASK	0x7fffff
#define	RESERVE_OFFSET	16*fxsched_ptr->play_rate[pb_id][st_index]	
/*
---------------------------------------------------- LOAD PARAMETERS
*/
int	tlplay( fxsync_ptr, fxsched_ptr, BRD, TCU, pb_id, st_index)
	struct shared_mem1 *fxsync_ptr;   /* Pointer of Shared Memory			*/
	struct shared_mem2 *fxsched_ptr;  /* Pointer of Shared Memory			*/
	int			BRD;		/* GP-IB Board ID							*/
	Addr4882_t	TCU;		/* Device I/O of TCU						*/
	int			pb_id;		/* Plaback ID								*/
	int			st_index;	/* Index of /ST/							*/
{
	/*-------- TIMING-RELATED VARIABLES --------*/
	int		st_tss;			/* Start TSS ID								*/
	int		st_idr;			/* Start IDR								*/
	int		et_idr;			/* Stop IDR									*/


	int		current_tss;	/* Current TSS ID of the TCU				*/
	int		current_frc;	/* Current Fraction of the TCU				*/
	char	idr_char[8];	/* IDR expression in DIR-1000 format		*/
	char	cmd[64];
	short	result;
/*
---------------------------------------------------- WAIT UNTILL ST EVENT
*/
	st_tss	= fxsched_ptr->master_tssepc[pb_id][st_index];
	st_idr	= fxsched_ptr->st_idr[pb_id][st_index];
	et_idr	= fxsched_ptr->et_idr[pb_id][st_index];

	while(1){
		if( get_TCU_tss(BRD, TCU, &current_tss, &current_frc) == 1){
			if( current_tss > st_tss - RESERVE_OFFSET ){	break;}
		}
		if(fxsync_ptr->validity >= FINISH){ return(0); }
		sleep(1);
	}
	printf("PB%d : TLRUN COMMAND ... [IDR = %d - %d] at %d\n",
				pb_id, st_idr, et_idr, st_tss);
/*
---------------------------------------------------- START PLAY
*/
	/*-------- TimeLine Stop --------*/
	dirsend(BRD, TCU, "1010", 0, 0);				/* Timeline Stop */

	/*-------- DRUM ACTIVE --------*/
	dirsend(BRD, TCU, "2004", 0, 0);				/* Stand-by On */

	/*-------- SET TIMELINE COUNTER --------*/
	tss2idr( (st_idr - TLRUN_OFS) & TSS_MASK, idr_char);
	sprintf(cmd, "181203%06s", idr_char);	/* Set Timeline Counter */
	dirsend(BRD, TCU, cmd, 0, 0);

	/*-------- Event Clear --------*/
	dirsend(BRD, TCU, "181901FF", 0, 0);			/* Event Clear */

	/*-------- Define SYNC Event --------*/
	tss2idr( (st_idr - SYNC_OFS) & TSS_MASK, idr_char );
	sprintf(cmd, "18180A00%06s284003%06s", idr_char, idr_char);
	dirsend(BRD, TCU, cmd, 0, 0);				/* 1-st Event is SYNC */

	/*-------- Define PLAY Event --------*/
	tss2idr( st_idr, idr_char );
	sprintf(cmd, "18180601%06s2001", idr_char);
	dirsend(BRD, TCU, cmd, 0, 0);				/* 2-nd Event is PLAY */

	/*-------- Define STOP Event --------*/
	tss2idr( et_idr, idr_char );
	sprintf(cmd, "18180602%06s2000", idr_char);
	dirsend(BRD, TCU, cmd, 0, 0);				/* 3-rd Event is STOP */

	/*-------- PREROLL --------*/
	dirsend(BRD,  TCU, "288F023001", 0, 0);
	fsleep(WAIT);
	dirsend(BRD,  TCU, "2030", 0, 0);

	/*-------- Reserve TimeLine Run Command --------*/
	sprintf(cmd, "TSSACT=%07d\r", (st_tss - TLRUN_OFS) & 0x7fffff );
	Send(BRD, TCU, cmd, strlen(cmd), NLend); fsleep(WAIT);
	ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);

	sprintf(cmd, "ACTION=TLNRUN\r");
	Send(BRD, TCU, cmd, strlen(cmd), NLend); fsleep(WAIT);
	ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);

	return(0);
}
