/*********************************************************
**	VLBALOG.C	: Generate VLBA Log File				**
**														**
**	AUTHOR	: Kameno Seiji								**
**	CREATED	: 1999 6/15									**
*********************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	"vlbalog.inc"

#define	COPYLOG	1
#define	VLBALOG	2
#define	CLG_FMT	"%04d%03d%02d%02d%02d%02d"

main(argc, argv)
	int		argc;				/* Number of Arguments		*/
	char	**argv;				/* Poineter of Arguments	*/
{

	FILE	*copylog_ptr;		/* File Pointer of the Copy Log	*/
	FILE	*vlbalog_ptr;		/* File Pointer of the VLBA Log	*/
	char	line_buf[256];		/* 1-line buffer				*/
	char	line_buf2[256];		/* 1-line buffer				*/
	int		year;				/* Time of Log Entry			*/
	int		doy;				/* Time of Log Entry			*/
	int		hour;				/* Time of Log Entry			*/
	int		min;				/* Time of Log Entry			*/
	int		sec;				/* Time of Log Entry			*/
	int		fsec;				/* Time of Log Entry			*/
	char	obscode[16];		/* OBSCODE						*/
	char	bbc_str[16];		/* BBC FREQUENCY [MHz]			*/
	char	vsn[16];			/* VLBA Tape Label				*/
	char	source[16];			/* Source Name					*/
	int		bbc_freq;			/* BBC Frequency [MHz]			*/
	int		step_back;			/* Number of Steps to Back		*/
	int		ptr_index;			/* Pointer Index				*/

/*
----------------------------------- CHECK COMMAND ARGUMENTS
*/
	if(argc < 3){
		printf("USAGE: vlbalog [INPUT FILE] [OUTPUT FILE] !!\n");
		exit(0);
	}

/*
----------------------------------- OPEN LOG FILES
*/
	/*-------- OPEN COPY LOG FILE --------*/
	copylog_ptr = fopen( argv[COPYLOG], "r");
	if(copylog_ptr == NULL ){
		printf("Can't Open Copy Log File [%s].\n", argv[COPYLOG]);
		exit(0);
	}

	/*-------- CREATE VLBA LOG FILE --------*/
	vlbalog_ptr = fopen( argv[VLBALOG], "w");
	if(vlbalog_ptr == NULL ){
		printf("Can't Create VLBA Log File [%s].\n", argv[VLBALOG]);
		exit(0);
	}
/*
----------------------------------- INITIAL TIME
*/
	if( fgets( line_buf, 256, copylog_ptr) != NULL ){
		sscanf(line_buf, CLG_FMT, &year, &doy, &hour, &min, &sec, &fsec);
	}
	rewind(copylog_ptr);
/*
----------------------------------- WRITE /SOURCE/
*/
	clg_input(source, "SOURCE");
	fprintf(vlbalog_ptr, SOURCE_FMT, (year%100), doy, hour, min, sec, fsec, source);
/*
----------------------------------- WRITE /EQUIP/
*/
	fprintf(vlbalog_ptr, EQUIP_FMT, (year%100), doy, hour, min, sec, fsec);
/*
----------------------------------- WRITE /OBSCODE/
*/
	clg_input(obscode, "OBSCODE");
	fprintf(vlbalog_ptr, OBSCODE_FMT, (year%100), doy, hour, min, sec, fsec, obscode);
/*
----------------------------------- WRITE /BBC/
*/
	clg_input(bbc_str, "BBC1 [MHz]"); sscanf(bbc_str, "%d", &bbc_freq);
	fprintf(vlbalog_ptr, BBC_FMT,(year%100), doy, hour, min, sec, fsec, 1, bbc_freq);
	clg_input(bbc_str, "BBC2 [MHz]"); sscanf(bbc_str, "%d", &bbc_freq);
	fprintf(vlbalog_ptr, BBC_FMT,(year%100), doy, hour, min, sec, fsec, 2, bbc_freq);
/*
----------------------------------- WRITE /FORM/
*/
	fprintf(vlbalog_ptr, FORM_FMT, (year%100), doy, hour, min, sec, fsec);
/*
----------------------------------- WRITE /BIT_DENSITY/
*/
	fprintf(vlbalog_ptr, BIT_DENSITY_FMT, (year%100), doy, hour, min, sec, fsec);
/*
----------------------------------- WRITE /VSN/
*/
	if( scan_log( copylog_ptr, "VSN", line_buf) < 0 ){
		clg_input( vsn, "VLBA TAPE LABEL");
	} else {
		sscanf(line_buf, VSNCLG_FMT,&year, &doy, &hour, &min, &sec, &fsec, vsn);
	}
	rewind(copylog_ptr);


	fprintf(vlbalog_ptr, LABEL_FMT, (year%100), doy, hour, min, sec, fsec, vsn);
	fprintf(vlbalog_ptr, VSN_FMT, (year%100), doy, hour, min, sec, fsec, vsn);
/*
----------------------------------- OPEN LOG FILES
*/
	while( scan_log( copylog_ptr, "PASS", line_buf) > 0 ){

		/*-------- WRITE /TRACKFORM/ --------*/
		sscanf(line_buf, CLG_FMT, &year, &doy, &hour, &min, &sec, &fsec);
		fprintf(vlbalog_ptr, TRACK_FMT, (year%100), doy, hour, min, sec, fsec);

		/*-------- WRITE /FORM/ --------*/
		sscanf(line_buf, CLG_FMT, &year, &doy, &hour, &min, &sec, &fsec);
		fprintf(vlbalog_ptr, FORM_FMT, (year%100), doy, hour, min, sec, fsec);

		/*-------- WRITE /PASS/ --------*/
		sscanf(line_buf, CLG_FMT, &year, &doy, &hour, &min, &sec, &fsec);
		fprintf(vlbalog_ptr, PASS_FMT, (year%100), doy, hour, min, sec, fsec, &line_buf[15]);

		/*-------- WRITE /TAPE/ --------*/
		scan_log( copylog_ptr, "TAPE", line_buf);
		sscanf(line_buf, CLG_FMT, &year, &doy, &hour, &min, &sec, &fsec);
		fprintf(vlbalog_ptr, TAPE_FMT, (year%100), doy, hour, min, sec, fsec, &line_buf[15]);

		/*-------- WRITE /ST/ --------*/
		scan_log( copylog_ptr, "ST", line_buf);
		sscanf(line_buf, CLG_FMT, &year, &doy, &hour, &min, &sec, &fsec);
		line_buf[18] = '=';
		fprintf(vlbalog_ptr, ST_FMT, (year%100), doy, hour, min, sec, fsec, &line_buf[15]);

		/*-------- WRITE /ET/ --------*/
		scan_log( copylog_ptr, "ET", line_buf);
		sscanf(line_buf, CLG_FMT, &year, &doy, &hour, &min, &sec, &fsec);
		fprintf(vlbalog_ptr, ET_FMT, (year%100), doy, hour, min, sec, fsec, &line_buf[15]);

		/*-------- WRITE /TAPE/ --------*/
		scan_log( copylog_ptr, "TAPE", line_buf);
		sscanf(line_buf, CLG_FMT, &year, &doy, &hour, &min, &sec, &fsec);
		fprintf(vlbalog_ptr, TAPE_FMT, (year%100), doy, hour, min, sec, fsec, &line_buf[15]);

	}


	close(copylog_ptr);
	close(vlbalog_ptr);
}
