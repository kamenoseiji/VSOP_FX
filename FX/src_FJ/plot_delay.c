/*********************************************************
**	PLOT_DELAY.C :	Plot Antenna-Based Delay Data		**
**														**
**	AUTHOR  : KAMENO Seiji								**
**	CREATED : 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"

#define	HDUNIT		3
#define	MAX_ANT		10

	struct	header		obs;			/* OBS HEADDER */
	struct	head_obj	*obj_ptr;		/* Pointer of Objects Header */
	struct	head_obj	*first_obj_ptr;	/* First Pointer of Objects Header */
	struct	head_stn	*stn_ptr;		/* Pointer of Station Header */
	struct	head_stn	*first_stn_ptr;	/* First Pointer of Station Header */
	long	fcal_ptr[MAX_ANT];			/* Pointer of CLOCK data */

	int		shrd_obj_id;				/* Shared Memory ID for Source HD */
	int		shrd_stn_id;				/* Shared Memory ID for Station HD */
	int		obj_id;						/* OBJECT ID */
	int		stn_index;					/* Index for Station */
	int		refant_id;					/* REFANT ID in CFS */
	int		ret;						/* Return Code from CFS Library */
	int		lunit;						/* Unit Number of CFS File */
	int		recid;						/* Record ID in CFS */
	char	fname[128];					/* File Name of CFS Files */
	char	omode[2];					/* Access Mode of CFS Files */
	int		ssnum;						/* Number of Sub-Stream */
	double	loss;						/* Loss Factor */
	double	mjd_min,	mjd_max;		/* Max and Min of MJD */
	double	rate_min,	rate_max;		/* Max and Min of Rate */
	double	delay_min,	delay_max;		/* Max and Min of Delay */
	double	acc_min,	acc_max;		/* Max and Min of Acc */
	double	curr_rate_min,	curr_rate_max;		/* Max and Min of Rate */
	double	curr_delay_min,	curr_delay_max;		/* Max and Min of Delay */
	double	atm_prm[3*MAX_ANT-1];		/* Atmospheric Parameters */

	char	pg_dev[256];				/* PGPLOT Device Name */
	int		err_code;

MAIN__(argc, argv)
	int		argc;			/* Number of Arguments */
	char	**argv;			/* Pointer of Arguments */
{

	memset( fcal_ptr, 0, sizeof(fcal_ptr) );

/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 4 ){
		printf("USAGE : plot_delay [OBS_NAME] [SRC_NAME] [DEVICE] [STN_NAME1] [STN_NAME2] ... !!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  SRC_NAME ------ Object Name\n");
		printf("  DEVICE -------- PGPLOT Device \n");
		printf("  STN_NAME ------ STATION NAMEs (`all' is acceptable)\n");
		exit(0);
	}

	cfs000_( &ret );			cfs_ret( 000, ret );
	cfs020_( &ret );			cfs_ret( 020, ret );
	cfs006_( argv[1], &ret, strlen( argv[1] ));	cfs_ret( 006, ret );

	mjd_max		= -99999.0;	mjd_min		=  999999.0;
	rate_max	= -99999.0;	rate_min	=  999999.0;
	delay_max	= -99999.0;	delay_min	=  999999.0;
	acc_max		= -99999.0;	acc_min		=  999999.0;

	/*-------- OPEN PGPLOT DEVICE --------*/
	if( strstr(argv[3], "/cps") != NULL){
		sprintf( pg_dev, "pgplot.ps/vcps");
		printf( "SAVE PGPLOT TO %s\n", pg_dev );
		cpgbeg(1, pg_dev, 1, 1);
		cpgscrn(0, "White", &err_code); /* COLOR DEFINISHON */
		cpgscrn(13, "Black", &err_code);         /* COLOR DEFINISHON */
		cpgscrn(14, "ivory", &err_code);     /* COLOR DEFINISHON */
		cpgscrn(15, "Yellow", &err_code);        /* COLOR DEFINISHON */

	} else if( strstr(argv[3], "/ps") != NULL){
		sprintf( pg_dev, "pgplot.ps/vps");
		printf( "SAVE PGPLOT TO %s\n", pg_dev );
		cpgbeg(1, pg_dev, 1, 1);

		cpgscrn(0, "White", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(13, "Black", &err_code);	/* COLOR DEFINISHON */
		cpgscrn(14, "Gray", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(15, "Black", &err_code);	/* COLOR DEFINISHON */

	} else {
		cpgbeg(1, argv[3], 1, 1);
		cpgscrn(0, "DarkSlateGray", &err_code); /* COLOR DEFINISHON */
		cpgscrn(13, "White", &err_code);         /* COLOR DEFINISHON */
		cpgscrn(14, "SlateGray", &err_code);     /* COLOR DEFINISHON */
		cpgscrn(15, "Yellow", &err_code);        /* COLOR DEFINISHON */
	}
	cpgeras();

	/*-------- FILE OPEN --------*/
	lunit	= HDUNIT;
	sprintf( fname, "HEADDER" );
	sprintf( omode, "r" );
	cfs287_( fname, &ret, strlen(fname) );	cfs_ret( 287, ret );
	cfs103_( &lunit, fname, omode, &ret, strlen(fname), strlen(omode) );
	cfs_ret( 103, ret );

	/*-------- READ OBSHEAD --------*/
	read_obshead( lunit, &obs, &obj_ptr, &stn_ptr, &shrd_obj_id, &shrd_stn_id );

	first_obj_ptr	= obj_ptr;
	first_stn_ptr	= stn_ptr;

	/*-------- LINK STN-ID to AUTOCORR PAIR ID --------*/
	acorr_pair( &obs, stn_ptr, &ssnum, &loss );

	/*-------- SELECT STATION --------*/
	stn_index = 0;
	if( strstr(argv[argc - 1], "all") != NULL ){

		/*-------- SELECTED ALL STATIONS --------*/
		while(stn_ptr != NULL){
			printf("STATION %-10s: ID = %2d\n",
				stn_ptr->stn_name, stn_ptr->stn_index );

			/*-------- MAKE STATION-BASED BANDPASS --------*/
			if(stn_ptr->acorr_index != -1){

				if( read_delay( stn_ptr->stn_index,		argv[2],
					&fcal_ptr[stn_index],	&mjd_min,	&mjd_max,
					&curr_rate_min,		&curr_rate_max,
					&curr_delay_min,	&curr_delay_max,
					&acc_min,	&acc_max) == -1){

					/*-------- IN CASE OF REFANT --------*/
					refant_id	= stn_ptr->stn_index;
				} else {
					/*-------- NON-REFANT --------*/
					if(curr_delay_min <delay_min){delay_min = curr_delay_min;}
					if(curr_delay_max >delay_max){delay_max = curr_delay_max;}
					if(curr_rate_min <rate_min){rate_min = curr_rate_min;}
					if(curr_rate_max >rate_max){rate_max = curr_rate_max;}
					stn_index++;
				}
			}
			stn_ptr = stn_ptr->next_stn_ptr;
		}

	}
	shmctl( shrd_obj_id, IPC_RMID, 0 );
	shmctl( shrd_stn_id, IPC_RMID, 0 );

/*
	atmdelay_solve( &obs, first_obj_ptr, refant_id, fcal_ptr, first_stn_ptr,
			atm_prm);
*/

	cpg_delay( &obs, first_obj_ptr, refant_id, fcal_ptr, first_stn_ptr,
				atm_prm, mjd_min, mjd_max, rate_min, rate_max,
				delay_min,	delay_max,	acc_min,	acc_max);

/*
	cpg_delay_el( &obs, first_obj_ptr, refant_id, fcal_ptr,		first_stn_ptr,
				mjd_min,	mjd_max,	rate_min,	rate_max,
				delay_min,	delay_max,	acc_min,	acc_max);
*/

	cpgend();
	return(0);
}
