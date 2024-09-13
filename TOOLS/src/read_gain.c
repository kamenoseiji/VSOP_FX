/*********************************************************
**	READ_GAIN.C :	Plot Antenna-Based Delay Data		**
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

#define	GCALUNIT	4

int	read_gain( stn_id,	first_gcal_ptr, mjd_min_ptr,	mjd_max_ptr,
				gain_min_ptr,	gain_max_ptr )

	int		stn_id;							/* Station ID in CFS			*/
	long	*first_gcal_ptr;				/* First Pointer of GAIN data	*/
	double	*mjd_min_ptr,	*mjd_max_ptr;	/* Max and Min of MJD			*/
	double	*gain_min_ptr,	*gain_max_ptr;	/* Max and Min of GAIN			*/
{
	int		ret;			/* Return Code from CFS Library */
	int		recid;			/* Record ID in CFS */
	char	fname[128];		/* File Name of CFS Files */
	char	omode[2];		/* Access Mode of CFS Files */
	int		ssnum;			/* Number of Sub-Stream */
	int		stn_index;		/* Index for Station */
	int		gclunit;		/* Logical Unit for Gain File */
	int		*int_ptr;		/* Pointer for Integer data in CFS */
	double	*double_ptr;	/* Pointer for Double data in CFS */
	char	cvalue[64];		/* Character Data in CFS */
	int		nival;			/* Number of Integer Data in CFS */
	int		nrval;			/* Number of Double Data in CFS */
	int		ncval;			/* Byte Number of Character Data in CFS */
	int		gaindata_num;	/* Number of GAIN Data */
	double	rvalue;			/* Dummy Variable for Reading CFS */
	char	obs_name[32];
	char	stn_name[32];
	struct	gcal_data	*gcal_ptr;		/* Pointer of GAIN data */
	struct	gcal_data	*prev_gcal_ptr;	/* Previous Pointer of GAIN data */

	gclunit	= GCALUNIT;
	sprintf(omode, "r");

	/*-------- OPEN DELAY FILE --------*/
	sprintf(fname, "CALIB/GCAL.%d", stn_id );
	cfs287_(fname, &ret, strlen(fname) );	cfs_ret( 287, ret );
	cfs103_(&gclunit, fname, omode, &ret, strlen(fname), strlen(omode) );
	cfs_ret( 103, ret );

	/*-------- READ HEADER ITEMS --------*/
	recid	= 0;	nival	= 2;	nrval	= 0;	ncval	= 64;
	int_ptr	= (int *)malloc( nival*sizeof(int) );

	cfs125_( &gclunit, &recid, &nival, int_ptr, &nrval, &rvalue,
				&ncval, cvalue, &ret, ncval );
	cfs_ret( 125, ret );
	
	/*-------- READ CHARACTER DATA --------*/
	sscanf( &cvalue[0], "%s", obs_name);
	sscanf( &cvalue[32], "%s", stn_name);

	/*-------- READ INTEGER DATA --------*/
	stn_index	= int_ptr[0];
	ssnum		= int_ptr[1];

	/*-------- READ DELAY DATA --------*/
	nival	= 0;	nrval	= 2 + 4*ssnum;	ncval	= 32;
	double_ptr	= (double *)malloc( nrval*sizeof(double) );

	gaindata_num	= 0;

	while(1){

		/*-------- READ FROM CFS --------*/
		cfs125_( &gclunit, &recid, &nival, int_ptr, &nrval, double_ptr,
			&ncval, cvalue, &ret, ncval );

		/*-------- IN CASE OF EOF --------*/
		if(ret == 55555){	break;}

		/*-------- ALLOCATE MEMORY FOR THE DATA --------*/
		gcal_ptr = (struct gcal_data *)malloc( sizeof(struct gcal_data) );
		if( gcal_ptr == NULL ){
			printf(" MEMORY ERROR at INDEX = %d\n", gaindata_num );
			exit(0);
		}

		/*-------- REMEMBER THE INITIAL POINTER OF LINKED LIST --------*/
		if( gaindata_num == 0 ){
			*first_gcal_ptr	= (long)gcal_ptr;
			prev_gcal_ptr	= gcal_ptr;
		}

		gcal_ptr->mjd		= double_ptr[0];
		gcal_ptr->time_incr	= double_ptr[1];
		gcal_ptr->phase		= double_ptr[2];
		gcal_ptr->real		= double_ptr[2+ssnum];
		gcal_ptr->imag		= double_ptr[2+2*ssnum];
		gcal_ptr->weight	= double_ptr[2+3*ssnum];
		sprintf(gcal_ptr->objnam, "%s", cvalue);


		printf("MJD=%lf : GAIN=%lf\n", gcal_ptr->mjd, gcal_ptr->real);


	if( gcal_ptr->mjd   > *mjd_max_ptr ){	*mjd_max_ptr = gcal_ptr->mjd; }
	if( gcal_ptr->mjd   < *mjd_min_ptr ){	*mjd_min_ptr = gcal_ptr->mjd; }

	if(gcal_ptr->weight > 0.001){

		if(gcal_ptr->real > *gain_max_ptr ){*gain_max_ptr = gcal_ptr->real; }
		if(gcal_ptr->real < *gain_min_ptr ){*gain_min_ptr = gcal_ptr->real; }
	}

		/*-------- LINK TO THE NEXT POINTER --------*/
		prev_gcal_ptr->next_gcal_ptr	= gcal_ptr;
		gcal_ptr->next_gcal_ptr			= NULL;
		prev_gcal_ptr					= gcal_ptr;

		gaindata_num++;
	}

	/*-------- CLOSE DELAY FILE --------*/
	free(int_ptr);
	free(double_ptr);
	cfs104_( &gclunit, &ret );	cfs_ret( 104, ret );

	return(gaindata_num);
}
