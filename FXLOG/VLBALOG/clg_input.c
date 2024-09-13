/*********************************************************
**	CLG_INPUT.C : Read One-Line from Obs Log and 		** 
**						Select its category.			**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1995/11/26								**
**********************************************************/

#include "vlbalog.inc"

int clg_input(input_text, message)
	char	*input_text;
	char	*message;
{
/*
---------------------------------------------------- READ ONE-LINE
*/

	printf("Please Input %s : ", message);
	gets( input_text );

	return(0);
}
