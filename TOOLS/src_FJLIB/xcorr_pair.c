/*********************************************************
**	READ_CODE.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include "obshead.inc"

int	xcorr_pair( obs_ptr, first_cor_ptr )

	struct	header		*obs_ptr;		/* Pointer of OBS HEADER */
	struct	head_cor	*first_cor_ptr;	/* Pointer of CORR Header */
{
	struct	head_cor	*cor_ptr;		/* Pointer of CORR Header */
	struct	head_cor	*prev_cor_ptr;	/* Pointer of CORR Header */
	int		cor_index;					/* Index for Correlation Pair */
	int		ant_id[2];					/* Antenna ID Number */
	int		ssnum;						/* Number of Sub-Stream */
	double	loss;

	#ifdef DEBUG
	printf("TOTAL %d CORRELATION PAIRS \n", obs_ptr->cor_num);
	printf("TOTAL %d STATIONS (include dummy)\n", obs_ptr->stn_num);
	#endif

	/*-------- READ CORR-HEAD --------*/
	for( cor_index=0; cor_index < obs_ptr->cor_num; cor_index++){
		if(cor_index == 0){
			cor_ptr		= first_cor_ptr;
			prev_cor_ptr = first_cor_ptr;
		} else {
			cor_ptr	= (struct head_cor *)malloc( sizeof(struct head_cor));
		}

		/*-------- LINK STN-ID to AUTOCORR PAIR ID --------*/
		read_corhead( cor_index+1, ant_id, &ssnum, &loss );

		cor_ptr->cor_id	= cor_index+1;
		cor_ptr->ant1	= ant_id[0];
		cor_ptr->ant2	= ant_id[1];
		cor_ptr->nss	= ssnum;
		cor_ptr->loss	= loss;

		prev_cor_ptr->next_cor_ptr	= cor_ptr;
		prev_cor_ptr				= cor_ptr;
		cor_ptr->next_cor_ptr		= NULL;
	}
		
	return(0);
}
