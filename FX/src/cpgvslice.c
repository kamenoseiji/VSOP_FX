/*********************************************************
**	CPGPSEUD :	SUBROUTINE FOR PSEUD COLOR PLOT 		**
**														**
**	AUTHOR :	KAMENO S.								**
**	CREATED:	1996/2/5								**
*********************************************************/
#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<math.h>
#include	"cpgplot.h"
#define		MAX_PT	1024
#define		MAX_COLOR	20

int	cpgslice( z_ptr, ny, arr_y, y_incr, zmax, max_index )

	float	*z_ptr;				/* POINTER OF VALUE TO PLOT		*/
	long	ny;					/* DIMENSION NUMBER FOR Y		*/
	long	arr_y;				/* DIMENSION NUMBER FOR Y		*/
	float	y_incr;				/* Increment of Y-Axis			*/
	float	zmax;				/* MAXIMUM OF Z					*/
	int		max_index;			/* MAXIMUM OF Z					*/
{ 
	int		y_index;			/* Index for X-Offset			*/
	int		ptr_index1;			/* Pointer of Image Data Set	*/
	int		ptr_index2;			/* Pointer of Original Data Set	*/
	float	y_min, y_max;		/* Minimum and Maximum of Y-AXIS*/
	float	*amp_ptr;			/* Amplitude to be plot			*/
	float	*rate_ptr;			/* Transfer Matrix				*/
/*
------------------------------------- PREPARE MEMORY 
*/
	amp_ptr = (float *)malloc( ny* sizeof(float) ); 
	rate_ptr= (float *)malloc( ny* sizeof(float) ); 
	memset( amp_ptr, 0, ny*sizeof(float) );
	memset(rate_ptr, 0, ny*sizeof(float) );
/*
------------------------------------- DRAW AXIS (x,z)=0 and (y,z)=0
*/
	for( y_index=0; y_index< ny; y_index++){
/*
		ptr_index2 = arr_y* max_index + y_index;
*/
		ptr_index2 = arr_y *y_index + max_index;

		amp_ptr[y_index] = z_ptr[ptr_index2];
		rate_ptr[y_index]= y_incr* (float)(y_index - ny/2);
	}
/*
------------------------------------- TRANSFER MATRIX
*/
	y_min = -y_incr* (ny/2 + 0.5);		y_max = - y_min;
/*
------------------------------------- DRAW AXIS (x,z)=0 and (y,z)=0
*/
	cpgbbuf();
	cpgeras();
	cpgsvp( 0.07, 0.70, 0.1, 0.9);
	cpgswin( y_min, y_max, 0.0, 1.2* zmax);
	cpgbox( "BCINTS", 0.0, 0, "BCINTS", 0.0, 0 ); 
	cpglab( "Residual Rate [\\gmsec/sec]", "Corr. Coefficient",
			"Result from Fringe Search");

	cpgline( ny, rate_ptr, amp_ptr );
	cpgebuf();
/*
------------------------------------- ENDING
*/
	free(amp_ptr);
	free(rate_ptr);
	return(0);
}
