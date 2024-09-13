/*********************************************************
**	CODA_COPY.C	: COPY CODA File System					**
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
#include "obshead.inc"
#define CP_DIR	"/home/kameno/FX/bin"
#define	MAX_ANT	10
#define	MAX_BL	45
#define	MAX_CL	120
#define	MAX_SS	32	
#define	MAX_CH	1024
#define RADDEG	57.29577951308232087721	
#define	SECDAY	86400
#define WIND_X	32	
#define	OBS_NAME	1
#define	NEW_NAME	2

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

	/*-------- INDEX  --------*/ 
	int		stn_index;					/* Index for Station				*/
	int		stn_index2;					/* Index for Station				*/
	int		bl_index;					/* Index for Baseline				*/
	int		ss_index;					/* Index of Sub-Stream				*/
	int		time_index;					/* Index for Time					*/
	int		data_index;					/* Index for PP						*/

	/*-------- TOTAL NUMBER  --------*/ 
	int		stn_num;					/* Number of Station				*/
	int		blnum;						/* Total Number of Baseline			*/
	int		ssnum;						/* Number of Sub-Stream				*/
	int		ssnum_in_cfs;				/* Number of Sub-Stream in CFS		*/
	int		freq_num[MAX_SS];			/* Number of Frequency				*/
	int		time_num;					/* Number of Time Data				*/
	int		spec_num;					/* Number of Spectral Points		*/

	/*-------- IDENTIFIER  --------*/ 
	int		obj_id;						/* OBJECT ID						*/
	int		ss_id[MAX_SS];				/* SS ID in CFS						*/
	int		ss_suffix[MAX_SS];			/* SS ID Given From Command Line	*/
	int		stnid_in_cfs[10];			/* Station ID Number in CODA		*/
	int		refant_id;					/* Station ID of REF ANT			*/
	int		ret;						/* Return Code from CFS Library		*/
	int		write_ret;					/* Return Code from CFS Library		*/
	int		lunit;						/* Unit Number of CFS File 			*/
	int		flgunit;					/* Unit Number of CFS FLag File 	*/
	int		newunit;					/* Unit Number of CFS FLag File 	*/
	int		valid_pp;					/* Number of Valid PP				*/
	int		position;					/* PP Position in CFS				*/
	int		current_obj;				/* Current Object ID				*/
	int		prev_obj;					/* Previous Object ID				*/
	int		origin;						/* Origin in CFS records			*/
	int		skip;						/* Skip Number in CFS records		*/

	/*-------- GENERAL VARIABLE  --------*/ 
	int		start_year,	stop_year;		/* Start and Stop Year				*/
	int		start_doy,	stop_doy;		/* Start and Stop Day of Year		*/
	int		start_hh,	stop_hh;		/* Start and Stop Hour				*/
	int		start_mm,	stop_mm;		/* Start and Stop Minute			*/
	double	start_ss,	stop_ss;		/* Start and Stop Second			*/
	double	start_mjd,	stop_mjd;		/* Start and Stop Time [MJD] 		*/
	double	earliest_mjd;				/* Earliest MJD in CFS				*/
	double	latest_mjd;					/* Latest MJD in CFS				*/
	double	mjd_flag;					/* MJD in FLAG File					*/
	double	loss;						/* Quantize Efficiency 				*/
	double	time_incr;					/* Time Increment [sec] 			*/
	double	rf[MAX_SS];					/* RF Frequency [MHz] 				*/
	double	freq_incr[MAX_SS];			/* Frequency Increment [MHz] 		*/
	double	uvw[3];						/* U, V, W [m]						*/
	float	work[2*MAX_CH];				/* Work Area to Read Visibility 	*/
	char	fname[128];					/* File Name of CFS Files 			*/
	char	omode[2];					/* Access Mode of CFS Files 		*/
	char	stn_name[16][32];			/* Station Name						*/
	char	obj_name[512][32];			/* Object Name 						*/
	unsigned char	flag[1024];			/* Flag Data						*/
	char	fctgr[128];					/* File Category					*/
	char	ftype[4];					/* File Type						*/
	int		finfo[6];					/* File Info						*/

/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 3 ){
		printf("USAGE : coda_copy [OBS_NAME] [NEW_NAME]!!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. d96135]\n");
		printf("  NEW_NAME ------ New Observation Code [e.g. d96135b]\n");
		exit(0);
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
	if(cfs_ret( 103, ret ) != 0 ){
		close_shm(shrd_obj_id, shrd_stn_id);
		exit(0);
	}


	/*-------- READ OBSHEAD --------*/
	read_obshead( lunit, &obs, &obj_ptr, &stn_ptr, &shrd_obj_id, &shrd_stn_id );
	first_obj_ptr	= obj_ptr;
	first_stn_ptr	= stn_ptr;
	acorr_pair( &obs, &stn_ptr, &ssnum, &loss );
	fmjd2doy( obs.start_mjd,
				&start_year, &start_doy, &start_hh, &start_mm, &start_ss);
	fmjd2doy( obs.stop_mjd,
				&stop_year, &stop_doy, &stop_hh, &stop_mm, &stop_ss);

	/*-------- READ CORR-INFO and MAKE COR-HEAD --------*/
	cor_ptr	= (struct head_cor *)malloc( sizeof(struct head_cor) );
	xcorr_pair( &obs, cor_ptr );
	first_cor_ptr = cor_ptr;

	/*-------- READ SS-HEAD --------*/
	for(ss_index=0; ss_index<ssnum; ss_index++){
		read_sshead( 1, ss_index+1, &rf[ss_index], &freq_incr[ss_index],
					&freq_num[ss_index], &time_num, &time_incr); 

		write_sshead( 2, 1, ss_index+1, &rf[ss_index], &freq_incr[ss_index],
					&freq_num[ss_index], &time_num, &time_incr); 
	}



	printf("START TIME              : %lf [%04d %03d %02d:%02d:%04.1lf]\n",
		obs.start_mjd, start_year, start_doy, start_hh, start_mm, start_ss );
	printf("END   TIME              : %lf [%04d %03d %02d:%02d:%04.1lf]\n",
		obs.stop_mjd, stop_year, stop_doy, stop_hh, stop_mm, stop_ss);
	printf("TIME INCREMENT          : %lf [sec]\n", time_incr);
	printf("Total Number of PP      : %d \n", time_num );
	printf("Total Number of STATION : %d (include DUMMY)\n", obs.stn_num );
	printf("Total Number of STATION : %d (exclude DUMMY)\n", obs.real_stn_num );
	while( stn_ptr != NULL ){
		printf("  STATION %2d            : %s \n",
				stn_ptr->stn_index, stn_ptr->stn_name );
		strcpy( stn_name[stn_ptr->stn_index-1], stn_ptr->stn_name );
		stn_ptr = stn_ptr->next_stn_ptr;
	}
	printf("Total Number of CORR.   : %d (include ACORR)\n", obs.cor_num );
	while( cor_ptr != NULL ){
		printf("  CORR. %2d              : %d ( %s) - %d ( %s) \n",
			cor_ptr->cor_id,
			cor_ptr->ant1, stn_name[cor_ptr->ant1 - 1],
			cor_ptr->ant2, stn_name[cor_ptr->ant2 - 1]);
		cor_ptr = cor_ptr->next_cor_ptr;
	}


	printf("Total Number of SOURCE  : %d \n", obs.obj_num );
	while( obj_ptr != NULL ){
		printf("  SOURCE %2d             : %s \n",
				obj_ptr->obj_index, obj_ptr->obj_name );
		strcpy( obj_name[obj_ptr->obj_index-1], obj_ptr->obj_name );
		obj_ptr = obj_ptr->next_obj_ptr;
	}

	printf("Total Number of SS      : %d \n", ssnum );
	for(ss_index=0; ss_index<ssnum; ss_index++){
		printf("  SS[%2d]                : %.2lf -> %.2lf MHz / %d POINTS \n",
		ss_index, rf[ss_index] - 0.5*freq_incr[ss_index],
		rf[ss_index] + freq_incr[ss_index]*((double)freq_num[ss_index] - 0.5),
		freq_num[ss_index]);
	}

	flgunit	= 4;
	newunit	= 5;
	ss_index = 0;

	/*-------- OBJECT ID --------*/
	for( bl_index=0; bl_index<obs.cor_num; bl_index++){

		bl2ant( bl_index, &stn_index2, &stn_index );

		sprintf(omode, "r");
		sprintf(fname, "CORR.%d/SS.%d/FLAG.1\0", bl_index+1, ss_index+1 );
		cfs287_(fname, &ret, strlen(fname) );	cfs_ret( 287, ret );
		cfs103_(&flgunit, fname, omode, &ret, strlen(fname), strlen(omode));
		if(cfs_ret( 103, ret ) != 0 ){
			close_shm(shrd_obj_id, shrd_stn_id);
			exit(0);
		}

		sprintf(fname, "CORR.%d/SS.%d/FLAG.2\0", bl_index+1, ss_index+1 );
		sprintf(fctgr, "FLG_DATA");
		sprintf(ftype, "BB");
		sprintf(omode, "u");
		finfo[1]	= 10405;
		finfo[3]	= 64;
		finfo[4]	= 16;
		finfo[2]	= finfo[3] + finfo[4];
		finfo[0]	= finfo[1] * finfo[2] /1024;
		cfs101_(fname, fctgr, ftype, omode, finfo, &ret), cfs_ret(101, ret);
		cfs103_(&newunit, fname, omode, &ret), cfs_ret(103, ret);


		cfs225_(&flgunit,&start_mjd,&current_obj,uvw,flag,&spec_num,&ret);
		origin = 2; skip = -1;
		cfs400_(&flgunit, &origin, &skip, &ret);
		cfs225_(&flgunit,&stop_mjd,&current_obj,uvw,flag,&spec_num,&ret);
		cfs401_( &flgunit, &ret);

		time_num = (int)( SECDAY*(stop_mjd - start_mjd)/time_incr + 2 );
		data_index = 0;
		while(ret == 0 ){
			cfs225_( &flgunit,&mjd_flag,&current_obj,
					uvw, flag, &spec_num,&ret);

			cfs226_( &newunit,&mjd_flag,&current_obj,
					uvw, flag, &spec_num,&write_ret);

			time_index = (int)(SECDAY*(mjd_flag - start_mjd)/time_incr
						+ 0.5);

			data_index++;
		}
		valid_pp = data_index;
		printf(" READ %d PP Data.\n", valid_pp);

		cfs104_( &flgunit, &ret );  cfs_ret( 104, ret );
		cfs104_( &newunit, &ret );  cfs_ret( 104, ret );
	}

	close_shm(shrd_obj_id, shrd_stn_id);
	return(0);
}

close_shm( shrd_obj_id, shrd_stn_id )
	int		shrd_obj_id;				/* Shared Memory ID for Source HD	*/
	int		shrd_stn_id;				/* Shared Memory ID for Station HD	*/
{
	/*-------- CLOSE SHARED MEMORY --------*/
	shmctl( shrd_obj_id, IPC_RMID, 0 );
	shmctl( shrd_stn_id, IPC_RMID, 0 );
	return;
}
