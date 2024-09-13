/*********************************************************
**	SAVE_BP.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#define	BPDATA	3
#define	MAX_SS	32	

int	save_bp( obs_name, stn_name, stn_id, obj_name, ssnum, freq_num_ptr,
			start_mjd,	integ_time, rf_ptr, freq_incr_ptr,
			vis_r_ptr, vis_i_ptr )

	char	*obs_name;				/* Observation Code */
	char	*stn_name;				/* Station Name */
	int		stn_id;					/* Station ID */
	char	*obj_name;				/* Object Name */
	int		ssnum;					/* Number of Sub-Stream */
	int		*freq_num_ptr;			/* Pointer of Number of Freq. Channels */
	double	start_mjd;				/* INTEG START TIME [MJD] */
	double	integ_time;				/* INTEGRATION TIME [sec] */
	double	*rf_ptr;				/* RF Frequency [MHz] */
	double	*freq_incr_ptr;			/* Frequency Increment */
	double	**vis_r_ptr;			/* Pointer of Visibility Data (real) */
	double	**vis_i_ptr;			/* Pointer of Visibility Data (imag) */
{
	int		ss_index;				/* Index of Sub-Stream */
	int		bpunit;					/* Logical Unit Number for CORR data */
	int		recid;					/* Record ID in CFS Data */
	int		origin;					/* Origin used in CFS400 */
	int		nrcd;					/* Record Shift Number in CFS400 */
	int		ret;					/* Return Code from CFS Library */
	int		nival;					/* Number of Integer Data in CFS Library */
	int		nrval;					/* Number of Real Data in CFS Library */
	int		ncval;					/* Number of Char Data in CFS Library */
	double	rvalue;					/* Real Data in CFS Library */
	char	cvalue[64];				/* Char Data in CFS Library */
	int		*int_ptr;				/* Pointer of Integer Data */
	double	*double_ptr;			/* Pointer of Double Data */
	char	fmode[2];				/* CFS Access Mode */
	char	fname[32];				/* CFS File Name */
	int		i;

	bpunit	= BPDATA;
	sprintf(fmode, "w");

	/*-------- OPEN BANDPASS DATA --------*/
	sprintf(fname, "CALIB/BCAL.%d", stn_id ); 
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&bpunit, fname, fmode, &ret, strlen(fname), strlen(fmode));
	cfs_ret( 103, ret );
	cfs401_(&bpunit, &ret);	cfs_ret(401, ret);
/*
--------------------------------------- WRITE HEADER ITEMS
*/
	recid = 0;			/* Record ID for HEADER is 0 */
	nival = 0;	nrval  = 0;	ncval = 0;

	/*-------- SET CHATACTER DATA --------*/
	memset(cvalue, 0x20, 64);
	sprintf( &cvalue[0], "%s", obs_name);
	sprintf( &cvalue[32], "%s", stn_name);
	ncval	= 64;

	int_ptr = (int *)malloc( (2+MAX_SS)*sizeof(int) );
	/*-------- SET INTEGER DATA --------*/
	*int_ptr	= stn_id;	int_ptr++;	nival++;
	*int_ptr	= ssnum;	int_ptr++;	nival++;

	for( ss_index=0; ss_index<ssnum; ss_index++){
		*int_ptr	= *freq_num_ptr;
		int_ptr++;	freq_num_ptr++;
		nival++;
	}

	freq_num_ptr -= ssnum;
	for( ss_index=ssnum; ss_index<MAX_SS; ss_index++){
		*int_ptr	= 0;	int_ptr++;
		nival++;
	}
	int_ptr -= (2+MAX_SS);


	/*-------- WRITE TO CFS DATA FILE --------*/
	cfs126_( &bpunit, &recid, &nival, int_ptr, &nrval, &rvalue,
			&ncval, cvalue, &ret, ncval );
	cfs_ret( 126, ret );
	free(int_ptr);

	/*-------- CLOSE BANDPASS FILE to SAVE --------*/
	cfs104_( &bpunit, &ret );	cfs_ret( 104, ret );
/*
--------------------------------------- OPEN CFS AGAIN
*/
	sprintf(fmode, "w");

	/*-------- OPEN BANDPASS DATA --------*/
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&bpunit, fname, fmode, &ret, strlen(fname), strlen(fmode));
	cfs_ret( 103, ret );
	cfs401_(&bpunit, &ret);	cfs_ret(401, ret);
	origin = 0;	nrcd = 1;
	cfs400_(&bpunit, &origin, &nrcd, &ret);	cfs_ret(400, ret);
/*
--------------------------------------- WRITE DATA ITEMS
*/
	/*-------- SET REAL DATA --------*/
	memset(cvalue, 0x20, 64);
	sprintf( cvalue, "%s", obj_name );

	for( ss_index=0; ss_index<ssnum; ss_index++){
		recid++;
		if( *freq_num_ptr != NULL ){

		nival = 1;	nrval = 4*(*freq_num_ptr) + 4; ncval = 32;

		int_ptr = (int *)malloc( sizeof(int) );
		/*-------- SET INTEGER DATA --------*/
		*int_ptr	=stn_id;

		double_ptr = (double *)malloc(nrval*sizeof(double));
		memset(double_ptr, 0, nrval*sizeof(double) );
		/*-------- SET REAL DATA --------*/
		double_ptr[0]	= start_mjd;
		double_ptr[1]	= integ_time;
		double_ptr[2]	= *rf_ptr;
		double_ptr[3]	= *freq_incr_ptr;

		set_vis(*freq_num_ptr, *vis_r_ptr, &double_ptr[4]);
		set_vis(*freq_num_ptr, *vis_i_ptr, &double_ptr[4 + 2*(*freq_num_ptr)]);

		/*-------- WRITE TO CFS FILE --------*/

		#ifdef DEBUG
		printf(" RECID=%d : NIVAL=%d, NRVAL=%d, NCVAL=%d\n",
			recid, nival, nrval, ncval );
		#endif

		cfs126_( &bpunit, &recid, &nival, int_ptr, &nrval, &double_ptr[0],
				&ncval, cvalue, &ret, ncval );
		cfs_ret( 126, ret );

		free(int_ptr);
		free(double_ptr);

		}

		vis_r_ptr++;
		vis_i_ptr++;
		freq_num_ptr++;
		rf_ptr++;
		freq_incr_ptr++;

	}	/*-------- END OF SUB_STREAM LOOP --------*/

	/*-------- CLOSE BANDPASS FILE --------*/
	cfs104_( &bpunit, &ret );	cfs_ret( 104, ret );

	return(0);
}

set_vis( freq_num, vis_org_ptr, vis_dest_ptr )
	int		freq_num;
	double	*vis_org_ptr;
	double	*vis_dest_ptr;
{
	int		freq_index;

	/*-------- SET BANDPASS DATA TO BUFFER --------*/
	for(freq_index=0; freq_index<freq_num; freq_index++){
		*vis_dest_ptr = *vis_org_ptr;
		vis_org_ptr++;	vis_dest_ptr++;
	}

	/*-------- SET BANDPASS ERROR TO BUFFER --------*/
	for(freq_index=0; freq_index<freq_num; freq_index++){
		*vis_dest_ptr = 0.0; vis_dest_ptr++;
	}
	return;
}
