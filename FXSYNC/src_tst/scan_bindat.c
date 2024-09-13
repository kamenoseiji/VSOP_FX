/*********************************************************
**	SCAN_BINDAT.C : Read fxsync.info and set the params	**
**			into the shared memory						**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxsync.inc"
#include <cpgplot.h>
#include <unistd.h>

int	scan_bindat(fxsched_ptr, dms_id, tape_label, bin_id_ptr)
	struct shared_mem2 *fxsched_ptr;	/* Pointer of Shared Memory	*/
	int		dms_id;					/* DMS ID					*/
	char	*tape_label;			/* Tape Label				*/
	int		*bin_id_ptr;			/* Bin ID					*/
{
	int		bin_index;				/* Index for Bin			*/
/*
---------------------------------------------------- OPEN PB Schedule File
*/
	*bin_id_ptr = -1;
	for(bin_index=0; bin_index<MAX_BIN; bin_index++){

		#ifdef DEBUG
		printf("TARGET LABEL=%s,  CURRENT BIN [%d]= %s\n",
			tape_label, bin_index,
			fxsched_ptr->binlabel[dms_id][bin_index] );
		#endif

		if(strstr( fxsched_ptr->binlabel[dms_id][bin_index],
			tape_label) != NULL){
			*bin_id_ptr = bin_index + 1;
			break;
		}
	}
/*
---------------------------------------------------- LOOP FOR TIME
*/
	return(*bin_id_ptr);
}
