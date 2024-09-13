/*************************************************************
**	fxsync_shm.c : Open Shared Memory for Observing System	**
**															**
**	AUTHOR	: KAMENO Seiji									**
**	CREATED	: 1996/11/23									**
**************************************************************/

#include "fxsync.inc"

main(argc, argv)
	int		argc;				/* Number of Arguments */
	char	**argv;				/* Pointer of Arguments */
{
	key_t	fxsync_key;					/* Keyword for Current Status	*/
	key_t	fxsched_key;				/* Keyword for PB Schedule		*/
	int		shrd_fxsync_id;				/* Shared Memory ID				*/
	int		shrd_fxsched_id;			/* Shared Memory ID				*/
	struct shared_mem1	*fxsync_ptr;	/* Pointer of Current Status	*/
	struct shared_mem2	*fxsched_ptr;	/* Pointer of Plaback Schedule	*/
/*
-------------------------------------------- ALLOC SHARED MEMORY
*/
	/*-------- SHARED MEMORY for CURRENT STATUS --------*/
	fxsync_key = FXSYNC_KEY1;
	shrd_fxsync_id = shmget(
			fxsync_key, sizeof(struct shared_mem1), IPC_CREAT|0666);

	if(shrd_fxsync_id < 0){
		printf("Error in [shmget] : %s !!\n", argv[0]);
		exit(1);
	}
	fxsync_ptr	= (struct shared_mem1 *)shmat(shrd_fxsync_id, NULL, 0);
	memset( fxsync_ptr, 0, sizeof(struct shared_mem1) );


	/*-------- SHARED MEMORY for PB SCHEDULE --------*/
	fxsched_key = FXSYNC_KEY2;
	shrd_fxsched_id = shmget(
			fxsched_key, sizeof(struct shared_mem2), IPC_CREAT|0666);

	if(shrd_fxsched_id < 0){
		printf("Error in [shmget] : %s !!\n", argv[0]);
		exit(1);
	}
	fxsched_ptr	= (struct shared_mem2 *)shmat(shrd_fxsched_id, NULL, 0);
	memset( fxsched_ptr, 0, sizeof(struct shared_mem2) );

/*
-------------------------------------------- LOOP
*/
	while(1){
		if( fxsync_ptr->validity == FINISH ){	break;}

		/*-------- ABSOLUTE FINISH --------*/
		if( fxsync_ptr->validity == ABSFIN ){
			if( shmctl(shrd_fxsync_id, IPC_RMID, 0) < 0){
				printf("Error in [shmctl] : %s\n", argv[0] );
				exit(1);
			}
			if( shmctl(shrd_fxsched_id, IPC_RMID, 0) < 0){
				printf("Error in [shmctl] : %s\n", argv[0] );
				exit(1);
			}
			exit(0);
		}

		sleep(2);
	}
	sleep(60);
/*
-------------------------------------------- CLEAR SHARED MEMORY
*/
	if( shmctl(shrd_fxsync_id, IPC_RMID, 0) < 0){
		printf("Error in [shmctl] : %s\n", argv[0] );
		exit(1);
	}
	if( shmctl(shrd_fxsched_id, IPC_RMID, 0) < 0){
		printf("Error in [shmctl] : %s\n", argv[0] );
		exit(1);
	}
	exit(0);
}
