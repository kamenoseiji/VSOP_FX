/*****************************************************************
**	TSS.C : Calculate TSSID and FRACTION from UTC				**
**	FUNCTION: SOY2TSS accepts UTC time and clock. And calcurate **
**				TSSID and FRACTION.								**
**	AUTHOUR	: KAMENO Seiji										**
**	CREATED	: 1994/7/25											**
******************************************************************/

#include <stdio.h>

main( argc, argv)
	int		argc;					/* Number of Arguments	*/
	char	**argv;					/* Pointer of Arguments	*/
{
	/*-------- Variables for Time --------*/
	unsigned long	clk;			/* System Clock [8, 16, or 32 MHz]	*/
	unsigned long	doy;			/* Day of Year				*/
	unsigned long	hour;			/* Hour						*/
	unsigned long	min;			/* Minute					*/
	unsigned long	sec;			/* Second					*/
	unsigned long	soy;			/* Second of Year			*/
	unsigned long	tss;			/* TSS ID					*/
	unsigned long	frc;			/* TSS Fraction				*/

	/*-------- Characters/String --------*/
	char	dummy[256];				/* Dummy Input Buffer		*/
/*
---------------------------------------- PARSE INPUT PARAMETER
*/
	if(argc < 3){
		printf("USAGE: tss -c [CLOCK] -d [DDDHHMMSS] !!\n");
		exit(0);
	}

	while( (argc>1) && (argv[1][0] == '-') ){
		switch(argv[1][1]){

		case 'c':		/* Clock in [MHz]		*/
			sscanf(argv[1], "%*02s%s", dummy);
			/*-------- BLANK AFTER OPTION DESCRIPTOR --------*/
			if( strlen(dummy) == 0 ){
				argc--; argv++;
				sscanf(argv[1], "%d", &clk);
			} else {
				sscanf(dummy, "%d", &clk);
			}
			break;

		case 'd':		/* DDDHHMMSS in UTC		*/
			sscanf(argv[1], "%*02s%s", dummy);
			/*-------- BLANK AFTER OPTION DESCRIPTOR --------*/
			if( strlen(dummy) == 0 ){
				argc--; argv++;
				sscanf(argv[1], "%03d%02d%02d%02d", &doy, &hour, &min, &sec);
			} else {
				sscanf(dummy, "%03d%02d%02d%02d", &doy, &hour, &min, &sec);
			}
			break;


		default:
			printf("Unknown Option --- [-%c]\n", argv[1][1]);

		}	/*-------- End of Switch --------*/
		argc--; argv++;
	}
/*
---------------------------------------- CHECK INPUT PARAMETER
*/
	/*-------- CHECK FOR CLOCK --------*/
	if( (clk != 8) && (clk != 16) && (clk != 32) ){
		printf("Clock must be 8, 16 or 32 !!\n");	exit(0); }

	/*-------- CHECK FOR DOY --------*/
	if( (doy < 1) || (doy > 400)){
		printf("Day of Year must be 0 - 400 !!\n");	exit(0); }

	/*-------- CHECK FOR Hour --------*/
	if( (hour < 0) || (hour > 24)){
		printf("Hour must be 0 - 24 !!\n");	exit(0); }

	/*-------- CHECK FOR Minute --------*/
	if( (min < 0) || (min > 60)){
		printf("Minute must be 0 - 60 !!\n");	exit(0); }

	/*-------- CHECK FOR Second --------*/
	if( (sec < 0) || (sec > 60)){
		printf("Second must be 0 - 60 !!\n");	exit(0); }
/*
---------------------------------------- CONVERSION to TSS
*/
	dhms2soy( doy, hour, min, sec, &soy );	/* DDDHHMMSS -> SOY		*/
	soy2tss( soy, clk, &tss, &frc );		/* SOY -> TSS			*/
/*
---------------------------------------- ENDING
*/
	printf("UTC   = %03d%02d%02d%02d\n", doy, hour, min, sec);
	printf("CLOCK = %d\n", clk);
	printf("TSS   = %d\n", tss);
	printf("FRC   = %d\n", frc);
	return(0);
}
