/*********************************************************
**	CP_VIS.C: (REAL, IMAG) -> (AMP, PHASE)				**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define	PI2	6.28318530717958647692

int	cp_vis( rf, freq_incr, d_time, delay, rate, freq_num,
		org_real_ptr, org_imag_ptr,
		dest_amp_ptr, dest_phs_ptr, vis_max_ptr, index_max )

	double	rf;					/* RF Frequency [MHz]			*/
	double	freq_incr;			/* Frequency Increment [MHz]	*/
	double	d_time;				/* Relative Time [sec]			*/
	double	delay;				/* Delay [microsec]				*/
	double	rate;				/* Rate [microsec/sec]			*/
	int		freq_num;			/* Number of Frequency			*/
	float	*org_real_ptr;		/* Original Visibility [real]	*/
	float	*org_imag_ptr;		/* Original Visibility [imag]	*/
	float	*dest_amp_ptr;		/* Visibility Amplitude			*/
	float	*dest_phs_ptr;		/* Visibility Phase				*/
	float	*vis_max_ptr;		/* Visibility Maximum			*/
	int		*index_max;			/* Freq CH at Maximum			*/
{
	int		freq_index;			/* Frequency Index				*/
	double	omega;				/* Angular Frequency			*/
	double	d_omega;			/* Relative Angular Frequency	*/
	double	phase;				/* Phase to correct				*/
	double	cs, sn;				/* cos(phase) and sin(phase)	*/
	double	vis_r, vis_i;		/* Temporal visibility			*/

	for(freq_index=0; freq_index<freq_num; freq_index++){

		omega	= PI2* (rf + freq_incr* freq_num / 2 );
		d_omega	= PI2* freq_incr* (freq_index - freq_num / 2);
		phase	= d_omega*delay + omega* rate* d_time;

		cs		= cos(phase);
		sn		= sin(phase);

		vis_r	= cs* org_real_ptr[freq_index] + sn* org_imag_ptr[freq_index];
		vis_i	=-sn* org_real_ptr[freq_index] + cs* org_imag_ptr[freq_index];

		dest_amp_ptr[freq_index] = (float)sqrt( vis_r* vis_r + vis_i* vis_i);
		dest_phs_ptr[freq_index] = (float)atan2( vis_i, vis_r);
	}

	/*-------- MAX SEARCH --------*/
	if( *vis_max_ptr <= 0.0){
		for(freq_index=0; freq_index<freq_num; freq_index++){
			if(dest_amp_ptr[freq_index] > *vis_max_ptr){
				*vis_max_ptr = dest_amp_ptr[freq_index];
				*index_max	= freq_index;
			}
		}
	}

	return(freq_num);
}
