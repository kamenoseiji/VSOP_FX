/*********************************************************
**	FXLOG_OPEN.C : OPEN Input and Output Files			** 
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1995/11/26								**
**********************************************************/

#include <stdio.h>

long fxlog_open(input_log_fname,	output_log_fname,
				input_file_ptr,		output_file_ptr )

	char	input_log_fname[20];	/* INPUT : Observation Log File Name */
	char	output_log_fname[20];	/* INPUT : FX Log File Name */
	FILE	**input_file_ptr;		/* OUTPUT: FILE Pointer of Obs Log */
	FILE	**output_file_ptr;		/* OUTPUT: FILE Pointer of FX Log */
{
/*
---------------------------------------------------- LOAD PARAMETERS
*/
	if(( *input_file_ptr = fopen( input_log_fname, "r" )) == 0){
		printf("Can't Open Log File [%s]!!\n", input_log_fname);
		exit(0);
	}

	if(( *output_file_ptr = fopen( output_log_fname, "w" )) == 0){
		printf("Can't Open Log File [%s]!!\n", output_log_fname);
		exit(0);
	}

	return(0);
}
