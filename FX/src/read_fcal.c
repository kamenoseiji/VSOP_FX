/*********************************************************
**	PLOT_DELAY.C :	Plot Antenna-Based Delay Data		**
**														**
**	AUTHOR  : KAMENO Seiji								**
**	CREATED : 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>
#include "obshead.inc"

read_fcal(	stn_id, fcalptr, epoch_ptr,
			delay_0_ptr, delay_1_ptr, rate_0_ptr, rate_1_ptr )

	int		stn_id;						/* Station ID */
	int		*fcalptr;					/* Pointer of CLOCK data */
	double	*epoch_ptr;					/* Clock Epoch [MJD] */
	double	*delay_0_ptr;				/* Clock Offset at the Epoch */
	double	*delay_1_ptr;				/* Clock Offset increment per time */
	double	*rate_0_ptr;				/* Clock Rate */
	double	*rate_1_ptr;				/* Clock Rate increment per time */
{
	struct	fcal_data	*fcal_ptr;		/* Pointer of Clock Data */
	double	mjd_min,	mjd_max;		/* Max and Min of MJD */
	double	rate_min,	rate_max;		/* Max and Min of Rate */
	double	delay_min,	delay_max;		/* Max and Min of Delay */
	double	acc_min,	acc_max;		/* Max and Min of Acc */
	double	x_data;
	double	sum_x,	sum_xx,	sum_y,	sum_z,	sum_xy,	sum_xz;
	double	weight, sum_weight;
/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	printf(" STATION ID = %d\n", stn_id );
	mjd_max= -99999.0;		mjd_min=  999999.0;
	rate_max= -99999.0;		rate_min=  999999.0;
	delay_max= -99999.0;	delay_min=  999999.0;
	acc_max= -99999.0;		acc_min=  999999.0;

	if( read_delay( stn_id, fcalptr,	&mjd_min,	&mjd_max,
		&rate_min,	&rate_max,	&delay_min,	&delay_max,
		&acc_min,	&acc_max) == -1){
		return(0);
	}

	fcal_ptr	= (struct fcal_data *)*fcalptr;

	sum_x = 0.0;	sum_xx = 0.0;	sum_y = 0.0;	sum_z = 0.0;
	sum_xy= 0.0;	sum_xz = 0.0;	sum_weight = 0.0;

	while(fcal_ptr != NULL){

		weight	=  fcal_ptr->delay_err * fcal_ptr->rate_err; 
		if( weight > 0.0 ){
			weight	= 1.0/weight;
		}

		x_data	= fcal_ptr->mjd - mjd_min;
		sum_x	+= (x_data * weight);
		sum_xx	+= (x_data * x_data * weight);
		sum_xy	+= (x_data * (fcal_ptr->delay) * weight);
		sum_y	+= (fcal_ptr->delay * weight);
		sum_xz	+= (x_data * fcal_ptr->rate * weight);
		sum_z	+= (fcal_ptr->rate * weight);
		sum_weight += weight;
		fcal_ptr = fcal_ptr->next_fcal_ptr;
	}

	*epoch_ptr	= mjd_min;
	*delay_1_ptr= (sum_weight*sum_xy - sum_x*sum_y)
				/ (sum_weight*sum_xx - sum_x*sum_x);
	*delay_0_ptr= (sum_xx*sum_y - sum_x*sum_xy)
				/ (sum_weight*sum_xx - sum_x*sum_x);
	*rate_1_ptr = (sum_weight*sum_xz - sum_x*sum_z)
				/ (sum_weight*sum_xx - sum_x*sum_x);
	*rate_0_ptr = (sum_xx*sum_z - sum_x*sum_xz)
				/ (sum_weight*sum_xx - sum_x*sum_x);

	return(0);
}
