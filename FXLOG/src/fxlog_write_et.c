/*********************************************************
**	FXLOG_WRITE_ET.C : Write TSS Infomation				**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"

long fxlog_write_et( et )
	struct	tss_line	et;
{
	unsigned long	tss;
	unsigned long	frc;
/*
---------------------------------------------------- WRITE
	ss_obslog   = ss_obslog + 3 - (ss_obslog+3)%4;
*/
	(void)utc2tss((unsigned long)doy_obslog,	(unsigned long)hh_obslog,
			(unsigned long)mm_obslog,	(unsigned long)ss_obslog, clk,
			&tss,	&frc);

	tss &= 0xffffffff;
	fprintf(fx_log_ptr, "%03d%02d%02d%02d/ET/ %d\n",
		doy_obslog, hh_obslog, mm_obslog, ss_obslog, tss);
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
