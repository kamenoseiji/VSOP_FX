/*********************************************************
**	WRIGHT_SSHEAD.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>

int	write_sshead( version, bl_index, ss_index,
		rf_ptr,	freq_incr_ptr,	freq_num_ptr,	time_num_ptr,	time_incr_ptr)

	int		version;				/* File Versiont				*/
	int		bl_index;				/* Baseline Index				*/
	int		ss_index;				/* Sub-Stream Index				*/
	double	*rf_ptr;				/* RF Frequency [MHz]			*/
	double	*freq_incr_ptr;			/* Frequency Increment [MHz]	*/
	int		*freq_num_ptr;			/* Number of freq. channels		*/
	int		*time_num_ptr;			/* Number of time points		*/
	double	*time_incr_ptr;			/* Time Increment [sec]			*/
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
	char	omode[2];				/* CFS Access Mode				*/
	char	fname[128];				/* CFS File Name				*/
	char	category[128];			/* CFS File Category			*/
	char	ftype[4];				/* CFS File Type				*/
	int		finfo[5];				/* File Information				*/

	lunit	= 7;
	sprintf(fname, "CORR.%d/SS.%d/HEADDER.%d\0", bl_index, ss_index, version ); 
	sprintf(omode, "w"); 
	sprintf(ftype, "AT"); 
	sprintf(category, "SS_HEAD"); 
	finfo[0]	= 1;
	finfo[1]	= 0;
	finfo[2]	= 0;
	finfo[3]	= 0;
	finfo[4]	= 8;
	printf("FILE = %s\n", fname);
/******NOW EDITING HERE *****/

	/*-------- OPEN SS-HEADER --------*/
	cfs102_(fname, &ret, strlen(fname));	cfs_ret( 102, ret );
	cfs101_(fname, category, ftype, omode, finfo, &ret );	cfs_ret(101, ret);
	cfs103_(&lunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- RF (SKY) FREQUENCY --------*/
	sprintf( keywd, "rfs" );
	nival	= 0;	nrval	= 1;	ncval	= 0;
	cfs116_( &lunit, keywd, &nival, &ivalue, &nrval, rf_ptr,
			&ncval, cvalue, &ret, strlen(keywd), ncval );
	cfs_ret( 116, ret );

	/*-------- FREQUENCY INCREMENT --------*/
	sprintf( keywd, "rfd" );
	nival	= 0;	nrval	= 1;	ncval	= 0;
	cfs116_( &lunit, keywd, &nival, &ivalue, &nrval, freq_incr_ptr,
			&ncval, cvalue, &ret, strlen(keywd), ncval );
	cfs_ret( 116, ret );

	/*-------- FREQUENCY CHANNEL NUM --------*/
	sprintf( keywd, "nch" );
	nival	= 1;	nrval	= 0;	ncval	= 0;
	cfs116_( &lunit, keywd, &nival, freq_num_ptr, &nrval, &rvalue,
			&ncval, cvalue, &ret, strlen(keywd), ncval );
	cfs_ret( 116, ret );

	/*-------- TIME NUM --------*/
	sprintf( keywd, "nrec" );
	nival	= 1;	nrval	= 0;	ncval	= 0;
	cfs116_( &lunit, keywd, &nival, time_num_ptr, &nrval, &rvalue,
			&ncval, cvalue, &ret, strlen(keywd), ncval );
	cfs_ret( 116, ret );

	/*-------- TIME INCREMENT --------*/
	sprintf( keywd, "tintg" );
	nival	= 0;	nrval	= 1;	ncval	= 0;
	cfs116_( &lunit, keywd, &nival, &ivalue, &nrval, time_incr_ptr,
			&ncval, cvalue, &ret, strlen(keywd), ncval );
	cfs_ret( 116, ret );

	/*-------- STOKES PARAMETER --------*/
	sprintf( keywd, "stokes" );
	nival	= 0;	nrval	= 0;
	sprintf(cvalue, "rr");	ncval = strlen(cvalue);
	cfs116_( &lunit, keywd, &nival, &ivalue, &nrval, &rvalue,
			&ncval, cvalue, &ret, strlen(keywd), strlen(cvalue) );
	cfs_ret( 116, ret );

	/*-------- TONE PARAMETER --------*/
	sprintf( keywd, "ntone" );
	nival	= 1;	nrval	= 0;	ncval	= 0;
	ivalue	= 0;
	cfs116_( &lunit, keywd, &nival, &ivalue, &nrval, &rvalue,
			&ncval, cvalue, &ret, strlen(keywd), ncval );
	cfs_ret( 116, ret );

	/*-------- CLOSE SS-HEADER --------*/
	cfs104_( &lunit, &ret );	cfs_ret( 104, ret );

	return(0);
}
