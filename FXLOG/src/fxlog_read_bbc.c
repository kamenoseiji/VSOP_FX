/*********************************************************
**	FXLOG_READ_BBC.C : Read BaseBand Converter Info.	**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"
int fxlog_read_bbc()
{
/*
---------------------------------------------------- READ CODE CHAP. 
*/
	if( category_flag[BBC] != IN_LOG){	return(0);}

	/*-------- READ FROM LOG FILE --------*/
	sscanf(line_buf, "%03d%02d%02d%02d%s %s %d %s",
		&doy_obslog,&hh_obslog, &mm_obslog, &ss_obslog, dum,
		dum,	&bbc_cntr,	dum);

	/*-------- READ FROM LOG FILE --------*/
	sscanf(line_buf, "%03d%02d%02d%02d%s %d %d %lf",
		&doy_obslog,&hh_obslog, &mm_obslog, &ss_obslog, dum,
		&log_bbc[bbc_cntr].lo_num,	&log_bbc[bbc_cntr].bbc_num,
		&log_bbc[bbc_cntr].freq);
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
