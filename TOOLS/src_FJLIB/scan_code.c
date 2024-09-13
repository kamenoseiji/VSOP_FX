/*********************************************************
**	SCAN_CODE.C : Read Drudge File [PC-SCHED]			**
**														**
**	FUNCTION: Open Drudge File and Read.				**
**				Data File.								**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "drudge.inc"
#define CODE_F_FMT	"%*s %s %s %d %d"

int scan_code( drg_fname, code1, first_code_ptr )

	char			*drg_fname;	/* INPUT : Pointer of DRG File Name */
	char			*code1;		/* INPUT : 1-Letter Station Code	*/
	struct code_line	**first_code_ptr;	/* OUTPUT: First Pointer of CODE*/
{
	struct code_line	*prev_code_ptr;
	struct code_line	*new_code_ptr;

	FILE	*drg_file_ptr;		/* FILE Pointer of drudge file	*/
	int		current_chapter;	/* Current Chapter ID			*/
	int		code_num;			/* Number of CODE				*/
	int		if_num;				/* Number of IF					*/
	int		if_index;			/* Index Number of IF			*/
	double	local_freq;			/* Local Frequency of Sub-Group	*/
	char	mode[3];			/* Abbr. 2-Letter Code Name		*/
	char	sg_id[2];			/* 1-Letter Sub-Group Code Name	*/
	char	stn_code[2];		/* 1-Letter Station Code		*/
/*
---------------------------------------------------- OPEN FILE
*/
	if( (drg_file_ptr = fopen(drg_fname, "r")) == 0 ){
		printf("Can't open Drudge file [%s]. \n", drg_fname);
		return(-1);
    }
/*
---------------------------------------------------- READ FILE
*/
	current_chapter		= 0;		/* Chapter is not defined	*/
	code_num			= 0;		/* Number of Freq. CODE Sets */
	while(1){

		/*------------ Read 1-Line -------------*/
		if(fgets(line_buf, sizeof(line_buf), drg_file_ptr) == 0){
			break;
   		 }
		
		/*------------ DETECT CHAPTER -------------*/
		current_chapter = detect_chapter(current_chapter);
		if(	current_chapter == CODE ){		/* SEARCH in CODE Chap. */

			switch(line_buf[0]){
			case '*':	/* COMMENT 		*/		break;
			case '$':	/* CHAPTER DEF	*/		break;

			case 'F':	/*-------- CODE ID Table INFORMATION --------*/
				/*-------- Allocate New Memory Area for ID List -------*/
				new_code_ptr = (struct code_line *)
							malloc(sizeof(struct code_line));
				if( new_code_ptr == NULL){
					printf("Memory Error in Reading CODE Information.\n");
					return(-1);
				} 

				/*-------- REMEMBER FIRST POINTER --------*/
				if(code_num == 0){
					*first_code_ptr	= new_code_ptr;
					prev_code_ptr	= new_code_ptr;
				}	

				/*--------- Format in CODE Chapter ----------*/
				sscanf(line_buf, CODE_F_FMT,
				new_code_ptr->name,		new_code_ptr->mode,
				&new_code_ptr->n_sg,	&new_code_ptr->n_ch );

				/*----------- Link for Next Source -----------*/
				prev_code_ptr->next_code_ptr= new_code_ptr;
				new_code_ptr->next_code_ptr	= NULL;		/* TERMINATOR */
				prev_code_ptr				= new_code_ptr;
				code_num ++;	if_num = 0;
				break;

			case 'C':	/*-------- RF FREQUENCY --------*/

				/*--------- Format in CODE Chapter ----------*/
				sscanf(line_buf,"%*s %s %s %lf %*f %*d %*s %lf",
				mode,						new_code_ptr->sg_id[if_num],
				&new_code_ptr->rf[if_num],	&new_code_ptr->bw[if_num]);

				if( strcmp(new_code_ptr->mode, mode) != NULL){
					printf(" WARNINIG: FREQUENCY CODE is INCONSISTENT!!\n");
					printf(" CODE MODE = %s, MODE = %s\n",
						new_code_ptr->mode, mode);
					break;
				}

				if_num ++;
				break;

			case 'L':	/*-------- LOCAL INFORMATION --------*/

				/*--------- Format in LO Chapter ----------*/
				sscanf(line_buf,"%*s %s %s %s %*s %lf",
				stn_code,	mode,	sg_id,	&local_freq);

				if( strcmp(new_code_ptr->mode, mode) != NULL){
					printf(" WARNINIG: FREQUENCY CODE is INCONSISTENT!!\n");
					printf(" CODE MODE = %s, MODE = %s\n",
						new_code_ptr->mode, mode);
					break;
				}

				/*-------- SKIP UNINTEREST STATION --------*/
				if( code1[0] != stn_code[0] ){ break; }

				/*--------- Scan LO Frequency ----------*/
				for( if_index=0; if_index<new_code_ptr->n_ch; if_index++){
					if( new_code_ptr->sg_id[if_index][0] == sg_id[0] ){
						new_code_ptr->lo_freq[if_index] = local_freq;
					}
				}
				break;
			}	/*-------- End of Case 'L'	*/
		} /*-------- End of CODE Chapter Selection */
	} /*-------- End of 1-Line Read in the File */
/*
---------------------------------------------------- ENDING
*/
	fclose(drg_file_ptr);
	return(code_num);
}
