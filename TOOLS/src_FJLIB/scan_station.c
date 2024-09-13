/*********************************************************
**	SCAN_STATION.C : Read Drudge File [PC-SCHED]		**
**														**
**	FUNCTION: Open Drudge File and Read.				**
**				Data File.								**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "drudge.inc"

long scan_station( drg_fname, stn_ptr )

	char				*drg_fname;	/* INPUT : Pointer of DRG File Name */
	struct station_line	**stn_ptr;	/* OUTPUT: Pointer of Source */
{
	struct station_line	*prev_stn_ptr;
	struct station_line	*new_stn_ptr;
	struct station_line	*current_ptr;

	FILE			*drg_file_ptr;	/* FILE Pointer of drudge file */
	long			station_num;	/* Number of Source */
	char			dum[4];			/* Dummy String */
	float			fdum;			/* Dummy Integer */
	long			current_chapter;/* Current Chapter ID */
	char			stn_name[9];	/* STATION NAME */
	double			stn_x;			/* STATION X-Position */
	double			stn_y;			/* STATION Y-Position */
	double			stn_z;			/* STATION Z-Position */

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
	station_num			= 0;		/* Init station_num */
	while(1){

		/*------------ Read 1-Line -------------*/
		if(fgets(line_buf, sizeof(line_buf), drg_file_ptr) == 0){
			break;
   		 }
		
		/*------------ DETECT CHAPTER -------------*/
		current_chapter = detect_chapter(current_chapter);

		if(	current_chapter == STATION ){	/* SEARCH in SOURCE Chap. */

			switch(line_buf[0]){
			case '*':	/* COMMENT LINE */		break;
			case '$':	/* CHAPTER DEF */		break;

			case 'A':	/*-------- ANTENNA INFORMATION --------*/
				/*-------- Allocate New Memory Area for STATION List -------*/
				new_stn_ptr = (struct station_line *)
							malloc(sizeof(struct station_line));
				if( new_stn_ptr == NULL){
					printf("Memory Error in Reading STATION Information.\n");
					return(-1);
				} 

				/*-------- REMEMBER FIRST POINTER --------*/
				if(station_num == 0){
					*stn_ptr	= new_stn_ptr;
					prev_stn_ptr= new_stn_ptr;
				}

				/*--------- Format in STATION Chapter ----------*/
				sscanf(line_buf,"%s %s %s %s %f %f %f %f %f %f %f %f %f %f %s",
				dum,					new_stn_ptr->code1,
				new_stn_ptr->name,		new_stn_ptr->type,
				&new_stn_ptr->offset,
				&fdum, &fdum, &fdum, &fdum, &fdum, &fdum, &fdum, &fdum, &fdum,
				new_stn_ptr->code2);

				/*----------- Link for Next Source -----------*/
				prev_stn_ptr->next_stn_ptr	= new_stn_ptr;
				new_stn_ptr->next_stn_ptr	= NULL;	/* SOURCE List Terminator */
				prev_stn_ptr				= new_stn_ptr;
				station_num = station_num + 1;
				break;

			case 'P':	/*-------- STATION POSITION --------*/
				sscanf(line_buf, "%s %s %s %lf %lf %lf", 
					dum,		dum,		stn_name,
					&stn_x,		&stn_y,		&stn_z);

				/*-------- SEARCH BY STATION NAME --------*/
				current_ptr = *stn_ptr;			/* TOP OF THE STN LIST */
				while(current_ptr != NULL){
					if( strcmp(current_ptr->name, stn_name) == 0){
						current_ptr->x	= stn_x;
						current_ptr->y	= stn_y;
						current_ptr->z	= stn_z;
						break;
					}
					current_ptr = current_ptr->next_stn_ptr;
				}
				break;

			case 'T':	/*-------- TERMINAL INFORMATION --------*/
				break;
			}
		}
	}
/*
---------------------------------------------------- ENDING
*/
	fclose(drg_file_ptr);
	return(station_num);
}
