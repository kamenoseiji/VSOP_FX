/*********************************************************
**	INTEG_RAWBP.C	: Integrate Bandpass Data for Each	**
**					Antenna and Each Sub-Stream			**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#define	CORRDATA	4

int	integ_rawbp( cor_id, ss_index, freq_num, start_pp, integ_pp,
			work_ptr, vis_r_ptr, vis_i_ptr, vis_max_ptr )

	int		cor_id;					/* Baseline Index				*/
	int		ss_index;				/* Sub-Stream Index				*/
	int		freq_num;				/* Number of Freq. Channels		*/
	int		start_pp;				/* INTEG START RECORD			*/
	int		integ_pp;				/* INTEG NUMBER of RECORDS		*/
	float	*work_ptr;				/* Pointer of Visibility Data	*/
	double	*vis_r_ptr;				/* Pointer of Visibility Data	*/
	double	*vis_i_ptr;				/* Pointer of Visibility Data	*/
	double	*vis_max_ptr;			/* Pointer of Max Visibility	*/
{
	int		corunit;				/* Logical Unit Number for CORR data */
	int		ret;					/* Return Code from CFS Library		*/
	char	omode[2];				/* CFS Access Mode					*/
	char	fname[32];				/* CFS File Name					*/
	int		spec_num;				/* Total Number of Spectral Channel	*/
	int		freq_index;				/* Index for Crequency Channel		*/
	int		valid_pp;				/* How Many DATA Was Valid			*/
	double	mjd_data;				/* MJD in CFS Data Record			*/

	corunit	= CORRDATA;
	sprintf(omode, "r\0"); 

	/*-------- OPEN CORR DATA --------*/
	memset(fname, 0, 32);
	sprintf(fname, "CORR.%d/SS.%d/DATA.1\0", cor_id, ss_index ); 

#ifdef DEBUG
	printf("INTEG_RAWBP: Try to Open %s.\n", fname ); 
	printf("INTEG_RAWBP: COR_ID   = %d\n", cor_id ); 
	printf("INTEG_RAWBP: SS_INDEX = %d\n", ss_index ); 
	printf("INTEG_RAWBP: FREQ_NUM = %d\n", freq_num ); 
	printf("INTEG_RAWBP: START_PP = %d\n", start_pp ); 
	printf("INTEG_RAWBP: INTEG_PP = %d\n", integ_pp ); 
#endif

	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&corunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	valid_pp = 0;	mjd_data = 0.0;
	spec_num = freq_num;
	while(valid_pp < integ_pp){

	#ifdef DEBUG
	printf("CURRENT MJD IS %lf\n", mjd_data ); 
	#endif

		cfs235_( &corunit, &mjd_data, work_ptr, &spec_num, &ret );
		cfs_ret( 235, ret );

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

	if(valid_pp == 0){ return(-1); }

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

#ifdef DEBUG
	for(freq_index=0; freq_index<freq_num; freq_index++){
		printf("INTEG_RAWBP: SPEC[%03d] = %e\n",
			freq_index, vis_r_ptr[freq_index] );
	}
#endif

	/*-------- CLOSE SS-HEADER --------*/
	cfs104_( &corunit, &ret );	cfs_ret( 104, ret );

	return(valid_pp);
}
