/******************************************************
**	READ_MRG :	TEST MODULE FOR READING MERGE FORMAT **
******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "merge.inc"

main(argc, argv)
	long	argc;		/* Number of Arguments */
	char	**argv;		/* Pointer of Arguments */
{
	long	byte_len;
	long	ant_index;				/* Index of Antenna */
	long	bl_index;				/* Index of Baseline */
	long	cl_index;				/* Index of Closure */
	long	history_index;			/* Index of History */
	long	bl_id;					/* ID Number of Baseline */
	long	cl_id;					/* ID Number of Closure */
	long	ut;
	FILE	*mrg_file_ptr;			/* Merge File Pointer */
	char	history[80];			/* History */
	struct mrg_header	header;		/* Header in Merge File */
	struct mrg_source	source;		/* Source Information */
	struct mrg_station1	stn_1;		/* Station Format 1 */
	struct mrg_station2	stn_2;		/* Station Format 2 */
	struct mrg_misc		misc;		/* Miscellaneous Information */
	struct mrg_vis1		vis1;		/* Visivility in Format 1 */
	struct mrg_vis2		vis2;		/* Visivility in Format 1 */
	struct mrg_close	cls;		/* Closure Phase */
/*
------------------------------------------ CHECK FOR ARGUMENTS 
*/
	if(argc < 2){
		printf("USAGE : read_mrg [merge file] !!\n");
		exit(0);
	}
/*
------------------------------------------ OPEN MERGE FILE 
*/
	if( (mrg_file_ptr = fopen( argv[1], "r" )) == NULL){
		printf("Can't Open Merge File [%s] !!\n", argv[1]);
		exit(0);
	}

	/*-------- READ HEADER PART --------*/
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	printf("BYTE LENGTH    = %d\n", byte_len);

	fread( &header, 1, byte_len, mrg_file_ptr);
	printf("FORMAT VERSION = %d\n", header.format_version);
	printf("HISTORY NUM    = %d\n", header.history_num);
	printf("HEADER NUM     = %d\n", header.header_num);
	printf("ANTENNA NUM    = %d\n", header.ant_num);
	printf("BASELINE NUM   = %d\n", header.bl_num);
	printf("CLOSURE NUM    = %d\n", header.cl_num);
	printf("RECORD LENGTH  = %d\n", header.record_len);
	printf("CAL FLAG       = %d\n", header.cal_flag);
	printf("STOKES         = %d\n", header.stokes_index);

	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	printf("BYTE LENGTH    = %d\n", byte_len);

	/*-------- READ BASELINE ID --------*/
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	printf("BYTE LENGTH    = %d\n", byte_len);
	for(bl_index=0; bl_index<header.bl_num; bl_index++){
		fread( &bl_id, 1, sizeof(bl_id), mrg_file_ptr);
		printf("BL_NAME        = %d\n", bl_id);
	}
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	printf("BYTE LENGTH    = %d\n", byte_len);

	/*-------- READ CLOSURE ID --------*/
	if(header.cl_num > 0){
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
		printf("BYTE LENGTH    = %d\n", byte_len);

		for(cl_index=0; cl_index<header.cl_num; cl_index++){
			fread( &cl_id, 1, sizeof(cl_id), mrg_file_ptr);
			printf("CL_NAME        = %d\n", cl_id);
		}

		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
		printf("BYTE LENGTH    = %d\n", byte_len);
	}

	/*-------- READ HISTORY --------*/
	for(history_index=0; history_index<header.history_num; history_index++){
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
		printf("BYTE LENGTH    = %d\n", byte_len);
		fread( &history, 1, byte_len, mrg_file_ptr);
		history[sizeof(history)-1] = '\0';
		printf("HISTORY : %s\n", history);
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
		printf("BYTE LENGTH    = %d\n", byte_len);
	}

	/*-------- READ SOURCE INFORMATION --------*/
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	printf("BYTE LENGTH    = %d\n", byte_len);
	fread( &source, 1, byte_len, mrg_file_ptr);

	source.name[sizeof(source.name)-1] = '\0';
	printf("SOURCE NAME    = %s\n", source.name);
	printf(" R.A. DEC. [B1950]    = %lf  %lf\n", source.ra_1950, source.dec_1950);
	printf(" R.A. DEC. [APPARENT] = %lf  %lf\n", source.ra_app, source.dec_app);
	printf(" TOTAL FLUX DENSITY   = %lf [JY]\n", source.total_flux);

	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	printf("BYTE LENGTH    = %d\n", byte_len);

	/*-------- READ STATION INFORMATION --------*/
	switch(header.format_version){
	case 1:
		for(ant_index=0; ant_index< header.ant_num; ant_index++){
			fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
			printf("BYTE LENGTH    = %d\n", byte_len);
			fread( &stn_1, 1, byte_len, mrg_file_ptr);
			stn_1.name[sizeof(stn_1.name)-1] = '\0';
			printf("STATION        = %s\n", stn_1.name);
			printf(" X             = %lf\n", stn_1.x);
			printf(" Y             = %lf\n", stn_1.y);
			printf(" Z             = %lf\n", stn_1.z);
			fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
			printf("BYTE LENGTH    = %d\n", byte_len);
		}
		break;
	case 2:
		for(ant_index=0; ant_index< header.ant_num; ant_index++){
			fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
			printf("BYTE LENGTH    = %d\n", byte_len);
			fread( &stn_2, 1, byte_len, mrg_file_ptr);
			fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
			printf("BYTE LENGTH    = %d\n", byte_len);
		}
		break;
	}

	/*-------- READ MISC INFORMATION --------*/
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	printf("BYTE LENGTH    = %d\n", byte_len);
	fread( &misc, 1, byte_len, mrg_file_ptr);
	printf("YEAR           = %d\n", misc.year);
	printf("GST  = %lf at UT = %d\n",misc.gst, misc.ut);
	printf("FREQENCY       = %lf\n", misc.rf);
	printf("BANDWIDTH      = %lf\n", misc.bw);
	printf("INTEG [COH]    = %lf\n", misc.coh_integ);
	printf("INTEG [INCOH]  = %lf\n", misc.inc_integ);
	fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	printf("BYTE LENGTH    = %d\n", byte_len);

	/*-------- READ DATA --------*/
	while(1){
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
		printf("BYTE LENGTH    = %d\n", byte_len);
		if( fread( &ut, 1, sizeof(ut), mrg_file_ptr) != sizeof(ut)){
			printf("Failed to Read Data.\n");
			break;
		}
		printf(" UT = %d\n", ut);
		for(bl_index=0; bl_index<header.bl_num; bl_index++){
			fread( &vis1, 1, sizeof(vis1), mrg_file_ptr);
		}
		for(cl_index=0; cl_index<header.cl_num; cl_index++){
			fread( &cls, 1, sizeof(cls), mrg_file_ptr);
			printf("CLOSURE PHASE = %f\n", cls.phs);
		}
		fread( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
		printf("BYTE LENGTH    = %d\n", byte_len);
	}
	fclose(mrg_file_ptr);
	return(0);
}
