/*********************************************************
**	SKIP_CODA.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "obshead.inc"
#define	SECDAY	86400
#define	MAX_GAP		600.0
#define	MAX_LOOP1	10
#define	MAX_BLOCK	1024

int	skip_coda( corunit, flgunit, position, time_num_cfs, start_mjd,
				freq_num, time_incr )
	int		corunit;				/* Logical Unit Number for CORR data*/
	int		flgunit;				/* Logical Unit Number for FLAG data*/
	int		*position;				/* Position to search				*/
	double	start_mjd;				/* INTEG START TIME [MJD] 			*/
	int		freq_num;				/* Frequency Number					*/
	double	time_incr;				/* Time Increment					*/
{
	int		skip_pp;				/* SKIP NUMBER 						*/
	int		origin;					/* REFERENCE POINT IN CFS 			*/
	int		loop_counter;			/* Loop Counter 					*/
	int		loop_limit;				/* Limit Number of Loop 			*/
	int		ret;					/* Return Code from CFS Library 	*/
	double	mjd_data;
	double	mjd_flag;
	double	init_mjd;				/* Initial MJD in CFS				*/
	double	final_mjd;				/* Final MJD in CFS					*/
	double	time_span;				/* Time Span [sec]					*/
	double	time_diff;				/* Time Difference [sec]			*/
	double	uvw[3];
	int		spec_num;
	unsigned char	flag[1024];
	int		current_obj;
	int		valid_pp;				/* How Many DATA Was Valid */
	int		result;					/* SEARCH RESULT					*/
/*
--------------------------------- STEP 0 : SKIP TO POSITION
*/
	#ifdef DEBUG
	printf("CORUNIT   = %d\n", corunit);
	printf("POSITION  = %d\n", *position);
	printf("START MJD = %lf\n", start_mjd);
	printf("FREQ NUM  = %d\n", freq_num);
	#endif

	if( *position >= 0 ){
		origin	= 0;	skip_pp = *position;
		cfs400_( &corunit, &origin, &skip_pp, &ret); cfs_ret(400,ret);
		cfs400_( &flgunit, &origin, &skip_pp, &ret); cfs_ret(400,ret);
		return(0);
	}
/*
--------------------------------- GET INITIAL and FINAL TIME in CFS
*/

	/*-------- IF TIME_NUM_CFS IS NOT CORRECT, SEARCH SEAQUENTLY --------*/
	if(time_num_cfs < 1){
		printf("Warning : CFS headder has some trouble...\n");
		cfs401_( &flgunit, &ret);					/* REWIND TO THE TOP	*/
		mjd_flag = 0.0;
		result = sequent_search( corunit, flgunit, spec_num, position, flag,
				 start_mjd, mjd_flag, time_diff, SECDAY, time_incr);
		return( result );
	}

	spec_num = freq_num;
	/*-------- READ INITIAL MJD from FLAG --------*/
	cfs401_( &flgunit, &ret);					/* REWIND TO THE TOP	*/
	cfs225_( &flgunit, &init_mjd, &current_obj,	/* READ INITIAL TIME	*/
			uvw, flag, &spec_num, &ret);
	cfs_ret( 225, ret );

	/*-------- READ FINAL MJD from FLAG --------*/
	origin = 0;	skip_pp = time_num_cfs - 1;
	cfs400_( &flgunit, &origin, &skip_pp, &ret);/* GO TO THE END		*/
	cfs225_( &flgunit, &final_mjd,&current_obj,	/* READ FINAL TIME		*/
			uvw, flag, &spec_num,&ret);

	/*-------- CHECK FOR START TIME --------*/
	if( init_mjd > start_mjd ){
		printf("WARNIG...START TIME is before the first data.\n");
		printf("START = %lf, INITIAL MJD = %lf\n", start_mjd, init_mjd);
		cfs401_( &flgunit, &ret);					/* REWIND TO THE TOP	*/
		mjd_flag = 0.0;
		result = sequent_search( corunit, flgunit, spec_num, position, flag,
				 start_mjd, mjd_flag, time_diff, SECDAY, time_incr);
		return( result );
	}

	if( final_mjd < start_mjd ){
		printf("WARNIG...START TIME exceeds the end of data.\n");
		printf("START = %lf, FINAL MJD = %lf\n", start_mjd, final_mjd);
		cfs401_( &flgunit, &ret);					/* REWIND TO THE TOP	*/
		mjd_flag = 0.0;
		result = sequent_search( corunit, flgunit, spec_num, position, flag,
				 start_mjd, mjd_flag, time_diff, SECDAY, time_incr);
		return( result );
	}
	time_span	= SECDAY * (final_mjd - init_mjd);
	mjd_flag	= final_mjd;
/*
--------------------------------- STEP 1 : COARSE SEARCH
*/
	origin=1; loop_counter = 0;	spec_num = freq_num;
	while(loop_counter < MAX_LOOP1){
		time_diff	= SECDAY*(start_mjd - mjd_flag);

#ifdef DEBUG
		printf("TIME DIFFERENCE = %lf SEC ...", time_diff);
#endif

		if( (time_diff >= 0.0) && (time_diff < MAX_GAP) ){	break;}
		skip_pp = (int)(time_diff * (double)time_num_cfs / time_span ) - 2;

#ifdef DEBUG
		printf("SKIP %d RECORD.\n", skip_pp);
#endif

		cfs400_(&flgunit, &origin, &skip_pp, &ret); cfs_ret(400,ret);
		cfs225_(&flgunit, &mjd_flag, &current_obj, uvw, flag, &spec_num,&ret);
		cfs_ret( 225, ret );

		loop_counter++;
	}

	result = sequent_search( corunit, flgunit, spec_num, position, flag,
		start_mjd, mjd_flag, time_diff, (int)(MAX_GAP/time_incr), time_incr );
	return( result );
}


/*
--------------------------------- SEARCH START TIME STEP by STEP
*/
int sequent_search( corunit, flgunit, spec_num, position, flag,
					start_mjd, mjd_flag, time_diff, loop_limit, time_incr )
	int		corunit;				/* Logical Unit Number for CORR data*/
	int		flgunit;				/* Logical Unit Number for FLAG data*/
	int		spec_num;				/* Number of Spectral Channel		*/
	int		*position;				/* Position to search				*/
	char	*flag;					/* Flag								*/
	double	start_mjd;				/* INTEG START TIME [MJD] 			*/
	double	mjd_flag;				/* MJD in FLAG File [MJD] 			*/
	double	time_diff;				/* Time Difference [sec]  			*/
	int		loop_limit;				/* Limit of Search Loop				*/
	double	time_incr;				/* Time Increment					*/
{
	int		result;					/* Result of Search					*/
	int		loop_counter;			/* Loop Counter						*/
	int		skip_pp;				/* PP Number to Skip				*/
	int		ret;					/* Return Code in CFS				*/
	int		origin;					/* REFERENCE POINT IN CFS 			*/
	int		current_obj;			/* Current Object ID				*/
	int		rec_num;				/* Number of Records				*/
	int		rec_len;				/* Record Length					*/
	int		blk_len;				/* Block Length						*/
	int		current_rec;			/* Current Record ID				*/
	double	uvw[3];					/* UVW Coordinate					*/

	loop_counter = 0;
	while( loop_counter < loop_limit ){

		time_diff	= SECDAY*(start_mjd - mjd_flag);

		/*-------- FIND SEARCH POSITION --------*/
		if( fabs(time_diff) < 0.6* time_incr){

			cfs405_( &flgunit, &current_rec,
					 &rec_num, &rec_len, &blk_len, &ret); 
			#ifdef DEBUG
			printf(" Current Rec ID = %d\n", current_rec);
			#endif

			*position	= current_rec - 1;
			origin	= 0;	skip_pp = *position;
			cfs400_( &corunit, &origin, &skip_pp, &ret); cfs_ret(400,ret);
			result	= 0;
			return(result);
		}

		/*-------- OVER the SEARCH POSITION --------*/
		if( time_diff < 0.0 ){
			origin	= 1;	skip_pp = -2;
			cfs400_( &corunit, &origin, &skip_pp, &ret); cfs_ret(400,ret);
		}

		/*-------- READ CURRENT MJD from DATA and FLAG --------*/
		cfs225_( &flgunit,&mjd_flag,&current_obj, uvw, flag, &spec_num, &ret);
		if(ret != 0){	break;}

		#ifdef DEBUG
		printf("START = %lf, CURRECT MJD = %lf.\n", start_mjd, mjd_flag);
		#endif

		loop_counter++;
	}

	printf("Warnig : Failed to search the Data Point...[MJD = %lf]\n",
			start_mjd);
	result	= -1;
	return(result);
}
