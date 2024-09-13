/*****************************************************************
**	TIME_BLOCK_CODA.C : Check Time Tags in CODA					**
**																**
**	AUTHOR	: KAMENO Seiji										**
**	CREATED	: 1996/6/27											**
*****************************************************************/
#include "obshead.inc"
#include <stdio.h>
#include <math.h>
#define	CORRFLAG	5
#define	SECDAY		86400

int	time_block_coda( bl_index, ss_index, freq_num, time_gap, block )

	int		bl_index;				/* Baseline Index 					*/
	int		ss_index;				/* Sub-Stream Index 				*/
	int		freq_num;				/* Number of Frequency				*/
	double	time_gap;				/* Gap Threshold between block		*/
	struct block_info	*block;		/* Pointer of Block Informatoin		*/
	
{
	int		flgunit;				/* Logical Unit Number for FLAG data */
	int		block_index;			/* Index for Data Block				*/
	int		current_pp;				/* Current PP Number				*/
	int		ret;					/* Return Code from CFS Library		*/
	char	omode[2];				/* CFS Access Mode 					*/
	char	fname[32];				/* CFS File Name 					*/
	double	mjd_flag;				/* Current MJD						*/
	double	prev_mjd;				/* Previous MJD						*/
	double	uvw[3];					/* (u, v, w) coordinate [m]			*/
	char	flag[1024];				/* Frag at every freq. ch			*/
	int		current_obj;			/* Current Object ID				*/
	int		prev_obj;				/* Previous Object ID				*/
	int		block_num;				/* How Many DATA Was Valid */
/*
------------------------------- INITIALIZE MEMORY AREA FOR VISIBILITIES
*/

	flgunit = CORRFLAG;
	sprintf(omode, "r"); 

	/*-------- OPEN FLAG DATA --------*/
	sprintf(fname, "CORR.%d/SS.%d/FLAG.1\0", bl_index, ss_index ); 
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&flgunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );
	cfs401_( &flgunit, &ret);	/* Rewind to the Top	*/

	/*-------- READ VISIBILITY AND FLAG DATA --------*/
	block_index	= 0;
	current_pp	= 0;
	prev_obj = -9999;
	ret = 0;
	while( 1 ){

		/*-------- LOAD VISIBILITY TO WORK AREA --------*/
		cfs225_( &flgunit, &mjd_flag, &current_obj, uvw,
					flag, &freq_num, &ret );
		if( ret != 0 ){	break;	}

/*
		printf("FLAG = %02X %02X %02X %02X \n",
			flag[0], flag[1], flag[2], flag[3]);
		current_obj *= (!flag[0]);
*/

		/*-------- New Block ? --------*/
		if(  (current_obj !=  prev_obj)
		  || (SECDAY* (mjd_flag - prev_mjd) > time_gap) ){
			block[block_index].st		= mjd_flag;
			block[block_index].st_pp	= current_pp;
			block[block_index].obj		= current_obj;

			if( block_index > 0){
				block[block_index - 1].et		= prev_mjd;
				block[block_index - 1].et_pp	= current_pp - 1;
			}

			block_index ++;
		}

		prev_obj = current_obj;
		prev_mjd = mjd_flag;
		current_pp ++;
	}

	block[block_index - 1].et	= mjd_flag;
	block[block_index - 1].et_pp= current_pp - 1;
	block_num = block_index;

	/*-------- CLOSE FLAG FILE --------*/
	cfs104_( &flgunit, &ret );	cfs_ret( 104, ret );

	return(block_num);
}
