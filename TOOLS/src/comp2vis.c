#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define PI    3.141592653589793238462643383
#define PI2   6.283185307179586476925286766

long	comp2vis( comp, uv, vis )
	double	comp[6];			/* INPUT: Gaussian Component Parameter */
	double	uv[2];				/* INPUT: Baseline Vector (u, v) */
	double	vis[2];				/* OUTPUT:Visibility Amplitude and Phase */
{
	double			cos_ph, sin_ph;
	double			u, v;
	double			maj2, min2;

	cos_ph = cos(comp[5]);	sin_ph = sin(comp[5]);

	u =   sin_ph*uv[0] + cos_ph*uv[1];
	v =  -cos_ph*uv[0] + sin_ph*uv[1];

	maj2 = comp[3]*comp[3]/4;
	min2 = comp[4]*comp[4]*maj2;

	vis[0] = comp[0] * exp(-( PI2*PI*(maj2*u*u + min2*v*v)));
	vis[1] = PI2 * (uv[0]*comp[1]*sin(comp[2]) + uv[1]*comp[1]*cos(comp[2]));

	return(0);
}
