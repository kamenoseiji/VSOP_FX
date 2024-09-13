/*********************************************************
**	CODA_GFS.C	: Global Fringe Search using 			**
**					CODA File System					**
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
#define CP_DIR	"/home/kameno/FX/bin"
#define	MAX_ANT	10
#define	MAX_BL	45	
#define	MAX_SS	32	
#define	MAX_CH	1024
#define WIND_Y	2048
#define	SNR_LIMIT 9.0
#define RADDEG      57.29577951308232087721
#define	PI		3.141592653589793238
#define VISIB_FMT   " BL%2d : SS=%2d: AMP[%%]= %7.5lf +/- %8.6lf, PHS[deg]= %7.2lf +/- %7.3lf\n"

MAIN__( argc, argv )
	int		argc;		/* Number of Arguments */
	char	**argv;		/* Pointer of Arguments */
{
	/*-------- STRUCT for CFS --------*/
	struct	header		obs;			/* OBS HEADDER						*/
	struct	head_obj	*obj_ptr;		/* Pointer of Objects Header		*/
	struct	head_obj	*first_obj_ptr;	/* First Pointer of Objects Header	*/
	struct	head_stn	*stn_ptr;		/* Pointer of Station Header		*/
	struct	head_stn	*first_stn_ptr;	/* First Pointer of Station Header	*/
	struct	head_cor	*cor_ptr;		/* Pointer of CORR Header			*/
	struct	head_cor	*first_cor_ptr;	/* First Pointer of CORR Header		*/
	struct	bl_list		bl_info[MAX_BL];/* Baseline Information				*/
	struct	obj_list	obj_info[MAX_BL];/* Object Information				*/

	/*-------- SHARED MEMORY --------*/
	int		shrd_obj_id;				/* Shared Memory ID for Source HD	*/
	int		shrd_stn_id;				/* Shared Memory ID for Station HD	*/
	key_t	vismap_key;					/* Keyword for Shared memory map	*/
	int		shrd_vismap_id;				/* Shared Memory ID for memory map	*/
	int		*shrd_vismap_ptr;			/* First Pointer of memory map		*/
	int		shrd_vis_id[45][16];		/* Shared Memory ID for Vis			*/

	/*-------- ID Number --------*/
	int		obj_id;						/* OBJECT ID						*/
	float	bl_direction;				/* Baseline Direction				*/
	int		ret;						/* Return Code from CFS Library		*/
	int		lunit;						/* Unit Number of CFS File			*/
	int		ss_id[MAX_SS];				/* SS ID in CFS						*/
	int		position;					/* PP Position in CFS				*/
	int		icon;						/* Condition Code					*/
	int		stnid_in_cfs[10];			/* Station ID Number in CODA		*/

	/*-------- INDEX Number --------*/
	int		stn_index;					/* Index for Station				*/
	int		stn_index2;					/* Index for Station				*/
	int		bl_index;					/* Index for Baseline				*/
	int		ss_index;					/* Index of Sub-Stream				*/
	int		time_index;					/* Index for Time					*/
	int		freq_index;					/* Index for Frequency				*/
	int		acc_index;					/* Index for Acceleration			*/
	int		stn_arg;					/* Arg Index which STATION starts	*/

	/*-------- TOTAL Number --------*/
	int		blnum;						/* Total Number of Baseline			*/
	int		ssnum;						/* Number of Sub-Stream				*/
	int		freq_num[MAX_SS];			/* Number of Frequency				*/
	int		bunch_num;					/* Number of Bunching				*/
	int		valid_pp;					/* Valid Time Number				*/
	int		time_num_cfs;				/* Number of Time Data				*/
	int		time_num;					/* Number of Time Data				*/
	int		integ_pp;					/* Coherent Integration PP			*/
	int		antnum;						/* Total Number of Selected Station	*/
	int		accnum;						/* Acceleration Search Number		*/
	int		bp_ssnum;					/* SS-Number in Band-Pass File		*/
	int		bp_freq_num[MAX_SS];		/* Frequency Channel Number			*/

	/*-------- CHARACTERS --------*/
	char	fname[128];					/* File Name of CFS Files			*/
	char	omode[2];					/* Access Mode of CFS Files			*/
	char	obj_name[32];				/* Object Name						*/
	char	bp_obj_name[32];			/* Object Name in Band-Pass File	*/
	char	stn_name[10][32];			/* Station Name to Plot				*/

	/*-------- GENERAL VARIABLES --------*/
	double	start_mjd;					/* Start Time [MJD]					*/
	double	stop_mjd;					/* Stop Time [MJD]					*/
	double	uvw[45][3];					/* Baseline Vector from Obj [m]		*/
	double	loss;						/* Quantize Efficiency				*/
	double	rf[MAX_SS];					/* RF Frequency [MHz]				*/
	double	freq_incr[MAX_SS];			/* Frequency Increment [MHz]		*/
	double	time_incr;					/* Time Increment [sec]				*/
	double	acc;						/* Fringe Acceleration [fHz/sec]	*/
	float	work[2*MAX_CH];				/* Work Area to Read Visibility		*/
	float	*visamp_ptr;				/* Pointer of Visibility Amplitude	*/
	int		wind_x, wind_y;				/* Window Range (X, Y)				*/
	double	*bl_delay_ptr;				/* Residual Delay (BL-Based)		*/
	double	*bl_delay_err;				/* Residual Delay Error				*/
	double	*bl_rate_ptr;				/* Residual Delay Rate (BL-Based)	*/
	double	*bl_rate_err;				/* Residual Rate Error				*/
	double	*ant_delay_ptr;				/* Residual Delay (ANT-Based)		*/
	double	*ant_delay_err;				/* Residual Delay Error				*/
	double	*ant_rate_ptr;				/* Residual Delay Rate (ANT-Based)	*/
	double	*ant_rate_err;				/* Residual Rate Error				*/
	float	vis_snr;					/* SNR of Visivility				*/
	float	vismax;						/* Maximum Visibility Amp			*/
	float	x_incr, y_incr;				/* Increment of Delay and Rate		*/
	float	*gff_result;				/* RESULT of GFF					*/
	float	*gff_err;					/* ERROR of GFF						*/
	double	sec;						/* Second							*/
	int		start_time;					/* START TIME [DDDHHMMSS]			*/
	int		stop_time;					/* STOP TIME [DDDHHMMSS]			*/
	int		year, doy;					/* YEAR and Day of Year				*/
	int		hour, min;					/* Hour and Minutre					*/ 
	double	bp_mjd;						/* MJD of the Band-Pass File		*/
	double	bp_integ_time;				/* Integ Time of the Band-Pass File	*/
	double	bp_rf[MAX_SS];				/* RF Freq. [MHz] in BP File		*/
	double	bp_freq_incr[MAX_SS];		/* Freq. Increment [MHz] in BP File	*/
	double	*bp_r_ptr[MAX_ANT][MAX_SS];	/* Pointer of Bandpass (REAL)		*/
	double	*bp_i_ptr[MAX_ANT][MAX_SS];	/* Pointer of Bandpass (IMAG)		*/
	double	bp_vis_max[MAX_ANT][MAX_SS];/* Maximum of Bandpass				*/
	float	vismax_acc;					/* Maximum Visibility along ACC		*/
	double	delay_max_acc;				/* Residual Delay (BL-Based)		*/
	double	delay_err_acc;				/* Residual Delay Error				*/
	double	rate_max_acc;				/* Residual Delay Rate (BL-Based)	*/
	double	rate_err_acc;				/* Residual Delay Rate (BL-Based)	*/
	double	best_acc[MAX_ANT];			/* Residual Delay Rate (BL-Based)	*/
	double	best_acc_err[MAX_ANT];		/* Residual Delay Rate (BL-Based)	*/
	double	init_acc;					/* Residual Delay Rate (BL-Based)	*/
	double	weight;						/* Residual Delay Rate (BL-Based)	*/
	double	sum_weight;					/* Residual Delay Rate (BL-Based)	*/
/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 8 ){
		printf("USAGE : coda_gfs [OBS_NAME] [SOURCE] [START] [STOP] [INTEG] [DEVICE] [TOTAL STN] [TOTAL SS] [STN_NAME1] [STN_NAME2] ... [SS1] [SS2]... !!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  SOURCE -------- Source Name for Band-Pass Calib\n");
		printf("  START --------- Start TIME [DDDHHMMSS] \n");
		printf("  STOP ---------- Stop TIME  [DDDHHMMSS] \n");
		printf("  INTEG --------- Integration TIME  [sec] \n");
		printf("  DEVICE -------- PGPLOT Device \n");
		printf("  TOTAL STN ----- TOTAL NUMBER OF STATION\n");
		printf("  TOTAL SS ------ TOTAL NUMBER OF SUB-STREAM\n");
		printf("  STN_NAME ------ STATION NAMEs \n");
		exit(0);
	}

	/*-------- START and STOP Time --------*/
	start_time	= atoi(argv[3]);
	stop_time	= atoi(argv[4]);
	sprintf(obj_name, "%s", argv[2]);
/*
------------------------ ACCESS TO THE CODA FILE SYSTEM (CFS)
*/
	cfs000_( &ret );			cfs_ret( 000, ret );
	cfs020_( &ret );			cfs_ret( 020, ret );
	cfs006_( argv[1], &ret, strlen( argv[1] ));	cfs_ret( 006, ret );

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
	for(stn_arg=9; stn_arg<9+atoi(argv[7]); stn_arg++){
		stn_ptr = first_stn_ptr;
		while(stn_ptr != NULL){

			/*-------- SEEK SPECIFIED STATION --------*/
			if( strstr(stn_ptr->stn_name, argv[stn_arg]) != NULL ){
				printf("STATION %-10s: ID = %2d\n",
					stn_ptr->stn_name, stn_ptr->stn_index );
				stnid_in_cfs[antnum] = stn_ptr->stn_index;
				strcpy( stn_name[stn_ptr->stn_index], stn_ptr->stn_name);
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
	ssnum	= atoi(argv[8]);
	wind_x	= 32768;
	for(ss_index=0; ss_index<ssnum; ss_index++){
		ss_id[ss_index] = atoi(argv[9+antnum+ss_index]);

		/*-------- CHECK MAX FREQ NUMBER -> SEARCH WINDOW --------*/
		read_sshead( 1, ss_id[ss_index]+1, &rf[ss_index],
			&freq_incr[ss_index], &freq_num[ss_index], 
			&time_num_cfs, &time_incr);
		if( freq_num[ss_index] < wind_x){ wind_x = freq_num[ss_index]; }
	}
	wind_x *= 2;
	if(wind_x > MAX_CH){ wind_x = MAX_CH; }
	printf("FREQ NUM = %d\n", wind_x);

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

	gff_result	= (float *)malloc( 2*(blnum*ssnum + antnum)
				* sizeof(float));
	gff_err		= (float *)malloc( 2*(blnum*ssnum + antnum)
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
			if( (cor_ptr->ant1 == stnid_in_cfs[stn_index]) && 
				(cor_ptr->ant2 == stnid_in_cfs[stn_index2]) ){

				printf(" BASELINE [NORMAL] %d : %d - %d assigned to %d\n",
				bl_index, cor_ptr->ant1, cor_ptr->ant2, cor_ptr->cor_id );

				bl_info[bl_index].cor_id	= cor_ptr->cor_id;
				bl_info[bl_index].stn_id1	= cor_ptr->ant1;
				bl_info[bl_index].stn_id2	= cor_ptr->ant2;
				bl_info[bl_index].bl_dir	= NORMAL;
				strcpy( bl_info[bl_index].stn_name1, stn_name[cor_ptr->ant1]);
				strcpy( bl_info[bl_index].stn_name2, stn_name[cor_ptr->ant2]);

				break;
			}

			if( (cor_ptr->ant1 == stnid_in_cfs[stn_index2]) && 
				(cor_ptr->ant2 == stnid_in_cfs[stn_index]) ){

				printf(" BASELINE [INVERT] %d : %d - %d assigned to %d\n",
				bl_index, cor_ptr->ant1, cor_ptr->ant2, cor_ptr->cor_id );

				bl_info[bl_index].cor_id	= cor_ptr->cor_id;
				bl_info[bl_index].stn_id1	= cor_ptr->ant1;
				bl_info[bl_index].stn_id2	= cor_ptr->ant2;
				bl_info[bl_index].bl_dir	= INVERT;
				strcpy( bl_info[bl_index].stn_name1, stn_name[cor_ptr->ant2]);
				strcpy( bl_info[bl_index].stn_name2, stn_name[cor_ptr->ant1]);

				break;
			}
			cor_ptr = cor_ptr->next_cor_ptr;
		}
	}

	for( bl_index=0; bl_index<blnum; bl_index++){
		for(ss_index=0; ss_index < ssnum; ss_index++){
			/*-------- READ SUB_STREAM HEADDER --------*/
			read_sshead( bl_info[bl_index].cor_id, ss_id[ss_index]+1,
				&rf[ss_index],	&freq_incr[ss_index], &freq_num[ss_index], 
				&time_num_cfs, &time_incr);

			/*-------- PP NUMBER ROUND UP TO 2^n --------*/
			time_num	= (int)(atof(argv[5])/time_incr)+1;
			integ_pp	= pow2round(time_num);
			bunch_num	= freq_num[ss_index] / (wind_x/2);
			wind_y		= integ_pp;

			/*-------- ALLOCATE MEMORY AREA --------*/
			memalloc( freq_num[ss_index]/bunch_num, integ_pp,
				&shrd_vismap_ptr[bl_index*ssnum + ss_index],
				&shrd_vismap_ptr[(blnum + bl_index)*ssnum + ss_index]);
		}
	}

	visamp_ptr	= (float *)malloc(wind_x * wind_y * sizeof(float) );
/*
------------------------ LOAD VISIBILITY DATA TO MEMORY
*/
	while(start_mjd + (double)(time_incr*time_num/2)/86400.0 < stop_mjd){
	for( bl_index=0; bl_index<blnum; bl_index++){

		bl2ant( bl_index, &stn_index2, &stn_index );
		position		= -1;

		for(ss_index=0; ss_index < ssnum; ss_index++){
			/*-------- READ SUB_STREAM HEADDER --------*/
			read_sshead( bl_info[bl_index].cor_id, ss_id[ss_index]+1,
				&rf[ss_index], &freq_incr[ss_index], &freq_num[ss_index], 
				&time_num_cfs, &time_incr);

			/*-------- PP NUMBER ROUND UP TO 2^n --------*/
			time_num	= (int)(atof(argv[5])/time_incr)+1;
			integ_pp	= pow2round(time_num);
			bunch_num	= freq_num[ss_index] / (wind_x/2);

			/*-------- LOAD VISIBILITY DATA  CFS -> MEMORY --------*/
			valid_pp = load_vis( bl_info[bl_index].cor_id,
				(float)bl_info[bl_index].bl_dir, obj_id, ss_id[ss_index]+1,
				&position, &freq_num[ss_index], bunch_num, time_num_cfs,
				time_num, time_incr, start_mjd, stop_mjd, uvw[bl_index],
				bp_r_ptr[stn_index][ss_id[ss_index]],
				bp_r_ptr[stn_index2][ss_id[ss_index]],
				work, shrd_vismap_ptr[bl_index*ssnum + ss_index], 
				shrd_vismap_ptr[(blnum + bl_index)*ssnum + ss_index]);

			freq_incr[ss_index]	*= bunch_num;
		}
	}
/*
------------------------ ACCELERATION SEARCH
*/
	for( bl_index=0; bl_index<blnum; bl_index++){

		bl2ant( bl_index, &stn_index2, &stn_index );

		if( stn_index2 == antnum - 1){
		/*-------- WITHOUT ACCELERATION SEARCH --------*/
			printf("SATELLITE DATA!... ACCELERATION SEARCH\n");

			accnum = 7;
			vismax = 0.0;
			for(acc_index=0; acc_index < accnum; acc_index++){
				memset(visamp_ptr, 0, (wind_x * wind_y * sizeof(float))); 
				acc = 6.0*PI/(time_num*time_num) * (acc_index - accnum/2);

				printf("ACC = %d: %9.3e Hz/sec \n",
					acc_index - accnum/2,
					acc /(double)( 2.0* PI* time_incr* time_incr ) );

				for(ss_index=0; ss_index < ssnum; ss_index++){
					acc_search( shrd_vismap_ptr[bl_index*ssnum + ss_index],
						shrd_vismap_ptr[(blnum + bl_index)*ssnum + ss_index],
						acc, freq_num[ss_index], time_num, wind_x, wind_y,
						visamp_ptr);
				}

				bl_search( visamp_ptr, ssnum,
					(double)(freq_num[0]*freq_incr[0]),		/* Bandwidth	*/
					(double)(time_num*time_incr),			/* Integ Time	*/
					rf[0], wind_x, wind_y,
					&delay_max_acc, &delay_err_acc,
					&rate_max_acc,  &rate_err_acc,
					&vismax_acc, &vis_snr);

				x_incr	= 1.0/(freq_incr[0]*wind_x);
				y_incr	= 1.0/( (rf[0] + freq_incr[0]*freq_num[0]/2)
						* time_incr* integ_pp* 2 );

				plot_blsearch( argv[6], visamp_ptr,
					SNR_LIMIT* vismax_acc/ vis_snr,
					wind_x, wind_y, x_incr, y_incr,
					&obs, first_obj_ptr, &bl_info[bl_index],
					valid_pp*time_incr, obj_name, start_mjd, 
					delay_max_acc, rate_max_acc, vismax_acc, vis_snr);

				if( vismax_acc  > vismax ){
					vismax = vismax_acc;
					best_acc[stn_index]	= acc;
					best_acc_err[stn_index]	= 1.0/vis_snr;
					*bl_delay_ptr	= delay_max_acc;
					*bl_delay_err	= delay_err_acc;
					*bl_rate_ptr	= rate_max_acc;
					*bl_rate_err	= rate_err_acc;
				}

			}	/* End of ACC LOOP */
		} else {
		/*-------- WITHOUT ACCELERATION SEARCH --------*/
			printf("No Satellite ... Acceleration is Assumed to be 0.\n");
			memset(visamp_ptr, 0, (wind_x * wind_y * sizeof(float))); 
			for(ss_index=0; ss_index < ssnum; ss_index++){

				crs_search( shrd_vismap_ptr[bl_index*ssnum + ss_index],
					shrd_vismap_ptr[(blnum + bl_index)*ssnum + ss_index],
					freq_num[ss_index], time_num, wind_x, wind_y, visamp_ptr);

				bl_search( visamp_ptr, ssnum,
					(double)(freq_num[0]*freq_incr[0]),		/* Bandwidth	*/
					(double)(time_num*time_incr),			/* Integ Time	*/
					rf[0], wind_x, wind_y,
					bl_delay_ptr, bl_delay_err,
					bl_rate_ptr, bl_rate_err,
					&vismax, &vis_snr);

				x_incr	= 1.0/(freq_incr[0]*wind_x);
				y_incr	= 1.0/( (rf[0] + freq_incr[0]*freq_num[0]/2)
						* time_incr* integ_pp* 2 );

				plot_blsearch( argv[6], visamp_ptr, SNR_LIMIT* vismax/ vis_snr,
					wind_x, wind_y, x_incr, y_incr,
					&obs, first_obj_ptr, &bl_info[bl_index],
					valid_pp*time_incr, obj_name, start_mjd, 
					*bl_delay_ptr, *bl_rate_ptr, vismax, vis_snr);
			}
		}


		bl_delay_ptr++; bl_rate_ptr++;
		bl_delay_err++; bl_rate_err++;

	}	/* End of Baseline Loop */
	bl_delay_ptr	-= blnum;
	bl_rate_ptr		-= blnum;
	bl_delay_err	-= blnum;
	bl_rate_err		-= blnum;
/*
------------------------ BL-BASED DELAY -> ANT-BASED DELAY
*/
	closure_solve( antnum, bl_delay_ptr,  bl_delay_err,
				ant_delay_ptr, ant_delay_err);
	closure_solve( antnum, bl_rate_ptr,  bl_rate_err,
				ant_rate_ptr, ant_rate_err);

	init_acc= 0.0;
	sum_weight	= 0.0;
	for(stn_index=0; stn_index<antnum-1; stn_index++){
		weight		= 1.0/(best_acc_err[stn_index]* best_acc_err[stn_index]);
		init_acc	+= best_acc[stn_index]* weight;
		sum_weight	+= weight;
	}
	init_acc /= sum_weight;

	for(stn_index=0; stn_index<antnum; stn_index++){
		printf(" STN %d : DELAY= %7.5lf, RATE= %7.5lf\n",
			stnid_in_cfs[stn_index],
			ant_delay_ptr[stn_index],
			ant_rate_ptr[stn_index]*1.0e6);
	}
	printf(" STN %d : ACC= %e \n", stn_index, init_acc);


	for(stn_index=0; stn_index<antnum-1; stn_index++){
		gff_result[2*blnum*ssnum + stn_index] = -ant_delay_ptr[stn_index+1];
		gff_result[2*blnum*ssnum + antnum + stn_index - 1]
			= -ant_rate_ptr[stn_index+1];

		gff_err[2*blnum*ssnum + stn_index]
				= 1.0/(2.0*freq_num[0]*freq_incr[0]*vis_snr);
		gff_err[2*blnum*ssnum + antnum + stn_index - 1] 
				= 1.0/(rf[0]*time_num*time_incr*vis_snr);
	}
	gff_result[2*blnum*ssnum + 2* antnum - 2]	= -init_acc;
	gff_err[2*blnum*ssnum + 2* antnum - 2]		= 6.0*PI/(time_num*time_num);

	integ_mult_acc( &shrd_vismap_ptr[0],
				&shrd_vismap_ptr[blnum*ssnum],
				ssnum, freq_num, rf, freq_incr, time_num, time_incr,
				antnum, gff_result );

/*
	for(bl_index=0; bl_index<blnum; bl_index++){
		for(ss_index=0; ss_index<ssnum; ss_index++){

			printf( VISIB_FMT, bl_index, ss_index,
				gff_result[bl_index* ssnum + ss_index]*100.0,
				gff_err[bl_index* ssnum + ss_index]*100.0,
				gff_result[(blnum + bl_index)* ssnum + ss_index]*RADDEG,
				gff_err[(blnum + bl_index)* ssnum + ss_index]*RADDEG);
		}
	}
*/


	icon = gff_mult_acc( &shrd_vismap_ptr[0],
				&shrd_vismap_ptr[blnum*ssnum],
				ssnum, freq_num, rf, freq_incr, time_num, time_incr,
				antnum, gff_result, gff_err);

	printf("******** SUMMERY OF GLOBAL FRINGE SEARCH ********\n");
	fmjd2doy( start_mjd, &year, &doy, &hour, &min, &sec);
	printf(" START = %03d %02d:%02d:%02d, INTEG TIME = %lf\n",
			doy, hour, min, (int)sec, time_num*time_incr);
	for(stn_index=0; stn_index<antnum-1; stn_index++){
		printf(" STN %d : DELAY= %7.5lf +/- %8.7lf, RATE= %7.5lf +/- %8.7lf\n",
				stnid_in_cfs[stn_index+1],
				-gff_result[2*blnum*ssnum + stn_index],
				gff_err[2*blnum*ssnum + stn_index],
				-gff_result[2*blnum*ssnum + antnum + stn_index - 1]*1.0e6,
				gff_err[2*blnum*ssnum + antnum + stn_index - 1]*1.0e6);
	}
	printf(" STN %d : ACC= %7.5lf +/- %8.7lf [mHz/sec]\n",
			stnid_in_cfs[stn_index+1],
			-gff_result[2*blnum*ssnum + 2* antnum - 2]* 1.0e3
			/(2.0* PI* time_incr* time_incr ),
			gff_err[2*blnum*ssnum + 2* antnum - 2]* 1.0e3
			/(2.0* PI* time_incr* time_incr ) );
/*
------------------------ SAVE GFF RESULT TO FCAL-DATA
*/
	for(stn_index=0; stn_index<antnum; stn_index++){

		save_gff( argv[1], obj_name, first_stn_ptr,
			stnid_in_cfs[stn_index], stnid_in_cfs[0],
			stn_index, antnum, ssnum, first_cor_ptr->nss, ss_id, start_mjd,
			time_num*time_incr, gff_result, gff_err);
	}

			
	for(bl_index=0; bl_index<blnum; bl_index++){
		for(ss_index=0; ss_index<ssnum; ss_index++){
			printf( VISIB_FMT, bl_index, ss_index,
			gff_result[bl_index* ssnum + ss_index]*100.0,
			gff_err[bl_index* ssnum + ss_index]*100.0,
			gff_result[(blnum + bl_index)* ssnum + ss_index]*RADDEG,
			gff_err[(blnum + bl_index)* ssnum + ss_index]*RADDEG);
		}
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
