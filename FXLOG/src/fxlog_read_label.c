/*********************************************************
**	FXLOG_READ_LABEL.C : Read LABEL Information.		**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"
fxlog_read_label()
{
/*
---------------------------------------------------- READ CODE CHAP. 
*/
	if( category_flag[LABEL] != IN_LOG){	return(0);}

	/*-------- READ FROM LOG FILE --------*/
	sscanf(line_buf, "%03d%02d%02d%02d%s %s",
		&doy_obslog, &hh_obslog, &mm_obslog, &ss_obslog,
		dum,	log_label.label);
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
