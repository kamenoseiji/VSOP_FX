/*********************************************************
**	FXLOG_WRITE_EXPER.C : Write EXPER to FX LOG FILE 	** 
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1995/11/26								**
**********************************************************/

#include "fxlog.inc"

long fxlog_write_exper(exper)
	struct	exper_line	exper;
{
/*
---------------------------------------------------- READ ONE-LINE
*/
	#ifdef DEBUG
	printf("%03d%02d%02d%02d/EXPER/ %01s%02d%03d\n",
		doy_obslog, hh_obslog, mm_obslog, ss_obslog,
		exper.code, exper.year, exper.doy);

	#endif

	fprintf(fx_log_ptr, "%03d%02d%02d%02d/EXPER/ %01s%02d%03d\n",
		doy_obslog, hh_obslog, mm_obslog, ss_obslog,
		exper.code, exper.year, exper.doy);

	category_flag[EXPER] = WRITTEN;
	return(0);
}
