/*****************************************************
**	DMSINIT.C	: Test Module to Control DMS-16/24	**
**													**
**	AUTHOR	: KAMENO Seiji							**
**	CREATED	: 1997 6/14								**
*****************************************************/

#include	"fxsync.inc"
#include	<unistd.h>
#include	<sys/ugpib.h>

#define	DMS_ADDR	1
#define	MAX_TRIAL	5

main(argc, argv)
	int		argc;			/* Number of Arguments			*/
	char	**argv;			/* Pointer of Arguments			*/
{

	/*-------- GPIB DEVICES --------*/
	int		dms_addr;		/* GPIB Address of DMS-16/24	*/
	int		DMS;			/* I/O Address of DMS-16/24		*/
	int		BOARD;			/* I/O Address of GPIB Board	*/
	int		gpib_ret;		/* Return Code of GPIB			*/

	/*-------- INDEX --------*/
	int		trial_index;	/* Index for Communication Trial*/

	/*-------- COMMAND/MESSAGE --------*/
	char	cmd[256];		/* GPIB Command					*/
	char	srq[1];			/* Service Request Return		*/
/*
-------------------------------------------------- INIT GPIB CONTROLLER
*/
	if( board_gpini( &BOARD ) == -1 ){
		printf("Can't Open GPIB Controller !!!\n");
		exit(0);
	}
/*
-------------------------------------------------- SENS DMS STATUS
*/
	dms_addr= atoi( argv[DMS_ADDR] );
	if( dms_gpini( dms_addr, &DMS ) == -1 ){
		printf(" Can't Open DKMS001 for ADDRESS %d\n", dms_addr);
	}

	for( trial_index=0; trial_index<MAX_TRIAL; trial_index++){
		gpib_ret = ibrsp( DMS, srq );	fsleep(100);
		printf("CODE %X: SRQ = %02X\n",gpib_ret, srq[0] );
		if( (gpib_ret & TIMO) != 0){
			printf(" TIME OUT.... Please Initialize the CART [ADDR=%d]!!\n",
				dms_addr);
			exit(0);
		}
		if( (int)srq[0] == 0 ){	break; }
	}

	if((int)srq[0] != 0 ){
		printf("DMS-16/24 #%d does not respons...\n", dms_addr); 
		exit(0);
	}
	sprintf( cmd, "LAMP=1\r\n" ); ibwrt( DMS, cmd, strlen(cmd) ); fsleep(200);
	sprintf( cmd, "INIT\r\n" ); ibwrt( DMS, cmd, strlen(cmd) ); sleep(60);
/*
-------------------------------------------------- ENDING
*/
	sprintf( cmd, "LAMP=0\r\n" ); ibwrt( DMS, cmd, strlen(cmd) );
	return(0);
}
