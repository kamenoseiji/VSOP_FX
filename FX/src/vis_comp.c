/*********************************************************
**	VIS_COMP.C	: Integrate Visibility in CFS and Disp	**
**			Cross Power Spectrum.						**
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
#define	MAX_BLOCK	1024
#define	RADDEG	57.29577951308232087721
#define	RESVEL	0.4213408731				/* km/sec / CH 	*/
#define	SECDAY	86400
#define FILE1		1
#define FILE2		2
#define FREQNUM		3
#define CALSOM		4
#define BUNCH		5
#define DEVICE		6


MAIN__( argc, argv )
	int		argc;		/* Number of Arguments */
	char	**argv;		/* Pointer of Arguments */
{
	int		ss_index;				/* Index of Sub-Stream				*/
	int		index;					/* Index for Frequency				*/
	int		index2;					/* Index for Frequency				*/
	int		index3;					/* Index for Frequency				*/
	int		node_index;				/* Index for Node Point				*/

	/*-------- TOTAL NUMBER --------*/
	int		freq_num;				/* Number of Frequency				*/
	int		bunch_num;				/* Number of Frequency				*/

	/*-------- IDENTIFIER --------*/
	FILE	*file_ptr1;				/* Visibility File Pointer			*/
	FILE	*file_ptr2;				/* Calib File Pointer				*/
	double	*visr, *visi;			/* Visibility Pointer (real, imag)	*/
	double	*calr, *cali;			/* Calib Pointer (real, imag)		*/
	float	*amp, *phs;				/* Amplitude and Phase to Plot		*/
	float	*freq_ptr;
	double	*real, *imag;			/* Temporal Visibility				*/
	double	*real_coeff;			/* Spline Coefficient				*/
	double	*imag_coeff;			/* Spline Coefficient				*/
	double	*real_wgt;				/* Spline Coefficient				*/
	double	*real_node;				/* Spline Coefficient				*/
	double	*imag_node;				/* Spline Coefficient				*/
	double	*freq_data;				/* Spline Coefficient				*/
	double	real_sum, imag_sum;
	double	current_real;			/* Smoothed Real Data				*/
	double	current_imag;			/* Smoothed Real Data				*/
	int		solint;					/* Solution Interval				*/
	int		real_node_num;			/* Spline Coefficient				*/
	int		imag_node_num;			/* Spline Coefficient				*/
	char	pg_dev[64];				/* PGPLOT Device Name				*/
	double	max_amp;

	/*-------- SSL2 VALIABLE --------*/
	int		isw;					/* Control Code						*/
	int		icon;					/* Condition Code					*/
	int		spline_dim;				/* Spline Function Dimension		*/
	double	vw[4];					/* Work Space						*/
/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 6 ){
		printf("USAGE : vis_comp [FILE1] [FILE2] [FREQ NUM] [CALSOM] [BUNCH NUM] [DEVICE] !!\n");
		printf("  FILE1 --------- SOURCE file \n");
		printf("  FILE2 --------- CALIB file \n");
		printf("  FREQ NUM ------ Number of Frequency CH \n");
		printf("  CALSOM -------- Bandpass Smoothing (Node Interval) \n");
		printf("  BUNCH NUM ----- Bunching Number of Frequency CH \n");
		printf("  DEVICE -------- PGPLOT Device \n");
		exit(0);
	}
/*
------------------------ ACCESS TO THE CODA FILE SYSTEM (CFS)
*/
	freq_num = atoi(argv[FREQNUM]);
	file_ptr1 = fopen( argv[FILE1], "r" );
	file_ptr2 = fopen( argv[FILE2], "r" );
	if( file_ptr1 == NULL ){
		printf("Can't Open %s !!\n", argv[FILE1]);	exit(0); }
	if( file_ptr2 == NULL ){
		printf("Can't Open %s !!\n", argv[FILE2]);	exit(0); }


	visr = (double *)malloc( freq_num* sizeof(double) );
	visi = (double *)malloc( freq_num* sizeof(double) );
	calr = (double *)malloc( freq_num* sizeof(double) );
	cali = (double *)malloc( freq_num* sizeof(double) );
	real = (double *)malloc( freq_num* sizeof(double) );
	imag = (double *)malloc( freq_num* sizeof(double) );
	real_wgt	= (double *)malloc( freq_num* sizeof(double) );
	freq_data	= (double *)malloc( freq_num* sizeof(double) );


	fread( visr, 1, freq_num* sizeof(double), file_ptr1);
	fread( visi, 1, freq_num* sizeof(double), file_ptr1);
	fread( calr, 1, freq_num* sizeof(double), file_ptr2);
	fread( cali, 1, freq_num* sizeof(double), file_ptr2);

	for(index=0; index<freq_num; index ++){
		real[index] = calr[index];
		imag[index] = cali[index];
		freq_data[index] = (double)index;
		real_wgt[index]  = 1.0;
	}

	solint = atoi( argv[CALSOM] );
	real_spline( freq_data, real, real_wgt, freq_num, (double)solint,
		&real_node_num, &real_coeff, &real_node);
	real_spline( freq_data, imag, real_wgt, freq_num, (double)solint,
		&imag_node_num, &imag_coeff, &imag_node);

	isw = 0;
	spline_dim = 3;
	node_index = 0;
	for(index=0; index<freq_num; index ++){
		dbsf1_( &spline_dim, real_node, &real_node_num, real_coeff, &isw,
				&freq_data[index], &node_index, &current_real, vw, &icon);
		calr[index] = current_real;

		dbsf1_( &spline_dim, imag_node, &imag_node_num, imag_coeff, &isw,
				&freq_data[index], &node_index, &current_imag, vw, &icon);
		cali[index] = current_imag;
	}

	for(index=0; index<freq_num; index ++){
		real[index] = (visr[index]* calr[index] + visi[index]* cali[index])
			 		/ (calr[index]* calr[index] + cali[index]* cali[index]);
		imag[index] = (calr[index]* visi[index] - visr[index]* cali[index])
			 		/ (calr[index]* calr[index] + cali[index]* cali[index]);
	}

	bunch_num = atoi(argv[BUNCH]);
	freq_num /= bunch_num;

	amp  = (float *)malloc( freq_num* sizeof(float) );
	phs  = (float *)malloc( freq_num* sizeof(float) );
	freq_ptr  = (float *)malloc( freq_num* sizeof(float) );

	max_amp = 0.0;
	for(index=0; index<freq_num; index ++){
		real_sum = 0.0; imag_sum = 0.0;
		for(index2=0; index2<bunch_num; index2++){
			index3 = index* bunch_num + index2;
			real_sum += real[index3];
			imag_sum += imag[index3];
		}

		amp[index] = (float)sqrt( real_sum* real_sum + imag_sum* imag_sum );
		phs[index] = (float)atan2(imag_sum, real_sum);
		freq_ptr[index] = (float)index + 0.5;

		if( amp[index] > max_amp ){	max_amp = amp[index];	}
	}

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
	cpgsvp(0.1, 0.9, 0.1, 0.9);
	cpgswin( 0.0, (float)freq_num, 0.0, (float)max_amp*1.2 );
	cpgbox("BCNTS", 0.0, 0, "BCNTS", 0.0, 0);
	cpglab("Freq. CH", "Relative Amplitude", argv[FILE1]);
	cpgline( freq_num, freq_ptr, amp );

	fclose(file_ptr1);	fclose(file_ptr2);
	free(visr);			free(visi);
	free(calr);			free(cali);
	cpgend();
	return(0);
}
