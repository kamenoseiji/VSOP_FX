/*****************************************************************
**	TIME_SPAN_CODA.C : Load Visibility Phase with P-Cal Correction	**
**																**
**	AUTHOR	: KAMENO Seiji										**
**	CREATED	: 1996/6/27											**
*****************************************************************/
#include <stdio.h>
#include <math.h>
#define	CORRFLAG	5

int	time_span_coda( bl_index, ss_index,  obj_id, position,
			freq_num, time_num_cfs, time_incr, start_mjd, stop_mjd,
			time_data_ptr,	pp_num_ptr )

	int		bl_index;				/* Baseline Index 					*/
	int		obj_id;					/* Object ID Number 				*/
	int		ss_index;				/* Sub-Stream Index 				*/
	int		*position;				/* Search Position					*/
	int		freq_num;				/* Number of Frequency				*/
	int		time_num_cfs;			/* Number of Time in CFS			*/
	double	time_incr;				/* Time Increment [sec]				*/
	double	start_mjd;				/* INTEG START TIME [MJD]			*/
	double	stop_mjd;				/* INTEG STOP TIME [MJD]			*/
	double	*time_data_ptr;			/* Pointer of Time Data [MJD]		*/
	int		*pp_num_ptr;			/* Pointer of PP Number				*/
{
	int		flgunit;				/* Logical Unit Number for FLAG data */
	int		data_index;				/* Index for Data					*/
	int		current_pp;				/* Current PP Number				*/
	int		ret;					/* Return Code from CFS Library */
	char	omode[2];				/* CFS Access Mode */
	char	fname[32];				/* CFS File Name */
	double	mjd_flag;
	double	uvw[3];
	unsigned char	flag[1024];
	int		current_obj;
	int		valid_pp;				/* How Many DATA Was Valid */
/*
------------------------------- INITIALIZE MEMORY AREA FOR VISIBILITIES
*/
	flgunit = CORRFLAG;
	sprintf(omode, "r"); 

	/*-------- OPEN FLAG DATA --------*/
	sprintf(fname, "CORR.%d/SS.%d/FLAG.1\0", bl_index, ss_index ); 
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&flgunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- SKIP TO THE START MJD --------*/
	skip_flag( flgunit, position, freq_num, time_num_cfs, start_mjd, time_incr);

	/*-------- READ VISIBILITY AND FLAG DATA --------*/
	valid_pp	= 0;
	data_index	= 0;
	current_pp	= *position;
	while( mjd_flag <= stop_mjd ){

		/*-------- LOAD VISIBILITY TO WORK AREA --------*/
		cfs225_( &flgunit, &mjd_flag, &current_obj, uvw,
					flag, &freq_num, &ret );

		/*-------- IS THIS THE TARGET SOURCE ? --------*/
		if( current_obj ==  obj_id){
			pp_num_ptr[ data_index ]	= current_pp;
			time_data_ptr[ data_index ]	= mjd_flag;
			data_index ++;
		}
		current_pp ++;
	}
	valid_pp	= data_index;

	/*-------- FOUND NO TARGET SOURCE --------*/
	if( valid_pp == 0){
		printf(" CAUTION : TARGET SOURCE [ID=%d] WAS NOT FOUND...\n", obj_id );
		return(0);
	}

	/*-------- CLOSE FLAG FILE --------*/
	cfs104_( &flgunit, &ret );	cfs_ret( 104, ret );

	return(valid_pp);
}
