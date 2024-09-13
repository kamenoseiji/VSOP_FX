/*********************************************
**	FXLOG_READ_OPEN.C : OPEN Input File		** 
**											**
**	AUTHOR	: KAMENO Seiji					**
**	CREATED	: 1995/11/26					**
**********************************************/

#include <stdio.h>

long fxlog_read_open(input_log_fname, input_file_ptr )

	char	input_log_fname[20];	/* INPUT : Observation Log File Name */
	FILE	**input_file_ptr;		/* OUTPUT: FILE Pointer of Obs Log */
{
/*
---------------------------------------------------- LOAD PARAMETERS
*/
	if(( *input_file_ptr = fopen( input_log_fname, "r" )) == 0){
		printf("Can't Open Previous File [%s]!!\n", input_log_fname);
		exit(0);
	}

	return(0);
}
