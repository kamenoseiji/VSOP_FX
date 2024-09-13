/*********************************************************
**	CFS_RET.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>

int	cfs_ret( cfs_index, cfs_ret )
	int		cfs_index;
	int		cfs_ret;
{
	if( cfs_ret != 0 ){
		printf("Error in CFS%03d...CODE %d !!\n",
			cfs_index, cfs_ret);
		return(-1);
	}
	return(0);
}
