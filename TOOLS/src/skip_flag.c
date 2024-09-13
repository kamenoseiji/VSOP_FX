/*********************************************************
**	SKIP_FLAG.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define	SECDAY	86400
#define	MAX_GAP		600.0
#define	MAX_LOOP1	10

int	skip_flag( flgunit, position, freq_num, time_num_cfs, start_mjd, time_incr )
	int		flgunit;				/* Logical Unit Number for FLAG data*/
	int		*position;				/* Position to search				*/
	int		freq_num;				/* Frequency Channel Number			*/
	int		time_num_cfs;			/* Time Data Points in CFS			*/
	double	start_mjd;				/* INTEG START TIME [MJD] 			*/
	double	time_incr;				/* Time Increment					*/
{
	int		skip_pp;				/* SKIP NUMBER 						*/
	int		origin;					/* REFERENCE POINT IN CFS 			*/
	int		loop_counter;			/* Loop Counter 					*/
	int		ret;					/* Return Code from CFS Library 	*/
	double	mjd_flag;
	double	time_diff;				/* Time Difference [sec]			*/
	double	init_mjd;				/* Initial MJD in CFS				*/
	double	final_mjd;				/* Final MJD in CFS					*/
	double	time_span;				/* Time Span [sec]					*/
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
	printf("POSITION  = %d\n", *position);
	printf("START MJD = %lf\n", start_mjd);
	printf("TIME_NUM  = %lf\n", time_num_cfs);
	#endif

	if( *position >= 0 ){
		origin	= 0;	skip_pp = *position;
		cfs400_( &flgunit, &origin, &skip_pp, &ret); cfs_ret(400,ret);
		return(0);
	}
	spec_num	= freq_num;
/*
--------------------------------- GET INITIAL and FINAL TIME in CFS
*/
	/*-------- IF TIME_NUM_CFS IS NOT CORRECT, SEARCH SEAQUENTLY --------*/
	if(time_num_cfs < 1){
		printf("Warning : CFS headder has some trouble...\n");
		cfs401_( &flgunit, &ret);					/* REWIND TO THE TOP	*/
		mjd_flag = 0.0;

		sequent_flag_search(&flgunit, position, &spec_num, start_mjd, &mjd_flag,
				&current_obj, (int)SECDAY, flag, uvw, &result );

		return( result );
	}

	/*-------- READ INITIAL MJD from FLAG --------*/
	cfs401_( &flgunit, &ret);					/* REWIND TO THE TOP	*/
	cfs225_( &flgunit, &init_mjd, &current_obj,	/* READ INITIAL TIME	*/
			uvw, flag, &spec_num, &ret);
	cfs_ret( 225, ret );

	/*-------- READ FINAL MJD from FLAG --------*/
	origin = 0;	skip_pp = time_num_cfs - 1;
	cfs400_( &flgunit, &origin, &skip_pp, &ret);/* GO TO THE END		*/
	cfs225_( &flgunit, &final_mjd, &current_obj,/* READ FINAL TIME		*/
			uvw, flag, &spec_num,&ret);

	/*-------- CHECK FOR START TIME --------*/
	if( init_mjd > start_mjd ){
		printf("WARNIG...START TIME is before the first data.\n");
		printf("START = %lf, INITIAL MJD = %lf\n", start_mjd, init_mjd);
		cfs401_( &flgunit, &ret);					/* REWIND TO THE TOP	*/
		mjd_flag = 0.0;

		sequent_flag_search(&flgunit, position, &spec_num, start_mjd, &mjd_flag,
				&current_obj, (int)SECDAY, flag, uvw, &result );
		return( result );
	}

	if( final_mjd < start_mjd ){
		printf("WARNIG...START TIME exceeds the end of data.\n");
		printf("START = %lf, FINAL MJD = %lf\n", start_mjd, final_mjd);
		cfs401_( &flgunit, &ret);					/* REWIND TO THE TOP	*/
		mjd_flag = 0.0;

		sequent_flag_search(&flgunit, position, &spec_num, start_mjd, &mjd_flag,
				&current_obj, (int)SECDAY, flag, uvw, &result );

		return( result );
	}
	time_span	= SECDAY*(final_mjd - init_mjd);
	mjd_flag	= final_mjd;
/*
--------------------------------- STEP 1 : COARSE SEARCH
*/
	origin=1; loop_counter = 0;	spec_num = freq_num;
	while(loop_counter < MAX_LOOP1){
		time_diff	= SECDAY * (start_mjd - mjd_flag);

#ifdef DEBUG
		printf("TIME DIFFERENCE = %lf SEC ... ", time_diff);
#endif

		if( (time_diff >= 0.0) && (time_diff < MAX_GAP) ){	break;}
		skip_pp = (int)(time_diff * (double)time_num_cfs / time_span );

#ifdef DEBUG
		printf("SKIP %d RECORD.\n", skip_pp);
#endif

		cfs400_( &flgunit, &origin, &skip_pp, &ret); cfs_ret(400,ret);
		cfs225_( &flgunit,&mjd_flag,&current_obj, uvw, flag, &spec_num,&ret);
		cfs_ret( 225, ret );

		loop_counter++;
	}

	sequent_flag_search(&flgunit, position, &spec_num, start_mjd, &mjd_flag,
				&current_obj, (int)(MAX_GAP/time_incr), flag, uvw, &result );

	return( result );
}


/*
--------------------------------- SEARCH START TIME STEP by STEP
*/
sequent_flag_search( flgunit_ptr, position, freq_num_ptr, start_mjd,
		mjd_flg_ptr, current_obj_ptr, loop_limit, flag_ptr, uvw_ptr,
		result_ptr )

	int		*flgunit_ptr;			/* Logical Unit Number for FLAG data*/
	int		*position;				/* Position to search				*/
	int		*freq_num_ptr;			/* Pointer of Frequency Number		*/
	double	start_mjd;				/* INTEG START TIME [MJD] 			*/
	double	*mjd_flg_ptr;			/* Pointer of Current MJD			*/
	int		*current_obj_ptr;		/* Pointer of Current Object ID		*/
	int		loop_limit;				/* Limit of Search Loop				*/
	char	*flag_ptr;				/* Pointer of Flag					*/
	double	*uvw_ptr;				/* Pointer of U, V, W				*/
	int		*result_ptr;			/* SEARCH RESULT					*/
{
	int		loop_counter;			/* Loop Counter						*/
	int		current_rec;			/* Current Record ID				*/
	int		rec_num;				/* Record Number					*/
	int		rec_len;				/* Record Length					*/
	int		blk_len;				/* Block Length						*/
	int		ret;					/* Return Code						*/
	double	time_diff;				/* Time Difference [sec]			*/

	loop_counter = 0;
	while( loop_counter < loop_limit ){

		time_diff	= SECDAY*(start_mjd - *mjd_flg_ptr);
		if( time_diff <= 0.0){

			cfs405_( flgunit_ptr, &current_rec,
					 &rec_num, &rec_len, &blk_len, &ret); 
			#ifdef DEBUG
			printf(" Current Rec ID = %d\n", current_rec);
			#endif

			*position	= current_rec - 1;
			*result_ptr	= 0;
			return;
		}

		/*-------- READ CURRENT MJD from DATA and FLAG --------*/
		cfs225_( flgunit_ptr, mjd_flg_ptr, current_obj_ptr, uvw_ptr,
				flag_ptr, freq_num_ptr, &ret);

		if(ret != 0){	break;}

		#ifdef DEBUG
		printf("START = %lf, CURRECT MJD = %lf.\n", start_mjd, *mjd_flg_ptr);
		#endif

		loop_counter++;
	}

	printf("Warnig : Failed to search the Data Point...[MJD = %lf]\n",
			start_mjd);
	*result_ptr	= -1;
	return;
}
