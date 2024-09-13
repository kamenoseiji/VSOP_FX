/*************************************************************************
**	FXVISP: VISIBILITY PLOT for FX DATA									**
**	AUTHOR:		KAMENO S.												**
**	CREATED:	1995/8/17												**
**	Usage:	fxvisp -e[exper_name] -s[ss_num] -f[start_pp] -d[duration]	**
**					-p[pgplot_dev]										**
**  Example: fxvisp -est01 -s6 -f2 -d512 -p/xwin						**
**************************************************************************/

#ifdef	DEBUG
#define	DEBUG_SHOW(x)	x
#else
#define DEBUG_SHOW(x)
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	"fxvisp_param.inc"
#include	"coda.inc"

MAIN__(argc, argv)
	long	argc;
	char	**argv;
{
	/*-------- Accept Command-Line Parameters --------*/
	fxvisp_param(argc, argv);		/* FOLLOWING PARAMETERS ARE TAKEN */ 
									/* exper_name	: Experiment Code Name */ 
									/* ss_num		: Sub-Stream Number */ 
									/* start_pp_num	: Start PP Number */ 
									/* dur_pp_num	: Duration PP Number */ 
									/* pgplot_dev	: PGPLOT Device */ 

	/*-------- Initialize CODA File System --------*/
	cfs000_(&err_code);
	if(err_code != 0){
		printf("ERROR Occured in cfs000! \n");
		exit(0);
	}

	cfs020_(&err_code);
	if(err_code != 0){
		printf("ERROR Occured in cfs020! \n");
		exit(0);
	}

	cfs006_(exper_name, &err_code);
	if(err_code != 0){
		printf("ERROR Occured in cfs006! \n");
		exit(0);
	}

	/*-------- Initialize CODA File System --------*/
	cfs002_(dhome, oname, ofull, srdir, lgdir, crdir, vcdir, cbdir);
	DEBUG_SHOW(printf("Data Home Directory        : %s\n", dhome));
	DEBUG_SHOW(printf("Observation Directory      : %s\n", oname));
	DEBUG_SHOW(printf("Observation Full Path      : %s\n", ofull));
	DEBUG_SHOW(printf("Search Table Sub-Directory : %s\n", srdir));
	DEBUG_SHOW(printf("LOG Directory              : %s\n", lgdir));
	DEBUG_SHOW(printf("Baseline Directory         : %s\n", crdir));
	DEBUG_SHOW(printf("VC Directory               : %s\n", vcdir));
	DEBUG_SHOW(printf("CALIB Directory            : %s\n", cbdir));

	cfs003_(cthed, cttab, srtab, headf, histf, corrf, flagf,
			antef, gcalf, fcalf, pcalf, bcalf, lcalf);
	DEBUG_SHOW(printf("Admin Table Header File    : %s\n", cthed));
	DEBUG_SHOW(printf("Admin Table Table File     : %s\n", cttab));
	DEBUG_SHOW(printf("Search Table File          : %s\n", srtab));
	DEBUG_SHOW(printf("Header File                : %s\n", headf));
	DEBUG_SHOW(printf("History File               : %s\n", histf));
	DEBUG_SHOW(printf("Correlation Data File      : %s\n", corrf));
	DEBUG_SHOW(printf("Flag Data File             : %s\n", flagf));

}
