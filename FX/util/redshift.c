#include	<stdio.h>
#include	<math.h>

main(argc, argv)
	int		argc;		/* Number of Arguments	*/
	char	**argv;		/* Pointer of Arguments	*/
{
	double	q0;			/* Cosmological Density Parameter	*/
	double	z;			/* Redshift							*/
	double	h0;			/* Hubble Constant [km/sec/pc]		*/
	double	c;			/* Light Velocity [km/sec]			*/
	double	d;			/* Linear Scale [pc/mas]			*/
	double	rad;		/* mas per radian					*/

	q0	= 0.5;
	h0	= 1.0e-4;
	c	= 299792.458;
	rad	= 206264806.2;

	if(argc < 2){
		printf("USAGE: redshift [z] !!\n");
		exit(0);
	}

	z = atof( argv[1] );

	d = c * (q0* z + (q0 - 1.0)*(sqrt(1.0 + 2.0* q0* z) - 1.0));
	d /= (h0* q0* q0* (1.0 + z)*(1.0 + z));
	d /= rad;

	printf("z=%lf : Linear Size = %lf pc/mas \n", z, d);
}
