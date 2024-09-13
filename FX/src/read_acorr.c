/*********************************************************
**	INTEG_BP.C	: Integrate Bandpass Data for Each		**
**					Antenna and Each Sub-Stream			**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>

int	read_acorr( corunit, flgunit, obj_id, freq_num, integ_pp,
			stop_mjd, mjd_data_ptr, work_ptr, vis_r_ptr, vis_i_ptr,
			vis_max_ptr )

	int		corunit;				/* Logical Unit Number for CORR data */
	int		flgunit;				/* Logical Unit Number for FLAG data */
	int		obj_id;					/* Target Object ID */
	int		freq_num;				/* Number of Freq. Channels */
	int		integ_pp;				/* Integ. Number of Time */
	double	stop_mjd;				/* MJD at End of Data	*/
	double	*mjd_data_ptr;			/* Pointer of MJD in DATA */
	float	*work_ptr;				/* Pointer of Visibility Data */
	double	*vis_r_ptr;				/* Pointer of Visibility Data */
	double	*vis_i_ptr;				/* Pointer of Visibility Data */
	double	*vis_max_ptr;			/* Pointer of Max Visibility */
{
	int		time_index;				/* Index for Time */
	int		freq_index;				/* Index for Spectrum Channel */
	int		ret;					/* Return Code from CFS Library */
	char	omode[2];				/* CFS Access Mode */
	char	fname[32];				/* CFS File Name */
	double	mjd_flag;
	double	mjd_data;
	double	uvw[3];
	unsigned char	flag[1024];
	int		current_obj;
	int		spec_num;
	int		valid_pp;				/* How Many DATA Was Valid */
	int		year, doy, hour, min;
	double	sec;

	valid_pp = 0;	*mjd_data_ptr = 0.0;
	spec_num = freq_num;

	time_index = 0;
	while( time_index<integ_pp){

		cfs235_( &corunit, &mjd_data, work_ptr, &spec_num, &ret );
		cfs_ret( 235, ret );

		cfs225_( &flgunit, &mjd_flag, &current_obj, uvw,
				flag, &spec_num, &ret );
		cfs_ret( 225, ret );

		#ifdef DEBUG
		fmjd2doy( mjd_data, &year, &doy, &hour, &min, &sec);
		printf("MJD = %lf, %lf [%02d:%02d:%02d]  SOURCE ID = %d, TARGET = %d\n",
			mjd_data, mjd_flag, hour, min, (int)sec, current_obj, obj_id ); 
		#endif


		if(  current_obj != obj_id ){
			if( valid_pp == 0 ){ return(-1); }
			else { break; }
		}
		if( mjd_data > stop_mjd ) { break; }
		if( ret != 0 ){ return(-2); }

		*mjd_data_ptr = mjd_data;
		/*-------- INTEGRATE VISIBILITY --------*/
		for(freq_index=0; freq_index<freq_num; freq_index++){
			*vis_r_ptr += (double)(*work_ptr);	work_ptr++;	vis_r_ptr++;
			*vis_i_ptr += (double)(*work_ptr);	work_ptr++;	vis_i_ptr++;
		}
		work_ptr -= 2*freq_num;
		vis_r_ptr  -= freq_num;
		vis_i_ptr  -= freq_num;
		valid_pp++;
		time_index++;
	}

	/*-------- NORMALIZATOIN and PEAK SEARCH --------*/
	*vis_max_ptr = -9999.0;
	for(freq_index=0; freq_index<freq_num; freq_index++){
		*vis_r_ptr /= valid_pp;
		*vis_i_ptr /= valid_pp;
		if( *vis_r_ptr > *vis_max_ptr){
			*vis_max_ptr = *vis_r_ptr;
		}
		vis_r_ptr++;	vis_i_ptr++;
	}
	vis_r_ptr  -= freq_num;
	vis_i_ptr  -= freq_num;

	return(valid_pp);
}
