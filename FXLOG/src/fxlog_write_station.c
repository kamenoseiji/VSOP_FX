/*********************************************************
**	FXLOG_WRITE_STATION.C : Write STATION INFORMATION	**
**						FX LOG FILE.					**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1995/11/26								**
**********************************************************/

#include "fxlog.inc"

long fxlog_write_station(station)
	struct	antenna_line	station;
{
/*
---------------------------------------------------- READ ONE-LINE
*/

	fprintf(fx_log_ptr, "%03d%02d%02d%02d/STATION/ %1s, %2s, %s, %s, %13.4f, %13.4f, %13.4f, %8f\n",
			doy_obslog, hh_obslog, mm_obslog, ss_obslog,
			station.code1,	station.code2,
			station.name,	station.type,
			station.x,		station.y,
			station.z,		station.offset);
	return(0);
}
