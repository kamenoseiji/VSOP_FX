/*****************************************************
**	TCUCOPY_UTCSET : Set UTC befor CORR Start Time 	**
**	AUTHOUR	: KAMENO Seiji							**
**	CREATED	: 1994/11/1								**
******************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/ugpib.h>
unsigned int get_TCU_utc( BRD, TCU, soy_ptr )

	int			BRD;			/* Device I/O of TCU */
	Addr4882_t	TCU;			/* Device I/O of TCU */
	int			*soy_ptr;		/* Second of Year at UTCSET */
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
	memset(cmd, 0, 256);
	sprintf(cmd, "UTCSET?\r");
	Send(BRD, TCU, cmd, strlen(cmd), NLend); fsleep(100);
	Receive(BRD, TCU, cmd, 9, STOPend);
	ReadStatusByte(BRD, TCU, &result ); DevClear(BRD, TCU);

	if( cmd[0] != 'B' ){
		sscanf(cmd, "%03d%02d%02d%02d", &doy, &hh, &mm, &ss);
		dhms2soy( doy, hh, mm, ss, soy_ptr);

		return(1);
	} else {
		*soy_ptr = 0;
		return(-1);
	}
/*
--------------------------------------- Ending
*/
}
