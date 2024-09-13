/*********************************************************
**	AVE_BANDPASS.C: Test Module to Read CODA File System**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>
#define	CORRDATA	4
#define	CORRFLAG	5

int	ave_bandpass( freq_num, bpr1_ptr, bpi1_ptr,  bpr2_ptr, bpi2_ptr,
					ave_bp_r, ave_bp_i )

	double	*bpr1_ptr;				/* Pointer of Bandpass (real) for STN 1	*/
	double	*bpi1_ptr;				/* Pointer of Bandpass (imag) for STN 1	*/
	double	*bpr2_ptr;				/* Pointer of Bandpass (real) for STN 2	*/
	double	*bpi2_ptr;				/* Pointer of Bandpass (imag) for STN 2	*/
	double	*ave_bp_r;				/* Pointer of Averaged BP Data			*/
	double	*ave_bp_i;				/* Pointer of Averaged BP Data			*/
{
	int		freq_index;				/* Frequency Index						*/
	double	power;
/*
------------------------------- INITIALIZE MEMORY AREA FOR VISIBILITIES
*/
	for(freq_index=0; freq_index < freq_num; freq_index++){
		power	= sqrt( bpr1_ptr[freq_index]*bpr1_ptr[freq_index]
					  + bpi1_ptr[freq_index]*bpi1_ptr[freq_index] )
				* sqrt( bpr2_ptr[freq_index]*bpr2_ptr[freq_index]
					  + bpi2_ptr[freq_index]*bpi2_ptr[freq_index] );
		power = sqrt( power );

		ave_bp_r[freq_index] = (bpr1_ptr[freq_index]* bpr2_ptr[freq_index]
							 +  bpi1_ptr[freq_index]* bpi2_ptr[freq_index])
							 / power;

		ave_bp_i[freq_index] = (bpr2_ptr[freq_index]* bpi1_ptr[freq_index]
							 -  bpr1_ptr[freq_index]* bpi2_ptr[freq_index])
							 / power;
	}
	return(freq_num);
}
