/*********************************************************
**	READ_CORHEAD.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>

int	read_sshead( bl_index, ss_index,
		rf_ptr,	freq_incr_ptr,	freq_num_ptr,	time_num_ptr,	time_incr_ptr)

	int		bl_index;				/* Baseline Index */
	int		ss_index;				/* Sub-Stream Index */
	double	*rf_ptr;				/* RF Frequency [MHz] */
	double	*freq_incr_ptr;			/* Frequency Increment [MHz] */
	int		*freq_num_ptr;			/* Number of freq. channels */
	int		*time_num_ptr;			/* Number of time points */
	double	*time_incr_ptr;			/* Time Increment [sec] */
{
	int		lunit;
	int		ret;		/* Return Code from CFS Library */
	int		nival;
	int		nrval;
	int		ncval;
	int		ivalue;
	double	rvalue;
	char	keywd[32];
	char	cvalue[80];
	char	omode[2];				/* CFS Access Mode */
	char	fname[32];				/* CFS File Name */

	lunit	= 3;
	sprintf(fname, "CORR.%d/SS.%d/HEADDER.1\0", bl_index, ss_index ); 
	sprintf(omode, "r"); 

	/*-------- OPEN SS-HEADER --------*/
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&lunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- RF (SKY) FREQUENCY --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "rfs" );
	nival	= 0;	nrval	= 1;	ncval	= 0;
	cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, rf_ptr,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	/*-------- FREQUENCY INCREMENT --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "rfd" );
	nival	= 0;	nrval	= 1;	ncval	= 0;
	cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, freq_incr_ptr,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	/*-------- FREQUENCY CHANNEL NUM --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "nch" );
	nival	= 1;	nrval	= 0;	ncval	= 0;
	cfs117_( &lunit, keywd, &nival, freq_num_ptr, &nrval, &rvalue,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	/*-------- TIME NUM --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "nrec" );
	nival	= 1;	nrval	= 0;	ncval	= 0;
	cfs117_( &lunit, keywd, &nival, time_num_ptr, &nrval, &rvalue,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	/*-------- TIME INCREMENT --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "tintg" );
	nival	= 0;	nrval	= 1;	ncval	= 0;
	cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, time_incr_ptr,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	/*-------- CLOSE SS-HEADER --------*/
	cfs104_( &lunit, &ret );	cfs_ret( 117, ret );

	return(0);
}
