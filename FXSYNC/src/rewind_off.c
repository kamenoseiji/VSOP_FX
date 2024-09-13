/*********************************************************
**	REWIND_OFF.C : Stop PB Rewind						**
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
	key_t	fxsync_key;		/* Keyword for Shared Memory	*/
	int		shrd_fxsync_id;	/* Shared Memory ID				*/
	struct shared_mem1 *fxsync_ptr;  /* Pointer of Shared Memory	*/
/*
---------------------------------------------------- ACCESS TO SHARED MEMORY
*/
	/*-------- ACCESS TO CURRENT STATUS --------*/
	fxsync_key	= FXSYNC_KEY1;
	shrd_fxsync_id = shmget(fxsync_key, sizeof(struct shared_mem1), 0666);
	if( shrd_fxsync_id  < 0 ){
		printf("Can't Access to the Shared Memory : %s !!\n", argv[0]);
		printf("RUN fxsync_shm at first.\n");
		exit(1);
	}
	fxsync_ptr	= (struct shared_mem1 *)shmat(shrd_fxsync_id, NULL, 0);
/*
---------------------------------------------------- TARGET PB ID
*/
	if(argc < 2){
		printf("USAGE: fxsync_stim [Playback ID] !!\n");
		exit(0);
	}
	pb_id = atoi( argv[1] );

	fxsync_ptr->rewind_enable &= FALSE_MASK;
	fxsync_ptr->rewind_done   |= TRUE_MASK;
	fxsync_ptr->search_enable |= TRUE_MASK;
/*
---------------------------------------------------- LOOP FOR TIME
*/
	return(0);
}
