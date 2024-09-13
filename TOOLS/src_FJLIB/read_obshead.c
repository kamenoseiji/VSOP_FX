/*****************************************************************
**	READ_OBSHEAD.C	: Module to Read OBSHEAD in CODA File System**
**																**
**	AUTHOR	: KAMENO Seiji										**
**	CREATED	: 1996/6/27											**
*****************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "obshead.inc"

int	read_obshead( lunit, obs_ptr, obj_ptr, stn_ptr,
					shrd_obj_id_ptr, shrd_stn_id_ptr )
	int		lunit;						/* Logical Unit Number */
	struct	header		*obs_ptr;		/* OBS HEADDER POINTER */
	struct	head_obj	**obj_ptr;		/* OBJECT HEADDER POINTER */
	struct	head_stn	**stn_ptr;		/* STATION HEADDER POINTER */
	int		*shrd_obj_id_ptr;			/* Shared Memory ID */
	int		*shrd_stn_id_ptr;			/* Shared Memory ID */
{
	int		ret;		/* Return Code from CFS Library */
	int		nival;
	int		nrval;
	int		ncval;
	int		ivalue;
	double	rvalue;
	char	keywd[32];
	char	cvalue[80];
	int		i;
	key_t	obj_key;
	key_t	stn_key;

	struct	head_obj	*new_obj_ptr;
	struct	head_obj	*prev_obj_ptr;
	struct	head_stn	*new_stn_ptr;
	struct	head_stn	*prev_stn_ptr;

	/*-------- OBSERVATION CODE --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "obscode" );
	nival	= 0;	nrval	= 0;	ncval	= 16;
	cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, &rvalue,
			&ncval, obs_ptr->obscode, &ret, strlen(keywd), ncval );
	cfs_ret( 117, ret );

	/*-------- NUMBER OF CORRELATION PAIR --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "ncor" );
	nival	= 1;	nrval	= 0;	ncval	= 0;
	cfs117_( &lunit, keywd, &nival, &obs_ptr->cor_num, &nrval, &rvalue,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	/*-------- NUMBER OF SOURCES --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "nobj" );
	nival	= 1;	nrval	= 0;	ncval	= 0;
	cfs117_( &lunit, keywd, &nival, &obs_ptr->obj_num, &nrval, &rvalue,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	/*-------- OPEN SHARED MEMORY FOR SOURCES --------*/
	obj_key	= ftok(KEY_DIR, OBJ_KEY);
	if(( *shrd_obj_id_ptr	= shmget( obj_key,
		obs_ptr->obj_num*sizeof(struct head_obj), IPC_CREAT |0644)) < 0){
		printf("Error in [shmget] !!\n");
		exit(1);
	}
	new_obj_ptr	= (struct head_obj *)shmat(*shrd_obj_id_ptr, NULL, 0);
	memset(new_obj_ptr, 0, obs_ptr->obj_num*sizeof(struct head_obj));

	/*-------- REMEMBER FIRST POINTER --------*/
	*obj_ptr	= new_obj_ptr;
	prev_obj_ptr= new_obj_ptr;

	/*-------- READ OF SOURCE INFORMATION --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	for(i=0; i < obs_ptr->obj_num; i++){

		/*-------- READ OBJECT INDEX --------*/
		sprintf( keywd, "objnum" );
		nival	= 1;	nrval	= 0;	ncval	= 0;
		cfs117_( &lunit, keywd, &nival, &new_obj_ptr->obj_index, &nrval,
			&rvalue, &ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
		cfs_ret( 117, ret );
	
		/*-------- READ OBJECT NAME --------*/
		sprintf( keywd, "objnam" );
		nival	= 0;	nrval	= 0;	ncval	= 16;
		cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, &rvalue, 
		&ncval, new_obj_ptr->obj_name, &ret, strlen(keywd), ncval );
		cfs_ret( 117, ret );

		/*-------- READ OBJECT POSITION --------*/
		sprintf( keywd, "objpos" );
		nival	= 0;	nrval	= 3;	ncval	= 0;
		cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, new_obj_ptr->obj_pos,
				&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
		cfs_ret( 117, ret );
	
		/*-------- LINK FROM PREVIOUS OBJECT --------*/
		prev_obj_ptr->next_obj_ptr	= new_obj_ptr;
		new_obj_ptr->next_obj_ptr	= NULL;
		prev_obj_ptr				= new_obj_ptr;
		new_obj_ptr++;
	}

	/*-------- NUMBER OF STATIONS --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "nsta" );
	nival	= 1;	nrval	= 0;	ncval	= 0;
	cfs117_( &lunit, keywd, &nival, &obs_ptr->stn_num, &nrval, &rvalue,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	/*-------- OPEN SHARED MEMORY FOR STATION --------*/
	stn_key	= ftok(KEY_DIR, STN_KEY);
	if((*shrd_stn_id_ptr = shmget(stn_key, 
		obs_ptr->stn_num*sizeof(struct head_stn), IPC_CREAT | 0644)) < 0 ){
		printf("Error in [shmget] !! \n");
		exit(1);
	}
	new_stn_ptr	= (struct head_stn *)shmat(*shrd_stn_id_ptr, NULL, 0);
	memset( new_stn_ptr, 0, obs_ptr->stn_num*sizeof(struct head_stn) );

	/*-------- REMEMBER FIRST POINTER --------*/
	*stn_ptr	= new_stn_ptr;
	prev_stn_ptr= new_stn_ptr;

	/*-------- READ OF STATION INFORMATION --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	for(i=0; i < obs_ptr->stn_num; i++){

		new_stn_ptr->acorr_index	= -1;

		/*-------- READ OBJECT INDEX --------*/
		sprintf( keywd, "stanum" );
		nival	= 1;	nrval	= 0;	ncval	= 0;
		cfs117_( &lunit, keywd, &nival, &new_stn_ptr->stn_index, &nrval,
			&rvalue, &ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
		cfs_ret( 117, ret );

		/*-------- READ STATION NAME --------*/
		sprintf( keywd, "stanam" );
		nival	= 0;	nrval	= 0;	ncval	= 16;
		cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, &rvalue, 
		&ncval, new_stn_ptr->stn_name, &ret, strlen(keywd), ncval );
		cfs_ret( 117, ret );

		/*-------- READ OBJECT POSITION --------*/
		sprintf( keywd, "stapos" );
		nival	= 0;	nrval	= 3;	ncval	= 0;
		cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, new_stn_ptr->stn_pos,
				&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
		cfs_ret( 117, ret );

		/*-------- LINK FROM PREVIOUS STATION --------*/
		prev_stn_ptr->next_stn_ptr	= new_stn_ptr;
		new_stn_ptr->next_stn_ptr	= NULL;
		prev_stn_ptr				= new_stn_ptr;
		new_stn_ptr++;
	}

	/*-------- START and END TIME --------*/
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	nival	= 0;	nrval	= 1;	ncval	= 0;
	sprintf( keywd, "obsmjd1" );
	cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, &obs_ptr->start_mjd,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	sprintf( keywd, "obsmjd2" );
	cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, &obs_ptr->stop_mjd,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	/*-------- MISC PARAMETERS --------*/
	nival	= 0;	nrval	= 1;	ncval	= 0;
	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "epoch" );
	cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, &obs_ptr->epoch,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "gstia0" );
	cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, &obs_ptr->gstia0,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "degpdy" );
	cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, &obs_ptr->degpdy,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "polarx" );
	cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, &obs_ptr->polarx,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );
	sprintf( keywd, "polary" );
	cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, &obs_ptr->polary,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );

	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "ut1utc" );
	cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, &obs_ptr->ut1utc,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	cfs401_( &lunit, &ret );	cfs_ret( 401, ret );
	sprintf( keywd, "iatutc" );
	cfs117_( &lunit, keywd, &nival, &ivalue, &nrval, &obs_ptr->iatutc,
			&ncval, cvalue, &ret, strlen(keywd), sizeof(cvalue) );
	cfs_ret( 117, ret );

	/*-------- FILE CLOSE --------*/
	cfs104_( &lunit, &ret );    cfs_ret( 104, ret );

	return(0);
}
