/*********************************************************
**	SQR_FIT.C	: 2-D SQUARE FIT		 				**
**														**
**	FUNCTION: INPUT 5-Point DATA and SOLVE for 2-D 		**
**				Square Polynominal Function				**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/3/14									**
*********************************************************/

long	sqr_fit(	x1, y1, z1,
					x2, y2, z2,
					x3, y3, z3,
					x4, y4, z4,
					x5, y5, z5,
					afact_ptr)

	double	x1, x2, x3, x4, x5;		/* X-Axis Data */
	double	y1, y2, y3, y4, y5;		/* X-Axis Data */
	double	z1, z2, z3, z4, z5;		/* X-Axis Data */
	double	*afact_ptr;
{
	double	p[5][5], z[5];
	double	vw[5];
	double	epsz;
	long	ndim;
	long	isw;
	long	is;
	long	ip[5];
	long	icon;
	long	i;

	p[0][0]=x1*x1; p[1][0]=y1*y1; p[2][0]=x1; p[3][0]=y1; p[4][0]=1.0;
	p[0][1]=x2*x2; p[1][1]=y2*y2; p[2][1]=x2; p[3][1]=y2; p[4][1]=1.0;
	p[0][2]=x3*x3; p[1][2]=y3*y3; p[2][2]=x3; p[3][2]=y3; p[4][2]=1.0;
	p[0][3]=x4*x4; p[1][3]=y4*y4; p[2][3]=x4; p[3][3]=y4; p[4][3]=1.0;
	p[0][4]=x5*x5; p[1][4]=y5*y5; p[2][4]=x5; p[3][4]=y5; p[4][4]=1.0;

	z[0] = z1; 	z[1] = z2; 	z[2] = z3; 	z[3] = z4;	z[4] = z5;

	epsz	= 0.0;	isw	= 1;	ndim=5;
	dlax_(p, &ndim, &ndim, z, &epsz, &isw, &is, vw, ip, &icon);

	for(i=0; i<5; i++){
		*afact_ptr = z[i];
		afact_ptr++;
	}
	return(0);
}
