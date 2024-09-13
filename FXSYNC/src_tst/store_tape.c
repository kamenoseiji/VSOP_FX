/*********************************************************
**	STORE_TAPE.C : Tape Synchronization Using TCU		** 
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
/*
---------------------------------------------------- FINISHING PROCESS 
*/
int	store_tape(fxsync_ptr, fxsched_ptr, BRD, TCU, pb_id, pb_index )
	struct shared_mem1 *fxsync_ptr;  /* Pointer of Shared Memory			*/
	struct shared_mem2 *fxsched_ptr; /* Pointer of Shared Memory			*/
	int			BRD;				/* GP-IB Board ID					*/
	Addr4882_t	TCU;				/* GP-IB Address of TCU				*/
	int			pb_id;				/* Plaback ID						*/
	int			pb_index;			/* Index of PB in the Cart			*/
{
	int		dms_index;

	dms_index = fxsched_ptr->dms_id[pb_id] - 1;

	dirsend(BRD, TCU, "2000", 0, 0); sleep(1);		/* SEND STOP COMMAND	*/
	dirsend(BRD, TCU, "200F", 0, 0); sleep(5);		/* SEND EJECT COMMAND	*/
	fxsync_ptr->insert_done		&= FALSE_MASK;
	fxsync_ptr->rewind_enable	&= FALSE_MASK;
	fxsync_ptr->rewind_done		&= FALSE_MASK;
	fxsync_ptr->search_enable	&= FALSE_MASK;
	fxsync_ptr->search_done		&= FALSE_MASK;
	fxsync_ptr->tlrun_enable	&= FALSE_MASK;
	fxsync_ptr->tlrun_done		&= FALSE_MASK;
	fxsync_ptr->tape_sync 		&= FALSE_MASK;
	while(1){
		if( fxsync_ptr->cart_event[dms_index][pb_index] == ENABLE ){
			fxsync_ptr->load_pb_id[dms_index][pb_index] = pb_id;
			fxsync_ptr->cart_event[dms_index][pb_index] = RESERVED;
			fxsync_ptr->cart_source[dms_index][pb_index]
						= fxsync_ptr->tape_dest[pb_id];
			fxsync_ptr->cart_dest[dms_index][pb_index]
						= fxsync_ptr->tape_home[pb_id];
			break;
		}
		sleep(1);
	}
	return(0);
}
