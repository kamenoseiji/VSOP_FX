/*********************************************************
**	CODA_ANIM.C	: Integrate Visibility in CFS and Disp	**
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
#define	PI		3.14159265358979323846
#define	PI2		6.28318530717958647692
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
#define AMP			10
#define STN_NAME1	11
#define STN_NAME2	12
#define	TOTAL_SS	13
#define SS_ARG		14

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
	int		time_ptr_ptr[2];			/* Index for Time Series			*/
	int		delay_ptr_ptr[2];			/* Index for Time Series			*/
	int		rate_ptr_ptr[2];			/* Index for Time Series			*/
	int		acc_ptr_ptr[2];				/* Index for Time Series			*/
	int		delaywgt_ptr_ptr[2];		/* Index for Time Series			*/
	int		ratewgt_ptr_ptr[2];			/* Index for Time Series			*/
	int		accwgt_ptr_ptr[2];			/* Index for Time Series			*/

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
	int		bp_ssnum;					/* SS-Number in Band-Pass Filei		*/
	int		bp_freq_num[MAX_SS];		/* Frequency Channel Number			*/
	int		node_delay_num[MAX_ANT];	/* Number of Node Points in SPLINE	*/
	int		node_rate_num[MAX_ANT];		/* Number of Node Points in SPLINE	*/

	/*-------- IDENTIFIER --------*/
	int		refant_id;					/* REFANT ID in CFS					*/
	int		stnid_in_cfs[10];			/* Station ID Number in CODA		*/
	int		ssid_in_cfs[MAX_SS];		/* SS ID in CODA					*/
	int		position;					/* Start PP Position				*/
	int		obj_id;						/* OBJECT ID						*/
	int		delaydata_num[2];			/* Number of Delay Data 			*/
	int		pp_block_ini[MAX_BLOCK];	/* PP Index at Begin of PP Block	*/
	int		delay_coeff[MAX_ANT];		/* Pointer of SPLINE coefficient	*/
	int		rate_coeff[MAX_ANT];		/* Pointer of SPLINE coefficient	*/
	int		time_delay_node[MAX_ANT];	/* Node Points in SPLINE			*/
	int		time_rate_node[MAX_ANT];	/* Node Points in SPLINE			*/
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
	double	bldelay;					/* Baseline-Based Delay [microsec]	*/
	double	blrate;						/* Baseline-Based Rate [picosec/sec]*/
	float	work[2*MAX_CH];				/* Work Area to Read Visibility		*/
	float	vismax;						/* Maximum Visibility Amp			*/
	int		start_time;					/* START TIME [DDDHHMMSS]			*/
	int		stop_time;					/* STOP TIME [DDDHHMMSS]			*/
	int		year, doy;					/* YEAR and Day of Year				*/
	int		hour;						/* Hour								*/
	int		min;						/* Minute							*/
	int		solint;						/* Solution Interval [sec]			*/
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
	double	loss;
	double	mjd_min,	mjd_max;		/* MAX and Min of MJD				*/
	double	rate_min,	rate_max;		/* MAX and Min of Rate				*/
	double	delay_min,	delay_max;		/* MAX and Min of Delay				*/
	double	acc_min,	acc_max;		/* MAX and Min of MJD				*/
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
	if( argc < 10 ){
		printf("USAGE : coda_anim [OBS_NAME] [SOURCE] [DELAY CAL] [START] [STOP] [INTEG] [BUNCH] [DEVICE] [AMP] [STN_NAME1] [STN_NAME2] [TOTAL_SS] [SS1] [SS2] ... !!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  SOURCE -------- Source Name\n");
		printf("  DELAY CAL ----- Source Name of Delays Calibrator\n");
		printf("  START --------- Start TIME [DDDHHMMSS] \n");
		printf("  STOP ---------- Stop TIME  [DDDHHMMSS] \n");
		printf("  INTEG --------- Integration TIME  [sec] \n");
		printf("  DEVICE -------- PGPLOT Device \n");
		printf("  STN_NAME ------ STATION NAMEs \n");
		printf("  TOTAL_SS ------ TOTAL STATION NUMBER \n");
		exit(0);
	}

	/*-------- START and STOP Time --------*/
	start_time	= atoi(argv[START]);
	stop_time	= atoi(argv[STOP]);
	sprintf(obj_name, "%s", argv[SOURCE]);
	solint		= atoi(argv[SOLINT]);
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
	/*-------- LINK STATION ID 1 --------*/
	stn_ptr = first_stn_ptr;
	while(stn_ptr != NULL){
		/*-------- SEEK SPECIFIED STATION --------*/
		if( strstr(stn_ptr->stn_name, argv[STN_NAME1]) != NULL ){
			printf("STATION %-10s: ID = %2d\n",
				stn_ptr->stn_name, stn_ptr->stn_index );
			sprintf(stn_name1, "%s", stn_ptr->stn_name);
			stnid_in_cfs[0] = stn_ptr->stn_index;

			/*-------- READ STATION-BASED DELAY and RATE --------*/
			delaydata_num[0] = read_delay( stn_ptr->stn_index, argv[DELAY_CAL],
				&fcal_addr[0],	&mjd_min,	&mjd_max,
				&rate_min,	&rate_max,	&delay_min,	&delay_max,
				&acc_min,	&acc_max);
			if( delaydata_num[0]  == -1){
				refant_id	= stn_ptr->stn_index;
			}

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

			/*-------- READ STATION-BASED DELAY and RATE --------*/
			delaydata_num[1] = read_delay( stn_ptr->stn_index, argv[DELAY_CAL],
				&fcal_addr[1],	&mjd_min,	&mjd_max,
				&rate_min,	&rate_max,	&delay_min,	&delay_max,
				&acc_min,	&acc_max);
			if( delaydata_num[1]  == -1){
				refant_id	= stn_ptr->stn_index;
			}

			break;
		} else {
			stn_ptr = stn_ptr->next_stn_ptr;
		}
	}

	antnum	= 2;
	blnum	= 1;
	vismap_ptr	= (int *)malloc( 2*blnum*ssnum*sizeof(int) );
/*
------------------------ CALC SPLINE COEFF
*/
	for(stn_index=0; stn_index<antnum; stn_index++){
		if(delaydata_num[stn_index] > 0){
			restore_delay( fcal_addr[stn_index], delaydata_num[stn_index],
				&time_ptr_ptr[stn_index],	&delay_ptr_ptr[stn_index],
				&rate_ptr_ptr[stn_index],	&acc_ptr_ptr[stn_index],
				&delaywgt_ptr_ptr[stn_index],	&ratewgt_ptr_ptr[stn_index],
				&accwgt_ptr_ptr[stn_index]);

			real_spline(time_ptr_ptr[stn_index],	delay_ptr_ptr[stn_index],
				delaywgt_ptr_ptr[stn_index],	delaydata_num[stn_index],
				(double)solint, &node_delay_num[stn_index],
				(int)(&delay_coeff[stn_index]),
				(int)(&time_delay_node[stn_index]) );

			real_spline(time_ptr_ptr[stn_index],	rate_ptr_ptr[stn_index],
				ratewgt_ptr_ptr[stn_index],	delaydata_num[stn_index],
				(double)solint, &node_rate_num[stn_index],
				(int)(&rate_coeff[stn_index]),
				(int)(&time_rate_node[stn_index]) );
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
------------------------ START PGPLOT
*/
	if( strstr(argv[DEVICE], "/cps") != NULL){
		sprintf( pg_dev, "pgplot.cps/cps" );
		cpgbeg(1, pg_dev, 1, 1);
		printf("SAVE PGPLOT TO %s\n", pg_dev );
		cpgscrn(0, "White", &err_code);			/* COLOR DEFINITION */
		cpgscrn(1, "Black", &err_code);			/* COLOR DEFINITION */
		cpgscrn(2, "ivory", &err_code);			/* COLOR DEFINITION */
		cpgscrn(3, "Blue", &err_code);			/* COLOR DEFINITION */
		cpgscrn(4, "coral", &err_code);			/* COLOR DEFINITION */

	} else if( strstr(argv[DEVICE], "/vcps") != NULL){
		sprintf( pg_dev, "pgplot.cps/vcps" );
		cpgbeg(1, pg_dev, 1, 1);
		printf("SAVE PGPLOT TO %s\n", pg_dev );
		cpgscrn(0, "White", &err_code);			/* COLOR DEFINITION */
		cpgscrn(1, "Black", &err_code);			/* COLOR DEFINITION */
		cpgscrn(2, "ivory", &err_code);			/* COLOR DEFINITION */
		cpgscrn(3, "Blue", &err_code);			/* COLOR DEFINITION */
		cpgscrn(4, "coral", &err_code);			/* COLOR DEFINITION */

	} else if( strstr(argv[DEVICE], "/ps") != NULL){
		sprintf( pg_dev, "pgplot.ps/ps" );
		cpgbeg(1, pg_dev, 1, 1);
		printf("SAVE PGPLOT TO %s\n", pg_dev );
		cpgscrn(0, "White", &err_code);			/* COLOR DEFINITION */
		cpgscrn(1, "Black", &err_code);			/* COLOR DEFINITION */
		cpgscrn(2, "LightGrey", &err_code);		/* COLOR DEFINITION */
		cpgscrn(3, "Black", &err_code);			/* COLOR DEFINITION */
		cpgscrn(4, "Black", &err_code);			/* COLOR DEFINITION */

	} else if( strstr(argv[DEVICE], "/vps") != NULL){
		sprintf( pg_dev, "pgplot.ps/vps" );
		cpgbeg(1, pg_dev, 1, 1);
		printf("SAVE PGPLOT TO %s\n", pg_dev );
		cpgscrn(0, "White", &err_code);			/* COLOR DEFINITION */
		cpgscrn(1, "Black", &err_code);			/* COLOR DEFINITION */
		cpgscrn(2, "LightGrey", &err_code);		/* COLOR DEFINITION */
		cpgscrn(3, "Black", &err_code);			/* COLOR DEFINITION */
		cpgscrn(4, "Black", &err_code);			/* COLOR DEFINITION */

	} else {
		sprintf( pg_dev, "%s", argv[DEVICE] );
		cpgbeg(1, pg_dev, 1, 1);
		cpgscrn(0, "DarkSlateGrey", &err_code);	/* COLOR DEFINITION */
		cpgscrn(1, "White", &err_code);			/* COLOR DEFINITION */
		cpgscrn(2, "SlateGrey", &err_code);		/* COLOR DEFINITION */
		cpgscrn(3, "Yellow", &err_code);		/* COLOR DEFINITION */
		cpgscrn(4, "Cyan", &err_code);			/* COLOR DEFINITION */
	}
	cpgeras();
	cpgsvp(0.0, 1.0, 0.0, 1.0);
	cpgswin(0.0, 1.0, 0.0, 1.0);
	cpgsci(1); cpgsch(1.0);
	cpgtext(0.35, 0.975, "Visibility Amplitude and Phase");
	cpgtext(0.45, 0.005, "Time");
	cpgsch(0.5);
	sprintf(text, "EXPER : %s", argv[OBS_NAME]);    cpgtext(0.70, 0.99, text);
	sprintf(text, "SOURCE: %s", obj_name);          cpgtext(0.70, 0.975, text);
	sprintf(text, "RANGE : %4d %03d %02d:%02d:%02d - %02d:%02d:%02d", year,
	doy,(start_time/10000)%100, (start_time/100)%100, start_time%100,
	(stop_time/10000)%100,  (stop_time/100)%100,  stop_time%100);
	cpgtext(0.70, 0.960, text);
	sprintf(text, "BASELINE: %s - %s", stn_name1, stn_name2);
	cpgtext(0.70, 0.945, text);
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
	cpgsch(0.7);
	for( ss_index=0; ss_index<ssnum; ss_index++){
		nx_index = (ss_index) % nxwin;
		ny_index = (ss_index) / nxwin;
		cpgsvp( 0.067+xwin_incr*nx_index, 0.067+xwin_incr*(nx_index+0.9),
			0.067+ywin_incr*ny_index, 0.067+ywin_incr*(ny_index+0.45) );
		cpgswin( xmin, xmax, phs_min, phs_max);
		cpgsci(2);  cpgrect(xmin, xmax, phs_min, phs_max);
		cpgsci(0); cpgtbox( "G", 0.0, 0, "G", 0.0, 0);
		cpgsci(1); cpgtbox( "BCNTSZH", 0.0, 0, "BCNTS", 0.0, 0);
		if(ss_index == 0){  cpglab("", "Phase [rad]", ""); }

		cpgsvp( 0.067+xwin_incr*nx_index, 0.067+xwin_incr*(nx_index+0.9),
			0.067+ywin_incr*(ny_index+0.45),0.067+ywin_incr*(ny_index+0.9));
		cpgswin( xmin, xmax, amp_min, amp_max);
		cpgsci(2);  cpgrect(xmin, xmax, amp_min, amp_max);
		cpgsci(0); cpgtbox( "G", 0.0, 0, "G", 0.0, 0);
		cpgsci(1); cpgtbox( "BCTSZH", 0.0, 0, "BCNTS", 0.0, 0);
		sprintf(text, "IF %d", ssid_in_cfs[ss_index]+1);
		cpgtext( xmin* 0.95 + xmax* 0.05, amp_min*0.1 + amp_max* 0.9, text);
		if(ss_index == 0){  cpglab("", "Corr. Coeff.", ""); }
	}

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

			load_vis_anim( cor_ptr->cor_id, bl_direction, obj_id, ss_index2+1,
				&position, 
				&bunchfreq_num[ss_index], bunch_num, &bunchfreq_incr[ss_index],
				(rf[ss_index] + 0.5* freq_incr[ss_index]* freq_num[ss_index]),
				time_num_cfs, &ppnum_bunch[block_index], integ_num,
				time_incr, time_data_ptr, mjd_min,
				node_delay_num, time_delay_node, delay_coeff, rate_coeff,
				bp_r_ptr[0][ss_index2], 
				bp_r_ptr[1][ss_index2], 
				work, vismap_ptr[ss_index], vismap_ptr[ssnum + ss_index]);

			phase_track(ppnum_bunch[block_index], time_data_ptr, mjd_min,
				bunchfreq_num[ss_index], solint, ss_index, ssnum,
				xmin, xmax, amp_min, amp_max, phs_min, phs_max,
				vismap_ptr[ss_index], vismap_ptr[ssnum + ss_index] );
		}
		sleep(3);
		obj_ptr	= first_obj_ptr;
		stn_ptr	= first_stn_ptr;


/*
		cpg_vis_anim( stn_name1, stn_name2, argv[OBS_NAME], obj_name,
			time_data_ptr, ppnum_bunch[block_index], time_incr,
			ssnum, ssid_in_cfs, bunchfreq_num, rf, bunchfreq_incr,
			amp_max, vismap_ptr, &vismap_ptr[ssnum]);
*/

		cpg_spec( stn_name1, stn_name2, argv[OBS_NAME], obj_name,
			time_data_ptr, ppnum_bunch[block_index], time_incr,
			ssnum, ssid_in_cfs, bunchfreq_num, rf, bunchfreq_incr,
			amp_max, vismap_ptr, &vismap_ptr[ssnum]);





/*
		for( ss_index=0; ss_index<ssnum; ss_index++){
			memfree(&vismap_ptr[ss_index], &vismap_ptr[ssnum + ss_index]);
		}
*/

		time_data_ptr += ppnum_in_block[block_index];

/*
		for( ss_index=0; ss_index<ssnum; ss_index++){
			free( &vismap_ptr[ss_index] );
			free( &vismap_ptr[ssnum + ss_index] );
			free( vismap_ptr[ss_index] );
			free( vismap_ptr[ssnum + ss_index] );
		}
*/

	}
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
