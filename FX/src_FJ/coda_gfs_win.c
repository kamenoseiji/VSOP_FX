/*********************************************************
**	CODA_GFS_WIN.C	: Global Fringe Search using 		**
**					CODA File System					**
**					with Limited Search Window			**
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

#define	ARG_OBS_NAME			1
#define	ARG_SOURCE				2
#define	ARG_START				3
#define	ARG_STOP				4
#define	ARG_INTEG				5
#define	ARG_DELAY_WIN_CENTER	6
#define	ARG_DELAY_WIN_WIDTH		7
#define	ARG_RATE_WIN_CENTER		8
#define	ARG_RATE_WIN_WIDTH		9
#define	ARG_SNR_LIMIT			10
#define	ARG_DEVICE				11
#define	ARG_STN_NUM				12
#define	ARG_SS_NUM				13
#define	ARG_STN_NAME			14

#define CP_DIR	"/home/kameno/FX/bin"
#define	MAX_ANT	10
#define	MAX_SS	32	
#define	MAX_CH	1024
#define	RADDEG		57.29577951308232087721
#define MATCH11  (cor_ptr->ant1 == stnid_in_cfs[stn_index])
#define MATCH22  (cor_ptr->ant2 == stnid_in_cfs[stn_index2])
#define MATCH12  (cor_ptr->ant1 == stnid_in_cfs[stn_index2])
#define MATCH21  (cor_ptr->ant2 == stnid_in_cfs[stn_index])

#define	VISIB_FMT	"VISIBILITY %03d%02d%02d%02d BL%2d : SS=%2d: AMP[%%]= %7.5lf +/- %8.6lf, PHS[deg]= %7.2lf +/- %7.3lf\n"
#define	UVW_FMT		"UVW        %03d%02d%02d%02d BL%2d : UVW= %9.3lf %9.3lf %9.3lf, d(UVW)/dt= %9.4lf %9.4lf %9.4lf\n"
#define	DELAY_FMT	" STN %d : DELAY= %7.5lf +/- %8.7lf, RATE= %7.5lf +/- %8.7lf\n"


MAIN__(
	int		argc,			/* Number of Arguments			*/
	char	**argv,			/* Pointer to Arguments			*/
	char	**envp)			/* Pointer to Environments		*/
{

	/*-------- STRUCTURE DEFINITION --------*/
	struct	header		obs;			/* OBS HEADDER						*/
	struct	head_obj	*obj_ptr;		/* Pointer of Objects Header		*/
	struct	head_obj	*first_obj_ptr;	/* First Pointer of Objects Header	*/
	struct	head_stn	*stn_ptr;		/* Pointer of Station Header		*/
	struct	head_stn	*first_stn_ptr;	/* First Pointer of Station Header	*/
	struct	head_cor	*cor_ptr;		/* Pointer of CORR Header			*/
	struct	head_cor	*first_cor_ptr;	/* First Pointer of CORR Header		*/

	/*-------- IDENTIFIER --------*/
	int		shrd_obj_id;				/* Shared Memory ID for Source HD	*/
	int		shrd_stn_id;				/* Shared Memory ID for Station HD	*/
	int		shrd_vismap_id;				/* Shared Memory ID for memory map	*/
	int		shrd_vis_id[45][16];		/* Shared Memory ID for Vis			*/
	int		obj_id;						/* OBJECT ID						*/
	int		ret;						/* Return Code from CFS Library		*/
	int		lunit;						/* Unit Number of CFS File			*/
	int		ss_id[MAX_SS];				/* SS ID in CFS						*/
	int		position;					/* PP Position in CFS				*/
	int		delay_pos;					/* Peak Position for Delay			*/ 
	int		rate_pos;					/* Peak Position for Rate			*/ 
	int		icon;						/* Condition Code					*/
	int		stnid_in_cfs[10];			/* Station ID Number in CODA		*/
	int		blid_in_cfs[45];			/* Baseline ID Number in CODA		*/
	int		stn_arg;					/* Argument Index which STATION starts*/
	int		init_win_x, init_win_y;		/* Search Start Position in (X,Y)	*/

	/*-------- INDEX --------*/
	int		stn_index;					/* Index for Station				*/
	int		stn_index2;					/* Index for Station				*/
	int		bl_index;					/* Index for Baseline				*/
	int		ss_index;					/* Index of Sub-Stream				*/
	int		time_index;					/* Index for Time					*/
	int		freq_index;					/* Index for Frequency				*/

	/*-------- TOTAL NUMBER --------*/
	int		blnum;						/* Total Number of Baseline			*/
	int		ssnum;						/* Number of Sub-Stream				*/
	int		freq_num[MAX_SS];			/* Number of Frequency				*/
	int		bunch_num;					/* Number of Bunching				*/
	int		valid_pp;					/* Valid PP Number					*/
	int		time_num_cfs;				/* Number of Time Data				*/
	int		time_num;					/* Number of Time Data				*/
	int		integ_pp;					/* Coherent Integration PP			*/
	int		wind_x, wind_y;				/* Nyquist Window Size (X, Y)		*/
	int		width_win_x, width_win_y;	/* Search Window Size (X, Y)		*/
	int		antnum;						/* Total Number of Selected Station	*/
	int		bp_ssnum;					/* SS-Number in Band-Pass File		*/
	int		bp_freq_num[MAX_SS];		/* Frequency Channel Number			*/

	/*-------- TIME VARIABLES --------*/
	double	start_mjd;					/* Start Time [MJD]					*/
	double	stop_mjd;					/* Stop Time [MJD]					*/
	double	time_incr;					/* Time Increment [sec]				*/
	int		start_time;					/* START TIME [DDDHHMMSS]			*/
	int		stop_time;					/* STOP TIME [DDDHHMMSS]			*/
	int		year, doy;					/* YEAR and Day of Year				*/
	int		hour, min;					/* Hour and Minutre					*/ 
	double	sec;						/* Second							*/
	double	bp_mjd;						/* MJD of the Band-Pass File		*/
	double	bp_integ_time;				/* Integ Time of the Band-Pass File	*/

	/*-------- CHARACTER VARIABLES --------*/
	key_t	vismap_key;					/* Keyword for Shared memory map	*/
	char	fname[128];					/* File Name of CFS Files			*/
	char	omode[2];					/* Access Mode of CFS Files			*/
	char	obj_name[32];				/* Object Name						*/
	char	bp_obj_name[32];			/* Object Name in Band-Pass File	*/
	char	pg_text[64];				/* TEXT to Plot						*/
	char	stn_name[10][32];			/* TEXT to Plot						*/

	/*-------- POINTER VARIABLES --------*/
	int		*shrd_vismap_ptr;			/* First Pointer of memory map		*/
	float	*visamp_ptr;				/* Pointer of Visibility Amplitude	*/
	double	*bp_r_ptr[MAX_ANT][MAX_SS];	/* Pointer of Bandpass (REAL)		*/
	double	*bp_i_ptr[MAX_ANT][MAX_SS];	/* Pointer of Bandpass (IMAG)		*/

	/*-------- VALUE --------*/
	float	bl_direction;				/* Baseline Direction				*/
	double	uvw_bl[45][6];				/* Baseline Vector from the Obj [m]	*/
	double	loss;						/* Quantize Efficiency				*/
	double	rf[MAX_SS];					/* RF Frequency [MHz]				*/
	double	freq_incr[MAX_SS];			/* Frequency Increment [MHz]		*/
	double	*bl_delay_ptr;				/* Residual Delay (BL-Based)		*/
	double	*bl_delay_err;				/* Residual Delay Error				*/
	double	*bl_rate_ptr;				/* Residual Delay Rate (BL-Based)	*/
	double	*bl_rate_err;				/* Residual Rate Error				*/
	double	*ant_delay_ptr;				/* Residual Delay (ANT-Based)		*/
	double	*ant_delay_err;				/* Residual Delay Error				*/
	double	*ant_rate_ptr;				/* Residual Delay Rate (ANT-Based)	*/
	double	*ant_rate_err;				/* Residual Rate Error				*/
	double	gff_snr;					/* SNR of GFF Results				*/
	float	vis_snr;					/* SNR of Visivility				*/
	float	vismax;						/* Maximum Visibility Amp			*/
	float	*gff_result;				/* RESULT of GFF					*/
	float	*gff_err;					/* ERROR of GFF						*/
	double	coeff[5];					/* Square-Fit Coefficient			*/
	float	work[2*MAX_CH];				/* Work Area to Read Visibility		*/
	float	x_incr, y_incr;				/* Increment of Delay and Rate		*/
	double	bp_rf[MAX_SS];				/* RF Freq. [MHz] in BP File		*/
	double	bp_freq_incr[MAX_SS];		/* Freq. Increment [MHz] in BP File	*/
	double	bp_vis_max[MAX_ANT][MAX_SS];/* Maximum of Bandpass				*/
	float	win_x_min,	win_y_min;		/* Window Edge						*/

	/*-------- BLOCK INFO --------*/
	char		ppblock_fname[128];	/* Block File Name				*/
	FILE		*ppblock_file;		/* Block File					*/
	struct block_info   ppblock[MAX_BLOCK];	/* Block Information	*/
	int			ppblock_num;			/* Total Number of Blocks		*/

/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 11 ){
		printf("USAGE : coda_gfs_win [OBS_NAME] [SOURCE] [START] [STOP] [INTEG] [DELAY CENTER] [DELAY WIDTH] [RATE CENTER] [RATE WIDTH] [SNR LIMIT] [DEVICE] [TOTAL STN] [TOTAL SS] [STN_NAME1] [STN_NAME2] ... [SS1] [SS2]... !!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  SOURCE -------- Source Name for Band-Pass Calib\n");
		printf("  START --------- Start TIME [DDDHHMMSS] \n");
		printf("  STOP ---------- Stop TIME  [DDDHHMMSS] \n");
		printf("  INTEG --------- Integration TIME  [sec] \n");
		printf("  DELAY CENTER -- Search Delay Window Center [microsec] \n");
		printf("  DELAY WIDTH --- Search Delay Window Width [microsec] \n");
		printf("  RATE CENTER --- Search Rate Window Center [picosec/sec] \n");
		printf("  RATE WIDTH ---- Search Rate Window Width [picosec/sec] \n");
		printf("  SNR LIMIT ----- Fringe Detection Threshold \n");
		printf("  DEVICE -------- PGPLOT Device \n");
		printf("  TOTAL STN ----- TOTAL NUMBER OF STATION\n");
		printf("  TOTAL SS ------ TOTAL NUMBER OF SUB-STREAM\n");
		printf("  STN_NAME ------ STATION NAMEs \n");
		exit(0);
	}

	/*-------- START and STOP Time --------*/
	start_time	= atoi(argv[ARG_START]);
	stop_time	= atoi(argv[ARG_STOP]);
	sprintf(obj_name, "%s", argv[ARG_SOURCE]);

/*
------------------------ ACCESS TO THE CODA FILE SYSTEM (CFS)
*/
	cfs000_( &ret );			cfs_ret( 000, ret );
	cfs020_( &ret );			cfs_ret( 020, ret );
	cfs006_( argv[ARG_OBS_NAME], &ret, strlen( argv[ARG_OBS_NAME] ));
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

	/*-------- LINK STATION ID --------*/
	antnum = 0;
	for(stn_arg=ARG_STN_NAME; stn_arg<ARG_STN_NAME+atoi(argv[ARG_STN_NUM]);
				stn_arg++){
		stn_ptr = first_stn_ptr;
		while(stn_ptr != NULL){

			/*-------- SEEK SPECIFIED STATION --------*/
			if( strstr(stn_ptr->stn_name, argv[stn_arg]) != NULL ){
				printf("STATION %-10s: ID = %2d\n",
					stn_ptr->stn_name, stn_ptr->stn_index );
				stnid_in_cfs[antnum] = stn_ptr->stn_index;
				strcpy(stn_name[stn_arg - ARG_STN_NAME], stn_ptr->stn_name);
				antnum++;
				break;
			} else {
				stn_ptr = stn_ptr->next_stn_ptr;
			}
		}
	}
/*
------------------------ NUMBER OF BASELINE AND SUB_STREAM
*/
	printf(" TOTAL %d REAL STATIONS.\n", antnum);
	blnum	= (antnum * (antnum - 1))/2;
	ssnum	= atoi(argv[ARG_SS_NUM]);
	wind_x	= 32768;
	for(ss_index=0; ss_index<ssnum; ss_index++){
		ss_id[ss_index] = atoi(argv[ARG_STN_NAME+antnum+ss_index]);

		/*-------- CHECK MAX FREQ NUMBER -> SEARCH WINDOW --------*/
		read_sshead( 1, ss_id[ss_index]+1, &rf[ss_index],
			&freq_incr[ss_index], &freq_num[ss_index], 
			&time_num_cfs, &time_incr);
		if( freq_num[ss_index] < wind_x){ wind_x = freq_num[ss_index]; }
	}
	wind_x = pow2round( wind_x );
	wind_x *= 2;

	if(wind_x > MAX_CH){ wind_x = MAX_CH; }
/*
------------------------ LOAD BANDPASS DATA TO MEMORY
*/
	printf("READING BANDPASS DATA\n");
	for(stn_index=0; stn_index<antnum; stn_index++){
		read_bp(stnid_in_cfs[stn_index], bp_obj_name,
			&bp_ssnum, bp_freq_num, &bp_mjd, &bp_integ_time, 
			bp_rf, bp_freq_incr,
			bp_r_ptr[stn_index], bp_i_ptr[stn_index], bp_vis_max[stn_index]);
	}
	printf("....DONE!!\n");
/*
------------------------ PREPARE MEMORY AREA
*/
	/*-------- ALLOC MEMORY AREA FOR BASELINE-BASE DELAY and RATE --------*/
	bl_delay_ptr	= (double *)malloc(blnum*sizeof(double));
	bl_delay_err	= (double *)malloc(blnum*sizeof(double));
	bl_rate_ptr		= (double *)malloc(blnum*sizeof(double));
	bl_rate_err		= (double *)malloc(blnum*sizeof(double));
	ant_delay_ptr	= (double *)malloc(antnum*sizeof(double));
	ant_delay_err	= (double *)malloc(antnum*sizeof(double));
	ant_rate_ptr	= (double *)malloc(antnum*sizeof(double));
	ant_rate_err	= (double *)malloc(antnum*sizeof(double));

	gff_result	= (float *)malloc( 2*(blnum*ssnum + antnum - 1)
				* sizeof(float));
	gff_err		= (float *)malloc( 2*(blnum*ssnum + antnum - 1)
				* sizeof(float));

	/*-------- ALLOC MEMORY MAP --------*/
	vismap_key	= ftok(CP_DIR, 142857);
	if( (shrd_vismap_id	= shmget( vismap_key,
		2*blnum*ssnum*sizeof(int), IPC_CREAT | 0644 )) < 0){
		printf(" Error in shmget [%s]\n", argv[0]);
		exit(0);
	}
	shrd_vismap_ptr	= (int *)shmat(shrd_vismap_id, NULL, 0);

	/*-------- LINK BASELINE ID -> STATION ID --------*/
	for( bl_index=0; bl_index<blnum; bl_index++){
		bl2ant( bl_index, &stn_index2, &stn_index );
		cor_ptr	= first_cor_ptr;
		while(cor_ptr != NULL){

			if( MATCH11 && MATCH22 ){
				blid_in_cfs[bl_index] = cor_ptr->cor_id;
				printf(" BASELINE [NORMAL] %d : %d - %d assigned to %d\n",
				bl_index, cor_ptr->ant1, cor_ptr->ant2, cor_ptr->cor_id );
				bl_direction	= 1.0;
			}
			if( MATCH12 && MATCH21 ){
				blid_in_cfs[bl_index] = cor_ptr->cor_id;
				printf(" BASELINE [INVERT] %d : %d - %d assigned to %d\n",
				bl_index, cor_ptr->ant1, cor_ptr->ant2, cor_ptr->cor_id );
				bl_direction	= -1.0;
			}

			if( (MATCH11 && MATCH22) || (MATCH12 && MATCH21) ){
				for(ss_index=0; ss_index < ssnum; ss_index++){

					/*-------- READ SUB_STREAM HEADDER --------*/
					read_sshead( cor_ptr->cor_id, ss_id[ss_index]+1,
						&rf[ss_index], &freq_incr[ss_index],
						&freq_num[ss_index], &time_num_cfs, &time_incr);

					/*-------- PP NUMBER ROUND UP TO 2^n --------*/
					time_num	= (int)(atof(argv[ARG_INTEG])/time_incr)+1;
					integ_pp	= pow2round(time_num);
					bunch_num	= freq_num[ss_index] / (wind_x/2);
					wind_y		= integ_pp* 2;

					/*-------- ALLOCATE MEMORY AREA --------*/
					memalloc( freq_num[ss_index]/bunch_num, integ_pp,
						&shrd_vismap_ptr[bl_index*ssnum + ss_index],
						&shrd_vismap_ptr[(blnum + bl_index)*ssnum + ss_index]);
				}
				break;
			}
			cor_ptr = cor_ptr->next_cor_ptr;
		}
	}


	visamp_ptr	= (float *)malloc(wind_x * wind_y * sizeof(float) );
/*
------------------------ LOAD VISIBILITY DATA TO MEMORY
	while(start_mjd < stop_mjd){
*/
	while(start_mjd + (double)(time_incr*time_num/2)/86400.0 < stop_mjd){
	for( bl_index=0; bl_index<blnum; bl_index++){
		bl2ant( bl_index, &stn_index2, &stn_index );
		cor_ptr	= first_cor_ptr;
		while(cor_ptr != NULL){

			position		= -1;
			bl_direction	= 1.0;
			if( MATCH12 && MATCH21 ){
				printf(" BASELINE [INVERT] %d : %d - %d assigned to %d\n",
				bl_index, cor_ptr->ant1, cor_ptr->ant2, cor_ptr->cor_id );
				bl_direction	= -1.0;
			}

			if( (MATCH11 && MATCH22) || ( MATCH12 && MATCH21) ){

				/*-------- BLOCK INFO --------*/
				sprintf(ppblock_fname, BLOCK_FMT, getenv("CFS_DAT_HOME"), argv[1], cor_ptr->cor_id);
				printf("BLOCK FILE NAME = %s\n", ppblock_fname);
				ppblock_file = fopen(ppblock_fname, "r");
				if( ppblock_file == NULL){
					fclose(ppblock_file);
					printf("  Missing Block Record Information... Please Run codainfo in advance !\n");
					return(0);
				}
				fread(&ppblock_num, sizeof(int), 1, ppblock_file);
				fread(ppblock, ppblock_num* sizeof(struct block_info), 1, ppblock_file );
				fclose(ppblock_file);
				position = block_search(ppblock_num, ppblock, start_mjd);

				for(ss_index=0; ss_index < ssnum; ss_index++){

				/*-------- READ SUB_STREAM HEADDER --------*/
				read_sshead( cor_ptr->cor_id, ss_id[ss_index]+1, &rf[ss_index],
					&freq_incr[ss_index], &freq_num[ss_index], 
					&time_num_cfs, &time_incr);

				/*-------- PP NUMBER ROUND UP TO 2^n --------*/
				time_num	= (int)(atof(argv[ARG_INTEG])/time_incr)+1;
				integ_pp	= pow2round(time_num);
				bunch_num	= freq_num[ss_index] / (wind_x/2);

				/*-------- LOAD VISIBILITY DATA  CFS -> MEMORY --------*/
				valid_pp = load_vis_uvw( cor_ptr->cor_id, bl_direction, obj_id,
				ss_id[ss_index]+1, &position, &freq_num[ss_index], bunch_num,
				time_num_cfs, time_num, time_incr, start_mjd, stop_mjd,
				uvw_bl[bl_index],
				bp_r_ptr[stn_index][ss_id[ss_index]],
				bp_r_ptr[stn_index2][ss_id[ss_index]],
				work, shrd_vismap_ptr[bl_index*ssnum + ss_index], 
				shrd_vismap_ptr[(blnum + bl_index)*ssnum + ss_index]);
				freq_incr[ss_index]	*= bunch_num;

				}
				break;
			}

			cor_ptr = cor_ptr->next_cor_ptr;
		}
	}

	for( bl_index=0; bl_index<blnum; bl_index++){
		memset(visamp_ptr, 0, (wind_x * wind_y * sizeof(float))); 

		/*-------- MAPPING (X-SPECTRUM -> CORR Func.) --------*/
		for(ss_index=0; ss_index < ssnum; ss_index++){
			crs_search( shrd_vismap_ptr[bl_index*ssnum + ss_index],
				shrd_vismap_ptr[(blnum + bl_index)*ssnum + ss_index],
				freq_num[ss_index], time_num, wind_x, wind_y, visamp_ptr);

		}

		/*-------- WINDOW PIXEL SIZE and SEARCH RANGE --------*/
		x_incr	= 1.0/(freq_incr[0]*wind_x);
		y_incr	= 1.0/( (rf[0] + freq_incr[0]*freq_num[0]/2)
					* time_incr* integ_pp* 2 );
		width_win_x = 2* (int)(atof(argv[ARG_DELAY_WIN_WIDTH]) / x_incr);
		width_win_y = 2* (int)(atof(argv[ARG_RATE_WIN_WIDTH]) / (1.0e6*y_incr));

		init_win_x	= (wind_x - width_win_x)/2;
		init_win_y	= (wind_y - width_win_y)/2;
		init_win_x += (int)( atof(argv[ARG_DELAY_WIN_CENTER]) / x_incr );
		init_win_y += (int)( atof(argv[ARG_RATE_WIN_CENTER])/ (1.0e6*y_incr) );

		win_x_min	= x_incr* ((float)( init_win_x - wind_x/2 ) - 1);
		win_y_min	= y_incr* ((float)( init_win_y - wind_y/2 ) - 1);

/*
		printf("WINDOW SIZE   : X=%d  Y=%d\n", wind_x, wind_y); 
		printf("WINDOW X RANG : INIT=%d  [%f] WIDTH=%d\n",
								init_win_x, win_x_min, width_win_x);
		printf("WINDOW Y RANG : INIT=%d  [%f] WIDTH=%d\n",
								init_win_y, win_y_min, width_win_y);
*/

		/*-------- SEARCH PEAK POSITION --------*/
		max_search_win(visamp_ptr, ssnum, wind_x, wind_y,
					init_win_x,	width_win_x,
					init_win_y,	width_win_y,
					&vismax, &delay_pos, &rate_pos);

		/*-------- SEARCH around the PEAK POSITION --------*/
		sqr_fit((double)(delay_pos-1),
			(double)(rate_pos),
			(double)visamp_ptr[wind_x*(rate_pos+wind_y/2)+delay_pos+wind_x/2-1],

			(double)(delay_pos),
			(double)(rate_pos),
			(double)visamp_ptr[wind_x*(rate_pos+wind_y/2)+delay_pos+wind_x/2],

			(double)(delay_pos+1),
			(double)(rate_pos),
			(double)visamp_ptr[wind_x*(rate_pos+wind_y/2)+delay_pos+wind_x/2+1],

			(double)(delay_pos),
			(double)(rate_pos-1),
			(double)visamp_ptr[wind_x*(rate_pos+wind_y/2-1)+delay_pos+wind_x/2],

			(double)(delay_pos),
			(double)(rate_pos+1),
			(double)visamp_ptr[wind_x*(rate_pos+wind_y/2+1)+delay_pos+wind_x/2],
			coeff);

		/*-------- ESTIMATE the PEAK AMP and POSITION --------*/
		vismax = -(coeff[2]*coeff[2]/coeff[0]+coeff[3]*coeff[3]/coeff[1])/4
				+ coeff[4];
		*bl_delay_ptr	= -coeff[2]/(coeff[0]*freq_incr[0]*freq_num[0]*4);
		*bl_rate_ptr	= -coeff[3]/(coeff[1]
						* (rf[0] + freq_incr[0] * freq_num[0] / 2)
						* time_incr*integ_pp*4);

		/*-------- CALCURATE SIGNAL-to-NOISE RATIO --------*/
		vis_snr	= vismax*sqrt(2.0*time_num*time_incr
				*freq_num[0]* fabs(freq_incr[0])*1.0e6*sqrt(ssnum));

		*bl_delay_err	= 1.0/(2.0*freq_num[0]*freq_incr[0]*vis_snr);
		*bl_rate_err	= 1.0/(rf[0]*time_num*time_incr*vis_snr);

		if(vis_snr < 0.75*atof(argv[ARG_SNR_LIMIT]) ){
			*bl_delay_err	= 10000.0/(2.0*freq_num[0]*freq_incr[0]* vis_snr);
			*bl_rate_err	= 10000.0/(rf[0]*time_num*time_incr* vis_snr);
		} else if(vis_snr < atof(argv[ARG_SNR_LIMIT]) ) {
			*bl_delay_err	= 100.0/(2.0*freq_num[0]*freq_incr[0]* vis_snr);
			*bl_rate_err	= 100.0/(rf[0]*time_num*time_incr* vis_snr);
		} else {

			*bl_delay_err	= 1.0/(2.0*freq_num[0]*freq_incr[0]*
							(vis_snr - atof(argv[ARG_SNR_LIMIT])));
			*bl_rate_err	= 1.0/(rf[0]*time_num*time_incr*
							(vis_snr - atof(argv[ARG_SNR_LIMIT])));
		}

		printf("VISMAX = %f at delay=%8.5lf +/- %8.5lf  rate=%8.5lf +/- %8.5lf (SNR = %7.2f)\n",
			vismax, *bl_delay_ptr, *bl_delay_err,
			*bl_rate_ptr* 1.0e6, *bl_rate_err* 1.0e6, vis_snr );

		cpgbeg(1, argv[ARG_DEVICE], 1, 1);

#ifdef PGPSEUD
#endif
/*
		cpgpseud( visamp_ptr, wind_x, wind_y, wind_y, x_incr, y_incr,
				SNR_LIMIT* vismax / vis_snr);
*/
		cpgpseud_win( visamp_ptr, width_win_x, width_win_y, wind_x,
				init_win_x, init_win_y, win_x_min, win_y_min, x_incr, y_incr,
				1.5* atof(argv[ARG_SNR_LIMIT]) * vismax / vis_snr);

		cpgsvp(0.77, 0.95, 0.1, 0.9);
		cpgswin(0.0, 1.0, 0.0, 1.0);
		cpgsch(0.7);
		sprintf(pg_text, "EXPER:  %s", obs.obscode);
			cpgtext(0.1, 1.0, pg_text);

		sprintf(pg_text, "SOURCE: %s", obj_name);	
			cpgtext(0.1, 0.95, pg_text);

		fmjd2doy( start_mjd, &year, &doy, &hour, &min, &sec);
		sprintf(pg_text, "START:  %03d %02d:%02d:%02d",
			doy, hour, min, (int)sec, time_num*time_incr);
			cpgtext(0.1, 0.90, pg_text);

		sprintf(pg_text, "INTEG:  %7.3lf sec.", time_num*time_incr);
			cpgtext(0.1, 0.85, pg_text);

		bl2ant( bl_index, &stn_index2, &stn_index );
		sprintf(pg_text, "ANT:    %s - %s", stn_name[stn_index], stn_name[stn_index2]);
			cpgtext(0.1, 0.80, pg_text);

		sprintf(pg_text, "PEAK = %9.3e", vismax);
			cpgtext(0.1, 0.75, pg_text);

		sprintf(pg_text, "    at delay=%7.3lf \\gmsec", *bl_delay_ptr);
			cpgtext(0.1, 0.72, pg_text);

		sprintf(pg_text, "       rate =%7.3lf psec/sec",*bl_rate_ptr* 1.0e6);
			cpgtext(0.1, 0.69, pg_text);

		sprintf(pg_text, "SNR = %7.2f", vis_snr);
			cpgtext(0.1, 0.65, pg_text);
#ifdef PGPSEUD
#endif

#ifdef PGSLICE
		cpgslice( visamp_ptr, wind_y, wind_x, y_incr,	/* Rate Slice	*/
				vismax, delay_pos+ wind_x/2 );
#endif

#ifdef PGSLICE
		cpgslice( visamp_ptr, wind_x, wind_x, x_incr,	/* DELAY Slice */
				vismax, rate_pos+ wind_y/2 );
#endif

#ifdef PGSLICE
		cpgsvp(0.77, 0.95, 0.1, 0.9);
		cpgswin(0.0, 1.0, 0.0, 1.0);
		cpgsch(0.7);
		sprintf(pg_text, "EXPER:  %s", obs.obscode);
			cpgtext(0.1, 1.0, pg_text);
		sprintf(pg_text, "SOURCE: %s", obj_ptr->obj_name);	
			cpgtext(0.1, 0.95, pg_text);
		fmjd2doy( start_mjd, &year, &doy, &hour, &min, &sec);
		sprintf(pg_text, "START:  %03d %02d:%02d:%02d",
			doy, hour, min, (int)sec, time_num*time_incr);
			cpgtext(0.1, 0.90, pg_text);

		sprintf(pg_text, "INTEG:  %7.3lf sec.", time_num*time_incr);
			cpgtext(0.1, 0.85, pg_text);

		sprintf(pg_text, "ANT:    %s - %s", stn_name[0], stn_name[1]);
			cpgtext(0.1, 0.80, pg_text);

		sprintf(pg_text, "PEAK = %9.3e", vismax);
			cpgtext(0.1, 0.75, pg_text);

		sprintf(pg_text, "    at delay=%7.3lf \\gmsec", *bl_delay_ptr);
			cpgtext(0.1, 0.72, pg_text);

		sprintf(pg_text, "       rate =%7.3lf psec/sec",*bl_rate_ptr* 1.0e6);
			cpgtext(0.1, 0.69, pg_text);

		sprintf(pg_text, "SNR = %7.2f", vis_snr);
			cpgtext(0.1, 0.65, pg_text);
#endif

#ifdef PGBIRD
        cpgenv(-cos(0.5), sin(0.5), 0.0, 1.5, 0, -2);
		cpgbbuf();
        cpgbird( visamp_ptr, wind_y, wind_x, wind_x, (double)vismax,
                0.5, 0.3, 1 );
		cpgebuf();
#endif

		bl_delay_ptr++; bl_rate_ptr++;
		bl_delay_err++; bl_rate_err++;


		cpgend();

	}

	bl_delay_ptr	-= blnum;
	bl_rate_ptr		-= blnum;
	bl_delay_err	-= blnum;
	bl_rate_err		-= blnum;
	
	closure_solve( antnum, bl_delay_ptr,  bl_delay_err,
					ant_delay_ptr, ant_delay_err);
	closure_solve( antnum, bl_rate_ptr,  bl_rate_err,
					ant_rate_ptr, ant_rate_err);

	for(stn_index=0; stn_index<antnum; stn_index++){
		printf(" STN %d : DELAY= %7.5lf, RATE= %7.5lf\n",
			stnid_in_cfs[stn_index], *ant_delay_ptr, *ant_rate_ptr*1.0e6);
		ant_delay_ptr++;
		ant_rate_ptr++;
	}

	ant_delay_ptr -= antnum;
	ant_rate_ptr -= antnum;

	ant_delay_ptr++;
	ant_rate_ptr++;
	for(stn_index=0; stn_index<antnum-1; stn_index++){
		gff_result[2*blnum*ssnum + stn_index] = -(*ant_delay_ptr);
		gff_result[2*blnum*ssnum + antnum + stn_index - 1]
			= -(*ant_rate_ptr);

		gff_err[2*blnum*ssnum + stn_index]
				= 1.0/(2.0*freq_num[0]*freq_incr[0]*vis_snr);
		gff_err[2*blnum*ssnum + antnum + stn_index - 1] 
				= 1.0/(rf[0]*time_num*time_incr*vis_snr);

		ant_delay_ptr++;
		ant_rate_ptr++;
	}
	ant_delay_ptr	-= antnum;
	ant_rate_ptr	-= antnum;

	integ_mult( &shrd_vismap_ptr[0],
				&shrd_vismap_ptr[blnum*ssnum],
				ssnum, freq_num, rf, freq_incr, time_num, time_incr,
				antnum, gff_result );

	icon = gff_mult( &shrd_vismap_ptr[0],
				&shrd_vismap_ptr[blnum*ssnum],
				ssnum, freq_num, rf, freq_incr, time_num, time_incr,
				antnum, gff_result, gff_err);

	printf("******** SUMMERY OF GLOBAL FRINGE SEARCH ********\n");
	fmjd2doy( start_mjd, &year, &doy, &hour, &min, &sec);
	printf(" START = %03d %02d:%02d:%02d, INTEG TIME = %lf\n",
			doy, hour, min, (int)sec, time_num*time_incr);
	for(stn_index=0; stn_index<antnum-1; stn_index++){
/*
		printf("SOLUTION STN %d : DELAY= %7.5lf +/- %8.7lf, RATE= %7.5lf +/- %8.7lf\n",
*/
		gff_snr = 1.0/ (2.0*freq_num[0]*freq_incr[0]
				* gff_err[2*blnum*ssnum + stn_index]);

		/*----- FCORRECT [DDDHHMMSS]    [STN] [CLOCK] [RATE] [SNR] ------*/
		printf("FCORRECT %03d%02d%02d%02d  %d  %7.4lf  %7.4lf  %7.4lf\n",
				doy, hour, min, (int)sec,
				stn_index,
				gff_result[2*blnum*ssnum + stn_index],
				gff_result[2*blnum*ssnum + antnum + stn_index - 1]*1.0e6,
				gff_snr);
	}
	for(bl_index=0; bl_index<blnum; bl_index++){
		for(ss_index=0; ss_index<ssnum; ss_index++){

			/*-------- OUTPUT VISIBILITY AMPLITUDE --------*/
			printf( VISIB_FMT,
				doy, hour, min, (int)sec,
				bl_index, ss_index,
				gff_result[bl_index* ssnum + ss_index]*100.0,
				gff_err[bl_index* ssnum + ss_index]*100.0,
				gff_result[(blnum + bl_index)* ssnum + ss_index]*RADDEG,
				gff_err[(blnum + bl_index)* ssnum + ss_index]*RADDEG);
		}
	}

	/*-------- PRINT (u, v, w) and (du/dt, dv/dt, dw/dt) --------*/
	for(bl_index=0; bl_index<blnum; bl_index++){
		printf( UVW_FMT,
			doy, hour, min, (int)sec, bl_index,
			uvw_bl[bl_index][0], uvw_bl[bl_index][1], uvw_bl[bl_index][2],
			uvw_bl[bl_index][3], uvw_bl[bl_index][4], uvw_bl[bl_index][5]);
	}

	printf("\n");

/*
------------------------ SAVE GFF RESULT TO FCAL-DATA
*/
	for(stn_index=0; stn_index<antnum; stn_index++){

		save_gff( argv[ARG_OBS_NAME], obj_name, first_stn_ptr,
			stnid_in_cfs[stn_index], stnid_in_cfs[0],
			stn_index, antnum, ssnum, first_cor_ptr->nss, ss_id, start_mjd,
			time_num*time_incr, gff_result, gff_err);
	}

	start_mjd	+= (double)(time_incr*time_num)/86400.0;
	}	/* END OF TIME LOOP */

	free(visamp_ptr);

	for(stn_index=0; stn_index<antnum; stn_index++){
		for(ss_index=0; ss_index<first_cor_ptr->nss; ss_index++){
			free(bp_r_ptr[stn_index][ss_index]);
			free(bp_i_ptr[stn_index][ss_index]);
		}
	}

	/*-------- CLOSE SHARED MEMORY --------*/
	shmctl( shrd_obj_id, IPC_RMID, 0 );
	shmctl( shrd_stn_id, IPC_RMID, 0 );
	shmctl( shrd_vismap_id, IPC_RMID, 0 );

	return(0);
}
