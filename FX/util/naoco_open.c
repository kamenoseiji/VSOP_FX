/*************************************************************
**	BLDELAY.C	: BASELINE-BASED FRINGE SEARCH FUNCTION 	**
**															**
**	FUNCTION	: INPUT BASELINE-BASED CORRELATED DATA		**
**					and RETURNS DELAY, DELAY RATE, AMP,		**
**					PHASE, and SNR.							**
**	AUTHOR		: KAMENO Seiji								**
**	CREATED		: 1996/2/26									**
*************************************************************/

#include "naoco.inc"
#include "delaydata.inc"

naoco_open( dir_ptr,		filenum,	bbcnum,	corr_file)

	char	*dir_ptr;			/* Pointer of Directory */
	long	filenum;			/* File Number */
	long	bbcnum;				/* BaseBand Convertor Number */
	FILE	**corr_file;		/* CORR File Pointer */
{

	char	corr_fname[256];	/* CORR Data File Name */

/*
-------------------------------- OPEN CORR DATA FILE
*/
	sprintf(corr_fname, "%s/R%06d.%03d",dir_ptr, filenum, bbcnum);
	if( (*corr_file = fopen(corr_fname, "rb")) == NULL){
		printf("Can't Open CORR DATA File [%s].\n", corr_fname);
		return(-1);
	}

	return(0);
}
