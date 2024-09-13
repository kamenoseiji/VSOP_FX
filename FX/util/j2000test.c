/*********************************************
**	J2000TEST								**
**	AUTHOUR	: KAMENO Seiji					**
**	CREATED	: 1994/7/25						**
**********************************************/

#define	SEC2RAD	4.84813681109535993589e-6
#define	RAD2SEC	206264.80624709635515647335
#include	<stdio.h>
#include	<stdlib.h>

main(argc, argv)
	int		argc;			/* Number of Arguments */
	char	**argv;			/* Pointer of Arguments */
{
	double	ra_1950;
	double	dec_1950;
	double	ra_2000;
	double	dec_2000;
	char	dec_sign[2];
	long	rh;
	long	rm;
	double	rs;
	long	dd;
	long	dm;
	double	ds;

	if(argc < 7){
		printf("j2000test [RH] [RM] [RS] [DD] [DM] [DS] !!\n");
		return(0);
	}

	ra_2000	= (atof(argv[1])*60.0 + atof(argv[2]))*60.0 + atof(argv[3]);

	ra_2000	*= 15.0*SEC2RAD;
	if(argv[4][0] == '-'){
		printf("HIDOI\n");
		dec_2000= (-60.0*atof(argv[4]) + atof(argv[5]))*60.0 + atof(argv[6]);
		dec_2000	*= -SEC2RAD;
	} else {
		dec_2000= (60.0*atof(argv[4]) + atof(argv[5]))*60.0 + atof(argv[6]);
		dec_2000	*= SEC2RAD;
	}

	j2000tob1950(ra_2000, dec_2000, &ra_1950, &dec_1950);

	ra_1950	*= (RAD2SEC/15.0);

	if(dec_1950 < 0.0){
		printf("HIDOI\n");
		strcpy(dec_sign, "-");
		dec_1950	*= -RAD2SEC;
	} else {
		strcpy(dec_sign, "+");
		dec_1950	*= RAD2SEC;
	}

	rh	= ((long)ra_1950)/3600;
	rm	= (((long)ra_1950)%3600)/60;
	rs	= ra_1950 - (double)((rh*60 + rm)*60);
	dd	= ((long)dec_1950)/3600;
	dm	= (((long)dec_1950)%3600)/60;
	ds	= dec_1950 - (double)((dd*60 + dm)*60);

	printf("R.A. [B1950.0] =  %02d %02d %08.5lf\n", rh, rm, rs);
	printf("DEC. [B1950.0] = %s%02d %02d %08.5lf\n", dec_sign, dd, dm, ds);

	return(0);
}
