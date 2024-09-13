/*********************************************************
**	FXLOG_READ_LO.C : Read LOCAL Information.			**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"
fxlog_read_lo()
{
/*
---------------------------------------------------- READ CODE CHAP. 
*/
	if( category_flag[LO] != IN_LOG){	return(0);}

	/*-------- READ FROM LOG FILE --------*/
	sscanf(line_buf, "%03d%02d%02d%02d%s %d",
		&doy_obslog, &hh_obslog, &mm_obslog, &ss_obslog,
		dum,	&if_cntr);

	sscanf(line_buf, "%03d%02d%02d%02d%s %s %lf",
		&doy_obslog,&hh_obslog, &mm_obslog, &ss_obslog,
		dum,	dum,	&log_station.flocal[if_cntr]);
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
