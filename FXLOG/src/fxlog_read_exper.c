/*********************************************************
**	FXLOG_READ_EXPER.C : Read Experiment Code from DRG.	** 
**														**
**	FUNCTION: Read Experiment Code.						**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1995/11/15									**
**********************************************************/

#include "fxlog.inc"
fxlog_read_exper()
{
/*
---------------------------------------------------- READ EXPER CHAP. 
*/
	switch(category_flag[EXPER]){

	case UNFOUND:	return(0);		break;

	case IN_LOG:	/*--------- READ FROM LOG ----------*/
		sscanf(line_buf, "%03d%02d%02d%02d%s %1s%2d%3d",
			&doy_obslog, &hh_obslog, &mm_obslog, &ss_obslog, dum,
			log_exper.code, &log_exper.year, &log_exper.doy);
		break;

	case IN_DRG:	/*--------- READ FROM DRG ----------*/
		if((drg_chapter != EXPER) || (drg_eof_flag == 1)){	return(0);}
		sscanf(line_buf, "%s %1s%2d%3d", dum,
			drg_exper.code,	&drg_exper.year,	&drg_exper.doy);
		doy_obslog = drg_exper.doy;
		hh_obslog = 0;
		mm_obslog = 0;
		ss_obslog = 0;

		drg_chapter = 0;				/* Exit from EXPER CHAPTER	*/
		break;
	}

/*
---------------------------------------------------- ENDING
*/
	return(1);
}
