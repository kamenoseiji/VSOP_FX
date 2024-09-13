/*********************************************************
**	FXLOG.C :											**	
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1995/11/26								**
*********************************************************/

#include "fxlog.inc"
/*
---------------------------------------- LOAD PARAMS FROM INPUT
*/
main(argc, argv)
	long	argc;
	char	**argv;
{
/*
---------------------------------------- READ PARAMETERS
*/
	fxlog_param(argc, argv);
/*
---------------------------------------- SCAN DRUDGE FILE
*/
	fxlog_write_open( fx_log_fname, &fx_log_ptr );
	if( drg_use_flag == 1 ){
		fxlog_read_open( obs_drg_fname, &obs_drg_ptr );
	}
/*
---------------------------------------- SCAN LOG FILE
*/
	fxlog_read_open( obs_log_fname, &obs_log_ptr );
	fxlog_scan();
	fxlog_input();
	fclose(obs_log_ptr);
/*
---------------------------------------- READ FILES
*/
	fxlog_read_open( obs_log_fname, &obs_log_ptr );
	fxlog_read();
/*
---------------------------------------- CLOSE FILES
*/
	fclose(obs_drg_ptr);
	fclose(obs_log_ptr);
	fclose(fx_log_ptr);
	#ifndef DEBUG
	unlink("hidoi.log");
	#endif
}
