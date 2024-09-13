/*********************************************************
**	READ_STATION_BP.C : Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"

#define	MAX_SS	32							/* Maximum Number of SS */

MAIN__( argc, argv )
	int		argc;			/* Number of Arguments */
	char	**argv;			/* Pointer of Arguments */
{
	struct	head_stn	*stn_ptr;			/* Pointer of Station Header */
	struct	head_obj	*obj_ptr;			/* Pointer of Object Header */
	char				obscode[32];		/* Observation Code */
	char				stn_name[32];		/* Station Name */
	int					stn_id;				/* Station ID */
	int					stn_num;			/* Total Number of Stations */
	char				obj_name[32];		/* Object Name */
	int					obj_num;			/* Total Number of Objects */
	int					ssnum;				/* Number of Sub-Stream */
	int					integ_pp;			/* Integrated PP Number */
	double				start_mjd;			/* Start Time [MJD] */
	double				stop_mjd;			/* Stop Time [MJD] */

	int			ss_index;			/* Index for Sub-Stream */
	double		rf[MAX_SS];			/* RF [MHz] for Each Sub-Stream */
	double		freq_incr[MAX_SS];	/* Freq. Increment for Each SS */
	int			freq_num[MAX_SS];	/* Freq. Channel Number in Each SS */
	int			time_num;			/* Total Number of PP in CFS */
	double		time_incr;			/* Increment for Time [sec] */
	double		*vis_r_ptr[MAX_SS];	/* Pointer of Vis. for BL and SS */
	double		*vis_i_ptr[MAX_SS];	/* Pointer of Vis. for BL and SS */
	double		vis_max[MAX_SS];	/* Maximum Visibility */
	int			ret;				/* CFS Library Return Code */
	char		pg_dev[256];

	if( argc < 4 ){
		printf("USAGE : station_bp [OBSNAME] [STN_ID] [PGPLOT DEVICE] !!\n");
		exit(-1);
	}

	#ifdef DEBUG
	printf("COMMAND : %s %s %s %s %s\n",
		argv[0], argv[1], argv[2], argv[3], argv[4] );
	#endif

	stn_id	= atoi( argv[3] );

	cfs000_( &ret );			cfs_ret( 000, ret );
	cfs020_( &ret );			cfs_ret( 020, ret );
	cfs006_( argv[1], &ret, strlen( argv[1] )); cfs_ret( 006, ret );

	/*-------- OPEN PGPLOT DEVICE --------*/
	cpgbeg(1, argv[4], 1, 1);

	/*-------- SAVE BANDPASS DATA --------*/
	read_bp( stn_id, obj_name,
			&ssnum, freq_num, &start_mjd, &time_incr,
			rf, freq_incr, vis_r_ptr, vis_i_ptr, vis_max);

	stop_mjd	= start_mjd + time_incr;
	sprintf(obscode, "%s", argv[1]);
	sprintf(stn_name, "%s", argv[2]);
	

	#ifdef DEBUG
	printf("OBS CODE = %s\n", obscode);
	printf("STN NAME = %s\n", stn_name);
	printf("OBJ NAME = %s\n", obj_name);
	printf("SS NUMBER= %d\n", ssnum);
	printf("START MJD= %lf\n", start_mjd);
	printf("INTEG TIM= %lf\n", time_incr);

	for(ss_index=0; ss_index<ssnum; ss_index++){
		printf("FREQ. NUM= %d\n", freq_num[ss_index]);
		printf("  RF     = %lf MHz\n", rf[ss_index]);
		printf("  INCR   = %lf MHz\n", freq_incr[ss_index]);
	}
	#endif

	/*-------- PLOT BANDPASS DATA --------*/
	cpg_bp( obscode, stn_name, obj_name, start_mjd, stop_mjd,
		time_incr, ssnum, freq_num, rf, freq_incr,
		vis_max, vis_r_ptr, vis_i_ptr);

	#ifdef HIDOI
	cpg_bp( obscode, stn_name, obj_name, start_mjd, stop_mjd,
		time_incr, 1, &freq_num[10], &rf[10], &freq_incr[10],
		&vis_max[10], &vis_r_ptr[10], &vis_i_ptr[10]);
	#endif

	cpgend();
	return(0);
}
