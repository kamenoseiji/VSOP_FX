/*********************************************************
**	INTEG_BP_FLAG.C	: Integrate Bandpass Data for Each	**
**					Antenna and Each Sub-Stream			**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include "aoslog.inc"
#include <math.h>
#define	CORRDATA	4
#define	CORRFLAG	5
#define MAX(a,b)    ( (a>b)?a:b )

int	integ_bp_flag(
	int		scan_type,				/* Scan Type to Integrate		*/
	int		first_scan,				/* Address to the AOS Scans		*/
	int		cor_id,					/* Baseline Index				*/
	int		obj_id,					/* Object ID Number				*/
	int		ss_index,				/* Sub-Stream Index				*/
	int		freq_num,				/* Number of Freq. Channels		*/
	double	start_mjd,				/* INTEG START TIME [MJD]		*/
	double	stop_mjd,				/* INTEG STOP TIME [MJD]		*/
	int		*position,				/* Start PP Position			*/
	int		time_num_cfs,			/* Number of Time in CFS		*/
	double	time_incr,				/* INTEG TIME [SEC]				*/
	float	*work_ptr,				/* Pointer of Visibility Data	*/
	double	*vis_r_ptr,				/* Pointer of Visibility Data	*/
	double	*vis_i_ptr,				/* Pointer of Visibility Data	*/
	double	*vis_max_ptr)			/* Pointer of Max Visibility	*/
{
	int		corunit;				/* Logical Unit Number for CORR data */
	int		flgunit;				/* Logical Unit Number for FLAG data */
	int		time_index;
	int		freq_index;
	int		current_scan_type;		/* AOS Scan Type				*/
	int		ret;					/* Return Code from CFS Library */
	char	omode[2];				/* CFS Access Mode */
	char	fname[32];				/* CFS File Name */
	double	mjd_data;
	double	mjd_flag;
	double	amp_avg;				/* Average Amplitude in midele range	*/
	int		range_start;			/* Freqency at Sampling Start 			*/
	int		range_stop;				/* Freqency at Sampling Start 			*/
	int		num_range;				/* Freq. channels in the range			*/
	double	amp_thresh;				/* Amplitude Validity THreshold			*/
	double	uvw[3];
	unsigned char	flag[1024];
	int		current_obj;
	int		spec_num;
	int		valid_pp;				/* How Many DATA Was Valid */

	corunit	= CORRDATA;		flgunit = CORRFLAG;
	sprintf(omode, "r"); 

	/*-------- OPEN CORR DATA --------*/
	sprintf(fname, "CORR.%d/SS.%d/DATA.1 \0", cor_id, ss_index ); 
	#ifdef DEBUG
	printf("%s\n", fname ); 
	#endif
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&corunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- OPEN CORR DATA --------*/
	sprintf(fname, "CORR.%d/SS.%d/FLAG.1 \0", cor_id, ss_index ); 
	#ifdef DEBUG
	printf("%s\n : %d Bytes", fname, strlen(fname), strlen(omode) ); 
	#endif
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&flgunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- READ VISIBILITY AND FLAG DATA --------*/

	skip_coda( corunit, flgunit, position, time_num_cfs, start_mjd,
				freq_num, time_incr);

	valid_pp = 0;	mjd_data = 0.0;
	spec_num = freq_num;
	range_start = 0.2* freq_num;
	range_stop  = 0.8* freq_num;
	num_range   = range_stop - range_start + 1;
	amp_thresh  = sqrt((double)spec_num);
	while(mjd_data <= stop_mjd){

		cfs235_( &corunit, &mjd_data, work_ptr, &spec_num, &ret );
		cfs_ret( 235, ret );

		cfs225_( &flgunit, &mjd_flag, &current_obj, uvw,
					flag, &spec_num, &ret );
		cfs_ret( 225, ret );
		current_scan_type = detect_scan(first_scan, mjd_data);

		/*-------- Automated Flagging --------*/
		amp_avg = 0.0;
		for(freq_index=range_start; freq_index<range_stop; freq_index++){
			amp_avg += work_ptr[freq_index* 2];
		}
		amp_avg /= ((double)num_range);
		printf("AMP_SVG = %lf,  THRESH = %lf\n", amp_avg, amp_thresh);

		/*-------- IS THIS THE TARGET SOURCE ? --------*/
		if(	(current_obj ==  obj_id)
			&& 	(mjd_data >= start_mjd)
			&&	(mjd_data <= stop_mjd)
			&&	(amp_avg < (amp_thresh * 1.4))
			&&	(amp_avg > (amp_thresh * 0.7))
			&&  (current_scan_type == scan_type)){

			/*-------- INTEGRATE VISIBILITY --------*/
			for(freq_index=0; freq_index<freq_num; freq_index++){
				*vis_r_ptr += (double)(*work_ptr);	work_ptr++;	vis_r_ptr++;
				*vis_i_ptr += (double)(*work_ptr);	work_ptr++;	vis_i_ptr++;
			}
			work_ptr -= 2*freq_num;
			vis_r_ptr  -= freq_num;
			vis_i_ptr  -= freq_num;
			valid_pp++;
		}
	}

	/*-------- FOUND NO TARGET SOURCE --------*/
	if( valid_pp == 0){
		printf(" CAUTION : TARGET SOURCE [ID=%d] WAS NOT FOUND...\n", obj_id );
		return(0);
	}

	/*-------- NORMALIZATOIN and PEAK SEARCH --------*/
	for(freq_index=0; freq_index<freq_num; freq_index++){
		*vis_r_ptr /= valid_pp;
		*vis_i_ptr /= valid_pp;
		vis_r_ptr++;	vis_i_ptr++;
	}
	vis_r_ptr  -= freq_num;
	vis_i_ptr  -= freq_num;

	*vis_max_ptr = -9999.0;
	for(freq_index=(0.2* freq_num); freq_index<(0.8* freq_num); freq_index++){
		*vis_max_ptr = MAX( *vis_max_ptr, vis_r_ptr[freq_index]);
	}

	/*-------- CLOSE SS-HEADER --------*/
	cfs104_( &corunit, &ret );	cfs_ret( 104, ret );
	cfs104_( &flgunit, &ret );	cfs_ret( 104, ret );

	return(valid_pp);
}
