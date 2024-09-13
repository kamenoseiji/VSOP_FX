/*********************************************************
**	FXLOG_WRITE_SOURCE.C : Write SOURCE Information		**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"

long fxlog_write_source( src )
	struct	source_line	src;
{
	char	sign[2];			/* Sign of Declination */
	long	soy;
	switch(src.sign){
		case 1:		sprintf(sign, "+\0");	break;
		case -1:	sprintf(sign, "-\0");	break;
		default:	sprintf(sign, "+\0");	break;
	}
/*
---------------------------------------------------- WRITE
*/
	/*-------- UTC for SOURCE should be 4*n --------*/
	dhms2soy( doy_obslog, hh_obslog, mm_obslog, ss_obslog, &soy);
	soy   += ( 3 - (soy+3)%4 );
	soy2dhms( soy, &doy_obslog, &hh_obslog, &mm_obslog, &ss_obslog);

	fprintf(fx_log_ptr, "%03d%02d%02d%02d/SOURCE/ %8s, %8s, %02d, %02d, %08.5lf, %1s%02d, %02d, %07.4lf, %6.1lf, %6.2lf, %6.2lf\n",
		doy_obslog, hh_obslog, mm_obslog, ss_obslog,
		src.comname,	src.iauname,	src.rh,		src.rm,
		src.rs,			sign,			src.dd,		src.dm,
		src.ds,			src.epoch,		src.pa,		src.pd);

/*
---------------------------------------------------- ENDING
*/
	return(1);
}
