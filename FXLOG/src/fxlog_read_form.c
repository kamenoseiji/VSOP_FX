/*********************************************************
**	FXLOG_READ_FORM.C : Read Sampler Infomation			**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"
fxlog_read_form()
{
/*
---------------------------------------------------- READ CODE CHAP. 
*/
	if( category_flag[FORM] != IN_LOG){	return(0);}

	/*-------- READ FROM LOG FILE --------*/
	sscanf(line_buf, "%03d%02d%02d%02d%s %s %d",
		&doy_obslog,&hh_obslog, &mm_obslog, &ss_obslog, dum,
		dum,	&form_cntr);

	/*-------- READ FROM LOG FILE --------*/
	sscanf(line_buf, "%03d%02d%02d%02d%s %d %d %s %lf %d",
		&doy_obslog,&hh_obslog, &mm_obslog, &ss_obslog, dum,
		&log_form[form_cntr].bbc_num,	&log_form[form_cntr].form_num,
		log_form[form_cntr].side,		&log_form[form_cntr].rate,
		&log_form[form_cntr].bit);
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
