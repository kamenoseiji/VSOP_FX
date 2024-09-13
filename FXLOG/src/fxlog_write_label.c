/*********************************************************
**	FXLOG_WRITE_LABEL.C : Write Tape Label				**
**						FX LOG FILE.					**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1995/11/26								**
**********************************************************/

#include "fxlog.inc"

long fxlog_write_label(label)
	struct	label_line	label;
{
	int	i;
/*
-------------------------------------------- CONVERT TO CAPITAL LETTER
*/
	for(i=0; i<strlen(label.label); i++){
		if( label.label[i] == '\0'){	break;}
		label.label[i] = toupper(label.label[i]);
	}
/*
---------------------------------------------------- READ ONE-LINE
*/
	fprintf(fx_log_ptr, "%03d%02d%02d%02d/LABEL/ %s\n",
			doy_obslog, hh_obslog, mm_obslog, ss_obslog,
			label.label);

	return(0);
}
