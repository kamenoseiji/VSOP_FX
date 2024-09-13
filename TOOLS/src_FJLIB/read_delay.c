/*********************************************************
**	READ_DELAY.C :	Plot Antenna-Based Delay Data		**
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

int	read_delay( stn_id,	obj_name, first_fcal_ptr,
				mjd_min_ptr,	mjd_max_ptr,
				rate_min_ptr,	rate_max_ptr,
				delay_min_ptr,	delay_max_ptr,
				acc_min_ptr,	acc_max_ptr	)

	int		stn_id;							/* Station ID in CFS */
	char	*obj_name;						/* Pointer of Object Name */
	int		*first_fcal_ptr;				/* First Pointer of CLOCK data */
	double	*mjd_min_ptr,	*mjd_max_ptr;	/* Max and Min of MJD */
	double	*rate_min_ptr,	*rate_max_ptr;	/* Max and Min of Rate */
	double	*delay_min_ptr,	*delay_max_ptr;	/* Max and Min of Delay */
	double	*acc_min_ptr,	*acc_max_ptr;	/* Max and Min of Acceleration */
{
	int		data_index;
	int		ret;			/* Return Code from CFS Library */
	int		recid;			/* Record ID in CFS */
	char	fname[128];		/* File Name of CFS Files */
	char	omode[2];		/* Access Mode of CFS Files */
	int		ssnum;			/* Number of Sub-Stream */
	int		stn_index;		/* Index for Station */
	int		dlunit;			/* Logical Unit for Delay File */
	int		*int_ptr;		/* Pointer for Integer data in CFS */
	double	*double_ptr;	/* Pointer for Double data in CFS */
	char	cvalue[64];		/* Character Data in CFS */
	int		nival;			/* Number of Integer Data in CFS */
	int		nrval;			/* Number of Double Data in CFS */
	int		ncval;			/* Byte Number of Character Data in CFS */
	int		dldata_num;		/* Number of Delay Data */
	double	rvalue;			/* Dummy Variable for Reading CFS */
	char	obs_name[32];
	char	stn_name[32];
	struct	fcal_data	*fcal_ptr;		/* Pointer of CLOCK data */
	struct	fcal_data	*prev_fcal_ptr;	/* Previous Pointer of CLOCK data */

	*mjd_min_ptr =  1.0e33;
	*mjd_max_ptr = -1.0e33;
	*rate_min_ptr=  1.0e33;
	*rate_max_ptr= -1.0e33;
	*delay_min_ptr =  1.0e33;
	*delay_max_ptr = -1.0e33;
	*acc_min_ptr =  1.0e33;
	*acc_max_ptr = -1.0e33;

	dlunit	= FCALUNIT;
	sprintf(omode, "r");

	/*-------- OPEN DELAY FILE --------*/
	sprintf(fname, "CALIB/FCAL.%d", stn_id );
	cfs287_(fname, &ret, strlen(fname) ); cfs_ret( 287, ret );

	cfs103_(&dlunit, fname, omode, &ret, strlen(fname), strlen(omode) );
	if( cfs_ret( 103, ret ) == -1){
		printf("Warning: Can't Access to %s !!\n", fname);
		cfs104_( &dlunit, &ret );	cfs_ret( 104, ret );
		return(0);
	}


	/*-------- READ HEADER ITEMS --------*/
	recid	= 0;	nival	= 2;	nrval	= 0;	ncval	= 64;
	int_ptr	= (int *)malloc( nival*sizeof(int) );

	cfs125_( &dlunit, &recid, &nival, int_ptr, &nrval, &rvalue,
				&ncval, cvalue, &ret, ncval );
	if( cfs_ret( 125, ret ) == -1 ){
		printf("Warning: Can't Access to %s !!\n", fname);
		cfs104_( &dlunit, &ret );	cfs_ret( 104, ret );
		return(0);
	}

#ifdef DEBUG
	printf("CFS125 [REDID=%2d] : NIVAL = %d, NRVAL = %d, NCVAL = %d\n",
			recid, nival, nrval, ncval);
	printf("OBSCODE = %s\n", &cvalue[0] ); 
	printf("STATION = %s\n", &cvalue[32] ); 
	printf("STANUM  = %d\n", int_ptr[0] ); 
	printf("NSS     = %d\n", int_ptr[1] ); 
#endif
	
	/*-------- READ CHARACTER DATA --------*/
	sscanf( &cvalue[0], "%s", obs_name);
	sscanf( &cvalue[32], "%s", stn_name);

	/*-------- READ INTEGER DATA --------*/
	stn_index	= int_ptr[0];
	ssnum		= int_ptr[1];
	free(int_ptr);

	/*-------- READ DELAY DATA --------*/
	nival	= 1;	nrval	= 2 + ssnum*6;	ncval	= 32;
	int_ptr		= (int *)malloc( nival*sizeof(int) );
	double_ptr	= (double *)malloc( nrval*sizeof(double) );

	dldata_num	= 0;

	while(1){

		/*-------- READ FROM CFS --------*/
		cfs125_( &dlunit, &recid, &nival, int_ptr, &nrval, double_ptr,
			&ncval, cvalue, &ret, ncval );

#ifdef DEBUG
		printf("CFS125 [REDID=%2d] : NIVAL = %d, NRVAL = %d, NCVAL = %d\n",
			recid, nival, nrval, ncval);
		for( data_index=0; data_index<nival; data_index++){
			printf(" INT  VAL [%2d] = %d\n", data_index, int_ptr[data_index]);
		}
		for( data_index=0; data_index<nrval; data_index++){
			printf(" REAL VAL [%2d] = %e\n",
			data_index, double_ptr[data_index]);
		}

#endif

		if(ret == 55555){	break;}

		if( *int_ptr == stn_index){		/* In Case of REFANT */
			dldata_num	= -1;
			break;
		}

		/*-------- ALLOCATE MEMORY FOR THE DATA --------*/
		fcal_ptr = (struct fcal_data *)malloc( sizeof(struct fcal_data) );
		if( fcal_ptr == NULL ){
			printf(" MEMORY ERROR at INDEX = %d\n", dldata_num );
			exit(0);
		}

		/*-------- REMEMBER THE INITIAL POINTER OF LINKED LIST --------*/
		if( dldata_num == 0 ){
			*first_fcal_ptr	= (int)fcal_ptr;
			prev_fcal_ptr	= fcal_ptr;
		}

		/*-------- CHECK OBJECT NAME --------*/
		if( (strstr( cvalue, obj_name ) != NULL) ||
			(strstr( obj_name, "all") != NULL)	){

			sprintf(fcal_ptr->objnam, "%s", cvalue);

			fcal_ptr->refant	= *int_ptr;
			fcal_ptr->mjd		= double_ptr[0];
			fcal_ptr->time_incr	= double_ptr[1];
			fcal_ptr->rate		= double_ptr[2];
			fcal_ptr->rate_err	= double_ptr[2+ssnum];
			fcal_ptr->delay		= double_ptr[2+2*ssnum];
			fcal_ptr->delay_err	= double_ptr[2+3*ssnum];
			fcal_ptr->acc		= double_ptr[2+4*ssnum];
			fcal_ptr->acc_err	= double_ptr[2+5*ssnum];

			if(fcal_ptr->mjd > *mjd_max_ptr ){ *mjd_max_ptr = fcal_ptr->mjd; }
			if(fcal_ptr->mjd < *mjd_min_ptr ){ *mjd_min_ptr = fcal_ptr->mjd; }
			if(fcal_ptr->rate > *rate_max_ptr){ *rate_max_ptr= fcal_ptr->rate;}
			if(fcal_ptr->rate < *rate_min_ptr){ *rate_min_ptr= fcal_ptr->rate;}
			if(fcal_ptr->delay> *delay_max_ptr){*delay_max_ptr=fcal_ptr->delay;}
			if(fcal_ptr->delay< *delay_min_ptr){*delay_min_ptr=fcal_ptr->delay;}
			if(fcal_ptr->acc  > *acc_max_ptr ){ *acc_max_ptr=fcal_ptr->acc; }
			if(fcal_ptr->acc  < *acc_min_ptr ){ *acc_min_ptr=fcal_ptr->acc; }

			/*-------- LINK TO THE NEXT POINTER --------*/
			prev_fcal_ptr->next_fcal_ptr	= fcal_ptr;
			fcal_ptr->next_fcal_ptr			= NULL;
			prev_fcal_ptr					= fcal_ptr;
			dldata_num++;
		}
	}


	/*-------- CLOSE DELAY FILE --------*/
	free(int_ptr);
	free(double_ptr);
	cfs104_( &dlunit, &ret );	cfs_ret( 104, ret );

	return(dldata_num);
}
