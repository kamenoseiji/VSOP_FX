/*********************************************************
**	STATION_INFO.C	: GET STATION-BASED INFORMATION		**
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

int	station_info( stn_list, src_name, first_stn_ptr, stn_num,
				pcal_ptr,	pcal_dat_num,	gcal_ptr,
				stnid_in_cfs, fcal_ptr, refant_id, antnum, station_num)

	char	**stn_list;					/* Pointer of Station Name List		*/
	char	*src_name;					/* Source Name of Delay Calibrator	*/
	struct head_stn	*first_stn_ptr;		/* First Pointer of Station Header	*/
	int		*pcal_ptr;					/* P-Cal Data Pointer				*/
	int		*pcal_dat_num;				/* Number of P-Cal Data 			*/
	int		*gcal_ptr;					/* Pointer of GCAL Data				*/
	int		stn_num;					/* Number of Stations				*/
	int		*stnid_in_cfs;				/* Station ID in CFS				*/
	int		*fcal_ptr;					/* Pointer of FCAL Address			*/
	int		*refant_id;					/* Pointer of REFANT ID				*/
	int		*antnum;					/* Pointer of Antenna Number		*/
	int		*station_num;				/* Pointer of Antenna Number		*/
{
	int		stn_index;					/* Index for Station				*/
	int		stn_arg;					/* Argument Number of Station		*/
	struct head_stn	*stn_ptr;			/* Pointer of Station Header		*/
	double	mjd_min, mjd_max;			/* Min and Max of MJD				*/
	double	rate_min, rate_max;			/* Min and Max of Rate				*/
	double	delay_min, delay_max;		/* Min and Max of Delay				*/
	double	acc_min, acc_max;			/* Min and Max of Acceleration		*/
	double	gain_min, gain_max;			/* Min and Max of Gain				*/

	struct	gcal_data	*sefd_ptr;

	*antnum = 0;	stn_index = 0;

	/*-------- LOOP FOR GIVEN STATION NAME LIST --------*/
	for(stn_arg=0; stn_arg<stn_num; stn_arg++){
		stn_ptr	= first_stn_ptr;

		/*-------- SEEK SPECIFIED STATION NAME --------*/
		while( stn_ptr != NULL ){

			/*-------- STATION NAME MATCHING --------*/ 
			if( strstr(stn_ptr->stn_name, stn_list[stn_arg]) != NULL ){
				printf("STATION %-10s: ID = %2d\n",
					stn_ptr->stn_name, stn_ptr->stn_index );

				/*-------- KEEP STATION ID IN CFS --------*/
				stnid_in_cfs[*antnum] = stn_ptr->stn_index;

				/*-------- READ GCAL DATA --------*/
				mjd_min = 1.0e9;	mjd_max = -1.0e9;
				gain_min = 1.0e9;	gain_max = -1.0e9;
				read_gain( stn_ptr->stn_index, &gcal_ptr[stn_arg],
					&mjd_min, &mjd_max, &gain_min, &gain_max);

				/*-------- READ FCAL DATA --------*/
				if( read_delay(stn_ptr->stn_index,  src_name,
					&fcal_ptr[stn_index], &mjd_min,	&mjd_max,
					&rate_min,	&rate_max,	&delay_min,	&delay_max,
					&acc_min,	&acc_max) == -1){

					/*-------- MEMORY REFANT ID --------*/
					*refant_id = stn_ptr->stn_index;

				} else {

					/*-------- COUNT UP SLAVE STATION ID --------*/
					stn_index ++;
				}

				/*-------- READ P-CAL DATA --------*/
				pcal_dat_num[stn_ptr->stn_index-1] = read_pcal(
					stn_ptr->stn_index, &pcal_ptr[stn_ptr->stn_index-1] );

				(*antnum)++;
				break;
			} else {

				/*-------- GO TO NEXT STATION LIST  --------*/
				stn_ptr = stn_ptr->next_stn_ptr;
			}
		}

	}
	*station_num = stn_index;

	return(1);
}
