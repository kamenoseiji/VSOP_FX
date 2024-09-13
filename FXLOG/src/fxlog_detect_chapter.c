/*********************************************************
**	FXLOG_DETECT_CHAPTER.C								**
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

#include "fxlog.inc"
fxlog_detect_chapter()
{
/*
---------------------------------------------------- READ FILE
*/
	/*------------ Read 1-Line -------------*/
	if(fgets(line_buf, sizeof(line_buf), obs_drg_ptr) == 0){
		drg_eof_flag = 1;				/* Detect End of File */
		return(-1);	
	}

	if(line_buf[0] == '$'){			/* Detect Begin of Chapter */
		sscanf(line_buf, "%1s%s", dum, drg_mode);

		if(strcmp(drg_mode, "EXPER") == 0){				/* EXPER Chap */
			drg_chapter = EXPER;

		} else if(strcmp(drg_mode, "PARAM") == 0){		/* PARAM Chap */
			drg_chapter = PARAM;

		} else if(strcmp(drg_mode, "SOURCES") == 0){	/* SOURCE Chap */
			drg_chapter = SOURCE;

		} else if(strcmp(drg_mode, "STATIONS") == 0){	/* STATION Chap */
			drg_chapter = STATION;

		} else if(strcmp(drg_mode, "SKED") == 0){		/* SKED Chap */
			drg_chapter = SKED;

		} else if(strcmp(drg_mode, "HEAD") == 0){	/* STATION Chap */
			drg_chapter = HEAD;

		} else if(strcmp(drg_mode, "CODES") == 0){		/* CODE Chap */
			drg_chapter = CODE;

		}
	}
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
