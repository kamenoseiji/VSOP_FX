/*********************************************************
**	OBJECT_ID.C	: LINK SOURCE NAME to OBJECT ID			**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include "obshead.inc"

int	objct_id( obj_ptr, srcname,  obj_id_ptr )
	struct	head_obj	*obj_ptr;		/* Pointer of Station Header */
	char				*srcname;		/* Source Name */
	int					*obj_id_ptr;	/* Pointer of Object ID */
{
	*obj_id_ptr = -1;
	while( obj_ptr != NULL ){

		/*-------- LINK STN-ID to AUTOCORR PAIR ID --------*/
		if( strstr( obj_ptr->obj_name, srcname ) != NULL) {
			printf(" SOURCE ID %d : %s\n", obj_ptr->obj_index, obj_ptr->obj_name);
			*obj_id_ptr = obj_ptr->obj_index;
			sprintf( srcname, "%s", obj_ptr->obj_name);
			return(*obj_id_ptr);
		}

		obj_ptr = obj_ptr->next_obj_ptr;
	}

	return(*obj_id_ptr);
}
