/*********************************************************
**	TCU_MASTER_CNTL.C : Tape Synchronization Using TCU	** 
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/11/1									**
**********************************************************/

#include "fxsync.inc"
#include <stdio.h>
#include <unistd.h>
#include <sys/ugpib.h>

#define	UTCSET_OFS	40			/* MASTER CLOCK RESET before FXSTART	*/
#define	UTCSET_OFS2	30			/* SLAVE CLOCK RESET before FXSTART		*/
#define	UTCSET_OFS3	20			/* SLAVE CLOCK RESET before FXSTART		*/
#define	LOOP_WAIT	100
/*
---------------------------------------------------- LOAD PARAMETERS
*/
main(argc, argv)
	long	argc;			/* Number of Arguments	*/
	char	**argv;			/* Pointer of Arguments	*/
{
	/*-------- SHARED MEMORY FOR CURRENT STATUS --------*/
	key_t	fxsync_key;					/* Keyword for Shared Memory		*/
	key_t	fxsched_key;				/* Keyword for Shared Memory		*/
	int		shrd_fxsync_id;				/* Shared Memory ID					*/
	int		shrd_fxsched_id;			/* Shared Memory ID					*/
	struct shared_mem1 *fxsync_ptr;		/* Pointer of Shared Memory			*/
	struct shared_mem2 *fxsched_ptr;	/* Pointer of Shared Memory			*/

	/*-------- IDENTIFIER --------*/
	Addr4882_t		tcu_addr;		/* GP-IB Address of Master TCU		*/
	int		board;			/* Device I/O of GP-IB Board				*/
	int		TCU;			/* Device I/O of TCU						*/
	int		pb_id;			/* Plaback ID (== 0 for Master)				*/
	int		time_base;		/* Time Base Clock [MHz]					*/
	int		obs_rate;		/* Recording data rate at Observation [MHz] */
	unsigned int soy_fxstart;/* Second of Year at FX-START				*/
	int		doy_fxstart;	/* Day of Year at Observation				*/
	int		hh_fxstart;		/* Hour at Observation						*/
	int		mm_fxstart;		/* Minute at Observation					*/
	int		ss_fxstart;		/* Second at Observation					*/
	int		master_utcset;	/* UTCSET Flag for MASTER TCU				*/
	unsigned int soy_tcu;	/* Current TCU SOY							*/
	unsigned int tss_tcu;	/* Current TCU SOY							*/
	int		tcuread_flag;
	int		sound_flag;

	/*-------- INDEX --------*/
	int		pb_index;		/* Slave Plaback Index						*/

	/*-------- TIME VARIABLE --------*/
	double	intval_timer;	/* Time interval counter					*/
	unsigned int tssid;		/* Current Master TSSID						*/
	unsigned int fraction;	/* Current Master Fraction					*/
	int			clk_diff;	/* Sync Difference (REAL - CALC) in CLK		*/
/*
---------------------------------------------------- INPUT ARGUMENT 
*/
	if(argc < 2){
		printf("USAGE : tcu_master_cntl [PB-ID]!!\n");
		printf("  PB_ID ----- Plaback ID OF THE MASTER TCU.\n");
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
	printf("Can't Access to the Shared Memory : %s !!\n", argv[0]);
		printf("RUN fxsync_shm at first.\n");
		exit(1);
	}
	fxsync_ptr  = (struct shared_mem1 *)shmat(shrd_fxsync_id, NULL, 0);

	/*-------- ACCESS TO CURRENT STATUS --------*/
	fxsched_key  = FXSYNC_KEY2;
	shrd_fxsched_id = shmget(fxsched_key, sizeof(struct shared_mem2), 0444);
	if( shrd_fxsched_id  < 0 ){
	printf("Can't Access to the Shared Memory : %s !!\n", argv[0]);
		printf("RUN fxsync_shm at first.\n");
		exit(1);
	}
	fxsched_ptr  = (struct shared_mem2 *)shmat(shrd_fxsched_id, NULL, 0);
/*
---------------------------------------------------- OPEN MASTER TCU GPIB
*/
	tcu_addr= (Addr4882_t)fxsched_ptr->tcu_addr[pb_id];
	board	= fxsched_ptr->tcu_board[pb_id];
	time_base	= fxsync_ptr->master_clk / 8;

/*
	printf("[TCU_MASTER_CNTL] GPIB BOARD = %d\n", board);
	printf("[TCU_MASTER_CNTL] TCU ADDR   = %d\n", (int)tcu_addr);
*/

	SendIFC( board );
	if(ibsta & ERR){	printf("SendIFC Error\n");	exit(1);	}

	/*-------- Initialize TCU --------*/
	tcu_init(board, tcu_addr, time_base, time_base);
	fxsync_ptr->utcrst_active = DISABLE;
	fxsync_ptr->tssset_active = DISABLE;
	fxsync_ptr->fxstrt_active = DISABLE;
	master_utcset = ENABLE;
	sleep(2);
/*
---------------------------------------------------- TIME LOOP 
*/
	sound_flag = 0;
	while(1){

		tcuread_flag = 0;
		tcuread_flag |= (fxsync_ptr->utcrst_active);
		tcuread_flag |= (fxsync_ptr->tssset_active);
		tcuread_flag |= (fxsync_ptr->fxstrt_active);

/*
		printf("STATUS=%d, MASK=%d, RESULT=%d  ",
			tcuread_flag, (ACTIVE + RESERVED), (tcuread_flag & (ACTIVE + RESERVED)) );
*/

		/*-------- READ MASTER TCU UTC --------*/
		if((tcuread_flag & (ACTIVE + RESERVED)) == 0 ){
			if(get_TCU_utc(board, tcu_addr, &soy_tcu ) == 1){
				fxsync_ptr->master_soy = soy_tcu; }
		}

		/*-------- CALC MASTER TCU TSSID --------*/
		soy2tss( fxsync_ptr->master_soy, fxsync_ptr->master_clk/8,
					&tssid, &fraction ); 
		fxsync_ptr->master_tss = tssid;
		fxsync_ptr->master_frc = fraction;

		/*-------- CHECK UTCSET POSSIILITY --------*/
		if(( (fxsync_ptr->utcset_enable & fxsync_ptr->pb_usage) ==
			fxsync_ptr->pb_usage ) && 
			( master_utcset == ENABLE ) ){

			/*-------- UTC in OBSERVATION TIME --------*/
			soy_fxstart	= (fxsync_ptr->fxstart_utc / 1000000 - 1) * 86400 
				+ ((fxsync_ptr->fxstart_utc % 1000000)/10000 )* 3600
				+ ((fxsync_ptr->fxstart_utc % 10000)/100 )	* 60
				+ (fxsync_ptr->fxstart_utc % 100 );

			/*-------- UTCSET for MASTER TCU --------*/
			fxsync_ptr->utcrst_soy = soy_fxstart - UTCSET_OFS2;
			master_utcset = ACTIVE;
			tcu_utcset(board, tcu_addr, soy_fxstart - UTCSET_OFS );
			master_utcset = DONE;
			fxsync_ptr->utcrst_active = ENABLE;
			if(get_TCU_utc(board, tcu_addr, &soy_tcu ) == 1){
				fxsync_ptr->master_soy = soy_tcu; }
			fsleep(100);
		}

		/*-------- UTC RESET PULSE --------*/
		if( (fxsync_ptr->utcrst_active == ENABLE ) &&
			(fxsync_ptr->master_soy > soy_fxstart - UTCSET_OFS2 - 5)){
			fxsync_ptr->utcrst_active = ACTIVE;
		}
		if( fxsync_ptr->utcrst_active == ACTIVE ){
			tcu_utcrst(board, tcu_addr, soy_fxstart - UTCSET_OFS2 );
			fxsync_ptr->utcrst_active = RESERVED;
			intval_timer = 0.0;
		}
		if(fxsync_ptr->utcrst_active == RESERVED ){
			intval_timer += (0.001* LOOP_WAIT);
			if( intval_timer > 1.0 ){
				intval_timer -= 1.0;
				fxsync_ptr->master_soy ++;
			}
		}
		if( (fxsync_ptr->utcrst_active == RESERVED ) &&
			(fxsync_ptr->master_soy > soy_fxstart - UTCSET_OFS2 + 1)){
			fxsync_ptr->utcrst_active = DONE;
			fxsync_ptr->tssset_active = ENABLE;
		}

		/*-------- SET TSS ID --------*/
		if( (fxsync_ptr->tssset_active == ENABLE) &&
			(fxsync_ptr->master_soy > soy_fxstart - UTCSET_OFS3 - 5) ){
			fxsync_ptr->tssset_active = ACTIVE;
		}
		if(	fxsync_ptr->tssset_active == ACTIVE ){
			tcu_tssset(board, tcu_addr, time_base, soy_fxstart-UTCSET_OFS3, 0);
			fxsync_ptr->tssset_active = RESERVED;
			intval_timer = 0.0;
		}
		if(fxsync_ptr->tssset_active == RESERVED ){
			intval_timer += (0.001* LOOP_WAIT);
			if( intval_timer > 1.0 ){
				intval_timer -= 1.0;
				fxsync_ptr->master_soy ++;
			}
		}
		if( (fxsync_ptr->tssset_active == RESERVED ) &&
			(fxsync_ptr->master_soy > soy_fxstart - UTCSET_OFS3 + 1) ){
			fxsync_ptr->tssset_active = DONE;
			fxsync_ptr->fxstrt_active = ENABLE;
		}


		/*-------- SEND FX START ENABLE SIGNAL --------*/
		if( (fxsync_ptr->master_soy > soy_fxstart - 12 ) &&
			(fxsync_ptr->fxstrt_active == ENABLE ) ){
			fxsync_ptr->fxstrt_active = ACTIVE;
		}
		if(	fxsync_ptr->fxstrt_active == ACTIVE ){
			tcu_fxstart(board, tcu_addr, fxsync_ptr->fxstart_tss );
			fxsync_ptr->fxstrt_active = RESERVED;
			intval_timer = 0.0;
		}
		if(fxsync_ptr->fxstrt_active == RESERVED ){
			intval_timer += (0.001* LOOP_WAIT);
			if( intval_timer > 1.0 ){
				intval_timer -= 1.0;
				fxsync_ptr->master_soy ++;
			}
		}
		if( (fxsync_ptr->master_soy > soy_fxstart - 6) &&
			(fxsync_ptr->fxstrt_active == RESERVED ) &&
			(sound_flag == 0 ) ){
			system(START_SOUND);
			sound_flag = 1;
		}

		if( (fxsync_ptr->master_soy > soy_fxstart + 4) && 
			(fxsync_ptr->fxstrt_active == RESERVED ) ){
			fxsync_ptr->fxstrt_active = DONE;
		}


		/*-------- CHECK UTC/TSS SYNC --------*/
		if( ( fxsync_ptr->fxstrt_active == DONE) &&
			((fxsync_ptr->master_soy % 10 ) == 0) ){

			/*-------- SYNC CHECK --------*/
			if( check_utc_tss(board, tcu_addr, time_base,
					&soy_tcu, &tss_tcu, &clk_diff) == 1){
				fxsync_ptr->master_soy = soy_tcu;
/*
				printf("SYNC DIFF = %d CLOCK at SOY= %d\n", clk_diff, soy_tcu);
*/
			}
		}

/*
---------------------------------------------------- TIME MONITOR
*/
		if(fxsync_ptr->validity >= FINISH){	sleep(10); break;}
		if(fxsync_ptr->validity >= ABSFIN){	break;}
		fsleep(LOOP_WAIT);
	}

	return(0);
}
