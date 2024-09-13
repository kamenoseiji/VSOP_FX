/*********************************************************
**	CLG_SELECTOR.C : Read One-Line from Obs Log and 	** 
**						Select its category.			**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1995/11/26								**
**********************************************************/

#include "fxlog.inc"

int fxlog_selector(line_buf)
{
	char	category_detector[12];
/*
---------------------------------------------------- READ ONE-LINE
*/

	/*-------- Detect Comment Line --------*/
	if( strchr(line_buf, '/') == NULL){	/* Comment Line */
		log_category = COMMENT;
		return(0);
	}

	/*-------- Detect LOG Category --------*/
	for( i=1; i< MAX_CATEGORY_NUM; i++){ 
		sprintf( category_detector, "/%s/", category_array[i]);

		if( (log_pointer = strstr(line_buf, category_detector)) != NULL){
			log_category = i;				/* Category Type Number */
			log_pointer = log_pointer		/* Where is Category */
				+ strlen(category_detector) - 1;
			*log_pointer = ' ';				/* Replace / to ` ' */
		}
	}
		
	return(0);
}
