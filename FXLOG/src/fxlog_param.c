/*********************************************************
**	FXLOG_PARAM.C : Command-Line Parameters for FXLOG	** 
**														**
**	FUNCTOIN: READ Command-Line Parameters in FXLOG 	**
**														**
**	INPUT	: long	argc		Number of Input Argument**		
**	INPUT	: char	**argv		Pointer of Arguments	**		
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1995/11/26								**
**********************************************************/

#include "fxlog.inc"
fxlog_param(argc, argv)
	long	argc;
	char	**argv;
{
	char	tty_option[3];
	char	dummy[16];
/*
---------------------------------------------------- LOAD PARAMETERS
*/
	clk			= 8;
	k4			= 0;
	offset 		= 4;
	drg_use_flag = 0;
	strcpy(prog_name, argv[0]);		/* What is The Running Program? */

	/*-------- Command Option --------*/
	while( (argc > 1) && (argv[1][0] == '-') ){
		switch(argv[1][1]) {

		case 'd':   		/* option -d is Drudge File Name */
			sscanf(argv[1], "%02s%s", tty_option, obs_drg_fname);
			/*-------- BLANK AFTER OPTION DESCRIPTOR --------*/
			if( strlen(obs_drg_fname) == 0){
				argc--; argv++;
				sscanf(argv[1], "%s", obs_drg_fname); 
			}
			drg_use_flag = 1;
			break;

		case 'o':   		/* option -o is Offset */
			sscanf(argv[1], "%02s%s", tty_option, dummy);
			/*-------- BLANK AFTER OPTION DESCRIPTOR --------*/
			if( strlen(dummy) == 0){
				argc--; argv++;
				sscanf(argv[1], "%d", &offset); 
			} else {
				sscanf(dummy, "%d", &offset); 
			}

			break;


		case 'c':   		/* option -c is CLOCK */
			sscanf(argv[1], "%02s%s", tty_option, dummy);
			/*-------- BLANK AFTER OPTION DESCRIPTOR --------*/
			if( strlen(dummy) == 0){
				argc--; argv++;
				sscanf(argv[1], "%d", &clk); 
			} else {
				sscanf(dummy, "%d", &clk); 
			}

			/*-------- CHECK CLOCK --------*/
			if( (clk != 8) && (clk != 16) && (clk != 32) ){
				printf(" Error : CLOCK must be 8, 16, or 32 !!\n");
				exit(0);
			}
			break;

		case 'k':   		/* option -d is K4 */
			k4 = 1;
			break;

		}	/* switch */
		argc--; argv++;			/* Next Command-Line Option */
	}
				
/*
---------------------------------------------------- INPUT AND OUTPUT FILE NAME
*/
	switch( argc ){
		case 2:		/* Input File Only */
			strcpy(obs_log_fname, argv[1]);
			sprintf( fx_log_fname, "FXCORR.LOG" );
			printf( "Output LOG File Name is %s.\n", fx_log_fname);
			break;

		case 3:		/* Input and Output File Only */
			strcpy(obs_log_fname, argv[1]);
			strcpy(fx_log_fname, argv[2]);
			break;

		default:	/* Illegal Case */
			printf( "USAGE : fxlog [-option] [input_filename] [output_filename] !!\n");
			printf( "  OPTIONS \n");
			printf( "    -k            : K-4 Type0 Observation\n");
			printf( "    -d [drg_file] : Input Drudge File\n");
			printf( "    -c [clk]      : CLOCK [8/16/32 MHz]\n");
			printf( "    -o [offset]   : ST offset in [sec]\n");
			exit(0);
			break;
	}
/*
---------------------------------------------------- REPLACE COMMA(,) to SPACE 
*/
	sprintf(dum, "sed 's/,/ /g' < %s > hidoi.log", obs_log_fname);
	system(dum);
	sprintf( obs_log_fname, "hidoi.log");

	return(0);
}
