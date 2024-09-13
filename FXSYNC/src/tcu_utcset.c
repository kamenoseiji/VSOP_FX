/*****************************************************
**	TCUCOPY_UTCSET : Set UTC befor CORR Start Time 	**
**	AUTHOUR	: KAMENO Seiji							**
**	CREATED	: 1994/11/1								**
******************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/ugpib.h>
#define	WAIT		100	/* Wait Timer [msec] */
#define	CMD_OFS		3

unsigned int tcu_utcset(BRD, TCU, set_soy )

	int			BRD;			/* Device I/O of TCU */
	Addr4882_t	TCU;			/* Device I/O of TCU */
	int			set_soy;		/* Second of Year at UTCSET */
{
	unsigned int	doy;
	unsigned int	hh;
	unsigned int	mm;
	unsigned int	ss;
	char			cmd[256];
	short			result;
/*
--------------------------------------- UTCSET to TCU
*/
	soy2dhms( set_soy, &doy, &hh, &mm, &ss );
	sprintf(cmd, "UTCSET=%03d%02d%02d%02d\r", doy, hh, mm,ss );
	Send(BRD, TCU, cmd, strlen(cmd), NLend); fsleep(WAIT);
	ReadStatusByte(BRD, TCU, &result); DevClear(BRD, TCU);
/*
--------------------------------------- Ending
*/
	return(1);
}
