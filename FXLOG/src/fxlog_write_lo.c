/*********************************************************
**	FXLOG_WRITE_LO.C : Write LO (Local Oscillator) to	**
**						FX LOG FILE.					**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1995/11/26								**
**********************************************************/

#include "fxlog.inc"

long fxlog_write_lo(station)
	struct	antenna_line	station;
{
/*
---------------------------------------------------- READ ONE-LINE
*/
	fprintf(fx_log_ptr, "%03d%02d%02d%02d/LO/ %02d, %.2f\n",
			doy_obslog, hh_obslog, mm_obslog, ss_obslog,
			if_cntr+1,	station.flocal[if_cntr]); 

	return(0);
}
