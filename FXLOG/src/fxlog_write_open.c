/*********************************************
**	FXLOG_WRITE_OPEN.C : OPEN OUTput File	** 
**											**
**	AUTHOR	: KAMENO Seiji					**
**	CREATED	: 1995/11/26					**
**********************************************/

#include <stdio.h>

long fxlog_write_open(output_log_fname, output_file_ptr )

	char	output_log_fname[20];	/* INPUT : Observation Log File Name */
	FILE	**output_file_ptr;		/* OUTPUT: FILE Pointer of Obs Log */
{
/*
---------------------------------------------------- LOAD PARAMETERS
*/
	if(( *output_file_ptr = fopen( output_log_fname, "w" )) == 0){
		printf("Can't Open New File [%s]!!\n", output_log_fname);
		exit(0);
	}

	return(0);
}
