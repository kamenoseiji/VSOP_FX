/*********************************************************
**	FXLOG_READ.C :	Module for Reading Obseravation Log **
**					Files for FX Correlator				**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1995/11/26								**
*********************************************************/

#include "fxlog.inc"

long fxlog_read()
{
	eof_flag	= 0;

	/*-------- SCAN for CATEGORIES --------*/
	while( eof_flag != 1 ){
		fxlog_selector();		/* Read 1-line and select category */
		switch(log_category){
			case EXPER:
				fxlog_read_exper();
				fxlog_write_exper(log_exper);
				break;

			case STATION:
				fxlog_read_station();
				fxlog_write_station(log_station);
				break;

			case TSID:		category_flag[TSID] = 1;	break;

			case SOURCE:
				fxlog_read_source();
				fxlog_write_source(log_src);
				break;

			case LO:
				fxlog_read_lo();
				fxlog_write_lo(log_station);
				break;

			case BBC:
				fxlog_read_bbc();
				fxlog_write_bbc(log_bbc[bbc_cntr]);
				break;

			case FORM:
				fxlog_read_form();
				fxlog_write_form(log_form[form_cntr]);
				break;

			case LABEL:
				fxlog_read_label();
				fxlog_write_label(log_label);
				break;

			case WX:
				fxlog_read_wx();
				fxlog_write_wx( log_wx );
				break;

			case ST:
				fxlog_read_st();
				if( category_flag[SOURCE] == IN_DRG ){
					/*-------- DERIVE SOURCE FROM ST --------*/
					fxlog_detect_source();
				}
				fxlog_write_st( log_st );
				break;

			case ET:
				fxlog_read_et();
				fxlog_write_et( log_et );
				break;

			case CLOCK:		category_flag[CLOCK] = 1;	break;
			case CLKRATE:	category_flag[CLKRATE] = 1;	break;
			case ONSOURCE:	category_flag[ONSOURCE] = 1;break;
			case FLAG:		category_flag[FLAG] = 1;	break;
			case POLAR:		category_flag[POLAR] = 1;	break;
			case TEC:		category_flag[TEC] = 1;		break;
			case CABLE:		category_flag[CABLE] = 1;	break;
			case TSYS:		category_flag[TSYS] = 1;	break;
			case PCAL:		category_flag[PCAL] = 1;	break;
		}
		log_category	=	COMMENT;	/* Initialize */
	}
}
