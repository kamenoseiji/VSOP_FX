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

/*-------- AIPS-Like Color Table in cpgimag() --------*/
static float pg_lumi[20] = {0.0, 0.1, 0.1, 0.2, 0.2, 0.3, 0.3, 0.4, 0.4, 0.5,
							0.5, 0.6, 0.6, 0.7, 0.7, 0.8, 0.8, 0.9, 0.9, 1.0};
static float pg_red[20]  = {0.0, 0.0, 0.3, 0.3, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0,
							0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
static float pg_green[20]= {0.0, 0.0, 0.3, 0.3, 0.0, 0.0, 0.0, 0.0, 0.8, 0.8,
							0.6, 0.6, 1.0, 1.0, 1.0, 1.0, 0.8, 0.8, 0.0, 0.0};
static float pg_blue[20] = {0.0, 0.0, 0.3, 0.3, 0.7, 0.7, 0.7, 0.7, 0.9, 0.9,
							0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
/*
static float pg_lumi[20] = {0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35,
							0.40, 0.45, 0.50, 0.55, 0.60, 0.65, 0.70, 0.75,
							0.80, 0.85, 0.90, 0.95};
static float pg_red[20]  = {0.00, 0.15, 0.30, 0.40, 0.50, 0.50, 0.00, 0.00,
							0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 1.00, 1.00,
							1.00, 1.00, 1.00, 1.00};
static float pg_green[20]= {0.00, 0.15, 0.3, 0.3, 0.0, 0.0, 0.0, 0.0, 0.8, 0.73,
							0.67, 0.6, 1.0, 1.0, 1.0, 1.0, 0.9, 0.8, 0.0, 0.0};
static float pg_blue[20] = {0.0, 0.15, 0.3, 0.3, 0.7, 0.7, 0.7, 0.77, 0.83, 0.9,
							0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
*/

long	cpgpseud( z_ptr, nx, ny, arr_y, x_incr, y_incr, zmax )

	float	*z_ptr;				/* POINTER OF VALUE TO PLOT		*/
	long	nx;					/* DIMENSION NUMBER FOR X		*/
	long	ny;					/* DIMENSION NUMBER FOR Y		*/
	long	arr_y;				/* DIMENSION NUMBER FOR Y		*/
	float	x_incr;				/* Increment of X-Axis			*/
	float	y_incr;				/* Increment of Y-Axis			*/
	float	zmax;				/* MAXIMUM OF Z */
{ 
	int		x_index;			/* Index for X-Offset			*/
	int		y_index;			/* Index for X-Offset			*/
	int		ptr_index1;			/* Pointer of Image Data Set	*/
	int		ptr_index2;			/* Pointer of Original Data Set	*/
	double	plot_x_incr;		/* Increment of Major Ticks		*/
	double	plot_y_incr;		/* Increment of Major Ticks		*/
	float	tr[6];				/* Transfer Matrix				*/
	float	x_min, x_max;		/* Minimum and Maximum of X-AXIS*/
	float	y_min, y_max;		/* Minimum and Maximum of Y-AXIS*/
	float	*amp_ptr;			/* Amplitude to be plot			*/

/*
------------------------------------- PREPARE MEMORY 
*/
	amp_ptr = (float *)malloc( nx* ny* sizeof(float) ); 
	memset( amp_ptr, 0, nx* ny* sizeof(float) );
/*
------------------------------------- DRAW AXIS (x,z)=0 and (y,z)=0
*/
	for( x_index=0; x_index< nx; x_index++){
		for( y_index=0; y_index< ny; y_index++){

			ptr_index1 = ny* x_index +  y_index;
			ptr_index2 = arr_y* x_index + y_index;
			amp_ptr[ptr_index1] = z_ptr[ptr_index2];
		}
	}
/*
------------------------------------- TRANSFER MATRIX
*/
	x_min = -x_incr* (nx/2 + 0.5);		x_max = - x_min;
	y_min = -y_incr* (ny/2 + 0.5);		y_max = - y_min;

	tr[0] = x_min;	tr[1] = x_incr;	tr[2] = 0.0;	
	tr[3] = y_min;	tr[4] = 0.0;	tr[5] = y_incr;	

/*
	for( x_index=0; x_index< nx; x_index++){
		for( y_index=0; y_index< arr_y; y_index++){

		}
	}
------------------------------------- DRAW AXIS (x,z)=0 and (y,z)=0
*/
	cpgbbuf();
	cpgeras();
	cpgsvp( 0.07, 0.70, 0.1, 0.9);
	cpg_incr( (x_max - x_min), &plot_x_incr);
	cpg_incr( (y_max - y_min), &plot_y_incr);
	cpgswin( x_min, x_max, y_min, y_max);
	cpgbox( "BCINTS", plot_x_incr, 10, "BCINTS", plot_y_incr, 10 ); 
	cpglab("Residual Delay [\\gmsec]", "Residual Rate [\\gmsec/sec]",
			"Result from Fringe Search");

	cpgctab( pg_lumi, pg_red, pg_green, pg_blue, 20, 1.0, 0.5 );
/*
	cpgimag( amp_ptr, nx, ny, 1, nx, 1, ny, 0.0, zmax, tr);
*/
	cpggray( amp_ptr, nx, ny, 1, nx, 1, ny, zmax, 0.0, tr);

	cpgwedg( "RG", 1.0, 3.0, zmax, 0.0, "Correlation Coefficient" );
	cpgebuf();
/*
------------------------------------- ENDING
*/
	free(amp_ptr);
	return(0);
}
