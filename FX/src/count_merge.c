/******************************************************
**	COUNT_MRG :	TEST MODULE FOR READING MERGE FORMAT **
******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "merge.inc"

int	count_merge( fname, antnum_ptr, blnum_ptr, clnum_ptr, timenum_ptr  )
	char	*fname;					/* Merge File Name					*/
	int		*antnum_ptr;			/* Pointer of Total Antenna Number	*/
	int		*blnum_ptr;				/* Pointer of Total Baseline Number	*/
	int		*clnum_ptr;				/* Pointer of Total Closure Number	*/
	int		*timenum_ptr;			/* Pointer of Data Record Number	*/
{
	struct mrg_header	header;		/* Header in Merge File				*/
	struct mrg_source	source;		/* Source Information				*/
	struct mrg_misc		misc;		/* Miscellaneous Information		*/
	struct mrg_station2	station;	/* Miscellaneous Information		*/
	struct mrg_vis2		vis2;		/* Visivility in Format 2			*/
	struct mrg_close	cls;		/* Closure Phase					*/

	long	byte_len;
	long	ant_index;				/* Index of Antenna */
	long	bl_index;				/* Index of Baseline */
	long	cl_index;				/* Index of Closure */
	long	history_index;			/* Index of History */
	long	time_index;				/* Index of Time	*/
	long	bl_id;					/* ID Number of Baseline */
	long	cl_id;					/* ID Number of Closure */
	long	ut;
	FILE	*mrg_file_ptr;			/* Merge File Pointer */
	char	history[80];			/* History */
/*
------------------------------------------ OPEN MERGE FILE 
*/
	if( (mrg_file_ptr = fopen( fname, "r" )) == NULL){
		printf("Can't Open Merge File [%s] !!\n", fname);
		return(0);
	}

	/*-------- READ HEADER PART --------*/
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	fread( &header, 1, byte_len, mrg_file_ptr);
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	if(header.format_version != 2){
		printf(" This File is not Format 2 !!\n");
		return(0);
	}

	/*-------- READ BASELINE ID --------*/
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	for(bl_index=0; bl_index<header.bl_num; bl_index++){
		fread( &bl_id, 1, sizeof(bl_id), mrg_file_ptr);
	}
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);

	/*-------- READ CLOSURE ID --------*/
	if(header.cl_num > 0){
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);

		for(cl_index=0; cl_index<header.cl_num; cl_index++){
			fread( &cl_id, 1, sizeof(cl_id), mrg_file_ptr);
		}
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	}

	/*-------- READ HISTORY --------*/
	for(history_index=0; history_index<header.history_num; history_index++){
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
		fread( &history, 1, byte_len, mrg_file_ptr);
		history[sizeof(history)-1] = '\0';
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	}

	/*-------- READ SOURCE INFORMATION --------*/
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	fread( &source, 1, byte_len, mrg_file_ptr);
	source.name[sizeof(source.name)-1] = '\0';
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);


	/*-------- READ STATION INFORMATION --------*/
	for(ant_index=0; ant_index< header.ant_num; ant_index++){
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
		fread( &station, 1, byte_len, mrg_file_ptr);
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	}

	/*-------- READ MISC INFORMATION --------*/
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	fread( &misc, 1, byte_len, mrg_file_ptr);
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);

	/*-------- READ DATA --------*/
	time_index = 0;
	while(1){
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);

		if( fread( &ut, 1, sizeof(ut), mrg_file_ptr) != sizeof(ut)){
			printf("Failed to Read Data.\n"); break;
		}

		if( ut < 0 ){
			printf("Detect End of File.\n"); break;
		}


		for(bl_index=0; bl_index<header.bl_num; bl_index++){
			fread( &vis2, 1, sizeof(vis2), mrg_file_ptr);
		}
		for(cl_index=0; cl_index<header.cl_num; cl_index++){
			fread( &cls, 1, sizeof(cls), mrg_file_ptr);
		}

		time_index ++;
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	}
	fclose(mrg_file_ptr);

	*antnum_ptr = header.ant_num;
	*blnum_ptr  = header.bl_num;
	*clnum_ptr  = header.cl_num;
	*timenum_ptr= time_index;

	return(time_index);
}
