/*********************************************************
**	READ_BP.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#define	BPDATA	3
#define	MAX_SS	32

int	read_bp( stn_id, obj_name, ssnum_ptr,
			freq_num_ptr, start_mjd_ptr, integ_time_ptr,
			rf_ptr, freq_incr_ptr, vr_ptr_ptr, vi_ptr_ptr, vis_max_ptr )

	int		stn_id;					/* Station ID */
	char	*obj_name;				/* Object Name */
	int		*ssnum_ptr;				/* Number of Sub-Stream */
	int		*freq_num_ptr;			/* Pointer of Number of Freq. Channels */
	double	*start_mjd_ptr;			/* INTEG START TIME [MJD] */
	double	*integ_time_ptr;		/* INTEGRATION TIME [sec] */
	double	*rf_ptr;				/* RF Frequency [MHz] */
	double	*freq_incr_ptr;			/* Frequency Increment */
	int		*vr_ptr_ptr;			/* Pointer of Visibility Data (real) */
	int		*vi_ptr_ptr;			/* Pointer of Visibility Data (imag) */
	double	*vis_max_ptr;			/* Pointer of Maximum Visibility */
{
	int		ss_index;				/* Index of Sub-Stream */
	int		freq_index;				/* Index of Frequency Channel */
	int		bpunit;					/* Logical Unit Number for CORR data */
	int		recid;					/* Record ID in CFS Data */
	int		ret;					/* Return Code from CFS Library */
	int		nival;					/* Number of Integer Data in CFS Library */
	int		nrval;					/* Number of Real Data in CFS Library */
	int		ncval;					/* Number of Char Data in CFS Library */
	double	rvalue;					/* Real Data in CFS Library */
	char	cvalue[64];				/* Char Data in CFS Library */
	int		*int_ptr;				/* Pointer of Integer Data */
	double	*double_ptr;			/* Pointer of Double Data */
	double	*visr_ptr;				/* Pointer of Visibility (double) */
	double	*visi_ptr;				/* Pointer of Visibility (double) */
	char	fmode[2];				/* CFS Access Mode */
	char	fname[32];				/* CFS File Name */

	bpunit	= BPDATA;
	sprintf(fmode, "r"); 

	/*-------- OPEN BANDPASS DATA --------*/
	sprintf(fname, "CALIB/BCAL.%d", stn_id ); 
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&bpunit, fname, fmode, &ret, strlen(fname), strlen(fmode));
	cfs_ret( 103, ret );

/*
--------------------------------------- READ HEADER ITEMS
*/
	recid = 0;			/* Record ID for HEADER is 0 */
	int_ptr = (int *)malloc( (2+MAX_SS)*sizeof(int) );
	nival = MAX_SS+2;	nrval  = 0;	ncval = 64;
	cfs125_( &bpunit, &recid, &nival, int_ptr, &nrval, &rvalue,
			&ncval, cvalue, &ret, ncval );

	/*-------- READ INTEGER DATA --------*/
	*ssnum_ptr	= int_ptr[1];
	for(ss_index=0; ss_index< *ssnum_ptr; ss_index++){
		freq_num_ptr[ss_index] = int_ptr[ss_index + 2];
	}
	free(int_ptr);

/*
--------------------------------------- READ DATA ITEMS
*/
	for( ss_index=0; ss_index< *ssnum_ptr; ss_index++){
		recid++;

		nival = 2;	nrval  = 4* freq_num_ptr[ss_index] +4;	ncval = 32;
		int_ptr = (int *)malloc( nival * sizeof(int));

		if( freq_num_ptr[ss_index] > 0){
			double_ptr = (double *)malloc( nrval * sizeof(double));
			visr_ptr = (double *)malloc(freq_num_ptr[ss_index]* sizeof(double));
			visi_ptr = (double *)malloc(freq_num_ptr[ss_index]* sizeof(double));

			vr_ptr_ptr[ss_index] = (int)visr_ptr;
			vi_ptr_ptr[ss_index] = (int)visi_ptr;

			cfs125_( &bpunit, &recid, &nival, int_ptr, &nrval, double_ptr,
				&ncval, cvalue, &ret, 64 );
			if( cfs_ret( 125, ret ) == -1 ){
				printf("Warning: Can't Access to BP File [%s]!!\n", fname);
				for(freq_index=0;freq_index<freq_num_ptr[ss_index];freq_index++){
					visr_ptr[freq_index] = 1.0;
					visi_ptr[freq_index] = 0.0;
				}
				*start_mjd_ptr	= 0.0;
				*integ_time_ptr	= 0.0;
				rf_ptr[ss_index]= 0.0;
				freq_incr_ptr[ss_index]	= 0.0;
				vis_max_ptr[ss_index] = 1.0;

			} else {

				/*-------- READ REAL DATA --------*/
				*start_mjd_ptr	= double_ptr[0];
				*integ_time_ptr	= double_ptr[1];
				rf_ptr[ss_index]= double_ptr[2];
				freq_incr_ptr[ss_index]	= double_ptr[3];
				vis_max_ptr[ss_index]	= -9999.0;
				for(freq_index=0; freq_index<freq_num_ptr[ss_index]; freq_index++){
					visr_ptr[freq_index] = double_ptr[4 + freq_index];
					visi_ptr[freq_index] = double_ptr[4 + freq_index +
									2*(freq_num_ptr[ss_index]) ];
					if(visr_ptr[freq_index] > vis_max_ptr[ss_index]){
						vis_max_ptr[ss_index] = visr_ptr[freq_index];
					}
				}
			}

			/*-------- READ CHARACTER DATA --------*/
			sscanf( cvalue, "%s", obj_name );

			free(double_ptr);
		}

		free(int_ptr);
	}	/*-------- END OF SUB_STREAM LOOP --------*/

	/*-------- CLOSE BANDPASS FILE --------*/
	cfs104_( &bpunit, &ret );	cfs_ret( 104, ret );

	return(0);
}
