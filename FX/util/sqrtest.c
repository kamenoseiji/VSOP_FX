#include <stdio.h>
#include <stdlib.h>
main(argc, argv)
	long	argc;
	char	**argv;
{
	float	a1, a2, a3;
	float	x[3], y[3];

	x[0]	= (float)atof(argv[1]);
	x[1]	= (float)atof(argv[2]);
	x[2]	= (float)atof(argv[3]);
	y[0]	= (float)atof(argv[4]);
	y[1]	= (float)atof(argv[5]);
	y[2]	= (float)atof(argv[6]);

	sqr_fit(	x[0], x[1], x[2],	
				y[0], y[1], y[2],
				&a1, &a2, &a3);
	printf("%f %f %f\n", a1, a2, a3);
	return(0);
}
