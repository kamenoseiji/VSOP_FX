/*********************************************************
**	TAPE_LOAD_REQUEST.C : Tape Synchronization Using TCU** 
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/11/1									**
**********************************************************/

#include "fxsync.inc"
#include <stdio.h>
#include <unistd.h>
#define TRUE_MASK	(1 << pb_id)
#define FALSE_MASK	(~(1 << pb_id))
#define	MAX_LOOP	20
/*
---------------------------------------------------- LOAD PARAMETERS
*/
int	tape_load_request(fxsync_ptr, fxsched_ptr, pb_id, tape_index )

	struct shared_mem1 *fxsync_ptr;  /* Pointer of Shared Memory			*/
	struct shared_mem2 *fxsched_ptr;  /* Pointer of Shared Memory			*/
	int		pb_id;			/* Plaback ID								*/
	int		tape_index;		/* Index of /ST/							*/
{
	/*-------- IDINTIFIER --------*/
	int		dms_id;			/* DMS-16/24 ID (1 - 5)						*/
	int		dms_index;		/* DMS-16/24 Index (0 - 4)					*/
	int		bin_id;			/* Source/Destination Bin ID				*/
	int		loop_count;		/* Loop Counter								*/
	int		pb_index;		/* PB Index in Cart							*/
/*
---------------------------------------------------- INITIAL TAPE STORE
*/
	dms_id	= fxsched_ptr->dms_id[pb_id];
	dms_index = dms_id - 1;
	pb_index = (fxsched_ptr->pos[pb_id]) % 2;

	/*-------- SCAN DMS BIN and FIND THE TARGET TAPE --------*/
	scan_bindat( fxsched_ptr, dms_index,
		fxsched_ptr->tape_label[pb_id][tape_index], &bin_id );
	if(bin_id == -1){
		printf("tcu_slave_cntl: TAPE LABEL %s was not found in CART %d!!\n",
			fxsched_ptr->tape_label[pb_id][tape_index], dms_id );
		system(WARN_SOUND);
		return(MAX_LOOP);
	}

	/*-------- REQUEST TAPE LOADING to DMS --------*/
	loop_count = 0;
	while(1){
		if( fxsync_ptr->cart_event[dms_index][pb_index] == ENABLE ){
			fxsync_ptr->load_pb_id[dms_index][pb_index] = pb_id;
			fxsync_ptr->cart_event[dms_index][pb_index] = RESERVED;
			fxsync_ptr->cart_source[dms_index][pb_index] = bin_id;
			fxsync_ptr->cart_dest[dms_index][pb_index] = fxsched_ptr->pos[pb_id];

			fxsync_ptr->tape_home[pb_id] = bin_id;
			fxsync_ptr->tape_dest[pb_id] = fxsched_ptr->pos[pb_id];
			break;
		}
		if(loop_count > MAX_LOOP){
			printf("CART %d does not accept Tape Insert Order !!\n", dms_id);
			system(WARN_SOUND);
			return(loop_count);
		}

		if(fxsync_ptr->validity >= FINISH){ return(-1); }
		loop_count ++;
		sleep(1);
	}

	sleep(10);
	/*-------- WAIT UNTIL TAPE LOADING IS OVER --------*/
	loop_count = 0;
	while( (fxsync_ptr->insert_done & TRUE_MASK) == 0 ){

		if(fxsync_ptr->validity >= FINISH){ return(-1); }

		if(loop_count > MAX_LOOP){
			printf("CART %d does not finish Tape Insert !!\n", dms_id);
			return(loop_count);
		}
		sleep(1);
		loop_count ++;
	}

	return(1);
}
