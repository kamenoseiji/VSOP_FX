/*********************************************************
**	SCAN_LOG.C : Read One-Line from Obs Log and 		** 
**						Select its category.			**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1995/11/26								**
**********************************************************/

#include    <stdio.h>
#include    <stdlib.h>
#include "vlbalog.inc"

int scan_log(log_ptr, keyword, line_buf)
	FILE	*log_ptr;
	char	*keyword;
	char	*line_buf;
{
/*
---------------------------------------------------- READ ONE-LINE
*/
	while( fgets( line_buf, 256, log_ptr ) != NULL ){
		if( strstr( line_buf, keyword ) != NULL){
			return(1);
		}
	}


	return(-1);
}
