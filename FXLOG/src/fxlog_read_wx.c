/*********************************************************
**	FXLOG_READ_WX.C : Read Weather Infomation			**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"
fxlog_read_wx()
{
/*
---------------------------------------------------- READ CODE CHAP. 
*/
	if( category_flag[WX] != IN_LOG){	return(0);}

	/*-------- READ FROM LOG FILE --------*/
	sscanf(line_buf, "%03d%02d%02d%02d%s %lf %lf %lf",
		&doy_obslog,&hh_obslog, &mm_obslog, &ss_obslog, dum,
		&log_wx.temp,	&log_wx.press,	&log_wx.humd);
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
