/*********************************************************
**	RESTORE_GAIN.C : Sort GCAL-DATA by time order and 	**
**					Store into memory					**
**														**
**	AUTHOR  : KAMENO Seiji								**
**	CREATED : 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"
#define SECDAY	86400

int	restore_gain(gcal_ptr, data_num, time_ptr_ptr, gain_ptr_ptr, wgt_ptr_ptr)
	struct gcal_data	*gcal_ptr;		/* INPUT: Pointer of GCAL data		*/
	int		data_num;					/* INPUT: Number of GCAL Data		*/
	int		*time_ptr_ptr;				/* OUTPUT:Pointer of Time Data		*/
	int		*gain_ptr_ptr;				/* OUTPUT:Pointer of Gain Data		*/
	int		*wgt_ptr_ptr;				/* OUTPUT:Pointer of Weight Data	*/
{
	double	*time_ptr;					/* Pointer of Time for each Station	*/
	double	*gain_ptr;					/* Pointer of Gain for each Station	*/
	double	*wgt_ptr;					/* Pointer of Gain Error			*/
	double	*tmp_time_ptr;				/* Pointer of Time for each Station	*/
	double	*tmp_gain_ptr;				/* Pointer of Gain for each Station	*/
	double	*tmp_wgt_ptr;				/* Pointer of Gain Error 			*/
	double	init_mjd;					/* Day of Initial Data				*/
	int		data_index;					/* Index for Data					*/
	int		data_index2;				/* Index for Data					*/
	int		*index_ptr;					/* Index of time series to sort		*/

	/*-------- STORE DELAY DATA --------*/
	time_ptr	= (double *)malloc(data_num* sizeof(double));
	gain_ptr	= (double *)malloc(data_num* sizeof(double));
	wgt_ptr		= (double *)malloc(data_num* sizeof(double));
	index_ptr	= (int *)malloc( data_num* sizeof(int) );

	tmp_time_ptr	= (double *)malloc(data_num* sizeof(double));
	tmp_gain_ptr	= (double *)malloc(data_num* sizeof(double));
	tmp_wgt_ptr		= (double *)malloc(data_num* sizeof(double));

	*time_ptr_ptr		= (int)time_ptr;
	*gain_ptr_ptr		= (int)gain_ptr;
	*wgt_ptr_ptr		= (int)wgt_ptr;

	data_index = 0;
	/*-------- FOR EACH STATION --------*/
	while( gcal_ptr != NULL ){

		/*-------- DAY OF START TIME --------*/ 
		if(data_index == 0){
			init_mjd = (double)((int)gcal_ptr->mjd);
		}

		tmp_time_ptr[data_index]	= SECDAY* (gcal_ptr->mjd - init_mjd);
		tmp_gain_ptr[data_index]	= gcal_ptr->real;
		tmp_wgt_ptr[data_index]		= gcal_ptr->weight;
		index_ptr[data_index]		= data_index;

		gcal_ptr = gcal_ptr->next_gcal_ptr;
		data_index ++;
	}

	/*-------- SORT by TIME ORDER --------*/
	d_index_sort( data_num, tmp_time_ptr, index_ptr );
	for( data_index=0; data_index<data_num; data_index++){
		data_index2	= index_ptr[data_index];

		time_ptr[data_index]	= tmp_time_ptr[data_index2];
		gain_ptr[data_index]	= tmp_gain_ptr[data_index2];
		wgt_ptr[data_index]		= tmp_wgt_ptr[data_index2];
	}

	/*-------- RELEASE WORK AREA --------*/
	free( index_ptr );
	free( tmp_time_ptr );
	free( tmp_gain_ptr );
	free( tmp_wgt_ptr );
	return(0);
}
