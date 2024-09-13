/*********************************************************
**	SCAN_SOURCE.C : Read Drudge File [PC-SCHED]			**
**														**
**	FUNCTION: Open Drudge File and Read.				**
**				Data File.								**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "drudge.inc"

long scan_source( drg_fname, src_ptr )

	char				*drg_fname;	/* INPUT : Pointer of DRG File Name */
	struct source_line	**src_ptr;	/* OUTPUT: Pointer of Source */
{
	struct source_line	*prev_src_ptr;
	struct source_line	*new_src_ptr;
	FILE			*drg_file_ptr;	/* FILE Pointer of drudge file */
	long			source_num;		/* Number of Source */
	char			sourcedum[4];	/* +DD */
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
	source_num			= 0;		/* Init source_num */
	while(1){

		/*------------ Read 1-Line -------------*/
		if(fgets(line_buf, sizeof(line_buf), drg_file_ptr) == 0){
			break;
   		 }
		
		/*------------ DETECT CHAPTER -------------*/
		current_chapter = detect_chapter(current_chapter);

		if(	(current_chapter == SOURCE)		/* SEARCH in SOURCE Chap. */
			&& (line_buf[0] != '*')			/* SKIP COMMENT */
			&& (line_buf[0] != '$') ){		/* SKIP CHAPTER DEF. */

			/*-------- Allocate New Memory Area for Source List -------*/
			new_src_ptr = (struct source_line *)
							malloc(sizeof(struct source_line));
			if( new_src_ptr == NULL){
				printf("Memory Error in Reading SOURCE Information.\n");
				return(-1);
			} 
			/*-------- REMEMBER FIRST POINTER --------*/
			if(source_num == 0){
				*src_ptr	= new_src_ptr;
				prev_src_ptr= new_src_ptr;
			}

			/*--------- Format in SOURCE Chapter ----------*/
			sscanf(line_buf, "%s %s %d %d %f %s %d %f %f %d %d %d %d %f",
				new_src_ptr->iauname,	new_src_ptr->comname,
				&new_src_ptr->rh,		&new_src_ptr->rm,
				&new_src_ptr->rs,		sourcedum,
				&new_src_ptr->dm,		&new_src_ptr->ds,
				&new_src_ptr->epoch,
				&idum, &idum, &idum, &idum,
				&new_src_ptr->vlsr);

			/*-------- OPERATION FOR SIGN of DD --------*/
			if(sourcedum[0] == '-'){ new_src_ptr->sign =-1;}
			else{ new_src_ptr->sign = 1; }
			sscanf(sourcedum, "%d", &new_src_ptr->dd);
			new_src_ptr->dd = abs(new_src_ptr->dd);

			/*----------- If Common Name is not Defined -----------*/
			if( new_src_ptr->comname[0] == '$' ){
				strcpy(new_src_ptr->comname, new_src_ptr->iauname);
			}

			/*----------- Link for Next Source -----------*/
			prev_src_ptr->next_src_ptr	= new_src_ptr;
			new_src_ptr->next_src_ptr	= NULL;		/* SOURCE List Terminator */
			prev_src_ptr				= new_src_ptr;
			source_num = source_num + 1;

			#ifdef DEBUG
			printf("%-8s %-8s %f ", new_src_ptr->comname,
				new_src_ptr->iauname, new_src_ptr->vlsr);
			printf("ADDRESS=%X\n", new_src_ptr);
			#endif
		}
	}
/*
---------------------------------------------------- ENDING
*/
	fclose(drg_file_ptr);
	return(source_num);
}
