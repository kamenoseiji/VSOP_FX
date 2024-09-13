/*********************************************************
**	FXSYNC_DUMP.C : Generate PS File of FXSYNC STATUS	**
**														**
**	FUNCTION: Open Drudge File and Read.				**
**				Data File.								**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxsync.inc"
#include <cpgplot.h>
#include <unistd.h>
#define	REC_START	0
#define	REC_STOP	1
#define	DUMP_FILE1	"/home1/FXSYNC/fxsync.dump1"
#define	DUMP_FILE2	"/home1/FXSYNC/fxsync.dump2"
#define TRUE_MASK   (1 << pb_id)
#define FALSE_MASK  (~(1 << pb_id))


main(argc, argv)
	long	argc;		/* Number of Argument */
	char	**argv;		/* Pointer of Argument */
{
	/*-------- DUMP FILE --------*/
	FILE	*file_ptr1;		/* File Pointer of Dump File	*/
	FILE	*file_ptr2;		/* File Pointer of Dump File	*/

	/*-------- INDEX --------*/
	int		pb_index;		/* Index for Playback	*/
	int		pb_id;

	/*-------- SHARED MEMORY FOR CURRENT STATUS --------*/
	key_t	fxsync_key;			/* Keyword for Shared Memory	*/
	key_t	fxsched_key;		/* Keyword for Shared Memory	*/
	int		shrd_fxsync_id;		/* Shared Memory ID				*/
	int		shrd_fxsched_id;	/* Shared Memory ID				*/
	struct shared_mem1 *fxsync_ptr;	/* Pointer of Shared Memory	*/
	struct shared_mem2 *fxsched_ptr;	/* Pointer of Shared Memory	*/

/*
---------------------------------------------------- ACCESS TO SHARED MEMORY
*/
	/*-------- ACCESS TO CURRENT STATUS --------*/
	fxsync_key	= FXSYNC_KEY1;
	shrd_fxsync_id = shmget(fxsync_key, sizeof(struct shared_mem1), 0444);
	if( shrd_fxsync_id  < 0 ){
		printf("Can't Access to the Shared Memory : %s !!\n", argv[0]);
		printf("RUN fxsync_shm at first.\n");
		exit(1);
	}
	fxsync_ptr	= (struct shared_mem1 *)shmat(shrd_fxsync_id, NULL, 0);

	/*-------- ACCESS TO PB SCHEDULE --------*/
	fxsched_key	= FXSYNC_KEY2;
	shrd_fxsched_id = shmget(fxsched_key, sizeof(struct shared_mem2), 0444);
	if( shrd_fxsched_id  < 0 ){
		printf("Can't Access to the Shared Memory : %s !!\n", argv[0]);
		printf("RUN fxsync_shm at first.\n");
		exit(1);
	}
	fxsched_ptr	= (struct shared_mem2 *)shmat(shrd_fxsched_id, NULL, 0);
/*
---------------------------------------------------- OPEN PGPLOT DEVICE
	while( (argc>1) && (argv[1][0] == '-' )){
		switch(argv[1][1]){
		case 'e' :
			exit(0);
			break;

		default :
			break;
		}
		argc--; argv++;
	}
---------------------------------------------------- LOOP FOR TIME
*/
	/*-------- DUMP FILE 2: PB SCHEDULE --------*/
	file_ptr2 = fopen( DUMP_FILE2, "w" );
	fwrite( fxsched_ptr, 1, sizeof(struct shared_mem2), file_ptr2 );
	fclose( file_ptr2 );

	file_ptr1 = fopen( DUMP_FILE1, "w" );
	while(fxsync_ptr->validity < FINISH){
		fwrite( fxsync_ptr, 1, sizeof(struct shared_mem1), file_ptr1 );
		sleep(1);
	}
	fclose( file_ptr1 );
	return(0);
}
