/*************************************************
**	FXVISP_PARAM: VISIBILITY PLOT for FX DATA	**
**	AUTHOR:		KAMENO S.						**
**	CREATED:	1995/8/17						**
**************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	"fxvisp_param.inc"

fxvisp_param(argc, argv)	/* INPUTS */
	long	argc;
	char	**argv;
{
	char	dum[256];

    /* CHECH Command Options */
	while( (argc > 1) && (argv[1][0] == '-') ){

		switch(argv[1][1]) {

		case 'e':   /* option -e is EXPERIMENT CODE NAME */
			sscanf(argv[1], "%02s%s", dum, exper_name);
			break;

		case 's':   /* option -s is SUB-STREAM NUMBER */
			ss_num = atoi(&argv[1][2]);
			break;

		case 'f':   /* option -f is START PP NUMBER */
			start_pp_num = atoi(&argv[1][2]);
			break;

		case 'd':   /* option -d is DURATION of PP NUMBER */
			dur_pp_num = atoi(&argv[1][2]);
			break;

		case 'p':   /* option -p is PGPLOT DEVICE NAME */
			sscanf(argv[1], "%02s%s", dum, pgplot_dev);
			break;
		}
		argv++;
		argc--;
	}
}
