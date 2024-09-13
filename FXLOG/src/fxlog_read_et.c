/*********************************************************
**	FXLOG_READ_ET.C : Read Tape END ID Infomation		**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"
fxlog_read_et()
{
/*
---------------------------------------------------- READ CODE CHAP. 
*/
	if( category_flag[ET] != IN_LOG){	return(0);}

	/*-------- READ FROM LOG FILE --------*/
	sscanf(line_buf, "%03d%02d%02d%02d%s %d",
		&doy_obslog,&hh_obslog, &mm_obslog, &ss_obslog, dum,
		&log_et.tss);
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
