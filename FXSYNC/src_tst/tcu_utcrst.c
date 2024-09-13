/*****************************************************
**	TCUCOPY_UTCSET : Set UTC befor CORR Start Time 	**
**	AUTHOUR	: KAMENO Seiji							**
**	CREATED	: 1994/11/1								**
******************************************************/

#include "fxsync.inc"
#include <stdio.h>
#include <unistd.h>
#include <sys/ugpib.h>
unsigned int tcu_utcrst(BRD, TCU, set_soy )

	int			BRD;			/* GP-IB Board ID			*/
	Addr4882_t	TCU;			/* Device I/O of TCU		*/
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
	sprintf(cmd, "UTCACT=%02d%02d%02d,UTCRST\r", hh, mm,ss );
	Send(BRD, TCU, cmd, strlen(cmd), NLend);
	fsleep(100); ReadStatusByte(BRD, TCU, &result), DevClear(BRD, TCU);
/*
--------------------------------------- Ending
*/
	return(1);
}
