/*********************************************************
**	SCAN_SKED.C : Read Drudge File [PC-SCHED]			**
**														**
**	FUNCTION: Open Drudge File and Read.				**
**				Data File.								**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "drudge.inc"

long scan_sked( drg_fname, sked_ptr )

	char				*drg_fname;	/* INPUT : Pointer of DRG File Name */
	struct sked_line	**sked_ptr;	/* OUTPUT: Pointer of SKED */
{
	struct sked_line	*prev_sked_ptr;		/* Pointer of Previous List */ 
	struct sked_line	*new_sked_ptr;		/* Pointer of New List */
	FILE			*drg_file_ptr;	/* FILE Pointer of drudge file */
	long			sked_num;		/* Number of SCHEDULE */
	long			idum;			/* Dummy Integer */
	long			current_chapter;/* Current Chapter ID */
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

	current_chapter		= 0;		/* Chapter is not defined */
	sked_num			= 0;		/* Init sked_num */
	while(1){

		/*------------ Read 1-Line -------------*/
		if(fgets(line_buf, sizeof(line_buf), drg_file_ptr) == 0){
			break;
   		 }
		
		/*------------ DETECT CHAPTER -------------*/
		current_chapter = detect_chapter(current_chapter);

		if(	(current_chapter == SKED)		/* SEARCH in SKED Chap. */
			&& (line_buf[0] != '*')			/* SKIP COMMENT */
			&& (line_buf[0] != '$') ){		/* SKIP CHAPTER DEF. */

			/*-------- Allocate New Memory Area for Source List -------*/
			new_sked_ptr = (struct sked_line *)malloc(sizeof(struct sked_line));
			if( new_sked_ptr == NULL){
				printf("Memory Error in Reading SKED Information.\n");
				return(-1);
			} 

			/*-------- REMEMBER FIRST POINTER --------*/
			if(sked_num == 0){
				*sked_ptr = new_sked_ptr;
				prev_sked_ptr = new_sked_ptr;
			}

			/*--------- Format in SOURCE Chapter ----------*/
			sscanf(line_buf,
				"%s %d %s %s %02d%03d%02d%02d%02d %d %s %d %s %s",
				new_sked_ptr->comname,		&new_sked_ptr->cal,
				new_sked_ptr->freq,			new_sked_ptr->preob,
				&new_sked_ptr->year,		&new_sked_ptr->doy,
				&new_sked_ptr->hour,		&new_sked_ptr->minute,
				&new_sked_ptr->second,		&new_sked_ptr->duration,
				new_sked_ptr->midob,		&new_sked_ptr->idle,
				new_sked_ptr->postob,		new_sked_ptr->station);

			if( new_sked_ptr->year < 80 ){	new_sked_ptr->year += 2000;	}
									else {	new_sked_ptr->year += 1900;	}

			/*----------- Link for Next Source -----------*/
			prev_sked_ptr->next_sked_ptr= new_sked_ptr;
			new_sked_ptr->next_sked_ptr	= NULL;	/* SKED List Terminator */
			prev_sked_ptr				= new_sked_ptr;
			sked_num = sked_num + 1;

			#ifdef DEBUG
			printf("%-8s %d day %02d:%02d:%02d ", new_sked_ptr->comname,
				new_sked_ptr->doy,		new_sked_ptr->hour,
				new_sked_ptr->minute,	new_sked_ptr->second);
			printf("ADDRESS=%X\n", new_sked_ptr);
			#endif
		}
	}
/*
---------------------------------------------------- ENDING
*/
	fclose(drg_file_ptr);
	return(sked_num);
}
