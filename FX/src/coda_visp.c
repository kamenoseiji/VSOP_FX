/*********************************************
**	CODA_VISP.C	: Visibility Browser for	**
**					CODA File System		**
**											**
**	AUTHOR	: KAMENO Seiji					**
**	CREATED	: 1996/6/27						**
*********************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"
#include "merge.inc"
#define CP_DIR	"/sys01/custom/bin"
#define	MAX_ANT	10
#define	MAX_BL	45
#define	MAX_CL	120
#define	MAX_SS	32	
#define	MAX_CH	1024
#define	MAX_NODE 1024
#define RADDEG	57.29577951308232087721	
#define	SECDAY	86400
#define WIND_X	32	
#define	AMP_MAX		3.0
#define	PLL_LIMIT	0.75
#define	LOCK		1
#define	UNLOCK		0

#define	OBS_NAME	1
#define	SOURCE		2
#define	DL_CAL		3
#define	START		4
#define	STOP		5
#define	INTEG		6
#define	START_FREQ	7
#define	STOP_FREQ	8
#define	REF_SS		9
#define	DEVICE		10	
#define	STN_NAME1	11
#define	STN_NAME2	12	
#define	PC_SOLINT1	13
#define	PC_SOLINT2	14	

	/*-------- SHARED MEMORY --------*/
	int		shrd_obj_id;				/* Shared Memory ID for Source HD	*/
	int		shrd_stn_id;				/* Shared Memory ID for Station HD	*/
	key_t	vismap_key;					/* Keyword for Shared memory map	*/
	int		shrd_vismap_id;				/* Shared Memory ID for memory map	*/
	int		*shrd_vismap_ptr;			/* First Pointer of memory map		*/
	double	*bp_r_ptr[MAX_ANT][MAX_SS];	/* Pointer of Bandpass (REAL)		*/
	double	*bp_i_ptr[MAX_ANT][MAX_SS];	/* Pointer of Bandpass (IMAG)		*/

MAIN__(
	int		argc,			/* Number of Arguments			*/
	char	**argv,			/* Pointer to Arguments			*/
	char	**envp)			/* Pointer to Environments		*/
{

	/*-------- STRUCT for HEADDER in CFS --------*/
	struct	header		obs;			/* OBS HEADDER						*/
	struct	head_obj	*obj_ptr;		/* Pointer of Objects Header		*/
	struct	head_obj	*first_obj_ptr;	/* First Pointer of Objects Header	*/
	struct	head_stn	*stn_ptr;		/* Pointer of Station Header		*/
	struct	head_stn	*first_stn_ptr;	/* First Pointer of Station Header	*/
	struct	head_cor	*cor_ptr;		/* Pointer of CORR Header			*/
	struct	head_cor	*first_cor_ptr;	/* First Pointer of CORR Header		*/
	struct	pcal_data	*pcal_ptr;		/* P-Cal Data Pointer				*/
	int		fcal_ptr_ptr[MAX_ANT];		/* Pointer of CLOCK data			*/
	int		gcal_ptr_ptr[MAX_ANT];		/* Pointer of GCAL DATA				*/
	int		first_pcal_ptr[MAX_ANT];	/* Pointer of P-Cal Data			*/

	/*-------- INDEX --------*/
	int		stn_index;					/* Index for Station				*/
	int		stn_index2;					/* Index for Station				*/
	int		bl_index;					/* Index for Baseline				*/
	int		cl_index;					/* Index for Closure Phase			*/
	int		ss_index;					/* Index of Sub-Stream				*/
	int		ss_index2;					/* Index of SS in CFS				*/
	int		time_index;					/* Index for Time					*/
	int		pcal_index;					/* Index for P-Cal Data				*/
	int		ant1, ant2, ant3;			/* Antenna Index					*/
	int		delay_index;				/* Index of Delay Search Points		*/
	int		rate_index;					/* Index of Rate Search Points		*/
	int		search_index;				/* Index of Search Points			*/
	int		bl_id;						/* Baseline Index in Merge File		*/
	int		cl_id;						/* Closure Index in Merge File		*/

	/*-------- TOTAL NUMBER --------*/
	int		stn_num;					/* Number of Station				*/
	int		blnum;						/* Total Number of Baseline			*/
	int		clnum;						/* Total Number of Closure			*/
	int		ssnum;						/* Number of Sub-Stream				*/
	int		ssnum_in_cfs;				/* Number of Sub-Stream in CFS		*/
	int		freq_num;					/* Number of Frequency				*/
	int		bunch_num;					/* Number of Bunching				*/
	int		time_num_cfs;				/* Number of Time Data				*/
	int		time_num;					/* Number of Time Data				*/
	int		valid_pp;					/* Number of Valid PP				*/
	int		integ_pp;					/* Coherent Integration PP			*/
	int		antnum;						/* Total Number of Selected Station	*/
	int		bp_ssnum;					/* SS-Number in Band-Pass File		*/
	int		bp_freq_num[MAX_SS];		/* Frequency Channel Number			*/
	int		pcal_num[MAX_ANT];			/* Number of P-Cal Data				*/
	int		node_num[MAX_ANT];			/* Number of Node in Spline			*/
	int		stn_node_num;				/* Number of Node in Spline			*/
	int		delay_num;					/* Number of Delay Search Points	*/
	int		rate_num;					/* Number of Rate Search Points		*/
	int		search_num;					/* Number of Search Points			*/
	int		gff_result_size;			/* Memory Size for GFF Parameters	*/
	int		byte_len;					/* Buffer Byte Size in MERGE File 	*/

	/*-------- IDENTIFIER --------*/
	int		obj_id;						/* OBJECT ID						*/
	int		ret;						/* Return Code from CFS Library		*/
	int		lunit;						/* Unit Number of CFS File			*/
	int		position;					/* PP Position in CFS				*/
	int		start_offset;				/* Offset for Integration Start		*/
	int		stnid_in_cfs[MAX_ANT];		/* Station ID Number in CODA		*/
	int		refant_id;					/* Station ID of REF ANT			*/
	int		stn_arg;					/* Arg Index at Start of STATION	*/
	int		ss_id;						/* Target SS ID						*/
	int		ref_ss;						/* P-Cal Reference SS				*/
	int		pll_status[MAX_ANT];		/* Phase Lock Status				*/

	/*-------- GENERAL VARIABLES --------*/
	int		start_time;					/* START TIME [DDDHHMMSS]			*/
	int		stop_time;					/* STOP TIME [DDDHHMMSS]			*/
	int		year, doy;					/* YEAR and Day of Year				*/
	int		hour, min;					/* Hour and Minute					*/
	int		ut;							/* UT in MERGE File [1/60 sec]		*/
	int		real_coeff[MAX_ANT];		/* Pointer of Spline Coefficient	*/
	int		imag_coeff[MAX_ANT];		/* Pointer of Spline Coefficient	*/
	double	sec;						/* Second							*/
	double	start_mjd;					/* Start Time [MJD]					*/
	double	stop_mjd;					/* Stop Time [MJD]					*/
	double	epoch_mjd;					/* Epoch Time [MJD]					*/
	double	start_freq;					/* Integ Start Frequency [MHz]		*/
	double	stop_freq;					/* Integ Stop Frequency [MHz]		*/
	double	loss;						/* Quantize Efficiency				*/
	double	sum_delay[MAX_ANT];			/* Summation of Residual Delay		*/
	double	sum_dummy[MAX_ANT];			/* Summation of Residual Delay		*/
	double	rf;							/* RF Frequency [MHz]				*/
	double	freq_incr;					/* Frequency Increment [MHz]		*/
	double	bandedge1;					/* Lower Bandwdge [MHz]				*/
	double	bandedge2;					/* Lower Bandwdge [MHz]				*/
	double	time_incr;					/* Time Increment [sec]				*/
	double	atm_prm[3*MAX_ANT-1];		/* Atmospheric Parameters			*/
	double	sefd_prm[3*MAX_ANT];		/* SEFD Parameters					*/
	double	sefd;						/* SEFD at the Elevation			*/
	double	sin_el;						/* sin(EL)							*/
	double	dsecz;						/* dsecz / dt						*/
	double	*gff_result;				/* RESULT of GFF					*/
	double	*gff_trial;					/* Trial of GFF Parameters			*/
	double	*gff_best;					/* Best GFF Parameters				*/
	double	*gff_err;					/* ERROR of GFF						*/
	double	*gff_best_err;				/* ERROR of GFF	at Best Fit			*/
	double	bp_mjd;						/* MJD of the Band-Pass File		*/
	double	bp_integ_time;				/* Integ Time of the Band-Pass File */
	double	bp_rf[MAX_SS];				/* RF Freq. [MHz] in BP File		*/
	double	bp_freq_incr[MAX_SS];		/* Freq. Increment [MHz] in BP File	*/
	double	bp_vis_max[MAX_ANT][MAX_SS];/* Maximum of Bandpass				*/
	double	*time_ptr;					/* Pointer of Time					*/
	double	*pcphs_ptr;					/* Pointer of P-Cal Phase			*/
	double	*weight_ptr;				/* Pointer of P-Cal Weight			*/
	double	ref_phs;					/* Reference P-Cal Phase			*/
	double	time_node[MAX_ANT][MAX_NODE];/* Time Node in Spline				*/
	double	pc_solint[MAX_ANT];			/* Solution Interval for P-Cal		*/
	double	delay_incr;					/* Delay Increment for Grid Search	*/
	double	delay_offset;				/* Delay Offset for Grid Search		*/
	double	rate_incr;					/* Rate Increment for Grid Search	*/
	double	rate_offset;				/* Rate Offset for Grid Search		*/
	double	resid[100];					/* Residual Map						*/
	double	resid_min;					/* Minimum Residual					*/
	float	bl_direction;				/* Baseline Direction				*/
	float	work[2*MAX_CH];				/* Work Area to Read Visibility		*/
	float	vis_amp[MAX_BL];			/* Visibility Amplitude of Each BL	*/
	float	vis_amp_err[MAX_BL];		/* Visibility Amplitude Error		*/
	float	vis_phs[MAX_BL];			/* Visibility Phase of Each BL		*/
	float	vis_phs_err[MAX_BL];		/* Visibility Phase Error			*/
	float	clphs[MAX_CL];				/* Closure Phase 					*/
	float	clphs_err[MAX_CL];			/* Closure Phase Error				*/
	char	fname[128];					/* File Name of CFS Files			*/
	char	omode[2];					/* Access Mode of CFS Files			*/
	char	obj_name[32];				/* Object Name						*/
	char	bp_obj_name[32];			/* Object Name in Band-Pass File	*/
	char	history[80];				/* History Notation in MERGE File	*/

	/*-------- PGPLOT VARIABLES --------*/
	float	x_plot, y_plot;				/* X- and Y-Coordinate to Plot		*/
	float	y_top, y_bottom;			/* Top and Bottom Coordinate		*/
	float	xmin, xmax;					/* Max and Min of the Window Frame	*/
	float	ymin, ymax;					/* Max and Min of the Window Frame	*/
	double	x_incr, y_incr;				/* Increment of the Grid			*/
	int		x_offset;					/* Offset Value						*/

	/*-------- BLOCK INFO --------*/
	char		block_fname[128];	/* Block File Name				*/
	FILE		*block_file;		/* Block File					*/
	struct block_info   block[MAX_BLOCK];	/* Block Information	*/
	int			block_num;			/* Total Number of Blocks		*/

	memset( fcal_ptr_ptr, 0, sizeof(fcal_ptr_ptr) );
	memset( gcal_ptr_ptr, 0, sizeof(gcal_ptr_ptr) );
/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 15 ){
		printf("USAGE : coda_visp [OBS_NAME] [SOURCE] [DL_CAL] [START] [STOP] [INTEG] [START_FREQ] [STOP_FREQ] [REF_SS] [DEVICE] [STN_NAME1] [STN_NAME2] [PC_SOLINT1] [PC_SOLINT2]!!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  SOURCE -------- Source Name \n");
		printf("  DL_CAL -------- Source Name of Delay Calibrator\n");
		printf("  START --------- Start TIME [DDDHHMMSS] \n");
		printf("  STOP ---------- Stop TIME  [DDDHHMMSS] \n");
		printf("  INTEG --------- Integration TIME  [sec] \n");
		printf("  START_FREQ ---- Integration Start Freq. [MHz] \n");
		printf("  STOP_FREQ ----- Integration Stop Freq. [MHz] \n");
		printf("  REF_SS -------- P-Cal Reference SS \n");
		printf("  DEVICE -------- PGPLOT DEVICE \n");
		printf("  STN_NAME1 ----- STATION NAME 1 \n");
		printf("  STN_NAME2 ----- STATION NAME 2 \n");
		printf("  PC_SOLINT1 ---- P-Cal Solution Interval for STATION 1 \n");
		printf("  PC_SOLINT2 ---- P-Cal Solution Interval for STATION 2 \n");
		exit(0);
	}

	/*-------- START and STOP Time --------*/
	start_time	= atoi(argv[START]);
	stop_time	= atoi(argv[STOP]);
	start_freq	= atof(argv[START_FREQ]);
	stop_freq	= atof(argv[STOP_FREQ]);
	ref_ss		= atoi(argv[REF_SS]);
	pc_solint[0]= atof(argv[PC_SOLINT1]);
	pc_solint[1]= atof(argv[PC_SOLINT2]);

	sprintf(obj_name, "%s", argv[SOURCE]);
/*
------------------------ ACCESS TO THE CODA FILE SYSTEM (CFS)
*/
	cfs000_( &ret );			cfs_ret( 000, ret );
	cfs020_( &ret );			cfs_ret( 020, ret );
	cfs006_( argv[OBS_NAME], &ret, strlen( argv[OBS_NAME] ));
	cfs_ret( 006, ret );

	/*-------- FILE OPEN --------*/
	lunit	= 3;
	sprintf( fname, "HEADDER" ); sprintf( omode, "r" );
	cfs287_( fname, &ret, strlen(fname) );	cfs_ret( 287, ret );
	cfs103_( &lunit, fname, omode, &ret, strlen(fname), strlen(omode) );
	cfs_ret( 103, ret );

	/*-------- READ OBSHEAD --------*/
	read_obshead( lunit, &obs, &obj_ptr, &stn_ptr, &shrd_obj_id, &shrd_stn_id );
	first_obj_ptr	= obj_ptr;
	first_stn_ptr	= stn_ptr;
	acorr_pair( &obs, stn_ptr, &ssnum_in_cfs, &loss);

	/*-------- CONVERT START and STOP TIME -> MJD --------*/
	mjd2doy( (long)obs.start_mjd, &year, &doy );
	doy2fmjd( year, start_time/1000000,					/* YEAR and DOY */
		(start_time/10000)%100, (start_time/100)%100,	/* Hour and Minute */
		(double)(start_time%100),						/* Second */
		&start_mjd );
	doy2fmjd( year, stop_time/1000000,					/* YEAR and DOY */
		(stop_time/10000)%100, (stop_time/100)%100,		/* Hour and Minute */
		(double)(stop_time%100),						/* Second */
		&stop_mjd );

	/*-------- VERIFY START and STOP TIME --------*/
	if(start_mjd > obs.stop_mjd){
		printf("INTEG START [MJD=%lf] EXCEEDS OBS END TIME [MJD=%lf]!!\n",
			start_mjd, obs.stop_mjd);
	}
	if(stop_mjd < obs.start_mjd){
		printf("INTEG STOP [MJD=%lf] IS BEFORE OBS START TIME [MJD=%lf]!!\n",
			stop_mjd, obs.start_mjd);
	}

	/*-------- LINK SOURCE NAME to OBJECT ID --------*/
	objct_id( obj_ptr, obj_name, &obj_id );

	/*-------- LINK CORRELATION PAIR ID --------*/
	first_cor_ptr = (struct head_cor *)malloc( sizeof(struct head_cor));
	xcorr_pair( &obs, first_cor_ptr );

	/*-------- READ SS-HEAD --------*/
	for(ss_index=0; ss_index<ssnum_in_cfs; ss_index++){
		read_sshead( 1, ss_index+1, &rf, &freq_incr,
			&freq_num, &time_num, &time_incr);

		/*-------- PP NUMBER ROUND UP TO 2^n --------*/
		time_num	= (int)(atof(argv[INTEG])/time_incr)+1;
		integ_pp	= pow2round(time_num);

		/*-------- IN CASE OF USB (Upper Side Band)--------*/
		if( freq_incr > 0.0 ){
			bandedge1	= rf - 0.5*freq_incr;
			bandedge2	= rf + freq_incr * ((double)freq_num - 0.5);

		/*-------- IN CASE OF LSB (Lower Side Band)--------*/
		} else {
			bandedge2	= rf - 0.5*freq_incr;
			bandedge1	= rf + freq_incr * ((double)freq_num - 0.5);
		}

		if( (start_freq > bandedge1) && (start_freq < bandedge2) ){
			printf(" FIND TARGET SUB_STREAM [SS = %d] !\n", ss_index);
			if( stop_freq > bandedge2 ){
				printf(" CAUTION: SPECIFIED FREQ. RANGE LEAP OVER THE SS !!\n");
				printf("          STOP FREQUENCY IS MODIFIED TO %.2lf MHz\n",
					bandedge2 );
				stop_freq = bandedge2;
				freq_num = (int)((stop_freq - start_freq)/freq_incr + 0.5);
			}
			ss_id	= ss_index;
		}
	}

	/*-------- LINK STATION ID --------*/
	station_scan( first_stn_ptr, fcal_ptr_ptr, first_pcal_ptr, gcal_ptr_ptr,
			argv[DL_CAL], pcal_num, stnid_in_cfs, &refant_id,
			&antnum, &stn_num);
	stn_ptr = first_stn_ptr;
	while( stn_ptr != NULL ){
		if( strstr( stn_ptr->stn_name, argv[STN_NAME1] ) != NULL ){
			stnid_in_cfs[0]	= stn_ptr->stn_index;	}

		if( strstr( stn_ptr->stn_name, argv[STN_NAME2] ) != NULL ){
			stnid_in_cfs[1]	= stn_ptr->stn_index;	}
		
		stn_ptr = stn_ptr->next_stn_ptr;
	}
/*
------------------------ SCAN P-CAL Data
*/
	for(stn_index=0; stn_index<2; stn_index++){
		stn_index2	= stnid_in_cfs[stn_index] - 1;

		/*-------- MEMORY FOR P-CAL DATA STORAGE --------*/
		pcphs_ptr	= (double *)malloc( pcal_num[stn_index2] * sizeof(double));
		time_ptr	= (double *)malloc( pcal_num[stn_index2] * sizeof(double));
		weight_ptr	= (double *)malloc( pcal_num[stn_index2] * sizeof(double));

		pcal_ptr	= (struct pcal_data *)first_pcal_ptr[stn_index2];

		/*-------- PREPARATION FOR SPLINE CALCULATION --------*/
		while( pcal_ptr != NULL ){
			ref_phs	= pcal_ptr->phs[ss_id] - pcal_ptr->phs[ref_ss];
			*time_ptr   = (double)( pcal_ptr->mjd - (int)obs.start_mjd );

			/*-------- WEIGHT FOR SPLINE CALCULATION --------*/
			if( pcal_ptr->err[ss_id] > 0.0 ){
				*weight_ptr = 1.0/( pcal_ptr->err[ss_id]
							*		pcal_ptr->err[ss_id]);
			} else { *weight_ptr = 1.0e-6; }

			*pcphs_ptr	= atan2( sin(ref_phs), cos(ref_phs) );

			time_ptr ++;	weight_ptr ++;	pcphs_ptr ++;
			pcal_ptr = pcal_ptr->next_pcal_ptr;
		}
		time_ptr	-= pcal_num[stn_index2];
		weight_ptr	-= pcal_num[stn_index2];
		pcphs_ptr	-= pcal_num[stn_index2];


		/*-------- SPLINE CALCULATION ----------*/
		/* THIS MODULE REQUIRES SSL2			*/
		/*--------------------------------------*/
		pcal_spline( time_ptr, pcphs_ptr, weight_ptr, pcal_num[stn_index2],
			pc_solint[stn_index]/SECDAY,
			&real_coeff[stn_index], &imag_coeff[stn_index],
			time_node[stn_index], &stn_node_num );

		node_num[stn_index] = stn_node_num;
		free( pcphs_ptr );
		free( time_ptr );
		free( weight_ptr );
	}

/*
------------------------ SOLVE ATMOSPHERIC PARAMETERS
*/
	stn_ptr = first_stn_ptr;
	stn_num	= atmdelay_solve( &obs, first_obj_ptr, refant_id, fcal_ptr_ptr,
		stn_ptr, atm_prm);

	atmgain_solve( &obs, first_obj_ptr, gcal_ptr_ptr, first_stn_ptr,
		sefd_prm);

/*
------------------------ LOAD BANDPASS DATA TO MEMORY
*/
	antnum= 2; blnum = 1;
	for(stn_index=0; stn_index<antnum; stn_index++){
		read_bp(stnid_in_cfs[stn_index], bp_obj_name,
			&bp_ssnum, bp_freq_num, &bp_mjd, &bp_integ_time, 
			bp_rf, bp_freq_incr,
			bp_r_ptr[stn_index], bp_i_ptr[stn_index], bp_vis_max[stn_index]);
	}
/*
------------------------ PREPARE MEMORY AREA
*/
	printf(" ANTNUM = %d, BLNUM = %d\n", antnum, blnum);
	gff_result_size	= 2*(blnum + antnum - 1) * sizeof(double); 
	gff_result	= (double *)malloc( gff_result_size );
	gff_trial	= (double *)malloc( gff_result_size );
	gff_best	= (double *)malloc( gff_result_size );
	gff_err		= (double *)malloc( gff_result_size );
	gff_best_err= (double *)malloc( gff_result_size );

	memset( gff_result, 0, gff_result_size);
	memset( gff_err, 0, gff_result_size );

	/*-------- ALLOC MEMORY MAP --------*/
	vismap_key	= ftok(CP_DIR, 142857);
	if( (shrd_vismap_id	= shmget( vismap_key,
		2*blnum*sizeof(int), IPC_CREAT | 0644 )) < 0){
		printf(" Error in shmget [%s]\n", argv[0]);
		exit(0);
	}
	shrd_vismap_ptr	= (int *)shmat(shrd_vismap_id, NULL, 0);

	/*-------- LINK BASELINE ID -> STATION ID --------*/
	bl_index=0; bl2ant( bl_index, &stn_index2, &stn_index );
	cor_ptr	= first_cor_ptr;

	while(cor_ptr != NULL){
		if( (cor_ptr->ant1 == stnid_in_cfs[stn_index]) && 
			(cor_ptr->ant2 == stnid_in_cfs[stn_index2]) ){
			printf(" BASELINE [NORMAL] %d : %d - %d assigned to %d\n",
			bl_index, cor_ptr->ant1, cor_ptr->ant2, cor_ptr->cor_id );

			/*-------- ALLOCATE MEMORY AREA --------*/
			if( memalloc( 1, integ_pp, &shrd_vismap_ptr[bl_index],
				&shrd_vismap_ptr[blnum + bl_index]) == -1 ){
				printf("Failed to Allocate Memory ...BL=%d\n", bl_index);
				close_shm(antnum);
				return(0);
			}
			break;
		}

		if( (cor_ptr->ant1 == stnid_in_cfs[stn_index2]) && 
			(cor_ptr->ant2 == stnid_in_cfs[stn_index]) ){
			printf(" BASELINE [INVERT] %d : %d - %d assigned to %d\n",
			bl_index, cor_ptr->ant1, cor_ptr->ant2, cor_ptr->cor_id );

			/*-------- ALLOCATE MEMORY AREA --------*/
			if( memalloc( 1, integ_pp, &shrd_vismap_ptr[bl_index],
				&shrd_vismap_ptr[blnum + bl_index]) == -1 ){
				printf("Failed to Allocate Memory ...BL=%d\n", bl_index);
				close_shm(antnum);
				return(0);
			}
			break;
		}
		cor_ptr = cor_ptr->next_cor_ptr;
	}
/*
------------------------ LOAD VISIBILITY DATA TO MEMORY
*/
	time_index	= 0;
	/*-------- START TIME LOOP --------*/
	while(start_mjd + (double)(time_incr*time_num/2)/86400.0 < stop_mjd){

		fmjd2doy( start_mjd, &year, &doy, &hour, &min, &sec);
		printf("INTEG START = %03d %02d:%02d:%02d \n",
			doy, hour, min, (int)sec);

		/*-------- FIND SOURCE POSITION --------*/
		obj_ptr = first_obj_ptr;
		while(obj_ptr != NULL){
			if(obj_ptr->obj_index == obj_id){   break;}
			obj_ptr = obj_ptr->next_obj_ptr;
		}

		/*-------- CALC DELAY and RATE INCLUDING ATMOSPHARE --------*/
		stn_ptr = first_stn_ptr;
		printf("TOTAL STATION NUMBER = %d\n", stn_num);
		calc_delay( &obs, obj_ptr, stn_ptr, antnum, stn_num+1, 1, refant_id,
					stnid_in_cfs, atm_prm, start_mjd, gff_result );

		/*-------- LOAD BASELINE-BASE VISIBILITY TO MEMORY --------*/
		bl2ant( bl_index, &stn_index2, &stn_index );
		cor_ptr	= first_cor_ptr;

		/*-------- LOOP FOR BASELINE ID --------*/
		while(cor_ptr != NULL){

			/*-------- IN CASE OF NORMAL DIRECTION --------*/
			if( (cor_ptr->ant1 == stnid_in_cfs[stn_index]) && 
				(cor_ptr->ant2 == stnid_in_cfs[stn_index2]) ){
				bl_direction	= 1.0;

			/*-------- BLOCK INFO --------*/
			sprintf(block_fname, BLOCK_FMT, getenv("CFS_DAT_HOME"), argv[1], cor_ptr->cor_id);
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

			position = block_search(block_num, block, start_mjd);


				/*-------- LOAD VISIBILITY DATA  CFS -> MEMORY --------*/
#ifdef HIDOI
				valid_pp = bunchsel_visp( cor_ptr->cor_id, bl_direction, obj_id,
					ss_id+1, &position,
					time_num_cfs, time_num, time_incr,
					obs.start_mjd, start_mjd, stop_mjd,
					start_freq, freq_num,
					bp_r_ptr[stn_index][ss_id], bp_r_ptr[stn_index2][ss_id],

					real_coeff[stn_index], imag_coeff[stn_index],
					time_node[stn_index],	node_num[stn_index],
					real_coeff[stn_index2], imag_coeff[stn_index2],
					time_node[stn_index2],	node_num[stn_index2],

					work, gff_result);
#endif

				break;
			}
			cor_ptr = cor_ptr->next_cor_ptr;
		}
		printf(" GFF[0] = %e\n", gff_result[0]);
		printf(" GFF[1] = %e\n", gff_result[1]);
		printf(" GFF[2] = %e\n", gff_result[2]);
		printf(" GFF[3] = %e\n", gff_result[3]);

		integ_pp	= (int)(atof(argv[INTEG])/time_incr)+1;

#ifdef HIDOI
		if( valid_pp < integ_pp){ integ_pp = valid_pp;}
		start_offset	= 0;
		printf(" SOLINT PP = %d\n", valid_pp);
		printf(" INTEG PP  = %d\n", integ_pp);

		x_plot  = (float)( start_mjd
				+ (double)((time_incr*integ_pp/2)/86400.0)
				- x_offset);

		delay_num	= atoi( argv[DELAY_WIN] );
		rate_num	= atoi( argv[RATE_WIN]  );

		delay_incr	= 0.5/(rf[ssnum-1] - rf[0]);
		rate_incr	= 0.5/(rf[0]*valid_pp*time_incr);
		search_num	= delay_num * rate_num;
		resid_min	= 9999.0;

		for(stn_index=0; stn_index<antnum-1; stn_index++){
			switch( pll_status[stn_index] ){
			case LOCK:
				printf(" PLL STATUS [STN = %d] ... LOCK.....\n", stn_index);
				break;

			case UNLOCK:
				printf(" PLL STATUS [STN = %d] ... UNLOCK !!\n", stn_index);
				resid_min	= 9999.0;
				for( search_index=0; search_index<search_num; search_index++){

					delay_index		= search_index / rate_num - delay_num/2;
					rate_index		= search_index % rate_num - rate_num/2;
					delay_offset	= delay_incr * (double)delay_index;
					rate_offset		= rate_incr * (double)rate_index;

					memcpy( gff_trial, gff_result, gff_result_size );

					gff_trial[2*blnum + stn_index ] 			+= delay_offset;
					gff_trial[2*blnum + antnum + stn_index - 1] += rate_offset;

					/*-------- INITIAL INTEGRATION --------*/
					integ_fine( &shrd_vismap_ptr[0],
						&shrd_vismap_ptr[blnum*ssnum],
						ssnum, freq_num[0], rf, freq_incr,
						valid_pp, valid_pp, start_offset, time_incr,
						sum_dummy, antnum, gff_trial,  ss_id );

					/*-------- FINE SEARCH  --------*/
					gff_fine( &shrd_vismap_ptr[0],
						&shrd_vismap_ptr[blnum*ssnum],
						ss_id,	ssnum, freq_num[0], rf, freq_incr,
						valid_pp, time_incr, antnum, gff_trial, gff_err,
						&resid[search_index] );

					printf(" SEARCH %d: STN=%d DELAY_OFS=%9.2e RATE_OFS= %9.2e RESID= %8.5lf\n",
						search_index,		stn_index,
						delay_offset,		rate_offset, resid[search_index] );

					if( resid[search_index] < resid_min ){
						resid_min = resid[search_index];
						memcpy( gff_best, gff_trial, gff_result_size );
					}
				}
				gff_result[2*blnum + stn_index]
						= gff_trial[2*blnum + stn_index];
				gff_result[2*blnum + antnum + stn_index - 1]
						= gff_trial[2*blnum + antnum + stn_index - 1];
				break;
			}
		}

		/* GFF FINE */
		integ_fine( &shrd_vismap_ptr[0], &shrd_vismap_ptr[blnum*ssnum],
			ssnum, freq_num[0], rf, freq_incr, valid_pp, valid_pp,
			start_offset, time_incr, sum_dummy, antnum, gff_best,  ss_id );

		gff_fine( &shrd_vismap_ptr[0],	&shrd_vismap_ptr[blnum*ssnum],
				ss_id,	ssnum, freq_num[0], rf, freq_incr,
				valid_pp, time_incr, antnum, gff_best, gff_err, &resid[0] );

		memcpy( gff_result, gff_best, gff_result_size );

		/*-------- CHECK PHASE LOCK STATUS --------*/
		for(stn_index=0; stn_index<antnum-1; stn_index++){
			pll_status[stn_index]	= LOCK;
		}
		for(bl_index=0; bl_index<blnum; bl_index++){
			if( gff_err[bl_index + blnum] > PLL_LIMIT ){
				bl2ant( bl_index, &ant2, &ant1);
				pll_status[ant2 -1]	= UNLOCK;
				if( ant1 != 0 ){
					pll_status[ant1 -1]	= UNLOCK;
				}
			}
		}


		for(stn_index=0; stn_index<antnum-1; stn_index++){
			sum_delay[stn_index]
				+= (double)(time_incr*integ_pp)
				*  gff_result[2*blnum + antnum + stn_index - 1];
		}

		/*-------- FINAL INTEGRATION --------*/
		integ_fine( &shrd_vismap_ptr[0],	&shrd_vismap_ptr[blnum*ssnum],
					ssnum, freq_num[0], rf, freq_incr,
					valid_pp, integ_pp, start_offset, time_incr,
					sum_delay, antnum, gff_result, ss_id );

		for( bl_index=0; bl_index<blnum; bl_index++){

			bl2ant( bl_index, &ant2, &ant1);

			/*-------- CALC EL and SEFD	--------*/
			sefd = 1.0;
			stn_ptr	= first_stn_ptr;
			while(stn_ptr != NULL){

				if( stn_ptr->stn_index == ant1 + 1 ){
					calc_el(&obs, obj_ptr, stn_ptr, start_mjd, &sin_el, &dsecz);

					sefd *= ( sefd_prm[3*ant1]
							* exp( sefd_prm[3*ant1 + 2] / sin_el )
							+ sefd_prm[3*ant1 + 1] );
				}

				if( stn_ptr->stn_index == ant2 + 1 ){
					calc_el(&obs, obj_ptr, stn_ptr, start_mjd, &sin_el, &dsecz);

					sefd *= ( sefd_prm[3*ant2]
							* exp( sefd_prm[3*ant2 + 2] / sin_el )
							+ sefd_prm[3*ant2 + 1] );
				}
				stn_ptr	= stn_ptr->next_stn_ptr;
			}
			sefd	= sqrt(sefd);

			vis_amp[bl_index]	= gff_result[bl_index] * sefd;
			vis_amp_err[bl_index]	= gff_err[bl_index] * sefd;

			vis_phs[bl_index]	= gff_result[bl_index + blnum];
			vis_phs_err[bl_index]	= gff_err[bl_index + blnum];

			printf("BL%02d: AMP= %9.3e +/- %9.3e  PHS = %8.4f +/- %8.4f\n",
				bl_index, vis_amp[bl_index], vis_amp_err[bl_index], 
				vis_phs[bl_index], vis_phs_err[bl_index] );

		}
#endif

		start_mjd	+= (double)(time_incr*integ_pp)/SECDAY;
		time_index	++;

	}	/* END OF TIME LOOP */


	close_shm(antnum);
	return(0);
}

#ifdef HIDOI
baseline_vis(argv)
	char	**argv;
{
	bunch_num	= freq_num;


	freq_incr	*= bunch_num;
}
#endif

close_shm( antnum )
	int		antnum;				/* Total Antenna Number				*/
{
	int		stn_index;			/* Station Index					*/

	for(stn_index=0; stn_index<antnum; stn_index++){
		free(bp_r_ptr[stn_index]);
		free(bp_i_ptr[stn_index]);
	}

	/*-------- CLOSE SHARED MEMORY --------*/
	shmctl( shrd_obj_id, IPC_RMID, 0 );
	shmctl( shrd_stn_id, IPC_RMID, 0 );
	shmctl( shrd_vismap_id, IPC_RMID, 0 );
	cpgend();

	return;
}
