/*********************************************************
**	read_aos_log: Read AOS Log File 					**
**														**
**	FUNCTION: Open Drudge File and Read.				**
**				Data File.								**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#define		SCAN_FMT	"%*s %*s %s"
#define		NUM_TYPE	5

int	scan2type(
	char	*scan_buf)					/* 1-line buffer		*/
{
	char	ant_stat[][8] = {"ZERO", "R", "SKY", "ON", "OFF"};
	char	scan_type[8];				/* Scan Type				*/
	int		scan_id;
	int		index;
/*
------------------------------------------- PARSE the LINE
*/
	sscanf(scan_buf, SCAN_FMT, scan_type);
/*
------------------------------------------- SCAN TYPE DETECTOR
*/
	scan_id = -1;		/* Default Value	*/
	for(index=0; index<NUM_TYPE; index ++){
		if(strcmp( scan_type, ant_stat[index] ) == 0){
			scan_id = index;
			break;
		}
	}
	return(scan_id);
}
