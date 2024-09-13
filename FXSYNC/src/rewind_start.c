/*****************************************************
**	REWIND_START.C	: Control a PlayBack to Rewind	**
**													**
**	AUTHOR	: KAMENO Seiji							**
**	CREATED	: 1997 6/14								**
*****************************************************/

#include	"fxsync.inc"
#include	<unistd.h>
#include	<sys/ugpib.h>

#define	DMS_ID		1
#define	DMS_ADDR	2
#define	MAX_TRIAL	5

main(argc, argv)
	int		argc;			/* Number of Arguments			*/
	char	**argv;			/* Pointer of Arguments			*/
{

	/*-------- IDENTIFIER --------*/
	key_t	fxsync_key;		/* Keyword for Shared Memory	*/
	key_t	fxsched_key;	/* Keyword for Shared Memory	*/
	int		shrd_fxsync_id;	/* ID for Shared Memory			*/
	int		shrd_fxsched_id;/* ID for Shared Memory			*/
	int		dms_id;			/* ID for DMS-16/24				*/
	int		pb_id[2];		/* PB ID of Installed DIR		*/ 

	/*-------- INDEX --------*/
	int		index;			/* General Index 				*/

	/*-------- COMMAND/MESSAGE --------*/
	struct shared_mem1 *fxsync_ptr;	/* Pointer of Shared Memory	*/
	struct shared_mem2 *fxsched_ptr;/* Pointer of Shared Memory	*/
	char	cmd[8][16];		/* Chiled Process Comannd		*/

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
-------------------------------------------------- READ PB SCHEDULE
*/
	if( fork() == 0){
		sprintf( cmd[0], "read_pbsched" );
		printf(" Exec %s as a Child Process [PID=%d]\n", cmd[0], getpid() );
		if( execl( PBSCHED, cmd[0], (char *)NULL ) == -1){
			printf("Error in fxsync_start [EXECL %s]\n", PBSCHED);
			exit(1);
		}
	}
	wait(NULL);
	fxsync_info( fxsched_ptr );
/*
-------------------------------------------------- TCU CONTROL START
*/
	/*-------- ACTIVATE CART/DMS --------*/
	for(index=0; index<MAX_CART; index++){

		if( (fxsync_ptr->cart_usage & (0x01<<index)) !=  UNUSE){
			fxsync_ptr->cart_event[index][0] = DISABLE;
			fxsync_ptr->cart_event[index][1] = DISABLE;
			if( fork() == 0){
				sprintf( cmd[0], "dms_cntl" );
				sprintf( cmd[1], "%d", index+1 );
				printf(" Exec %s as a Child Process [PID=%d, DMS=%d] (%s %s)\n",
					cmd[0], getpid(), dms_id, cmd[0], cmd[1] );
				if( execl( DMS_CNTL, cmd[0], cmd[1], (char *)NULL ) == -1){
					printf("Error in fxsync_start [EXECL %s]\n", DMS_CNTL);
					exit(1);
				}
			}
		}

	}


	sleep(5);
	/*-------- ACTIVATE PB/TCU --------*/
	for(index=1; index<MAX_PB; index++){
		if(	(fxsync_ptr->pb_usage & (0x01<<index)) != UNUSE){
			if( fork() == 0){
				sprintf( cmd[0], "tcu_rewind_cntl" );
				sprintf( cmd[1], "%d", index );
				printf(" Exec %s as a Child Process [PID=%d, PB=%d]\n",
					cmd[0], getpid(), index );
				if( execl( TCU_REWIND, cmd[0], cmd[1], (char *)NULL ) == -1){
					printf("Error in fxsync_start [EXECL %s]\n", TCU_REWIND);
					exit(1);
				}
			}
			sleep(1);
		}
	}
/*
-------------------------------------------------- INIT GPIB CONTROLLER
	wait(NULL);
	fxsync_ptr->validity = FINISH;

	printf(" SET VALIDITY %d\n", fxsync_ptr->validity);
*/

	return(0);
}
