/*********************************************************
**	LOAD_VIS.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>
#define	CORRDATA	4
#define	CORRFLAG	5
#define SECDAY		86400
#define	LOW_EDGE	(int)( *freq_num_ptr * 0.05 )
#define	HIGH_EDGE	(int)( *freq_num_ptr * 0.95 + 0.5)

int	load_vis_uvw( bl_index, bl_direction, obj_id, ss_index, position,
			freq_num_ptr, bunch_num, time_num_cfs, time_num, time_incr,
			start_mjd, stop_mjd, uvw_ptr,
			bp1_ptr, bp2_ptr,
			work_ptr, vis_r_ptr, vis_i_ptr )

	int		bl_index;				/* Baseline Index 					*/
	float	bl_direction;			/* Baseline Direction				*/
	int		obj_id;					/* Object ID Number 				*/
	int		ss_index;				/* Sub-Stream Index 				*/
	int		*position;				/* Search Position					*/
	int		*freq_num_ptr;			/* Number of Freq. Channels			*/
	int		bunch_num;				/* Number of Bunching				*/
	int		time_num;				/* Number of Time					*/
	int		time_num_cfs;			/* Number of Time in CFS			*/
	double	time_incr;				/* Time Increment [sec]				*/
	double	start_mjd;				/* INTEG START TIME [MJD]			*/
	double	stop_mjd;				/* INTEG STOP TIME [MJD]			*/
	double	*uvw_ptr;				/* Baseline Vector from the Obj [m]	*/
	double	*bp1_ptr;				/* Pointer of Bandpass for STN 1	*/
	double	*bp2_ptr;				/* Pointer of Bandpass for STN 2	*/
	float	*work_ptr;				/* Work Area of Visibility Data		*/
	float	*vis_r_ptr;				/* Pointer of Visibility (real) Data */
	float	*vis_i_ptr;				/* Pointer of Visibility (imag) Data */
{
	int		corunit;				/* Logical Unit Number for CORR data */
	int		flgunit;				/* Logical Unit Number for FLAG data */
	int		freq_index;
	int		bunch_index;
	int		freq_id;
	int		skip_pp;				/* SKIP NUMBER 						*/
	int		origin;					/* SKIP NUMBER						*/
	int		loop_counter;
	int		ret;					/* Return Code from CFS Library		*/
	int		first_data_flag;		/* -1:Not Yet, 0:1st Data, 1:2nd or later */
	char	omode[2];				/* CFS Access Mode					*/
	char	fname[32];				/* CFS File Name					*/
	double	uvw[3];					/* UVW in [m]						*/
	double	rel_t;					/* Relative Time [sec]				*/
	double	sum_t;					/* Summation of rel_t				*/
	double	sum_tt;					/* Summation of rel_t^2				*/
	double	sum_u, sum_v, sum_w;	/* Summation of u, v, w				*/
	double	sum_ut, sum_vt, sum_wt;	/* Summation of u*t, v*t, w*t		*/
	double	factor;					/* Normalization Factor				*/
	double	first_mjd;
	double	mjd_data;
	double	mjd_flag;
	unsigned char	flag[1024];
	int		current_obj;
	int		valid_pp;				/* How Many DATA Was Valid */
	double	bp[1024];

/*
------------------------------- INITIALIZE MEMORY AREA FOR VISIBILITIES
*/
	first_data_flag = -1;
	memset( vis_r_ptr, 0, (*freq_num_ptr)*time_num*sizeof(float)/bunch_num );
	memset( vis_i_ptr, 0, (*freq_num_ptr)*time_num*sizeof(float)/bunch_num );
/*
------------------------------- INITIALIZE MEMORY AREA FOR VISIBILITIES
*/
	for(freq_index=0; freq_index < *freq_num_ptr; freq_index++){
		bp[freq_index] = sqrt( bp1_ptr[freq_index] * bp2_ptr[freq_index] );
	}

	corunit	= CORRDATA;		flgunit = CORRFLAG;
	sprintf(omode, "r"); 

	/*-------- OPEN CORR DATA --------*/
	sprintf(fname, "CORR.%d/SS.%d/DATA.1\0", bl_index, ss_index ); 

	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&corunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- OPEN CORR DATA --------*/
	sprintf(fname, "CORR.%d/SS.%d/FLAG.1\0", bl_index, ss_index ); 

	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&flgunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- SKIP TO THE START MJD --------*/
	skip_coda( corunit, flgunit, position, time_num_cfs, start_mjd,
				*freq_num_ptr, time_incr);

	/*-------- READ VISIBILITY AND FLAG DATA --------*/
	valid_pp = 0;
	mjd_data = 0.0;
	sum_t	 = 0.0;
	sum_tt	 = 0.0;
	sum_u = 0.0;	sum_v = 0.0;	sum_w = 0.0;
	sum_ut= 0.0;	sum_vt= 0.0;	sum_wt= 0.0;
	while( (mjd_data <= stop_mjd) && ( valid_pp < time_num)){

		/*-------- LOAD VISIBILITY TO WORK AREA --------*/
		cfs235_( &corunit, &mjd_data, work_ptr, freq_num_ptr, &ret );
		cfs225_( &flgunit, &mjd_flag, &current_obj, uvw,
					flag, freq_num_ptr, &ret );

		if( SECDAY* fabs(mjd_data - mjd_flag) > time_incr* 0.5){
			printf(" DATA Error... MJD in DATA and FLAG are Different !\n"); 
			printf(" DATA MJD = %lf,  FLAG MJD = %lf!\n", mjd_data, mjd_flag); 
			return(0);
		}

/*
		if(valid_pp == 1){
			uvw_ptr[0] = uvw[0];
			uvw_ptr[1] = uvw[1];
			uvw_ptr[2] = uvw[2];
		}
*/

		/*-------- IS THIS THE TARGET SOURCE ? --------*/
		if(		(current_obj ==  obj_id)
			&&	(mjd_data >= start_mjd)
			&&	(mjd_data <= stop_mjd)	){


			if(first_data_flag == 0){ first_data_flag = 1;}
			if(first_data_flag == -1){
				first_data_flag = 0;
				first_mjd	= mjd_data;
			}

			/*-------- RELATIVE SECOND FROM THE INTEG CENTER --------*/
			rel_t = SECDAY* (mjd_data - first_mjd)
					- 0.5* time_incr* (double)time_num;
			sum_t	+= rel_t;
			sum_tt	+= (rel_t* rel_t);
			sum_u	+= uvw[0];	sum_ut	+= (rel_t* uvw[0]);
			sum_v	+= uvw[1];	sum_vt	+= (rel_t* uvw[1]);
			sum_w	+= uvw[2];	sum_wt	+= (rel_t* uvw[2]);

			/*-------- INTEGRATE VISIBILITY --------*/

			for(freq_index=0; freq_index<*freq_num_ptr/bunch_num; freq_index++){
				for(bunch_index=0; bunch_index<bunch_num; bunch_index++){
					freq_id = freq_index*bunch_num + bunch_index;

					/*-------- BANDPASS FILTER to REMOVE DC OFFSET --------*/
					if( (freq_id > LOW_EDGE) && (freq_id < HIGH_EDGE) ){
						*vis_r_ptr += (*work_ptr / bp[freq_id]);
						work_ptr++;
						*vis_i_ptr += (*work_ptr / bp[freq_id] * bl_direction);
						work_ptr++;
					} else {
						work_ptr += 2;
					}
				}
				*vis_r_ptr	/= bunch_num; vis_r_ptr++;
				*vis_i_ptr	/= bunch_num; vis_i_ptr++;
			}
			work_ptr-= 2*( *freq_num_ptr );
			valid_pp++;
		}
	}

	/*-------- FOUND NO TARGET SOURCE --------*/
	if( valid_pp == 0){
		printf(" CAUTION : TARGET SOURCE [ID=%d] WAS NOT FOUND...\n", obj_id );
		return(0);
	}

	/*-------- AVERAGE (u, v, w) --------*/
	factor = ((double)valid_pp)* sum_tt - sum_t* sum_t;
	uvw_ptr[0] = (sum_tt* sum_u - sum_t* sum_ut )/ factor;	/* u [m]		*/
	uvw_ptr[1] = (sum_tt* sum_v - sum_t* sum_vt )/ factor;	/* v [m]		*/
	uvw_ptr[2] = (sum_tt* sum_w - sum_t* sum_wt )/ factor;	/* v [m]		*/
	uvw_ptr[3] = (valid_pp* sum_ut - sum_u* sum_t)/ factor;	/* du/dt [m/s]	*/
	uvw_ptr[4] = (valid_pp* sum_vt - sum_v* sum_t)/ factor;	/* dv/dt [m/s]	*/
	uvw_ptr[5] = (valid_pp* sum_wt - sum_w* sum_t)/ factor;	/* dw/dt [m/s]	*/

	/*-------- CLOSE SS-HEADER --------*/
	cfs104_( &corunit, &ret );	cfs_ret( 104, ret );
	cfs104_( &flgunit, &ret );	cfs_ret( 104, ret );

	*freq_num_ptr /= bunch_num;
	start_mjd	= first_mjd;
	return(valid_pp);
}
