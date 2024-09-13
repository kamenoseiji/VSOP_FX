/*********************************************************
**	FXLOG_WRITE_FORM.C : Write Formatter Infomation		**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"

long fxlog_write_form( form )
	struct	form_line	form;
{
/*
---------------------------------------------------- WRITE
*/
	if(k4){
		fprintf(fx_log_ptr, "%03d%02d%02d%02d/FORM/ %02d, %02d, %s, %d, %d\n",
			doy_obslog, hh_obslog, mm_obslog, ss_obslog,
			17-form.bbc_num,		form.form_num,		form.side,
			(long)form.rate,			form.bit);
	} else {
		fprintf(fx_log_ptr, "%03d%02d%02d%02d/FORM/ %02d, %02d, %s, %d, %d\n",
			doy_obslog, hh_obslog, mm_obslog, ss_obslog,
			form.bbc_num,		form.form_num,		form.side,
			(long)form.rate,			form.bit);
	}
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
