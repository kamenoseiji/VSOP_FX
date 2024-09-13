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
unsigned int check_utc_tss(BRD, TCU, clk, tcu_soy, tcu_tss, clk_diff)
	int			BRD;		/* GP-IB board ID		*/
	Addr4882_t	TCU;		/* Device I/O of TCU	*/
	unsigned int	clk;	/* TCU Clock (8/16/32)	*/
	unsigned int	*tcu_soy;	/* SOY in the TCU	*/
	unsigned int	*tcu_tss;	/* TSSID in the TCU							*/
	int			*clk_diff;	/* Clock Difference (REAL - CALC) */
{
	char			cmd[64];	/* GP-IB Command							*/
	short			result;		/* SRQ Status Byte Return					*/
	int				doy;		/* DOY of Check Timing						*/
	int				hh;			/* Hour of Check Timing						*/
	int				mm;			/* Minute of Check Timing					*/
	int				ss;			/* Second of Check Timing					*/
	unsigned int	tcu_frc;	/* Fraction in the TCU						*/
	unsigned int	calc_tss;	/* Ideal TSSID								*/
	unsigned int	calc_frc;	/* Ideal Fraction							*/
/*
--------------------------------------- READ CURRENT TCU UTC
*/
	/*-------- CHECK UTC in the TCU --------*/
	if( get_TCU_utc(BRD, TCU, tcu_soy) == -1 ){	return(-1);	}

	*tcu_soy += MARGIN;
/*
--------------------------------------- UTCSET to TCU
*/
	soy2dhms( *tcu_soy, &doy, &hh, &mm, &ss );
	soy2tss( *tcu_soy, clk, &calc_tss, &calc_frc);

	sprintf(cmd, "UTCACT=%02d%02d%02d\r", hh, mm,ss );
	Send(BRD, TCU, cmd, strlen(cmd), NLend); fsleep(50);
	ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);

	sprintf(cmd, "ACTION=TSSSET?\r");
	Send(BRD, TCU, cmd, strlen(cmd), NLend);
	sleep(MARGIN); fsleep(100);
	Receive(BRD, TCU, cmd, 16, STOPend); fsleep(50);

	if( cmd[0] != 'B' ){
		sscanf(cmd, "%07d%*1s%d", tcu_tss, &tcu_frc);
		ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);
		*clk_diff = (*tcu_tss - calc_tss)* TSSBYTE + (tcu_frc - calc_frc);
		return(1);
	} else {
		ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);
		return(-1);
	}
/*
--------------------------------------- Ending
*/
}
