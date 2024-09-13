/*********************************************
**	DRUDGE.INC : Include File for I/O 		**
**				 for Drudge File			**
**											**
**	AUTHOR	: KAMENO Seiji					**
**	CREATED	: 1994/5/2						**
**********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define	MAX_CHAPTER	8			/* Number of Chpater */

#define	EXPER	1				/* EXPER Chpater */
#define	PARAM	2				/* PARAM Chpater */
#define	SOURCE	3				/* SOURCE Chpater */
#define	STATION	4				/* STATITION Chpater */
#define	SKED	5				/* SKED Chpater */
#define	HEAD	6				/* HEAD Chpater */
#define	CODE	7				/* CODE Chpater */

static char	chapter_array[MAX_CHAPTER][16]    = {
        "UNDEF",  	"EXPER",	"PARAM",	"SOURCES",
        "STATIONS",	"SKED",		"HEAD",		"CODES"};

	char	line_buf[256];		/* 1-line Buffer to Read File */

struct	mdhms{					/* Time Format of MDHMS	*/
	int		mon;				/* Month				*/
	int		day;				/* Day					*/
	int		hour;				/* Hour					*/
	int		min;				/* Minute				*/
	int		sec;				/* Second				*/
};

struct	source_line{
	struct	source_line	*next_src_ptr;
	char	iauname[9];			/* IAU Name of the Source */
	char	comname[9];			/* Common Name of the Source */
	long	rh;					/* Hour of Right Accesnssion */
	long	rm;					/* Minute of Right Accesnssion */
	float	rs;					/* Second of Right Accesnssion */
	long	sign;				/* Sign of Declination */
	long	dd;					/* Degree of Declination */
	long	dm;					/* Minute of Declination */
	float	ds;					/* Second of Declination */
	float	epoch;				/* Epoch of the Coordinate */
	float	vlsr;				/* Radial Velocity in LSR [km/s] */
};

struct	sked_line{
	struct	sked_line	*next_sked_ptr;
	char	comname[9];			/* Common Name of the Souce in SKED */
	int		cal;				/* Calibration Duration */
	char	freq[3];			/* Frequency Table Code */
	long	nch;				/* Number of Channel */
	float	flocal;				/* Local Freq. [MHz] */
	float	fsky[16];			/* Sky Freq [MHz] */
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

struct	station_line{
	struct	station_line	*next_stn_ptr;
	char	code1[2];			/* Station 1-letter CODE */	
	char	code2[3];			/* Station 2-letter CODE */	
	char	name[9];			/* Station Name */	
	char	type[5];			/* Mount Type [AZEL, HADC, XYNS] */	
	float	offset;				/* Axis Offset [m] */
	double	x;					/* Station Position [m] */
	double	y;					/* Station Position [m] */
	double	z;					/* Station Position [m] */
	long	if_num;				/* Number of IF */
	double	flocal;				/* Local Frequency [MHz] */
};

struct	code_line{
	struct code_line		*next_code_ptr;
	char	name[9];			/* Frequency Group Code Name	*/
	char	mode[3];			/* Abbr. 2-Letter Code Name		*/
	int		n_sg;				/* Number of Sub-Group (IF)		*/
	int		n_ch;				/* Number of Channel (A/D)		*/
	char	sg_id[16][2];		/* 1-Letter IF Code Name		*/
	double	lo_freq[16];		/* Local Frequency [STN, IF]	*/
	double	rf[16];				/* Bandedge RF Frequency [MHz]	*/
	double	bw[16];				/* Bandwidth [MHz]				*/
};
