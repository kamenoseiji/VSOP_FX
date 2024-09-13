/*********************************************************
**	SCHED_SHIFT.C : Tape Synchronization Using TCU		** 
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/11/1									**
**********************************************************/

#include "fxsync.inc"
#include <stdio.h>
#include <unistd.h>
#include <sys/ugpib.h>

#define	TRUE_MASK	(1 << pb_id)
#define	FALSE_MASK	(~(1 << pb_id))
#define	LOG_FMT		"%03d%02d%02d%02d/PB%02d/ %s\n"
/*
---------------------------------------------------- LOAD PARAMETERS
*/
int	sched_shift(fxsync_ptr, fxsched_ptr, log_ptr, board, tcu_addr, pb_id,
				st_index_ptr, tape_index_ptr, pb_index, tcu_soy)
	struct shared_mem1 *fxsync_ptr;  /* Pointer of Shared Memory			*/
	struct shared_mem2 *fxsched_ptr;  /* Pointer of Shared Memory			*/
	FILE	*log_ptr;		/* Log File Pointer		*/
	int		board;			/* Device I/O of GP-IB Board				*/
	Addr4882_t	tcu_addr;	/* GP-IB Address of Master TCU				*/
	int		pb_id;			/* Plaback ID								*/
	int		*st_index_ptr;	/* Index of /ST/							*/
	int		*tape_index_ptr;/* Index of /LABEL/							*/
	int		pb_index;		/* Index for PB in Cart						*/
	unsigned int tcu_soy;	/* Current TCU SOY							*/
{
	char	log_item[256];	/* Logging Data								*/
	int		doy, hh, mm, ss;/* Doy, Hour, Min, Sec						*/
	int		prev_tapeid;	/* Previous Tape ID							*/
/*
---------------------------------------------------- CONTROL START
*/
	/*-------- FLAG OUT SYNC STATUS --------*/
	fxsync_ptr->tape_sync &= FALSE_MASK;

	/*-------- Go To Concurrent /ST/ --------*/
	prev_tapeid = fxsched_ptr->tape_id[pb_id][*st_index_ptr];
	(*st_index_ptr) ++;

	while( (tcu_soy > fxsched_ptr->et_soy[pb_id][*st_index_ptr])
		&& ( (*st_index_ptr) <= fxsched_ptr->st_num[pb_id]) ){
		(*st_index_ptr) ++;	
	}

	/*-------- Put /ET/ into Log File --------*/
	sprintf( log_item, "/ET/");
	soy2dhms( fxsync_ptr->master_soy, &doy, &hh, &mm, &ss );
	fprintf( log_ptr, LOG_FMT, doy, hh, mm, ss, pb_id, log_item  ); 

	/*-------- ALL SCHEDULE FINISHED ? --------*/
	if(*st_index_ptr >= fxsched_ptr->st_num[pb_id]){
		sprintf( log_item, "------End of File------");
		printf( "------All Schedule Finished------\n");
		soy2dhms( fxsync_ptr->master_soy, &doy, &hh, &mm, &ss );
		fprintf( log_ptr, LOG_FMT, doy, hh, mm, ss, pb_id, log_item  ); 
		store_tape(fxsync_ptr, fxsched_ptr, board,tcu_addr, pb_id, pb_index);
		fclose(log_ptr);
		return(FINISH);
	}

	/*-------- TAPE CHANGE ? --------*/
	if(fxsched_ptr->tape_id[pb_id][*st_index_ptr] != prev_tapeid ){
		(*tape_index_ptr) ++;
		sprintf( log_item, "-------- TAPE CHANGE --------");
		soy2dhms( fxsync_ptr->master_soy, &doy, &hh, &mm, &ss );
		fprintf( log_ptr, LOG_FMT, doy, hh, mm, ss, pb_id, log_item ); 
		store_tape(fxsync_ptr, fxsched_ptr, board, tcu_addr, pb_id, pb_index);
		fxsync_ptr->insert_enable |= TRUE_MASK;
		fxsync_ptr->insert_done	&= FALSE_MASK;
		fxsync_ptr->rewind_enable &= FALSE_MASK;
		fxsync_ptr->rewind_done	&= FALSE_MASK;
		fxsync_ptr->search_enable &= FALSE_MASK;
		fxsync_ptr->search_done	&= FALSE_MASK;
		sleep(10);
	} else {
		sprintf( log_item, "-------- SCAN CHANGE --------");
		soy2dhms( fxsync_ptr->master_soy, &doy, &hh, &mm, &ss );
		fprintf( log_ptr, LOG_FMT, doy, hh, mm, ss, pb_id, log_item ); 
		fxsync_ptr->search_enable |= TRUE_MASK;
		fxsync_ptr->search_done	  &= FALSE_MASK;
		fxsync_ptr->tlrun_enable  &= FALSE_MASK;
		fxsync_ptr->tlrun_done	  &= FALSE_MASK;
	}
	return(0);
}
