/*********************************************************
**	STATION_SCAN.C	: GET STATION-BASED INFORMATION		**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"
#define MAX_ANT 10
#define MAX_SS  32

int	station_scan( first_stn_ptr, fcal_ptr,	pcal_ptr,	first_gcal_ptr,
				src_name,	pcal_dat_num,	stnid_in_cfs,
				refant_id_ptr,	antnum_ptr,		stn_num_ptr)

	struct head_stn	*first_stn_ptr;		/* First Pointer of Station Header	*/
	int		*fcal_ptr;					/* Pointer of FCAL Address			*/
	int		*pcal_ptr;					/* P-Cal Data Pointer				*/
	int		*first_gcal_ptr;			/* Pointer of GCAL Data				*/
	char	*src_name;					/* Source Name of Delay Calibrator	*/
	int		*pcal_dat_num;				/* Number of P-Cal Data 			*/
	int		*stnid_in_cfs;				/* Station ID in CFS				*/
	int		*refant_id_ptr;					/* Pointer of REFANT ID				*/
	int		*antnum_ptr;				/* Pointer of Antenna Number		*/
	int		*stn_num_ptr;				/* Pointer of Antenna Number		*/
{
	int		stn_index;					/* Index for Station				*/
	int		stn_arg;					/* Argument Number of Station		*/
	int		*gcal_ptr;					/* Pointer of GCAL Data				*/
	struct head_stn	*stn_ptr;			/* Pointer of Station Header		*/
	double	mjd_min, mjd_max;			/* Min and Max of MJD				*/
	double	rate_min, rate_max;			/* Min and Max of Rate				*/
	double	delay_min, delay_max;		/* Min and Max of Delay				*/
	double	acc_min, acc_max;			/* Min and Max of Acceleration		*/
	double	gain_min, gain_max;			/* Min and Max of Gain				*/

	*antnum_ptr = 0;	stn_index = 0;

	stn_ptr	= first_stn_ptr;
	gcal_ptr= first_gcal_ptr;

	/*-------- SEEK SPECIFIED STATION NAME --------*/
	while( stn_ptr != NULL ){

		/*-------- KEEP STATION ID IN CFS --------*/
		stnid_in_cfs[*antnum_ptr] = stn_ptr->stn_index;

		/*-------- READ GCAL DATA --------*/
		mjd_min = 1.0e9;	mjd_max = -1.0e9;
		gain_min = 1.0e9;	gain_max = -1.0e9;
		read_gain( stn_ptr->stn_index, gcal_ptr,
				&mjd_min, &mjd_max, &gain_min, &gain_max);
		gcal_ptr++;

		/*-------- READ FCAL DATA --------*/
		if( read_delay(stn_ptr->stn_index,  src_name,
			&fcal_ptr[stn_index], &mjd_min,	&mjd_max,
			&rate_min,	&rate_max,	&delay_min,	&delay_max,
			&acc_min,	&acc_max) == -1){

			/*-------- MEMORY REFANT ID --------*/
			*refant_id_ptr = stn_ptr->stn_index;

		} else {

			/*-------- COUNT UP SLAVE STATION ID --------*/
			stn_index ++;
		}

		/*-------- READ P-CAL DATA --------*/
		pcal_dat_num[stn_ptr->stn_index-1] = read_pcal(
			stn_ptr->stn_index, &pcal_ptr[stn_ptr->stn_index-1] );

		(*antnum_ptr)++;

		/*-------- GO TO NEXT STATION LIST  --------*/
		stn_ptr = stn_ptr->next_stn_ptr;

	}

	*stn_num_ptr = stn_index;

	return(1);
}
