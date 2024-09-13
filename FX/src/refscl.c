/*********************************************************
**	REFSCL.C : Scaling for Reference Antenna Gain		**
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

#define	MAX_SS	32							/* Maximum Number of SS */
#define	PI2		1.57079632679489661922

MAIN__(
	int		argc,			/* Number of Arguments			*/
	char	**argv,			/* Pointer to Arguments			*/
	char	**envp)			/* Pointer to Environments		*/
{
	struct	head_stn	*stn_ptr;			/* Pointer of Station Header */
	struct	head_obj	*obj_ptr;			/* Pointer of Object Header */
	int					stn_id;				/* Station ID */
	int					stn_num;			/* Total Number of Stations */
	int					obj_num;			/* Total Number of Objects */
	int					ssnum;				/* Number of Sub-Stream */
	int					integ_pp;			/* Integrated PP Number */
	int					vanvnode_num;		/* Number of Spline Nodes	*/
	double				start_mjd;			/* Start Time [MJD] */
	double				stop_mjd;			/* Stop Time [MJD] */
	double				*spline_node;		/* Pointer of Spline Nodes	*/
	double				*spline_fact;		/* Pointer of Spline Factor	*/

	char			bp_obj_name[32];	/* Object Name in BP FILE */
	int				bp_ssnum;			/* Total SS number in BP FILE */
	int				bp_freq_num[MAX_SS];/* Freq. CH Number in BP FILE */
	double			bp_mjd;				/* MJD in BP FILE */
	double			bp_integ_time;		/* INTEG Time [sec] in BP FILE */
	double			bp_rf[MAX_SS];		/* RF Freq [MHz] in BP FILE */
	double			bp_freq_incr[MAX_SS];/* Freq Increment [MHz] in BP */
	double			*bp_r_ptr[MAX_SS];	/* Pointer of Bandpass (REAL) */
	double			*bp_i_ptr[MAX_SS];	/* Pointer of Bandpass (IMAG) */
	double			bp_vis_max[MAX_SS];	/* MAXIMUM in Bandpass data */

	key_t		obj_key;			/* Keyword of Shared Memory */
	key_t		stn_key;			/* Keyword of Shared Memory */
	key_t		spec_key;			/* Keyword of Shared Memory */
	int			shrd_obj_id;		/* Shared Memory ID */
	int			shrd_stn_id;		/* Shared Memory ID */
	int			shrd_spec_id;		/* Shared Memory ID */
	int			ss_index;			/* Index for Sub-Stream */
	int			spec_index;			/* Index for Spectral Points */
	int			position;			/* Start PP Position				*/
	double		rf;					/* RF [MHz] for Each Sub-Stream		*/
	double		freq_incr;			/* Freq. Increment for Each SS		*/
	int			freq_num;			/* Freq. Channel Number in Each SS */
	int			time_num;			/* Total Number of PP in CFS */
	double		time_incr;			/* Increment for Time [sec] */
	float		*work_ptr;			/* Pointer for Work Area */
	double		*vis_r_ptr;			/* Pointer of Vis. for BL and SS */
	double		*vis_i_ptr;			/* Pointer of Vis. for BL and SS */
	double		*plvis_r_ptr;		/* Pointer of Vis. for BL and SS */
	double		*plvis_i_ptr;		/* Pointer of Vis. for BL and SS */
	double		vis_max;			/* Maximum Visibility */
	double		*spec_ptr;			/* Poninter of Shared SPEC INFO */
	double		offline_ratio;		/* Tsys Ratio Between BP and VIS	*/
	double		spec_pos[6];		/* Selected Spectral Position		*/
	int			ret;				/* CFS Library Return Code */
	char		pg_dev[256];
	float		x_pos[6], y_pos[6];
	int			npoint;
	char		block_fname[128];	/* Block File Name				*/
	FILE		*block_file;		/* Block File					*/
	struct block_info   block[MAX_BLOCK];	/* Block Information	*/
	int			block_num;			/* Total Number of Blocks		*/



	if( argc < 11 ){
		printf("USAGE : refscl [OBSNAME] [SRCNAME] [SRC_NUM] [STN_ID] [STN_NUM] [SS_INDEX] [SSNUM] [START_MJD] [STOP_MJD] [AFACT]!!\n");
		exit(-1);
	}

	#ifdef DEBUG
	printf("COMMAND : %s %s %s %s %s %s %s %s %s %s %s\n",
		argv[0], argv[1], argv[2], argv[3],
		argv[4], argv[5], argv[6], argv[7],
		argv[8], argv[9], argv[10]);
	#endif

	obj_num	= atoi( argv[3] );
	stn_id	= atoi( argv[4] );
	stn_num	= atoi( argv[5] );
	ss_index= atoi( argv[6] );
	ssnum	= atoi( argv[7] );
	start_mjd	= atof( argv[8] );
	stop_mjd	= atof( argv[9] );

	cfs000_( &ret );			cfs_ret( 000, ret );
	cfs020_( &ret );			cfs_ret( 020, ret );
	cfs006_( argv[1], &ret, strlen( argv[1] )); cfs_ret( 006, ret );

	vanvleck2_init( &vanvnode_num, &spline_node, &spline_fact );

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

	/*-------- ACCESS TO SPECTRUM SHARED MEMORY --------*/
	spec_key = ftok(KEY_DIR, SPC_KEY);
	if(( shrd_spec_id = shmget(spec_key, 7*sizeof(double), 0444)) < 0 ){
		printf("Can't Access to Shared Memory : %s !!\n", argv[0] );
		exit(1);
	}
	spec_ptr	= (double *)shmat(shrd_spec_id, NULL, 0);

	/*-------- OPEN PGPLOT DEVICE --------*/
	cpgbeg(1, "/xw", 1, 1);

/*
----------------------------------------- READ BANDPASS TABLE
*/
	/*-------- READ BANDPASS DATA --------*/
	read_bp( stn_id, bp_obj_name,
			&bp_ssnum, bp_freq_num, &bp_mjd, &bp_integ_time,
			bp_rf, bp_freq_incr, bp_r_ptr, bp_i_ptr, bp_vis_max);

/*
----------------------------------------- INTEGRATE VISIBILITY
*/

	/*-------- READ SUB-STREAM HEAD --------*/
	read_sshead( stn_ptr->acorr_index,  ss_index+1,	&rf,
		&freq_incr,	&freq_num, &time_num,	&time_incr );


	/*-------- ALLOCATE MEMORY AREA FOR VISIBILITY DATA --------*/
	work_ptr= (float *)malloc(2*freq_num*sizeof(float));
	vis_r_ptr=(double *)malloc(freq_num*sizeof(double));
	vis_i_ptr=(double *)malloc(freq_num*sizeof(double));
	plvis_r_ptr=(double *)malloc(freq_num*sizeof(double));
	plvis_i_ptr=(double *)malloc(freq_num*sizeof(double));

	if( (work_ptr == NULL) || (vis_r_ptr == NULL) ){
		printf("Memory Error to Alloc Visibility !!\n"); 
		return(-1);
	}
	memset( vis_r_ptr, 0, freq_num*sizeof(double));
	memset( vis_i_ptr, 0, freq_num*sizeof(double));

	/*-------- INTEGRATE VISIBILITY DATA --------*/
	position = block_search(block_num, block, start_mjd);
	integ_pp = integ_bp( stn_ptr->acorr_index, obj_ptr->obj_index,
		ss_index+1, freq_num, start_mjd, stop_mjd, &position,
		time_num, time_incr,
		work_ptr, vis_r_ptr, vis_i_ptr, &vis_max );

	memcpy( plvis_r_ptr, vis_r_ptr, freq_num*sizeof(double));
	memcpy( plvis_i_ptr, vis_i_ptr, freq_num*sizeof(double));

	/*-------- CALIBRATE VISIBILITY DATA --------*/
#ifdef HIDOI
	vanvleck( freq_num, -1.0, vis_r_ptr, vis_i_ptr );
	vanvleck( freq_num, -1.0, bp_r_ptr[ss_index], bp_i_ptr[ss_index] );
	vanvleck( freq_num, -1.0, plvis_r_ptr, plvis_i_ptr );
#endif
	vanvleck2( freq_num, vanvnode_num, spline_node, spline_fact, -1.0,
				vis_r_ptr, vis_i_ptr );
	vanvleck2( freq_num, vanvnode_num, spline_node, spline_fact, -1.0,
				bp_r_ptr[ss_index], bp_i_ptr[ss_index] );
	vanvleck2( freq_num, vanvnode_num, spline_node, spline_fact, -1.0,
				plvis_r_ptr, plvis_i_ptr );

	cal_exec( freq_num, plvis_r_ptr, plvis_i_ptr,
				bp_r_ptr[ss_index], bp_i_ptr[ss_index], &vis_max );

	/*-------- PLOT BANDPASS DATA --------*/
	cpg_acorr( argv[1], stn_ptr->stn_name, argv[2], start_mjd, stop_mjd,
		(double)integ_pp*time_incr, 1, &freq_num, &rf, &freq_incr,
		&vis_max, &plvis_r_ptr);

	/*-------- USER SELECTION FOR SIGNAL AND BASELINE --------*/
	npoint = 0; cpgsch(1.0);
	cpgncur( 6, &npoint, &x_pos[0], &y_pos[0], 31 );
	for( spec_index=0; spec_index<6; spec_index++){
		spec_pos[spec_index] = (double)x_pos[spec_index];
		spec_ptr[spec_index] = (double)x_pos[spec_index];
	}

	/*-------- TSYS RATIO BETWEEN VIS AND BP --------*/
	cal_offline( freq_num, rf, freq_incr, spec_pos,
				vis_r_ptr, vis_i_ptr, bp_r_ptr[ss_index], bp_i_ptr[ss_index],
				&offline_ratio );

	printf("OFFLINE RATIO = %lf\n", offline_ratio );

	cal_exec( freq_num, vis_r_ptr, vis_i_ptr,
				bp_r_ptr[ss_index], bp_i_ptr[ss_index], &vis_max );

	vis_max	= 0.0;

	cal_offset(0, atof(argv[10]), freq_num, rf, freq_incr, spec_pos,
				vis_r_ptr, vis_i_ptr, &vis_max, spec_ptr );

/*
	cal_offset(atof(argv[10]), freq_num, rf, freq_incr, spec_pos,
				vis_r_ptr, vis_i_ptr, offline_ratio, &vis_max, spec_ptr );
*/

	cpg_acorr( argv[1], stn_ptr->stn_name, argv[2], start_mjd, stop_mjd,
		(double)integ_pp*time_incr, 1, &freq_num, &rf, &freq_incr,
		&vis_max, &vis_r_ptr);

	cpgend();

	/*-------- PLOT BANDPASS TO PS FILE --------*/
	sprintf( pg_dev, "pgplot.%d.ps/ps", stn_id );
	printf( "SAVE PGPLOT TO %s\n", pg_dev );
	cpgbeg(1, pg_dev, 1, 1);
	cpg_acorr( argv[1], stn_ptr->stn_name, argv[2], start_mjd, stop_mjd,
		(double)integ_pp*time_incr, 1, &freq_num, &rf, &freq_incr,
		&vis_max, &vis_r_ptr);
	cpgend();

	shmdt((char *)obj_ptr);
	shmdt((char *)stn_ptr);
	shmdt((char *)spec_ptr);
	return(0);
}


#ifdef HIDOI

cal_offset( afact, freq_num, rf, freq_incr, xpos_ptr,
			vis_r_ptr, vis_i_ptr, offline_ratio, vis_max_ptr, spec_ptr )
	double	afact;
	int		freq_num;
	double	rf;
	double	freq_incr;
	double	*xpos_ptr;
	double	*vis_r_ptr;
	double	*vis_i_ptr;
	double	*vis_max_ptr;
	double	offline_ratio;
	double	*spec_ptr;
{
	double	*first_r_ptr;
	double	*first_i_ptr;
	double	freq;
	double	offline;
	double	online;
	int		freq_index;
	int		integ_off;

	first_r_ptr = vis_r_ptr;
	first_i_ptr = vis_i_ptr;

	freq		= rf;
	integ_off	= 0;
	offline		= 0.0;	online		= 0.0;

	/*-------- EMPTY AREA 1 --------*/
	spec_ptr[0]	= xpos_ptr[0];
	while( freq < xpos_ptr[0] ){
		vis_i_ptr++; vis_r_ptr++;
		freq += freq_incr;
	}

	/*-------- BASELINE AREA 1 --------*/
	spec_ptr[1]	= xpos_ptr[1];
	while( freq <= xpos_ptr[1] ){
		offline	+= sqrt((*vis_r_ptr)*(*vis_r_ptr) + (*vis_i_ptr)*(*vis_i_ptr));
		integ_off++;
		vis_i_ptr++; vis_r_ptr++;
		freq += freq_incr;
	}

	/*-------- EMPTY AREA 2 --------*/
	spec_ptr[2]	= xpos_ptr[2];
	while( freq < xpos_ptr[2] ){
		vis_i_ptr++; vis_r_ptr++;
		freq += freq_incr;
	}

	/*-------- SIGNAL AREA --------*/
	spec_ptr[3]	= xpos_ptr[3];
	while( freq <= xpos_ptr[3] ){
		vis_i_ptr++; vis_r_ptr++;
		freq += freq_incr;
	}

	/*-------- EMPTY AREA 3 --------*/
	spec_ptr[4]	= xpos_ptr[4];
	while( freq < xpos_ptr[4] ){
		vis_i_ptr++; vis_r_ptr++;
		freq += freq_incr;
	}

	/*-------- BASELINE AREA 2 --------*/
	spec_ptr[5]	= xpos_ptr[5];
	while( freq <= xpos_ptr[5] ){
		offline	+= sqrt((*vis_r_ptr)*(*vis_r_ptr) + (*vis_i_ptr)*(*vis_i_ptr));
		integ_off++;
		vis_i_ptr++; vis_r_ptr++;
		freq += freq_incr;
	}

	offline	/= integ_off;
	vis_r_ptr = first_r_ptr;
	vis_i_ptr = first_i_ptr;

	freq	= rf;
	for(freq_index=0; freq_index< freq_num; freq_index++){
		vis_r_ptr[freq_index]	-= offline;
		vis_r_ptr[freq_index]	*= (afact * offline_ratio);

		if( (freq >= xpos_ptr[2]) && (freq <= xpos_ptr[3] )){
			online	+= (freq_incr * vis_r_ptr[freq_index] );

			if( vis_r_ptr[freq_index] > *vis_max_ptr ){
				*vis_max_ptr	= vis_r_ptr[freq_index];
			}
		}
		freq	+= freq_incr;
	}

	printf(" INTEGRATED FLUX DENSYTY = %7.2lf Jy MHz [ %lf - %lf MHz]\n", 
		online,	xpos_ptr[2], xpos_ptr[3] );

	spec_ptr[6]	= online;

	return;
}

#endif


cal_exec( freq_num, vis_r_ptr, vis_i_ptr, bp_r_ptr, bp_i_ptr, vis_max_ptr )
	int		freq_num;
	double	*vis_r_ptr;
	double	*vis_i_ptr;
	double	*bp_r_ptr;
	double	*bp_i_ptr;
	double	*vis_max_ptr;
{
	int		freq_index;
	double	bp_sqr;
	double	visamp;
	double	vis_r, vis_i;

	*vis_max_ptr	= -9999.0;
	for(freq_index=0; freq_index<freq_num; freq_index++){
		vis_r	= *vis_r_ptr;
		vis_i	= *vis_i_ptr;

		bp_sqr	= (*bp_r_ptr)*(*bp_r_ptr) + (*bp_i_ptr)*(*bp_i_ptr);

		*vis_r_ptr	= (vis_r*(*bp_r_ptr) + vis_i*(*bp_i_ptr)) / bp_sqr;
		*vis_i_ptr	= (vis_i*(*bp_r_ptr) - vis_r*(*bp_i_ptr)) / bp_sqr;

		visamp	= sqrt((*vis_r_ptr)*(*vis_r_ptr) + (*vis_i_ptr)*(*vis_i_ptr));

		if( visamp > *vis_max_ptr ){
			*vis_max_ptr = visamp;
		}
		vis_r_ptr++;	vis_i_ptr++;	bp_r_ptr++;	bp_i_ptr++;
	}
	return;
}
