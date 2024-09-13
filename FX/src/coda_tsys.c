/*********************************************************
**	CODA_TSYS.C : Read Tsys Data and Put Into CFS		**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"

#define GCALDATA    4
#define	MAX_SS	32							/* Maximum Number of SS */
#define	SECPERDAY	86400
#define OBS_NAME	1
#define	STATION		2
#define TSYSFILE	3

MAIN__( argc, argv )
	int		argc;			/* Number of Arguments */
	char	**argv;			/* Pointer of Arguments */
{
	/*-------- STRUCT for HEADDER in CFS --------*/
	struct header	obs;			/* OBS Headder						*/
	struct head_obj	*obj_ptr;		/* Pointer of Object Header			*/
	struct head_obj	*first_obj_ptr;	/* Pointer of Object Header			*/
	struct head_stn	*stn_ptr;		/* Pointer of Station Header		*/
	struct head_stn	*first_stn_ptr;	/* Pointer of Station Header		*/

	/*-------- SHARED MEMORY --------*/
	int		shrd_obj_id;			/* Shared Memory ID for Source Info	*/
	int		shrd_stn_id;			/* Shared Memory ID for Station Info*/

	/*-------- TOTAL NUMBER --------*/
	int		data_num;				/* Total Number of Tsys Data		*/
	int		cfs_ssnum;				/* Total SS Number in CFS			*/
	int		ncval;					/* Character Data Bytes				*/
	int		nival;					/* Number of Integer Data			*/
	int		nrval;					/* Number of Daouble Data			*/

	/*-------- IDENTIFIER --------*/
	int		cfs_stnid;				/* Station Id in CFS				*/
	int		recid;					/* Record ID in CFS					*/

	/*-------- INDEX --------*/
	int		data_index;				/* Index of Tsys Data				*/
	int		ss_index;				/* Index of Sub-Stream				*/

	/*-------- POINTER -------*/
	double	*mjd_ptr;				/* MJD of Tsys Data					*/
	double	*sefd_ptr;				/* SEFD Data						*/

	/*-------- CFS VARIABLE -------*/
	int		ret;					/* Return Code from CFS Library		*/
	int		lunit;					/* Unit Number of HEADDER			*/
	int		gclunit;				/* Unit Number of GCAL Data			*/
	char	fname[128];				/* File Name of CFS					*/
	char	omode[2];				/* CFS Access Mode					*/
	char	cvalue[64];				/* Char Data in CFS					*/
	double	loss;					/* Loss Factor						*/
	double	rvalue;					/* Double Value	in CFS				*/
	int		*int_ptr;				/* Pointer for Integer Data			*/
	double	*double_ptr;			/* Pointer for Double Data			*/

/*
------------------------ CHECK FOR INPUT ARGUMENT
*/
	if( argc < 4 ){
		printf("USAGE : coda_tsys [OBS_NAME] [STN_NAME] [TSYS FILE] !!\n");
		printf("  OBS_NAME ------ Observation Code [e.g. D96135]\n");
		printf("  STN_NAME ------ STATION NAMEs \n");
		printf("  TSYS FILE ----- Tsys File Name \n");
		exit(0);
	}
/*
------------------------ ACCESS TO THE CODA FILE SYSTEM (CFS)
*/
	cfs000_( &ret );			cfs_ret( 000, ret );
	cfs020_( &ret );			cfs_ret( 020, ret );
	cfs006_( argv[1], &ret, strlen( argv[1] )); cfs_ret( 006, ret );

	/*-------- CFS HEADDER FILE OPEN --------*/
	lunit = 3;
	sprintf( fname, "HEADDER" ); sprintf( omode, "r" );
	cfs287_( fname, &ret, strlen(fname) );  cfs_ret( 287, ret );
	cfs103_( &lunit, fname, omode, &ret, strlen(fname), strlen(omode) );
	cfs_ret( 103, ret );

	/*-------- READ OBSHEAD --------*/
	read_obshead( lunit, &obs, &obj_ptr, &stn_ptr, &shrd_obj_id, &shrd_stn_id );
	first_obj_ptr   = obj_ptr;
	first_stn_ptr   = stn_ptr;
	acorr_pair( &obs, stn_ptr, &cfs_ssnum, &loss);

	/*-------- FIND SPECIFIED STATION in CFS --------*/
    stn_ptr = first_stn_ptr;
    while(stn_ptr != NULL){
        /*-------- SEEK SPECIFIED STATION --------*/
        if( strstr(stn_ptr->stn_name, argv[STATION]) != NULL ){
            printf("STATION %-10s: ID = %2d\n",
                stn_ptr->stn_name, stn_ptr->stn_index );
            cfs_stnid = stn_ptr->stn_index;
            break;
        } else {
            stn_ptr = stn_ptr->next_stn_ptr;
        }
    }
/*
------------------------ READ TSYS (SEFD) DATA
*/
	data_num = read_tsys( 1999, argv[TSYSFILE], &mjd_ptr, &sefd_ptr);
/*
----------------------------------------- OPEN GCAL-DATA FILE IN CFS
*/
	gclunit = GCALDATA;
	sprintf( omode, "w" );
	sprintf(fname, "CALIB/GCAL.%d", cfs_stnid );
	cfs287_( fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_( &gclunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );
/*
----------------------------------------- WRITE HEADDER IN GCAL-DATA
*/
	cfs401_( &gclunit, &ret ); cfs_ret(401,ret);/* Go to the File Top		*/
	recid	= 0;								/* Rec ID for HEADDER is 0	*/
	nival=2;	ncval=64;	nrval=0;			/* Number of parameters		*/
	int_ptr	= (int *)malloc(nival*sizeof(int));	/* Alloc Memory for INT		*/
	memset( cvalue, 0x20, ncval );				/* Fill 0x20 for FORTRAN file */
	sprintf( &cvalue[0],  "%s", argv[OBS_NAME]);/* Write Obs Name			*/
	sprintf( &cvalue[32], "%s", stn_ptr->stn_name);	/* Write Station Name	*/
	int_ptr[0]	= cfs_stnid;					/* Station ID				*/
	int_ptr[1]	= cfs_ssnum;					/* Total Number of SS		*/
	cfs126_( &gclunit, &recid, &nival, int_ptr,	/* Write Headder to CFS		*/
		&nrval, &rvalue, &ncval, cvalue,
		&ret, ncval ); cfs_ret(126, ret);
	cfs402_(&gclunit, &ret); cfs_ret(402,ret);	/* Go to the File END		*/
/*
----------------------------------------- WRITE SEFD DATA INTO GCAL-DATA
*/
	/*-------- PREPARATION TO SAVE GAIN INTO CFS  --------*/
	recid	= 1;							/* Record ID for GAIN DATA	*/
	nival	= 0;							/* Number of Integer Param.	*/
	nrval	= 2 + 4*cfs_ssnum;				/* Number of Real Parameter	*/
	ncval	= 32;							/* Byte Number of Char Param*/
	memset( cvalue, 0x20, 64 );				/* Fill 0x20 for FORTRAN	*/
	sprintf( &cvalue[0],  "%s", obj_ptr->obj_name);	/* Write Obj Name	*/
	double_ptr = (double *)malloc(nrval * sizeof(double));

	/*-------- SAVE SEFD INTO CFS  --------*/
	for(data_index=0; data_index<data_num; data_index++){
		double_ptr[0]	= mjd_ptr[data_index];
		double_ptr[1]	= 1.0;
		for( ss_index=0; ss_index<cfs_ssnum; ss_index++){
			double_ptr[2+ ss_index]	= 0.0;
			double_ptr[2+ cfs_ssnum + ss_index]	= sefd_ptr[data_index];
			double_ptr[2+ cfs_ssnum*2 +ss_index]	= 0.0;
			double_ptr[2+ cfs_ssnum*3 +ss_index]	= 1.0;
		}
		cfs126_( &gclunit, &recid, &nival, int_ptr, &nrval, double_ptr,
				&ncval, cvalue, &ret, ncval ); cfs_ret( 126, ret );
	}
	free(int_ptr);
	free(double_ptr);
	cfs104_( &gclunit, &ret );  cfs_ret( 104, ret );

	/*-------- CLOSE SHARED MEMORY --------*/
	shmctl( shrd_obj_id, IPC_RMID, 0 );
	shmctl( shrd_stn_id, IPC_RMID, 0 );

	return;
}
