/*********************************************************
**	BL_SEARCH.C	: Baseline_Based Fringe Search			**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>
#include "obshead.inc"
#define	SNR_LIMIT 7.0
#define	PI		3.141592653589793238

int	bl_search( visamp_ptr, ssnum, bandwidth, integ_time, rf, wind_x, wind_y,
		bl_delay_ptr, bl_delay_err, bl_rate_ptr, bl_rate_err,
		vismax_ptr, vis_snr_ptr)

	float	*visamp_ptr;				/* Pointer of Visibility Amplitude	*/
	int		ssnum;						/* Number of Sub-Stream				*/
	double	bandwidth;					/* Bandwidth [MHz]					*/
	double	integ_time;					/* integ_time [sec]					*/
	double	rf;							/* RF Frequency [MHz]				*/
	int		wind_x;						/* Window Range (X, Y)				*/
	int		wind_y;						/* Window Range (X, Y)				*/
	double	*bl_delay_ptr;				/* Residual Delay (BL-Based)		*/
	double	*bl_delay_err;				/* Residual Delay Error				*/
	double	*bl_rate_ptr;				/* Residual Delay Rate (BL-Based)	*/
	double	*bl_rate_err;				/* Residual Rate Error				*/
	float	*vismax_ptr;				/* Maximum Visibility Amp			*/
	float	*vis_snr_ptr;				/* SNR of Visivility				*/
{

	/*-------- ID Number --------*/
	int		delay_pos;					/* Peak Position for Delay			*/ 
	int		rate_pos;					/* Peak Position for Rate			*/ 

	/*-------- INDEX Number --------*/
	int		x_index;					/* Index for Window X-Axis			*/
	int		y_index;					/* Index for Window Y-Axis			*/

	/*-------- GENERAL VARIABLES --------*/
	double	coeff[5];					/* Square-Fit Coefficient			*/


	/*-------- PEAK SEARCH (GRID) --------*/
	*vismax_ptr	= -9999.0;
	for(y_index=0; y_index<wind_y; y_index++){
		for(x_index=0; x_index<wind_x; x_index++){

			visamp_ptr[y_index* wind_x + x_index] /= (float)ssnum;
			if( visamp_ptr[y_index* wind_x + x_index] > *vismax_ptr ){
				*vismax_ptr		= visamp_ptr[y_index* wind_x + x_index];
				delay_pos	= x_index - wind_x/2;
				rate_pos	= y_index - wind_y/2;
			}
		}
	}

	/*-------- PEAK SEARCH (SQURE FIT) --------*/
	sqr_fit((double)(delay_pos-1),
			(double)(rate_pos),
			(double)visamp_ptr[wind_x*(rate_pos+wind_y/2)
					+delay_pos+wind_x/2-1],

			(double)(delay_pos),
			(double)(rate_pos),
			(double)visamp_ptr[wind_x*(rate_pos+wind_y/2)
					+delay_pos+wind_x/2],

			(double)(delay_pos+1),
			(double)(rate_pos),
			(double)visamp_ptr[wind_x*(rate_pos+wind_y/2)
					+delay_pos+wind_x/2+1],

			(double)(delay_pos),
			(double)(rate_pos-1),
			(double)visamp_ptr[wind_x*(rate_pos+wind_y/2-1)
					+delay_pos+wind_x/2],

			(double)(delay_pos),
			(double)(rate_pos+1),
			(double)visamp_ptr[wind_x*(rate_pos+wind_y/2+1)
					+delay_pos+wind_x/2],
		coeff);

	/*-------- SQURE FIT RESULTS --------*/
	*vismax_ptr	= -(coeff[2]*coeff[2]/coeff[0]+coeff[3]
			* coeff[3]/coeff[1])/4 + coeff[4];

	*bl_delay_ptr	= -coeff[2]/(coeff[0]* bandwidth* 4);
	*bl_rate_ptr	= -coeff[3]/(coeff[1]
					* (rf + bandwidth / 2) * integ_time* 4);


	*vis_snr_ptr	= *vismax_ptr*sqrt(2.0* integ_time
			* bandwidth* 1.0e6* sqrt(ssnum));

	*bl_delay_err	= 1.0/(2.0* bandwidth* (*vis_snr_ptr));
	*bl_rate_err	= 1.0/(rf* integ_time* (*vis_snr_ptr));

	if( *vis_snr_ptr < SNR_LIMIT ){
		*bl_delay_err	*= 100.0;	*bl_rate_err	*= 100.0;
	}

	printf("VISMAX= %f at delay=%8.5lf rate=%8.5lf (SNR = %7.2f)\n",
		*vismax_ptr, *bl_delay_ptr, *bl_rate_ptr*1.0e6, *vis_snr_ptr );

	return(0);
}
