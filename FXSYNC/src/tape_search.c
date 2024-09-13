/*********************************************************
**	TAPE_SEARCH.C : Tape Search Using TCU				** 
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
#define	TSS_MASK	0x7fffff
#define LOG_FMT     "%03d%02d%02d%02d/PB%02d/ %s\n"
/*
---------------------------------------------------- LOAD PARAMETERS
*/
int	tape_search(fxsync_ptr, fxsched_ptr, BRD, TCU, pb_id, st_index, log_ptr)
	struct shared_mem1 *fxsync_ptr;   /* Pointer of Shared Memory		*/
	struct shared_mem2 *fxsched_ptr;  /* Pointer of Shared Memory		*/
	int			BRD;			/* GP-IB Board ID					*/
	Addr4882_t	TCU;			/* GP-IB Address of TCU				*/
	int			pb_id;			/* Plaback ID						*/
	int			st_index;		/* Index for /ST/					*/
	FILE		*log_ptr;		/* Log File Pointer					*/
{
	/*-------- STATUS  --------*/
	int		doy, hh, mm, ss;/* Doy, Hour, Min, Sec						*/
	int		istat;			/* DIR-1000 TTP-SENSE Status				*/
	int		current_idr;	/* Current IDR of the DIR-1000				*/
	int		target_idr;		/* Desitination IDR of the DIR-1000			*/
	char	dir_stat[32];	/* DIR-1000 TTP-SENSE Status				*/
	char	idr_char[8];	/* IDR expression in DIR-1000 format		*/
	char	cmd[64];		/* GPIB Command								*/
	short	result;			/* SRQ Status Byte							*/
	char	log_item[256];	/* Logging Data								*/
/*
---------------------------------------------------- TAPE SEARCH
*/
#ifdef DRUM_SAVING
#endif
	dirsend(BRD, TCU, "2003", 0, 0); sleep(2);	/* SEND DRUM-OFF COMMAND*/
	dirsend(BRD, TCU, "2001", 0, 0); sleep(6);	/* SEND PLAY COMMAND	*/
	while(1){
		dirsend(BRD, TCU, "3012", 0, 0);			/* IDR DATA SENSE	*/
		Receive(BRD, TCU, dir_stat, 12, STOPend);
		ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);
		if(strncmp( dir_stat, "3C12", 4) == 0){ break; }

		if(fxsync_ptr->validity == FINISH){ return(-1); }
		sleep(1);
	}
	sscanf( dir_stat, "%*6s%06s", idr_char);
	current_idr	= idr2tss( idr_char );
	target_idr	= fxsched_ptr->st_idr[pb_id][st_index] - 100;
	fxsync_ptr->current_idr[pb_id] = current_idr;

	soy2dhms(fxsync_ptr->master_soy, &doy, &hh, &mm, &ss );
	sprintf( log_item, "SEARCH START...Target=%07d  Current IDR=%07d\n",
		target_idr, current_idr);
	fprintf(log_ptr, LOG_FMT, doy, hh, mm, ss, pb_id, log_item);

	/*-------- IDR JUMP! --------*/
	if( (current_idr - target_idr) > TSS_MASK/2 ){ 
		dirsend(BRD, TCU, "28830120", 0, 0);			/* SET IDR SIGNED   */
	}
	tss2idr( target_idr, idr_char );
	sprintf(cmd, "283103%06s", idr_char);
	dirsend(BRD, TCU, cmd, 0, 0); sleep(2);

	/*-------- SEARCH FINISHED? --------*/
	while(1){
		sleep(2);
		dirsend(BRD, TCU, "3001", 0, 0);		/* SEND TTP-SENSE COMMAND	*/
		Receive(BRD, TCU, dir_stat, 20, STOPend);
		sscanf(dir_stat, "%8X", &istat);

		if( (istat & STANDBY_STAT) == STANDBY_STAT){ sleep(1); break;}

		if(fxsync_ptr->validity >= FINISH){ return(-1); }
	}
	dirsend(BRD, TCU, "28830100", 0, 0);			/* SET IDR UNSIGNED   */

	/*-------- CORRECT SEARCH POINT? --------*/
	while(1){
		dirsend(BRD, TCU, "3012", 0, 0);			/* IDR DATA SENSE	*/
		fsleep(200);
		Receive(BRD, TCU, dir_stat, 12, STOPend);
		ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);
		if(strncmp( dir_stat, "3C12", 4) == 0){ break; }

		if(fxsync_ptr->validity >= FINISH){ return(-1); }
		sleep(1);
	}
	sscanf( dir_stat, "%*6s%06s", idr_char);
	current_idr	= idr2tss( idr_char );
	if( labs(current_idr - target_idr) > 100 ){ 
		system(ERROR_SOUND);
		soy2dhms(fxsync_ptr->master_soy, &doy, &hh, &mm, &ss );
		sprintf( log_item, "Failed to Search...Target=%07d  Current IDR=%07d\n",
			target_idr, current_idr);
/*
		fprintf(log_ptr, LOG_FMT, doy, hh, mm, ss, pb_id, log_item);
*/
		printf("PB[%d] %s\n", pb_id, log_item );

		return(0);
	} else {
		fxsync_ptr->search_done |= TRUE_MASK;
		fxsync_ptr->utcset_enable |= TRUE_MASK;
		soy2dhms(fxsync_ptr->master_soy, &doy, &hh, &mm, &ss );
		sprintf( log_item, "Search Success...Target=%07d  Current IDR=%07d\n",
			target_idr, current_idr);
/*
		fprintf(log_ptr, LOG_FMT, doy, hh, mm, ss, pb_id, log_item);
*/
	}

	return(1);
}
