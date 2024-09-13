/*****************************************************
**	TCUCOPY_UTCSET : Set UTC befor CORR Start Time 	**
**	AUTHOUR	: KAMENO Seiji							**
**	CREATED	: 1994/11/1								**
******************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/ugpib.h>
#define	WAIT		100			/* Wait Timer [msec] */
#define	CMD_OFS		3
#define TSSMASK		0x7fffff	/* CLOCK per TSS */

unsigned int tcu_tssset(BRD,  TCU, clk, set_soy, polarity )

	int			BRD;			/* GP-IB Board				*/
	Addr4882_t	TCU;			/* Device I/O of TCU		*/
	int			clk;			/* TCU Clock (32/16/8)		*/
	int			set_soy;		/* Second of Year at UTCSET */
	int			polarity;		/* Clock Polarity			*/
{
	unsigned int	doy;	/* Day of Year				*/
	unsigned int	hh;		/* Hour 					*/
	unsigned int	mm;		/* Minute					*/
	unsigned int	ss;		/* Second					*/
	unsigned int	tss;	/* TSS ID					*/
	unsigned int	frc;	/* Fraction					*/
	char			cmd[256];
	short			result;

/*
--------------------------------------- UTCSET to TCU
*/
	soy2dhms( set_soy, &doy, &hh, &mm, &ss );
	soy2tss( set_soy, clk, &tss, &frc );
	frc += polarity;		/* Add Clock Polarity		*/

	sprintf(cmd, "UTCACT=%02d%02d%02d\r", hh, mm,ss );
	printf("TCU_ADDR[%d]: UTCACT=%02d%02d%02d\n", TCU, hh, mm,ss );
	Send(BRD, TCU, cmd, strlen(cmd), NLend); fsleep(WAIT);
	ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);

	sprintf(cmd, "ACTION=TSSSET=%07d,%06d\r", (tss & TSSMASK), frc);
	printf("TCU_ADDR[%d]: ACTION=TSSSET=%07d,%06d\n", TCU, (tss & TSSMASK), frc);
	Send(BRD, TCU, cmd, strlen(cmd), NLend); fsleep(WAIT);
	ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);
/*
--------------------------------------- Ending
*/
	sleep(1);
	return(1);
}
