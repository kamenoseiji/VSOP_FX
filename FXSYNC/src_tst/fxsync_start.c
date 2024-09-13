/*****************************************************
**	FXSYNC_PB.C	: Control a PlayBack in FXSYNC		**
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
#define	TRUE_MASK	(1 << index)
#define	FALSE_MASK	(~(1 << index))

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
-------------------------------------------------- ENABLE TO EXEC TCU_CNTL
*/
	fxsched_ptr->tcu_cntl_exec |= 0x01;				/* Enable Master Cntl	*/
	for(index=1; index<MAX_PB; index++){
		if(	(fxsync_ptr->pb_usage & (0x01<<index)) != 0){
			fxsched_ptr->tcu_cntl_exec |= TRUE_MASK;/* Enable Slave Cntl	*/

			/*-------- INITIAL STATUS --------*/
			fxsync_ptr->work_enable |= TRUE_MASK;		/* ENABLE  */
			fxsync_ptr->insert_enable	|= TRUE_MASK;	/* ENABLE  */
			fxsync_ptr->rewind_enable	&= FALSE_MASK;	/* DISABLE */
			fxsync_ptr->search_enable	&= FALSE_MASK;	/* DISABLE */
			fxsync_ptr->utcset_enable	&= FALSE_MASK;	/* DISABLE */
			fxsync_ptr->eject_enable	&= FALSE_MASK;	/* DISABLE */
		}
	}

/*
-------------------------------------------------- TCU CONTROL START
*/

	/*-------- EXEC MASTER CONTROL --------*/
	if( fork() == 0){
		sprintf( cmd[0], "tcu_master_cntl" );
		sprintf( cmd[1], "%d", 0 );
		fxsched_ptr->tcu_pid[0] =  getpid();
		printf(" Exec %s as a Child Process [PID=%d]\n",
				cmd[0], fxsched_ptr->tcu_pid[0] );
		if( execl( TCU_MASTER, cmd[0], cmd[1], (char *)NULL ) == -1){
			printf("Error in fxsync_start [EXECL %s]\n", TCU_MASTER);
			exit(1);
		}
		fxsched_ptr->tcu_cntl_exec &= 0x01;
	}

	/*-------- ACTIVATE CART/DMS --------*/
	for(index=0; index<MAX_CART; index++){

		if( (fxsync_ptr->cart_usage & (0x01<<index)) !=  UNUSE){
			fxsync_ptr->cart_event[index][0] = DISABLE;
			fxsync_ptr->cart_event[index][1] = DISABLE;
			if( fork() == 0){
				sprintf( cmd[0], "dms_cntl" );
				sprintf( cmd[1], "%d", index+1 );
				fxsched_ptr->dms_pid[index] =  getpid();
				printf(" Exec %s as a Child Process [PID=%d] (%s %s)\n",
					cmd[0], fxsched_ptr->dms_pid[index], cmd[0], cmd[1] );
				if( execl( DMS_CNTL, cmd[0], cmd[1], (char *)NULL ) == -1){
					printf("Error in fxsync_start [EXECL %s]\n", DMS_CNTL);
					exit(1);
				}
			}
		}
	}

	sleep(5);



	while(1){
		/*-------- ACTIVATE PB/TCU --------*/
		for(index=1; index<MAX_PB; index++){

			if(	(fxsched_ptr->tcu_cntl_exec & (0x01<<index)) != 0){
				fxsched_ptr->tcu_cntl_exec &= FALSE_MASK;
				if( fork() == 0){

					sprintf( cmd[0], "tcu_slave_cntl" );
					sprintf( cmd[1], "%d", index );
					fxsched_ptr->tcu_pid[index] =  getpid();
					printf(" Exec %s as a Child Process [PID=%d, PB=%d]\n",
						cmd[0], fxsched_ptr->tcu_pid[index], index );
					if( execl( TCU_SLAVE, cmd[0], cmd[1], (char *)NULL ) == -1){
						printf("Error in fxsync_start [EXECL %s]\n", TCU_SLAVE);
						fxsched_ptr->tcu_cntl_exec |= TRUE_MASK;
						exit(1);
					}
				}
				sleep(1);
			}
		}
		sleep(4);

		if(fxsync_ptr->validity >= FINISH){
			break;
		}
	}
/*
-------------------------------------------------- INIT GPIB CONTROLLER
*/

	return(0);
}
