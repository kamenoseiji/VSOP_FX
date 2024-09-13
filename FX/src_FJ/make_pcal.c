/*********************************************************
**	MAKEPCAL.C	: Global Fringe Search using 		**
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
#define	MAX_STN	10
#define	MAX_BL	45
#define	MAX_SS	32	
#define	MAX_CH	1024
#define RADDEG	57.29577951308232087721	
#define SECDAY	86400	
#define OBS_NAME	1
#define SOURCE		2
#define START		3
#define STOP		4
#define INTEG		5
#define TOTAL_STN	6
#define TOTAL_SS	7
#define STN_NAME	8
#define	MATCH11		(cor_ptr->ant1 == cfs_stnid[ant1])
#define	MATCH12		(cor_ptr->ant1 == cfs_stnid[ant2])
#define	MATCH21		(cor_ptr->ant2 == cfs_stnid[ant1])
#define	MATCH22		(cor_ptr->ant2 == cfs_stnid[ant2])
#define	BLMAP_FMT	"BL%d %-9s - %-9s  CFS_CORR= %d [%s]\n"
#define	REFANT		-1

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

	/*-------- POINTER --------*/
	int		fcal_addr[MAX_STN];			/* Pointer of CLOCK data			*/
	int		time_ptr_ptr[MAX_STN];		/* Pointer of FCAL Time List		*/
	int		delay_ptr_ptr[MAX_STN];		/* Pointer of FCAL Delay List		*/
	int		rate_ptr_ptr[MAX_STN];		/* Pointer of FCAL Rate List		*/
	int		acc_ptr_ptr[MAX_STN];		/* Pointer of FCAL Acc List			*/
	int		delaywgt_ptr_ptr[MAX_STN];	/* Pointer of FCAL Delay Err List	*/
	int		ratewgt_ptr_ptr[MAX_STN];	/* Pointer of FCAL Rate Err List	*/
	int		accwgt_ptr_ptr[MAX_STN];	/* Pointer of FCAL Acc Err List		*/
	int		delay_coeff[MAX_STN];		/* Pointer of Delay Spline Coeff	*/
	int		rate_coeff[MAX_STN];		/* Pointer of Rate Spline Coeff		*/
	int		acc_coeff[MAX_STN];			/* Pointer of Acc Spline Coeff		*/
	int		time_node[MAX_STN];			/* Pointer of Time Node				*/
	int		time_rate_node[MAX_STN];	/* Pointer of Time Node				*/
	int		*vismap_ptr;				/* First Pointer of memory map		*/
	double	*bp_r_ptr[MAX_STN][MAX_SS];	/* Pointer of Bandpass (REAL)		*/
	double	*bp_i_ptr[MAX_STN][MAX_SS];	/* Pointer of Bandpass (IMAG)		*/

	/*-------- INDEX --------*/
	int		stn_index;					/* Index for Station				*/
	int		bl_index;					/* Index for Baseline				*/
	int		ss_index;					/* Index of Sub-Stream				*/
	int		time_index;					/* Index for Time					*/
	int		freq_index;					/* Index for Frequency				*/
	int		ant1, ant2;					/* Antenna Index in BL				*/
	int		delay_index;				/* Index of Delay in gff_result[]	*/
	int		rate_index;					/* Index of Rate in gff_result[]	*/

	/*-------- TOTAL NUMBER --------*/
	int		stn_num;					/* Number of Station				*/
	int		bl_num;						/* Total Number of Baseline			*/
	int		ss_num;						/* Number of Sub-Stream				*/
	int		cfs_ssnum;					/* Number of Sub-Stream in CFS		*/
	int		freq_num[MAX_SS];			/* Number of Frequency				*/
	int		valid_pp;					/* Valid PP Number					*/
	int		time_num_cfs;				/* Number of Time Data				*/
	int		time_num;					/* Number of Time Data				*/
	int		integ_pp;					/* Coherent Integration PP			*/
	int		bp_ssnum;					/* SS-Number in Band-Pass File		*/
	int		bp_freq_num[MAX_SS];		/* Frequency Channel Number			*/
	int		fcal_num[MAX_STN];			/* Number of F-CAL data				*/
	int		node_delay_num[MAX_STN];	/* Total Number of Delay Node		*/
	int		node_rate_num[MAX_STN];		/* Total Number of Rate Node		*/

	/*-------- IDENTIFIER --------*/
	int		obj_id;						/* OBJECT ID						*/
	int		cfs_ssid[MAX_SS];			/* SS ID in CFS						*/
	int		cfs_blid[MAX_BL];			/* Baseline ID in CFS				*/
	int		cfs_stnid[MAX_STN];			/* Station ID in CFS				*/
	int		refant_id;					/* Station ID of REF ANT			*/
	int		ret;						/* Return Code from CFS Library		*/
	int		lunit;						/* Unit Number of CFS File			*/
	int		position;					/* Search Position					*/
	int		stn_arg;					/* Arg Index which STATION starts	*/

	/*-------- GENERAL VARIABLES --------*/
	int		start_time;					/* START TIME [DDDHHMMSS]			*/
	int		stop_time;					/* STOP TIME [DDDHHMMSS]			*/
	int		year, doy;					/* YEAR and Day of Year				*/
	int		hour, min;					/* Hour and Minute					*/
	int		solint;						/* Solution Interval				*/
	float	bl_dir[MAX_STN];			/* Baseline Direction				*/
	char	fname[128];					/* File Name of CFS Files			*/
	char	omode[2];					/* Access Mode of CFS Files			*/
	char	obj_name[32];				/* Object Name						*/
	char	stn_list[MAX_STN][32];		/* Station Name						*/
	double	start_mjd;					/* Start Time [MJD]					*/
	double	first_mjd;					/* Start Time [MJD]					*/
	double	stop_mjd;					/* Stop Time [MJD]					*/
	double	time_ip;					/* Second of Day					*/
	double	integ;						/* Integration Time [sec]			*/
	double	loss;						/* Quantize Efficiency				*/
	double	rf[MAX_SS];					/* RF Frequency [MHz]				*/
	double	freq_incr[MAX_SS];			/* Frequency Increment [MHz]		*/
	double	time_incr;					/* Time Increment [sec]				*/
	double	uvw[MAX_BL][3];				/* (u, v, w) Coordinate				*/
	float	work[2*MAX_CH];				/* Work Area to Read Visibility		*/
	double	*gff_result;				/* RESULT of GFF					*/
	double	*gff_err;					/* ERROR of GFF						*/
	double	sec;						/* Second							*/
	double	mjd_min,	mjd_max;		/* Min and Max of MJD				*/
	double	rate_min,	rate_max;		/* Min and Max of RATE				*/
	double	delay_min,	delay_max;		/* Min and Max of DELAY				*/
	double	acc_min,	acc_max;		/* Min and Max of ACC				*/
	char	bp_obj_name[32];			/* Object Name in Band-Pass File	*/
	double	bp_mjd;						/* MJD of the Band-Pass File		*/
	double	bp_integ_time;				/* Integ Time of the Band-Pass File	*/
	double	bp_rf[MAX_SS];				/* RF Freq. [MHz] in BP File		*/
	double	bp_freq_incr[MAX_SS];		/* Freq. Increment [MHz] in BP File	*/
	double	bp_vis_max[MAX_STN][MAX_SS];/* Maximum of Bandpass				*/
	double	delay[MAX_STN];				/* Station Delay					*/
	double	rate[MAX_STN];				/* Station Rate						*/
	double	blphs[MAX_BL];
	double	blerr[MAX_BL];
	double	antphs[MAX_SS][MAX_STN];
	double	anterr[MAX_SS][MAX_STN];
	double	pcphs[MAX_STN][MAX_SS];
	double	pcerr[MAX_STN][MAX_SS];

	memset( fcal_addr, 0, sizeof(fcal_addr) );
/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 10 ){
		printf("USAGE : make_pcal [OBS_NAME] [SOURCE] [START] [STOP] [INTEG] [TOTAL STN] [TOTAL SS] [STN_NAME1] [STN_NAME2] ... [SS1] [SS2]... !!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  SOURCE -------- Source Name\n");
		printf("  START --------- Start TIME [DDDHHMMSS] \n");
		printf("  STOP ---------- Stop TIME  [DDDHHMMSS] \n");
		printf("  INTEG --------- Integration TIME  [sec] \n");
		printf("  TOTAL STN ----- TOTAL NUMBER OF STATION\n");
		printf("  TOTAL SS ------ TOTAL NUMBER OF SUB-STREAM\n");
		printf("  STN_NAME ------ STATION NAMEs \n");
		exit(0);
	}

	/*-------- START and STOP Time --------*/
	start_time	= atoi(argv[START]);
	stop_time	= atoi(argv[STOP]);
	sprintf(obj_name, "%s", argv[SOURCE]);
	integ		= atof(argv[INTEG]);
	stn_num		= atoi( argv[TOTAL_STN] );
	ss_num		= atoi( argv[TOTAL_SS] );
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
	acorr_pair( &obs, stn_ptr, &cfs_ssnum, &loss);

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

	/*-------- RELATION BETWEEN INPUT SS and CFS SS LIST --------*/
	for(ss_index=0; ss_index<ss_num; ss_index++ ){
		cfs_ssid[ss_index] = atoi(argv[argc - ss_num + ss_index]) + 1;
	}

	/*-------- RELATION BETWEEN INPUT STATION and CFS STATION LIST --------*/
	for(stn_index=0; stn_index<stn_num; stn_index++ ){
		stn_ptr = first_stn_ptr;			/* Go To Top of List		*/
		cfs_stnid[stn_index] = -1;			/* Invalid Flag at Init		*/
		while( stn_ptr != NULL ){
			if( strstr( stn_ptr->stn_name, argv[STN_NAME+stn_index]) != NULL){
				cfs_stnid[stn_index] = stn_ptr->stn_index;
				sprintf(stn_list[stn_index], "%s\0", stn_ptr->stn_name);
				break; }
			stn_ptr = stn_ptr->next_stn_ptr;
		}
	}

	/*-------- READ BANDPASS DATA --------*/
	for(stn_index=0; stn_index<stn_num; stn_index++ ){
		read_bp(cfs_stnid[stn_index], bp_obj_name,
			&bp_ssnum, bp_freq_num, &bp_mjd, &bp_integ_time,
			bp_rf, bp_freq_incr,
			bp_r_ptr[stn_index], bp_i_ptr[stn_index], bp_vis_max[stn_index]);
	}

	/*-------- RELATION BETWEEN INPUT STATION and CFS BL LIST --------*/
    bl_num = (stn_num* (stn_num - 1))/2;
	for(bl_index=0; bl_index<bl_num; bl_index++ ){
		bl2ant(bl_index, &ant2, &ant1);
		cor_ptr = first_cor_ptr;            /* Go To Top of List        */
		bl_dir[bl_index] = 0;               /* Invalid Flag at Init     */
		while( cor_ptr != NULL ){
			if( MATCH11 && MATCH22 ){
				bl_dir[bl_index] = 1;
				cfs_blid[bl_index] = cor_ptr->cor_id;
				printf(BLMAP_FMT, bl_index, stn_list[ant1], stn_list[ant2],
					cor_ptr->cor_id, "NORMAL"); break; }
			if( MATCH12 && MATCH21 ){
				bl_dir[bl_index] = -1;
				cfs_blid[bl_index] = cor_ptr->cor_id;
				printf(BLMAP_FMT, bl_index, stn_list[ant1], stn_list[ant2],
					cor_ptr->cor_id, "INVERT"); break; }
			cor_ptr = cor_ptr->next_cor_ptr;
		}
	}

	/*-------- DELAY and RATE DATA of SPECIFIED STATIONS --------*/
	solint = atoi( argv[INTEG] );
	for(stn_index=0; stn_index<stn_num; stn_index++ ){
		fcal_num[stn_index] = read_delay(
			cfs_stnid[stn_index],	"all",	&fcal_addr[stn_index],
			&mjd_min, &mjd_max, &rate_min, &rate_max, &delay_min, &delay_max,
			&acc_min, &acc_max);

		if( fcal_num[stn_index] != REFANT ){
			restore_delay( fcal_addr[stn_index], fcal_num[stn_index],
				&time_ptr_ptr[stn_index],		&delay_ptr_ptr[stn_index],
				&rate_ptr_ptr[stn_index],		&acc_ptr_ptr[stn_index],
				&delaywgt_ptr_ptr[stn_index],	&ratewgt_ptr_ptr[stn_index],
				&accwgt_ptr_ptr[stn_index]);

			real_spline(time_ptr_ptr[stn_index],	delay_ptr_ptr[stn_index],
				delaywgt_ptr_ptr[stn_index],		fcal_num[stn_index],
				(double)solint,						&node_delay_num[stn_index],
				&delay_coeff[stn_index],			&time_node[stn_index]);

			real_spline(time_ptr_ptr[stn_index],	rate_ptr_ptr[stn_index],
				ratewgt_ptr_ptr[stn_index],			fcal_num[stn_index],
				(double)solint,						&node_rate_num[stn_index],
				&rate_coeff[stn_index],				&time_rate_node[stn_index]);
		}
	}
/*
------------------------ PREPARE MEMORY AREA
*/
	vismap_ptr	= (int *)malloc( 2* bl_num* ss_num* sizeof(int));
	gff_result	= (double *)malloc( 2*(bl_num*ss_num + stn_num)
				* sizeof(double));
	gff_err		= (double *)malloc( 2*(bl_num*ss_num + stn_num)
				* sizeof(double));
	memset( gff_result, 0, sizeof(gff_result));
	memset( gff_err, 0, sizeof(gff_result));


	/*-------- ALLOCATE MEMORY to STORE VISIBLITY --------*/
	for(bl_index=0; bl_index<bl_num; bl_index++ ){
		for(ss_index=0; ss_index<ss_num; ss_index++ ){

			/*-------- READ SUB_STREAM HEADDER --------*/
			read_sshead(cfs_blid[bl_index], cfs_ssid[ss_index], &rf[ss_index],
				&freq_incr[ss_index], &freq_num[ss_index], 
				&time_num_cfs, &time_incr);

			/*-------- PP NUMBER ROUND UP TO 2^n --------*/
			time_num	= (int)(integ/time_incr)+1;
			integ_pp	= pow2round(time_num);

			/*-------- ALLOCATE MEMORY AREA --------*/
			memalloc( freq_num[ss_index], integ_pp,
				&vismap_ptr[bl_index*ss_num + ss_index],
				&vismap_ptr[(bl_num + bl_index)*ss_num + ss_index]);
		}
	}
/*
------------------------ LOAD VISIBILITY DATA TO MEMORY
*/
	time_index	= 0;
	first_mjd = start_mjd;
	/*-------- START TIME LOOP --------*/
	while(start_mjd + (double)(time_incr*time_num/2)/SECDAY < stop_mjd){
		time_ip = SECDAY*(start_mjd - (int)first_mjd) + integ/2.0;
		fmjd2doy( start_mjd, &year, &doy, &hour, &min, &sec);
		printf("INTEG START = %03d %02d:%02d:%02d\n", doy, hour, min, (int)sec);

		/*-------- CALC DELAY and RATE BY INTERPOL --------*/
		for(stn_index=0; stn_index<stn_num; stn_index++ ){
			delay_index	= 2* bl_num* ss_num + stn_index;
			rate_index	= delay_index + stn_num;

			interp_real( time_ip, node_delay_num[stn_index],
				time_node[stn_index], delay_coeff[stn_index],&delay[stn_index]);

			interp_real( time_ip, node_delay_num[stn_index],
				time_node[stn_index], rate_coeff[stn_index], &rate[stn_index]);

			gff_result[delay_index]	= delay[stn_index];
			gff_result[rate_index]	= rate[stn_index];
        }

		/*-------- LOAD BASELINE-BASE VISIBILITY TO MEMORY --------*/
		for( bl_index=0; bl_index<bl_num; bl_index++){
			position = -1;
			bl2ant( bl_index, &ant2, &ant1 );
			for(ss_index=0; ss_index<ss_num; ss_index++){

				/*-------- READ SUB_STREAM HEADDER --------*/
				read_sshead(cfs_blid[bl_index], cfs_ssid[ss_index],
					&rf[ss_index], &freq_incr[ss_index], &freq_num[ss_index], 
					&time_num_cfs, &time_incr);

				/*-------- LOAD VISIBILITY DATA  CFS -> MEMORY --------*/
				valid_pp= load_vis(cfs_blid[bl_index], bl_dir[bl_index], obj_id,
					cfs_ssid[ss_index], &position, &freq_num[ss_index], 1,
					time_num_cfs, time_num, time_incr, start_mjd, stop_mjd,
					uvw[bl_index],
					bp_r_ptr[ant1][ss_index], bp_r_ptr[ant2][ss_index], work,
					vismap_ptr[bl_index*ss_num + ss_index], 
					vismap_ptr[(bl_num + bl_index)*ss_num + ss_index]);
			}
		} /*-------- END OF BASELINE LOOP --------*/




		/*-------- INTEGRATE VISIBILITY FOR TIME and FREQ. --------*/
		integ_mult_com( &vismap_ptr[0],
				&vismap_ptr[bl_num*ss_num],
				ss_num, freq_num, rf, freq_incr, time_num, time_incr,
				stn_num, gff_result );

		for(ss_index=0; ss_index<ss_num; ss_index++){
			/*-------- STORE VISIBILITY PHASE --------*/
			for( bl_index=0; bl_index<bl_num; bl_index++){
				blphs[bl_index] = gff_result[(bl_num + bl_index)*ss_num
											+ ss_index];
				blerr[bl_index] = 1.0/(sqrt(2.0e6*freq_incr[ss_index]*time_incr
								*(double)freq_num[ss_index]*(double)time_num )
								* gff_result[bl_index*ss_num + ss_index]);
			}

			/*-------- SOLVE FOR ANTENNA-BASED PHASE --------*/
			clphs_solve( stn_num, blphs, blerr,
				antphs[ss_index], anterr[ss_index]);

			for(stn_index=0; stn_index<stn_num; stn_index++){

				#ifdef DEBUG
				printf("ANT_PHASE [SS%d, ANT%d] = %6.3lf +/- %6.3lf\n",
					ss_index, stn_index,
					antphs[ss_index][stn_index], anterr[ss_index][stn_index]);
				#endif

				pcphs[stn_index][ss_index]	= antphs[ss_index][stn_index];
				pcerr[stn_index][ss_index]	= anterr[ss_index][stn_index];
			}

		}/*-------- END OF SS LOOP --------*/


		/*-------- SAVE P-CAL DATA TO CFS --------*/
		for(stn_index=0; stn_index<stn_num; stn_index++){
			save_pcal( argv[OBS_NAME],
					stn_list[stn_index],
					cfs_stnid[stn_index],
					obj_name,	cfs_ssnum, ss_num,	cfs_ssid,	rf,
					start_mjd,	(double)time_num*time_incr,
					pcphs[stn_index], pcerr[stn_index] );
		}

		start_mjd	+= (double)(time_incr*time_num)/86400.0;
		time_index	++;

	}	/*-------- END OF TIME LOOP --------*/


	/*-------- RELEASE MEMORY --------*/
	for(stn_index=0; stn_index<stn_num; stn_index++){
		for(ss_index=0; ss_index<first_cor_ptr->nss; ss_index++){
			free(bp_r_ptr[stn_index][ss_index]);
			free(bp_i_ptr[stn_index][ss_index]);
		}
	}

	/*-------- CLOSE SHARED MEMORY --------*/
	shmctl( shrd_obj_id, IPC_RMID, 0 );
	shmctl( shrd_stn_id, IPC_RMID, 0 );
	return(0);
}
