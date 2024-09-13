/*********************************************************
**	CODA_ACSCL.C: Scaling Antenna Gain for the REF ant	**
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
#define	REFSCL	"/sys01/custom/bin/refscl"
#define	GCAL	"/sys01/custom/bin/gcal"
#define BOLTZ	2761.324				/* 2000 times of k */

	struct	header		obs;			/* OBS HEADDER */
	struct	head_obj	*obj_ptr;		/* Pointer of Objects Header */
	struct	head_obj	*first_obj_ptr;	/* First Pointer of Objects Header */
	struct	head_stn	*stn_ptr;		/* Pointer of Station Header */
	struct	head_stn	*first_stn_ptr;	/* First Pointer of Station Header */

	key_t	spec_key;
	int		shrd_obj_id;				/* Shared Memory ID for Source HD */
	int		shrd_stn_id;				/* Shared Memory ID for Station HD */
	int		shrd_spec_id;				/* Shared Memory ID for Spectrum */
	int		obj_id;						/* OBJECT ID */
	int		stn_index;					/* Index for Station */
	int		ret;						/* Return Code from CFS Library */
	int		lunit;						/* Unit Number of CFS File */
	char	fname[128];					/* File Name of CFS Files */
	char	omode[2];					/* Access Mode of CFS Files */
	double	blstart_mjd, blstop_mjd;	/* Start and Stop Time [MJD] */
	double	start_mjd, stop_mjd;		/* Start and Stop Time [MJD] */
	int		ssnum;						/* Number of Sub-Stream */
	double	loss;						/* Quantize Efficiency */
	double	afact;
	double	*shrd_spec_ptr;
	char	bp_cmd[16][16];
	int		blstart_time, blstop_time;
	int		start_time, stop_time;
	int		soy, year, doy;
	int		mjd;

MAIN__( argc, argv )
	int		argc;		/* Number of Arguments */
	char	**argv;		/* Pointer of Arguments */
{

/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 11 ){
		printf("USAGE : coda_acscl [OBS_NAME] [SRC_NAME] [BL_START] [BL_STOP] [SS_ID] [STN_NAME] [AFACT] [START] [STOP] [INTEG]!!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  SOURCE -------- Source Name to be Calibrated\n");
		printf("  BL_START ------ Start Time [DDDHHMMSS] to Determine Baseline\n");
		printf("  BL_STOP ------- Stop Time  [DDDHHMMSS] to Determine Baseline\n");
		printf("  SS_ID --------- SUB-STREAM to be used\n");
		printf("  STN_NAME ------ STATION NAME\n");
		printf("  AFACT    ------ SENSITIVITY (Ae/Tsys) OF THE REF ANT [m2/K]\n");
		printf("  START --------- Start Time [DDDHHMMSS] for Gain Correction\n");
		printf("  STOP ---------- Stop Time [DDDHHMMSS] for Gain Correction\n");
		printf("  INTEG --------- Integration Time [sec]\n");
		exit(0);
	}

	/*-------- START and STOP Time --------*/
	blstart_time	= atoi(argv[3]);
	blstop_time		= atoi(argv[4]);
	start_time		= atoi(argv[8]);
	stop_time		= atoi(argv[9]);
	afact			= BOLTZ / atof(argv[7]);

	cfs000_( &ret );			cfs_ret( 000, ret );
	cfs020_( &ret );			cfs_ret( 020, ret );
	cfs006_( argv[1], &ret, strlen( argv[1] ));	cfs_ret( 006, ret );

	/*-------- FILE OPEN --------*/
	lunit	= 3;
	sprintf( fname, "HEADDER" );
	sprintf( omode, "r" );
	cfs287_( fname, &ret, strlen(fname) );	cfs_ret( 287, ret );
	cfs103_( &lunit, fname, omode, &ret, strlen(fname), strlen(omode) );
	cfs_ret( 103, ret );

	/*-------- READ OBSHEAD --------*/
	read_obshead( lunit, &obs, &obj_ptr, &stn_ptr, &shrd_obj_id, &shrd_stn_id );
	mjd2doy( (long)obs.start_mjd, &year, &doy );

	doy2fmjd( year, blstart_time/1000000,				/* YEAR and DOY */
		(blstart_time/10000)%100, (blstart_time/100)%100,/* Hour and Minute */
		(double)(blstart_time%100),						/* Second */
		&blstart_mjd );
	doy2fmjd( year, blstop_time/1000000,				/* YEAR and DOY */
		(blstop_time/10000)%100, (blstop_time/100)%100,	/* Hour and Minute */
		(double)(blstop_time%100),						/* Second */
		&blstop_mjd );
	doy2fmjd( year, start_time/1000000,					/* YEAR and DOY */
		(start_time/10000)%100, (start_time/100)%100,	/* Hour and Minute */
		(double)(start_time%100),						/* Second */
		&start_mjd );
	doy2fmjd( year, stop_time/1000000,					/* YEAR and DOY */
		(stop_time/10000)%100, (stop_time/100)%100,		/* Hour and Minute */
		(double)(stop_time%100),						/* Second */
		&stop_mjd );

	/*-------- VERIFY START and STOP TIME --------*/
	if(blstart_mjd > obs.stop_mjd){
		printf("INTEG START [MJD=%lf] EXCEEDS OBS END TIME [MJD=%lf]!!\n",
		blstart_mjd, obs.stop_mjd);
	}

	if(blstop_mjd < obs.start_mjd){
		printf("INTEG STOP [MJD=%lf] IS BEFORE OBS START TIME [MJD=%lf]!!\n",
		stop_mjd, obs.start_mjd);
	}

	first_obj_ptr	= obj_ptr;
	first_stn_ptr	= stn_ptr;

	/*-------- LINK SOURCE NAME to OBJECT ID --------*/
	objct_id( obj_ptr, argv[2], &obj_id );

	/*-------- LINK STN-ID to AUTOCORR PAIR ID --------*/
	acorr_pair( &obs, stn_ptr, &ssnum, &loss );

	spec_key	= ftok(KEY_DIR, SPC_KEY);
	if(( shrd_spec_id
		= shmget( spec_key, 7*sizeof(double), IPC_CREAT | 0644)) < 0){
		printf("Error in [shmget]!!, %s !!\n", argv[0]);
		exit(1);
	}
	shrd_spec_ptr	= (double *)shmat(shrd_spec_id, NULL, 0);

	/*-------- SELECT REFERENCE STATION --------*/
	stn_ptr = first_stn_ptr;
	while(stn_ptr != NULL){

		/*-------- SEEK SPECIFIED STATION --------*/
		if( strstr(stn_ptr->stn_name, argv[6]) != NULL ){
			printf("REFERENCE STATION %-10s: ID = %2d, ACORR = %2d\n",
				stn_ptr->stn_name, stn_ptr->stn_index,
				stn_ptr->acorr_index );

			/*-------- MAKE STATION-BASED BANDPASS --------*/
			if( stn_ptr->acorr_index != -1){
				bp_process(argv);
				break;
			}
		}
		stn_ptr = stn_ptr->next_stn_ptr;
	}
	wait(NULL);

	stn_ptr = first_stn_ptr;
	while(stn_ptr != NULL){
		if( stn_ptr->acorr_index != -1){
			gcal_process(argv);
			wait(NULL);
		}
		stn_ptr = stn_ptr->next_stn_ptr;
	}

	/*-------- CLOSE SHARED MEMORY --------*/
	if( shmctl( shrd_obj_id, IPC_RMID, 0 ) < 0 ){
		printf("Error in [shmctl] : %s\n", argv[0] );
		exit(1);
	}

	if( shmctl( shrd_stn_id, IPC_RMID, 0 ) < 0 ){
		printf("Error in [shmctl] : %s\n", argv[0] );
		exit(1);
	}

	if( shmctl( shrd_spec_id, IPC_RMID, 0 ) < 0 ){
		printf("Error in [shmctl] : %s\n", argv[0] );
		exit(1);
	}

	return(0);
}

bp_process(argv)
	char	**argv;
{
	sprintf( bp_cmd[0], "refscl" );
	sprintf( bp_cmd[1], "%s", argv[1] );			/* OBS CODE */
	sprintf( bp_cmd[2], "%s", argv[2] );			/* SOURCE NAME */
	sprintf( bp_cmd[3], "%d", obs.obj_num );		/* Number of Sources */
	sprintf( bp_cmd[4], "%d", stn_ptr->stn_index );	/* STATION INDEX */
	sprintf( bp_cmd[5], "%d", obs.stn_num );		/* Number of Stations */
	sprintf( bp_cmd[6], "%s", argv[5] );			/* SS Index */
	sprintf( bp_cmd[7], "%d", ssnum );				/* Number of SS */
	sprintf( bp_cmd[8], "%lf",blstart_mjd );		/* MJD at INTEG Start */
	sprintf( bp_cmd[9], "%lf",blstop_mjd );			/* MJD at INTEG Stop */
	sprintf( bp_cmd[10],"%lf",afact );				/* Antenna Gain */

	if( fork() == 0){
		printf("Exec as a Child Process [PID = %d].\n", getpid() );
		if( execl( REFSCL,
			bp_cmd[0],	bp_cmd[1],	bp_cmd[2],	bp_cmd[3],
			bp_cmd[4],	bp_cmd[5],	bp_cmd[6],	bp_cmd[7],
			bp_cmd[8],	bp_cmd[9],	bp_cmd[10],
			(char *)NULL) == -1){
			printf("Error in EXECL \n");
			exit(1);
		}
	}
	return;
}

gcal_process(argv)
	char	**argv;
{
	sprintf( bp_cmd[0], "gcal" );
	sprintf( bp_cmd[1], "%s", argv[1] );			/* OBS CODE */
	sprintf( bp_cmd[2], "%s", argv[2] );			/* SOURCE NAME */
	sprintf( bp_cmd[3], "%d", obs.obj_num );		/* Number of Sources */
	sprintf( bp_cmd[4], "%d", stn_ptr->stn_index );	/* STATION INDEX */
	sprintf( bp_cmd[5], "%d", obs.stn_num );		/* Number of Stations */
	sprintf( bp_cmd[6], "%s", argv[5] );			/* SS Index */
	sprintf( bp_cmd[7], "%d", ssnum );				/* Number of SS */
	sprintf( bp_cmd[8], "%15.8lf",start_mjd );		/* MJD at INTEG Start */
	sprintf( bp_cmd[9], "%15.8lf",stop_mjd );		/* MJD at INTEG Stop */
	sprintf( bp_cmd[10],"%lf", atof(argv[10]) );	/* Integ Time */

	if( fork() == 0){
		printf("Exec as a Child Process [PID = %d].\n", getpid() );
		if( execl( GCAL,
			bp_cmd[0],	bp_cmd[1],	bp_cmd[2],	bp_cmd[3],
			bp_cmd[4],	bp_cmd[5],	bp_cmd[6],	bp_cmd[7],
			bp_cmd[8],	bp_cmd[9],	bp_cmd[10],
			(char *)NULL) == -1){
			printf("Error in EXECL \n");
			exit(1);
		}
	}
	return;
}
