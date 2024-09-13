/*********************************************************
**	SAVE_GFF.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include "obshead.inc"
#define	FCALDATA	3
#define	MAX_SS		32	
#define	SEC_DAY		86400	

int	save_gff( obs_name, obj_name, stn_ptr,
			stn_id, ref_id, stn_index, stn_num,
			ssnum, ssnum_in_cfs, ss_id, start_mjd,
			integ_time, gff_result, gff_err )

	char	*obs_name;				/* Observation Code */
	char	*obj_name;				/* Object Name */
	struct	head_stn	*stn_ptr;	/* Pointer of Linked-list for Stations */ 
	int		stn_id;					/* Station ID */
	int		ref_id;					/* Reference Antenna ID */
	int		stn_index;				/* Station Index in GFF_RESULT */
	int		stn_num;				/* Total Number of Stations */
	int		ssnum;					/* Selected Number of Sub-Stream */
	int		ssnum_in_cfs;			/* Total Number of Sub-Stream */
	int		*ss_id;					/* Index of Sub-Stream */
	double	start_mjd;
	double	integ_time;
	float	*gff_result;			/* Result of GFF */
	float	*gff_err;				/* Error of GFF */
{
	int		ss_index;				/* Index of Sub-Stream */
	int		dldata_offset;			/* Pointer offset for Delay data */
	int		fcalunit;				/* Logical Unit Number for FCAL data */
	int		recid;					/* Record ID in CFS Data */
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

	dldata_offset	= stn_num*(stn_num - 1)*ssnum;
	fcalunit	= FCALDATA;
	sprintf(fmode, "u"); 

	/*-------- OPEN BANDPASS DATA --------*/
	sprintf(fname, "CALIB/FCAL.%d", stn_id ); 
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&fcalunit, fname, fmode, &ret, strlen(fname), strlen(fmode));
	cfs_ret( 103, ret );

	/*-------- SEARCH TARGET STATION NAME --------*/
	while(stn_ptr != NULL){
		if( stn_ptr->stn_index == stn_id ){	break;}
		stn_ptr	= stn_ptr->next_stn_ptr;
	}

	/*-------- WRITE HEADDER ITEMS --------*/
	cfs401_(&fcalunit, &ret);	cfs_ret( 401, ret );
	recid = 0; ncval = 64;	nrval = 0;	nival = 2;
	int_ptr	= (int *)malloc( nival*sizeof(int) );
	memset( cvalue, 0x20, ncval );
	sprintf( &cvalue[0],  "%s", obs_name );
	sprintf( &cvalue[32], "%s", stn_ptr->stn_name );
	*int_ptr	= stn_id;		int_ptr++;
	*int_ptr	= ssnum_in_cfs;	int_ptr--;
	cfs126_( &fcalunit, &recid, &nival, int_ptr, &nrval, &rvalue,
			&ncval, cvalue, &ret, ncval );
	cfs_ret( 126, ret );

	free(int_ptr);
/*
--------------------------------------- GO TO END OF FILE
*/
	int_ptr = (int *)malloc( sizeof(int));
	double_ptr = (double *)malloc( (6 * ssnum_in_cfs + 2)*sizeof(double));
	memset( double_ptr, 0, sizeof(double_ptr) );

	cfs402_(&fcalunit, &ret);	cfs_ret( 402, ret );

	nival = 0;	nrval  = 0;	ncval = 0;
	recid = 1;
/*
--------------------------------------- SET INTEGER DATA
*/
	*int_ptr	= ref_id;	nival++;
/*
--------------------------------------- SET DOUBLE DATA
*/
	/*-------- SET REAL DATA --------*/
	double_ptr[0]	= start_mjd
				+ integ_time/(2*SEC_DAY);
	double_ptr[1]	= integ_time;

	/*-------- SET DELAY RATE --------*/
	for( ss_index=0; ss_index<ssnum_in_cfs; ss_index++){
		if(stn_index == 0){
			double_ptr[ss_index + 2]	= 0.0;
		} else {
			double_ptr[ss_index + 2]
				= gff_result[dldata_offset + stn_num + stn_index - 2];
		}
	}

	/*-------- SET DELAY RATE ERROR --------*/
	for( ss_index=0; ss_index<ssnum_in_cfs; ss_index++){
		if(stn_index == 0){
			double_ptr[ss_index + ssnum_in_cfs + 2]	= 0.0;
		} else {
			double_ptr[ss_index + ssnum_in_cfs + 2]
				= gff_err[dldata_offset + stn_num + stn_index - 2];
		}
	}

	/*-------- SET DELAY --------*/
	for( ss_index=0; ss_index<ssnum_in_cfs; ss_index++){
		if(stn_index == 0){
			double_ptr[ss_index + 2*ssnum_in_cfs + 2]	= 0.0;
		} else {
			double_ptr[ss_index + 2*ssnum_in_cfs + 2]
				= gff_result[dldata_offset + stn_index -1];
		}
	}

	/*-------- SET DELAY ERROR --------*/
	for( ss_index=0; ss_index<ssnum_in_cfs; ss_index++){
		if(stn_index == 0){
			double_ptr[ss_index + 3*ssnum_in_cfs + 2]	= 0.0;
		} else {
			double_ptr[ss_index + 3*ssnum_in_cfs + 2]
				= gff_err[dldata_offset + stn_index -1];
		}
	}

	/*-------- SET ACC --------*/
	for( ss_index=0; ss_index<ssnum_in_cfs; ss_index++){
		double_ptr[ss_index + 4*ssnum_in_cfs + 2] = 0.0;	nrval++;
		double_ptr[ss_index + 5*ssnum_in_cfs + 2] = 0.0;	nrval++;
	}
	nrval	= 2 + 6*ssnum_in_cfs;
/*
--------------------------------------- SET CHARACTER DATA
*/
	sprintf( cvalue, "%s", obj_name ); ncval = 32;
/*
--------------------------------------- WRITE TO CFS
*/
	#ifdef DEBUG
	printf(" RECID=%d : NIVAL=%d, NRVAL=%d, NCVAL=%d\n",
		recid, nival, nrval, ncval );
	#endif

	cfs126_( &fcalunit, &recid, &nival, int_ptr, &nrval, double_ptr,
			&ncval, cvalue, &ret, ncval );
	cfs_ret( 126, ret );

	free(int_ptr);
	free(double_ptr);
/*
--------------------------------------- CLOSE CFS
*/
	cfs104_( &fcalunit, &ret );	cfs_ret( 104, ret );
	return(0);
}
