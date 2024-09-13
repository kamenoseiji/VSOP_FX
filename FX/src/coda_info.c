/*********************************************************
**	CODA_INFO.C	: Survey Visibility Information in 		**
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
#define CP_DIR	"/sys01/custom/bin"
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
	int		block_index;				/* Index for Data Block				*/

	/*-------- TOTAL NUMBER  --------*/ 
	int		stn_num;					/* Number of Station				*/
	int		blnum;						/* Total Number of Baseline			*/
	int		ssnum;						/* Number of Sub-Stream				*/
	int		ssnum_in_cfs;				/* Number of Sub-Stream in CFS		*/
	int		freq_num[MAX_SS];			/* Number of Frequency				*/
	int		time_num;					/* Number of Time Data				*/
	int		spec_num;					/* Number of Spectral Points		*/
	int		block_num;					/* Total Number of PP Block			*/

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
	char	flag[1024];					/* Flag Data						*/
	struct block_info	block[MAX_BLOCK];/* Block Information				*/
	char	block_fname[128];			/* File Name to Write Block Info	*/
	FILE	*block_file;				/* File to Record Block Info		*/

	/*-------- PGPLOT VARIABLE  --------*/ 
	int		err_code;					/* Error Code in PGPLOT				*/
	float	ywin_incr;					/* Increment of Y-Axis of Frame		*/
	float	xmin, xmax;					/* Min and Max of X-Axis			*/
	float	ymin, ymax;					/* Min and Max of Y-Axis			*/
	float	x_text, y_text;				/* Position of Text					*/
	double	x_incr, y_incr;				/* Increment of Axis				*/
	char	text[64];					/* Text in PGPLOT					*/

MAIN__(
	int		argc,		/* Number of Arguments		*/
	char	**argv,		/* Pointer of Arguments		*/
	char	**envp)		/* Pointer to Environments	*/
{
/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 3 ){
		printf("USAGE : coda_info [OBS_NAME] [DEVICE]!!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  DEVICE -------- PGPLOT Device [e.g. /xw]\n");
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
	time_num = SECDAY* (obs.stop_mjd - obs.start_mjd) / time_incr;
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
		memset(obj_name[obj_ptr->obj_index-1], 0, 32);
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

	printf(" SOURCE    START      STOP   STATION1  STATION2\n");
	printf("-------------------------------------------------\n");

	cpgbeg( 1, argv[DEVICE], 1, 1);
	/*-------- OPEN PGPLOT DEVICE --------*/
	if( strstr( argv[DEVICE], "cps") != NULL ){
		cpgscrn(0, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(12, "Black", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(13, "ivory", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(14, "SlateGray", &err_code); 	/* COLOR DEFINISHON */
		cpgscrn(15, "Green", &err_code);		/* COLOR DEFINISHON */
	} else if( strstr( argv[DEVICE], "ps") == NULL ){
		cpgscrn(0, "DarkSlateGray", &err_code); /* COLOR DEFINISHON */
		cpgscrn(12, "White", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(13, "SlateGray", &err_code);	/* COLOR DEFINISHON */
		cpgscrn(14, "DarkSlateGray", &err_code);/* COLOR DEFINISHON */
		cpgscrn(15, "Cyan", &err_code);			/* COLOR DEFINISHON */
	} else {
		cpgscrn(0, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(12, "Black", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(13, "White", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(14, "SlateGray", &err_code);	/* COLOR DEFINISHON */
		cpgscrn(15, "Black", &err_code);		/* COLOR DEFINISHON */
	}

	cpgeras();

	flgunit	= 4;
	xmin	= 0.0; xmax	= 2.0;
	ymin	= 0.0; ymax	= 2.0;
	cpgsvp( 0.10, 0.90, 0.10, 0.90);
	cpgswin(xmin, xmax, ymin, ymax);
	cpgptxt(1.0, 2.1, 0.0, 0.5, obs.obscode);
	cpgsch(0.5);
	cpgptxt(1.7, 2.05, 0.0, 0.0, (char *)getenv("CFS_DAT_HOME"));

	y_incr	= 5.0;
	ywin_incr = 0.85/(float)obs.cor_num;
	ss_index = 0;

	/*-------- TIME SPAN --------*/
	earliest_mjd= 1.0e36;
	latest_mjd	= -1.0e36;
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

		if( start_mjd > 0.0 ){		/* Avoid Invalid Data	*/
			if( start_mjd < earliest_mjd ){	earliest_mjd = start_mjd;} }

		if( stop_mjd > 0.0 ){		/* Avoid Invalid Data	*/
			if( stop_mjd  > latest_mjd ){	latest_mjd = stop_mjd;}	}

		cfs104_( &flgunit, &ret );  cfs_ret( 104, ret );
	}
	xmin	= (float)(SECDAY*( earliest_mjd - (int)earliest_mjd));
	xmax	= (float)(SECDAY*( latest_mjd  - (int)earliest_mjd));

	/*-------- OBJECT ID --------*/
	cor_ptr = first_cor_ptr;
	for( bl_index=0; bl_index<obs.cor_num; bl_index++){

		sprintf(block_fname, "%s/%s/.block.%d.info", getenv("CFS_DAT_HOME"), argv[OBS_NAME], bl_index+1);
		block_file = fopen(block_fname, "w");
		block_num = time_block_coda( bl_index+1, ss_index+1, spec_num,
						1.5*time_incr, block );
		fwrite(&block_num, sizeof(int), 1, block_file);
		fwrite(block, block_num* sizeof(struct block_info), 1, block_file);
		fclose(block_file);

#ifdef HIDOI
		for(block_index=0; block_index<block_num; block_index ++){
			fmjd2doy( block[block_index].st,
				&start_year, &start_doy, &start_hh, &start_mm, &start_ss);
			fmjd2doy( block[block_index].et,
				&stop_year, &stop_doy, &stop_hh, &stop_mm, &stop_ss);

			printf("BLOCK%3d : ST=%02d:%02d:%02d ET=%02d:%02d:%02d OBJ=%d\n",
					block_index, start_hh, start_mm, (int)start_ss,
					stop_hh, stop_mm, (int)stop_ss, block[block_index].obj );
		}
#endif

		stn_index  = cor_ptr->ant1 - 1;
		stn_index2 = cor_ptr->ant2 - 1;

		cpgbbuf();
		cpgsch(1.0);
		cpgsvp( 0.10, 0.92,
			0.067+ywin_incr*bl_index, 0.067+ywin_incr*(bl_index+1));
		cpgswin(xmin, xmax, ymin, ymax);
		cpgsci(13);  cpgrect(xmin, xmax, ymin, ymax);

		cpgsci(12);
		if( bl_index == 0 ){
			cpgtbox( "BSTNZH", 0.0, 0, "BCTS", y_incr, 5 );
		} else if( bl_index == obs.cor_num - 1) {
			cpgtbox( "BCSTZH", 0.0, 0, "BCTS", y_incr, 5 );
		} else {
			cpgtbox( "BSTZH", 0.0, 0, "BCTS", y_incr, 5 );
		}

		cpgsch(0.75);
		sprintf(text, "BL %d", bl_index+1);
		cpgtext( (xmin*1.15-xmax*0.15), (ymin*0.2+ymax*0.8), text);
		sprintf(text, "%s", stn_name[stn_index]);
		cpgtext( (xmin*1.12-xmax*0.12), (ymin*0.5+ymax*0.5), text);
		sprintf(text, "%s", stn_name[stn_index2]);
		cpgtext( (xmin*1.12-xmax*0.12), (ymin*0.8+ymax*0.2), text);

		/*-------- Background Color Box --------*/
		cpgsci(14); cpgrect(xmin, xmax, 0.1, 0.5);

		/*-------- Color Box for Each Block --------*/
		for(block_index=0; block_index<block_num; block_index ++){
			obj_id = block[block_index].obj;

			if( obj_id > 0 ){

				/*-------- Color Number by Object ID --------*/
				cpgsci( ( obj_id % 8) + 1);
				cpgrect((float)(SECDAY*(block[block_index].st-(int)(obs.start_mjd))),
						(float)(SECDAY*(block[block_index].et-(int)(obs.start_mjd))),
						0.1, 0.5);

				/*-------- Object Name by Text --------*/
				cpgsch(0.5);
				cpgsci(1);
				x_text =(float)(SECDAY*(block[block_index].st -(int)start_mjd));
				y_text = 0.5 +(float)((double)obj_id / (double)(obs.obj_num) );
				cpgtext(x_text, y_text, obj_name[obj_id - 1]);
				cpgsch(1.0);

				/*-------- Block Information --------*/
				fmjd2doy( block[block_index].st,
					&start_year, &start_doy, &start_hh, &start_mm, &start_ss);
				fmjd2doy( block[block_index].et,
					&stop_year, &stop_doy, &stop_hh, &stop_mm, &stop_ss);

				printf("%-10s %03d%02d%02d%02d %03d%02d%02d%02d %-10s %-10s\n",
					obj_name[obj_id - 1],
					start_doy, start_hh, start_mm, (int)start_ss,
					stop_doy,  stop_hh,  stop_mm,  (int)stop_ss,
					stn_name[stn_index], stn_name[stn_index2] );

			}
		}

		cpgebuf();
		cor_ptr = cor_ptr->next_cor_ptr;
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
