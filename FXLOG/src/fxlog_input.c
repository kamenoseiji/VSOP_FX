/*************************************************************
**	FXLOG_INPUT.C :	Require to input important information  **
**					for Log File for FX Correlator			**
**															**
**	AUTHOR	: KAMENO Seiji									**
**	CREATED	: 1995/11/26									**
**************************************************************/

#include "fxlog.inc"

long fxlog_input()
{
	char	tty_input[256];							/* Input from Keyboard */
	char	stn_code[2];							/* STATION CODE */
/*
--------------------------------------------- INPUT for EXPER
*/
	if( category_flag[EXPER] == UNFOUND){			/* OBS YEAR lacks */
		printf("INPUT observation YEAR and DOY (YYDDD)---> ");
		fgets(tty_input, sizeof(tty_input), stdin);	/* INPUT From Keyboard */
		tty_input[strlen(tty_input)-1] = '\0';		/* Delete CR CODE*/
		fxlog_write_exper(tty_input);
/*
		sscanf(tty_input, "%d", &tty_exper.year);
		fxlog_write_exper(tty_exper);
*/
	}
/*
--------------------------------------------- INPUT for STATION
*/
	while(category_flag[STATION] == IN_DRG){		/* STATION INFO lacks */

		/*-------- MESSAGE FOR INPUT --------*/
		printf("No Station ID Found in LOG .... Please Select.\n");
		for( i=0; i< num_station; i++){
			printf(" %s : %8s \n", drg_station[i].code1, drg_station[i].name);
		}
		printf("INPUT STATION CODE [");
		for( i=0; i< num_station-1; i++){
			printf(" %s /", drg_station[i].code1);
		}
		printf(" %s ]------> ", drg_station[num_station-1].code1);

		/*-------- TTY INPUT --------*/
		fgets(tty_input, sizeof(tty_input), stdin);	/* Keyboard INPUT*/
		tty_input[strlen(tty_input)-1] = '\0';		/* Delete CR CODE*/
		sscanf(tty_input, "%s", tty_station.code1);

		/*-------- IS SELECTED CODE CORRECT ? --------*/
		for( i=0; i< num_station; i++){
			if(strcmp(tty_station.code1, drg_station[i].code1) == 0){
				#ifdef DEBUG
				printf("STATION %s is Selected!\n", drg_station[i].name);
				#endif
				stn_id = i;
			}
			category_flag[STATION] = IN_TTY;
			strcpy(tty_station.code2,	drg_station[stn_id].code2);
			strcpy(tty_station.name,	drg_station[stn_id].name);
			strcpy(tty_station.type,	drg_station[stn_id].type);
			tty_station.x		= drg_station[stn_id].x;
			tty_station.y		= drg_station[stn_id].y;
			tty_station.z		= drg_station[stn_id].z;
			tty_station.offset	= drg_station[stn_id].offset;
			tty_station.if_num	= drg_station[stn_id].if_num;
			for(j=0; j<tty_station.if_num; j++){
				tty_station.flocal[j] = drg_station[stn_id].flocal[j];
			}
		}
		fxlog_write_station(tty_station);
	} /* while */
/*
--------------------------------------------- LO Setting
*/

	/*-------- CHECK FOR FREQUENCY CODE --------*/
	if(category_flag[LO] == UNFOUND){

		/*-------- IF NUMBER OF IF IS INCORRECT --------*/ 
		if(tty_station.if_num < 1 ){
			tty_station.if_num = drg_freq[0].num_sg;

			/*-------- IF DRG DO NOT CONTAIN LO FREQ. INFO --------*/
			for(i=0; i<tty_station.if_num; i++){
				tty_station.flocal[i] = drg_freq[0].sky_freq[i] - 200.0;
				printf("Warning: No LO Found...Assume %8.2f MHz for IF %d.\n",
					tty_station.flocal[i], i+1);

			}
		}
		/*-------- WRITE FOR LO --------*/
		category_flag[LO] = IN_DRG;
		for(if_cntr=0; if_cntr < tty_station.if_num; if_cntr++){
			fxlog_write_lo(tty_station);
		}

		/*-------- INFORMATION FOR BBC --------*/
		category_flag[BBC] = IN_DRG;
		drg_freq[0].total_bbc = 0;
		for(if_cntr=0; if_cntr < tty_station.if_num; if_cntr++){
			for(bbc_cntr=0; bbc_cntr<drg_freq[0].num_bbc[if_cntr]; bbc_cntr++){
				drg_freq[0].total_bbc	= drg_freq[0].total_bbc + 1;
				drg_bbc.lo_num	= if_cntr + 1;
				drg_bbc.bbc_num	= drg_freq[0].total_bbc;
				drg_bbc.freq	= drg_freq[0].sky_freq[if_cntr]
								- tty_station.flocal[if_cntr];
				fxlog_write_bbc(drg_bbc);
			}
		}

		/*-------- INFORMATION FOR FORMATTER --------*/
		category_flag[FORM] = IN_DRG;
		form_cntr	= 0;
		for(if_cntr=0; if_cntr < tty_station.if_num; if_cntr++){
			for(bbc_cntr=0; bbc_cntr<drg_freq[0].num_bbc[if_cntr]; bbc_cntr++){
				form_cntr			= form_cntr + 1;
				drg_form.bbc_num	= bbc_cntr + 1;
				drg_form.form_num	= form_cntr;
				sprintf(drg_form.side, "USB");
				drg_form.rate		= 2000.0	/* Nyquist Sample, MHz -> kHz */
									* drg_freq[0].bbc_bw[if_cntr];
				drg_form.bit		= 1;
				fxlog_write_form(drg_form);
			}
		}
	}
	return(0);
}
