/*********************************************************
**	CODA_DUMP.C	: DUMP Visibility in CFS and Save		**
**			To a Specified File.						**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 2000/2/3									**
*********************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include "obshead.inc"
#define	MAX_ANT	10
#define	MAX_SS	32	
#define	MAX_CH	1024
#define	MAX_BLOCK	1024
#define	RADDEG	57.29577951308232087721
#define	PI		3.14159265358979323846
#define	PI2		6.28318530717958647692
#define	SECDAY	86400
#define OBS_NAME	1
#define SOURCE		2
#define START		3
#define STOP		4
#define STN_NAME1	5
#define STN_NAME2	6
#define	SS			7
#define OUTFILE		8

MAIN__( argc, argv )
	int		argc;		/* Number of Arguments */
	char	**argv;		/* Pointer of Arguments */
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
	int		*vismap_ptr;				/* Pointer of Visibility Map		*/
	int		fcal_addr[MAX_ANT];			/* Pointer of FCAL Data */

	/*-------- INDEX --------*/
	int		stn_index;					/* Index for Station				*/
	int		stn_index2;					/* Index for Station				*/
	int		bl_index;					/* Index for Baseline				*/
	int		ss_index;					/* Index of Sub-Stream				*/
	int		ss_index2;					/* Index of Sub-Stream				*/
	int		data_index;					/* Index of PP Data					*/
	int		time_index;					/* Index for Time					*/
	int		freq_index;					/* Index for Frequency				*/
	int		nx_index, ny_index;			/* Index of Window					*/
	int		prev_pp;					/* Previous PP Number				*/
	int		block_index;				/* Index of PP Block				*/

	/*-------- TOTAL NUMBER --------*/
	int		stn_num;					/* Total Number of Stations			*/
	int		blnum;						/* Total Number of Baseline			*/
	int		ssnum;						/* Number of Sub-Stream				*/
	int		ssnum_in_cfs;				/* Number of Sub-Stream in CODA		*/
	int		freq_num[MAX_SS];			/* Number of Frequency				*/
	int		bunchfreq_num[MAX_SS];		/* Number of Frequency after BUNCH  */
	int		bunch_num;					/* Number of Bunching				*/
	int		time_num;					/* Number of Time Data in CFS		*/
	int		time_num_cfs;				/* Time Number of CFS				*/
	int		integ_num;					/* Integration PP Number			*/
	int		antnum;						/* Total Number of Selected Station	*/
	int		integ_pp;					/* Coherent Integration PP			*/
	int		valid_pp;					/* Valid PP Number					*/
	int		block_num;					/* Total Number of PP Block			*/
	int		nxwin, nywin;				/* Window Number to PLOT			*/
	int		ppnum_in_block[MAX_BLOCK];	/* PP Number in Each PP Block		*/
	int		ppnum_bunch[MAX_BLOCK];		/* PP Number in after BUNCH			*/

	/*-------- IDENTIFIER --------*/
	int		stnid_in_cfs[10];			/* Station ID Number in CODA		*/
	int		ssid_in_cfs[MAX_SS];		/* SS ID in CODA					*/
	int		position;					/* Start PP Position				*/
	int		obj_id;						/* OBJECT ID						*/
	int		pp_block_ini[MAX_BLOCK];	/* PP Index at Begin of PP Block	*/
	int		*pp_data_ptr;				/* Pointer of PP Data				*/
	int		err_code;					/* Error Code in PGPLOT				*/
	double	*time_data_ptr;				/* Pointer of Time Data				*/
	float	bl_direction;				/* Baseline Direction  (-1 or 1)	*/
	int		ret;						/* Return Code from CFS Library 	*/
	int		lunit;						/* Unit Number of CFS File			*/
	double	pp_incr;					/* PP Increment						*/
	char	fname[128];					/* File Name of CFS Files			*/
	char	omode[2];					/* Access Mode of CFS Files			*/
	char	obj_name[32];				/* Object Name						*/
	double	start_mjd;					/* Start Time [MJD]					*/
	double	stop_mjd;					/* Stop Time [MJD]					*/
	double	rf[MAX_SS];					/* RF Frequency [MHz]				*/
	double	freq_incr[MAX_SS];			/* Frequency Increment [MHz]		*/
	double	bunchfreq_incr[MAX_SS];		/* Frequency Increment [MHz]		*/
	double	time_incr;					/* Time Increment [sec]				*/
	double	current_mjd;				/* Current MJD [MJD]				*/
	float	work[2*MAX_CH];				/* Work Area to Read Visibility		*/
	float	vismax;						/* Maximum Visibility Amp			*/
	int		start_time;					/* START TIME [DDDHHMMSS]			*/
	int		stop_time;					/* STOP TIME [DDDHHMMSS]			*/
	int		year, doy;					/* YEAR and Day of Year				*/
	int		hour;						/* Hour								*/
	int		min;						/* Minute							*/
	double	sec;						/* Second							*/
	double	loss;
	double	mjd_min,	mjd_max;		/* MAX and Min of MJD				*/
	float	xmin,		xmax;			/* Plot Range						*/
	float	amp_min,	amp_max;		/* Amplitude Min and Max to Plot	*/
	float	phs_min,	phs_max;		/* Phase Min and Max to Plot		*/
	float	xwin_incr,	ywin_incr;		/* Phase Min and Max to Plot		*/
	char	pg_dev[256];				/* PGPLOT Device Name				*/
	char	stn_name1[16];				/* Station Name						*/
	char	stn_name2[16];				/* Station Name						*/
	char	text[64];					/* Station Name						*/


/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 9 ){
		printf("USAGE : coda_dump [OBS_NAME] [SOURCE] [START] [STOP] [STN_NAME1] [STN_NAME2] [SS] [OUTPUT FILE]!!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  SOURCE -------- Source Name\n");
		printf("  START --------- Start TIME [DDDHHMMSS] \n");
		printf("  STOP ---------- Stop TIME  [DDDHHMMSS] \n");
		printf("  STN_NAME1 ----- STATION NAME \n");
		printf("  STN_NAME2 ----- STATION NAME \n");
		printf("  SS ------------ Sub-Stream Number \n");
		printf("  OUTPUT FILE --- File Name to Save \n");
		exit(0);
	}

	/*-------- START and STOP Time --------*/
	start_time	= atoi(argv[START]);
	stop_time	= atoi(argv[STOP]);
	sprintf(obj_name, "%s", argv[SOURCE]);
	amp_max = (float)atof(argv[AMP]);

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
	read_obshead(lunit, &obs, &obj_ptr, &stn_ptr, &shrd_obj_id, &shrd_stn_id );
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
------------------------ CALC DELAY and DELAY RATE vs ELEVATION
*/
	printf("STATION 1 : %s\n", argv[STN_NAME1]);
	printf("STATION 2 : %s\n", argv[STN_NAME2]);

	/*-------- LINK STATION ID 1 --------*/
	stn_ptr = first_stn_ptr;
	while(stn_ptr != NULL){
		/*-------- SEEK SPECIFIED STATION --------*/
		if( strstr(stn_ptr->stn_name, argv[STN_NAME1]) != NULL ){
			printf("STATION %-10s: ID = %2d\n",
				stn_ptr->stn_name, stn_ptr->stn_index );
			sprintf(stn_name1, "%s", stn_ptr->stn_name);
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
			sprintf(stn_name2, "%s", stn_ptr->stn_name);
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

		/*-------- PP NUMBER ROUND UP TO 2^n --------*/
		time_num	= (int)( atof(argv[INTEG])/time_incr + 0.5 );
		if(time_num == 0){	time_num = 1;}
		bunch_num	= atoi(argv[BUNCH]);
	}
	time_data_ptr	= (double *)malloc( time_num_cfs * sizeof(double) );
	pp_data_ptr		= (int *)malloc( time_num_cfs * sizeof(int) );
	integ_num		= (int)(atof(argv[INTEG]) / time_incr );
	if( integ_num < 1){ integ_num = 1; }
/*
------------------------ VISIBILITY PLOT
*/
	current_mjd	= start_mjd;
	position	= -1;
/*
------------------------ SCAN TIME POINTS IN CFS
*/
	printf(" Checking Time Tags in CODA..."); 
	valid_pp = time_span_coda( cor_ptr->cor_id, ssid_in_cfs[0]+1, obj_id,
			&position, freq_num[0], time_num_cfs, time_incr,
			start_mjd, stop_mjd,	time_data_ptr,	pp_data_ptr );
	printf(" DONE! Valid PP = %d\n", valid_pp); 
	fmjd2doy( time_data_ptr[0], &year, &doy, &hour, &min, &sec );
	printf("INITIAL TIME = %03d %02d:%02d:%02d\n",
		doy, hour, min, (int)sec ); 
/*
------------------------ SELECT TIME (PP) BLOCK
*/
	prev_pp		= -9999;
	block_index	= 0;
	for( data_index=0; data_index<valid_pp; data_index++){
		if( (pp_data_ptr[data_index] - prev_pp) != 1 ){
			pp_block_ini[block_index]	= pp_data_ptr[data_index];
			ppnum_in_block[block_index] = 0;
			block_index ++;
		}

		ppnum_in_block[block_index-1] ++;
		prev_pp = pp_data_ptr[data_index];
	}
	block_num	= block_index;
	printf(" TOTAL %d DATA BLOCKS!!\n", block_num);
	for( block_index=0; block_index<block_num; block_index++){
		ppnum_bunch[block_index] = ppnum_in_block[block_index];
	}

	stn_index	= ssid_in_cfs[0];
	stn_index2	= ssid_in_cfs[1];

	/*-------- SETTING for PGPLOT MULTI-WINDOW --------*/
	nywin = (int)sqrt((double)(ssnum));
	nxwin = (ssnum + nywin - 1)/nywin;
	printf(" NXWIN = %d, NYWIN = %d\n", nxwin, nywin);
	xwin_incr = 0.9 / (float)nxwin;
	ywin_incr = 0.9 / (float)nywin;
	xmin = (float)(( 1.1* start_mjd- 0.1* stop_mjd- (int)start_mjd)* SECDAY);
	xmax = (float)((-0.1* start_mjd+ 1.1* stop_mjd- (int)start_mjd)* SECDAY);
	amp_min = 0.0;  amp_max = (float)atof(argv[AMP]);
	phs_min = -PI;  phs_max = PI;

	for( block_index=0; block_index<block_num; block_index++){

		position	= pp_block_ini[block_index];
		obj_ptr	= first_obj_ptr;
		stn_ptr	= first_stn_ptr;

		for( ss_index=0; ss_index<ssnum; ss_index++){
			ss_index2 = ssid_in_cfs[ss_index];
			bunchfreq_num[ss_index] = freq_num[ss_index];
			bunchfreq_incr[ss_index] = freq_incr[ss_index];
			ppnum_bunch[block_index] = ppnum_in_block[block_index];

			/*-------- ALLOCATE MEMORY AREA --------*/
			if( memalloc( freq_num[ss_index]/bunch_num,
				ppnum_in_block[block_index],
				&vismap_ptr[ss_index],
				&vismap_ptr[ssnum + ss_index]) == -1 ){
				printf("memalloc(): Failed to Allocate Memory ...SS=%d\n",
				ss_index2 );
			}

			load_vis_raw( cor_ptr->cor_id, bl_direction, obj_id, ss_index2+1,
				&position, 
				&bunchfreq_num[ss_index], bunch_num, &bunchfreq_incr[ss_index],
				time_num_cfs, &ppnum_bunch[block_index], integ_num,
				time_incr, time_data_ptr,
				work, vismap_ptr[ss_index], vismap_ptr[ssnum + ss_index]);

		}
		obj_ptr	= first_obj_ptr;
		stn_ptr	= first_stn_ptr;



		time_data_ptr += ppnum_in_block[block_index];

	}
/*
------------------------ RELEASE MEMORY AREA FOR BANDPASS
*/
	/*-------- CLOSE SHARED MEMORY --------*/
	shmctl( shrd_obj_id, IPC_RMID, 0 );
	shmctl( shrd_stn_id, IPC_RMID, 0 );

	cpgend();
	return(0);
}
