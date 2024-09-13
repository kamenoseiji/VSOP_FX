/*********************************************************
**	SORT_DELAY.C :	Sort Antenna-Based Delay Data		**
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
#include <string.h>
#include "obshead.inc"

#define	HDUNIT		3
#define	MAX_ANT		10
#define	STN_ID		2

MAIN__(argc, argv)
	int		argc;			/* Number of Arguments */
	char	**argv;			/* Pointer of Arguments */
{
	typedef	char	SRCNAME[32];
	SRCNAME	*src_ptr;					/* Pointer of Source Name			*/
	struct	header		obs;			/* OBS HEADDER						*/
	struct	head_obj	*obj_ptr;		/* Pointer of Objects Header		*/
	struct	head_obj	*first_obj_ptr;	/* First Pointer of Objects Header	*/
	struct	head_stn	*stn_ptr;		/* Pointer of Station Header		*/
	struct	head_stn	*first_stn_ptr;	/* First Pointer of Station Header	*/
	struct	fcal_data	*curr_fcal_ptr;	/* Current F-Cal Data Pointer		*/
	int		fcal_ptr[MAX_ANT];			/* Pointer of CLOCK data			*/

	/*-------- INDEX --------*/
	int		stn_index;					/* Index for Station				*/
	int		delay_index;				/* Index of delay data				*/
	int		*index_ptr;					/* Pointer of MJD Index				*/

	/*-------- IDENTIFIER --------*/
	int		shrd_obj_id;				/* Shared Memory ID for Source HD	*/
	int		shrd_stn_id;				/* Shared Memory ID for Station HD	*/
	int		obj_id;						/* OBJECT ID						*/
	int		recid;						/* Record ID in CFS					*/
	int		lunit;						/* Unit Number of CFS File			*/

	/*-------- TOTAL NUMBER --------*/
	int		ssnum;						/* Number of Sub-Stream				*/
	int		stn_num;					/* Total Number of Selected Stations*/
	int		delay_num;					/* Number of Delay Data				*/

	/*-------- MAX and MIN --------*/
	double	mjd_min, mjd_max;			/* Max and Min of MJD				*/
	double	rate_min, rate_max;			/* Max and Min of Rate Offset		*/
	double	delay_min, delay_max;		/* Max and Min of Delay Offset		*/
	double	acc_min, acc_max;			/* Max and Min of Acceleration		*/

	/*-------- GENERAL VARIAVLES --------*/
	int		ret;						/* Return Code from CFS Library		*/
	char	fname[128];					/* File Name of CFS Files			*/
	char	omode[2];					/* Access Mode of CFS Files			*/
	double	*mjd_ptr;					/* Pointer of MJD Data				*/
	double	*t_incr_ptr;				/* Pointer of Time Increment		*/
	double	*delay_ptr;					/* Pointer of Delay [microsec]		*/
	double	*dlerr_ptr;					/* Pointer of Delay Error [microsec]*/
	double	*rate_ptr;					/* Pointer of Rate [picosec/sec]	*/
	double	*rterr_ptr;					/* Pointer of Rate Error [psec/sec]	*/
	double	*acc_ptr;					/* Pointer of Acceleration			*/
	double	*accerr_ptr;				/* Pointer of Acceleration Error	*/
	double	loss;						/* Loss Factor						*/

	memset( fcal_ptr, 0, sizeof(fcal_ptr) );
/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 3 ){
		printf("USAGE : sort_delay [OBS_NAME] [STN_NAME1] [STN_NAME2] ...!!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  STN_NAME ------ STATION NAMEs (`all' is acceptable)\n");
		exit(0);
	}
	stn_num	= argc - STN_ID;

	cfs000_( &ret );			cfs_ret( 000, ret );
	cfs020_( &ret );			cfs_ret( 020, ret );
	cfs006_( argv[1], &ret, strlen( argv[1] ));	cfs_ret( 006, ret );

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

	/*-------- LOOP FOR STATIONS IN ARGMENT --------*/
	for(stn_index=0; stn_index<stn_num; stn_index++){
		stn_ptr	= first_stn_ptr;

		/*-------- FIND STATION --------*/
		while(stn_ptr != NULL){
			if( strstr(stn_ptr->stn_name, argv[STN_ID + stn_index]) != NULL){
				printf("STATION %-10s: ID = %2d\n",
					stn_ptr->stn_name, stn_ptr->stn_index );

				/*-------- READ F-CAL DATA --------*/
				delay_num = read_delay( stn_ptr->stn_index,		"all",
					&fcal_ptr[stn_index],	&mjd_min,	&mjd_max,
					&rate_min,	&rate_max,
					&delay_min,	&delay_max,
					&acc_min,	&acc_max);

				/*-------- ALLOCATE MEMORY AREA FOR F-CAL DATA --------*/
				mjd_ptr		= (double *)malloc( delay_num * sizeof(double) );
				t_incr_ptr	= (double *)malloc( delay_num * sizeof(double) );
				delay_ptr	= (double *)malloc( delay_num * sizeof(double) );
				dlerr_ptr	= (double *)malloc( delay_num * sizeof(double) );
				rate_ptr	= (double *)malloc( delay_num * sizeof(double) );
				rterr_ptr	= (double *)malloc( delay_num * sizeof(double) );
				acc_ptr		= (double *)malloc( delay_num * sizeof(double) );
				accerr_ptr	= (double *)malloc( delay_num * sizeof(double) );
				src_ptr		= (SRCNAME *)malloc( delay_num * sizeof(SRCNAME) );
				index_ptr	= (int *)malloc( delay_num * sizeof(int) );
				curr_fcal_ptr	= (struct fcal_data *)fcal_ptr[stn_index];

				/*-------- LOAD F-CAL DATA --------*/
				for( delay_index=0; delay_index<delay_num; delay_index++){
					index_ptr[delay_index]	= delay_index;
					strcpy( src_ptr[delay_index], curr_fcal_ptr->objnam);
					mjd_ptr[delay_index]	= curr_fcal_ptr->mjd;
					t_incr_ptr[delay_index]	= curr_fcal_ptr->time_incr;
					rate_ptr[delay_index]	= curr_fcal_ptr->rate;
					rterr_ptr[delay_index]	= curr_fcal_ptr->rate_err;
					delay_ptr[delay_index]	= curr_fcal_ptr->delay;
					dlerr_ptr[delay_index]	= curr_fcal_ptr->delay_err;
					acc_ptr[delay_index]	= curr_fcal_ptr->acc;
					accerr_ptr[delay_index]	= curr_fcal_ptr->acc_err;

					curr_fcal_ptr = curr_fcal_ptr->next_fcal_ptr;
				}

				/*-------- SORT F-CAL DATA --------*/
				d_index_sort( delay_num, mjd_ptr, index_ptr );

				/*-------- STORE SORTED F-CAL DATA INTO CFS --------*/
				curr_fcal_ptr	= (struct fcal_data *)fcal_ptr[stn_index];
				save_delay_sort( stn_ptr->stn_index,
					curr_fcal_ptr->refant, delay_num, index_ptr,
					src_ptr,	mjd_ptr,	t_incr_ptr,	delay_ptr,	dlerr_ptr,
					rate_ptr,	rterr_ptr,	acc_ptr,	accerr_ptr );

				/*-------- RELEASE MEMORY --------*/
				free( mjd_ptr );	free( t_incr_ptr );	free( delay_ptr );
				free( dlerr_ptr );	free( rate_ptr );	free( rterr_ptr );
				free( acc_ptr );	free( accerr_ptr );
			}

			stn_ptr = stn_ptr->next_stn_ptr;
		}
	}

	shmctl( shrd_obj_id, IPC_RMID, 0 );
	shmctl( shrd_stn_id, IPC_RMID, 0 );

	return(0);
}
