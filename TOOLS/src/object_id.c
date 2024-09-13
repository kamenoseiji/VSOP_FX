/*********************************************************
**	OBJECT_ID.C	: LINK SOURCE NAME to OBJECT ID			**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include "obshead.inc"

int	object_id( first_obj_ptr, srcname,  obj_list_ptr, obj_num, obj_id_ptr )
	struct	head_obj	*first_obj_ptr;	/* Pointer of Station Header	*/
	char				*srcname;		/* Source Name					*/
	int					*obj_list_ptr;	/* Pointer of Object Lists		*/
	int					*obj_num;		/* Total Number of Objects		*/
	int					*obj_id_ptr;	/* Pointer of Object ID			*/
{
	struct	head_obj	*obj_ptr;		/* Pointer of Station Header	*/
	struct	obj_list	*obj_info_ptr;	/* Pointer of Oblect List		*/
	int					obj_index;		/* Index for Object				*/


	obj_ptr = first_obj_ptr;
	*obj_id_ptr = -1;
	obj_index = 0;
	/*-------- GET NUMBER OF OBJECTS --------*/
	while( obj_ptr != NULL ){
		obj_index ++;
		obj_ptr = obj_ptr->next_obj_ptr;
	}
	*obj_num	= obj_index;

	/*-------- ALLOC MEMORY --------*/
	obj_info_ptr = (struct obj_list *)malloc(*obj_num*sizeof(struct obj_list)); 
	*obj_list_ptr = (int)obj_info_ptr;
	

	/*-------- RESTORE OBJECT INFO --------*/
	obj_ptr = first_obj_ptr;
	for(obj_index=0; obj_index< *obj_num; obj_index++){

		printf(" SOURCE ID %d : %s\n", obj_ptr->obj_index, obj_ptr->obj_name);
		obj_info_ptr[obj_index].obj_id = obj_ptr->obj_index;
		strcpy( obj_info_ptr[obj_index].obj_name, obj_ptr->obj_name); 
		obj_info_ptr[obj_index].obj_pos[0] = obj_ptr->obj_pos[0];
		obj_info_ptr[obj_index].obj_pos[1] = obj_ptr->obj_pos[1];
		obj_info_ptr[obj_index].obj_pos[2] = obj_ptr->obj_pos[2];

		/*-------- LINK STN-ID to AUTOCORR PAIR ID --------*/
		if( strstr( obj_ptr->obj_name, srcname ) != NULL) {
			*obj_id_ptr = obj_ptr->obj_index;
			sprintf( srcname, "%s", obj_ptr->obj_name);
			return(*obj_id_ptr);
		}

		obj_ptr = obj_ptr->next_obj_ptr;
	}

	return(*obj_id_ptr);
}
