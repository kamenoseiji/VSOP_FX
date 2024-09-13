/*********************************************************
**	FXLOG_WRITE_WX.C : Write Weather Infomation			**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"

long fxlog_write_wx( wx )
	struct	wx_line	wx;
{
/*
---------------------------------------------------- WRITE
*/
	fprintf(fx_log_ptr, "%03d%02d%02d%02d/WX/ %.2f, %.2f, %.2f\n",
		doy_obslog, hh_obslog, mm_obslog, ss_obslog,
		wx.temp,	wx.press,	wx.humd);
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
