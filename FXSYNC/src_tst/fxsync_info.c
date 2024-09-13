/*********************************************************
**	FXSYNC_INFO.C : Read fxsync.info and set the params	**
**			into the shared memory						**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxsync.inc"
#include <cpgplot.h>
#include <unistd.h>
#define INFO_FILE	"fxsync.info"
#define INFO_FMT	"%s"
#define GPIB_FMT	"%s %s %d %d %d"
#define DMS_FMT		"%s %d %s %d"

#define CATEG_COMMENT	0
#define CATEG_GPIB	1
#define CATEG_DMS	2

int	fxsync_info(fxsched_ptr)
	struct shared_mem2 *fxsched_ptr;  /* Pointer of Shared Memory	*/
{
	/*-------- STRING BUFFER --------*/
	FILE	*info_file_ptr;		/* File Pointer of PB Schedule			*/
	char	line_buf[256];		/* Line Buffer of PB Schedule			*/

	/*-------- GENERAL VARIABLE --------*/
	char	category[16];		/* Category in fxsync.info				*/
	char	device[16];			/* Device Name in fxsync.info			*/
	int		categ_id;			/* Category ID							*/
	int		id;					/* Device ID in fxsync.info				*/
	int		address;			/* GPIB Address							*/
	int		brd;				/* Board Number							*/
/*
---------------------------------------------------- OPEN PB Schedule File
*/
	/*-------- SENSE PATH NAME OF THE SCHEDULE FILE --------*/
	if( (info_file_ptr = fopen(INFO_FILE, "r")) == NULL){
		printf("Can't Open %s !!\n", INFO_FILE);	exit(0);
	}
/*
---------------------------------------------------- READ SECOND HEADDER
*/
	while(1){
		if(fgets(line_buf, sizeof(line_buf), info_file_ptr) == 0){	break;}

		/*-------- AVOID COMMENT --------*/
		if(line_buf[0] != '#'){	categ_id = CATEG_COMMENT;	}


		/*-------- SELECT CATEGORY --------*/
		sscanf( line_buf, INFO_FMT, category );
		if( strstr( category, "GPIB") != NULL){	categ_id = CATEG_GPIB; }
		if( strstr( category, "DMS" ) != NULL){	categ_id = CATEG_DMS; }


		switch( categ_id ){

		case CATEG_GPIB:
			sscanf( line_buf, GPIB_FMT, category, device, &id, &brd, &address);

			if(strstr(device, "DMS") != NULL){
				fxsched_ptr->dms_board[id - 1] = brd;
				fxsched_ptr->dms_addr[id - 1] = address;
			}

			if(strstr(device, "TCU") != NULL){
				fxsched_ptr->tcu_board[id] = brd;
				fxsched_ptr->tcu_addr[id] = address;
			}
			break;

		case CATEG_DMS:
			sscanf( line_buf, DMS_FMT, category, &id, device, &address);
			fxsched_ptr->dms_id[address] = id;

			if(strstr(device, "DR1") != NULL){
				fxsched_ptr->pos[address]	= 32;
				fxsched_ptr->dir_id[id-1][0] = address; }

			if(strstr(device, "DR2") != NULL){
				fxsched_ptr->pos[address] = 33;
				fxsched_ptr->dir_id[id-1][1] = address; }

			break;
		}
	}
/*
---------------------------------------------------- LOOP FOR TIME
*/

	fclose(info_file_ptr);
	return(0);
}
