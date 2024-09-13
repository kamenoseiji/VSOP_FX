/*********************************************************
**	CODA_UV.C	: Survey Visibility Information in 		**
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
#define	MAX_ANT	10
#define	MAX_BL	45
#define	MAX_CL	120
#define	MAX_SS	32	
#define	MAX_CH	1024
#define RADDEG	57.29577951308232087721	
#define	SECDAY	86400
#define WIND_X	32	
#define	OBS_NAME	1
#define	DEVICE		2
#define	EARTH_RAD	6378.0e3
#define	DISPFACT	2.0

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
	int		lunit;						/* Unit Number of CFS File 			*/
	int		flgunit;					/* Unit Number of CFS FLag File 	*/
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
	int		curr_year;					/* Current Year						*/
	int		curr_doy;					/* Current Day of Year				*/
	int		curr_hh;					/* Current Hour						*/
	int		curr_mm;					/* Current Minute					*/
	double	curr_ss;					/* Current Second					*/

	/*-------- PGPLOT VARIABLE  --------*/ 
	int		err_code;					/* Error Code in PGPLOT				*/
	double	max_u, max_v;				/* Min and Max of X-Axis			*/
	float	ywin_incr;					/* Increment of Y-Axis of Frame		*/
	float	xmin, xmax;					/* Min and Max of X-Axis			*/
	float	ymin, ymax;					/* Min and Max of Y-Axis			*/
	float	x_text, y_text;				/* Position of Text					*/
	double	x_incr, y_incr;				/* Increment of Axis				*/
	float	*pgflg;						/* Flag Information					*/
	float	u, v;						/* u-component of Baseline [m] 		*/
	char	text[64];					/* Text in PGPLOT					*/

MAIN__( argc, argv )
	int		argc;		/* Number of Arguments */
	char	**argv;		/* Pointer of Arguments */
{
/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 3 ){
		printf("USAGE : coda_info [OBS_NAME] [DEVICE] [UV_MAX]!!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  DEVICE -------- PGPLOT Device [e.g. /xw]\n");
		printf("  UV-MAX -------- U-V PLAIN MAX\n");
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
	if(cfs_ret( 103, ret ) != 0 ){	close_shm();	exit(0);	}


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

	cpgbeg( 1, argv[DEVICE], 1, 1);
	/*-------- OPEN PGPLOT DEVICE --------*/
	if( strstr( argv[DEVICE], "cps") != NULL ){
		cpgscrn(0,  "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(14, "Black", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(15, "ivory", &err_code);			/* COLOR DEFINISHON */
	} else if( strstr( argv[DEVICE], "ps") == NULL ){
		cpgscrn(0,  "DarkSlateGray", &err_code); /* COLOR DEFINISHON */
		cpgscrn(14, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(15, "SlateGray", &err_code);		/* COLOR DEFINISHON */
	} else {
		cpgscrn(0,  "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(14, "Black", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(15, "LightGray", &err_code);		/* COLOR DEFINISHON */
	}

	cpgeras();

	flgunit	= 4;
	xmin	= 0.0; xmax	= 2.0;
	ymin	= 0.0; ymax	= 2.0;
	cpgsvp( 0.10, 0.90, 0.10, 0.90);
	cpgswin(xmin, xmax, ymin, ymax);
	cpgptxt(1.0, 2.1, 0.0, 0.5, obs.obscode);

	y_incr	= 5.0;
	ywin_incr = 0.85/(float)obs.cor_num;
	ss_index = 0;

	/*-------- TIME SPAN --------*/
	max_u = 0.0;	max_v = 0.0;
	for( bl_index=0; bl_index<obs.cor_num; bl_index++){

		bl2ant( bl_index, &stn_index2, &stn_index );

		sprintf(fname, "CORR.%d/SS.%d/FLAG.1\0", bl_index+1, ss_index+1 );
		cfs287_(fname, &ret, strlen(fname) );	cfs_ret( 287, ret );
		cfs103_(&flgunit, fname, omode, &ret, strlen(fname), strlen(omode));
		if(cfs_ret( 103, ret ) != 0 ){	close_shm();	exit(0);	}
		cfs225_(&flgunit,&start_mjd,&current_obj,uvw,flag,&spec_num,&ret);
		origin = 2; skip = -1;
		cfs400_(&flgunit, &origin, &skip, &ret);

		cfs225_(&flgunit,&stop_mjd,&current_obj,uvw,flag,&spec_num,&ret);
		cfs401_( &flgunit, &ret);

/*
		if( fabs((double)uvw[0]) > max_u ){	max_u = fabs((double)uvw[0]);}
		if( fabs((double)uvw[1]) > max_v ){	max_v = fabs((double)uvw[1]);}
*/

		cfs104_( &flgunit, &ret );  cfs_ret( 104, ret );
	}

	if(argc >= 4){ max_u = atof(argv[3]);}
	else{ max_u = 3.0*EARTH_RAD; }

	if( max_u > max_v ){ max_v = max_u;}
	printf("MAX_U = %lf   MAX_V = %lf\n", max_u, max_v);
	
	xmin	=  DISPFACT* (float)max_v;	xmax	= -DISPFACT* (float)max_v;
	ymin	= -DISPFACT* (float)max_v;	ymax	=  DISPFACT* (float)max_v;
	cpgsch(1.0);
	cpgsvp( 0.10, 0.92, 0.10, 0.92);
	cpgwnad(xmin, xmax, ymin, ymax);
	cpgsci(15);  cpgrect(xmin, xmax, ymin, ymax);
	cpgsci(14); cpgbox( "BCNTS", 0.0, 0, "BCNTS", 0.0, 0 );
	cpgsci(1);	cpgsfs(2);
	cpgcirc(0.0, 0.0, 2.0*EARTH_RAD);
	cpglab("U [m]", "V [m]", "");

	/*-------- OBJECT ID --------*/
	for( bl_index=0; bl_index<obs.cor_num; bl_index++){

		cpgbbuf();
		bl2ant( bl_index, &stn_index2, &stn_index );

		sprintf(fname, "CORR.%d/SS.%d/FLAG.1\0", bl_index+1, ss_index+1 );
		cfs287_(fname, &ret, strlen(fname) );	cfs_ret( 287, ret );
		cfs103_(&flgunit, fname, omode, &ret, strlen(fname), strlen(omode));
		if(cfs_ret( 103, ret ) != 0 ){	close_shm();	exit(0);	}
		cfs225_(&flgunit,&start_mjd,&current_obj,uvw,flag,&spec_num,&ret);
		origin = 2; skip = -1;
		cfs400_(&flgunit, &origin, &skip, &ret);
		cfs225_(&flgunit,&stop_mjd,&current_obj,uvw,flag,&spec_num,&ret);
		cfs401_( &flgunit, &ret);

		time_num = (int)( SECDAY*(stop_mjd - start_mjd)/time_incr + 2 );
		pgflg	= (float *)malloc( sizeof(float) * time_num );

		for(time_index=0; time_index<time_num; time_index++){
			pgflg[time_index] = 0.0;
		}

		data_index = 0;
/*
		if( (uvw[0]!=0.0) || (uvw[1]!=0.0) || (uvw[2]!=0.0) ){
*/
		while(ret == 0 ){

			cfs225_( &flgunit,&mjd_flag,&current_obj,
					uvw, flag, &spec_num,&ret);


/*
			if( (uvw[0]!=0.0) || (uvw[1]!=0.0) || (uvw[2]!=0.0) ){
*/

				time_index = (int)(SECDAY*(mjd_flag - start_mjd)/time_incr
						+ 0.5);
				pgflg[time_index] = (float)((!flag[0])*current_obj );
				u = (float)uvw[0];
				v = (float)uvw[1];

				cpgsci( current_obj );
				cpgpt(1, &u, &v, 1 );
				u *= -1;	v *= -1;
				cpgpt(1, &u, &v, 1 );

/*	*/
				fmjd2doy( mjd_flag,
				&curr_year, &curr_doy, &curr_hh, &curr_mm, &curr_ss);

				printf("BL[%d]: OBJ=%03d  %03d %02d:%02d:%06.2lf (u,v,w) = %15.8e  %15.8e  %15.8e %15.8e\n",
					bl_index+1,  current_obj, curr_doy, curr_hh, curr_mm, curr_ss, uvw[0], uvw[1], uvw[2], sqrt(uvw[0]*uvw[0] + uvw[1]*uvw[1] + uvw[2]*uvw[2]));
/* */

				data_index++;
/*
			}
*/
		}
		valid_pp = data_index;

		printf(" READ %d PP Data.\n", valid_pp);

		cfs104_( &flgunit, &ret );  cfs_ret( 104, ret );

		cpgebuf();
		free(pgflg);
	}
	cpgend();
	close_shm();
	return(0);
}

close_shm()
{
	/*-------- CLOSE SHARED MEMORY --------*/
	shmctl( shrd_obj_id, IPC_RMID, 0 );
	shmctl( shrd_stn_id, IPC_RMID, 0 );
	return;
}
