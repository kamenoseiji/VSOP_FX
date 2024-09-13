/*************************************************************
**	COMP2IMAG.C : Brightness Distribution Generated with 	** 
**					Gaussian Components.					**
**															**
**	FUNCTION: INPUT GAUSSIAN COMPONENT and CALCULATE 		**
**				BRIGHTNESS at GRID POINTS.					**
**	AUTHOR	: KAMENO Seiji									**
**	CREATED	: 1996/3/14										**
*************************************************************/
#include	<stdio.h>
#include	<math.h>
#define MAS2RAD 4.84813681109535993589e-9
#define FWHM2SIGMA 0.84932180028801904272

long	comp2imag(nx, ny, x_incr, y_incr, gaus_comp, imag_ptr)

	long	nx;				/* INPUT : Number of X-Axis */
	long	ny;				/* INPUT : Number of X-Axis */
	double	x_incr;			/* INPUT : Increment of X-Axis */
	double	y_incr;			/* INPUT : Increment of Y-Axis */
	double	gaus_comp[6];	/* INPUT : Gaussian Component */
	float	*imag_ptr;		/* IN/OUT: Pointer of Brightness Distribution */
{
	long	x_index;		/* X-Axis Index */ 
	long	y_index;		/* Y-Axis Index */
	double	center_radius;	/* Center Position from Original */
	double	major_axis;		/* Major Axis of the Gaussian */
	double	minor_axis;		/* Minor Axis of the Gaussian */
	double	x_position;		/* X Position of each grid */ 
	double	y_position;		/* Y Position of each grid */
	double	x_offset;		/* X Offset of each grid from the Gaussian Center */
	double	y_offset;		/* Y Offset of each grid from the Gaussian Center */
	double	x, y;			/* X and Y offset in the Gauusian Coordinate */
	double	cs_phi;			/* COS( PA of the Major Axis ) */
	double	sn_phi;			/* SIN( PA of the Major Axis ) */

	center_radius	= gaus_comp[1];
	major_axis		= gaus_comp[3]*FWHM2SIGMA;
	minor_axis		= major_axis * gaus_comp[4];
	x_offset	= center_radius * sin(gaus_comp[2]);
	y_offset	= center_radius * cos(gaus_comp[2]);
	cs_phi		= sin( gaus_comp[5] );
	sn_phi		= cos( gaus_comp[5] );

/*
------------------------------------------ CALC BRIGHTNESS at EACH GRID
*/
	for(x_index=0; x_index<nx; x_index++){
		x_position	= (double)(x_index - nx/2)*x_incr - x_offset;
		for(y_index=0; y_index<ny; y_index++){
			y_position	= (double)(y_index - ny/2)*y_incr - y_offset;

			/*-------- Coordinate along the Major and Minor Axis --------*/ 
			x	=2.0*( cs_phi*x_position + sn_phi*y_position)/major_axis;
			y	=2.0*(-sn_phi*x_position + cs_phi*y_position)/minor_axis;

			*imag_ptr += (float)(gaus_comp[0]*exp(-0.5*(x*x + y*y)));
			imag_ptr++;
		}
	}

	return(0);
}
