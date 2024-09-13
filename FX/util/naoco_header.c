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

#define HEADLINE 193
#define	SIZE_COR 4168

naoco_header( corr_file,	ss_index,		delay_ptr )

	FILE	**corr_file;			/* Pointer Data File */
	long	ss_index;				/* Sub-Stream Index */
	struct	delay_data	delay_ptr;
{
	long	nch;				/* CHANNEL NUMBER */
	long	i, j, k;			/* GENERAL COUNTER */
	long	year;				/* YEAR  (GENERAL VARIABLE) */
	long	doy;				/* Day of YEAR (GENERAL VARIABLE) */
	long	hh;					/* Hour (GENERAL VARIABLE) */
	long	mm;					/* Minute (GENERAL VARIABLE) */
	long	ss;					/* Second (GENERAL VARIABLE) */
	long	mjd;				/* Modified Julian Date (GENERAL VARIABLE) */
	double	clock_epoch;		/* Clock Epoch in MJD */
	double	first_pp_time;		/* Epoch of the First PP in MJD */

/*
-------------------------------- READ CORR DATA FILE
*/
	for(i=0; i<HEADLINE; i++){
		fread(line_buf, 1, sizeof(line_buf), *corr_file);

		/*-------- INFORMATION FOR SOURCE NAME --------*/
		if(strstr(line_buf, "Common_name_of_radio_source") != NULL){
			sscanf(line_buf, "%*s%s", delay_ptr.source_name); /* Source Name */
		}

		/*-------- INFORMATION FOR SOURCE POSITION --------*/
		if(strstr(line_buf, "Right_ascension_of_radio_source") != NULL){
			sscanf(line_buf, "%*s%d %d %lf",
				&delay_ptr.rh, &delay_ptr.rm, &delay_ptr.rs );
		}
		if(strstr(line_buf, "Declination_of_radio_source") != NULL){
			sscanf(line_buf, "%*s%1s%d %d %lf",
				delay_ptr.sign, &delay_ptr.dd, &delay_ptr.dm, &delay_ptr.ds );
		}
		if(strstr(line_buf, "Epoch_of_source_coordinates") != NULL){
			sscanf(line_buf, "%*s%d", &delay_ptr.epoch);
		}

		/*-------- INFORMATION FOR STATION NAME --------*/
		if(strstr(line_buf, "X_site_name") != NULL){
			sscanf(line_buf, "%*s%s", delay_ptr.site_name_x); /* Station NAME */
		}
		if(strstr(line_buf, "Y_site_name") != NULL){
			sscanf(line_buf, "%*s%s", delay_ptr.site_name_y); /* Station NAME */
		}

		/*-------- INFORMATION FOR STATION CODE --------*/
		if(strstr(line_buf, "X_site_code") != NULL){
			sscanf(line_buf, "%*s%s", delay_ptr.site_code_x); /* Station Code */
		}
		if(strstr(line_buf, "Y_site_code") != NULL){
			sscanf(line_buf, "%*s%s", delay_ptr.site_code_y); /* Station Code */
		}

		/*-------- INFORMATION FOR STATION POSITION --------*/
		if(strstr(line_buf, "X_site_position_x") != NULL){
			sscanf(line_buf, "%*s%lf", &delay_ptr.x_pos_x); 	/* Station Position */
		}
		if(strstr(line_buf, "X_site_position_y") != NULL){
			sscanf(line_buf, "%*s%lf", &delay_ptr.x_pos_y); 	/* Station Position */
		}
		if(strstr(line_buf, "X_site_position_z") != NULL){
			sscanf(line_buf, "%*s%lf", &delay_ptr.x_pos_z); 	/* Station Position */
		}
		if(strstr(line_buf, "Y_site_position_x") != NULL){
			sscanf(line_buf, "%*s%lf", &delay_ptr.y_pos_x); 	/* Station Position */
		}
		if(strstr(line_buf, "Y_site_position_y") != NULL){
			sscanf(line_buf, "%*s%lf", &delay_ptr.y_pos_y); 	/* Station Position */
		}
		if(strstr(line_buf, "Y_site_position_z") != NULL){
			sscanf(line_buf, "%*s%lf", &delay_ptr.y_pos_z); 	/* Station Position */
		}

		/*-------- INFORMATION FOR SAMPLING RATE --------*/
		if(strstr(line_buf, "Sampling_rate") != NULL){
			sscanf(line_buf, "%*s%lf", &delay_ptr.fsample);		/* [MHz] */
		}

		/*-------- INFORMATION FOR CPP TIME DURATION --------*/
		if(strstr(line_buf, "Time_duration_of_compressed_AP") != NULL){
			sscanf(line_buf, "%*s%lf", &delay_ptr.cpp_len);
			delay_ptr.cpp_len *= 1.0e-6;					/* CPP in [sec] */
		}

		/*-------- INFORMATION FOR RF FREQUENCY --------*/
		if(strstr(line_buf, "RF_at_lower_bandedge") != NULL){
			sscanf(line_buf, "%*27s%02d%", &nch);
			if(nch == ss_index+1){
				sscanf(line_buf, "%*27s%02d%*2s%lf",
					&nch, &delay_ptr.rf[ss_index]);
				delay_ptr.rf[ss_index] *= 0.01;		/* RF in [MHz] */
			}
		}

		/*-------- INFORMATION FOR OBSERVATION DATE and TIME --------*/
		if(strstr(line_buf, "First_processing_time") != NULL){
			sscanf(line_buf, "%*s%5d", &delay_ptr.doy);
		}

		/*-------- INFORMATION FOR CLOCK PARAMETER --------*/
		if(strstr(line_buf, "Input_clock_offset_X") != NULL){
			sscanf(line_buf, "%*s%lf", &delay_ptr.input_offset_x);
			delay_ptr.input_offset_x = delay_ptr.input_offset_x / 1000.0;
		}
		if(strstr(line_buf, "Input_clock_offset_Y") != NULL){
			sscanf(line_buf, "%*s%lf", &delay_ptr.input_offset_y);
			delay_ptr.input_offset_y = delay_ptr.input_offset_y / 1000.0;
		}
		if(strstr(line_buf, "Input_clock_rate_X") != NULL){
			sscanf(line_buf, "%*s%lf", &delay_ptr.input_rate_x);
			delay_ptr.input_rate_x = delay_ptr.input_rate_x / 1000.0;
		}
		if(strstr(line_buf, "Input_clock_rate_Y") != NULL){
			sscanf(line_buf, "%*s%lf", &delay_ptr.input_rate_y);
			delay_ptr.input_rate_y = delay_ptr.input_rate_y / 1000.0;
		}
		if(strstr(line_buf, "Input_UT1-UTC") != NULL){
			sscanf(line_buf, "%*s%lf", &delay_ptr.input_ut1utc);
			delay_ptr.input_ut1utc *= 0.001;
		}


		/*-------- INFORMATION FOR CLOCK EPOCH --------*/
		if(strstr(line_buf, "Input_clock_epoch_X") != NULL){
			sscanf(line_buf, "%*s%02d%03d%02d%02d%02d",
				&year, &doy, &hh, &mm, &ss);
			if(year > 50){	year = year + 1900;		/* 19** year */
			} else {		year = year + 2000;}	/* 20** year */
			doy2mjd( year, doy, &mjd );				/* MJD */
			ss = 3600*hh + 60*mm + ss;				/* Second of Day */

			/*-------- CLOCK EPOCH in MJD --------*/
			delay_ptr.clock_epoch = (double)mjd + (double)ss/86400.0;
		}

		/*-------- INFORMATION FOR THE FIRST PP --------*/
		if(strstr(line_buf, "First_processing_time") != NULL){
			sscanf(line_buf, "%*s%02d%03d%02d%02d%02d",
				&year, &doy, &hh, &mm, &ss);
			if(year > 50){	year = year + 1900;		/* 19** year */
			} else {		year = year + 2000;}	/* 20** year */
			doy2mjd( year, doy, &mjd );				/* MJD */
			ss = 3600*hh + 60*mm + ss;				/* Second of Day */
			first_pp_time = (double)mjd + (double)ss/86400.0;
			delay_ptr.year	= year;
			delay_ptr.doy	= doy;
		}
	}

	for(i=0; i< (SIZE_COR - (80*HEADLINE)%SIZE_COR)/80; i++){
		fread(line_buf, 1, sizeof(line_buf), *corr_file);
	}
	fread(line_buf, 1, (SIZE_COR - (80*HEADLINE)%SIZE_COR)%80, *corr_file);

	return(0);
}
