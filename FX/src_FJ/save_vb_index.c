/*********************************************************
**	SAVE_VB_INDEX.C : SAVE Variable-Binary Data in CFS.	**
**														**
**	FUNCTON	: Save Valiable-Binary Data in CFS with 	**
**				index order. This module does			**
**				not depend on VB format.				**
**														**
**	RETURN	: Number of VB Data							**
**														**
**	AUTHOR  : KAMENO Seiji								**
**	CREATED : 1997/3/4									**
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"
#define	BUFSIZE	65536

#define	VBUNIT	4

int	save_vb_index(	fname,		vbdata_num,		index_ptr,
					mjd_ptr,	size_ptr,		buf_ptr_ptr )
	char	*fname;					/* VB File Name		 			*/
	int		vbdata_num;				/* Number of VB Data			*/
	int		*index_ptr;				/* Pointer of Index				*/
	double	*mjd_ptr;				/* Pointer of MJD				*/
	int		*size_ptr;				/* First Pointer of Buffer Size */
	int		*buf_ptr_ptr;			/* First Pointer of Buffer data */
{
	int		vbunit;			/* Unit Number in CFS					*/
	int		data_index;		/* Index for Data						*/
	int		sorted_index;	/* Index Sorted by MJD Order			*/
	int		buf_len;		/* Buffer Length [byte]					*/
	int		ret;			/* Return Code from CFS Library			*/
	int		recid;			/* Record ID in CFS						*/
	double	mjd;			/* MJD in Data							*/
	char	*buf_ptr;		/* Buffer Pointer						*/
	char	omode[2];		/* Access Mode of CFS Files				*/


	vbunit	= VBUNIT;
	sprintf(omode, "u");


	for(data_index=0; data_index<vbdata_num; data_index++){
		sorted_index	= index_ptr[data_index];
		buf_ptr			= (char *)buf_ptr_ptr[sorted_index];

		recid = buf_ptr[3];

		if( recid != 0){
			memcpy( &mjd, &buf_ptr[4], sizeof(double) );
#ifdef DEBUG
			printf("%2d: INDEX=%2d, RECID=%d, SIZE=%2d, MJD=%lf, MJD=%lf\n",
				data_index, sorted_index, buf_ptr[3], size_ptr[sorted_index],
				mjd_ptr[sorted_index], mjd/86400 );
		} else {
			printf("%2d: INDEX=%2d, RECID=%d, SIZE=%2d, MJD=%lf\n",
				data_index, sorted_index, buf_ptr[3],
				size_ptr[sorted_index],	mjd_ptr[sorted_index]);
#endif
		}
	}

	/*-------- OPEN VB FILE --------*/
	cfs287_(fname, &ret, strlen(fname) );	cfs_ret( 287, ret );
	cfs103_(&vbunit, fname, omode, &ret, strlen(fname), strlen(omode) );
	cfs_ret( 103, ret );
	if( ret != 0 ){	return(0);}

	/*-------- STORE DATA INTO VB FILE --------*/
	for(data_index=0; data_index<vbdata_num; data_index++){
		sorted_index	= index_ptr[data_index];
		buf_ptr			= (char *)buf_ptr_ptr[sorted_index];
		cfs106_( &vbunit, buf_ptr, &size_ptr[sorted_index], &ret );
		cfs_ret( 106, ret );
	}

	/*-------- CLOSE VB FILE --------*/
	cfs104_( &vbunit, &ret );	cfs_ret( 104, ret );

	return(vbdata_num);
}
