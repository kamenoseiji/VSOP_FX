/*********************************************************
**	DETECT_CHAPTER.C									**
**														**
**	FUNCTION: Read Each Line of Drudge File.			**
**				The character of "$" at the top of line	**
**				means the start of the chapter. This	**
**				function detects the begin of the chap-	**
**				ter and recognize the mode.				**
**				When the end of file is detected, it 	**
**				returns -1. Otherwise, it returns 1.	**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "drudge.inc"
long detect_chapter(current_chapter)
	long	current_chapter;		/* Current Chapter ID */
{
	char	drg_mode[10];			/* Chapter Name in DRG */
	char	dum[256];				/* Dummy String */
	long	chapter_detect_flag;	/* DETECT->1, UNDETECT->0 */
	long	i;						/* General Dummy Counter */
/*
---------------------------------------------------- DETECT CHAPTER
*/
	if(line_buf[0] == '$'){			/* Detect Begin of Chapter */
		chapter_detect_flag = 0;	/* Refresh for New Chapter */

		sscanf(line_buf, "%1s%s", dum, drg_mode);
		for(i=0; i<MAX_CHAPTER; i++){
			if(strcmp(drg_mode, chapter_array[i]) == 0){

				/*-------- CHAPTER IS IDENTIFIED ! --------*/
				current_chapter = i;
				chapter_detect_flag = 1;
				return(current_chapter);
			}
		}

		/*-------- UNIDENTIFIED CHAPTER --------*/
		if(chapter_detect_flag == 0){
			printf("WARNING: Undefined Chapter [%s] ! \n", drg_mode);
			return(-1);
		}
	}
/*
---------------------------------------------------- ENDING
*/
	return(current_chapter);
}
