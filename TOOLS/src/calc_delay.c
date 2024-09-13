/*********************************************************
**	CALC_DELAY.C : Calc Station-Based Residual Delay	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>
#include "obshead.inc"

int	calc_delay( obs_ptr, obj_ptr, first_stn_ptr, antnum, stn_num, ssnum,
			refant_id, stnid_in_cfs, atm_prm, mjd, gff_result )

	struct	header		*obs_ptr;		/* Pointer of OBS HEADDER		*/
	struct	head_obj	*obj_ptr;		/* Pointer of Objects Header	*/
	struct	head_stn	*first_stn_ptr;	/* Pointer of Station Header	*/
	int		antnum;						/* Number of ANT to Solve		*/
	int		stn_num;					/* Total Number of ANT in CFS	*/
	int		ssnum;						/* Number of Sub-Stream			*/
	int		refant_id;					/* Station ID of REF ANT		*/
	int		*stnid_in_cfs;				/* Station ID in CFS			*/
	double	mjd;						/* Time [MJD]					*/
	double	*atm_prm;					/* Atmospheric Parameters		*/
	double	*gff_result;				/* RESULT of GFF				*/
{
	struct	head_stn	*stn_ptr;		/* Pointer of Station Header	*/

	int		blnum;						/* Number of Baseline			*/
	int		stn_index;					/* Index for Station			*/
	int		stn_index2;					/* Index for Specified Station	*/
	int		delay_index;				/* Parameter Index for Delay	*/
	int		rate_index;					/* Parameter Index for Rate		*/
	int		stn_cfs;					/* Station ID in CFS			*/
	double	sin_el;						/* sin(EL)						*/
	double	dsecz;						/* d[sec z]/dt					*/
	double	rate_ref;
	double	delay_ref;

	blnum	= (antnum * (antnum - 1))/2;
	/*-------- SCAN FOR REFANT --------*/
	stn_ptr	= first_stn_ptr;
	while(stn_ptr != NULL){
		if( stn_ptr->stn_index == refant_id ){
			stn_cfs		= stn_ptr->stn_index;
			calc_el( obs_ptr, obj_ptr, stn_ptr, mjd, &sin_el, &dsecz);
			rate_ref	= atm_prm[stn_cfs - 1]*dsecz;
			delay_ref	= atm_prm[stn_cfs - 1]/sin_el;
			printf("REFANT : %s\n", stn_ptr->stn_name);
			printf("REFANT : DELAY = %e\n", delay_ref);
			printf("REFANT : RATE  = %e\n", rate_ref);
		}
		stn_ptr	= stn_ptr->next_stn_ptr;
	}

	/*-------- SET DELAY AND RATE TO GFF_RESULT --------*/
	stn_index2	= 0;
	for(stn_index=0; stn_index<antnum; stn_index++){
		stn_ptr	= first_stn_ptr;
		while(stn_ptr != NULL){

			if( stn_ptr->stn_index == stnid_in_cfs[stn_index] ){
				stn_cfs		= stnid_in_cfs[stn_index];

				if(stn_cfs != refant_id){

					delay_index	= 2*blnum*ssnum + stn_index2;
					rate_index	= delay_index   + antnum - 1;

				#ifdef DEBUG
				#endif
				printf("[%d] DELAY SOLUTION FOR %s : DL_IDX=%d, RATE_IDX=%d\n", 
					stn_index, stn_ptr->stn_name, delay_index, rate_index);
				#ifdef DEBUG
				#endif


					calc_el( obs_ptr, obj_ptr, stn_ptr, mjd,
							&sin_el, &dsecz);

					gff_result[rate_index]	= atm_prm[stn_cfs - 1]*dsecz
											+ atm_prm[2*stn_num + stn_cfs - 3]
											- rate_ref;
					gff_result[delay_index]	= atm_prm[stn_cfs - 1]/sin_el
											+ atm_prm[stn_num + stn_cfs - 2]
											- delay_ref;

				#ifdef DEBUG
				#endif
				printf("ATM_DELAY = %e\n", atm_prm[stn_cfs - 1]);
				printf("CLK_DELAY = %e\n", atm_prm[stn_num + stn_cfs - 2]);
				printf("CLK_RATE  = %e\n", atm_prm[2*stn_num + stn_cfs - 3]);
				printf("EL        = %lf\n", 57.3*asin(sin_el));
				printf("TOTAL DEL = %e\n", gff_result[delay_index]);
				printf("TOTAL RATE= %e\n", gff_result[rate_index]);
				#ifdef DEBUG
				#endif

					stn_index2 ++;
				}
			}
			stn_ptr	= stn_ptr->next_stn_ptr;
		}
	}
	return;
}
