/*************************************************************
**	COMP2CORNER.C : Brightness Distribution Generated with 	** 
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

long	comp2corner( gaus_comp, x_corner, y_corner)

	double	gaus_comp[6];	/* INPUT : Gaussian Component */
	float	x_corner[4];	/* OUTPUT: X-AXIS of Corners */
	float	y_corner[4];	/* OUTPUT: Y-AXIS of Corners */
{
	double	center_radius;	/* Distance of the Gaussian Center from Origin */ 
	double	major_axis;		/* Major Axis of the Gaussian */ 
	double	minor_axis;		/* Minor Axis of the Gaussian */ 
	double	x_center;		/* X-Axis of the Center of the Gaussian */ 
	double	y_center;		/* Y-Axis of the Center of the Gaussian */ 
	double	x_offset1;		/* X-Axis from the Center of the Gaussian */ 
	double	x_offset2;		/* X-Axis from the Center of the Gaussian */ 
	double	y_offset1;		/* Y-Axis from the Center of the Gaussian */ 
	double	y_offset2;		/* Y-Axis from the Center of the Gaussian */ 
	double	cs_phi;			/* COS( PA ) */
	double	sn_phi;			/* SIN( PA ) */

	center_radius	= gaus_comp[1];
	major_axis		= 0.5*gaus_comp[3];
	minor_axis		= major_axis * gaus_comp[4];
	x_center	= center_radius * sin(gaus_comp[2]);
	y_center	= center_radius * cos(gaus_comp[2]);
	cs_phi		= sin( gaus_comp[5] );
	sn_phi		= cos( gaus_comp[5] );
	x_offset1	= cs_phi*major_axis - sn_phi*minor_axis;
	x_offset2	= cs_phi*major_axis + sn_phi*minor_axis;
	y_offset1	= sn_phi*major_axis + cs_phi*minor_axis;
	y_offset2	= sn_phi*major_axis - cs_phi*minor_axis;

	x_corner[0]	= (float)(x_center + x_offset1);
	x_corner[1]	= (float)(x_center - x_offset2);
	x_corner[2]	= (float)(x_center - x_offset1);
	x_corner[3]	= (float)(x_center + x_offset2);
	y_corner[0]	= (float)(y_center + y_offset1);
	y_corner[1]	= (float)(y_center - y_offset2);
	y_corner[2]	= (float)(y_center - y_offset1);
	y_corner[3]	= (float)(y_center + y_offset2);
/*
------------------------------------------ CALC BRIGHTNESS at EACH GRID
*/
	return(0);
}
