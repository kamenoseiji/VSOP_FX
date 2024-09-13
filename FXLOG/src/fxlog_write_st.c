/*********************************************************
**	FXLOG_WRITE_ST.C : Write TSS Infomation				**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"

long fxlog_write_st( st )
	struct	tss_line	st;
{
	unsigned long	tss;
	unsigned long	frc;
	unsigned long	soy;
/*
---------------------------------------------------- WRITE
*/
	dhms2soy( doy_obslog, hh_obslog, mm_obslog, ss_obslog, &soy);
	soy	+= ( offset + 3 - (ss_obslog+3)%4 );

    soy2dhms( soy, &doy_obslog, &hh_obslog, &mm_obslog, &ss_obslog);

	(void)utc2tss((unsigned long)doy_obslog,	(unsigned long)hh_obslog,
			(unsigned long)mm_obslog,	(unsigned long)ss_obslog,	clk,
			&tss,	&frc);

	tss &= 0xffffffff;

	fprintf(fx_log_ptr, "%03d%02d%02d%02d/ST/ %d\n",
		doy_obslog, hh_obslog, mm_obslog, ss_obslog,
		tss);
/*
	fprintf(fx_log_ptr, "%03d%02d%02d%02d/ST/ %d\n",
		doy_obslog, hh_obslog, mm_obslog, ss_obslog,
		st.tss);
---------------------------------------------------- ENDING
*/
	return(1);
}
