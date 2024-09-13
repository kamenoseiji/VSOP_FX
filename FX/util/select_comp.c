/*************************************************************
**	SELECT_COMP.C : Brightness Distribution Generated with 	** 
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

long	select_comp( x_pos, y_pos, ncomp, gaus_comp_ptr)

	float	x_pos;			/* INPUT : X-Axis of Cursor Position */
	float	y_pos;			/* INPUT : Y-Axis of Cursor Position */
	long	ncomp;			/* INPUT : Number of Gaussian Component */
	double	*gaus_comp_ptr;	/* INPUT : Pointer of Gaussian Component */
{
	long	comp_index;		/* Index of the Gaussian Component */
	float	x_center;		/* X Position of Center of the Gaussian */ 
	float	y_center;		/* Y Position of Center of the Gaussian */
	float	dist;			/* Distance from the Center of the Component */
	float	dist_min;		/* Minimum Distance */
	long	comp_min;		/* The Nearest Component Index */
	double	radius;
	double	theta;

	dist_min	= 999999.0;
	for(comp_index=0; comp_index<ncomp; comp_index++){

		gaus_comp_ptr++;
		radius	= *gaus_comp_ptr;	gaus_comp_ptr++;
		theta	= *gaus_comp_ptr;	gaus_comp_ptr += 4;

		/*-------- CENTER POSITION OF THE COMPONENT --------*/
		x_center = radius*sin(theta);
		y_center = radius*cos(theta);

		/*-------- DISTANCE FROM THE CENTER --------*/
		dist	= (x_pos - x_center)*(x_pos - x_center) 
				+ (y_pos - y_center)*(y_pos - y_center);

		/*-------- NEAREST COMPONENT --------*/
		if(dist < dist_min){
			dist_min	= dist;
			comp_min	= comp_index;
		}
	}
	return(comp_min);
}
