/*********************************************************
**	CALC_BL_DELAY.C	: Calculate Baseline-Based Delay	**
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
#define	MAX_SS	32	
#define	MAX_CH	1024
#define	RADDEG	57.29577951308232087721
#define OBS_NAME	1
#define SOURCE		2
#define DELAY_CAL	3
#define START		4
#define STOP		5
#define INTEG		6
#define BUNCH		7
#define DEVICE		8
#define STN_NAME1	9
#define STN_NAME2	10
#define	TOTAL_SS	11
#define SS_ARG		12

int	calc_bl_delay( obs_ptr, obj_ptr, stn_ptr, 

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
	int		*vismap_ptr;				/* Pointer of Visibility Map		*/
	int		fcal_addr[MAX_ANT];			/* Pointer of FCAL Data */

	/*-------- INDEX --------*/
	int		stn_index;					/* Index for Station				*/
	int		stn_index2;					/* Index for Station				*/
	int		bl_index;					/* Index for Baseline				*/
	int		ss_index;					/* Index of Sub-Stream				*/
	int		time_index;					/* Index for Time					*/
	int		freq_index;					/* Index for Frequency				*/

	/*-------- TOTAL NUMBER --------*/
	int		stn_num;					/* Total Number of Stations			*/
	int		blnum;						/* Total Number of Baseline			*/
	int		ssnum;						/* Number of Sub-Stream				*/
	int		ssnum_in_cfs;				/* Number of Sub-Stream in CODA		*/
	int		freq_num[MAX_SS];			/* Number of Frequency				*/
	int		bunch_num;					/* Number of Bunching				*/
	int		time_num;					/* Number of Time Data in CFS		*/
	int		antnum;						/* Total Number of Selected Station	*/
	int		integ_pp;					/* Coherent Integration PP			*/
	int		valid_pp;					/* Valid PP Number					*/
	int		bp_ssnum;					/* SS-Number in Band-Pass Filei		*/
	int		bp_freq_num[MAX_SS];		/* Frequency Channel Number			*/

	/*-------- IDENTIFIER --------*/
	int		refant_id;					/* REFANT ID in CFS					*/
	int		stnid_in_cfs[10];			/* Station ID Number in CODA		*/
	int		ssid_in_cfs[MAX_SS];		/* SS ID in CODA					*/
	int		position;					/* Start PP Position				*/
	int		obj_id;						/* OBJECT ID						*/

	float	bl_direction;				/* Baseline Direction  (-1 or 1)	*/
	int		ret;						/* Return Code from CFS Library 	*/
	int		lunit;						/* Unit Number of CFS File			*/
	char	fname[128];					/* File Name of CFS Files			*/
	char	omode[2];					/* Access Mode of CFS Files			*/
	char	obj_name[32];				/* Object Name						*/
	double	start_mjd;					/* Start Time [MJD]					*/
	double	stop_mjd;					/* Stop Time [MJD]					*/
	double	loss;						/* Quantize Efficiency				*/
	double	rf[MAX_SS];					/* RF Frequency [MHz]				*/
	double	freq_incr[MAX_SS];			/* Frequency Increment [MHz]		*/
	double	time_incr;					/* Time Increment [sec]				*/
	float	work[2*MAX_CH];				/* Work Area to Read Visibility		*/
	float	vismax;						/* Maximum Visibility Amp			*/
	float	*gff_result;				/* RESULT of GFF					*/
	float	*gff_err;					/* ERROR of GFF						*/
	int		start_time;					/* START TIME [DDDHHMMSS]			*/
	int		stop_time;					/* STOP TIME [DDDHHMMSS]			*/
	int		year, doy;					/* YEAR and Day of Year				*/
	char	bp_obj_name[32];			/* Object Name in Band-Pass File	*/
	double	bp_mjd;						/* MJD of the Band-Pass File		*/
	double	bp_integ_time;				/* Integ Time of the Band-Pass File	*/
	double	bp_rf[MAX_SS];				/* RF Freq. [MHz] in BP File		*/
	double	bp_freq_incr[MAX_SS];		/* Freq. Increment [MHz] in BP File */
	double	*bp_r_ptr[MAX_ANT][MAX_SS];	/* Pointer of Bandpass (REAL)		*/
	double	*bp_i_ptr[MAX_ANT][MAX_SS];	/* Pointer of Bandpass (IMAG)		*/
	double	bp_vis_max[MAX_ANT][MAX_SS];/* Maximum of Bandpass				*/
	double	delay[MAX_ANT];				/* Clock Offset at the Epoch		*/
	double	rate[MAX_ANT];				/* Clock Rate						*/
	double	loss;
	double	mjd_min,	mjd_max;		/* MAX and Min of MJD				*/
	double	rate_min,	rate_max;		/* MAX and Min of Rate				*/
	double	delay_min,	delay_max;		/* MAX and Min of Delay				*/
	double	acc_min,	acc_max;		/* MAX and Min of MJD				*/
	double	atm_prm[3*MAX_ANT-1];		/* Atmospheric Parameters			*/
	double	sin_el;						/* sin(EL)							*/
	double	dsecz;						/* dsec(z)/dt						*/
	char	pg_dev[256];				/* PGPLOT Device Name				*/

MAIN__( argc, argv )
	int		argc;		/* Number of Arguments */
	char	**argv;		/* Pointer of Arguments */
{

/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 9 ){
		printf("USAGE : coda_spec [OBS_NAME] [SOURCE] [DELAY CAL] [START] [STOP] [INTEG] [BUNCH] [DEVICE] [STN_NAME1] [STN_NAME2] [TOTAL_SS] [SS1] [SS2] ... !!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  SOURCE -------- Source Name\n");
		printf("  DELAY CAL ----- Source Name of Delays Calibrator\n");
		printf("  START --------- Start TIME [DDDHHMMSS] \n");
		printf("  STOP ---------- Stop TIME  [DDDHHMMSS] \n");
		printf("  INTEG --------- Integration TIME  [sec] \n");
		printf("  DEVICE -------- PGPLOT Device \n");
		printf("  STN_NAME ------ STATION NAMEs \n");
		printf("  TOTAL_SS ------ TOTAL STATION NUMBER \n");
		exit(0);
	}

	/*-------- START and STOP Time --------*/
	start_time	= atoi(argv[START]);
	stop_time	= atoi(argv[STOP]);
	sprintf(obj_name, "%s", argv[SOURCE]);

	ssnum	= atoi( argv[TOTAL_SS] );
	for( ss_index=0; ss_index<ssnum; ss_index++){
		ssid_in_cfs[ss_index] = atoi( argv[SS_ARG + ss_index] );
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
	cfs_ret( 103, ret );

	/*-------- READ OBSHEAD --------*/
	read_obshead( lunit, &obs, &obj_ptr, &stn_ptr, &shrd_obj_id, &shrd_stn_id );
	first_obj_ptr	= obj_ptr;
	first_stn_ptr	= stn_ptr;
	acorr_pair( &obs, stn_ptr, &ssnum_in_cfs, &loss);

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
	obj_ptr	= first_obj_ptr;
	objct_id( obj_ptr, obj_name, &obj_id );

	/*-------- LINK CORRELATION PAIR ID --------*/
	first_cor_ptr = (struct head_cor *)malloc( sizeof(struct head_cor));
	xcorr_pair( &obs, first_cor_ptr );

	ssnum_in_cfs	= first_cor_ptr->nss;
/*
------------------------ CALC DELAY and DELAY RATE vs ELEVATION
*/
	stn_num = 0;
	stn_index = 0;
	stn_ptr	= first_stn_ptr;
	while( stn_ptr != NULL){

		if(stn_ptr->acorr_index != -1){	/* AVOID DUMMY STATION */

			/*-------- READ STATION-BASED DELAY and RATE --------*/
			if( read_delay( stn_ptr->stn_index, argv[DELAY_CAL],
				&fcal_addr[stn_index],	&mjd_min,	&mjd_max,
				&rate_min,	&rate_max,	&delay_min,	&delay_max,
				&acc_min,	&acc_max) == -1){

				/*-------- IN CASE of REFANT --------*/
				refant_id	= stn_ptr->stn_index;
			} else {
				stn_index++;
			}
			stn_num++;
		}
		stn_ptr = stn_ptr->next_stn_ptr;
	}

	obj_ptr	= first_obj_ptr;
	atmdelay_solve( &obs, obj_ptr, refant_id, fcal_addr, first_stn_ptr,
		atm_prm );

	/*-------- LINK STATION ID 1 --------*/
	stn_ptr = first_stn_ptr;
	while(stn_ptr != NULL){
		/*-------- SEEK SPECIFIED STATION --------*/
		if( strstr(stn_ptr->stn_name, argv[STN_NAME1]) != NULL ){
			printf("STATION %-10s: ID = %2d\n",
				stn_ptr->stn_name, stn_ptr->stn_index );
			stnid_in_cfs[0] = stn_ptr->stn_index;
			break;
		} else {
			stn_ptr = stn_ptr->next_stn_ptr;
		}
	}

	stn_ptr = first_stn_ptr;
	while(stn_ptr != NULL){
		/*-------- SEEK SPECIFIED STATION 2 --------*/
		if( strstr(stn_ptr->stn_name, argv[STN_NAME2]) != NULL ){
			printf("STATION %-10s: ID = %2d\n",
				stn_ptr->stn_name, stn_ptr->stn_index );
			stnid_in_cfs[1] = stn_ptr->stn_index;
			break;
		} else {
			stn_ptr = stn_ptr->next_stn_ptr;
		}
	}

	antnum	= 2;
	blnum	= 1;
	vismap_ptr	= (int *)malloc( 2*blnum*ssnum*sizeof(int) );
/*
------------------------ LOAD BANDPASS DATA TO MEMORY
*/
	for(stn_index=0; stn_index<antnum; stn_index++){
		read_bp(stnid_in_cfs[stn_index], bp_obj_name,
			&bp_ssnum, bp_freq_num, &bp_mjd, &bp_integ_time, 
			bp_rf, bp_freq_incr,
			bp_r_ptr[stn_index], bp_i_ptr[stn_index], bp_vis_max[stn_index]);
	}
/*
------------------------ PREPARE MEMORY AREA FOR VISIBILITY
*/

	cor_ptr	= first_cor_ptr;

	if( strstr(argv[DEVICE], "/cps") != NULL){
		sprintf( pg_dev, "pgplot.cps/cps" );
		printf("SAVE PGPLOT TO %s\n", pg_dev );

	} else if( strstr(argv[DEVICE], "/vcps") != NULL){
		sprintf( pg_dev, "pgplot.cps/vcps" );
		printf("SAVE PGPLOT TO %s\n", pg_dev );

	} else if( strstr(argv[DEVICE], "/ps") != NULL){
		sprintf( pg_dev, "pgplot.ps/ps" );
		printf("SAVE PGPLOT TO %s\n", pg_dev );

	} else if( strstr(argv[DEVICE], "/vps") != NULL){
		sprintf( pg_dev, "pgplot.ps/vps" );
		printf("SAVE PGPLOT TO %s\n", pg_dev );

	} else {
		sprintf( pg_dev, "%s", argv[DEVICE] );
	}
	cpgbeg(1, pg_dev, 1, 1);

	while( cor_ptr != NULL ){

		/*-------- NORMAL BASELINE DIRECTION --------*/
		if( (cor_ptr->ant1 == stnid_in_cfs[0] )&&
			(cor_ptr->ant2 == stnid_in_cfs[1] )){
			bl_direction	= 1.0;
			position = -1;
			for( ss_index=0; ss_index<ssnum; ss_index++){
				integ_vis_data(argv);
			}

			cpg_vis( argv[OBS_NAME], argv[SOURCE], start_mjd, stop_mjd,
				time_incr*valid_pp, ssnum, ssid_in_cfs, freq_num, rf, freq_incr,
				vismap_ptr, &vismap_ptr[blnum*ssnum]);
			cpgend();
			break;
		}

		/*-------- INVERSE BASELINE DIRECTION --------*/
		if( (cor_ptr->ant2 == stnid_in_cfs[0] )&&
			(cor_ptr->ant1 == stnid_in_cfs[1] )){
			bl_direction	= -1.0;
			position = -1;
			for( ss_index=0; ss_index<ssnum; ss_index++){
				integ_vis_data(argv);
			}
			cpg_vis( argv[OBS_NAME], argv[SOURCE], start_mjd, stop_mjd,
				time_incr*valid_pp, ssnum, ssid_in_cfs, freq_num, rf, freq_incr,
				vismap_ptr, &vismap_ptr[blnum*ssnum]);
			cpgend();
			break;
		}

		cor_ptr	= cor_ptr->next_cor_ptr;
	}
/*
------------------------ RELEASE MEMORY AREA FOR BANDPASS
*/
	for(stn_index=0; stn_index<antnum; stn_index++){
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

integ_vis_data(argv)
	char	**argv;
{
	double	bldelay;					/* Baseline Delay at the Epoch	*/
	double	blrate;						/* Baseline Rate at the Epoch	*/
	int		time_num_cfs;				/* Time Number in CFS			*/
	int		ss_index2;					/* SS Index in CFS				*/

	ss_index2 = ssid_in_cfs[ss_index];

	obj_ptr	= first_obj_ptr;
	while(obj_ptr != NULL){
		if(obj_ptr->obj_index == obj_id){	break;}
		obj_ptr = obj_ptr->next_obj_ptr;
	}

	stn_ptr	= first_stn_ptr;
	while( stn_ptr != NULL ){
		if( stn_ptr->stn_index == stnid_in_cfs[stn_index] ){

			calc_el(&obs, obj_ptr, stn_ptr, start_mjd, &sin_el, &dsecz);
			#ifdef DEBUG
			printf(" MJD = %lf : EL = %lf, DSECZ = %e \n",
				start_mjd, RADDEG*asin(sin_el), dsecz );
			#endif


			rate[stn_index]	= atm_prm[stnid_in_cfs[stn_index]-1]*dsecz;
			delay[stn_index]= atm_prm[stnid_in_cfs[stn_index]-1]/sin_el;
			if(stnid_in_cfs[stn_index] != refant_id){
				rate[stn_index]	+= atm_prm[2*stn_num+stnid_in_cfs[stn_index]-3];
				delay[stn_index]+= atm_prm[stn_num+stnid_in_cfs[stn_index]-2];
			}
		}

		if( stn_ptr->stn_index == stnid_in_cfs[stn_index2] ){
			calc_el(&obs, obj_ptr, stn_ptr, start_mjd, &sin_el, &dsecz);
			#ifdef DEBUG
			printf(" MJD = %lf : EL = %lf, DSECZ = %e \n",
				start_mjd, RADDEG*asin(sin_el), dsecz );
			#endif

			rate[stn_index2]	= atm_prm[stnid_in_cfs[stn_index2]-1]*dsecz;
			delay[stn_index2]	= atm_prm[stnid_in_cfs[stn_index2]-1]/sin_el;
			if(stnid_in_cfs[stn_index2] != refant_id){
				rate[stn_index2]
					+= atm_prm[2*stn_num+stnid_in_cfs[stn_index2]-3];
				delay[stn_index2]
					+= atm_prm[stn_num+stnid_in_cfs[stn_index2]-2];
			}
		}

		stn_ptr = stn_ptr->next_stn_ptr;
	}
	stn_ptr	= first_stn_ptr;

	blrate = rate[stn_index2] - rate[stn_index];
	bldelay =delay[stn_index2]- delay[stn_index];

	return(0);
}
