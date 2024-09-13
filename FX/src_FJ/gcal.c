/*********************************************************
**	GCAL.C : Scaling for Reference Antenna Gain			**
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

#define GCALDATA    3
#define CORRDATA    4
#define CORRFLAG    5
#define	MAX_SS	32							/* Maximum Number of SS */
#define	SECPERDAY	86400

struct	head_stn	*stn_ptr;	/* Pointer of Station Header */
struct	head_obj	*obj_ptr;	/* Pointer of Object Header */
int			corunit;			/* Logical Unit of CORR File */
int			flgunit;			/* Logical Unit of FLAG File */
int			gclunit;			/* Logical Unit for GCAL DATA */
int			stn_id;				/* Station ID */
int			stn_num;			/* Total Number of Stations */
int			obj_num;			/* Total Number of Objects */
int			ssnum;				/* Number of Sub-Stream */
int			vanvnode_num;		/* Number of Spline Node	*/
int			integ_pp;			/* Integrated PP Number */
int			position;			/* Start PP Position	*/
double		start_mjd;			/* Start Time [MJD] */
double		stop_mjd;			/* Stop Time [MJD] */
double		*spline_node;		/* Pointer of Spline Nodes	*/
double		*spline_fact;		/* Pointer of Spline Factor	*/

char		bp_obj_name[32];	/* Object Name in BP FILE */
int			bp_ssnum;			/* Total SS number in BP FILE */
int			bp_freq_num[MAX_SS];/* Freq. CH Number in BP FILE */
double		bp_mjd;				/* MJD in BP FILE */
double		bp_integ_time;		/* INTEG Time [sec] in BP FILE */
double		bp_rf[MAX_SS];		/* RF Freq [MHz] in BP FILE */
double		bp_freq_incr[MAX_SS];/* Freq Increment [MHz] in BP */
double		*bp_r_ptr[MAX_SS];	/* Pointer of Bandpass (REAL) */
double		*bp_i_ptr[MAX_SS];	/* Pointer of Bandpass (IMAG) */
double		bp_vis_max[MAX_SS];	/* MAXIMUM in Bandpass data */

key_t		obj_key;			/* Keyword of Shared Memory */
key_t		stn_key;			/* Keyword of Shared Memory */
key_t		spec_key;			/* Keyword of Shared Memory */
int			shrd_obj_id;		/* Shared Memory ID */
int			shrd_stn_id;		/* Shared Memory ID */
int			shrd_spec_id;		/* Shared Memory ID */
int			ss_index;			/* Index for Sub-Stream */
int			ss_id;				/* Sub-Stream ID */
double		rf;					/* RF [MHz] for Each Sub-Stream */
double		freq_incr;			/* Freq. Increment for Each SS */
int			freq_num;			/* Freq. Channel Number in Each SS */
int			time_num;			/* Total Number of PP in CFS */
int			pp_incr;			/* Increment for PP */
double		current_mjd;		/* Current Time in MJD */
double		time_incr;			/* Increment for Time [sec] */
float		*work_ptr;			/* Pointer for Work Area */
double		*vis_r_ptr;			/* Pointer of Vis. for BL and SS */
double		*vis_i_ptr;			/* Pointer of Vis. for BL and SS */
double		vis_max;			/* Maximum Visibility */
double		amp_max;			/* Maximum Visibility */
double		*spec_ptr;			/* Poninter of Shared SPEC INFO */
int			ret;				/* CFS Library Return Code */
char		pg_dev[256];
float		x_pos[6], y_pos[6];
int			npoint;
int			first_flag;
int			recid;				/* Record ID in CFS */
int			nival;				/* Number of Integer in CFS */
int			ncval;				/* Byte Number of Character in CFS */
int			nrval;				/* Number of Double in CFS */
double		rvalue;				/* Double Value in CFS */
char		cvalue[64];			/* Char Data in CFS */
int			*int_ptr;			/* Pointer of Integer Data */
double		*double_ptr;		/* Pointer of Double Data */
double		offset_ratio;		/* Tsys Ratio	*/
double		sefd;				/* System Equivalent Flux Density [Jy]*/
double		sefd_err;			/* Error of SEFD [Jy]*/
int			year, doy, hour, min;
double		sec;


MAIN__(
	int		argc,			/* Number of Arguments			*/
	char	**argv,			/* Pointer to Arguments			*/
	char	**envp)			/* Pointer to Environments		*/
{
	char		ppblock_fname[128];	/* Block File Name				*/
	FILE		*ppblock_file;		/* Block File					*/
	struct block_info   ppblock[MAX_BLOCK];	/* Block Information	*/
	int			ppblock_num;			/* Total Number of Blocks		*/






	if( argc < 11 ){
		printf("USAGE : gcal [OBSNAME] [SRCNAME] [SRC_NUM] [STN_ID] [STN_NUM] [SS_INDEX] [SSNUM] [START_MJD] [STOP_MJD] [INTEG_PP]!!\n");
		exit(-1);
	}

	#ifdef DEBUG
	printf("COMMAND : %s %s %s %s %s %s %s %s %s %s\n",
		argv[0], argv[1], argv[2], argv[3],
		argv[4], argv[5], argv[6], argv[7],
		argv[8], argv[9], argv[10]);
	#endif

	obj_num	= atoi( argv[3] );
	stn_id	= atoi( argv[4] );
	stn_num	= atoi( argv[5] );
	ss_id= atoi( argv[6] );
	ssnum	= atoi( argv[7] );
	start_mjd	= atof( argv[8] );
	stop_mjd	= atof( argv[9] );

	cfs000_( &ret );			cfs_ret( 000, ret );
	cfs020_( &ret );			cfs_ret( 020, ret );
	cfs006_( argv[1], &ret, strlen( argv[1] )); cfs_ret( 006, ret );

	vanvleck2_init( &vanvnode_num, &spline_node, &spline_fact );

	/*-------- ACCESS TO SOURCE SHARED MEMORY --------*/
	obj_key = ftok(KEY_DIR, OBJ_KEY);
	if(( shrd_obj_id = shmget(obj_key, 
		obj_num*sizeof(struct head_obj), 0444)) < 0 ){
		printf("Can't Access to Shared Memory : %s !!\n", argv[0] );
		exit(1);
	}
	obj_ptr	= (struct head_obj *)shmat(shrd_obj_id, NULL, 0);

	/*-------- SEARCH FOR THE TARGET STATION --------*/
	while( obj_ptr != NULL ){

		if( strstr(obj_ptr->obj_name, argv[2]) != NULL ){
			printf("FIND SOURCE NAME %s : ID = %d\n",
				argv[2], obj_ptr->obj_index);
			break;
		}
		obj_ptr	= obj_ptr->next_obj_ptr;
	}

	/*-------- ACCESS TO STATION SHARED MEMORY --------*/
	stn_key = ftok(KEY_DIR, STN_KEY);
	if(( shrd_stn_id = shmget(stn_key, 
		stn_num*sizeof(struct head_stn), 0444)) < 0 ){
		printf("Can't Access to Shared Memory : %s !!\n", argv[0] );
		exit(1);
	}
	stn_ptr	= (struct head_stn *)shmat(shrd_stn_id, NULL, 0);

	/*-------- SEARCH FOR THE TARGET STATION --------*/
	while( stn_ptr != NULL ){
		if(stn_ptr->stn_index == stn_id){	break;	}
		stn_ptr	= stn_ptr->next_stn_ptr;
	}

	/*-------- ACCESS TO SPECTRUM SHARED MEMORY --------*/
	spec_key = ftok(KEY_DIR, SPC_KEY);
	if(( shrd_spec_id = shmget(spec_key, 7*sizeof(double), 0444)) < 0 ){
		printf("Can't Access to Shared Memory : %s !!\n", argv[0] );
		exit(1);
	}
	spec_ptr	= (double *)shmat(shrd_spec_id, NULL, 0);
	printf("INTEG FLUX = %lf Jy MHz [%lf - %lf MHz]\n",
		spec_ptr[6], spec_ptr[2], spec_ptr[3] ); 

	/*-------- OPEN PGPLOT DEVICE --------*/
	cpgbeg(1, "/xw", 1, 1);
/*
	sprintf( pg_dev, "pgplot.%d.ps/ps", stn_id);
	cpgbeg(1, pg_dev, 1, 1);
*/

/*
----------------------------------------- READ BANDPASS TABLE
*/
	/*-------- READ BANDPASS DATA --------*/
	read_bp( stn_id, bp_obj_name,
			&bp_ssnum, bp_freq_num, &bp_mjd, &bp_integ_time,
			bp_rf, bp_freq_incr, bp_r_ptr, bp_i_ptr, bp_vis_max);

#ifdef HIDOI
	vanvleck( bp_freq_num[ss_id], -1.0,
				bp_r_ptr[ss_id], bp_i_ptr[ss_id] );
#endif
	vanvleck2( bp_freq_num[ss_id], vanvnode_num, spline_node, spline_fact, -1.0,
				bp_r_ptr[ss_id], bp_i_ptr[ss_id] );
/*
----------------------------------------- INTEGRATE VISIBILITY
*/

	/*-------- READ SUB-STREAM HEAD --------*/
	read_sshead( stn_ptr->acorr_index,  ss_id+1,	&rf,
		&freq_incr,	&freq_num, &time_num,	&time_incr );

	/*-------- ALLOCATE MEMORY AREA FOR VISIBILITY DATA --------*/
	work_ptr= (float *)malloc(2*freq_num*sizeof(float));
	vis_r_ptr=(double *)malloc(freq_num*sizeof(double));
	vis_i_ptr=(double *)malloc(freq_num*sizeof(double));

	if( (work_ptr == NULL) || (vis_r_ptr == NULL) ){
		printf("Memory Error to Alloc Visibility !!\n"); 
		return(-1);
	}

	corunit = CORRDATA;
	flgunit = CORRFLAG;

	gcal_open( argv );

	first_flag = 1;
	pp_incr	= (int)( atof(argv[10])/time_incr + 0.5);
	if( pp_incr == 0){	pp_incr = 1;}

	position = -1;

/*----*/
	/*-------- BLOCK INFO --------*/
	sprintf(ppblock_fname, BLOCK_FMT, getenv("CFS_DAT_HOME"), argv[1], stn_ptr->acorr_index);
	printf("BLOCK FILE NAME = %s\n", ppblock_fname);
	ppblock_file = fopen(ppblock_fname, "r");
	if( ppblock_file == NULL){
		fclose(ppblock_file);
		printf("  Missing Block Record Information... Please Run codainfo in advance !\n");
		return(0);
	}
	fread(&ppblock_num, sizeof(int), 1, ppblock_file);
	fread(ppblock, ppblock_num* sizeof(struct block_info), 1, ppblock_file );
	fclose(ppblock_file);

	position = block_search(ppblock_num, ppblock, start_mjd);
/*----*/

	search_coda( CORRDATA, CORRFLAG, stn_ptr->acorr_index, obj_ptr->obj_index,
		ss_id+1, &position, freq_num, start_mjd, time_num, time_incr, work_ptr);

	/*-------- PREPARATION TO SAVE GAIN INTO CFS  --------*/
	current_mjd = 0.0;						/* Initialize MJD 			*/
	recid	= 1;							/* Record ID for GAIN DATA	*/
	nival	= 0;							/* Number of Integer Param.	*/
	nrval	= 2 + 4*ssnum;					/* Number of Real Parameter	*/
	ncval	= 32;							/* Byte Number of Char Param*/

	memset( cvalue, 0x20, 64 );				/* Fill 0x20 for FORTRAN	*/
	sprintf( &cvalue[0],  "%s", obj_ptr->obj_name);	/* Write Obj Name	*/
	double_ptr = (double *)malloc(nrval * sizeof(double));

	while( (current_mjd + time_incr*(double)(pp_incr/2)/SECPERDAY) < stop_mjd ){

#ifdef DEBUG
		fmjd2doy( current_mjd, &year, &doy, &hour, &min, &sec);
		printf(" CURRENT TIME	= %02d:%02d:%02d\n",
			hour, min, (int)sec );
#endif


		/*-------- CLEAR MEMORY FOR INTEGRATED VISIBILITY --------*/
		memset( vis_r_ptr, 0, freq_num*sizeof(double));
		memset( vis_i_ptr, 0, freq_num*sizeof(double));

		/*-------- READ ACORR DATA FROM CFS --------*/
		integ_pp = read_acorr( CORRDATA, CORRFLAG, obj_ptr->obj_index,
			freq_num, pp_incr, stop_mjd, &current_mjd,
			work_ptr, vis_r_ptr, vis_i_ptr, &vis_max );

		if( integ_pp == -2 ){ break;	}	/* In Case of EOF			*/

		if( integ_pp > pp_incr/2){			/* Integrated has been done */
			#ifdef DEBUG
			printf("MJD = %lf,  STOP = %lf, INTEGRATE %d PP Data. \n",
				current_mjd, stop_mjd, integ_pp);
			printf("VIS MAX = %lf\n", vis_max );
			#endif

			/*-------- CALIBRATE VISIBILITY DATA --------*/
#ifdef HIDOI
			vanvleck( freq_num, -1.0, vis_r_ptr, vis_i_ptr );
#endif
			vanvleck2( freq_num, vanvnode_num, spline_node, spline_fact, -1.0,
				vis_r_ptr, vis_i_ptr );
			cal_offline( freq_num, rf, freq_incr, spec_ptr,
					vis_r_ptr, vis_i_ptr, bp_r_ptr[ss_id], bp_i_ptr[ss_id],
					&offset_ratio );

			cal_exec( freq_num, vis_r_ptr, vis_i_ptr,
				bp_r_ptr[ss_id], bp_i_ptr[ss_id], &vis_max );

			/*-------- PLOT SPECTRUM DATA --------*/
			vis_max	= 0.0;
			cal_offset( 1,  spec_ptr[6], freq_num, rf, freq_incr,
				spec_ptr, vis_r_ptr, vis_i_ptr, &vis_max, &sefd );

			sefd_err= 1.0/(time_incr*integ_pp* (spec_ptr[1] - spec_ptr[0]))
					+ 1.0/(time_incr*integ_pp* (spec_ptr[3] - spec_ptr[2]))
					+ 1.0/(time_incr*integ_pp* (spec_ptr[5] - spec_ptr[4]));

			sefd /= offset_ratio;
			if( sefd > 0.0 ){
				sefd_err	= 1.0e-3 * sefd
							* sqrt(sefd_err * sefd / spec_ptr[6]);
			} else {
				sefd_err = 0.0;
			}

			*double_ptr	= current_mjd
						- (time_incr*(double)integ_pp/2.0 / SECPERDAY);
			double_ptr++;
			*double_ptr	= integ_pp*time_incr;	double_ptr++;
			for( ss_index=0; ss_index<ssnum; ss_index++){
				*double_ptr	= 0.0;	double_ptr++;
			}
			for( ss_index=0; ss_index<ssnum; ss_index++){
				*double_ptr	= sefd;	double_ptr++;
			}
			for( ss_index=0; ss_index<ssnum; ss_index++){
				*double_ptr	= 0.0;	double_ptr++;
			}
			for( ss_index=0; ss_index<ssnum; ss_index++){
				*double_ptr	= sefd_err;
				double_ptr++;
			}
			double_ptr -= nrval;

			cfs126_( &gclunit, &recid, &nival, int_ptr,
				&nrval, double_ptr, &ncval, cvalue, &ret, ncval );
			cfs_ret( 126, ret );

			fmjd2doy( double_ptr[0], &year, &doy, &hour, &min, &sec);

			printf("%s %02d:%02d:%02d  SEFD = %lf +/- = %lf\n",
				cvalue, hour, min, (int)sec, double_ptr[ssnum + 2],
				double_ptr[3*ssnum + 2]);

			if(first_flag == 1){
				first_flag = 0;
				amp_max = vis_max;
			}

			cpgbbuf();
			cpg_acorr( argv[1], stn_ptr->stn_name, argv[2],
				current_mjd, current_mjd + (double)integ_pp*time_incr/SECPERDAY,
				(double)integ_pp*time_incr,
				1, &freq_num, &rf, &freq_incr,
				&amp_max, &vis_r_ptr);
			cpgebuf();
		}
	}

	free(int_ptr);								/* Release Memory for INT */
	free(double_ptr);

	cfs104_( &corunit, &ret );  cfs_ret( 104, ret );
	cfs104_( &flgunit, &ret );  cfs_ret( 104, ret );
	cfs104_( &gclunit, &ret );  cfs_ret( 104, ret );

	cpgend();

	shmdt((char *)obj_ptr);
	shmdt((char *)stn_ptr);
	shmdt((char *)spec_ptr);
	return(0);
}


gcal_open( argv )
	char	**argv;
{
	char	fmode[2];				/* CFS Access Mode */
	char	fname[32];				/* CFS File Name */
/*
----------------------------------------- OPEN GCAL-DATA FILE IN CFS
*/
	gclunit = GCALDATA;
	sprintf( fmode, "w" );
	sprintf(fname, "CALIB/GCAL.%d", stn_ptr->stn_index );
	printf(" FILE NAME = %s\n", fname );
	cfs287_( fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_( &gclunit, fname, fmode, &ret, strlen(fname), strlen(fmode));
	cfs_ret( 103, ret );
/*
----------------------------------------- WRITE HEADDER IN GCAL-DATA
*/
	cfs401_( &gclunit, &ret );					/* Go to the File Top */
	cfs_ret(401, ret);
	recid	= 0;								/* Record ID for HEADDER is 0 */
	nival=2;	ncval=64;	nrval=0;			/* Number of parameters */
	int_ptr	= (int *)malloc(nival*sizeof(int));	/* Alloc Memory for INT */
	memset( cvalue, 0x20, ncval );				/* Fill 0x20 for FORTRAN file */
	sprintf( &cvalue[0],  "%s", argv[1]);		/* Write Obs Name */
	sprintf( &cvalue[32], "%s", stn_ptr->stn_name);	/* Write Station Name */
	int_ptr[0]	= stn_id;						/* Station ID */
	int_ptr[1]	= ssnum;						/* Total Number of SS */
	cfs126_( &gclunit, &recid, &nival, int_ptr,	/* Write Headder to CFS */
		&nrval, &rvalue, &ncval, cvalue,
		&ret, ncval );
	cfs_ret(126, ret);
	cfs402_( &gclunit, &ret );					/* Go to the File END */
	cfs_ret(402, ret);

	return;
}
