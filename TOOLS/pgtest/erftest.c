#include <math.h>
#include <stdio.h>
#include <stdlib.h>

main(argc, argv)
	int		argc;
	char	**argv;
{
	double	x;

	if( argc < 1 ){
		printf("USAGE: erftest [value] !!\n");
		exit(0);
	}

	x = 0.0;

	x = atof(argv[1]);
	printf("ERF[%lf]  = %lf\n", x, erf(x) );
}
