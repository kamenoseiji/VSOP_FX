/*********************************************************
**	TCU_SLAVE_CNTL.C : Tape Synchronization Using TCU	** 
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
#define	RETRY_OFS	40
#define	RESYNC_OFS	10
#define	LOG_FMT		"%03d%02d%02d%02d/PB%02d/ %s\n"
#define	ST_INDEX	fxsync_ptr->curr_scan[pb_id]
#define	TAPE_INDEX	fxsync_ptr->curr_tape[pb_id]
/*
---------------------------------------------------- LOAD PARAMETERS
*/
main(argc, argv)
	long	argc;			/* Number of Arguments	*/
	char	**argv;			/* Pointer of Arguments	*/
{
	/*-------- LOG FILE --------*/
	FILE	*log_ptr;		/* Log File Pointer		*/
	char	log_fname[16];	/* Log File Name		*/

	/*-------- SHARED MEMORY FOR CURRENT STATUS --------*/
	key_t	fxsync_key;				/* Keyword for Shared Memory		*/
	key_t	fxsched_key;			/* Keyword for Shared Memory		*/
	int		shrd_fxsync_id;			/* Shared Memory ID					*/
	int		shrd_fxsched_id;		/* Shared Memory ID					*/
	struct shared_mem1 *fxsync_ptr;	/* Pointer of Shared Memory			*/
	struct shared_mem2 *fxsched_ptr;/* Pointer of Shared Memory			*/

	/*-------- IDINTIFIER --------*/
	Addr4882_t	tcu_addr;		/* GP-IB Address of Master TCU				*/
	int		board;			/* Device I/O of GP-IB Board				*/
	int		TCU;			/* Device I/O of TCU						*/
	int		pb_id;			/* Plaback ID								*/
	int		bin_id;			/* Source/Destination Bin ID				*/
	unsigned int soy_fxstart;/* Second of Year at FX-START				*/
	int		doy_fxstart;	/* Day of Year at Observation				*/
	int		hh_fxstart;		/* Hour at Observation						*/
	int		mm_fxstart;		/* Minute at Observation					*/
	int		ss_fxstart;		/* Second at Observation					*/
	unsigned int play_soy;	/* Current TCU SOY							*/
	unsigned int tcu_soy;	/* Current TCU SOY							*/
	unsigned int tcu_tss;	/* Current TCU TSS							*/
	unsigned int tcu_frc;	/* Current TCU Fraction						*/
	unsigned int retry_tss;	/* TSSID to RESYNC							*/
	int		tape_home;		/* Home bin of the currently inserted tape	*/
	int		tape_dest;		/* Destination of the inserted tape			*/

#ifdef HIDOI
	int		st_index;		/* Index of /ST/							*/
	int		tape_index;		/* Index of /LABEL/							*/
#endif

	int		loop_count;		/* Counter for Loop							*/
	int		pb_index;		/* Index for PB in Cart						*/

	/*-------- TIMING-RELATED VARIABLES --------*/
	int		time_base;		/* Time Base Clock [MHz]					*/
	int		obs_rate;		/* Recording data rate at Observation [MHz] */
	int		play_rate;		/* Playback data rate [MHz]					*/
	int		st_tss;			/* Start TSS ID								*/
	int		st_idr;			/* Start IDR								*/
	int		et_idr;			/* Stop IDR									*/
	int		clk_diff;		/* SYNC Clock Difference [CLK]				*/
	int		tss_diff;		/* Given TSS Offset [TSS]					*/
	int		doy, hh, mm, ss;/* Doy, Hour, Min, Sec						*/
	unsigned int et_soy;	/* SOY at /ET/								*/

	/*-------- STATUS  --------*/
	int		istat;			/* DIR-1000 TTP-SENSE Status				*/
	int		current_idr;	/* Current IDR of the DIR-1000				*/
	int		target_idr;		/* Desitination IDR of the DIR-1000			*/
	int		end_flag;		/* Status Code for ENDING					*/
	char	dir_stat[32];	/* DIR-1000 TTP-SENSE Status				*/
	char	idr_char[8];	/* IDR expression in DIR-1000 format		*/
	char	log_item[256];	/* Logging Data								*/

	/*-------- COMMANDS  --------*/
	char	cmd[128];		/* Child Process Command					*/

/*
---------------------------------------------------- INPUT ARGUMENT 
*/
	if(argc < 2){
		printf("USAGE : tcu_slave_cntl [PB-ID] !!\n");
		printf("  PB_ID----- Plaback ID (1 - 10).\n");
		exit(0);
	}
	pb_id		= atoi(argv[1]);
/*
---------------------------------------------------- OPEN LOG FILEY 
*/
	sprintf( log_fname, "PB%02d.sync.log", pb_id );
	log_ptr = fopen( log_fname, "w" );
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

	/*-------- ACCESS TO CURRENT STATUS --------*/
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
#ifdef HIDOI
	fxsync_ptr->work_enable |= TRUE_MASK;		/* ENABLE  */
	fxsync_ptr->insert_enable |= TRUE_MASK;		/* ENABLE  */
	fxsync_ptr->rewind_enable &= FALSE_MASK;	/* DISABLE */
	fxsync_ptr->search_enable &= FALSE_MASK;	/* DISABLE */
	fxsync_ptr->utcset_enable	&= FALSE_MASK;	/* DISABLE */
	fxsync_ptr->eject_enable	&= FALSE_MASK;	/* DISABLE */
#endif
/*
---------------------------------------------------- INITIAL SETTING for TCU
*/
	tcu_addr	= (Addr4882_t)fxsched_ptr->tcu_addr[pb_id];
	board		= fxsched_ptr->tcu_board[pb_id];
	time_base	= fxsync_ptr->master_clk / 8;
	play_rate	= fxsched_ptr->play_rate[pb_id][0];
	obs_rate	= fxsched_ptr->rec_rate[pb_id][0];
	pb_index	= (fxsched_ptr->pos[pb_id]) % 2;
	soy_fxstart	= fxsync_ptr->start_soy;

	SendIFC( board );
	if(ibsta & ERR){	printf("SendIFC Error \n");	exit(1);	}
	tcu_init(board, tcu_addr, time_base, play_rate/8);
/*
---------------------------------------------------- CONTROL START
	st_index = fxsync_ptr->curr_scan[pb_id];
	tape_index = fxsync_ptr->curr_tape[pb_id];
*/
	end_flag = 0;
	tcu_soy = soy_fxstart;
	play_soy= soy_fxstart;
	while(1){

		/*-------- CHECK END OF PROCESS --------*/
		if(fxsync_ptr->validity >= FINISH){
			store_tape(fxsync_ptr, fxsched_ptr, board, tcu_addr, pb_id, pb_index );
			break;
		}

		/*-------- RECEIVE UTC-SET PULSE --------*/
		if(((fxsync_ptr->utcrst_active & (ACTIVE + RESERVED)) != 0) &&
			((fxsync_ptr->utcset_enable & TRUE_MASK) != 0) ){

			tcu_utcset(board, tcu_addr, fxsync_ptr->utcrst_soy );
			sleep(TSSSET_OFS + 1);
			fxsync_ptr->utcset_enable  &= FALSE_MASK;
			fxsync_ptr->utcset_done  |= TRUE_MASK;

			tcu_tssset(board, tcu_addr, play_rate/8,
						fxsync_ptr->utcrst_soy + TSSSET_OFS + 2,
						((fxsync_ptr->polarity & TRUE_MASK)>> pb_id) );
			sleep(2);
			fxsync_ptr->tlrun_enable |= TRUE_MASK;
		}

		/*-------- TAPE INSERT REQUEST --------*/
		if( (fxsync_ptr->insert_enable & TRUE_MASK) != 0 ){
			fxsync_ptr->insert_enable &= FALSE_MASK;

			sprintf( log_item, "TAPE INSERT REQUEST !!" );
			soy2dhms( fxsync_ptr->master_soy, &doy, &hh, &mm, &ss );
			fprintf( log_ptr, LOG_FMT, doy, hh, mm, ss, pb_id, log_item  ); 

			loop_count = 0;
			while( 1 ){
				loop_count = tape_load_request( fxsync_ptr, fxsched_ptr, pb_id, TAPE_INDEX );
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

		/*-------- TAPE SEARCH --------*/
		if( (fxsync_ptr->search_enable & TRUE_MASK) != 0 ){
			fxsync_ptr->search_enable &= FALSE_MASK;

			loop_count = tape_search(fxsync_ptr, fxsched_ptr, board, tcu_addr,
								pb_id, ST_INDEX, log_ptr);
			if(loop_count == -1){
				store_tape(fxsync_ptr, fxsched_ptr, board, tcu_addr, pb_id, pb_index ); }

			if( loop_count == 1 ){
				if( (fxsync_ptr->utcset_done & TRUE_MASK) == 0){
					/*-------- UTCSET is not yet Done --------*/
					fxsync_ptr->utcset_enable |= TRUE_MASK;
				} else {
					/*-------- UTCSET is already Done --------*/
					fxsync_ptr->utcset_enable &= FALSE_MASK;
					fxsync_ptr->tlrun_enable |= TRUE_MASK;
				}

			} else {	/*-------- TRY AGAIN --------*/

				/*-------- ADD 1998 5/1 --------*/
				soy2tss( RESYNC_OFS, play_rate/8, &retry_tss, &tcu_frc );
				retry_tss -= (retry_tss % 4);	/* Allow 1/4 speed down */ 
				fxsync_ptr->tlrun_done &= FALSE_MASK;
				fxsched_ptr->st_soy[pb_id][ST_INDEX] += RESYNC_OFS;
				fxsched_ptr->st_idr[pb_id][ST_INDEX] += retry_tss;
				fxsched_ptr->master_tssepc[pb_id][ST_INDEX] += retry_tss;
				/*-------- ADD 2000 3/17 --------*/
				if( fxsched_ptr->st_soy[pb_id][ST_INDEX] > (fxsched_ptr->et_soy[pb_id][ST_INDEX] - RESYNC_OFS)){
					ST_INDEX ++;
				}

				/*-------- ADD 1998 5/1 --------*/

				fxsync_ptr->search_enable |= TRUE_MASK;
				fxsync_ptr->tlrun_enable &= FALSE_MASK;
			}
		}

		/*-------- Timeline Play --------*/
		if((fxsync_ptr->tlrun_enable & TRUE_MASK) != 0 ){
			if(tlplay(fxsync_ptr, fxsched_ptr, board, tcu_addr, pb_id, ST_INDEX) == -1){
				store_tape(fxsync_ptr, fxsched_ptr, board, tcu_addr, pb_id, pb_index );
			}
			fxsync_ptr->tlrun_enable &= FALSE_MASK;
			sleep(16);
			fxsync_ptr->tlrun_done |= TRUE_MASK;
			et_idr = fxsched_ptr->et_idr[pb_id][ST_INDEX];
		}

		/*-------- CHECK CURRENT TCU TIME --------*/
		if( ((fxsync_ptr->utcset_done & TRUE_MASK) != 0 ) &&
			((fxsync_ptr->master_soy % CHECK_INTVAL) == 5 )){
			get_TCU_utc(board, tcu_addr, &tcu_soy);
			fsleep(800);
		}

		/*-------- Check SYNC --------*/
		if( ((fxsync_ptr->tlrun_done & TRUE_MASK) != 0 ) &&
			((fxsync_ptr->master_soy % CHECK_INTVAL) == 0 ) &&
			( tcu_soy > fxsched_ptr->st_soy[pb_id][ST_INDEX]) &&
			( tcu_soy < fxsched_ptr->et_soy[pb_id][ST_INDEX]) ){

			/*-------- SYNC between UTC and TSS --------*/
			if( check_utc_tss(board, tcu_addr, play_rate/8,
								&tcu_soy, &tcu_tss, &clk_diff) == 1){
#ifdef DEBUG
				printf("PB[%d]: SYNC DIFF = %d CLOCK at SOY= %d\n",
						pb_id, clk_diff, tcu_soy);
#endif
			}

			tcu_soy += 2;
			/*-------- SYNC between UTC and TSS --------*/
			soy2tss( tcu_soy, play_rate/8, &tcu_tss, &tcu_frc );
			tss_diff = fxsched_ptr->master_tssepc[pb_id][ST_INDEX]
					-  fxsched_ptr->st_idr[pb_id][ST_INDEX];

			/*-------- SUCCEEDED TO SENSE SYNC or ASYNC --------*/
			if(check_tss_idr(pb_id, board, tcu_addr, tcu_tss, &clk_diff) == 1){
#ifdef DEBUG
				printf("PB[%d] TSS -IDR = %d\n", pb_id,
					(clk_diff - tss_diff)&TSS_MASK );
#endif
				/*-------- TSS/IDR ASINC -> TRY RESYNC --------*/
				if( (((clk_diff - tss_diff)&TSS_MASK) != 0) &&
					(tcu_soy + RETRY_OFS
					< fxsched_ptr->et_soy[pb_id][ST_INDEX])){

					/*-------- Release SYNC Status --------*/
					fxsync_ptr->tape_sync &= FALSE_MASK;
				}
			}

			/*-------- Is ASYNC TRUE?-> confirm --------*/
			if( (fxsync_ptr->tape_sync & TRUE_MASK) == 0){

				loop_count = 0;

				/*-------- Is Tape Running? --------*/
				while( loop_count < 2 ){
					dirsend( board, tcu_addr, "3001", 0, 0);		/* SEND TTP-SENSE COMMAND	*/
					Receive( board, tcu_addr, dir_stat, 20, STOPend);
					sscanf(dir_stat, "%8X", &istat);

					/*-------- IF RUNNING, CHECK AGAIN --------*/
					if( (istat & FWD_STAT) == FWD_STAT){

						/*-------- RUNNING -> READ UTC and TSS --------*/
						check_utc_tss(board, tcu_addr, play_rate/8, &tcu_soy, &tcu_tss, &clk_diff);
						tcu_soy += 2;
						soy2tss( tcu_soy, play_rate/8, &tcu_tss, &tcu_frc );
						tss_diff = fxsched_ptr->master_tssepc[pb_id][ST_INDEX]
								-  fxsched_ptr->st_idr[pb_id][ST_INDEX];

						if(check_tss_idr(pb_id, board, tcu_addr, tcu_tss, &clk_diff) == 1){

							/*-------- SYNC CHECK is AVAILABLE -------*/
							if( (((clk_diff - tss_diff)&TSS_MASK) != 0) && (tcu_soy + RETRY_OFS
									< fxsched_ptr->et_soy[pb_id][ST_INDEX])){

								loop_count ++;			/*-------- ASYNC, indeed --------*/
								fxsync_ptr->tape_sync &= FALSE_MASK;

							} else {
															/*-------- SYNC !! --------*/
								fxsync_ptr->tape_sync |= TRUE_MASK;
								break;
							}

						} else {
							/*-------- UNCERTAIN -> CHECK AGAIN! --------*/
							sleep(1);
						}
					} else {
						/*-------- TAPE IN NOT RUNNING -> SEARCH for RESYNC --------*/
						fxsync_ptr->tape_sync &= FALSE_MASK;
						break;
					}
				}
			}

			/*-------- TAPE ASYNC is CONFIRMED --------*/
			if( (fxsync_ptr->tape_sync & TRUE_MASK) == 0){

				dirsend( board, tcu_addr, "2000", 0, 0);	/* TAPE STOP COMMAND	*/

				/*-------- Warning Sound --------*/
				sprintf( log_item, "TSS/IDR SYNC ERROR !! TSSID=%d  IDR=%d ... TRY TO RESYNC", tcu_tss, tcu_tss - clk_diff);
				system(ERROR_SOUND);

				/*-------- Logging the SYNC Error --------*/
				soy2dhms(fxsync_ptr->master_soy, &doy, &hh, &mm, &ss );
				fprintf(log_ptr,LOG_FMT, doy, hh, mm, ss, pb_id, log_item); 

				/*-------- CALC RESYNC Point --------*/
				soy2tss( RETRY_OFS, play_rate/8, &retry_tss, &tcu_frc );
				retry_tss -= (retry_tss % 4);	/* Allow 1/4 speed down */ 
				fxsync_ptr->tlrun_done &= FALSE_MASK;
				fxsched_ptr->st_soy[pb_id][ST_INDEX] = tcu_soy + RETRY_OFS;
				fxsched_ptr->st_idr[pb_id][ST_INDEX]
					= tcu_tss - tss_diff + retry_tss;
				fxsched_ptr->master_tssepc[pb_id][ST_INDEX]
					= tcu_tss + retry_tss;
				fxsync_ptr->search_enable |= TRUE_MASK;
				fxsync_ptr->search_done &= FALSE_MASK;


			} else if(((clk_diff - tss_diff)&TSS_MASK) == 0){
				fxsync_ptr->tape_sync |= TRUE_MASK;
			} else {
				end_flag = sched_shift( fxsync_ptr, fxsched_ptr, log_ptr,
				board, tcu_addr, pb_id, &ST_INDEX, &TAPE_INDEX, pb_index, tcu_soy);
				if( end_flag == FINISH){
					fxsync_ptr->work_enable &= FALSE_MASK;
					fclose( log_ptr );
					if(fxsync_ptr->work_enable == 0){
						fxsync_ptr->validity = FINISH;
						system(END_SOUND);
					}
					exit(0);
				}
			}
		}

		/*-------- CHECK ET TIMING --------*/
		if(tcu_soy > fxsched_ptr->et_soy[pb_id][ST_INDEX]){
			end_flag = sched_shift( fxsync_ptr, fxsched_ptr, log_ptr, board,
				tcu_addr, pb_id, &ST_INDEX, &TAPE_INDEX, pb_index, tcu_soy);
			if( end_flag == FINISH){
				fxsync_ptr->work_enable &= FALSE_MASK;
				fclose( log_ptr );
				if(fxsync_ptr->work_enable == 0){
					fxsync_ptr->validity = FINISH;
					system(END_SOUND);
				}
				exit(0);
			}
		}

		fsleep(LOOP_WAIT);
	}
	fclose( log_ptr );
}
