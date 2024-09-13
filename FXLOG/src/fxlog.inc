/*****************************************************
**	FXLOG.INC : Include File for FXLOG CONVERTER	**
**													**
**	AUTHOR	: KAMENO Seiji							**
**	CREATED	: 1995/11/26							**
******************************************************/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>

#define	MAX_CATEGORY_NUM	32	/* NUMBER OF CATEGORY */
#define	MAX_IF				4	/* NUMBER OF IF */
#define	MAX_BBC				32	/* NUMBER OF BBC */
#define	MAX_FORM			32	/* NUMBER OF SAMPLER (FORMATTER) */
#define	COMMENT		0			/* LOG CATEGORY */
#define	EXPER		1			/* LOG CATEGORY */
#define	STATION		2			/* LOG CATEGORY */
#define	TSID		3			/* LOG CATEGORY */
#define	SOURCE		4			/* LOG CATEGORY */
#define	LO			5			/* LOG CATEGORY */
#define	BBC			6			/* LOG CATEGORY */
#define	FORM		7			/* LOG CATEGORY */
#define	LABEL		8			/* LOG CATEGORY */
#define	WX			9			/* LOG CATEGORY */
#define	ST			10			/* LOG CATEGORY */
#define	ET			11			/* LOG CATEGORY */
#define	CLOCK		12			/* LOG CATEGORY */
#define	CLKRATE		13			/* LOG CATEGORY */
#define	ONSOURCE	14			/* LOG CATEGORY */
#define	FLAG		15			/* LOG CATEGORY */
#define	POLAR		16			/* LOG CATEGORY */
#define	TEC			17			/* LOG CATEGORY */
#define	CABLE		18			/* LOG CATEGORY */
#define	TSYS		19			/* LOG CATEGORY */
#define	PCAL		20			/* LOG CATEGORY */

#define	PARAM	101				/* PARAM Chpater */
#define	SKED	102				/* SKED Chpater */
#define	HEAD	103				/* HEAD Chpater */
#define	CODE	104				/* CODE Chpater */

#define	UNFOUND	0				/* FOR CATEGORY FLAG */
#define	IN_LOG	1				/* FOR CATEGORY FLAG */
#define	IN_DRG	2				/* FOR CATEGORY FLAG */
#define	IN_TTY	3				/* FOR CATEGORY FLAG */
#define	WRITTEN	999				/* FOR CATEGORY FLAG */

static char category_array[MAX_CATEGORY_NUM][10]	= {
		"COMMENT",	"EXPER",	"STATION",	"TSID",
		"SOURCE",	"LO",		"BBC",		"FORM",
		"LABEL",	"WX",		"ST",		"ET",
		"CLOCK",	"CLKRATE",	"ONSOURCE",	"FLAG",
		"POLAR",	"TEC",		"CABLE",	"TSYS",
		"PCAL"};

long	k4;						/* K4 Flag 1->K-4 0-> not K-4 */
long	i,j,k;					/* GENERAL COUNER */
long	clk;					/* CLOCK [8, 16 or 32 MHz] */
long	offset;					/* Start Offset [sec] */
char	prog_name[20];			/* Running Program Name */
char	obs_log_fname[20];		/* Input Observation Log File Name */
char	obs_drg_fname[20];			/* Drudge File Name */
char	fx_log_fname[20];		/* Output Correlator Log File Name */
FILE	*obs_log_ptr;			/* FILE Pointer of Obs Log */
FILE	*obs_drg_ptr;			/* FILE Pointer of Drudge Log */
FILE	*fx_log_ptr;			/* FILE Pointer of FX Log */
long	year_obslog;			/* Year in Obs. Log File */
long	doy_obslog;				/* Day of Year in Obs. Log File */
long	hh_obslog;				/* Hour in Obs. Log File */
long	mm_obslog;				/* Minute in Obs. Log File */
long	ss_obslog;				/* Second in Obs. Log File */
char	station_obslog[10];		/* STATION Name */
long	stn_id;					/* Station ID Number */
long	eof_flag;				/* FLAG for End of File */
long	drg_use_flag;			/* FLAG for Using Drudge File */
long	log_category;			/* Category in Log */
char	*log_pointer;			/* Charactor Pointer in Log */
long	category_flag[MAX_CATEGORY_NUM];	/* CATEGORY EXIST -> 1 */
char	line_buf[256];			/* 1-Line Buffer */
char	dum[256];				/* Dummy Buffer */


char	drg_mode[10];		/* Drudge File Chapter Name */
long	drg_eof_flag;		/* Detect End of File -> 1 */
long	source_counter;		/* Source Counter */
long	drg_chapter;		/* Chapter Identifier */
long	drg_subchap;		/* Sub-Chapter Identifier */
char	exper_code[9];		/* Experiment Code */
long	num_source;			/* Number of Source */
long	num_station;		/* Number of Station */
double	fdum;				/* Dummy Real Variable */
long	ftable_cntr;		/* Frequency Table Counter */	
long	if_cntr;			/* IF Counter */	
long	bbc_cntr;			/* Base Band Converter Counter */	
long	form_cntr;			/* Sampler (Formatter) Counter */	
long	sked_suffix;		/* SKED Number to be used */
long	src_suffix;			/* SOURCE Number to be used */

struct	exper_line{
	char	code[2];			/* Experiment code */
	long	year;				/* Observation Year */
	long	doy;				/* Observation Day of Year */
};
struct	exper_line	log_exper;
struct	exper_line	tty_exper;
struct	exper_line	drg_exper;

struct	label_line{
	char	label[9];			/* LABEL*/
};
struct	label_line	log_label;

struct	source_line{
	char	iauname[9];			/* IAU Name of the Source */
	char	comname[9];			/* Common Name of the Source */
	long	rh;					/* Hour of Right Accesnssion */
	long	rm;					/* Minute of Right Accesnssion */
	double	rs;					/* Second of Right Accesnssion */
	long	sign;				/* Sign of Declination */
	long	dd;					/* Degree of Declination */
	long	dm;					/* Minute of Declination */
	double	ds;					/* Second of Declination */
	double	epoch;				/* Epoch of the Coordinate */
	double	pa;					/* Proper Motion for RA [mas/yr] */
	double	pd;					/* Proper Motion for DEC [mas/yr] */
};
struct	source_line	log_src;		/* Source Information */
struct	source_line	drg_src[500];	/* Source Information */

struct	antenna_line{
	char	code1[2];			/* Station 1-letter CODE */	
	char	code2[3];			/* Station 2-letter CODE */	
	char	name[9];			/* Station Name */	
	char	type[5];			/* Mount Type [AZEL, HADC, XYNS] */	
	double	offset;				/* Axis Offset [m] */
	double	x;					/* Station Position [m] */
	double	y;					/* Station Position [m] */
	double	z;					/* Station Position [m] */
	long	if_num;				/* Number of IF */
	double	flocal[MAX_IF];		/* Local Frequency [MHz] */
};
struct	antenna_line	log_station;	/* Station Information */
struct	antenna_line	tty_station;	/* Station Information */
struct	antenna_line	drg_station[20];/* Station Information */

struct	freq_line{
	char	name[9];			/* Freq. Table Name */
	char	code[3];			/* 2-letter Freq. Code */
	long	num_sg;				/* Number of Sub Group */
	long	total_bbc;			/* Total Number of BBC */
	char	sg_name[MAX_IF][2];	/* SG name*/
	long	num_bbc[MAX_IF];	/* Base Band Converter for each SG*/
	long	if_num[MAX_BBC];	/* IF Number of the BBC */	
	double	sky_freq[MAX_BBC];	/* SKY Freq [MHz]*/	
	double	bbc_bw[MAX_BBC];	/* BBC Bandwidth [MHz]*/	
	double	pcal_freq[MAX_BBC];	/* Pcal Freq at BBC [kHz]*/	
};
struct	freq_line	log_freq;		/* Frequency Code Information */
struct	freq_line	drg_freq[20];	/* Frequency Code Information */

struct	bbc_line{
	long	lo_num;					/* LO Number */
	long	bbc_num;				/* BBC Number */
	double	freq;					/* Frequency [MHz]*/
};
struct	bbc_line	log_bbc[MAX_BBC];
struct	bbc_line	drg_bbc;

struct	form_line{
	long	bbc_num;				/* BBC Number */
	long	form_num;				/* FORMATTER Number */
	char	side[4];				/* SIDEBAND [USB/LSB/DSB] */
	double	rate;					/* Frequency [ksps]*/
	long	bit;					/* Quantization Bit */
};
struct	form_line	log_form[MAX_FORM];
struct	form_line	drg_form;

struct	param_line{
	long	form_num;				/* Total Number of FORM		*/
	long	data_rate;				/* Total Data Rate [Mbps]	*/
	long	quant_bit;				/* Quantization Bits		*/
	long	fsample;				/* Sampling Frequency [kHz]	*/
	long	fedge[MAX_BBC];			/* Lowest Edge Freq. [MHz]	*/
	long	sideb[MAX_BBC];			/* Sideband (1:USB, -1:LSB)	*/
};
struct	param_line	drg_param;

struct	wx_line{
	double	temp;					/* Temperature [deg]*/
	double	press;					/* Pressure [hPa]*/
	double	humd;					/* Humidity [%]*/
};
struct	wx_line		log_wx;

struct	tss_line{
	long	tss;					/* TSS ID */
};
struct	tss_line	log_st;
struct	tss_line	log_et;

struct	sked_line{
	char	comname[9];			/* Common Name of the Souce in SKED */
	int		cal;				/* Calibration Duration */
	char	freq[3];			/* Frequency Table Code */
	long	nch;				/* Number of Channel */
	double	flocal;				/* Local Freq. [MHz] */
	double	fsky[16];			/* Sky Freq [MHz] */
	char	preob[7];			/* PREOB Table Name */
	int		year;				/* Year of the Start Time */
	int		doy;				/* Day of the Start Time */
	int		hour;				/* Hour of the Start Time */
	int		minute;				/* Minute of the Start Time */
	int		second;				/* Second of the Start Time */
	int		duration;			/* Recording Duration in Second */
	char	midob[7];			/* MIDOB Table Name */
	int		idle;				/* Idle Time in Second */
	char	postob[7];			/* POSTOB Table Name */
	int		az_mode;			/* -1 -> CCW, 0 -> Normal, 1 -> CW */
	char	station[41];		/* Stations Item in SKED */
};
struct	sked_line	sked[500];	/* Scan Information */



/*-------------------- Maybe unnecessary --------*/



char	obsmode[7];			/* PREOB, MIDOB, or POSTOB */
int		num_preob;			/* Number of PREOB Parameter */
int		num_midob;			/* Number of MIDOB Parameter */
int		num_postob;			/* Number of POSTOB Parameter */
int		num_sked;			/* Number of Sked Line */
int		num_code;			/* Number of Freq. Code */
int		code_counter;		/* BAND Counter */
int		channel_counter;	/* CHANNEL Counter */

struct	midob_line{
	char	name[7];			/* Name of MIDOB */
	char	device[5];			/* LINE or CONT */
	char	scan[9];			/* POINTING or TRACKING */
};

struct	code_line{				/* Frequency Table in VCOSMOS */
	char	name[9];			/* Frequence Band Table Name */
	char	mode[3];			/* ABBREVIATED (2-CHARACTER) TABLE FAME */
	long	nch;				/* Number of Channel */
	double	fsky[16];			/* Sky Frequency [MHz]*/
	double	flocal;				/* Local Frequency [MHz]*/
};

struct	midob_line	midob[500];	/* MIDOB Information */
struct	code_line	code[50];	/* CODE Information */
