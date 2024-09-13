/*****************************************************
**	DMSBIN_SCAN.C: Test Module to Control DMS-16/24	**
**													**
**	AUTHOR	: KAMENO Seiji							**
**	CREATED	: 1997 6/14								**
*****************************************************/

#include	<stdio.h>
#include	<unistd.h>
#include	<sys/ugpib.h>
#include	"fxsync.inc"
#define	MOV_FMT1	"MOVE=%02dC,%02dC\r"
#define	MOV_FMT2	"MOVE=%02dC,DR%d\r"
#define	MOV_FMT3	"MOVE=DR%d,%02dC\r"
#define	MOV_FMT4	"MOVE=DR%d,DR%d\r"

int	dms_move(BRD, DMS, src_bin, dest_bin )
	int			BRD;				/* GP-IB Board ID				*/
	Addr4882_t	DMS;				/* GP-IB Address DMS-16/24		*/
	int			src_bin;			/* Source Bin Number			*/
	int			dest_bin;			/* Destination Bin Number		*/
{

	/*-------- COMMAND/MESSAGE --------*/
	char	cmd[256];		/* GPIB Command						*/
	short	result;			/* Service Request					*/

	if( (src_bin <= DMS_BIN) && (dest_bin <= DMS_BIN) ){/* MOVE BIN -> BIN	*/
		sprintf(cmd, MOV_FMT1, src_bin, dest_bin);	}

	if( (src_bin <= DMS_BIN) && (dest_bin > DMS_BIN) ){	/* LOAD BIN -> DR	*/
		sprintf(cmd, MOV_FMT2, src_bin, dest_bin - 31 );	}

	if( (src_bin > DMS_BIN) && (dest_bin <= DMS_BIN) ){	/* STORE DR -> BIN	*/
		sprintf(cmd, MOV_FMT3, src_bin - 31, dest_bin );	}

	if( (src_bin > DMS_BIN) && (dest_bin > DMS_BIN) ){	/* MOVE DR -> DR	*/
		sprintf(cmd, MOV_FMT4, src_bin - 31, dest_bin - 31 );	}

	printf("DMS[%d] COMMAND : %s\n", DMS, cmd);

	/*------- SEND COMMAND to DMS --------*/
	Send(BRD, DMS, cmd, strlen(cmd), NLend); fsleep(100);
	DevClear(BRD, DMS); sleep(20);

	while(1){
		ReadStatusByte(BRD, DMS, &result);  fsleep(100);
/*
		printf("SRQ = %X\n", result);
*/
		if( ibsta & TIMO ){
			printf("TIME OUT .... Please Init the Cart [%d]\n", DMS);
			return(-1);
		}
		if(result == 0){   break;}
	}
	sleep(1);
/*
-------------------------------------------------- ENDING
*/
	return(0);
}
