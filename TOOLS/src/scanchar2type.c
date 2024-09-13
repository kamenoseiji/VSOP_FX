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
#define		NUM_TYPE	5

int	scanchar2type(
	char	*scan_type)					/* 1-line buffer		*/
{
	char	ant_stat[][8] = {"ZERO", "R", "SKY", "ON", "OFF"};
	int		scan_id;
	int		index;
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
