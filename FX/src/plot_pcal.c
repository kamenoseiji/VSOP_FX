/*********************************************
**	PLOT_PCAL.C	: Plot P-CAL Phase  in 		**
**					CODA File System		**
**											**
**	AUTHOR	: KAMENO Seiji					**
**	CREATED	: 1996/6/27						**
**********************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"
#define CP_DIR	"/sys01/custom/bin"
#define	MAX_OBJ	64
#define	MAX_ANT	16
#define	MAX_BL	120
#define	MAX_CL	560
#define	MAX_SS	32	
#define	MAX_CH	1024
#define RADDEG	57.29577951308232087721	
#define	SECDAY	86400
#define WIND_X	32	
#define	OBS_NAME	1
#define	STN_NAME	2
#define	DEVICE		3
#define	REF_SS		4
#define	SOLINT		5

	/*-------- STRUCT for HEADDER in CFS --------*/ 
	struct	header		obs;			/* OBS HEADDER						*/
	struct	head_obj	*obj_ptr;		/* Pointer of Objects Header		*/
	struct	head_obj	*first_obj_ptr;	/* First Pointer of Objects Header	*/
	struct	head_stn	*stn_ptr;		/* Pointer of Station Header		*/
	struct	head_stn	*first_stn_ptr;	/* First Pointer of Station Header	*/
	struct	head_cor	*cor_ptr;		/* Pointer of CORR Header			*/
	struct	head_cor	*first_cor_ptr;	/* First Pointer of CORR Header		*/
	struct	pcal_data	*pcal_ptr;		/* Pointer of P-Cal data			*/
	int		first_pcal_ptr;				/* First Pointer of P-Cal data		*/
	int		real_coeff[MAX_SS];			/* Pointer of Spline Coefficient	*/
	int		imag_coeff[MAX_SS];			/* Pointer of Spline Coefficient	*/

	/*-------- SHARED MEMORY --------*/ 
	int		shrd_obj_id;				/* Shared Memory ID for Source HD	*/
	int		shrd_stn_id;				/* Shared Memory ID for Station HD	*/
	int		stn_id;						/* Target Station ID				*/

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
	int		node_num;					/* Number of Node in Spline			*/

	/*-------- IDENTIFIER  --------*/ 
	int		obj_id;						/* OBJECT ID						*/
	int		ss_id[MAX_SS];				/* SS ID in CFS						*/
	int		ss_suffix[MAX_SS];			/* SS ID Given From Command Line	*/
	int		stnid_in_cfs[MAX_ANT];		/* Station ID Number in CODA		*/
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
	int		ref_ss;						/* Reference SS ID					*/

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
	double	pcphs_ptr[32];				/* P-Cal Phase for Sub-Stream		*/
	double	pcerr_ptr[32];				/* P-Cal Phase Error for Sub-Stream	*/
	double	*time_ptr;					/* Pointer of Time Data				*/
	double	*pcal_phs;					/* Pointer of P-CAL Phase			*/
	double	*weight_ptr;				/* Pointer of P-CAL Phase			*/
	double	time_node[256];				/* Time Node						*/
	float	work[2*MAX_CH];				/* Work Area to Read Visibility 	*/
	char	fname[128];					/* File Name of CFS Files 			*/
	char	omode[2];					/* Access Mode of CFS Files 		*/
	char	stn_name[MAX_ANT][32];		/* Station Name						*/
	char	obj_name[MAX_OBJ][32];		/* Object Name 						*/
	unsigned char	flag[1024];			/* Flag Data						*/

	/*-------- PGPLOT VARIABLE  --------*/ 
	int		err_code;					/* Error Code in PGPLOT				*/
	float	ywin_incr;					/* Increment of Y-Axis of Frame		*/
	float	xmin, xmax;					/* Min and Max of X-Axis			*/
	float	ymin, ymax;					/* Min and Max of Y-Axis			*/
	float	x_data, y_data;				/* X- and Y-Position for Data		*/
	float	y_top, y_bottom;			/* Top and Bottom End of Error		*/
	float	x_text, y_text;				/* Position of Text					*/
	double	x_incr, y_incr;				/* Increment of Axis				*/
	double	ref_phs;					/* Reference Phase					*/
	float	*pgflg;						/* Flag Information					*/
	float	tr[6];						/* Transpose Matrix					*/
	char	text[64];					/* Text in PGPLOT					*/
	int		spline_num;					/* Number of Spline Data			*/
	float	*spline_time;				/* Spline Time Data					*/
	float	plot_phs;					/* Spline Phase Data				*/
	double	spline_phs;					/* Spline Phase Data				*/

MAIN__( argc, argv )
	int		argc;		/* Number of Arguments */
	char	**argv;		/* Pointer of Arguments */
{
/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 5 ){
		printf("USAGE : plot_pcal [OBS_NAME] [STN_NAME] [DEVICE] [REF_SS]!!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. d96135]\n");
		printf("  STN_NAME ------ Staition Name [e.g. NOBEYA45]\n");
		printf("  DEVICE -------- PGPLOT Device [e.g. /xw]\n");
		printf("  REF_SS -------- Reference Sub-Stream \n");
		printf("  SOLINT -------- Solution Interval [sec] \n");
		exit(0);
	}
	ref_ss	= atoi(argv[REF_SS]);
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

	/*-------- SEARCH in Station List --------*/
	while( stn_ptr != NULL ){
		printf("  STATION %2d            : %s \n",
				stn_ptr->stn_index, stn_ptr->stn_name );
		strcpy( stn_name[stn_ptr->stn_index - 1], stn_ptr->stn_name);
		if( strstr(stn_ptr->stn_name, argv[STN_NAME]) != NULL){
			stn_id	= stn_ptr->stn_index;
			break;
		}
		stn_ptr = stn_ptr->next_stn_ptr;
	}

	/*-------- READ P-Cal Table and Make Linked List --------*/
	time_num = read_pcal( stn_id, &first_pcal_ptr );
	printf(" Total %d P-Cal Data.\n", time_num );
	pcal_ptr	= (struct pcal_data *)first_pcal_ptr;
	pcal_phs	= (double *)malloc( time_num * sizeof(double) );
	weight_ptr	= (double *)malloc( time_num * sizeof(double) );
	time_ptr	= (double *)malloc( time_num * sizeof(double) );


	/*-------- Start and Stop Time of P-Cal Data --------*/
	start_mjd	= 999999.0;
	stop_mjd	=-999999.0;
	while( pcal_ptr != NULL ){
		*time_ptr	= (double)( SECDAY * (pcal_ptr->mjd - (int)obs.start_mjd));
		if( pcal_ptr->mjd < start_mjd ){ start_mjd	= pcal_ptr->mjd; }
		if( pcal_ptr->mjd > stop_mjd ){ stop_mjd	= pcal_ptr->mjd; }
		pcal_ptr = pcal_ptr->next_pcal_ptr;
		time_ptr ++;
	}
	time_ptr -= time_num;

	/*-------- OPEN PGPLOT DEVICE --------*/
	cpgbeg( 1, argv[DEVICE], 1, 1);
	if( strstr( argv[DEVICE], "cps") != NULL ){
		cpgscrn(0, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(1, "Black", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(2, "ivory", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(3, "Blue", &err_code); 			/* COLOR DEFINISHON */
		cpgscrn(4, "Green", &err_code);			/* COLOR DEFINISHON */
	} else if( strstr( argv[DEVICE], "ps") == NULL ){
		cpgscrn(0, "DarkSlateGray", &err_code); /* COLOR DEFINISHON */
		cpgscrn(1, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(2, "SlateGray", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(3, "Yellow", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(4, "Cyan", &err_code);			/* COLOR DEFINISHON */
	} else {
		cpgscrn(0, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(1, "Black", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(2, "Gray", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(3, "Black", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(4, "Black", &err_code);			/* COLOR DEFINISHON */
	}
	cpgeras();

	/*-------- Definesion of Window Frame --------*/
	xmin	= (float)(SECDAY*(start_mjd - (int)obs.start_mjd));
	xmax	= (float)(SECDAY*(stop_mjd  - (int)obs.start_mjd));
	ymin	= -M_PI;
	ymax	=  M_PI;
	ywin_incr	= 0.85/(float)ssnum;
	spline_num	= (int)(0.1 * (xmax - xmin));
	spline_time	= (float *)malloc( spline_num * sizeof(float) );
	for( time_index=0; time_index<spline_num; time_index++){
		spline_time[time_index] = (float)(10*time_index + (int)xmin);
	}

	/*-------- WINDOW FRAME FOR EACH SS --------*/
	for(ss_index=0; ss_index<ssnum; ss_index++){
		cpgbbuf();
		cpgsvp( 0.12, 0.92,
				0.067+ywin_incr*ss_index, 0.067+ywin_incr*(ss_index+1));
		cpgswin( xmin, xmax, ymin, ymax );
		cpgsci(2); cpgrect( xmin, xmax, ymin, ymax );

		/*-------- TICK MARK AND LABEL --------*/
		cpgsci(1);
		if(ss_index == 0 ){
			cpgsch(1.0);
			cpgtbox( "BSTNZH", 0.0, 0, "BCTS", 0.0, 0 );
		} else if( ss_index == ssnum - 1){
			cpgtbox( "BCSTZH", 0.0, 0, "BCTS", 0.0, 0 );
			cpgsch(1.0);
			x_text = 0.5*xmin + 0.5*xmax;	y_text =-0.4*ymin + 1.4*ymax;
			cpgptxt( x_text, y_text, 0.0, 0.5, "P-CAL PLOT");

			cpgsch(0.5);
			x_text = 0.2*xmin + 0.8*xmax;	y_text =-0.5*ymin + 1.5*ymax;
			sprintf( text, "OBSCODE : %s", obs.obscode );
			cpgtext( x_text, y_text, text );

			y_text =-0.2*ymin + 1.2*ymax;
			sprintf( text, "STATION : %s", stn_name[stn_id - 1]);
			cpgtext( x_text, y_text, text );

		} else {
			cpgtbox( "BSTZH", 0.0, 0, "BCTS", 0.0, 0 );
		}
		cpgsch(0.5);
		cpgsci( ss_index%8 + 3);
		x_text	= 1.075*xmin - 0.075*xmax;
		y_text	= 0.2*ymin + 0.8*ymax;
		sprintf(text, "SS = %d", ss_index);
		cpgtext( x_text, y_text, text );

		/*-------- PLOT P-CAL PHASE DATA --------*/
		pcal_ptr	= (struct pcal_data *)first_pcal_ptr;
		while( pcal_ptr != NULL ){
			x_data	= (float)( SECDAY * (pcal_ptr->mjd - (int)obs.start_mjd));
			ref_phs	= pcal_ptr->phs[ss_index] - pcal_ptr->phs[ref_ss];

			if( pcal_ptr->err[ss_index] > 0.0 ){ 
				*weight_ptr	= 1.0/(	pcal_ptr->err[ss_index]
								*	pcal_ptr->err[ss_index]);
			} else {
				*weight_ptr	= 1.0e-6;
			}
			*pcal_phs	= atan2( sin(ref_phs), cos(ref_phs) );
			y_data	= (float)( *pcal_phs );
			y_top	= y_data + (float)(pcal_ptr->err[ss_index]);
			y_bottom= y_data - (float)(pcal_ptr->err[ss_index]);
			cpgpt( 1, &x_data, &y_data, 17 );
#ifdef HIDOI
			printf(" SS=%2d: TIME=%f PHS=%f ERR=%lf\n",
				ss_index, x_data, y_data, pcal_ptr->err[ref_ss]);
#endif
			cpgerry( 1, &x_data, &y_bottom, &y_top, 0.0 );
			pcal_ptr = pcal_ptr->next_pcal_ptr;
			weight_ptr	++;
			pcal_phs	++;
		}
		weight_ptr -= time_num;
		pcal_phs -= time_num;

		pcal_spline( time_ptr, pcal_phs, weight_ptr, time_num,
				atof(argv[SOLINT]),
				&real_coeff[ss_index], &imag_coeff[ss_index],
				time_node, &node_num );

		for( time_index=0; time_index<spline_num; time_index++){


			get_pcalphase( real_coeff[ss_index], imag_coeff[ss_index],
				time_node, node_num, spline_time[time_index], &spline_phs );

			plot_phs	= (float)spline_phs;
			cpgsci(1);
			cpgpt( 1, &spline_time[time_index], &plot_phs, 1);

		}

		cpgebuf();

	}
	cpgend();
	free( time_ptr );
	free( pcal_phs );
	free( spline_time );

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
