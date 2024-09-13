/*********************************************************
**	GET_REAL_SPLINE.C: Solution for Spline Function		**
**														**
**	FUNCTION : Input Baseline-Based Data and Returns 	**
**				Antenna-Based Solution and Error		**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/2/8									**
**	COMMENT : This Module Needs SSL2 (Scientific Sub-	**
**				routine Library) Released by Fujitsu.	** 
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int	get_real_spline( real_coeff, time_node, node_num, time, value_ptr )
	double	*real_coeff;				/* Pointer of Spline Coefficient	*/
	double	*time_node;					/* Pointer of Nodes in Spline		*/
	int		node_num;					/* Total Number of Nodes			*/
	double	time;						/* Destination Time					*/
	double	*value_ptr;					/* Pointer of Estimated Value		*/
{
	int		node_index;					/* Index of Node Points				*/
	int		icon;						/* Condition Code					*/
	int		isw;						/* Control Code						*/
	int		dimension;					/* Spline Dimension					*/
	int		index;						/* General Index					*/
	double	*vw;						/* Work Area						*/

	/*-------- WORKING MEMORY AREA FOR SSL2 --------*/
	dimension	= 3;
	vw			= (double *)malloc((node_num + dimension)*(dimension + 1)
									* sizeof(double));
/*
----------------------------------------------- SPLINE
*/
	isw	= 0;
	index = 0;
	node_index = 0;

	while( node_index < node_num ){
		if( time_node[node_index] > time  ){
			index	= node_index - 1;
			break;
		}
		node_index++;
	}


	dbsf1_( &dimension, time_node, &node_num, real_coeff, &isw,
			&time, &index, value_ptr, vw, &icon);

/*
----------------------------------------------- ENDING
*/
	free( vw );
	return(0);
}
