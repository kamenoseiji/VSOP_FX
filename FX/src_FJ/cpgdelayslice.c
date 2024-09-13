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

int	cpgdelayslice( z_ptr, nx, arr_y, x_incr, zmax, max_index )

	float	*z_ptr;				/* POINTER OF VALUE TO PLOT		*/
	long	nx;					/* DIMENSION NUMBER FOR Y		*/
	long	arr_y;				/* DIMENSION NUMBER FOR Y		*/
	float	x_incr;				/* Increment of Y-Axis			*/
	float	zmax;				/* MAXIMUM OF Z					*/
	int		max_index;			/* MAXIMUM OF Z					*/
{ 
	int		x_index;			/* Index for X-Offset			*/
	int		ptr_index1;			/* Pointer of Image Data Set	*/
	int		ptr_index2;			/* Pointer of Original Data Set	*/
	float	x_min, x_max;		/* Minimum and Maximum of Y-AXIS*/
	float	*amp_ptr;			/* Amplitude to be plot			*/
	float	*rate_ptr;			/* Transfer Matrix				*/

/*
------------------------------------- PREPARE MEMORY 
*/
	amp_ptr = (float *)malloc( nx* sizeof(float) ); 
	rate_ptr= (float *)malloc( nx* sizeof(float) ); 
	memset( amp_ptr, 0, nx*sizeof(float) );
	memset(rate_ptr, 0, nx*sizeof(float) );
/*
------------------------------------- DRAW AXIS (x,z)=0 and (y,z)=0
*/
	for( x_index=0; x_index< nx; x_index++){
		ptr_index2 = arr_y* max_index + x_index;	/* DELAY Slice	*/

		amp_ptr[x_index] = z_ptr[ptr_index2];
		rate_ptr[x_index]= x_incr* (float)(x_index - nx/2);
	}
/*
------------------------------------- TRANSFER MATRIX
*/
	x_min = -x_incr* (nx/2 + 0.5);		x_max = - x_min;
/*
------------------------------------- DRAW AXIS (x,z)=0 and (y,z)=0
*/
	cpgbbuf();
	cpgeras();
	cpgsvp( 0.07, 0.70, 0.1, 0.9);
	cpgswin( x_min, x_max, 0.0, 1.2* zmax);
	cpgbox( "BCINTS", 0.0, 0, "BCINTS", 0.0, 0 ); 

	cpglab( "Residual Delay [\\gmsec]", "Corr. Coefficient",
			"Result from Fringe Search");
	cpgline( nx, rate_ptr, amp_ptr );
	cpgebuf();
/*
------------------------------------- ENDING
*/
	free(amp_ptr);
	free(rate_ptr);
	return(0);
}
