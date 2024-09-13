/*********************************************************
**	FXLOG_DETECT_SOURCE.C								**
**														**
**	FUNCTION: Read SOURCE Items in SKED Chapter			**
**				Detect Station Code of Nobeyama 45m 	**
**				in Each SKED Line. If Nobeyama Code is 	**
**				Detected in the Scan It Returns 1, while**
**				Nobeyama is not Included in the Scan 	**
**				It Returns -1.							** 
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"
int fxlog_detect_source()
{
	long	i, j;				/* Counter of sked_dum pointer */
	long	log_st_soy;			/* Second of Year for ST */
	long	sked_prev_et_soy;	/* Second of Year for Previous ET */
	long	sked_et_soy;		/* Second of Year for ST */
	char	stn_code[2];
	char	sign[2];			/* Sign of Declination */
/*
---------------------------------------------------- READ FILE
*/
	log_st_soy = (((doy_obslog * 24
				+ hh_obslog ) * 60
				+ mm_obslog ) * 60
				+ ss_obslog );

	for(i=0; i<num_sked; i++){

		/*-------- WHICH SKED WILL BE USED ? --------*/
		if(i == 0){
			sked_prev_et_soy = 0;
		} else {
			sked_prev_et_soy = (((sked[i-1].doy * 24
					+ sked[i-1].hour ) * 60
					+ sked[i-1].minute ) * 60
					+ sked[i-1].second 
					+ sked[i-1].duration);
		}

		sked_et_soy = (((sked[i].doy * 24
					+ sked[i].hour ) * 60
					+ sked[i].minute ) * 60
					+ sked[i].second 
					+ sked[i].duration);

		/*-------- STATION DETECTOR --------*/
		if( category_flag[STATION] == IN_TTY ){
			strcpy( stn_code, tty_station.code1 );
		} else if( category_flag[STATION] == IN_LOG ){
			strcpy( stn_code, log_station.code1 );
		}
		/*-------- SURVEY IN DRG FILE --------*/
		if((sked_prev_et_soy <= log_st_soy) && (log_st_soy <= sked_et_soy)){

			/*-------- BROWSE FOR STATION --------*/
			for(j=0; j<strlen(sked[i].station); j+=2){
				if( sked[i].station[j] == stn_code[0] ){	/* Station Code */
					sked_suffix = i;
					#ifdef DEBUG
					printf("SCHEDULE %d [%8s] ",
						sked_suffix, sked[sked_suffix].comname);
					#endif
				} /* IF */
			} /* FOR */

			/*-------- BROWSE FOR STATION --------*/
			for(j=0; j<num_source; j++){
				if(strcmp(sked[sked_suffix].comname, drg_src[j].comname) == 0){
					src_suffix = j;
					#ifdef DEBUG
					switch(drg_src[src_suffix].sign){
						case 1:		sprintf(sign, "+\0");	break;
						case -1:	sprintf(sign, "-\0");	break;
						default:	sprintf(sign, "+\0");	break;
					}
					printf("%02d %02d %7.4lf %1s%02d %02d %7.4lf\n",
						drg_src[src_suffix].rh,		drg_src[src_suffix].rm,
						drg_src[src_suffix].rs,		sign,
						drg_src[src_suffix].dd,		drg_src[src_suffix].dm,
						drg_src[src_suffix].ds);
					#endif
				}
			}
			/*-------- WRITE TO LOG FILE --------*/
			fxlog_write_source(drg_src[src_suffix]);
		}
	}
	return(1);
/*
---------------------------------------------------- ENDING
*/
}
