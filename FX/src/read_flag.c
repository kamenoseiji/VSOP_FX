/*********************************************************
**	READ_FLAG.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#define	CORRFLAG	5

int	read_flag( bl_index, ss_index, freq_num,	start_pp, integ_pp,
			mjd_ptr, objnum_ptr,	uvw_ptr,	flag_ptr)

	int		bl_index;				/* Baseline Index */
	int		ss_index;				/* Sub-Stream Index */
	int		freq_num;				/* Number of Freq. Channels */
	int		start_pp;				/* PP Number for START */
	int		integ_pp;				/* PP Number for INTEG */
	double	*mjd_ptr;				/* Pointer of MJD */
	int		*objnum_ptr;			/* Pointer of Object Index */
	double	*uvw_ptr;				/* Pointer of U, V, W */
	unsigned char	*flag_ptr;		/* Flag */
{
	int		lunit;
	int		time_index;
	int		freq_index;
	int		ret;					/* Return Code from CFS Library */
	char	omode[2];				/* CFS Access Mode */
	char	fname[32];				/* CFS File Name */

	lunit	= CORRFLAG;
	sprintf(omode, "r"); 
	sprintf(fname, "CORR.%d/SS.%d/FLAG.1", bl_index, ss_index ); 

	/*-------- OPEN FLAG FILE --------*/
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&lunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- SKIP FLAG DATA --------*/
	for(time_index=0; time_index<start_pp; time_index++){
		cfs225_( &lunit, mjd_ptr, objnum_ptr, uvw_ptr, flag_ptr,
				&freq_num, &ret );
		cfs_ret( 225, ret );
	}

	/*-------- READ FLAG DATA --------*/
	for(time_index=0; time_index<integ_pp; time_index++){
		cfs225_( &lunit, mjd_ptr, objnum_ptr, uvw_ptr, flag_ptr,
				&freq_num, &ret );
		cfs_ret( 225, ret );
		mjd_ptr++;
		objnum_ptr++;
		uvw_ptr += 3;
		flag_ptr += freq_num;
	}

	/*-------- CLOSE FLAG FILE --------*/
	cfs104_( &lunit, &ret );	cfs_ret( 117, ret );

	return(0);
}
