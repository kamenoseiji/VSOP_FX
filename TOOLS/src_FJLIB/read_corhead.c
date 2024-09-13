/*********************************************************
**	READ_CORHEAD.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>

int	read_corhead( bl_index, ant_ptr, ssnum_ptr, loss_ptr )
	int		bl_index;				/* Baseline Index */
	int		*ant_ptr;				/* Antenna ID */
	int		*ssnum_ptr;				/* Number of Sub-Stream */
	double	*loss_ptr;				/* Quantize Efficiency */
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
	sprintf(fname, "CORR.%d/HEADDER", bl_index); 
	sprintf(omode, "r"); 

	/*-------- OPEN CORR-HEADER --------*/
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&lunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- ANTENNA ID --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "numa1" );
	nival	= 1;	nrval	= 0;	ncval	= 0;
	cfs117_( &lunit, keywd, &nival, ant_ptr, &nrval, &rvalue,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	ant_ptr++;
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "numa2" );
	nival	= 1;	nrval	= 0;	ncval	= 0;
	cfs117_( &lunit, keywd, &nival, ant_ptr, &nrval, &rvalue,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	/*-------- NUMBER OF SUB-STREAM --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "nss" );
	nival	= 1;	nrval	= 0;	ncval	= 0;
	cfs117_( &lunit, keywd, &nival, ssnum_ptr, &nrval, &rvalue,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	/*-------- QUANTIZE EFFICIENCY --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "closs" );
	nival	= 0;	nrval	= 1;	ncval	= 0;
	cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, loss_ptr,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	/*-------- CLOSE CORR-HEADER --------*/
	cfs104_( &lunit, &ret );	cfs_ret( 117, ret );

	return(0);
}
