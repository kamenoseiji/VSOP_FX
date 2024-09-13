/*********************************************************
**	RESTORE_DELAY.C : Sort FCAL-DATA by time order and 	**
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

int	restore_delay( fcal_ptr, data_num,
		time_ptr_ptr, delay_ptr_ptr, rate_ptr_ptr, acc_ptr_ptr,
		delaywgt_ptr_ptr, ratewgt_ptr_ptr, accwgt_ptr_ptr )
	struct fcal_data	*fcal_ptr;		/* INPUT: Pointer of FCAL data		*/
	int		data_num;					/* INPUT: Number of FCAL Data		*/
	int		*time_ptr_ptr;				/* OUTPUT:Pointer of Time Data		*/
	int		*delay_ptr_ptr;				/* OUTPUT:Pointer of Delay Data		*/
	int		*rate_ptr_ptr;				/* OUTPUT:Pointer of Rate Data		*/
	int		*acc_ptr_ptr;				/* OUTPUT:Pointer of Acc Data		*/
	int		*delaywgt_ptr_ptr;			/* OUTPUT:Pointer of Delay Weight	*/
	int		*ratewgt_ptr_ptr;			/* OUTPUT:Pointer of Rate Weight	*/
	int		*accwgt_ptr_ptr;			/* OUTPUT:Pointer of Acc Weight		*/
{
	double	*time_ptr;					/* Pointer of Time for each Station	*/
	double	*delay_ptr;					/* Pointer of Delay for each Station*/
	double	*rate_ptr;					/* Pointer of Rate for each Station	*/
	double	*acc_ptr;					/* Pointer of Acc for each Station	*/
	double	*delaywgt_ptr;				/* Pointer of Delay Error 			*/
	double	*ratewgt_ptr;				/* Pointer of Rate Error			*/
	double	*accwgt_ptr;				/* Pointer of Acc Error				*/
	double	*tmp_time_ptr;				/* Pointer of Time for each Station	*/
	double	*tmp_delay_ptr;				/* Pointer of Delay for each Station*/
	double	*tmp_rate_ptr;				/* Pointer of Rate for each Station	*/
	double	*tmp_acc_ptr;				/* Pointer of Acc for each Station	*/
	double	*tmp_delayerr_ptr;			/* Pointer of Delay Error 			*/
	double	*tmp_rateerr_ptr;			/* Pointer of Rate Error			*/
	double	*tmp_accerr_ptr;			/* Pointer of Acc Error				*/
	double	init_mjd;					/* Day of Initial Data				*/
	int		data_index;					/* Index for Data					*/
	int		data_index2;				/* Index for Data					*/
	int		*index_ptr;					/* Index of time series to sort		*/

	/*-------- STORE DELAY DATA --------*/
	time_ptr	= (double *)malloc(data_num* sizeof(double));
	delay_ptr	= (double *)malloc(data_num* sizeof(double));
	rate_ptr	= (double *)malloc(data_num* sizeof(double));
	acc_ptr		= (double *)malloc(data_num* sizeof(double));
	delaywgt_ptr= (double *)malloc(data_num* sizeof(double));
	ratewgt_ptr	= (double *)malloc(data_num* sizeof(double));
	accwgt_ptr	= (double *)malloc(data_num* sizeof(double));
	index_ptr	= (int *)malloc( data_num* sizeof(int) );
	tmp_time_ptr	= (double *)malloc(data_num* sizeof(double));
	tmp_delay_ptr	= (double *)malloc(data_num* sizeof(double));
	tmp_rate_ptr	= (double *)malloc(data_num* sizeof(double));
	tmp_acc_ptr		= (double *)malloc(data_num* sizeof(double));
	tmp_delayerr_ptr= (double *)malloc(data_num* sizeof(double));
	tmp_rateerr_ptr	= (double *)malloc(data_num* sizeof(double));
	tmp_accerr_ptr	= (double *)malloc(data_num* sizeof(double));

	*time_ptr_ptr		= (int)time_ptr;
	*delay_ptr_ptr		= (int)delay_ptr;
	*rate_ptr_ptr		= (int)rate_ptr;
	*acc_ptr_ptr		= (int)acc_ptr;
	*delaywgt_ptr_ptr	= (int)delaywgt_ptr;
	*ratewgt_ptr_ptr	= (int)ratewgt_ptr;
	*accwgt_ptr_ptr		= (int)accwgt_ptr;

	data_index = 0;
	/*-------- FOR EACH STATION --------*/
	while( fcal_ptr != NULL ){

		/*-------- DAY OF START TIME --------*/ 
		if(data_index == 0){
			init_mjd = (double)((int)fcal_ptr->mjd);
		}

		tmp_time_ptr[data_index]	= SECDAY* (fcal_ptr->mjd - init_mjd);
		tmp_delay_ptr[data_index]	= fcal_ptr->delay;
		tmp_rate_ptr[data_index]	= fcal_ptr->rate;
		tmp_acc_ptr[data_index]		= fcal_ptr->acc;
		tmp_delayerr_ptr[data_index]= fcal_ptr->delay_err;
		tmp_rateerr_ptr[data_index]	= fcal_ptr->rate_err;
		tmp_accerr_ptr[data_index]	= fcal_ptr->acc_err;

		index_ptr[data_index]	= data_index;
		fcal_ptr = fcal_ptr->next_fcal_ptr;
		data_index ++;
	}

	/*-------- SORT by TIME ORDER --------*/
	d_index_sort( data_num, tmp_time_ptr, index_ptr );
	for( data_index=0; data_index<data_num; data_index++){
		data_index2	= index_ptr[data_index];

		time_ptr[data_index]	= tmp_time_ptr[data_index2];
		delay_ptr[data_index]	= tmp_delay_ptr[data_index2];
		rate_ptr[data_index]	= tmp_rate_ptr[data_index2];
		acc_ptr[data_index]		= tmp_acc_ptr[data_index2];

		if( tmp_delayerr_ptr[data_index2] > 0.0 ){
			delaywgt_ptr[data_index]= 1.0e-6/
				(tmp_delayerr_ptr[data_index2]* tmp_delayerr_ptr[data_index2]);
		} else {
			delaywgt_ptr[data_index]= 0.0;
		}

		if( tmp_rateerr_ptr[data_index2] > 0.0 ){
			ratewgt_ptr[data_index]	= 1.0e-12/
				(tmp_rateerr_ptr[data_index2]* tmp_rateerr_ptr[data_index2]);
		} else {
			delaywgt_ptr[data_index]= 0.0;
		}

		if( tmp_accerr_ptr[data_index2] > 0.0 ){
			accwgt_ptr[data_index]	= 1.0e-15/
				(tmp_accerr_ptr[data_index2]* tmp_accerr_ptr[data_index2]);
		} else {
			accwgt_ptr[data_index]	= 0.0;
		}
	}

	/*-------- RELEASE WORK AREA --------*/
	free( index_ptr );
	free( tmp_time_ptr );
	free( tmp_delay_ptr );
	free( tmp_rate_ptr );
	free( tmp_accerr_ptr );
	free( tmp_delayerr_ptr );
	free( tmp_rateerr_ptr );
	free( tmp_accerr_ptr );

	return(0);
}
