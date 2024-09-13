/*********************************************************
**	FXLOG_READ_SKED.C : Read SKED from DRUDGE File.		** 
**														**
**	FUNCTION: Read SKED Chapter.						**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"
fxlog_read_sked()
{
/*
---------------------------------------------------- READ SOURCE CHAP. 
*/
	i = 0;							/* Initialize Source Counter */
	while(1){
		/*--------- Next Chapter or End Of File ----------*/
		if( (drg_chapter != SKED)||(drg_eof_flag == 1) ){	break;}

		if(line_buf[0] != '*'){		/* Skip Comment Line */

			/*--------- Format in SKED Chapter ----------*/
			sscanf(line_buf,
				"%s %d %s %s %02d%03d%02d%02d%02d %d %s %d %s %s",
				sked[i].comname,	&sked[i].cal,		sked[i].freq,
				sked[i].preob,		&sked[i].year,		&sked[i].doy,
				&sked[i].hour,		&sked[i].minute,	&sked[i].second,
				&sked[i].duration,	sked[i].midob,		&sked[i].idle,
				sked[i].postob,		sked[i].station);
			i=i+1;
		}
		fxlog_detect_chapter();			/* Read 1-Line */
	}
	num_sked = i;					/* Number Of Scans */
	printf("READ %d SCAN SEQUENCE.\n", num_sked);
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
