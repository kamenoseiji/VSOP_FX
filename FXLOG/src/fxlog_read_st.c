/*********************************************************
**	FXLOG_READ_ST.C : Read Tape Start ID Infomation		**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"
fxlog_read_st()
{
/*
---------------------------------------------------- READ CODE CHAP. 
*/
	if( category_flag[ST] != IN_LOG){	return(0);}

	/*-------- READ FROM LOG FILE --------*/
	sscanf(line_buf, "%03d%02d%02d%02d%s %d",
		&doy_obslog,&hh_obslog, &mm_obslog, &ss_obslog, dum,
		&log_st.tss);
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
