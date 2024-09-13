/*********************************************************
**	CODA_MRG.C	: Global Fringe Search using 		**
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
#include "merge.inc"
#define	MAX_STN		16
#define	MAX_BL		64
#define	MAX_CL		128
#define	MAX_SS		32
#define	MAX_CH		1024
#define	SECDAY		86400
#define	RADDEG		57.29577951308232087721
#define	OBS_NAME	1
#define	SOURCE		2
#define	START		3
#define	STOP		4
#define	INTEG		5
#define	SOLINT		6
#define	DEVICE		7
#define	MRGFILE		8
#define	TOTAL_STN	9
#define	TOTAL_SS	10
#define	STN_NAME	11
#define	MATCH11		(cor_ptr->ant1 == cfs_stnid[ant1]) 
#define	MATCH12		(cor_ptr->ant1 == cfs_stnid[ant2]) 
#define	MATCH21		(cor_ptr->ant2 == cfs_stnid[ant1]) 
#define	MATCH22		(cor_ptr->ant2 == cfs_stnid[ant2]) 
#define	BLMAP_FMT	"BL%d %-9s - %-9s  CFS_CORR= %d [%s]\n"
#define	REFANT		-1
#define STNSS		stn_index* ss_num + ss_index
#define UVFACT		ref_freq / 299.792458

double	vanvleck2();

MAIN__(
	int		argc,			/* Number of Arguments			*/
	char	**argv,			/* Pointer to Arguments			*/
	char	**envp)			/* Pointer to Environments		*/
{


	/*-------- CIT MERGE FORMAT --------*/
	FILE	*mrg_file_ptr;				/* Merge File Poiter				*/
	struct	mrg_header		header;		/* Header in Merge File				*/
	struct	mrg_source		source;		/* Source information 				*/
	struct	mrg_station2	stn_2;		/* Station Format 2 				*/
	struct	mrg_misc		misc;		/* Miscellaneous Information		*/
	struct	mrg_vis2		vis2;		/* Visibility Format 2				*/
	struct	mrg_close		cls;		/* Closure Phase					*/
	int		byte_len;					/* Byte Length of Record in MRG		*/
	int		bl_id;						/* Baseline ID in MRG				*/
	int		cl_id;						/* Closure in MRG					*/
	int		ut;							/* UT in MERGE File [1/60 sec]		*/
	char	history[80];				/* History Record					*/

	/*-------- STRUCT --------*/
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

	/*-------- POINTER --------*/
	int		fcal_addr[MAX_STN];			/* Pointer of FCAL LIST				*/
	int		gcal_addr[MAX_STN];			/* Pointer of GCAL LIST				*/
	int		pcal_addr[MAX_STN];			/* Pointer of PCAL LIST				*/
	int		time_ptr_ptr[MAX_STN];		/* Pointer of FCAL Time List		*/
	int		gtime_ptr_ptr[MAX_STN];		/* Pointer of GCAL Time List		*/
	int		ptime_ptr_ptr;				/* Pointer of PCAL Time List		*/
	int		delay_ptr_ptr[MAX_STN];		/* Pointer of FCAL Delay List		*/
	int		rate_ptr_ptr[MAX_STN];		/* Pointer of FCAL Rate List		*/
	int		acc_ptr_ptr[MAX_STN];		/* Pointer of FCAL Acc List			*/
	int		delaywgt_ptr_ptr[MAX_STN];	/* Pointer of FCAL Delay Err List	*/
	int		ratewgt_ptr_ptr[MAX_STN];	/* Pointer of FCAL Rate Err List	*/
	int		accwgt_ptr_ptr[MAX_STN];	/* Pointer of FCAL Acc Err List		*/
	int		gain_ptr_ptr[MAX_STN];		/* Pointer of GCAL GAIN List		*/
	int		gainwgt_ptr_ptr[MAX_STN];	/* Pointer of GCAL Weight List		*/
	int		pcr_ptr_ptr;				/* Pointer of PCAL Phase List		*/
	int		pci_ptr_ptr;				/* Pointer of PCAL Phase List		*/
	int		pcwgt_ptr_ptr;				/* Pointer of PCAL Weight List	*/
	int		delay_coeff[MAX_STN];		/* Pointer of Delay Spline Coeff	*/
	int		rate_coeff[MAX_STN];		/* Pointer of Rate Spline Coeff		*/
	int		acc_coeff[MAX_STN];			/* Pointer of Acc Spline Coeff		*/
	int		gain_coeff[MAX_STN];		/* Pointer of Gain Spline Coeff		*/
	int		pcr_coeff[MAX_STN*MAX_SS];	/* Pointer of PCAL Spline Coeff		*/
	int		pci_coeff[MAX_STN*MAX_SS];	/* Pointer of PCAL Spline Coeff		*/
	int		time_node[MAX_STN];			/* Pointer of Time Node 			*/
	int		time_rate_node[MAX_STN];	/* Pointer of Time Node				*/
	int		time_gain_node[MAX_STN];	/* Pointer of Time Node				*/
	int		time_pcr_node[MAX_STN*MAX_SS];/* Pointer of Time Node			*/
	int		time_pci_node[MAX_STN*MAX_SS];/* Pointer of Time Node			*/
	int		vis_amp[MAX_BL];			/* Pointer of VisAmp vs Time		*/
	int		vis_phs[MAX_BL];			/* Pointer of VisPhs vs Time		*/
	int		err_amp[MAX_BL];			/* Pointer of VisAmpErr vs Time		*/
	int		err_phs[MAX_BL];			/* Pointer of VisPhsErr vs Time		*/
	int		cl_phs[MAX_CL];				/* Pointer of VisPhsErr vs Time		*/
	int		cl_err[MAX_CL];				/* Pointer of VisPhsErr vs Time		*/
	int		uvw_ptr[MAX_BL];			/* Pointer of UVW Coord				*/

	/*-------- INDEX --------*/
	int		stn_index;					/* Station Index in Input List		*/
	int		bl_index;					/* Baseline Index from Input List	*/
	int		cl_index;					/* Closure Index from Input List	*/
	int		ss_index;					/* Sub-Stream Index					*/
	int		time_index;					/* Index for Time					*/
	int		node_index;					/* Index for Node Points			*/
	int		ant1, ant2, ant3;			/* Ad-Hoc Stn. Index for BL and CL	*/
	int		bl1, bl2, bl3;				/* Ad-Hoc BL Index for STN and CL	*/

	/*-------- IDENTIFIER --------*/
	int		ret;						/* Return Code from CFS Library		*/
	int		lunit;						/* Local Unit Number				*/
	int		obj_id;						/* Selected Object ID in CFS		*/
	int		cfs_stnid[MAX_STN];			/* Station ID in CFS				*/
	int		cfs_blid[MAX_BL];			/* Baseline ID in CFS				*/
	int		cfs_ssid[MAX_SS];			/* Baseline ID in CFS				*/
	int		bl_dir[MAX_BL];				/* Baseline Direction				*/
	int		position;					/* PP Position						*/
	int		ref_ss;						/* Reference SS						*/

	/*-------- TOTAL NUMBER --------*/
	int		stn_num;					/* Number of Stations Selected		*/
	int		bl_num;						/* Number of Baseline Selected		*/
	int		cl_num;						/* Number of Closure Selected		*/
	int		ss_num;						/* Number of Sub-Stream				*/
	int		cfs_ssnum;					/* Total Number of SS in CFS		*/
	int		fcal_num[MAX_STN];			/* Total Number of FCAL Data		*/
	int		gcal_num[MAX_STN];			/* Total Number of GCAL Data		*/
	int		pcal_num[MAX_STN];			/* Total Number of PCAL Data		*/
	int		node_delay_num[MAX_STN];	/* Total Number of Delay Node		*/
	int		node_rate_num[MAX_STN];		/* Total Number of Rate Node		*/
	int		node_gain_num[MAX_STN];		/* Total Number of Gain Node		*/
	int		node_pcr_num[MAX_STN*MAX_SS];/* Total Number of P-Cal Node		*/
	int		node_pci_num[MAX_STN*MAX_SS];/* Total Number of P-Cal Node		*/
	int		freq_num[MAX_SS];			/* Frequency Channel Number			*/
	int		time_num_cfs;				/* Time Number in CFS				*/
	int		time_num;					/* Time Data Points					*/

	/*-------- TIME VARIABLE --------*/
	double	start_mjd;					/* Start MJD						*/
	double	first_mjd;					/* First MJD in Time Segment		*/
	double	stop_mjd;					/* Stop MJD							*/
	double	curr_mjd;					/* Current MJD						*/
	double	integ;						/* Integration Time [sec]			*/
	int		year;						/* Year								*/
	int		doy;						/* Day of Year						*/
	int		hour;						/* Day of Year						*/
	int		min;						/* Day of Year						*/
	double	sec;						/* Day of Year						*/
	int		start_time;					/* Start Time [DDDHHMMSS]			*/
	int		stop_time;					/* Stop Time [DDDHHMMSS]			*/
	int		solint;						/* Delay Solution Interval [sec]	*/
	double	mjd_min, mjd_max;			/* Min and Max of MJD in Delay Data	*/
	double	time_incr;					/* Time Increment [sec]				*/
	double	tmp_delay;					/* Tempolary Delay Value			*/
	double	tmp_rate;					/* Tempolary Delay Value			*/
	double	delay[MAX_STN];				/* Station-Based Delay at the time	*/
	double	rate[MAX_STN];				/* Station-Based Delay at the time	*/
	double	gain[MAX_STN];				/* Station-Based Gain at the time	*/
	double	pcphs[MAX_STN*MAX_SS];		/* Station-Based PCAL at the time	*/
	double	pc_real;					/* P-Cal Real Value					*/
	double	pc_imag;					/* P-Cal Real Value					*/
	double	bldelay;					/* Baseline-based Delay [microsec]	*/
	double	blrate;						/* Baseline-based Rate [microsec/sec]*/
	double	blsefd;						/* Baseline-based SEFD [Jy]			*/
	double	blpcal;						/* Baseline-based PCAL [RAD]		*/
	float	*time_list;					/* Time List [Sec of Day]			*/

	/*-------- Bandpass Variable --------*/
	int		bp_ssnum;					/* SS Number of BP File				*/
	int		bp_freq_num[MAX_SS];		/* Freq. CH Number of Bp File		*/
	char	bp_obj_name[32];			/* BandPass Object					*/
	double	bp_integ_time;				/* Integ Time of the Band-Pass File */
	double	bp_mjd;						/* MJD of Bandpass Data				*/
	double	bp_rf[MAX_SS];				/* RF Freq. [MHz] in BP File        */
	double	bp_freq_incr[MAX_SS];		/* Freq. Increment [MHz] in BP File */
	int		bp_r_ptr[MAX_STN*MAX_SS];	/* Pointer of Bandpass (REAL)       */
	int		bp_i_ptr[MAX_STN*MAX_SS];	/* Pointer of Bandpass (IMAG)       */
	double	bp_vis_max[MAX_STN*MAX_SS];	/* Maximum of Bandpass              */
	double	bp_power[MAX_BL*MAX_SS];	/* Total Power of Bandpass            */
	int		ave_bp_r[MAX_BL*MAX_SS];	/* Baseline-Based Bandpass			*/
	int		ave_bp_i[MAX_BL*MAX_SS];	/* Baseline-Based Bandpass			*/

	/*-------- Visibility Variable --------*/
	double	loss;						/* Correlation Loss Factor			*/
	float	*ave_vis;					/* Averaged Visibility Array		*/
	float	*err_vis;					/* Error of Ave. Visibility Array	*/
	double	acc_min, acc_max;			/* Min and Max of Acceleration		*/
	double	rate_min, rate_max;			/* Min and Max of Rate				*/
	double	delay_min, delay_max;		/* Min and Max of Delay				*/
	double	gain_min, gain_max;			/* Min and Max of Gain				*/
	float	work[2*MAX_CH];				/* Work Area						*/
	double	clphs[MAX_CL];				/* Closure Phase					*/
	double	vis_ss_r, vis_ss_i;			/* Vis Sum for SS					*/
	double	vis_sigma;					/* Vis Square Sum					*/
	double	uvw[3];						/* U, V, W							*/
	float	*vis_ptr;					/* Tempolary Pointer				*/
	double	*double_ptr;				/* Pointer of Double				*/

	/*-------- General Variable --------*/
	double	rf[MAX_SS];					/* RF Frequency at SS Edge [MHz]	*/
	double	freq_incr[MAX_SS];			/* Frequency increment [MHz]		*/
	double	vw[4];						/* Work Area for SSL2				*/
	double	ref_freq;					/* Reference Frequency				*/
	int		isw;						/* Control Code in SSL2				*/
	int		vanvnode_num;				/* Number of Nodes					*/
	double	*spline_node;				/* Pointer of Nodes					*/
	double	*spline_fact;				/* Pointer of Spline Factors		*/

	/*-------- CHARACTER --------*/
	char	obj_name[16];				/* Object Name						*/
	char	fname[128];					/* File Name in CFS					*/
	char	omode[2];					/* CFS Access Mode					*/
	char	stn_list[MAX_STN][16];		/* Station Name List				*/

	/*-------- BLOCK INFO --------*/
	char		block_fname[128];	/* Block File Name				*/
	FILE		*block_file;		/* Block File					*/
	struct block_info   block[MAX_BLOCK];	/* Block Information	*/
	int			block_num;			/* Total Number of Blocks		*/
/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 9 ){
		printf("USAGE : coda_mrg [OBS_NAME] [SOURCE] [START] [STOP] [INTEG] [TOTAL STN] [TOTAL SS] [STN_NAME1] [STN_NAME2] ...  [SS1] [SS2] ...!!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  SOURCE -------- Source Name for Band-Pass Calib\n");
		printf("  START --------- Start TIME [DDDHHMMSS] \n");
		printf("  STOP ---------- Stop TIME  [DDDHHMMSS] \n");
		printf("  INTEG --------- Integration TIME  [sec] \n");
		printf("  TOTAL STN ----- TOTAL NUMBER OF STATION\n");
		printf("  TOTAL SS ------ TOTAL NUMBER OF SUB-STREAM\n");
		printf("  STN_NAME ------ STATION NAMEs \n");
		exit(0);
	}

	/*-------- START and STOP Time --------*/
	start_time	= atoi(argv[START]);
	stop_time	= atoi(argv[STOP]);
	sprintf(obj_name, "%s", argv[SOURCE]);
	stn_num		= atoi(argv[TOTAL_STN]);
	ss_num		= atoi(argv[TOTAL_SS]);
	integ		= atof(argv[INTEG]);
	mrg_file_ptr = fopen(argv[MRGFILE], "w");

	vanvleck2_init( &vanvnode_num, &spline_node, &spline_fact );
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
	acorr_pair( &obs, stn_ptr, &cfs_ssnum, &loss);

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
			start_mjd, obs.stop_mjd); exit(0);
	}
	if(stop_mjd < obs.start_mjd){
		printf("INTEG STOP [MJD=%lf] IS BEFORE OBS START TIME [MJD=%lf]!!\n",
			stop_mjd, obs.start_mjd); exit(0);
	}
	if(start_mjd < obs.start_mjd){	start_mjd = obs.start_mjd;	}
	if(stop_mjd  > obs.stop_mjd) {	stop_mjd  = obs.stop_mjd;	}

	/*-------- LINK SOURCE NAME to OBJECT ID --------*/
	objct_id( obj_ptr, obj_name, &obj_id );

	/*-------- LINK CORRELATION PAIR ID --------*/
	first_cor_ptr = (struct head_cor *)malloc( sizeof(struct head_cor));
	xcorr_pair( &obs, first_cor_ptr );

	/*-------- RELATION BETWEEN INPUT SS and CFS SS LIST --------*/
	for(ss_index=0; ss_index<ss_num; ss_index++ ){
		cfs_ssid[ss_index] = atoi(argv[argc - ss_num + ss_index]) + 1;
	}

	/*-------- RELATION BETWEEN INPUT STATION and CFS STATION LIST --------*/
	for(stn_index=0; stn_index<stn_num; stn_index++ ){
		stn_ptr = first_stn_ptr;			/* Go To Top of List		*/
		cfs_stnid[stn_index] = -1;			/* Invalid Flag at Init		*/
		while( stn_ptr != NULL ){
			if( strstr( stn_ptr->stn_name, argv[STN_NAME+stn_index]) != NULL){ 
				cfs_stnid[stn_index] = stn_ptr->stn_index;
				sprintf(stn_list[stn_index], "%s\0", stn_ptr->stn_name);
				break; }
			stn_ptr = stn_ptr->next_stn_ptr;
		}
	}

	/*-------- READ BANDPASS DATA --------*/
	for(stn_index=0; stn_index<stn_num; stn_index++ ){
		read_bp(cfs_stnid[stn_index], bp_obj_name,
			&bp_ssnum, bp_freq_num, &bp_mjd, &bp_integ_time,
			bp_rf, bp_freq_incr,
			&bp_r_ptr[stn_index* cfs_ssnum],
			&bp_i_ptr[stn_index* cfs_ssnum],
			&bp_vis_max[stn_index* cfs_ssnum]);

		for(ss_index=0; ss_index<ss_num; ss_index++ ){
			printf("BP FREQ NUM = %d\n", bp_freq_num[ss_index]);
		}
	}

	/*-------- RELATION BETWEEN INPUT STATION and CFS BL LIST --------*/
	bl_num = (stn_num* (stn_num - 1))/2;
	cl_num = (stn_num* (stn_num - 1)* (stn_num - 2))/6;
	for(bl_index=0; bl_index<bl_num; bl_index++ ){
		bl2ant(bl_index, &ant2, &ant1);
		cor_ptr = first_cor_ptr;			/* Go To Top of List		*/
		bl_dir[bl_index] = 0;				/* Invalid Flag at Init		*/
		while( cor_ptr != NULL ){
			if( MATCH11 && MATCH22 ){
				bl_dir[bl_index] = 1;
				cfs_blid[bl_index] = cor_ptr->cor_id;
				printf(BLMAP_FMT, bl_index, stn_list[ant1], stn_list[ant2],
						cor_ptr->cor_id, "NORMAL"); break; }
			if( MATCH12 && MATCH21 ){
				bl_dir[bl_index] = -1;
				cfs_blid[bl_index] = cor_ptr->cor_id;
				printf(BLMAP_FMT, bl_index, stn_list[ant1], stn_list[ant2],
						cor_ptr->cor_id, "INVERT"); break; }
			cor_ptr = cor_ptr->next_cor_ptr;
		}
	}

	/*-------- DELAY and RATE DATA of SPECIFIED STATIONS --------*/
	solint = atoi( argv[INTEG] );
	for(stn_index=0; stn_index<stn_num; stn_index++ ){
		fcal_num[stn_index] = read_delay(
			cfs_stnid[stn_index],	"all",	&fcal_addr[stn_index],
			&mjd_min, &mjd_max, &rate_min, &rate_max, &delay_min, &delay_max,
			&acc_min, &acc_max);

		if( fcal_num[stn_index] != REFANT ){
			restore_delay( fcal_addr[stn_index], fcal_num[stn_index],
				&time_ptr_ptr[stn_index],		&delay_ptr_ptr[stn_index],
				&rate_ptr_ptr[stn_index],		&acc_ptr_ptr[stn_index],
				&delaywgt_ptr_ptr[stn_index],	&ratewgt_ptr_ptr[stn_index],
				&accwgt_ptr_ptr[stn_index]);

			real_spline(time_ptr_ptr[stn_index],	delay_ptr_ptr[stn_index],
				delaywgt_ptr_ptr[stn_index],		fcal_num[stn_index],
				(double)solint,						&node_delay_num[stn_index],
				&delay_coeff[stn_index],			&time_node[stn_index]);

			real_spline(time_ptr_ptr[stn_index],	rate_ptr_ptr[stn_index],
				ratewgt_ptr_ptr[stn_index],			fcal_num[stn_index],
				(double)solint,						&node_rate_num[stn_index],
				&rate_coeff[stn_index],				&time_rate_node[stn_index]);
		}
	}

	/*-------- GAIN DATA of SPECIFIED STATIONS --------*/
	for(stn_index=0; stn_index<stn_num; stn_index++ ){
		gcal_num[stn_index] = read_gain(
			cfs_stnid[stn_index],	&gcal_addr[stn_index],
			&mjd_min, &mjd_max, &gain_min, &gain_max );

		if( gcal_num[stn_index] > 1 ){
			restore_gain( gcal_addr[stn_index], gcal_num[stn_index],
				&gtime_ptr_ptr[stn_index], &gain_ptr_ptr[stn_index],
				&gainwgt_ptr_ptr[stn_index]);

			real_spline(gtime_ptr_ptr[stn_index],	gain_ptr_ptr[stn_index],
				gainwgt_ptr_ptr[stn_index],			gcal_num[stn_index],
				(double)solint,						&node_gain_num[stn_index],
				&gain_coeff[stn_index],				&time_gain_node[stn_index]);
		}
	}

	/*-------- PCAL DATA of SPECIFIED STATIONS --------*/
	ref_ss = 0;
	for(stn_index=0; stn_index<stn_num; stn_index++ ){
		pcal_num[stn_index] = read_pcal(
			cfs_stnid[stn_index], &pcal_addr[stn_index]);
		printf("READ %d P-CAL Data !!\n", pcal_num[stn_index]);
		if( pcal_num[stn_index] > 1 ){

			for(ss_index=0; ss_index<ss_num; ss_index++){
				restore_pcal( pcal_addr[stn_index], pcal_num[stn_index],
					cfs_ssid[ss_index] - 1, ref_ss,
					&ptime_ptr_ptr,	&pcr_ptr_ptr,
					&pci_ptr_ptr,	&pcwgt_ptr_ptr );

				real_spline(ptime_ptr_ptr,
					pcr_ptr_ptr,		pcwgt_ptr_ptr,
					pcal_num[stn_index],	(double)solint,
					&node_pcr_num[STNSS],	&pcr_coeff[STNSS],
					&time_pcr_node[STNSS]);

				real_spline(ptime_ptr_ptr,
					pci_ptr_ptr,			pcwgt_ptr_ptr,
					pcal_num[stn_index],	(double)solint,
					&node_pci_num[STNSS],	&pci_coeff[STNSS],
					&time_pci_node[STNSS]);
			}
		}
	}

	/*-------- MEMORY AREA for AVERAGED VISIBILITY --------*/
	time_num = SECDAY*1.05*(stop_mjd - start_mjd)/integ + 1;
	ave_vis = (float *)malloc( 2* bl_num* ss_num* sizeof(float) );
	err_vis = (float *)malloc( 2* bl_num* ss_num* sizeof(float) );
	time_list= (float *)malloc(time_num* sizeof(float));
	for(bl_index=0; bl_index<bl_num; bl_index++ ){
		vis_amp[bl_index] = malloc(time_num* sizeof(float));
		vis_phs[bl_index] = malloc(time_num* sizeof(float));
		err_amp[bl_index] = malloc(time_num* sizeof(float));
		err_phs[bl_index] = malloc(time_num* sizeof(float));
		uvw_ptr[bl_index] = malloc(3* time_num* sizeof(double));
	}
	for(cl_index=0; cl_index<cl_num; cl_index++ ){
		cl_phs[cl_index] = malloc(time_num* sizeof(float));
		cl_err[cl_index] = malloc(time_num* sizeof(float));
	}

	/*-------- BASELINE-BASED BANDPASS --------*/
	for(bl_index=0; bl_index<bl_num; bl_index++ ){
		bl2ant(bl_index, &ant2, &ant1);
		for(ss_index=0; ss_index<ss_num; ss_index++){

			ave_bp_r[bl_index* ss_num +ss_index]
					= malloc(bp_freq_num[cfs_ssid[ss_index]-1]
					* sizeof(double) );
			ave_bp_i[bl_index* ss_num + ss_index]
					= malloc(bp_freq_num[cfs_ssid[ss_index]-1]
					* sizeof(double) );
			ave_bandpass( bp_freq_num[ss_index],
				bp_r_ptr[ant1* cfs_ssnum + cfs_ssid[ss_index] - 1],
				bp_i_ptr[ant1* cfs_ssnum + cfs_ssid[ss_index] - 1],
				bp_r_ptr[ant2* cfs_ssnum + cfs_ssid[ss_index] - 1],
				bp_i_ptr[ant2* cfs_ssnum + cfs_ssid[ss_index] - 1],
		        ave_bp_r[bl_index* ss_num + ss_index],
				ave_bp_i[bl_index* ss_num + ss_index] );

			bp_power[bl_index* ss_num + ss_index] = vanvleck2(
				bp_freq_num[cfs_ssid[ss_index] - 1],
				vanvnode_num, spline_node, spline_fact, -1.0,
				ave_bp_r[bl_index* ss_num + ss_index],
				ave_bp_i[bl_index* ss_num + ss_index]);
		}
	}

	/*-------- CALC REFERENCE FERQUENCY --------*/
	ref_freq = 0.0;
	for(ss_index=0; ss_index<ss_num; ss_index++){
		read_sshead( 1, cfs_ssid[ss_index],
			&rf[ss_index], &freq_incr[ss_index], &freq_num[ss_index],
			&time_num_cfs, &time_incr );

		ref_freq += (rf[ss_index]	+ (0.5*(double)freq_num[ss_index] - 0.5)
									* freq_incr[ss_index] );
    }
	ref_freq /= ss_num;

	/*-------- TIME LOOP --------*/
	curr_mjd = start_mjd;
	time_index = 0;
	while( (curr_mjd + 0.5*integ/SECDAY) < stop_mjd ){
		time_list[time_index] = (curr_mjd - (int)start_mjd)*SECDAY + integ/2.0;
		fmjd2doy( curr_mjd, &year, &doy, &hour, &min, &sec);
		printf("-------- PROCESS for UT = %03d %02d:%02d:%02d --------\r",
				doy, hour, min, (int)sec );

		/*-------- INTERPOLATE DELAY, RATE, and GAIN for the MOMENT --------*/
		for(stn_index=0; stn_index<stn_num; stn_index++ ){
			interp_real( time_list[time_index], node_delay_num[stn_index],
				time_node[stn_index], delay_coeff[stn_index],&delay[stn_index]);

			interp_real( time_list[time_index], node_delay_num[stn_index],
				time_node[stn_index], rate_coeff[stn_index], &rate[stn_index]);

			interp_real( time_list[time_index], node_gain_num[stn_index],
				time_gain_node[stn_index], gain_coeff[stn_index],
				&gain[stn_index]);

			for(ss_index=0; ss_index<ss_num; ss_index++){
				interp_real( time_list[time_index], node_pcr_num[STNSS],
					time_pcr_node[STNSS], pcr_coeff[STNSS], &pc_real);

				interp_real( time_list[time_index], node_pci_num[STNSS],
					time_pci_node[STNSS], pci_coeff[STNSS], &pc_imag);

				pcphs[STNSS] = atan2( pc_imag, pc_real );
			}
		}

		for(bl_index=0; bl_index<bl_num; bl_index++ ){
			bl2ant(bl_index, &ant2, &ant1);
			/*-------- CALC BASELINE-BASED DELAY and RATE --------*/
			bldelay = (double)bl_dir[bl_index]* (delay[ant2] - delay[ant1]);
			blrate  = (double)bl_dir[bl_index]* (rate[ant2]  - rate[ant1]);
			blsefd	= sqrt( gain[ant1]* gain[ant2] ); 


			/*-------- ACCESS TO THE BLOCK INFO --------*/
			sprintf(block_fname, BLOCK_FMT, getenv("CFS_DAT_HOME"), argv[1], cfs_blid[bl_index]);
			printf("BLOCK FILE NAME = %s\n", block_fname);
			block_file = fopen(block_fname, "r");
			if( block_file == NULL){
				fclose(block_file);
				printf("  Missing Block Record Information... Please Run codainfo in advance !\n");
				return(0);
			}
			fread(&block_num, sizeof(int), 1, block_file);
			fread(block, block_num* sizeof(struct block_info), 1, block_file );
			fclose(block_file);

			position = block_search(block_num, block, curr_mjd);
			vis_ss_r = 0.0;
			vis_ss_i = 0.0;
			vis_sigma = 0.0;
			for(ss_index=0; ss_index<ss_num; ss_index++){

				blpcal	= pcphs[ant2* ss_num + ss_index]
						- pcphs[ant1* ss_num + ss_index];

				/*-------- RELATION BETWEEN INPUT SS and CFS SS LIST --------*/
				read_sshead( cfs_blid[bl_index], cfs_ssid[ss_index],
					&rf[ss_index], &freq_incr[ss_index], &freq_num[ss_index],
					&time_num_cfs, &time_incr );

				/*-------- AVERAGE VISIBILITY --------*/
				average_vis(
					&ave_vis[(bl_index* ss_num + ss_index)* 2],
					&err_vis[(bl_index* ss_num + ss_index)* 2],
					ave_bp_r[bl_index* ss_num + ss_index],
					ave_bp_i[bl_index* ss_num + ss_index],
					bp_power[bl_index* ss_num + ss_index],
					cfs_blid[bl_index],	cfs_ssid[ss_index],
					bldelay,	blrate,	blsefd,
					rf[ss_index],	ref_freq,
					freq_incr[ss_index],	freq_num[ss_index],
					curr_mjd, integ, time_incr, time_num_cfs,
					&position, &first_mjd, uvw, work );

				vis_ss_r += (ave_vis[(bl_index* ss_num+ ss_index)* 2]
					* cos(ave_vis[(bl_index* ss_num+ ss_index)*2+1] - blpcal) );
				vis_ss_i += (ave_vis[(bl_index* ss_num+ ss_index)* 2]
					* sin(ave_vis[(bl_index* ss_num+ ss_index)*2+1] - blpcal) );
				vis_sigma +=
					( err_vis[(bl_index* ss_num+ ss_index)* 2]
					* err_vis[(bl_index* ss_num+ ss_index)* 2] );

				curr_mjd = first_mjd - time_incr/SECDAY;
			}

			double_ptr = (double *)uvw_ptr[bl_index];
			double_ptr[3* time_index]	= uvw[0];
			double_ptr[3* time_index+1]	= uvw[1];
			double_ptr[3* time_index+2]	= uvw[2];

			vis_ptr = (float *)vis_amp[bl_index];
			vis_ptr[time_index] = (float)sqrt(
				vis_ss_r* vis_ss_r + vis_ss_i* vis_ss_i );

			vis_ptr = (float *)vis_phs[bl_index];
			vis_ptr[time_index] = (float)atan2( vis_ss_i, vis_ss_r );

			vis_ptr = (float *)err_amp[bl_index];
			vis_ptr[time_index] = (float)sqrt( vis_sigma / ss_num );

			vis_ptr = (float *)err_phs[bl_index];
			vis_ptr[time_index] = (float)sqrt( vis_sigma / (ss_num* 
                (vis_ss_r* vis_ss_r + vis_ss_i* vis_ss_i ) ) );

		}
		curr_mjd += integ/SECDAY;
		time_index ++;
	}
	time_num = time_index;
	printf("\n");

	/*-------- RELATION BETWEEN INPUT STATION and CFS CL LIST --------*/
	for(time_index=0; time_index<time_num; time_index++ ){
		for(bl_index=0; bl_index<bl_num; bl_index++ ){
			vis_ptr = (float *)vis_phs[bl_index];
			ave_vis[bl_index* 2] = vis_ptr[time_index];

			vis_ptr = (float *)err_phs[bl_index];
			ave_vis[bl_index* 2 + 1] = vis_ptr[time_index];
		}

		for(cl_index=0; cl_index<cl_num; cl_index++ ){
			cl2ant(cl_index, &ant3, &ant2, &ant1);
			bl1 = ant2bl(ant1, ant2);
			bl2 = ant2bl(ant1, ant3);
			bl3 = ant2bl(ant2, ant3);

			vis_ptr = (float *)cl_phs[cl_index];
			vis_ptr[time_index] = (float)remainder(
				(ave_vis[bl1*2] - ave_vis[bl2*2] + ave_vis[bl3*2]), 2.0*M_PI );

			vis_ptr = (float *)cl_err[cl_index];
			vis_ptr[time_index] = sqrt(
					ave_vis[bl1* 2 + 1] * ave_vis[bl1* 2 + 1]
				  +	ave_vis[bl2* 2 + 1] * ave_vis[bl2* 2 + 1]
				  +	ave_vis[bl3* 2 + 1] * ave_vis[bl3* 2 + 1] );
		}
	}

/*
	cpgbeg(1, argv[DEVICE], 1, 1);
	cpgpage();
	cpg_amphi(argv[OBS_NAME], obj_name, stn_num, stn_list,
		time_num, start_mjd, stop_mjd, time_list, 1,
		vis_amp, vis_phs, err_amp, err_phs );

	cpgpage();
	cpg_clphs(argv[OBS_NAME], obj_name, stn_num, stn_list,
		time_num, start_mjd, stop_mjd, time_list, 1,
		cl_phs, cl_err );
------------------------ WRITE TO MERGE FILE
*/
	printf(" START TO CREATE MERGE FILE !!\n");

	header.format_version= 2;					/* Format Version		*/
	header.history_num	= 1;					/* Number of History	*/
	header.ant_num		= stn_num;				/* Number of Stations	*/
	header.bl_num		= bl_num;				/* Number of Baselines	*/
	header.cl_num		= cl_num;				/* Number of Closures	*/
	header.record_len	= 7*bl_num + 2*cl_num + 1;/* Number of Records	*/
	header.cal_flag		= 1;					/* Correlated F. D [Jy]	*/
	header.stokes_index	= 0;					/* Undefined			*/
	header.header_num	= header.history_num + stn_num + 3;
	if(header.cl_num > 0){
		header.header_num ++;					/* For CL-Index			*/
	}

	/*-------- WRITE FILE HEADER --------*/
	byte_len = sizeof(header);
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );
	fwrite( &header, 1, sizeof(header), mrg_file_ptr );
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );

	/*-------- WRITE BASELINE ID --------*/
	byte_len = bl_num * sizeof(int);
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );
	for(bl_index=0; bl_index<bl_num; bl_index++){
		bl2ant( bl_index, &ant1, &ant2);
		bl_id    = (ant2+1)*100 + (ant1+1);
		fwrite( &bl_id, 1, sizeof(int), mrg_file_ptr);
	}
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);

	/*-------- WRITE CLOSURE ID --------*/
	byte_len = sizeof(cl_id)*cl_num;
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );
	for(cl_index=0; cl_index<cl_num; cl_index++){
		cl2ant( cl_index, &ant1, &ant2, &ant3);
		cl_id    = (ant3+1)*10000 + (ant2+1)*100 + (ant1+1);
		fwrite( &cl_id, 1, sizeof(cl_id), mrg_file_ptr);
	}
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);

	/*-------- WRITE HISTORY --------*/
	byte_len	= 80;
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	sprintf(history, "CODAMRG ");
	fwrite( history, 1, byte_len, mrg_file_ptr);
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);

	/*-------- WRITE SOURCE INFORMATION --------*/
	memset(source.name, 0, sizeof(source.name));
	sprintf(source.name, "%-016s", obj_ptr->obj_name);
	source.ra_app		= obj_ptr->obj_pos[0]/RADDEG;	/* Right Ascension	*/
	source.dec_app		= obj_ptr->obj_pos[1]/RADDEG;	/* Declination 		*/
	source.ra_1950		= obj_ptr->obj_pos[0]/RADDEG;	/* Right Ascension	*/
	source.dec_1950		= obj_ptr->obj_pos[1]/RADDEG;	/* Declination 		*/
	if( obj_ptr->obj_pos[2] == 2000.0 ){
		j2000tob1950( source.ra_app, source.dec_app,
			&source.ra_1950, &source.dec_1950 );
	}
	source.total_flux	= 1.0;
	byte_len	= sizeof(source);
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	fwrite( &source, 1, sizeof(source), mrg_file_ptr);
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);

	/*-------- WRITE STATION INFORMATION --------*/
	for(stn_index=0; stn_index<stn_num; stn_index ++){
		stn_ptr	= first_stn_ptr;
		while(stn_ptr != NULL){
			if( strstr(stn_ptr->stn_name, stn_list[stn_index] ) !=  NULL){
				break;
			}
			stn_ptr = stn_ptr->next_stn_ptr;
		}
		memset(stn_2.name, 0, sizeof(stn_2.name));
		sprintf(stn_2.name, "%-8s\0", stn_ptr->stn_name);
		if( strstr(stn_2.name, "HALCA") != NULL){/*-------- SATELLITE*/
			stn_2.coord[0] = 17370983.0;	/* Maj Axis		*/
			stn_2.coord[1] = 0.59990;		/* Eccentricity	*/
			stn_2.coord[2] = 31.3579/RADDEG;	/* Incli	*/
			stn_2.coord[3] = 163.2029/RADDEG;	/* Omega	*/
			stn_2.coord[4] = 238.8438/RADDEG;	/* omega	*/
			stn_2.coord[5] = 73.0334/RADDEG;	/* Mean Anm	*/
			stn_2.type = 2;					/* Orbit Station*/
		} else {	/*-------- GROUND STATION --------*/
			stn_2.coord[0]	=  stn_ptr->stn_pos[0];
			stn_2.coord[1]	= -stn_ptr->stn_pos[1];		/* FOR CALTECH DEF	*/
			stn_2.coord[2]	=  stn_ptr->stn_pos[2];
			stn_2.type = 1;					/* Ground Station*/
		}
		byte_len= sizeof(stn_2);
		fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
		fwrite( &stn_2, 1, sizeof(stn_2), mrg_file_ptr );
		fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );
		stn_ptr = stn_ptr->next_stn_ptr;
	}

	/*-------- WRITE MISC INFORMATION --------*/
	misc.year		= year;
	misc.ut			= 60.0*SECDAY*( first_mjd - (int)first_mjd + doy - 1 );
	mjd2gmst( first_mjd, 0.525508, &misc.gst );
	misc.rf			= 1.0e6 * ref_freq;
	misc.coh_integ	= integ;
	misc.inc_integ	= 0.0;
	misc.bw			= freq_incr[0]* freq_num[0]* ss_num;
	byte_len= sizeof(misc);
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	fwrite( &misc, 1, sizeof(misc), mrg_file_ptr );
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );

	/*-------- TIME LOOP --------*/
	for( time_index=0; time_index<time_num; time_index++){
		ut = (int)( 60.0* (time_list[time_index] + (doy - 1.0)* SECDAY) ); 

		/*-------- WRITE VISIBILITY TO MERGE FILE --------*/
		byte_len= sizeof(ut) + bl_num*sizeof(vis2) + cl_num*sizeof(cls);
		fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );
		fwrite( &ut, 1, sizeof(ut), mrg_file_ptr );
		for( bl_index=0; bl_index<bl_num; bl_index++){

			vis_ptr = (float *)vis_amp[bl_index];
			vis2.amp = vis_ptr[time_index];

			vis_ptr = (float *)err_amp[bl_index];
			vis2.amp_err= vis_ptr[time_index];

			vis_ptr = (float *)vis_phs[bl_index];
			vis2.phs	= (float)(RADDEG* vis_ptr[time_index]);

			vis_ptr = (float *)err_phs[bl_index];
			vis2.phs_err= (float)(RADDEG* vis_ptr[time_index]);

			double_ptr = (double *)uvw_ptr[bl_index];
			vis2.u= (float)(UVFACT * double_ptr[time_index* 3]);
			vis2.v= (float)(UVFACT * double_ptr[time_index* 3 + 1]);
			vis2.w= (float)(UVFACT * double_ptr[time_index* 3 + 2]);

			fwrite( &vis2, 1, sizeof(vis2), mrg_file_ptr );
		}

		for( cl_index=0; cl_index<cl_num; cl_index++){

			vis_ptr = (float *)cl_phs[cl_index];
			cls.phs		= (float)(RADDEG* vis_ptr[time_index]);

			vis_ptr = (float *)cl_err[cl_index];
			cls.phs_err	= (float)(RADDEG* vis_ptr[time_index]);

			fwrite( &cls, 1, sizeof(cls), mrg_file_ptr );
		}
		fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );

	}	/* END OF TIME LOOP */


	/*-------- WRITE END-OF-FILE TO MERGE FILE --------*/
	ut		= -1.0;
	byte_len= sizeof(ut) + bl_num*sizeof(vis2) + cl_num*sizeof(cls);
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );
	fwrite( &ut, 1, sizeof(ut), mrg_file_ptr );

	for( bl_index=0; bl_index<bl_num; bl_index++){
		fwrite( &vis2, 1, sizeof(vis2), mrg_file_ptr );
	}
	for( cl_index=0; cl_index<cl_num; cl_index++){
		fwrite( &cls, 1, sizeof(cls), mrg_file_ptr );
	}
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );


	/*-------- CLOSE SHARED MEMORY --------*/
	fclose(mrg_file_ptr);
	shmctl( shrd_obj_id, IPC_RMID, 0 );
	shmctl( shrd_stn_id, IPC_RMID, 0 );
	free(ave_vis);
	free(err_vis);
	for(bl_index=0; bl_index<bl_num; bl_index++){
		for(ss_index=0; ss_index<ss_num; ss_index++){
			free(ave_bp_r[bl_index* ss_num + ss_index]);
			free(ave_bp_i[bl_index* ss_num + ss_index]);
		}
	}

	cpgend();
	return(0);
}
