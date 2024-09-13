/*********************************************************
**	READ_CODE.C	: Test Module to Read CODA File System	**
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
#define	STN_ACORR_DIR	"/sys01/custom/bin/station_acorr"
#define	ARG_SCAN	1
#define	ARG_OBS		2
#define	ARG_SOURCE	3
#define	ARG_START	4
#define	ARG_STOP	5
#define	ARG_TYPE	6
#define	ARG_AE		7
#define	ARG_TSYS	8
#define	ARG_DEVICE	9
#define	ARG_STN		10

	struct	header		obs;			/* OBS HEADDER */
	struct	head_obj	*obj_ptr;		/* Pointer of Objects Header */
	struct	head_obj	*first_obj_ptr;	/* First Pointer of Objects Header */
	struct	head_stn	*stn_ptr;		/* Pointer of Station Header */
	struct	head_stn	*first_stn_ptr;	/* First Pointer of Station Header */

	int		shrd_obj_id;				/* Shared Memory ID for Source HD */
	int		shrd_stn_id;				/* Shared Memory ID for Station HD */
	int		obj_id;						/* OBJECT ID */
	int		stn_index;					/* Index for Station */
	int		ret;						/* Return Code from CFS Library */
	int		scan_type;					/* Scan Type to Integrate			*/
	int		lunit;						/* Unit Number of CFS File */
	char	fname[128];					/* File Name of CFS Files */
	char	omode[2];					/* Access Mode of CFS Files */
	double	start_mjd;					/* Start Time [MJD] */
	double	stop_mjd;					/* Stop Time [MJD] */
	int		ssnum;						/* Number of Sub-Stream */
	double	loss;						/* Quantize Efficiency */
	char	bp_cmd[16][64];
	int		start_time;
	int		stop_time;
	int		year, doy;
	int		selected_stn_num;
	int		stn_arg;

MAIN__( argc, argv )
	int		argc;		/* Number of Arguments */
	char	**argv;		/* Pointer of Arguments */
{

/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 7 ){
		printf("USAGE : coda_acorr [SCAN] [OBS_NAME] [SOURCE] [START] [STOP] [DEVICE] [STN_NAME1] [STN_NAME2] ... !!\n");
		printf("  SCAN ---------- Scan File Name\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  SOURCE -------- Source Name for Band-Pass Calib\n");
		printf("  START --------- Start TIME [DDDHHMMSS] \n");
		printf("  STOP ---------- Stop TIME  [DDDHHMMSS] \n");
		printf("  TYPE ---------- Scan Type to Integ [OFF or ON]\n");
		printf("  DEVICE -------- PGPLOT Device \n");
		printf("  STN_NAME ------ STATION NAMEs (`all' is acceptable)\n");
		exit(0);
	}
	/*-------- START and STOP Time --------*/
	start_time	= atoi(argv[ARG_START]);
	stop_time	= atoi(argv[ARG_STOP]);
	scan_type	= scanchar2type(argv[ARG_TYPE]);
	printf("SCAN TYPE = %s --- MODE = %d\n", argv[ARG_TYPE], scan_type);

	cfs000_( &ret );			cfs_ret( 000, ret );
	cfs020_( &ret );			cfs_ret( 020, ret );
	cfs006_( argv[ARG_OBS], &ret, strlen( argv[ARG_OBS] ));	cfs_ret( 006, ret );

	/*-------- FILE OPEN --------*/
	lunit	= 3;
	sprintf( fname, "HEADDER" );	sprintf( omode, "r" );
	cfs287_( fname, &ret, strlen(fname) );	cfs_ret( 287, ret );
	cfs103_( &lunit, fname, omode, &ret, strlen(fname), strlen(omode) );
	cfs_ret( 103, ret );

	/*-------- READ OBSHEAD --------*/
	read_obshead( lunit, &obs, &obj_ptr, &stn_ptr, &shrd_obj_id, &shrd_stn_id );
	first_obj_ptr	= obj_ptr;
	first_stn_ptr	= stn_ptr;

	/*-------- CONVERT START and STOP TIME -> MJD --------*/
	mjd2doy( (long)obs.start_mjd, &year, &doy );		/* GET OBS YEAR */

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
	objct_id( obj_ptr, argv[ARG_SOURCE], &obj_id );

	/*-------- LINK STN-ID to AUTOCORR PAIR ID --------*/
	acorr_pair( &obs, stn_ptr, &ssnum, &loss );


	/*-------- SELECT STATION --------*/
	if( strstr(argv[argc - 1], "all") != NULL ){

		/*-------- SELECTED ALL STATIONS --------*/
		while(stn_ptr != NULL){
			printf("STATION %-10s: ID = %2d, ACORR = %2d\n",
				stn_ptr->stn_name, stn_ptr->stn_index, stn_ptr->acorr_index );

			/*-------- MAKE STATION-BASED BANDPASS --------*/
			if( stn_ptr->acorr_index != -1){ bp_process(argv); }
			stn_ptr = stn_ptr->next_stn_ptr;
		}
		for( stn_index=0;  stn_index < obs.stn_num; stn_index++){
			wait(NULL);
		}

	} else {

		/*-------- SPECIFIED STATIONS --------*/
		selected_stn_num = 0;
		for(stn_arg=ARG_STN; stn_arg<argc; stn_arg++){
			stn_ptr = first_stn_ptr;
			while(stn_ptr != NULL){

				/*-------- SEEK SPECIFIED STATION --------*/
				if( strstr(stn_ptr->stn_name, argv[stn_arg]) != NULL ){
					printf("STATION %-10s: ID = %2d, ACORR = %2d\n",
						stn_ptr->stn_name, stn_ptr->stn_index,
						stn_ptr->acorr_index );

					/*-------- MAKE STATION-BASED BANDPASS --------*/
					if( stn_ptr->acorr_index != -1){
						bp_process(argv);
						selected_stn_num++;
						break;
					}
				}
				stn_ptr = stn_ptr->next_stn_ptr;
			}
		}
		for( stn_index=0;  stn_index < selected_stn_num; stn_index++){
			wait(NULL);
		}
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
	return(0);
}

bp_process(argv)
	char	**argv;
{
	sprintf( bp_cmd[0], "station_acorr" );
	sprintf( bp_cmd[1], "%s", argv[ARG_SCAN] );
	sprintf( bp_cmd[2], "%s", argv[ARG_OBS] );
	sprintf( bp_cmd[3], "%s", argv[ARG_SOURCE] );
	sprintf( bp_cmd[4], "%d", obs.obj_num );
	sprintf( bp_cmd[5], "%d", stn_ptr->stn_index );
	sprintf( bp_cmd[6], "%d", obs.stn_num );
	sprintf( bp_cmd[7], "%d", ssnum );
	sprintf( bp_cmd[8], "%15.8lf", start_mjd );
	sprintf( bp_cmd[9], "%15.8lf", stop_mjd );
	sprintf( bp_cmd[10], "%d", scan_type );
	sprintf( bp_cmd[11], "%s", argv[ARG_AE] );
	sprintf( bp_cmd[12], "%s", argv[ARG_TSYS] );
	if( strstr(argv[5], "/ps") != NULL ){
		sprintf( bp_cmd[13], "pgplot.%d.ps%s", stn_ptr->stn_index, argv[ARG_DEVICE] );
	} else if( strstr(argv[ARG_DEVICE], "/cps") != NULL ){
		sprintf( bp_cmd[13], "pgplot.%d.cps%s", stn_ptr->stn_index, argv[ARG_DEVICE] );
	} else {
		sprintf( bp_cmd[13], "%s", argv[ARG_DEVICE] );
	}

	if( fork() == 0){
		printf("Exec as a Child Process [PID = %d].\n", getpid() );
		if( execl( STN_ACORR_DIR,
			bp_cmd[0],	bp_cmd[1],	bp_cmd[2],	bp_cmd[3],
			bp_cmd[4],	bp_cmd[5],	bp_cmd[6],	bp_cmd[7],
			bp_cmd[8],	bp_cmd[9],	bp_cmd[10], bp_cmd[11],
			bp_cmd[12],	bp_cmd[13],	
			(char *)NULL) == -1){
			printf("Error in EXECL \n");
			exit(1);
		}
	}
	return;
}
