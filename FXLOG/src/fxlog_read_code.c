/*********************************************************
**	FXLOG_READ_CODE.C : Read Frequency Information.		**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"
fxlog_read_code()
{
	double	fsky;					/* SKY Frequency [MHz]*/
	double	flocal;					/* Local Frequency [MHz]*/
	double	pcalf;					/* Pcal Frequency [kHz]*/
	double	bw;						/* BBC Bandwidth [MHz]*/
	char	stn_code[3];			/* Station ID */
	char	previous_stn[3];		/* Previous Station ID */
	char	code1[2];				/* Frequency Table Name */
	char	code2[3];				/* IF Name */
	char	sg_namelist[16][2];		/* Sub-Group Name List	*/
	long	subchap;				/* Sub-Chapter Number */
	long	lo_cntr;
/*
---------------------------------------------------- READ CODE CHAP. 
*/
	ftable_cntr = 0;				/* Counter for Frequency Table  */
	if_cntr = 0;					/* Counter for IF Number  */
	bbc_cntr = 0;					/* Counter for Base Band Convertor  */
	lo_cntr = 0;					/* LO Number for Each Station */

	while(1){
		/*--------- Next Chapter or End Of File ----------*/
		if((drg_chapter != CODE) || (drg_eof_flag == 1)) break;

		if(line_buf[0] != '*'){		/* Skip Comment Line */

			/*--------- Select Sub-Chapter ----------*/
			switch(line_buf[0]){

			case 'F':	subchap = 1;			/* F-BAND TABLE NAME */
						if_cntr = 0;			/* Initialize IF Counter */
						bbc_cntr = 0;			/* Initialize BBC Counter */
						break;

			case 'C':	subchap = 2;			/* RF FREQUENCY */
						break;

			case 'L':	subchap = 3;			/* LOCAL FREQUENCY */
						break;

			case '$':	subchap = 0;			/* LOCAL FREQUENCY */
						break;

			default:	subchap = 0;
						printf("Warnig : Undefined ID '%c' in CODE Chapter !\n",
							line_buf[0]); 
						break;
			}

			switch(subchap){
			case 1:			/*--------- F-BAND TABLE NAME ----------*/
				sscanf(line_buf,"%s %s %s %d %d",
					dum,					/* Sub-Chapter ID */
					drg_freq[ftable_cntr].name,		/* F-BAND TABLE NAME */
					drg_freq[ftable_cntr].code,		/* 2-CHARACTER TABLE NAME */
					&drg_freq[ftable_cntr].num_sg,	/* Number of Sub-Group */
					&drg_freq[ftable_cntr].total_bbc);/* Total Number of BBC */
				ftable_cntr = ftable_cntr + 1;	/* Count up Number of Ftable */
				break;

			case 2:			/*--------- RF FREQUENCY ----------*/
				sscanf(line_buf,"%s %s %s %lf %lf %s %s %lf",
					dum,			/* Sub-Chapter ID */
					code2,			/* Freq TABLE NAME */
					code1,			/* Sub-Group Name */
					&fsky,			/* RF FREQUENCY */
					&pcalf,			/* P-CAL FREQUENCY */
					dum,			/* BBC Number */
					dum,			/* MKIII MODE (A, B, C ...) */
					&bw);			/* Bandwidth */

				/*-------- ANALYZE FREQUENCY TABLE --------*/
				if(strcmp(code2, drg_freq[ftable_cntr - 1].code) == 0){
#ifdef HIDOI


					if(strcmp(drg_freq[ftable_cntr-1].sg_name[if_cntr-1],
								code1)!= 0){
						/*-------- NEW IF --------*/
						strcpy(drg_freq[ftable_cntr-1].sg_name[if_cntr], code1);
						if_cntr = if_cntr + 1;
						drg_freq[ftable_cntr-1].num_sg = if_cntr;
						bbc_cntr= 0;
					}
#endif

					/*-------- READ BBC INFORMATION --------*/
					strcpy(sg_namelist[bbc_cntr], code1);
					drg_freq[ftable_cntr-1].sky_freq[bbc_cntr]	= fsky;
					drg_freq[ftable_cntr-1].bbc_bw[bbc_cntr]	= bw;
					drg_freq[ftable_cntr-1].pcal_freq[bbc_cntr]	= pcalf;
					bbc_cntr++;

					drg_freq[ftable_cntr-1].num_bbc[if_cntr-1] = bbc_cntr;

				} else {
					/*-------- FREQ. TABLE LINES ARE INCONSISTENT --------*/ 
					printf("Error! : Different Frequency Table Code.\n");
					return(0);
				}
				break;

			case 3:			/*--------- LOCAL FREQUENCY ----------*/
				sscanf(line_buf,"%s %s %s %s %s %lf",
					dum,		/* Sub-Chapter ID */
					stn_code,	/* Station 1-Char ID */
					code2,		/* Freq TABLE NAME */
					code1,		/* Sub-Group Name */
					dum,		/* Sub-Group ID */
					&flocal);	/* LOCAL FREQUENCY */
				#ifdef DEBUG
				printf("%s %s %s %lf\n", stn_code, code2, code1, flocal);
				#endif

				/*--------- NEW STATION or not ---------*/
				if( strcmp(stn_code, previous_stn) != 0){
					lo_cntr=0;		/* New Station -> Clear IF Counter */
				}
				strcpy( previous_stn, stn_code);

				for(i=0; i< num_station; i++){
					if( strcmp(stn_code, drg_station[i].code1) == 0){
						drg_station[i].flocal[lo_cntr] = flocal;
						lo_cntr++;		/* Count up IF Number */
						drg_station[i].if_num = lo_cntr;
						#ifdef DEBUG
						printf("STATION %s: LOCAL= %8.2f MHz\n",
							stn_code, flocal);
						#endif
					}
				}
				break;
			}	/* SUB-CHAPTER */
		}	/* Not Comment */
		fxlog_detect_chapter();			/* Read 1-Line */
	}	/* while */
/*
---------------------------------------------------- LINK SG -> BBC
*/



/*
---------------------------------------------------- REPORT SUMMERY
*/
	#ifdef DEBUG
	printf("TOTAL NUMBER OF FREQ. TABLE : %d\n", ftable_cntr);

	/*-------- FREQ TABLE LOOP --------*/
	for(i=0; i< ftable_cntr; i++){
		printf("  FOR FREQ TABLE %d........", i+1);
		printf("  %d SUB-GROUP (IF) INCLUDED\n", drg_freq[i].num_sg);

		/*-------- SUB-GROUP (IF) LOOP --------*/
		for(j=0; j< drg_freq[i].num_sg; j++){
			printf("    FOR SUB-GROUP %d........", j+1);
			printf("    CONTAINS %d Base Band Converters.\n",
					drg_freq[i].num_bbc[j]);

			/*-------- BBC LOOP --------*/
			for(k=0; k< drg_freq[i].num_bbc[j]; k++){
				printf("      BBC %2d: RF=  %8.2f MHz\n",
						k+1, drg_freq[i].sky_freq[j][k]);
			}
		}
	}

	/*-------- STATOIN LOOP --------*/
	for( i=0; i< num_station; i++){
		printf("FOR STATION %s........", drg_station[i].name);
		printf("%d IFs FOUND.\n", drg_station[i].if_num);

		/*-------- IF LOOP --------*/
		for( j=0; j< drg_station[i].if_num; j++){
			printf("  LOCAL FREQUENCY FOR IF %d....%8.2f MHz\n",
					j, drg_station[i].flocal[j]);
		}
	}
	
	#endif DEBUG
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
