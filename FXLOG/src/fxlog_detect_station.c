/*********************************************************
**	FXLOG_DETECT_STATION.C								**
**														**
**	FUNCTION: Read STATION Items in SKED Chapter		**
**				Detect Station Code of Nobeyama 45m 	**
**				in Each SKED Line. If Nobeyama Code is 	**
**				Detected in the Scan It Returns 1, while**
**				Nobeyama is not Included in the Scan 	**
**				It Returns -1.							** 
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"
int fxlog_detect_station()
{
	int j;			/* Counter of sked_dum pointer */
/*
---------------------------------------------------- READ FILE
*/
	for(j=0; j<strlen(sked_station); j+=2){
		if( sked_station[j] == nro_code1[0] ){	/* Detect Nobeyama Code */
			switch(sked_station[j+1]){
				case 'c' :	sked[i].az_mode = -1;	/* Counter Clockwise */
							break;
				case 'w' :	sked[i].az_mode = 1;	/* Clockwise */
							break;
				case '-' :	sked[i].az_mode = 0;	/* Normal Position */
							break;
			}
			return(1);
		}
	}
/*
---------------------------------------------------- ENDING
*/
	return(-1);
}
