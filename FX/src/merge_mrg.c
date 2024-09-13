/*********************************************************
**	MERGE_MRG.C	: MERGE CIT Merge Format Data	 		**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include <cpgplot.h>
#include "merge.inc"
#define	MAX_STN		16
#define	MAX_BL		64
#define	MAX_CL		128
#define	SECDAY		86400
#define	RADDEG		57.29577951308232087721

main( argc, argv )
	int		argc;		/* Number of Arguments			*/
	char	**argv;		/* Pointer of Arguments			*/
{
	/*-------- CIT MERGE FORMAT --------*/
	FILE	*mrg_file_ptr;				/* Merge File Poiter				*/
	struct	mrg_header		header1;	/* Header in Merge File				*/
	struct	mrg_header		header2;	/* Header in Merge File				*/
	struct	mrg_header		header;		/* Header in Merge File				*/
	struct	mrg_source		source1;	/* Source information 				*/
	struct	mrg_source		source2;	/* Source information 				*/
	struct	mrg_source		source;		/* Source information 				*/
	struct	mrg_station2	*stn1;		/* Station Format 2 				*/
	struct	mrg_station2	*stn2;		/* Station Format 2 				*/
	struct	mrg_station2	*stn;		/* Station Format 2 				*/
	struct	mrg_vis2		vis2;		/* Visibility Format 2				*/
	struct	mrg_misc		misc1;		/* Miscellaneous Information		*/
	struct	mrg_misc		misc2;		/* Miscellaneous Information		*/
	struct	mrg_misc		misc;		/* Miscellaneous Information		*/
	struct	mrg_close		cls;		/* Closure Phase					*/
	int		byte_len;					/* Byte Length of Record in MRG		*/
	int		bl_id;						/* Baseline ID in MRG				*/
	int		cl_id;						/* Closure in MRG					*/
	int		ut;							/* UT in MERGE File [1/60 sec]		*/
	char	history[80];				/* History Record					*/

	/*-------- POINTER --------*/
	int		stn_ptr1[MAX_STN];			/* Pointer of Station Infor			*/
	int		stn_ptr2[MAX_STN];			/* Pointer of Station Infor			*/
	int		vis_amp[MAX_BL];			/* Pointer of VisAmp vs Time		*/
	int		vis_phs[MAX_BL];			/* Pointer of VisPhs vs Time		*/
	int		err_amp[MAX_BL];			/* Pointer of VisAmpErr vs Time		*/
	int		err_phs[MAX_BL];			/* Pointer of VisPhsErr vs Time		*/
	int		cl_phs[MAX_CL];				/* Pointer of VisPhsErr vs Time		*/
	int		cl_err[MAX_CL];				/* Pointer of VisPhsErr vs Time		*/
	int		uvw_ptr[MAX_BL];			/* Pointer of UVW Coord				*/

	/*-------- IDENTIFIER --------*/
	int		stnmap1[MAX_STN];			/* Station Map (mrg1 -> dest)		*/
	int		stnmap2[MAX_STN];			/* Station Map (mrg2 -> dest)		*/
	int		blmap1[MAX_BL];				/* Baseline Map (mrg1 -> dest)		*/
	int		blmap2[MAX_BL];				/* Baseline Map (mrg2 -> dest)		*/
	int		clmap1[MAX_CL];				/* Closure Map (mrg1 -> dest)		*/
	int		clmap2[MAX_CL];				/* Closure Map (mrg2 -> dest)		*/

	/*-------- INDEX --------*/
	int		stn_index;					/* Station Index in Input List		*/
	int		stn_index2;					/* Station Index in Input List		*/
	int		bl_index;					/* Baseline Index from Input List	*/
	int		cl_index;					/* Closure Index from Input List	*/
	int		ss_index;					/* Sub-Stream Index					*/
	int		time_index;					/* Index for Time					*/
	int		index;						/* General Index					*/
	int		ant1, ant2, ant3;			/* Ad-Hoc Stn. Index for BL and CL	*/
	int		bl1, bl2, bl3;				/* Ad-Hoc BL Index for STN and CL	*/

	/*-------- TOTAL NUMBER --------*/
	int		stn_num1;					/* Number of Stations Selected		*/
	int		stn_num2;					/* Number of Stations Selected		*/
	int		stn_num;					/* Number of Stations Selected		*/
	int		bl_num1;					/* Number of Baseline Selected		*/
	int		bl_num2;					/* Number of Baseline Selected		*/
	int		bl_num;						/* Number of Baseline Selected		*/
	int		cl_num1;					/* Number of Closure Selected		*/
	int		cl_num2;					/* Number of Closure Selected		*/
	int		cl_num;						/* Number of Closure Selected		*/
	int		time_num1;					/* Time Data Points					*/
	int		time_num2;					/* Time Data Points					*/
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
	int		*time_ptr1;					/* Time List [UT in 1/60 sec]		*/
	int		*time_ptr2;					/* Time List [UT in 1/60 sec]		*/
	int		*time_ptr;					/* Time List [UT in 1/60 sec]		*/
	float	*time_list;					/* Time List [Sec of Day]			*/

	/*-------- Visibility Variable --------*/
	double	loss;						/* Correlation Loss Factor			*/
	float	*ave_vis;					/* Averaged Visibility Array		*/
	float	*err_vis;					/* Error of Ave. Visibility Array	*/
	double	acc_min, acc_max;			/* Min and Max of Acceleration		*/
	double	rate_min, rate_max;			/* Min and Max of Rate				*/
	double	delay_min, delay_max;		/* Min and Max of Delay				*/
	double	gain_min, gain_max;			/* Min and Max of Gain				*/
	double	clphs[MAX_CL];				/* Closure Phase					*/
	double	vis_ss_r, vis_ss_i;			/* Vis Sum for SS					*/
	double	vis_sigma;					/* Vis Square Sum					*/
	double	uvw[3];						/* U, V, W							*/
	double	*double_ptr;				/* Pointer of Double				*/
	struct mrg_vis2		*vis_ptr1;		/* Pointer of Visibility in MRG		*/
	struct mrg_vis2		*vis_ptr2;		/* Pointer of Visibility in MRG		*/
	struct mrg_close	*cl_ptr1;		/* Pointer of Closure in MRG		*/
	struct mrg_close	*cl_ptr2;		/* Pointer of Closure in MRG		*/

	/*-------- General Variable --------*/
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

/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 4 ){
		printf("USAGE : merge_mrg [FILE1] [FILE2] [DEST FILE] [STN1] [STN2] ... !!\n");
		exit(0);
	}
	stn_num = argc - 4;
	bl_num  = (stn_num* (stn_num - 1)) / 2;
	cl_num  = (stn_num* (stn_num - 1)* (stn_num - 2)) / 6;
	for(stn_index=0; stn_index<stn_num; stn_index++){
		sprintf( stn_list[stn_index], "%s", argv[stn_index + 4]);
	}

	count_merge( argv[1], &stn_num1, &bl_num1, &cl_num1, &time_num1 );
	printf("TOTAL %d Time Data!!\n", time_num1);
	count_merge( argv[2], &stn_num2, &bl_num2, &cl_num2, &time_num2 );
	printf("TOTAL %d Time Data!!\n", time_num2);

	time_ptr1 = (int *)malloc( time_num1* sizeof(int) );
	time_ptr2 = (int *)malloc( time_num2* sizeof(int) );
	vis_ptr1  = (struct mrg_vis2 *)
				malloc(time_num1* bl_num1* sizeof(struct mrg_vis2));
	vis_ptr2  = (struct mrg_vis2 *)
				malloc(time_num2* bl_num2* sizeof(struct mrg_vis2));
	cl_ptr1   = (struct mrg_close *)
				malloc(time_num1* cl_num1* sizeof(struct mrg_close));
	cl_ptr2   = (struct mrg_close *)
				malloc(time_num2* cl_num2* sizeof(struct mrg_close));

	read_merge( argv[1], time_num1, &header1, &source1, stn_ptr1, &misc1,
		time_ptr1, vis_ptr1, cl_ptr1 );
	read_merge( argv[2], time_num2, &header2, &source2, stn_ptr2, &misc2,
		time_ptr2, vis_ptr2, cl_ptr2 );

	/*-------- INITIALIZE STATION MAPPING FUNCTION --------*/
	for(stn_index=0; stn_index<MAX_STN; stn_index++){
		stnmap1[stn_index] = -1;
		stnmap2[stn_index] = -1;
	}

	/*-------- STATION MAPPING FUNCTION --------*/
	for(stn_index=0; stn_index<stn_num; stn_index++){

		/*-------- SCAN MRG FILE 1 --------*/ 
		stn_index2 = 0;
		while(stn_index2 < stn_num1){
			stn1 = (struct mrg_station2 *)stn_ptr1[stn_index2];
			if( strstr(stn1->name, stn_list[stn_index]) != NULL ){
				stnmap1[stn_index] = stn_index2;
				break; }
			stn_index2 ++;
		}

		/*-------- SCAN MRG FILE 2 --------*/ 
		stn_index2 = 0;
		while(stn_index2 < stn_num2){
			stn2 = (struct mrg_station2 *)stn_ptr2[stn_index2];
			if( strstr(stn2->name, stn_list[stn_index]) != NULL ){
				stnmap2[stn_index] = stn_index2;
				break; }
			stn_index2 ++;
		}
	}


	printf("TOTAL STATION NUM = %d\n", stn_num);
	for(stn_index=0; stn_index<stn_num; stn_index++){
		printf("  STATION ID %d  : %s\n", stn_index, stn_list[stn_index] );
		printf("     MAP1  : %d\n", stnmap1[stn_index] );
		printf("     MAP2  : %d\n", stnmap2[stn_index] );
	}

	/*-------- BASELINE MAPPING FUNCTION --------*/
	for(bl_index=0; bl_index<bl_num; bl_index++){
		blmap1[bl_index] = -1;		/* INITIALIZE BASELINE MAPPING	*/
		blmap2[bl_index] = -1;

		bl2ant(bl_index, &ant2, &ant1);

		printf(" BASELINE %d : %s - %s\n",
				bl_index, stn_list[ant1], stn_list[ant2]);

		if( (stnmap1[ant1] != -1) && (stnmap1[ant2] != -1) ){
			blmap1[bl_index] = ant2bl( stnmap1[ant1], stnmap1[ant2]);
			printf("    MRG1 : MAPPED FROM BL %d\n", blmap1[bl_index]);
		}
		if( (stnmap2[ant1] != -1) && (stnmap2[ant2] != -1) ){
			blmap2[bl_index] = ant2bl( stnmap2[ant1], stnmap2[ant2]);
			printf("    MRG2 : MAPPED FROM BL %d\n", blmap2[bl_index]);
		}
	}

	/*-------- CLOSURE MAPPING FUNCTION --------*/
	for(cl_index=0; cl_index<cl_num; cl_index++){
		clmap1[cl_index] = -1;		/* INITIALIZE CLOSURE MAPPING	*/
		clmap2[cl_index] = -1;
		cl2ant(cl_index, &ant3, &ant2, &ant1);
		printf(" CLOSURE %d : %s - %s - %s\n",
				cl_index, stn_list[ant1], stn_list[ant2], stn_list[ant3]);

		if( (stnmap1[ant1] != -1)	&& (stnmap1[ant2] != -1)
									&& (stnmap1[ant3] != -1) ){
			clmap1[cl_index]=ant2cl(stnmap1[ant1],stnmap1[ant2],stnmap1[ant3]);
			printf("    MRG1 : MAPPED FROM CL %d\n", clmap1[cl_index]);
		}

		if( (stnmap2[ant1] != -1)	&& (stnmap2[ant2] != -1)
									&& (stnmap2[ant3] != -1) ){
			clmap2[cl_index]=ant2cl(stnmap2[ant1],stnmap2[ant2],stnmap2[ant3]);
			printf("    MRG2 : MAPPED FROM CL %d\n", clmap2[cl_index]);
		}
	}
/*
------------------------ WRITE TO MERGE FILE
*/
	mrg_file_ptr = fopen(argv[3], "w");
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
	memcpy( &source, &source1, sizeof(source) );
	byte_len	= sizeof(source);
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	fwrite( &source, 1, sizeof(source), mrg_file_ptr);
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);

	/*-------- WRITE STATION INFORMATION --------*/
	for(stn_index=0; stn_index<stn_num; stn_index ++){

		if( stnmap1[stn_index] != -1){
			stn = (struct mrg_station2 *)stn_ptr1[stnmap1[stn_index]];
		} else if(stnmap2[stn_index] != -1){
			stn = (struct mrg_station2 *)stn_ptr2[stnmap2[stn_index]];
		} else {
			printf("ERROR in STATION LIST !!\n");
		}

		byte_len= sizeof(struct mrg_station2);
		fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
		fwrite( stn, 1, sizeof(struct mrg_station2), mrg_file_ptr );
		fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );
	}

	/*-------- WRITE MISC INFORMATION --------*/
	memcpy( &misc, &misc1, sizeof(misc) );
	byte_len= sizeof(misc);
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr);
	fwrite( &misc, 1, sizeof(misc), mrg_file_ptr );
	fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );

/*
------------------------ WRITE TO VISIBILITY from MRG1
*/
	/*-------- TIME LOOP --------*/
	for( time_index=0; time_index<time_num1; time_index++){
		ut = time_ptr1[time_index];

		/*-------- WRITE VISIBILITY TO MERGE FILE --------*/
		byte_len= sizeof(ut) + bl_num*sizeof(vis2) + cl_num*sizeof(cls);
		fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );
		fwrite( &ut, 1, sizeof(ut), mrg_file_ptr );


		for( bl_index=0; bl_index<bl_num; bl_index++){
			if( blmap1[bl_index] != -1){
				index = time_index* bl_num1 + blmap1[bl_index];
				fwrite( &vis_ptr1[index], 1, sizeof(vis2), mrg_file_ptr );
			} else {
				memcpy( &vis2, &vis_ptr1[0], sizeof(vis2) );
				vis2.amp_err = -1;
				vis2.phs_err = -1;
				fwrite( &vis2, 1, sizeof(vis2), mrg_file_ptr );
			}
		}

		for( cl_index=0; cl_index<cl_num; cl_index++){
			if( clmap1[cl_index] != -1){
				index = time_index* cl_num1 + clmap1[cl_index];
				fwrite( &cl_ptr1[index], 1, sizeof(cls), mrg_file_ptr );
			} else {
				memcpy( &cls, &cl_ptr1[0], sizeof(cls) );
				cls.phs_err = -1;
				fwrite( &cls, 1, sizeof(cls), mrg_file_ptr );
			}
		}

		fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );

	}	/* END OF TIME LOOP */
/*
------------------------ WRITE TO VISIBILITY from MRG2
*/
	/*-------- TIME LOOP --------*/
	for( time_index=0; time_index<time_num2; time_index++){
		ut = time_ptr2[time_index];

		/*-------- WRITE VISIBILITY TO MERGE FILE --------*/
		byte_len= sizeof(ut) + bl_num*sizeof(vis2) + cl_num*sizeof(cls);
		fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr );
		fwrite( &ut, 1, sizeof(ut), mrg_file_ptr );

		for( bl_index=0; bl_index<bl_num; bl_index++){
			if( blmap2[bl_index] != -1){
				index = time_index* bl_num2 + blmap2[bl_index];
				fwrite( &vis_ptr2[index], 1, sizeof(vis2), mrg_file_ptr );
			} else {
				memcpy( &vis2, &vis_ptr2[0], sizeof(vis2) );
				vis2.amp_err = -1;
				vis2.phs_err = -1;
				fwrite( &vis2, 1, sizeof(vis2), mrg_file_ptr );
			}
		}

		for( cl_index=0; cl_index<cl_num; cl_index++){
			if( clmap2[cl_index] != -1){
				index = time_index* cl_num2 + clmap2[cl_index];
				fwrite( &cl_ptr2[index], 1, sizeof(cls), mrg_file_ptr );
			} else {
				memcpy( &cls, &cl_ptr2[0], sizeof(cls) );
				cls.phs_err = -1;
				fwrite( &cls, 1, sizeof(cls), mrg_file_ptr );
			}
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

	fclose(mrg_file_ptr);
	free(time_ptr1);
	free(vis_ptr1);
	free(cl_ptr1);
	return(0);
}
