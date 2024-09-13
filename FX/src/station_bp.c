/*********************************************************
**	STATION_BP.C : Make Band-Pass Table using CFS Data	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"

#define	MAX_SS	32							/* Maximum Number of SS		*/

MAIN__(
	int		argc,			/* Number of Arguments 		*/
	char	**argv,			/* Pointer to Arguments 	*/
	char	**envp)			/* Pointer to Environments	*/
{
	struct	head_stn	*stn_ptr;			/* Pointer of Station Header */
	struct	head_obj	*obj_ptr;			/* Pointer of Object Header */
	int					stn_id;				/* Station ID */
	int					stn_num;			/* Total Number of Stations */
	int					obj_num;			/* Total Number of Objects */
	int					ssnum;				/* Number of Sub-Stream */
	int					integ_pp;			/* Integrated PP Number */
	int					position;			/* Start PP Position	*/
	double				start_mjd;			/* Start Time [MJD] */
	double				stop_mjd;			/* Stop Time [MJD] */

	key_t		obj_key;			/* Keyword of Shared Memory */
	key_t		stn_key;			/* Keyword of Shared Memory */
	int			shrd_obj_id;		/* Shared Memory ID */
	int			shrd_stn_id;		/* Shared Memory ID */
	int			ss_index;			/* Index for Sub-Stream */
	double		rf[MAX_SS];			/* RF [MHz] for Each Sub-Stream */
	double		freq_incr[MAX_SS];	/* Freq. Increment for Each SS */
	int			freq_num[MAX_SS];	/* Freq. Channel Number in Each SS */
	int			time_num;			/* Total Number of PP in CFS */
	double		time_incr;			/* Increment for Time [sec] */
	float		*work_ptr;			/* Pointer for Work Area */
	double		*vis_r_ptr[MAX_SS];	/* Pointer of Vis. for BL and SS */
	double		*vis_i_ptr[MAX_SS];	/* Pointer of Vis. for BL and SS */
	double		vis_max[MAX_SS];	/* Maximum Visibility */
	int			ret;				/* CFS Library Return Code */
	char		block_fname[128];	/* Block File Name				*/
	FILE		*block_file;		/* Block File					*/
	struct block_info	block[MAX_BLOCK];	/* Block Information	*/
	int			block_num;			/* Total Number of Blocks		*/

	if( argc < 10 ){
		printf("USAGE : station_bp [OBSNAME] [SRCNAME] [SRS_NUM] [STN_ID] [STN_NUM] [SS_NUM] [START_MJD] [STOP_MJD] [PGPLOT DEVICE] !!\n");
		exit(-1);
	}

	#ifdef DEBUG
	#endif
	printf("COMMAND : %s %s %s %s %s %s %s %s %s %s\n",
		argv[0], argv[1], argv[2], argv[3], argv[4],
		argv[5], argv[6], argv[7], argv[8], argv[9]);
	#ifdef DEBUG
	#endif

	obj_num = atoi( argv[3] );
	stn_id	= atoi( argv[4] );
	stn_num	= atoi( argv[5] );
	ssnum	= atoi( argv[6] );
	start_mjd	= atof( argv[7] );
	stop_mjd	= atof( argv[8] );

	cfs000_( &ret );			cfs_ret( 000, ret );
	cfs020_( &ret );			cfs_ret( 020, ret );
	cfs006_( argv[1], &ret, strlen( argv[1] )); cfs_ret( 006, ret );

	/*-------- ACCESS TO SOURCE SHARED MEMORY --------*/
	obj_key = ftok(KEY_DIR, OBJ_KEY);
	if(( shrd_obj_id = shmget(obj_key, 
		obj_num*sizeof(struct head_obj), 0444)) < 0 ){
		printf("Can't Access to Shared Memory : %s !!\n", argv[0] );
		exit(1);
	}
	obj_ptr	= (struct head_obj *)shmat(shrd_obj_id, NULL, 0);

	/*-------- SEARCH FOR THE TARGET STATION --------*/
	while( obj_ptr != NULL ){

		if( strstr(obj_ptr->obj_name, argv[2]) != NULL ){
			printf("FIND SOURCE NAME %s : ID = %d\n",
				argv[2], obj_ptr->obj_index);
			break;
		}
		obj_ptr	= obj_ptr->next_obj_ptr;
	}

	/*-------- ACCESS TO STATION SHARED MEMORY --------*/
	stn_key = ftok(KEY_DIR, STN_KEY);
	if(( shrd_stn_id = shmget(stn_key, 
		stn_num*sizeof(struct head_stn), 0444)) < 0 ){
		printf("Can't Access to Shared Memory : %s !!\n", argv[0] );
		exit(1);
	}
	stn_ptr	= (struct head_stn *)shmat(shrd_stn_id, NULL, 0);

	/*-------- SEARCH FOR THE TARGET STATION --------*/
	while( stn_ptr != NULL ){
		if(stn_ptr->stn_index == stn_id){	break;	}
		stn_ptr	= stn_ptr->next_stn_ptr;
	}
	if( stn_ptr == NULL){
		printf("Can't Find Station ID [%d] !!\n", stn_id);
	}

	/*-------- Read Block Information --------*/
	sprintf(block_fname, BLOCK_FMT, getenv("CFS_DAT_HOME"), argv[1], stn_ptr->acorr_index);
	printf("BLOCK FILE NAME = %s\n", block_fname);
	block_file = fopen(block_fname, "r");
	if( block_file == NULL){
		fclose(block_file);
		printf("  Missing Block Record Information... Please Run codainfo in advance !\n");
		return(0);
	}
	fread(&block_num, sizeof(int), 1, block_file);
	fread(block, block_num* sizeof(struct block_info), 1, block_file );
	fclose(block_file);

	printf("PGPLOT DEVICE = %s\n", argv[9]);
	cpgbeg(1, argv[9], 1, 1);

	/*-------- LOOP for SUB-STREAM --------*/
	position	= -1;
	position = block_search(block_num, block, start_mjd);
	printf("SEARCH PP = %d\n", position);
	for( ss_index=0; ss_index<ssnum; ss_index++){

		/*-------- READ SUB-STREAM HEAD --------*/
		read_sshead( stn_ptr->acorr_index,  ss_index+1,	&rf[ss_index],
			&freq_incr[ss_index],	&freq_num[ss_index],
			&time_num,	&time_incr );

		if( (freq_num[ss_index] * time_num != 0) ){

		/*-------- ALLOCATE MEMORY AREA FOR VISIBILITY DATA --------*/
		work_ptr= (float *)malloc(2*freq_num[ss_index]*sizeof(float));
		vis_r_ptr[ss_index]=(double *)malloc(freq_num[ss_index]*sizeof(double));
		vis_i_ptr[ss_index]=(double *)malloc(freq_num[ss_index]*sizeof(double));

		if( (work_ptr == NULL) || (vis_r_ptr[ss_index] == NULL) ){
			printf("Memory Error to Alloc Visibility !!\n"); 
			return(-1);
		}

		memset( vis_r_ptr[ss_index], 0, freq_num[ss_index]*sizeof(double));
		memset( vis_i_ptr[ss_index], 0, freq_num[ss_index]*sizeof(double));

		/*-------- INTEGRATE VISIBILITY DATA --------*/
		integ_pp = integ_bp( stn_ptr->acorr_index, obj_ptr->obj_index,
			ss_index+1, freq_num[ss_index], start_mjd, stop_mjd,
			&position, time_num, time_incr,
			work_ptr, vis_r_ptr[ss_index], vis_i_ptr[ss_index],
			&vis_max[ss_index] );

		}
	}


	/*-------- SAVE BANDPASS DATA --------*/
	save_bp( argv[1], stn_ptr->stn_name, stn_ptr->stn_index, argv[2],
			ssnum, &freq_num[0], start_mjd, time_incr*(double)integ_pp,
			rf, freq_incr, vis_r_ptr, vis_i_ptr);
	
	/*-------- PLOT BANDPASS DATA --------*/
	cpg_bp( argv[1], stn_ptr->stn_name, argv[2], start_mjd, stop_mjd,
		time_incr*(double)integ_pp, ssnum, freq_num, rf, freq_incr,
		vis_max, vis_r_ptr, vis_i_ptr);

	cpgend();
	return(0);
}
