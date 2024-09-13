/*****************************************************
**	check_utc_tss : Check TSS and UTC SYNC		 	**
**	AUTHOUR	: KAMENO Seiji							**
**	CREATED	: 1994/11/1								**
******************************************************/

#include "fxsync.inc"
#include <stdio.h>
#include <unistd.h>
#include <sys/ugpib.h>
#define	MARGIN	2
#define	TSSBYTE	144432
unsigned int check_tss_idr(pb_id, BRD, TCU, tcu_tss, clk_diff)
	int			pb_id;		/* Playback ID			*/
	int			BRD;		/* GP-IB board ID		*/
	Addr4882_t	TCU;		/* Device I/O of TCU	*/
	unsigned int	tcu_tss;	/* TSS in the TCU	*/
	int			*clk_diff;	/* Clock Difference (REAL - CALC) */
{
	char		cmd[64];		/* GP-IB Command					*/
	short		result;			/* SRQ Status Byte Return			*/
	unsigned int	pb_idr;		/* Playback IDR						*/
	char		dir_stat[32];	/* IDR Return 						*/
	char		idr_char[8];	/* IDR Return 						*/
/*
--------------------------------------- UTCSET to TCU
*/
	dirsend(BRD, TCU, "3012", tcu_tss, 0);		/* IDR DATA SENSE	*/
	sleep(MARGIN);
	Receive(BRD, TCU, dir_stat, 12, STOPend); fsleep(50);
	ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);

	if( strncmp(dir_stat, "3C12", 4) != 0 ){
		return(-1);
	} else {
		sscanf(dir_stat, "%*6s%06s", idr_char);
		pb_idr = idr2tss( idr_char );
		*clk_diff = tcu_tss - pb_idr;

/*
		printf("PB[%d]: TSS=%07d, IDR=%07d, DIFF=%d\n",
			pb_id, tcu_tss, pb_idr, *clk_diff);
*/
		return(1);
	}
/*
--------------------------------------- Ending
*/
}
