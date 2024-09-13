/*********************************************************
**	RESTORE_PCAL.C : Sort PCAL-DATA by time order and 	**
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

int	restore_pcal(pcal_ptr, data_num, ss_id, ref_ss, time_ptr_ptr,
				pcr_ptr_ptr, pci_ptr_ptr, wgt_ptr_ptr)
	struct pcal_data	*pcal_ptr;		/* INPUT: Pointer of PCAL data		*/
	int		data_num;					/* INPUT: Number of PCAL Data		*/
	int		ss_id;						/* INPUT: Index of Sub-Stream		*/
	int		ref_ss;						/* INPUT: Reference Sub-Stream		*/
	int		*time_ptr_ptr;				/* OUTPUT:Pointer of Time Data		*/
	int		*pcr_ptr_ptr;				/* OUTPUT:Pointer of PCAL Real		*/
	int		*pci_ptr_ptr;				/* OUTPUT:Pointer of PCAL Imag		*/
	int		*wgt_ptr_ptr;				/* OUTPUT:Pointer of Weight Data	*/
{
	double	*time_ptr;					/* Pointer of Time for each Station	*/
	double	*pcr_ptr;					/* Pointer of PCAL Real 			*/
	double	*pci_ptr;					/* Pointer of PCAL Imag 			*/
	double	*wgt_ptr;					/* Pointer of Pcal Phase Error		*/
	double	*tmp_time_ptr;				/* Pointer of Time for each Station	*/
	double	*tmp_pc_ptr;				/* Pointer of Phase for each Stn	*/
	double	*tmp_wgt_ptr;				/* Pointer of Phase Error 			*/
	double	init_mjd;					/* Day of Initial Data				*/
	int		data_index;					/* Index for Data					*/
	int		data_index2;				/* Index for Data					*/
	int		*index_ptr;					/* Index of time series to sort		*/


	/*-------- STORE DELAY DATA --------*/
	time_ptr	= (double *)malloc(data_num* sizeof(double));
	pcr_ptr		= (double *)malloc(data_num* sizeof(double));
	pci_ptr		= (double *)malloc(data_num* sizeof(double));
	wgt_ptr		= (double *)malloc(data_num* sizeof(double));
	index_ptr	= (int *)malloc( data_num* sizeof(int) );

	tmp_time_ptr	= (double *)malloc(data_num* sizeof(double));
	tmp_pc_ptr		= (double *)malloc(data_num* sizeof(double));
	tmp_wgt_ptr		= (double *)malloc(data_num* sizeof(double));

	*time_ptr_ptr		= (int)time_ptr;
	*pcr_ptr_ptr		= (int)pcr_ptr;
	*pci_ptr_ptr		= (int)pci_ptr;
	*wgt_ptr_ptr		= (int)wgt_ptr;

	data_index = 0;
	/*-------- FOR EACH STATION --------*/
	while( pcal_ptr != NULL ){

		/*-------- DAY OF START TIME --------*/ 
		if(data_index == 0){
			init_mjd = (double)((int)pcal_ptr->mjd);
		}

		tmp_time_ptr[data_index]	= SECDAY* (pcal_ptr->mjd - init_mjd);
		tmp_pc_ptr[data_index]		= pcal_ptr->phs[ss_id]
									- pcal_ptr->phs[ref_ss];
		if( pcal_ptr->err[ss_id] == 0.0 ){
			tmp_wgt_ptr[data_index]	= 1.0e-03;
		} else {
			tmp_wgt_ptr[data_index]		= 1.0 /
				(pcal_ptr->err[ss_id] * pcal_ptr->err[ss_id]);
		}

		index_ptr[data_index]		= data_index;

		pcal_ptr = pcal_ptr->next_pcal_ptr;
		data_index ++;
	}

	/*-------- SORT by TIME ORDER --------*/
	d_index_sort( data_num, tmp_time_ptr, index_ptr );
	for( data_index=0; data_index<data_num; data_index++){
		data_index2	= index_ptr[data_index];

		time_ptr[data_index]	= tmp_time_ptr[data_index2];
		pcr_ptr[data_index]		= cos( tmp_pc_ptr[data_index2] );
		pci_ptr[data_index]		= sin( tmp_pc_ptr[data_index2] );
		wgt_ptr[data_index]		= tmp_wgt_ptr[data_index2];
	}

	/*-------- RELEASE WORK AREA --------*/
	free( index_ptr );
	free( tmp_time_ptr );
	free( tmp_pc_ptr );
	free( tmp_wgt_ptr );
	return(0);
}
