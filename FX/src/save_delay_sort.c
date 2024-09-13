/*********************************************************
**	SAVE_DELAY_SORT.C :	Plot Antenna-Based Delay Data	**
**														**
**	FUNCTON	: Open FCAL-DATA in CFS and Read Delay, 	**
**			Rate, and Acceleration Data. And store them	**
**			to Specified Linked-List.					**
**			READ_DELAY returns 0 when the specified		**
**			station is the reference antenna. In other	**
**			case, it returns the number of data.		**
**	AUTHOR  : KAMENO Seiji								**
**	CREATED : 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"

#define	FCALUNIT	4

typedef	char	SRCNAME[32];

int	save_delay_sort( stn_id,	refant_id,	delay_num,	index_ptr,	src_ptr,
	mjd_ptr,	t_incr_ptr,	delay_ptr,	dlerr_ptr,
	rate_ptr,	rterr_ptr,	acc_ptr,	accerr_ptr )

	int		stn_id;						/* Station ID in CFS				*/
	int		refant_id;					/* REF ANT ID						*/
	int		delay_num;					/* Number of Delay Data				*/
	int		*index_ptr;					/* Pointer of Index					*/
	SRCNAME	*src_ptr;					/* Pointer of Object Name			*/
	double	*mjd_ptr;					/* Pointer of MJD Data              */
	double	*t_incr_ptr;				/* Pointer of Time Increment        */
	double	*delay_ptr;					/* Pointer of Delay [microsec]      */
	double	*dlerr_ptr;					/* Pointer of Delay Error [microsec]*/
	double	*rate_ptr;					/* Pointer of Rate [picosec/sec]    */
	double	*rterr_ptr;					/* Pointer of Rate Error [psec/sec] */
	double	*acc_ptr;					/* Pointer of Acceleration          */
	double	*accerr_ptr;				/* Pointer of Acceleration Error    */
{

	/*-------- INDEX --------*/
	int		data_index;		/* Index for Data								*/
	int		sorted_index;	/* Index for Sorted Data						*/
	int		stn_index;		/* Index for Station							*/
	int		ss_index;		/* Index for Sub-Stream							*/

	/*-------- IDENTIFIER --------*/
	int		ret;			/* Return Code from CFS Library					*/
	int		recid;			/* Record ID in CFS								*/
	int		dlunit;			/* Logical Unit for Delay File					*/

	/*-------- TOTAL NUMBER --------*/
	int		ssnum;			/* Number of Sub-Stream							*/

	/*-------- GENERAL VARIABLES --------*/
	int		nival;			/* Number of Integer Data in CFS				*/
	int		nrval;			/* Number of Double Data in CFS					*/
	int		ncval;			/* Byte Number of Character Data in CFS			*/
	int		*int_ptr;		/* Pointer for Integer data in CFS				*/
	double	*double_ptr;	/* Pointer for Double data in CFS				*/
	char	fname[128];		/* File Name of CFS Files						*/
	char	omode[2];		/* Access Mode of CFS Files						*/
	char	cvalue[64];		/* Character Data in CFS						*/
	double	rvalue;			/* Dummy Variable for Reading CFS				*/
	char	obs_name[32];
	char	stn_name[32];

	dlunit	= FCALUNIT;
	sprintf(omode, "u");

	/*-------- OPEN DELAY FILE --------*/
	sprintf(fname, "CALIB/FCAL.%d", stn_id );
	cfs287_(fname, &ret, strlen(fname) );	cfs_ret( 287, ret );
	cfs103_(&dlunit, fname, omode, &ret, strlen(fname), strlen(omode) );
	cfs_ret( 103, ret );


	/*-------- READ HEADER ITEMS --------*/
	recid	= 0;	nival	= 2;	nrval	= 0;	ncval	= 64;
	int_ptr	= (int *)malloc( nival*sizeof(int) );

	cfs125_( &dlunit, &recid, &nival, int_ptr, &nrval, &rvalue,
				&ncval, cvalue, &ret, ncval );
	cfs_ret( 125, ret );

	/*-------- READ CHARACTER DATA --------*/
	sscanf( &cvalue[0], "%s", obs_name);
	sscanf( &cvalue[32], "%s", stn_name);

	/*-------- READ INTEGER DATA --------*/
	stn_index	= int_ptr[0];
	ssnum		= int_ptr[1];
	free(int_ptr);

	/*-------- READ DELAY DATA --------*/
	recid	= 1;
	nival	= 1;	nrval	= 2 + ssnum*6;	ncval	= 32;
	int_ptr		= (int *)malloc( nival*sizeof(int) );
	double_ptr	= (double *)malloc( nrval*sizeof(double) );

	/*-------- LOOP FOR DATA --------*/
	for(data_index=0; data_index<delay_num; data_index++){

		/*-------- INDEX MAPPING by SEQUENTIAL ORDER --------*/
		sorted_index	= index_ptr[data_index];

		*int_ptr	= refant_id;
		strcpy( cvalue, src_ptr[sorted_index]);
		double_ptr[0]	= mjd_ptr[sorted_index];
		double_ptr[1]	= t_incr_ptr[sorted_index];

		for(ss_index=0; ss_index<ssnum; ss_index++){
			double_ptr[2 + ss_index]			= rate_ptr[sorted_index];
			double_ptr[2 + ssnum + ss_index]	= rterr_ptr[sorted_index];
			double_ptr[2 + 2*ssnum + ss_index]	= delay_ptr[sorted_index];
			double_ptr[2 + 3*ssnum + ss_index]	= dlerr_ptr[sorted_index];
			double_ptr[2 + 4*ssnum + ss_index]	= acc_ptr[sorted_index];
			double_ptr[2 + 5*ssnum + ss_index]	= accerr_ptr[sorted_index];
		}

		/*-------- WRITE TO CFS --------*/
		cfs126_( &dlunit, &recid, &nival, int_ptr, &nrval, double_ptr,
			&ncval, cvalue, &ret, ncval );

		if(ret == 55555){	break;}

	}

	/*-------- CLOSE DELAY FILE --------*/
	free(int_ptr);
	free(double_ptr);
	cfs104_( &dlunit, &ret );	cfs_ret( 104, ret );

	return(delay_num);
}
