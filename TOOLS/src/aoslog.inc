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

#define	ZERO	0				/* SCAN STATUS */
#define	R		1				/* SCAN STATUS */
#define	SKY		2				/* SCAN STATUS */
#define	ON		3				/* SCAN STATUS */
#define	OFF		4				/* SCAN STATUS */
#define	ON_OFF	8				/* ON - OFF		*/
#define	CAL		9				/* ON - OFF		*/
#define	MRG1	10				/* ON - OFF		*/
#define	MRG2	11				/* ON - OFF		*/
#define	MRG3	12				/* ON - OFF		*/
#define	MRG4	13				/* ON - OFF		*/
#define	MRG5	14				/* ON - OFF		*/
#define	MRG6	15				/* ON - OFF		*/

char	line_buf[256];		/* 1-line Buffer to Read File */

struct	scan_line{
	struct	scan_line	*next_scan_ptr;
	int		scan_type;			/* Scan Type */
	double	start_mjd;			/* Scan Start MJD */
	double	stop_mjd;			/* Scan Stop MJD */
};

struct	spec_info{
	char	obs_name[16];		/* Observation Name	*/
	char	stn_name[16];		/* Station Name	*/
	char	src_name[16];		/* Source Name	*/
	int		type;				/* Spectrum Type	*/
	double	start_mjd;			/* Integ Start MJD	*/
	double	stop_mjd;			/* Integ Stop MJD	*/
	double	integ_sec;			/* On-source Time 	*/
	double	sefd;				/* System Equivalent Flux Density	*/
	int		ss_id;				/* Sub-Stream Info					*/
	int		num_freq;			/* Number of frequency channels		*/
	double	ref_freq;			/* Reference Frequency [MHz]		*/
	double	freq_incr;			/* Frequency increment [MHz]		*/
};
