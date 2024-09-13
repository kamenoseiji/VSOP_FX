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
#define NLAG  512
#define NCPP  256
#define NCPP2 128
#define NSPEC 256
#define NSPEC2 128

blvis( dir_ptr, filenum_io, bbcnum_io, start, duration,	lag_number,
			vis_r,			vis_i,			ncpp_ptr,
			fsample_ptr,	cpp_len_ptr,	rf_ptr,
			start_ptr,		delay_ptr)

	char	*dir_ptr;			/* Pointer of Directory */
	long	filenum_io;			/* File Number */
	long	bbcnum_io;			/* BaseBand Convertor Number */
	long	start;				/* START Time from the Begin of the Day [sec] */
	long	duration;			/* DURATION [sec] */
	long	lag_number;			/* LAG Number to be Used */
	float	*vis_r;				/* Visibility Function */
	float	*vis_i;				/* Visibility Function */
	long	*ncpp_ptr;			/* Number of CPP */
	double	*fsample_ptr;		/* Sampling Frequency [MHz] */
	double	*cpp_len_ptr;		/* CPP Duration [sec] */
	double	*rf_ptr;			/* Observing Frequency [MHz] */
	long	*start_ptr;			/* START Time */
	struct	delay_data	delay_ptr;
{
	long	eof_flag;
	long	nfft;				/* FFT NUMBER */
	long	ndim;				/* FFT DIMENSION */
	long	ndir;				/* FFT DIRECTION */
	long	icon;				/* CONDITION CODE */
	long	nch;				/* CHANNEL NUMBER */
	long	i, j, k;			/* GENERAL COUNTER */
	double	cor_r[NLAG], cor_i[NLAG];				/* CORRELATION FUNCTION */
	float	x, y, r;								/* GENERAL VALUE */
	long	year;				/* YEAR  (GENERAL VARIABLE) */
	long	doy;				/* Day of YEAR (GENERAL VARIABLE) */
	long	hh;					/* Hour (GENERAL VARIABLE) */
	long	mm;					/* Minute (GENERAL VARIABLE) */
	long	ss;					/* Second (GENERAL VARIABLE) */
	long	mjd;				/* Modified Julian Date (GENERAL VARIABLE) */
	double	clock_epoch;		/* Clock Epoch in MJD */
	double	first_pp_time;		/* Epoch of the First PP in MJD */
	long	st_time;
	long	et_time;
	long	integ_dur;
	long	ncpp;
	double	corr_time_tag;
	double	afact;

	struct cordat	cor;		/* DATA FORMAT IN NAOCO FILE */

/*
-------------------------------- INPUT PARAMETERS
*/
	strcpy(pathname, dir_ptr);
	filenum = filenum_io;
	bbcnum	= bbcnum_io;
	st_time		= start;
	integ_dur = duration;
	et_time = st_time + integ_dur;
/*
-------------------------------- OPEN HEADER FILE
*/
	sprintf(header_fname, "%s/H%06d.000", pathname, filenum);
	if( (header_file = fopen(header_fname, "r")) == NULL){
		printf("Can't Open Header File [%s].\n", header_fname);
		return(-1);
	}
/*
-------------------------------- OPEN CORR DATA FILE
*/
	sprintf(corr_fname, "%s/R%06d.%03d",pathname, filenum, bbcnum);
	if( (corr_file = fopen(corr_fname, "rb")) == NULL){
		printf("Can't Open CORR DATA File [%s].\n", corr_fname);
		return(-1);
	}
/*
-------------------------------- READ CORR DATA FILE
*/
	for(i=0; i<HEADLINE; i++){
		fread(line_buf, 1, sizeof(line_buf), corr_file);

		/*-------- INFORMATION FOR SOURCE NAME --------*/
		if(strstr(line_buf, "Common_name_of_radio_source") != NULL){
			sscanf(line_buf, "%*s%s", delay_ptr.source_name); /* Source Name */
		}

		/*-------- INFORMATION FOR SOURCE POSITION --------*/
		if(strstr(line_buf, "Right_ascension_of_radio_source") != NULL){
			sscanf(line_buf, "%*s%d %d %lf", &delay_ptr.rh, &delay_ptr.rm, &delay_ptr.rs );
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
			sscanf(line_buf, "%*s%lf", &fsample);	/* FSAMPLE in [MHz] */
		}

		/*-------- INFORMATION FOR CPP TIME DURATION --------*/
		if(strstr(line_buf, "Time_duration_of_compressed_AP") != NULL){
			sscanf(line_buf, "%*s%lf", &cpp_len);
			cpp_len = cpp_len * 1.0e-6;		/* CPP in [sec] */
		}

		/*-------- INFORMATION FOR RF FREQUENCY --------*/
		if(strstr(line_buf, "RF_at_lower_bandedge") != NULL){
			sscanf(line_buf, "%*27s%02d%", &nch);
			if(nch == bbcnum){
				sscanf(line_buf, "%*27s%02d%*2s%lf", &nch, &rf);
				rf = rf * 0.01;		/* RF in [MHz] */
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
		fread(line_buf, 1, sizeof(line_buf), corr_file);
	}
	fread(line_buf, 1, (SIZE_COR - (80*HEADLINE)%SIZE_COR)%80, corr_file);

	eof_flag = 0;
	ncpp = 0;
	afact = 1.0/16777216;
	while(1){
		if(fread(&cor, 1, SIZE_COR, corr_file) != SIZE_COR){
			printf("Failed to Read Data at CPP=%d.\n", ncpp);
			eof_flag = 1;
			break;
		}
		corr_time_tag = (double)cor.time_sec + 1.0e-6*(double)cor.time_micro;
		if(corr_time_tag > et_time){	break;	}
		if(corr_time_tag + cpp_len > st_time){
			/*-------- SAVE LAG-DOMAIN DATA --------*/
			for(j=0; j<lag_number/2; j++){
				cor_r[j]				= cor.data[2*(j+NSPEC)];
				cor_i[j]				= cor.data[2*(j+NSPEC)+1];
				cor_r[j+lag_number/2]	= cor.data[2*(j+NSPEC)-lag_number];
				cor_i[j+lag_number/2]	= cor.data[2*(j+NSPEC)-lag_number+1];
			}
			nfft=lag_number; ndir=1; ndim=1;
			dcft_(cor_r, cor_i, &nfft, &ndim, &ndir, &icon);

			/*-------- SWITCH BY FRINGE RATE SIGN --------*/
			if(cor.ratapr >= 0.0){
				for(j=0; j<lag_number/2; j++){
					*vis_r	=	(float)(afact*cor_r[j]);	vis_r++;
					*vis_i	=	(float)(afact*cor_i[j]);	vis_i++;
				}
			} else {
				for(j=lag_number-1; j>=lag_number/2; j--){
					*vis_r	=	(float)(afact*cor_r[j]);	vis_r++;
					*vis_i	=	(float)(afact*cor_i[j]);	vis_i++;
				}
			}	/*--- END OF IF ---*/

			ncpp++;
		}
	}
	fclose(header_file);
	fclose(corr_file);

	*start_ptr	= st_time;
	*fsample_ptr	= fsample;
	*cpp_len_ptr	= cpp_len;
	*rf_ptr			= rf;
	*ncpp_ptr		= ncpp;

	return(eof_flag);
}
