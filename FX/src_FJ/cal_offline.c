/*********************************************************
**	CAL_OFFLINE.C : Scaling for Reference Antenna Gain	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>

int	cal_offline( freq_num, rf, freq_incr, xpos_ptr,
				vis_r_ptr, vis_i_ptr, bp_r_ptr, bp_i_ptr, offline_ptr)
	int		freq_num;			/* Number of Spectral Points	*/
	double	rf;					/* Initial Frequency [MHz]		*/
	double	freq_incr;			/* Frequency Increment [MHz]	*/
	double	*xpos_ptr;			/* Selected Spectral Position	*/
	double	*vis_r_ptr;			/* Pointer of Visibility (Real)	*/
	double	*vis_i_ptr;			/* Pointer of Visibility (Imag)	*/
	double	*bp_r_ptr;			/* Pointer of Bandpass (Real)	*/
	double	*bp_i_ptr;			/* Pointer of Bandpass (Imag)	*/
	double	*offline_ptr;		/* Tsys Ratio Between BP and Vis*/
{
	int		freq_index;			/* Frequency Index				*/
	double	freq;				/* Frequency [MHz]				*/
	double	vis_offline;		/* Tsys Level 					*/
	double	bp_offline;			/* Tsys Level in Bandpass		*/

/*
------------------------------- INITIALIZE CUMULATIVE VARIABLES
*/
	freq		= rf;
	vis_offline	= 0.0;
	bp_offline	= 0.0;
/*
------------------------------- AVERAGE TSYS LEVEL IN OFFLINE
*/
	for(freq_index=0; freq_index<freq_num; freq_index++){

		if(    ( (freq >= xpos_ptr[0]) && (freq <= xpos_ptr[1]) )
			|| ( (freq >= xpos_ptr[4]) && (freq <= xpos_ptr[5]) )	){

			vis_offline	+= sqrt((*vis_r_ptr)*(*vis_r_ptr) + 
								(*vis_i_ptr)*(*vis_i_ptr) ); 

			bp_offline	+= sqrt((*bp_r_ptr)*(*bp_r_ptr) + 
								(*bp_i_ptr)*(*bp_i_ptr) ); 

		}
		vis_r_ptr++;	vis_i_ptr++;	bp_r_ptr++;	bp_i_ptr++;
		freq	+= freq_incr;
	}
/*
------------------------------- CALC. TSYS RATIO
*/
	*offline_ptr	= bp_offline/vis_offline;
	return(0);
}
