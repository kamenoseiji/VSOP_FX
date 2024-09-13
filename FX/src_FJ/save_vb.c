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

int	save_vb_index( fname,	vbdata_num,	index_ptr,	mjd_ptr,	buf_ptr)
	char	*fname;					/* VB File Name		 			*/
	int		vbdata_num;				/* Number of VB Data			*/
	int		*index_ptr;				/* Pointer of Index				*/
	double	*mjd_ptr;				/* Pointer of MJD				*/
	char	*buf_ptr;				/* First Pointer of Buffer data */
{
	struct	vb_list	*vb_ptr;		/* Pointer of VB List			*/
	struct	vb_list	*prev_vb_ptr;	/* Previous Pointer of VB List	*/
	struct	vb_list	*first_vb_ptr;	/* Previous Pointer of VB List	*/
	int		vbunit;			/* Unit Number in CFS					*/
	int		data_index;		/* Index for Data						*/
	int		buf_len;		/* Buffer Length [byte]					*/
	int		ret;			/* Return Code from CFS Library			*/
	int		recid;			/* Record ID in CFS						*/
	double	mjd;			/* MJD in Data							*/
	char	omode[2];		/* Access Mode of CFS Files				*/
	char	buf[BUFSIZE];	/* Buffer								*/
	char	*header_ptr;	/* Header Pointer						*/


	vbunit	= VBUNIT;
	sprintf(omode, "r");


	for(data_index=0; data_index<vbdata_num; vbdata_index++){
		printf("%2d: INDEX=%2d, MJD=%lf\n",
			data_index, index_ptr[data_index], mjd_ptr[index_ptr[data_index]]);
	}

#ifdef DEBUG
	/*-------- OPEN DELAY FILE --------*/
	cfs287_(fname, &ret, strlen(fname) );	cfs_ret( 287, ret );
	cfs103_(&vbunit, fname, omode, &ret, strlen(fname), strlen(omode) );
	cfs_ret( 103, ret );
	if( ret != 0 ){	return(0);}

	data_index = 0;
	while(1){
		buf_len = BUFSIZE;
		cfs105_( &vbunit, buf, &buf_len, &ret);
		if(ret == 55555){	break;}
		recid	=  buf[3];

		vb_ptr = (struct vb_list *)malloc( sizeof(struct vb_list) );
		if( data_index == 0){
			/* REMEMBER */
			first_vb_ptr= vb_ptr;
			prev_vb_ptr	= vb_ptr;
		}

		vb_ptr->data_index	= data_index;
		vb_ptr->data_size	= buf_len;
		vb_ptr->data_ptr	= (char *)malloc( buf_len );
		memcpy( vb_ptr->data_ptr, buf, buf_len );

		/*-------- DATA RECORD --------*/
		if( recid != 0 ){
			memcpy( &mjd, &buf[4], sizeof(double) );
			vb_ptr->mjd = mjd/86400;
			printf("DATA INDEX = %d, RECID = %d, MJD = %lf, SIZE = %d bytes.\n",
			data_index, recid, mjd/86400,  buf_len);

		/*-------- HEADDER RECORD --------*/
		} else {
			vb_ptr->mjd = 0.0;

			printf("DATA INDEX = %d, RECID = %d, SIZE = %d bytes...\n",
				data_index, recid, buf_len);

			header_ptr	= (char *)malloc( buf_len );
			memcpy( header_ptr, buf, buf_len );
			printf("SIZE = %d : %s\n", sizeof(header_ptr), &header_ptr[4]);
		}

		/*-------- LINK from PREV LIST to CURRENT LIST --------*/
		prev_vb_ptr->next_vb_ptr	= vb_ptr;
		prev_vb_ptr					= vb_ptr;
		vb_ptr->next_vb_ptr			= NULL;

		data_index ++;

	}
	vbdata_num	= data_index;
	cfs104_( &vbunit, &ret );	cfs_ret( 104, ret );

	/*-------- RECONSTRUCT MEMORY MAP --------*/
	buf_ptr	= (int *)malloc( vbdata_num * sizeof(int) );
	mjd_ptr	= (double *)malloc( vbdata_num * sizeof(double) );
	vb_ptr	= first_vb_ptr;
	data_index	= 0;
	while( vb_ptr != NULL ){
		buf_ptr[data_index]	= (int)vb_ptr->data_ptr;
		mjd_ptr[data_index]	= vb_ptr->mjd;
		printf("MJD = %lf: SIZE=%d, RECID=%d\n",
			vb_ptr->mjd, vb_ptr->data_size, vb_ptr->data_ptr[3]);
		vb_ptr = vb_ptr->next_vb_ptr;
		data_index ++;
	}
	*buf_ptr_ptr	= (int)buf_ptr;
	*mjd_ptr_ptr	= (int)mjd_ptr;
#endif

	return(vbdata_num);
}
