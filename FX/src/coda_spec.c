/*********************************************************
**	CODA_SPEC.C	: Integrate Visibility in CFS and Disp	**
**			Cross Power Spectrum.						**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"
#define	MAX_ANT	10
#define	MAX_SS	32	
#define	MAX_CH	1024
#define	MAX_BLOCK	1024
#define	RADDEG	57.29577951308232087721
#define	SECDAY	86400
#define OBS_NAME	1
#define SOURCE		2
#define DELAY_CAL	3
#define START		4
#define STOP		5
#define INTEG		6
#define SOLINT		7
#define BUNCH		8
#define DEVICE		9
#define STN_NAME1	10
#define STN_NAME2	11
#define	TOTAL_SS	12
#define SS_ARG		13
#define	REFANT		-1


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

	/*-------- SHARED MEMORY --------*/
	int		shrd_obj_id;				/* Shared Memory ID for Source HD	*/
	int		shrd_stn_id;				/* Shared Memory ID for Station HD	*/

	/*-------- POINTER --------*/
	int		*vismap_ptr;				/* Pointer of Visibility Map		*/
	int		fcal_addr[2];				/* Pointer of FCAL Data				*/
	int		time_ptr_ptr[2];			/* Pointer of Time Series			*/
	int		delay_ptr_ptr[2];			/* Pointer of Delay Series			*/
	int		rate_ptr_ptr[2];			/* Pointer of Rate Series			*/
	int		acc_ptr_ptr[2];				/* Pointer of Acc Series			*/
	int		delaywgt_ptr_ptr[2];		/* Pointer of Delay Weight Series	*/
	int		ratewgt_ptr_ptr[2];			/* Pointer of Rate Weight Series	*/
	int		accwgt_ptr_ptr[2];			/* Pointer of Acc Weight Series		*/
	int		delay_coeff[2];				/* Pointer of Delay Spline Coeff	*/
	int		rate_coeff[2];				/* Pointer of Rate Spline Coeff		*/
	int		time_node[2];				/* Pointer of Time Node				*/
	int		time_rate_node[2];			/* Pointer of Time Node				*/
	double	*ave_bp_r;					/* Synthesized Bandpass (real)		*/
	double	*ave_bp_i;					/* Synthesized Bandpass (real)		*/

	/*-------- INDEX --------*/
	int		stn_index;					/* Index for Station				*/
	int		stn_index2;					/* Index for Station				*/
	int		bl_index;					/* Index for Baseline				*/
	int		ss_index;					/* Index of Sub-Stream				*/
	int		ss_index2;					/* Index of Sub-Stream				*/
	int		data_index;					/* Index of PP Data					*/
	int		time_index;					/* Index for Time					*/
	int		freq_index;					/* Index for Frequency				*/
	int		prev_pp;					/* Previous PP Number				*/
	int		block_index;				/* Index of PP Block				*/

	/*-------- TOTAL NUMBER --------*/
	int		stn_num;					/* Total Number of Stations			*/
	int		blnum;						/* Total Number of Baseline			*/
	int		ssnum;						/* Number of Sub-Stream				*/
	int		ssnum_in_cfs;				/* Number of Sub-Stream in CODA		*/
	int		freq_num[MAX_SS];			/* Number of Frequency				*/
	int		bunch_num;					/* Number of Bunching				*/
	int		time_num;					/* Number of Time Data in CFS		*/
	int		time_num_cfs;				/* Time Number of CFS				*/
	int		antnum;						/* Total Number of Selected Station	*/
	int		integ_pp;					/* Coherent Integration PP			*/
	int		valid_pp;					/* Valid PP Number					*/
	int		bp_ssnum;					/* SS-Number in Band-Pass Filei		*/
	int		bp_freq_num[MAX_SS];		/* Frequency Channel Number			*/
	int		node_delay_num[2];			/* Number of Delay Node				*/
	int		node_rate_num[2];			/* Number of Rate Node				*/

	/*-------- IDENTIFIER --------*/
	int		refant_id;					/* REFANT ID in CFS					*/
	int		stnid_in_cfs[10];			/* Station ID Number in CODA		*/
	int		ssid_in_cfs[MAX_SS];		/* SS ID in CODA					*/
	int		position;					/* Start PP Position				*/
	int		obj_id;						/* OBJECT ID						*/
	int		fcaldata_num[10];			/* Status of Delay Calib File		*/
	int		pp_block_ini[MAX_BLOCK];	/* PP Index at Begin of PP Block	*/
	int		*pp_data_ptr;				/* Pointer of PP Data				*/
	double	*time_data_ptr;				/* Pointer of Time Data				*/

	int		solint;						/* Solution Interval				*/

	double	power;						/* Total Power of Bandpass			*/
	float	bl_direction;				/* Baseline Direction  (-1 or 1)	*/
	int		ret;						/* Return Code from CFS Library 	*/
	int		lunit;						/* Unit Number of CFS File			*/
	double	pp_incr;					/* PP Increment						*/
	char	fname[128];					/* File Name of CFS Files			*/
	char	omode[2];					/* Access Mode of CFS Files			*/
	char	obj_name[32];				/* Object Name						*/
	double	start_mjd;					/* Start Time [MJD]					*/
	double	stop_mjd;					/* Stop Time [MJD]					*/
	double	loss;						/* Quantize Efficiency				*/
	double	rf[MAX_SS];					/* RF Frequency [MHz]				*/
	double	freq_incr[MAX_SS];			/* Frequency Increment [MHz]		*/
	double	time_incr;					/* Time Increment [sec]				*/
	double	current_mjd;				/* Current MJD [MJD]				*/
	double	bldelay;					/* Baseline-Based Delay [microsec]	*/
	double	blrate;						/* Baseline-Based Rate [picosec/sec]*/
	float	work[2*MAX_CH];				/* Work Area to Read Visibility		*/
	float	vismax;						/* Maximum Visibility Amp			*/
	int		start_time;					/* START TIME [DDDHHMMSS]			*/
	int		stop_time;					/* STOP TIME [DDDHHMMSS]			*/
	int		year, doy;					/* YEAR and Day of Year				*/
	int		hour;						/* Hour								*/
	int		min;						/* Minute							*/
	double	sec;						/* Second							*/
	char	bp_obj_name[32];			/* Object Name in Band-Pass File	*/
	double	bp_mjd;						/* MJD of the Band-Pass File		*/
	double	bp_integ_time;				/* Integ Time of the Band-Pass File	*/
	double	bp_rf[MAX_SS];				/* RF Freq. [MHz] in BP File		*/
	double	bp_freq_incr[MAX_SS];		/* Freq. Increment [MHz] in BP File */
	double	*bp_r_ptr[MAX_ANT][MAX_SS];	/* Pointer of Bandpass (REAL)		*/
	double	*bp_i_ptr[MAX_ANT][MAX_SS];	/* Pointer of Bandpass (IMAG)		*/
	double	bp_vis_max[MAX_ANT][MAX_SS];/* Maximum of Bandpass				*/
	double	delay[MAX_ANT];				/* Clock Offset at the Epoch		*/
	double	rate[MAX_ANT];				/* Clock Rate						*/
	double	mjd_min,	mjd_max;		/* MAX and Min of MJD				*/
	double	rate_min,	rate_max;		/* MAX and Min of Rate				*/
	double	delay_min,	delay_max;		/* MAX and Min of Delay				*/
	double	acc_min,	acc_max;		/* MAX and Min of MJD				*/
	double	atm_prm[3*MAX_ANT-1];		/* Atmospheric Parameters			*/
	double	sin_el;						/* sin(EL)							*/
	double	dsecz;						/* dsec(z)/dt						*/
	char	pg_dev[256];				/* PGPLOT Device Name				*/

	/*-------- BLOCK INFO --------*/
	char		block_fname[128];	/* Block File Name				*/
	FILE		*block_file;		/* Block File					*/
	struct block_info   block[MAX_BLOCK];	/* Block Information	*/
	int			block_num;			/* Total Number of Blocks		*/


/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 9 ){
		printf("USAGE : coda_spec [OBS_NAME] [SOURCE] [DELAY_CAL] [START] [STOP] [INTEG] [SOLINT] [BUNCH] [DEVICE] [STN_NAME1] [STN_NAME2] [TOTAL_SS] [SS1] [SS2] ... !!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  SOURCE -------- Source Name\n");
		printf("  DELAY_CAL------ Delay Calib Source Name\n");
		printf("  START --------- Start TIME [DDDHHMMSS] \n");
		printf("  STOP ---------- Stop TIME  [DDDHHMMSS] \n");
		printf("  INTEG --------- Integration TIME  [sec] \n");
		printf("  SOLINT -------- Delay Solution Interval  [sec] \n");
		printf("  BUNCH --------- Bunching Number \n");
		printf("  DEVICE -------- PGPLOT Device \n");
		printf("  STN_NAME ------ STATION NAMEs \n");
		printf("  TOTAL_SS ------ TOTAL STATION NUMBER \n");
		exit(0);
	}

	/*-------- START and STOP Time --------*/
	start_time	= atoi(argv[START]);
	stop_time	= atoi(argv[STOP]);
	sprintf(obj_name, "%s", argv[SOURCE]);

	ssnum	= atoi( argv[TOTAL_SS] );
	for( ss_index=0; ss_index<ssnum; ss_index++){
		ssid_in_cfs[ss_index] = atoi( argv[SS_ARG + ss_index] );
		printf("ssid_in_cfs = %d\n", ssid_in_cfs[ss_index]);
	}
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
	obj_ptr	= first_obj_ptr;
	objct_id( obj_ptr, obj_name, &obj_id );

	/*-------- LINK CORRELATION PAIR ID --------*/
	first_cor_ptr = (struct head_cor *)malloc( sizeof(struct head_cor));
	xcorr_pair( &obs, first_cor_ptr );

	ssnum_in_cfs	= first_cor_ptr->nss;
/*
------------------------ FIND SPECIFIED STATION in CFS
*/
	/*-------- LINK STATION ID 1 --------*/
	stn_ptr = first_stn_ptr;
	while(stn_ptr != NULL){
		/*-------- SEEK SPECIFIED STATION --------*/
		if( strstr(stn_ptr->stn_name, argv[STN_NAME1]) != NULL ){
			printf("STATION %-10s: ID = %2d\n",
				stn_ptr->stn_name, stn_ptr->stn_index );
			stnid_in_cfs[0] = stn_ptr->stn_index;
			break;
		} else {
			stn_ptr = stn_ptr->next_stn_ptr;
		}
	}

	stn_ptr = first_stn_ptr;
	while(stn_ptr != NULL){
		/*-------- SEEK SPECIFIED STATION 2 --------*/
		if( strstr(stn_ptr->stn_name, argv[STN_NAME2]) != NULL ){
			printf("STATION %-10s: ID = %2d\n",
				stn_ptr->stn_name, stn_ptr->stn_index );
			stnid_in_cfs[1] = stn_ptr->stn_index;
			break;
		} else {
			stn_ptr = stn_ptr->next_stn_ptr;
		}
	}
	antnum	= 2;
	blnum	= 1;
	vismap_ptr	= (int *)malloc( 2*blnum*ssnum*sizeof(int) );
/*
------------------------ READ DELAY and DELAY RATE for Specified Station
*/
	for( stn_index=0; stn_index<2; stn_index++){
		/*-------- READ STATION-BASED DELAY and RATE for STN 1 --------*/
		fcaldata_num[stn_index] = read_delay( stnid_in_cfs[stn_index],
			argv[DELAY_CAL], &fcal_addr[stn_index],	&mjd_min,	&mjd_max,
			&rate_min,	&rate_max,	&delay_min,	&delay_max,
			&acc_min,	&acc_max);

		printf(" CFS STN %d : Total %d delay data...\n",
			stnid_in_cfs[stn_index], fcaldata_num[stn_index]);


		if( fcaldata_num[stn_index] != REFANT ){
			/*-------- STORE DELAY DATA --------*/
			restore_delay( fcal_addr[stn_index], fcaldata_num[stn_index],
				&time_ptr_ptr[stn_index], &delay_ptr_ptr[stn_index],
				&rate_ptr_ptr[stn_index], &acc_ptr_ptr[stn_index],
				&delaywgt_ptr_ptr[stn_index],   &ratewgt_ptr_ptr[stn_index],
				&accwgt_ptr_ptr[stn_index]);

			solint  = atoi( argv[SOLINT] );

			/*-------- CALC SPLINE COEFFICIENT for DELAY DATA --------*/
			real_spline( time_ptr_ptr[stn_index],   delay_ptr_ptr[stn_index],
				delaywgt_ptr_ptr[stn_index],        fcaldata_num[stn_index],
				(double)solint, &node_delay_num[stn_index],
				(int)(&delay_coeff[stn_index]), (int)(&time_node[stn_index]) );

			/*-------- CALC SPLINE COEFFICIENT for RATE DATA --------*/
			real_spline( time_ptr_ptr[stn_index],   rate_ptr_ptr[stn_index],
				delaywgt_ptr_ptr[stn_index],        fcaldata_num[stn_index],
				(double)solint, &node_rate_num[stn_index],
				(int)(&rate_coeff[stn_index]), (int)(&time_rate_node[stn_index]) );
		}
	}
/*
------------------------ LOAD BANDPASS DATA TO MEMORY
*/
	for(stn_index=0; stn_index<antnum; stn_index++){
		read_bp(stnid_in_cfs[stn_index], bp_obj_name,
			&bp_ssnum, bp_freq_num, &bp_mjd, &bp_integ_time, 
			bp_rf, bp_freq_incr,
			bp_r_ptr[stn_index], bp_i_ptr[stn_index], bp_vis_max[stn_index]);
	}
/*
------------------------ SEARCH in CORR Pair
*/
	cor_ptr	= first_cor_ptr;
	while( cor_ptr != NULL ){

		/*-------- NORMAL BASELINE DIRECTION --------*/
		if( (cor_ptr->ant1 == stnid_in_cfs[0] )&&
			(cor_ptr->ant2 == stnid_in_cfs[1] )){
			bl_direction	= 1.0;
			break;
		}

		/*-------- INVERSE BASELINE DIRECTION --------*/
		if( (cor_ptr->ant1 == stnid_in_cfs[1] )&&
			(cor_ptr->ant2 == stnid_in_cfs[0] )){
			bl_direction	= -1.0;
			break;
		}
		cor_ptr	= cor_ptr->next_cor_ptr;
	}
	if( cor_ptr == NULL ){
		printf("Can't Find Correlation Pair!!\n"); 
		printf("    TARGET STATION ID = %d, %d\n",
					stnid_in_cfs[0], stnid_in_cfs[1]); 
		exit(0);
	}
/*
------------------------ PREPARE MEMORY AREA FOR VISIBILITY
*/
	for( ss_index=0; ss_index<ssnum; ss_index++){

		ss_index2 = ssid_in_cfs[ss_index];

		/*-------- READ SUB_STREAM HEADDER --------*/
		read_sshead( cor_ptr->cor_id, ss_index2+1, &rf[ss_index],
			&freq_incr[ss_index], &freq_num[ss_index],
			&time_num_cfs, &time_incr);

		bunch_num	= atoi(argv[BUNCH]);
		time_num	= (int)((double)atoi(argv[INTEG]) / time_incr) + 1;
	}
/*
------------------------ START PGPLOT
*/
	if( strstr(argv[DEVICE], "/cps") != NULL){
		sprintf( pg_dev, "pgplot.cps/cps" );
		printf("SAVE PGPLOT TO %s\n", pg_dev );

	} else if( strstr(argv[DEVICE], "/vcps") != NULL){
		sprintf( pg_dev, "pgplot.cps/vcps" );
		printf("SAVE PGPLOT TO %s\n", pg_dev );

	} else if( strstr(argv[DEVICE], "/ps") != NULL){
		sprintf( pg_dev, "pgplot.ps/ps" );
		printf("SAVE PGPLOT TO %s\n", pg_dev );

	} else if( strstr(argv[DEVICE], "/vps") != NULL){
		sprintf( pg_dev, "pgplot.ps/vps" );
		printf("SAVE PGPLOT TO %s\n", pg_dev );

	} else {
		sprintf( pg_dev, "%s", argv[DEVICE] );
	}
	cpgbeg(1, pg_dev, 1, 1);
/*
------------------------ VISIBILITY PLOT
*/
	/*-------- BLOCK INFO --------*/
	sprintf(block_fname, BLOCK_FMT, getenv("CFS_DAT_HOME"), argv[1], cor_ptr->cor_id );
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

	for( ss_index=0; ss_index<ssnum; ss_index++){

		/*-------- SYNTHESIZED BANDPASS TABLE --------*/
		ave_bp_r = (double *)malloc( freq_num[ss_index]* sizeof(double) );
		ave_bp_i = (double *)malloc( freq_num[ss_index]* sizeof(double) );
		ave_bandpass( freq_num[ss_index],
		bp_r_ptr[0][ssid_in_cfs[ss_index]], bp_i_ptr[0][ssid_in_cfs[ss_index]],
		bp_r_ptr[1][ssid_in_cfs[ss_index]], bp_i_ptr[1][ssid_in_cfs[ss_index]],
		ave_bp_r, ave_bp_i );

		memalloc( freq_num[ss_index]/bunch_num, 1,
			&vismap_ptr[ss_index], &vismap_ptr[ssnum + ss_index]);

		valid_pp = integ_vis(
			cor_ptr->cor_id,
			bl_direction,
			obj_id,
			ssid_in_cfs[ss_index]+1,
			&position,
			rf[ss_index],
			freq_incr[ss_index],
			&freq_num[ss_index],
			bunch_num,
			time_num_cfs,
			time_num,
			time_incr,
			start_mjd,
			stop_mjd,
			node_delay_num,
			time_node,
			delay_coeff,
			rate_coeff,
			ave_bp_r,
			ave_bp_i,
			work,
			vismap_ptr[ss_index],
			vismap_ptr[ssnum + ss_index]);

		printf("VALID PP = %d\n", valid_pp);
		printf("SPEC NUM = %d\n", freq_num[ss_index]);
		free(ave_bp_r);
		free(ave_bp_i);
	}

	cpg_vis( argv[OBS_NAME], argv[SOURCE], start_mjd, stop_mjd,
		time_incr*valid_pp, ssnum, ssid_in_cfs, freq_num,
		rf, freq_incr, vismap_ptr, &vismap_ptr[ssnum] );
	cpgend();
/*
------------------------ RELEASE MEMORY AREA FOR BANDPASS
*/
	for(stn_index=0; stn_index<antnum; stn_index++){
		for(ss_index=0; ss_index<first_cor_ptr->nss; ss_index++){
			free(bp_r_ptr[stn_index][ss_index]);
			free(bp_i_ptr[stn_index][ss_index]);
		}
	}

	/*-------- CLOSE SHARED MEMORY --------*/
	shmctl( shrd_obj_id, IPC_RMID, 0 );
	shmctl( shrd_stn_id, IPC_RMID, 0 );

	cpgend();
	return(0);
}
