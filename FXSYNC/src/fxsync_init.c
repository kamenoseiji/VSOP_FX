/*****************************************************
**	FXSYNC_INIT.C: INITIALIZE/FINISH all processes	**
**			releted to FXSYNC						**
**													**
**	AUTHOR	: KAMENO Seiji							**
**	CREATED	: 1997 6/14								**
*****************************************************/

#include	"fxsync.inc"
#include	<unistd.h>

#define	DMS_ID		1
#define	DMS_ADDR	2
#define	MAX_TRIAL	5

main(argc, argv)
	int		argc;			/* Number of Arguments			*/
	char	**argv;			/* Pointer of Arguments			*/
{

	/*-------- IDENTIFIER --------*/
	key_t	fxsync_key;		/* Keyword for Shared Memory	*/
	int		shrd_fxsync_id;	/* ID for Shared Memory			*/

	/*-------- COMMAND/MESSAGE --------*/
	struct shared_mem1 *fxsync_ptr;	/* Pointer of Shared Memory	*/
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


	if( (argc >= 2) && ( atoi(argv[1]) == 1 ) ){
		fxsync_ptr->validity = ABSFIN;
	} else {
		fxsync_ptr->validity = FINISH;
	}
	printf(" SET VALIDITY %d\n", fxsync_ptr->validity);

	return(0);
}
