/*********************************************************
**	FXLOG_READ_STATION.C : Read Station Information.	**
**														**
**	FUNCTION: Read STATION Chapter and Get Station Code.**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/
	/*---------------- CAUTION ----------------
	Now we are reading a tempolary file "hidoi.log" statt the
	true log file. The word separator comma (',') is replaced
	to a space (' ') by sed. See fxlog_param.c.
	---------------- End of CAUTION ----------------*/

#include "fxlog.inc"
fxlog_read_station()
{

	char	stn_code2[3];			/* Station 2-Letter CODE (temporary) */
	char	stn_name[9];			/* Station Name (temporary) */
	double	stn_x, stn_y, stn_z;	/* Station Position (tempolary) */
/*
---------------------------------------------------- READ STATION CHAP. 
	printf("CATEGORY_FLAG: %d\n", category_flag[STATION]);
*/
	switch(category_flag[STATION]){

	case UNFOUND:	return(0);	break;

	case IN_LOG:
		/*-------- READ FROM LOG FILE --------*/
		sscanf(line_buf, "%03d%02d%02d%02d%s %s %s %s %s %lf %lf %lf %lf",
			&doy_obslog, &hh_obslog,&mm_obslog, &ss_obslog, dum,
			log_station.code1,	log_station.code2,
			log_station.name,	log_station.type,
			&log_station.x,		&log_station.y,
			&log_station.z,		&log_station.offset);
		break;

	case IN_DRG:
		i = 0;								/* CLEAR ANTENNA COUNTER */
		while(1){
			/*--------- Next Chapter or End Of File ----------*/
			if((drg_chapter != STATION) || (drg_eof_flag == 1)) break;
			if(line_buf[0] != '*'){		/* Skip Comment Line */

				/*--------- Select Sub-Chapter ----------*/
				if(line_buf[0] == 'A'){				/* ANTENNA INFORMATION */
					drg_subchap = 1;
				} else if(line_buf[0] == 'P'){		/* STATION POSITION */
					drg_subchap = 2;
				} else if(line_buf[0] == 'T'){		/* ANTENNA INFORMATION */
					drg_subchap = 3;
				} else {
					drg_subchap = 0;
				}

				switch(drg_subchap){
				case 1:			/*--------- ANNTENNA INFORMATION ----------*/
					sscanf(line_buf,"%s %s %s %s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %s",
					dum,					drg_station[i].code1,
					drg_station[i].name,	drg_station[i].type,
					&drg_station[i].offset,	&fdum, &fdum, &fdum, &fdum,
					&fdum, &fdum, &fdum, &fdum, &fdum,
					drg_station[i].code2);

					i ++;
					num_station = i;
					break;


				case 2:			/*--------- STATION POSITION ----------*/
					sscanf(line_buf, "%s %s %s %lf %lf %lf %lf",
						dum, stn_code2, stn_name,
						&stn_x, &stn_y, &stn_z, &fdum);
					for(k=0; k < num_station; k++){
						if(strcmp(stn_name, drg_station[k].name) == 0){
							drg_station[k].x = stn_x;
							drg_station[k].y = stn_y;
							drg_station[k].z = stn_z;
							#ifdef DEBUG
							printf("STATION: %s %s %s %lf %lf %lf\n",
								drg_station[k].name,	drg_station[k].code1,
								drg_station[k].code2,	drg_station[k].x,
								drg_station[k].y,		drg_station[k].z);
							#endif
						}
					} /* for */
					break;
				}	/* switch */

			} /* IF */
			fxlog_detect_chapter();			/* Read 1-Line */
		}	/* while */
		break;
	}
/*
---------------------------------------------------- ENDING
*/
	printf("READ %d STATION INFORMATION\n", num_station);
	return(1);
}
