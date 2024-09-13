/*********************************************************
**	FXLOG_READ_SOURCE.C : Read Source Information.		** 
**														**
**	FUNCTION: Read Source Position in SOURCE Chapter.	**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"
fxlog_read_source()
{
	char	sourcedum[4];		/* +DD */
/*
---------------------------------------------------- READ SOURCE CHAP. 
*/
	switch(category_flag[SOURCE]){

	case IN_LOG: /*-------- READ FROM LOG FILE --------*/
		sscanf(line_buf, "%03d%02d%02d%02d%s %s %s %d %d %lf %s %d %lf %lf %lf %lf",
			&doy_obslog, &hh_obslog, &mm_obslog, &ss_obslog, dum,
			log_src.iauname,	log_src.comname,	&log_src.rh,
			&log_src.rm,		&log_src.rs,		sourcedum,
			&log_src.dm,		&log_src.ds,		&log_src.epoch,
			&log_src.pa,		&log_src.pd);
			if(sourcedum[0] == '-'){
				log_src.sign =-1;
			} else {
				log_src.sign = 1;
			}
			sscanf(sourcedum, "%d", &log_src.dd);
			log_src.dd = abs(log_src.dd);

			/*----------- If Common Name is not Defined -----------*/
			if( log_src.comname[0] == '$' ){
				strcpy(log_src.comname, log_src.iauname);
			}

		break;

	case IN_DRG: /*-------- READ FROM DRG FILE --------*/
		i = 0;							/* Initialize Source Counter */
		while(1){
			/*--------- Next Chapter or End Of File ----------*/
			if((drg_chapter != SOURCE) || (drg_eof_flag == 1)) break;

			/*-------- Skip Comment Line --------*/
			if((line_buf[0] != '*') && (line_buf[0] != '$')){

				/*--------- Format in SOURCE Chapter ----------*/
				sscanf(line_buf, "%s %s %d %d %lf %s %d %lf %lf",
					drg_src[i].iauname,		drg_src[i].comname,
					&drg_src[i].rh,			&drg_src[i].rm,
					&drg_src[i].rs,			sourcedum,
					&drg_src[i].dm,			&drg_src[i].ds,
					&drg_src[i].epoch);
				if(sourcedum[0] == '-'){
					drg_src[i].sign =-1;
				} else {
					drg_src[i].sign = 1;
				}
				sscanf(sourcedum, "%d", &drg_src[i].dd);
				drg_src[i].dd = abs(drg_src[i].dd);

				/*----------- If Common Name is not Defined -----------*/
				if( drg_src[i].comname[0] == '$' ){
					strcpy(drg_src[i].comname, drg_src[i].iauname);
				}

				/*----------- Count Up Source ID Number -----------*/
				i = i + 1;
			}
			fxlog_detect_chapter();			/* Read 1-Line */
		}
		num_source = i;				/* Number Of Source */
		printf("READ %d SOURCE INFORMATIONS.\n", num_source);
/*
---------------------------------------------------- BROUWSE SOURCE NAME
*/
		#ifdef DEBUG
			for(i=0; i<num_source; i++){
				printf("SOURCE %2d: %s\n", i+1, drg_src[i].comname);
			}
		#endif
		break;
	} /* switch */
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
