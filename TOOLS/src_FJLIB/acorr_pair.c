/*********************************************************
**	READ_CODE.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include "obshead.inc"

int	acorr_pair( obs_ptr, first_stn_ptr, ssnum_ptr, loss_ptr )
	struct	header		*obs_ptr;		/* Pointer of OBS HEADER */
	struct	head_stn	*first_stn_ptr;	/* Pointer of Station Header */
	int					*ssnum_ptr;		/* Pointer of Total SS Number */
	double				*loss_ptr;		/* Pointer of Corr. Efficiency */
{
	struct	head_stn	*stn_ptr;		/* Initial Pointer of Station Header */
	int		cor_index;					/* Index for Correlation Pair */
	int		ant_id[2];					/* Antenna ID Number */

	#ifdef DEBUG
	printf("TOTAL %d CORRELATION PAIRS \n", obs_ptr->cor_num);
	printf("TOTAL %d STATIONS (include dummy)\n", obs_ptr->stn_num);
	#endif

	obs_ptr->real_stn_num = 0;
	/*-------- READ CORR-HEAD --------*/
	for( cor_index=0; cor_index < obs_ptr->cor_num; cor_index++){

		/*-------- LINK STN-ID to AUTOCORR PAIR ID --------*/
		read_corhead( cor_index+1, ant_id, ssnum_ptr, loss_ptr );
		if(ant_id[0] == ant_id[1]){
			obs_ptr->real_stn_num++;
			stn_ptr = first_stn_ptr;
			while( stn_ptr != NULL){
				if( stn_ptr->stn_index == ant_id[0] ){
					stn_ptr->acorr_index = cor_index+1;
					break;
				}
				stn_ptr = stn_ptr->next_stn_ptr;
			}
		}
	}
	#ifdef DEBUG
	printf("TOTAL %d REAL STATIONS\n", obs_ptr->real_stn_num);
	#endif
	return(obs_ptr->real_stn_num);
}
