/*********************************************************
**	TCU_REWIND_CNTL.C : Rewind All Tapes Using TCU		** 
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/11/1									**
**********************************************************/

#include "fxsync.inc"
#include <stdio.h>
#include <unistd.h>
#include <sys/ugpib.h>

#define	TSSSET_OFS	5			/* SLAVE CLOCK RESET before FXSTART		*/
#define	LOOP_WAIT	500
#define	WAIT		100
#define	TLRUN_OFS	400
#define	SYNC_OFS	100
#define	CHECK_INTVAL	10
#define	TSS_MASK	0x7fffff
#define	TRUE_MASK	(1 << pb_id)
#define	FALSE_MASK	(~(1 << pb_id))
#define	RETRY_OFS	60
#define	RESYNC_OFS	10
#define	LOG_FMT		"%03d%02d%02d%02d/PB%02d/ %s\n"
/*
---------------------------------------------------- LOAD PARAMETERS
*/
main(argc, argv)
	long	argc;			/* Number of Arguments	*/
	char	**argv;			/* Pointer of Arguments	*/
{
	/*-------- SHARED MEMORY FOR CURRENT STATUS --------*/
	key_t	fxsync_key;				/* Keyword for Shared Memory		*/
	key_t	fxsched_key;			/* Keyword for Shared Memory		*/
	int		shrd_fxsync_id;			/* Shared Memory ID					*/
	int		shrd_fxsched_id;		/* Shared Memory ID					*/
	struct shared_mem1 *fxsync_ptr;	/* Pointer of Shared Memory			*/
	struct shared_mem2 *fxsched_ptr;/* Pointer of Shared Memory			*/

	/*-------- IDINTIFIER --------*/
	Addr4882_t	tcu_addr;	/* GP-IB Address of Master TCU				*/
	int		board;			/* Device I/O of GP-IB Board				*/
	int		TCU;			/* Device I/O of TCU						*/
	int		pb_id;			/* Plaback ID								*/
	int		bin_id;			/* Source/Destination Bin ID				*/
	int		tape_home;		/* Home bin of the currently inserted tape	*/
	int		tape_dest;		/* Destination of the inserted tape			*/
	int		st_index;		/* Index of /ST/							*/
	int		tape_index;		/* Index of /LABEL/							*/
	int		loop_count;		/* Counter for Loop							*/
	int		pb_index;		/* Index for PB in Cart						*/
	int		prev_tapeid;	/* Previous Tape ID							*/

	/*-------- TIMING-RELATED VARIABLES --------*/
	int		time_base;		/* Time Base of Master TCU [MHz] 			*/
	int		obs_rate;		/* Recording data rate at Observation [MHz] */
	int		play_rate;		/* Playback data rate [MHz]					*/

	/*-------- STATUS  --------*/
	int		end_flag;		/* Status Code for ENDING					*/

/*
---------------------------------------------------- INPUT ARGUMENT 
*/
	if(argc < 2){
		printf("USAGE : tcu_rewind_cntl [PB-ID] !!\n");
		printf("  PB_ID----- Plaback ID (1 - 10).\n");
		exit(0);
	}
	pb_id		= atoi(argv[1]);
/*
---------------------------------------------------- ACCESS TO SHARED MEMORY 
*/
	/*-------- ACCESS TO CURRENT STATUS --------*/
	fxsync_key  = FXSYNC_KEY1;
	shrd_fxsync_id = shmget(fxsync_key, sizeof(struct shared_mem1), 0444);
	if( shrd_fxsync_id  < 0 ){
	printf("tcu_slave_cntl: Can't Access to the Shared Memory : %s !!\n",
		argv[0]);
		printf("tcu_slave_cntl: RUN fxsync_shm at first.\n");
		exit(1);
	}
	fxsync_ptr  = (struct shared_mem1 *)shmat(shrd_fxsync_id, NULL, 0);

	/*-------- ACCESS TO PB SCHEDULE --------*/
	fxsched_key  = FXSYNC_KEY2;
	shrd_fxsched_id = shmget(fxsched_key, sizeof(struct shared_mem2), 0444);
	if( shrd_fxsched_id  < 0 ){
	printf("tcu_slave_cntl: Can't Access to the Shared Memory : %s !!\n",
		argv[0]);
		printf("tcu_slave_cntl: RUN fxsync_shm at first.\n");
		exit(1);
	}
	fxsched_ptr  = (struct shared_mem2 *)shmat(shrd_fxsched_id, NULL, 0);
/*
---------------------------------------------------- INITIAL STATUS SET
*/
	fxsync_ptr->work_enable |= TRUE_MASK;		/* ENABLE  */
	fxsync_ptr->insert_enable |= TRUE_MASK;		/* ENABLE  */
	fxsync_ptr->rewind_enable &= FALSE_MASK;	/* DISABLE */
	fxsync_ptr->search_enable &= FALSE_MASK;	/* DISABLE */
	fxsync_ptr->utcset_enable	&= FALSE_MASK;	/* DISABLE */
	fxsync_ptr->eject_enable	&= FALSE_MASK;	/* DISABLE */
/*
---------------------------------------------------- INITIAL SETTING for TCU
*/
	tcu_addr	= (Addr4882_t)fxsched_ptr->tcu_addr[pb_id];
	board		= fxsched_ptr->tcu_board[pb_id];
	time_base	= fxsync_ptr->master_clk / 8;
	play_rate	= fxsched_ptr->play_rate[pb_id][0];
	obs_rate	= fxsched_ptr->rec_rate[pb_id][0];
	pb_index	= (fxsched_ptr->pos[pb_id]) % 2;

	SendIFC( board );
	if(ibsta & ERR){	printf("SendIFC Error \n");	exit(1);	}
	tcu_init(board, tcu_addr, time_base, play_rate/8);
/*
---------------------------------------------------- CONTROL START
*/
	st_index = 0;
	tape_index = 0;
	end_flag = 0;
	prev_tapeid = fxsched_ptr->tape_id[pb_id][st_index];
	while(1){

		/*-------- CHECK END OF PROCESS --------*/
		fxsync_ptr->curr_scan[pb_id] = st_index;
		fxsync_ptr->curr_tape[pb_id] = tape_index;
		if(fxsync_ptr->validity >= FINISH){
			store_tape(fxsync_ptr, fxsched_ptr, board, tcu_addr,  pb_id, pb_index );
			break;
		}

		/*-------- TAPE INSERT REQUEST --------*/
		if( (fxsync_ptr->insert_enable & TRUE_MASK) != 0 ){
			fxsync_ptr->insert_enable &= FALSE_MASK;

			loop_count = 0;
			while( 1 ){
				loop_count = tape_load_request( fxsync_ptr, fxsched_ptr, pb_id, tape_index );
				if(loop_count == 1){	break;}
				if( loop_count == -1){
					store_tape(fxsync_ptr, fxsched_ptr, board, tcu_addr, pb_id, pb_index );
				}
			}
			fxsync_ptr->rewind_enable |= TRUE_MASK;
			fxsync_ptr->eject_enable  |= TRUE_MASK;
		}

		/*-------- TAPE REWIND --------*/
		if( (fxsync_ptr->rewind_enable & TRUE_MASK) != 0 ){
			fxsync_ptr->rewind_enable &= FALSE_MASK;
			if( tape_rewind( fxsync_ptr, board, tcu_addr, pb_id) == -1){
				store_tape(fxsync_ptr, fxsched_ptr, board, tcu_addr, pb_id, pb_index );
			}
			fxsync_ptr->search_enable |= TRUE_MASK;
		}

		/*-------- END REWIND -> STORE --------*/
		if( (fxsync_ptr->search_enable & TRUE_MASK) != 0 ){
			store_tape(fxsync_ptr,fxsched_ptr,board,tcu_addr,pb_id, pb_index);
			tape_index ++;
			fxsync_ptr->insert_enable |= TRUE_MASK;
			fxsync_ptr->insert_done &= FALSE_MASK;
			fxsync_ptr->rewind_enable &= FALSE_MASK;
			fxsync_ptr->rewind_done &= FALSE_MASK;
			fxsync_ptr->search_enable &= FALSE_MASK;
			fxsync_ptr->search_done &= FALSE_MASK;
			sleep(12);
		}

		/*-------- END OF SCHEDULE ? --------*/
		if(tape_index >= fxsched_ptr->tape_num[pb_id]){
			printf( "------All tapes were rewound ------\n");
			end_flag = FINISH;
		}

		/*-------- CHECK ET TIMING --------*/
		if( end_flag == FINISH){
			fxsync_ptr->work_enable &= FALSE_MASK;
			if(fxsync_ptr->work_enable == 0){
				fxsync_ptr->validity = FINISH;
				system(REWEND_SOUND);
			}
			exit(0);
		}
		fsleep(LOOP_WAIT);
	}
}
