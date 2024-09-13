/*********************************************************
**	CPGBIRD :	SUBROUTINE FOR 3-DIMENTIONAL PLOT 		**
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

long	cpgbird( z_ptr, nx, ny, arr_y, zmax, longitude, declination, color )
	float	*z_ptr;				/* POINTER OF VALUE TO PLOT	*/
	long	nx;					/* DIMENSION NUMBER FOR X	*/
	long	ny;					/* DIMENSION NUMBER FOR Y	*/
	long	arr_y;				/* DIMENSION NUMBER FOR Y	*/
	float	zmax;				/* MAXIMUM OF Z				*/
	float	longitude;			/* LONGTUDE [rad]			*/
	float	declination;		/* DECLINATION [rad]		*/
	long	color;				/* COLOR INDEX in PGPLOT	*/
{ 
	float	sin_l, cos_l;		/* Cos and Sin of Longitude		*/
	float	sin_d, cos_d;		/* Cos and Sin of Declinatiln	*/
	float	x, y;				/* Coordinates before rotation	*/
	long	i, j;				/* Suffix of (x, y)				*/
	float	u, v;				/* Coordinates After Rotation	*/
	float	*uu, *vv;			/* (u, v) Value					*/
	float	*px, *py;			/* Data to Plot					*/

	/*-------- ALLOCATE MEMORY FOR PLOT --------*/
	uu = (float *)malloc( nx * (arr_y+1) * sizeof(float) );
	vv = (float *)malloc( nx * (arr_y+1) * sizeof(float) );
	px = (float *)malloc( (ny + 2) * sizeof(float) );
	py = (float *)malloc( (ny + 2) * sizeof(float) );

	memset( uu, 0, nx*(arr_y+1)*sizeof(float));
	memset( vv, 0, nx*(arr_y+1)*sizeof(float));
	memset( px, 0, (ny + 2) * sizeof(float) );
	memset( py, 0, (ny + 2) * sizeof(float) );

	/*-------- ROTATION PARAMETER --------*/
	sin_l = sin(longitude);		cos_l = cos(longitude);
	sin_d = sin(declination);	cos_d = cos(declination);

	cpgsci(color);
/*
------------------------------------- DRAW AXIS (x,z)=0 and (y,z)=0
*/
	x = 1.0; y = 0.0;
	u = x*sin_l			- y * cos_l;
	v = x*sin_d*cos_l	+ y * sin_d * sin_l;
	cpgmove(u, v);

	x = 0.0; y = 0.0;
	u = x*sin_l			- y * cos_l;
	v = x*sin_d*cos_l	+ y * sin_d * sin_l;
	cpgdraw(u, v);

	x = 0.0; y = 1.0;
	u = x*sin_l			- y * cos_l;
	v = x*sin_d*cos_l	+ y * sin_d * sin_l;
	cpgdraw(u, v);
/*
------------------------------------- TRANSFER (x, y) -> (u, v)
*/
	for(i=0; i<nx; i++){
		x = ((float)i)/((float)nx);
		for(j=0; j<ny; j++){
			y = ((float)j)/((float)ny);
			uu[arr_y* i+ j] = x*sin_l 			- y*cos_l;
			vv[arr_y* i+ j] = x*sin_d*cos_l	+ y*sin_d*sin_l
				+ (*z_ptr) / zmax * cos_d;
			z_ptr++;
		}
		z_ptr = z_ptr + (arr_y - ny);
	}
/*
------------------------------------- PLOT DATA
*/
	i=nx-1;
	x = ((float)i)/((float)nx);
	for(j=0; j<ny; j++){
		px[j] = uu[arr_y* i+ j];
		py[j] = vv[arr_y* i+ j];
	}
	y = ((float)(ny-1))/((float)ny);
	px[ny] = x*sin_l          - y*cos_l;
	py[ny] = x*sin_d*cos_l    + y*sin_d*sin_l;
	y = 0;
	px[ny+1] = x*sin_l          - y*cos_l;
	py[ny+1] = x*sin_d*cos_l    + y*sin_d*sin_l;
	cpgsfs(2);
	cpgpoly( ny+2, px, py);

	for(i=nx-2; i>=0; i--){
		x = ((float)i)/((float)nx);

		for(j=0; j<ny; j++){
			cpgmove(uu[arr_y* i+ j], vv[arr_y* i+ j]);
			cpgdraw(uu[arr_y* (i+1)+ j], vv[arr_y*(i+1)+ j]);
		}

		for(j=0; j<ny; j++){
			px[j] = uu[arr_y* i+ j];
			py[j] = vv[arr_y* i+ j];
		}
		y = ((float)(ny-1))/((float)ny);
		px[ny] = x*sin_l          - y*cos_l;
		py[ny] = x*sin_d*cos_l    + y*sin_d*sin_l;
		y = 0;
		px[ny+1] = x*sin_l          - y*cos_l;
		py[ny+1] = x*sin_d*cos_l    + y*sin_d*sin_l;

		cpgsfs(1);
		cpgsci(0);
		cpgpoly( ny+2, px, py);

		cpgsfs(2);
		cpgsci(color);
		cpgpoly( ny+2, px, py);
	}
/*
------------------------------------- DRAW Y-Z FACE
*/
	for(j=ny-1; j>0; j--){
		y = ((float)j)/((float)ny);

		cpgmove(-y*cos_l, y*sin_d*sin_l);
		cpgdraw(uu[j], vv[j]);
		cpgdraw(uu[j-1], vv[j-1]);
	}
/*
------------------------------------- DRAW X-Z FACE
*/
	for(i=0; i<nx-1; i++){
		x = ((float)i)/((float)nx);

		cpgmove(x*sin_l, x*sin_d*cos_l);
		cpgdraw(uu[arr_y* i], vv[arr_y* i]);
		cpgdraw(uu[arr_y*(i+1)], vv[arr_y*(i+1)]);
	}

	free(uu);
	free(vv);
	free(px);
	free(py);

	return(0);
}
