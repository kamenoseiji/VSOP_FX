/*********************************************************
**	FXLOG_DRG_READ.C : Read Drudge File [PC-SCHED]		**
**														**
**	FUNCTION: Open Drudge File and Read.				**
**				Data File.								**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"
fxlog_drg_read()
{
/*
---------------------------------------------------- READ FILE
*/
	drg_chapter		= 0;					/* Chapter is not defined */
	drg_eof_flag	= 0;					/* Initial EOF Flag */
	source_counter = 0;						/* Initialize Source Counter */
	fxlog_detect_chapter();
	while( drg_eof_flag != 1 ){

		printf("STATION[1]: %s\n", drg_station[1].name);

		switch(drg_chapter){
		case EXPER :	if(category_flag[EXPER] != IN_LOG){
							/*---- IF LOG HAS NO EXPER LINE ----*/
							category_flag[EXPER] = IN_DRG;
							fxlog_read_exper();
							fxlog_write_exper(drg_exper);	/* Write EXPER */
						}
						drg_chapter=0;		/* Exit From EXPER Chpater */
						break;

		case PARAM :
						fxlog_read_param();
						drg_chapter=0;		/* Exit From EXPER Chpater */
						break;

		case SOURCE :	if(category_flag[SOURCE] != IN_LOG){
							category_flag[SOURCE] = IN_DRG;
							fxlog_read_source();
						} else {
							drg_chapter = 0;
						}
						break;

		case STATION :	if(category_flag[STATION] != IN_LOG){
							category_flag[STATION] = IN_DRG;
							fxlog_read_station();
						} else {
							drg_chapter = 0;
						}
						break;

		case SKED :		if(category_flag[SOURCE] != IN_LOG){
							category_flag[SOURCE] = IN_DRG;
							fxlog_read_sked();
						} else {
							drg_chapter = 0;
						}
						break;

#ifdef DEBUG
		case CODE :		fxlog_read_code();
						break;
#endif

		default :		fxlog_detect_chapter();	/* Recognize Mode */
						break;
		}
	}
	#ifdef DEBUG
		printf("Finished Reading Drudge File!\n");
	#endif
	return(0);
/*
---------------------------------------------------- ENDING
*/
}
