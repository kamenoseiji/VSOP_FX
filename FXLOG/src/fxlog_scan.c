/*********************************************************
**	FXLOG_SCAN.C :	Module for Reading Obseravation Log **
**					Files for FX Correlator				**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1995/11/26								**
*********************************************************/

#include "fxlog.inc"

long fxlog_scan()
{

	drg_eof_flag	= 0;
	/*-------- INITIALIZE CATEGORY FLAGS --------*/
	for(i=0; i<MAX_CATEGORY_NUM; i++){
		category_flag[i] = 0;
	}

	eof_flag	= 0;
	/*-------- SCAN for CATEGORIES --------*/
	while( eof_flag != 1 ){
		fxlog_selector();		/* Read 1-line and select category */
		switch(log_category){

		case EXPER:		category_flag[EXPER] =		IN_LOG;	break;
		case STATION:	category_flag[STATION] =	IN_LOG;	break;
		case TSID:		category_flag[TSID] = 		IN_LOG;	break;
		case SOURCE:	category_flag[SOURCE] = 	IN_LOG;	break;
		case LO:		category_flag[LO] = 		IN_LOG;	break;
		case BBC:		category_flag[BBC] = 		IN_LOG;	break;
		case FORM:		category_flag[FORM] = 		IN_LOG;	break;
		case LABEL:		category_flag[LABEL] = 		IN_LOG;	break;
		case WX:		category_flag[WX] = 		IN_LOG;	break;
		case ST:		category_flag[ST] = 		IN_LOG;	break;
		case ET:		category_flag[ET] = 		IN_LOG;	break;
		case CLOCK:		category_flag[CLOCK] = 		IN_LOG;	break;
		case CLKRATE:	category_flag[CLKRATE] = 	IN_LOG;	break;
		case ONSOURCE:	category_flag[ONSOURCE] = 	IN_LOG;	break;
		case FLAG:		category_flag[FLAG] = 		IN_LOG;	break;
		case POLAR:		category_flag[POLAR] = 		IN_LOG;	break;
		case TEC:		category_flag[TEC] = 		IN_LOG;	break;
		case CABLE:		category_flag[CABLE] = 		IN_LOG;	break;
		case TSYS:		category_flag[TSYS] = 		IN_LOG;	break;
		case PCAL:		category_flag[PCAL] = 		IN_LOG;	break;
		}
	}

	/*-------- SCAN for DRUDGE FILE --------*/
	if( drg_use_flag == 1 ){
		fxlog_drg_read();
	}

	/*-------- FLAG BROWSER --------*/
	#ifdef DEBUG
		for(i=1; i<MAX_CATEGORY_NUM; i++){
			printf("%-10s: %d\n", category_array[i], category_flag[i]);
		}
	#endif
}
