/*********************************************************
**	FXLOG_WRITE_BBC.C : Write BaseBand Converter Info.	**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"

long fxlog_write_bbc( bbc )
	struct	bbc_line	bbc;
{
/*
---------------------------------------------------- WRITE
*/
	fprintf(fx_log_ptr, "%03d%02d%02d%02d/BBC/ %02d, %02d, %8.2f\n",
		doy_obslog, hh_obslog, mm_obslog, ss_obslog,
		bbc.lo_num,		bbc.bbc_num,	bbc.freq);
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
