/*****************************************************
**	DMS_CNTL.C	: Test Module to Control DMS-16/24	**
**													**
**	AUTHOR	: KAMENO Seiji							**
**	CREATED	: 1997 6/14								**
*****************************************************/

#include	"fxsync.inc"
#include	<unistd.h>
#include	<sys/ugpib.h>

#define	DMS_ID		1
#define	MAX_TRIAL	5
#define TRUE_MASK   (1 << pb_id)
#define FALSE_MASK  (~(1 << pb_id))

main(argc, argv)
	int		argc;			/* Number of Arguments			*/
	char	**argv;			/* Pointer of Arguments			*/
{

	/*-------- GPIB DEVICES --------*/
	Addr4882_t	dms_addr;	/* GPIB Address of DMS-16/24	*/
	int		board;			/* I/O Address of GPIB Board	*/
	int		gpib_ret;		/* Return Code of GPIB			*/

	/*-------- IDENTIFIER --------*/
	key_t	fxsync_key;		/* Keyword for Shared Memory	*/
	key_t	fxsched_key;	/* Keyword for Shared Memory	*/
	int		dms_id;			/* ID Number of DMS-24/16 (1-5)	*/
	int		dms_index;		/* Index Number of DMS-24/16 (0-4)*/
	int		shrd_fxsync_id;	/* ID for Shared Memory			*/
	int		shrd_fxsched_id;/* ID for Shared Memory			*/
	int		pb_id;			/* Currently Using PB ID		*/

	/*-------- INDEX --------*/
	int		index;			/* General Index 				*/
	int		pb_index;		/* Index for PB in the Cart		*/
	int		trial_index;	/* Index for Communication Trial*/

	/*-------- FLAG --------*/
	int		empty_flag;
	int		fill_flag;
	int		finish_timer;

	/*-------- COMMAND/MESSAGE --------*/
	struct shared_mem1 *fxsync_ptr;	/* Pointer of Shared Memory	*/
	struct shared_mem2 *fxsched_ptr;	/* Pointer of Shared Memory	*/
	char	cmd[256];		/* GPIB Command					*/
	char	bindat[512];	/* Bin Data						*/
	char	label[24][16];	/* Tape Label					*/
	short	result;			/* Service Request Return		*/

	if(argc < 2){
		printf("USAGE: dms_cntl [DMS_ID] !!\n");
		exit(0);
	}
/*
-------------------------------------------------- OPEN SHARED MEMORY
*/
	/*-------- ACCESS TO CURRENT STATUS --------*/
	fxsync_key = FXSYNC_KEY1;
	shrd_fxsync_id = shmget(fxsync_key, sizeof(struct shared_mem1), 0444);
	if( shrd_fxsync_id < 0 ){
		printf("Can't Access to the Shared Memory : %s !! \n", argv[0] );
		printf("Run fxsync_shm at first.\n");
		exit(1);
	}
	fxsync_ptr = (struct shared_mem1 *)shmat(shrd_fxsync_id, NULL, 0);

	/*-------- ACCESS TO PB SCHEDULE --------*/
	fxsched_key = FXSYNC_KEY2;
	shrd_fxsched_id = shmget(fxsched_key, sizeof(struct shared_mem2), 0444);
	if( shrd_fxsched_id < 0 ){
		printf("Can't Access to the Shared Memory : %s !! \n", argv[0] );
		printf("Run fxsync_shm at first.\n");
		exit(1);
	}
	fxsched_ptr = (struct shared_mem2 *)shmat(shrd_fxsched_id, NULL, 0);
/*
-------------------------------------------------- INIT GPIB CONTROLLER
*/
	dms_id	= atoi( argv[DMS_ID] );
	dms_index = dms_id - 1;
	dms_addr= (Addr4882_t)fxsched_ptr->dms_addr[dms_index];
	board	= fxsched_ptr->dms_board[dms_index];
	SendIFC( board );
	if(ibsta & ERR){	printf("SendIFC Error\n");	exit(1);	}
/*
-------------------------------------------------- SENS DMS STATUS
*/
	fxsync_ptr->cart_event[dms_index][0] = DISABLE;
	fxsync_ptr->cart_event[dms_index][1] = DISABLE;

	for( trial_index=0; trial_index<MAX_TRIAL; trial_index++){
		ReadStatusByte(board, dms_addr, &result);	fsleep(100);

/*
		printf("dms_cntl: SRQ = %02X\n", result );
*/
		if( ibsta  & TIMO ){
			printf("dms_cntl: TIME OUT!! Please Initialize Cart#%d.\n", dms_id);
			exit(0);
		}
		if( result == 0 ){	break; }
	}

	if( result != 0 ){
		printf("dms_cntl: DMS-16/24 #%d does not respons...\n", dms_id); 
		exit(0);
	}
	sprintf( cmd, "LAMP=1\r\n" );
	Send(board, dms_addr, cmd, strlen(cmd), NLend );
	fsleep(50); DevClear(board, dms_addr);
/*
-------------------------------------------------- SENS DMS STATUS
*/
	fxsync_ptr->cart_event[dms_index][0] = ENABLE;
	fxsync_ptr->cart_event[dms_index][1] = ENABLE;
	finish_timer = 0;
	while(1){

		/*-------- SCAN BIN LABEL INFOMATION --------*/
		dmsbin_scan( board, dms_addr, dms_index, fxsched_ptr);
		ReadStatusByte(board, dms_addr, &result); sleep(2);

		/*-------- TAPE MOVE EVENT REQUEST --------*/
		for(pb_index=0; pb_index<2; pb_index++){
			if( fxsync_ptr->cart_event[dms_index][pb_index] == RESERVED ){
				fsleep(500);
				pb_id = fxsync_ptr->load_pb_id[dms_index][pb_index];
				fxsync_ptr->cart_event[dms_index][pb_index] = ACTIVE;

				dms_move(board, dms_addr,
						fxsync_ptr->cart_source[dms_index][pb_index],
						fxsync_ptr->cart_dest[dms_index][pb_index] );
				fxsync_ptr->cart_event[dms_index][pb_index] = ENABLE;

				/*-------- DESTINATION is DR --------*/
				if( fxsync_ptr->cart_dest[dms_index][pb_index] > DMS_BIN ){
					fxsync_ptr->insert_done |= TRUE_MASK;
				}

				/*-------- SOURCE is DR --------*/
				if( fxsync_ptr->cart_source[dms_index][pb_index] > DMS_BIN ){
					fxsync_ptr->insert_done &= FALSE_MASK;
				}
			}
		}

		if( fxsync_ptr->validity >= FINISH){
			finish_timer += 2;
			if( finish_timer > 30){ break;	}
		}

	}
/*
-------------------------------------------------- ENDING
*/
	sprintf( cmd, "LAMP=0\r\n" );
	Send(board, dms_addr, cmd, strlen(cmd), NLend );
	fsleep(50); DevClear(board, dms_addr);
	return(0);
}
