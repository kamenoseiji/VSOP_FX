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
#define	BIN_FMT		"%*s %*s %s  %*s %*s %s  %*s %*s %s  %*s %*s %s  %*s %*s %s  %*s %*s %s  %*s %*s %s  %*s %*s %s"	

int	dmsbin_scan( BRD, DMS, dms_index, fxsched_ptr )
	int			BRD;			/* GP-IB Board ID					*/
	Addr4882_t	DMS;			/* GP-IB Address of DMS-16/24		*/
	int			dms_index;		/* Index of DMS-16/24				*/
	struct shared_mem2	*fxsched_ptr;	/* Pointer of Shared Memory	*/
{
	/*-------- INDEX --------*/
	int		index;			/* General Index 				*/

	/*-------- COMMAND/MESSAGE --------*/
	char	cmd[256];		/* GPIB Command					*/
	char	bindat[512];	/* Bin Data						*/

	/*-------- UCB BIN --------*/
	sprintf( cmd, "UCB?\r\n" );
	Send(BRD, DMS, cmd, strlen(cmd), NLend ); fsleep(100);
	memset(bindat, 0, 512);
	Receive(BRD, DMS, bindat, 512, STOPend); DevClear(BRD, DMS);
	for(index=0; index<strlen(bindat); index++){
		if( bindat[index] == ',' ){	bindat[index] = ' ';}
		if( bindat[index] == ';' ){	bindat[index] = ' ';}
	}
	sscanf( bindat, BIN_FMT,
		fxsched_ptr->binlabel[dms_index][0],fxsched_ptr->binlabel[dms_index][1],
		fxsched_ptr->binlabel[dms_index][2],fxsched_ptr->binlabel[dms_index][3],
		fxsched_ptr->binlabel[dms_index][4],fxsched_ptr->binlabel[dms_index][5],
		fxsched_ptr->binlabel[dms_index][6],fxsched_ptr->binlabel[dms_index][7]);
	fsleep(100);

	/*-------- UCB BIN --------*/
	sprintf( cmd, "MCB?\r\n" );
	Send(BRD, DMS, cmd, strlen(cmd), NLend ); fsleep(100);
	memset(bindat, 0, 512);
	Receive(BRD, DMS, bindat, 512, STOPend); DevClear(BRD, DMS);
	for(index=0; index<strlen(bindat); index++){
		if( bindat[index] == ',' ){	bindat[index] = ' ';}
		if( bindat[index] == ';' ){	bindat[index] = ' ';}
	}
	sscanf( bindat, BIN_FMT,
		fxsched_ptr->binlabel[dms_index][8],
		fxsched_ptr->binlabel[dms_index][9],
		fxsched_ptr->binlabel[dms_index][10],
		fxsched_ptr->binlabel[dms_index][11],
		fxsched_ptr->binlabel[dms_index][12],
		fxsched_ptr->binlabel[dms_index][13],
		fxsched_ptr->binlabel[dms_index][14],
		fxsched_ptr->binlabel[dms_index][15]);
/*
-------------------------------------------------- ENDING
*/
	return(0);
}
