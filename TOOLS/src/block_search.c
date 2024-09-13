/*********************************************************
**	block_search.c	: Search for Start PP Using Block	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 2002/3/7									**
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "obshead.inc"
#define	MAX_BLOCK	1024

int	block_search(
	int		block_num,				/* Total Number of Blocks			*/
	struct block_info	*block,		/* Pointer to the Block Info		*/
	double	start_mjd)				/* INTEG START TIME [MJD] 			*/
{
	int		block_index;			/* Index for Blocks					*/
	int		result;					/* Search Result [PP or -1]			*/
	double	result_double;			/* Guessed PP in Double Precision	*/
/*
--------------------------------- STEP 0 : SKIP TO POSITION
*/
	block_index = 0;
	while( block[block_index].st < start_mjd){
#ifdef DEBUG
		printf("INDEX %d:  %lf %lf %lf\n",
				block_index, block[block_index].st, block[block_index].et, start_mjd);
#endif
		if(block[block_index].et > start_mjd){
			result_double = (double)(block[block_index].et_pp
								  -  block[block_index].st_pp);
			result_double *= start_mjd;
			result_double -= (block[block_index].st* block[block_index].et_pp);
			result_double += (block[block_index].et* block[block_index].st_pp);
			result_double /= (block[block_index].et - block[block_index].st);
			result = (int)(result_double + 0.99);
			return(result);
		}
		block_index ++;
	}

	result	= -1;
	return(result);
}
