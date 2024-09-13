/*********************************************************
**	TCU_RESET.C : Re-Execute TCU_SLAVE_CNTL				**
**														**
**	FUNCTION: Change REWIND_DONE Status Manually		**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxsync.inc"
#include <cpgplot.h>
#include <unistd.h>
#define TRUE_MASK   (1 << pb_id)
#define FALSE_MASK  (~(1 << pb_id))


main(argc, argv)
	long	argc;		/* Number of Argument */
	char	**argv;		/* Pointer of Argument */
{
	/*-------- INDEX --------*/
	int		pb_id;

	/*-------- SHARED MEMORY FOR CURRENT STATUS --------*/
	key_t	fxsched_key;		/* Keyword for Shared Memory	*/
	int		shrd_fxsched_id;	/* Shared Memory ID				*/
	struct shared_mem2 *fxsched_ptr;  /* Pointer of Shared Memory	*/

	/*-------- SYSTEM COMMAND --------*/
	char	cmd[64];

/*
---------------------------------------------------- ARGUMENT CHECK
*/
	if(argc < 2){
		printf("USAGE: tcu_reset [Playback ID] !!\n");
		exit(0);
	}
/*
---------------------------------------------------- ACCESS TO SHARED MEMORY
*/
	/*-------- ACCESS TO PB SCHEDULE --------*/
	fxsched_key	= FXSYNC_KEY2;
	shrd_fxsched_id = shmget(fxsched_key, sizeof(struct shared_mem2), 0666);
	if( shrd_fxsched_id  < 0 ){
		printf("Can't Access to the Shared Memory : %s !!\n", argv[0]);
		printf("RUN fxsync_shm at first.\n");
		exit(1);
	}
	fxsched_ptr	= (struct shared_mem2 *)shmat(shrd_fxsched_id, NULL, 0);
/*
---------------------------------------------------- TARGET PB ID
*/
	pb_id = atoi( argv[1] );

	/*-------- KILL TCU_SLAVE_CNTL --------*/
	sprintf(cmd, "kill -KILL %d\0", fxsched_ptr->tcu_pid[pb_id]);
	system(cmd);
	printf("EXEC %s\n", cmd);
	sleep(2);

	/*-------- EXEC TCU_SLAVE_CNTL --------*/
	fxsched_ptr->tcu_cntl_exec |= TRUE_MASK;
/*
---------------------------------------------------- END
*/
	return(0);
}
