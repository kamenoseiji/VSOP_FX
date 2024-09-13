/******************************************************
**	READ_MRG :	TEST MODULE FOR READING MERGE FORMAT **
******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "merge.inc"

int	read_merge( fname, time_num, header, source, stn_ptr, misc,
		time_ptr, vis_ptr, cl_ptr)
	char	*fname;					/* Merge File Name					*/
	int		time_num;				/* Number of Time Data				*/
	struct mrg_header	*header;	/* Header in Merge File				*/
	struct mrg_source	*source;	/* Source Information				*/
	struct mrg_misc		*misc;		/* Miscellaneous Information		*/
	int		*stn_ptr;				/* Pointer List of Station Format 2	*/
	int		*time_ptr;				/* Pointer of UT Data				*/
	struct mrg_vis2		*vis_ptr;	/* Pointer of Visibility Data		*/
	struct mrg_close	*cl_ptr;	/* Pointer of Closure Data			*/
{
	int		byte_len;
	int		ant_index;				/* Index of Antenna */
	int		bl_index;				/* Index of Baseline */
	int		cl_index;				/* Index of Closure */
	int		time_index;				/* Index of Time */
	int		history_index;			/* Index of History */
	int		index;					/* General Index					*/
	int		bl_id;					/* ID Number of Baseline */
	int		cl_id;					/* ID Number of Closure */
	int		ut;
	int		bl_num;					/* Total Number of Baseline			*/
	int		cl_num;					/* Total Number of Baseline			*/
	FILE	*mrg_file_ptr;			/* Merge File Pointer */
	char	history[80];			/* History */
	struct mrg_vis2		vis2;		/* Visivility in Format 1 */
	struct mrg_close	cls;		/* Closure Phase */
/*
------------------------------------------ OPEN MERGE FILE 
*/
	if( (mrg_file_ptr = fopen( fname, "r" )) == NULL){
		printf("Can't Open Merge File [%s] !!\n", fname);
		return(0);
	}

	/*-------- READ HEADER PART --------*/
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	fread( header, 1, byte_len, mrg_file_ptr);
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	if(header->format_version != 2){
		printf(" This File is not Format 2 !!\n");
		return(0);
	}
	bl_num = header->bl_num;
	cl_num = header->cl_num;

	/*-------- READ BASELINE ID --------*/
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	for(bl_index=0; bl_index<header->bl_num; bl_index++){
		fread( &bl_id, 1, sizeof(bl_id), mrg_file_ptr);
	}
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);

	/*-------- READ CLOSURE ID --------*/
	if(header->cl_num > 0){
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
		for(cl_index=0; cl_index<header->cl_num; cl_index++){
			fread( &cl_id, 1, sizeof(cl_id), mrg_file_ptr);
		}
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	}

	/*-------- READ HISTORY --------*/
	for(history_index=0; history_index<header->history_num; history_index++){
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
		fread( &history, 1, byte_len, mrg_file_ptr);
		history[sizeof(history)-1] = '\0';
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	}

	/*-------- READ SOURCE INFORMATION --------*/
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	fread( source, 1, byte_len, mrg_file_ptr);
	source->name[sizeof(source->name)-1] = '\0';
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);

	/*-------- READ STATION INFORMATION --------*/
	for(ant_index=0; ant_index< header->ant_num; ant_index++){
		stn_ptr[ant_index] = (int)malloc( sizeof(struct mrg_station2) );
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
		memset((struct mrg_station2 *)stn_ptr[ant_index], 0, byte_len);
		fread((struct mrg_station2 *)stn_ptr[ant_index],
				1, byte_len, mrg_file_ptr);
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	}

	/*-------- READ MISC INFORMATION --------*/
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	fread( misc, 1, byte_len, mrg_file_ptr);
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);

	/*-------- READ DATA --------*/
	for(time_index=0; time_index<time_num; time_index++){
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);

		/*-------- UT --------*/
		if( fread( &ut, 1, sizeof(ut), mrg_file_ptr) != sizeof(ut)){
			printf("Failed to Read Data.\n"); break; }
		if( ut < 0 ){ printf("Detect End of File.\n"); break; }
		time_ptr[time_index] = ut;

		/*-------- VISIBILITY --------*/
		for(bl_index=0; bl_index<header->bl_num; bl_index++){
			index = time_index* bl_num + bl_index;
			fread( &vis_ptr[index], 1, sizeof(struct mrg_vis2), mrg_file_ptr);
		}

		/*-------- CLOSURE --------*/
		for(cl_index=0; cl_index<header->cl_num; cl_index++){
			index = time_index* cl_num + cl_index;
			fread( &cl_ptr[index], 1, sizeof(struct mrg_close), mrg_file_ptr);
		}
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	}

	fclose(mrg_file_ptr);
	return(1);
}
