/*********************************************************
**	GET_PCALPHASE.C: Solution for Atmospheric GAIN 		**
**						at the Zenith					**
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


int	get_pcalphase( real_coeff,	imag_coeff, time_node,
					node_num,	time,		pcal_phs )
	double	*real_coeff;				/* Pointer of Spline Coefficient	*/
	double	*imag_coeff;				/* Pointer of Spline Coefficient	*/
	double	*time_node;					/* Pointer of Nodes in Spline		*/
	int		node_num;					/* Total Number of Nodes			*/
	double	time;						/* Destination Time					*/
	double	*pcal_phs;					/* Pointer of P-Cal Data			*/
{
	double	real_data;					/* Real Part						*/
	double	imag_data;					/* Imag Part						*/
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
			&time, &index, &real_data, vw, &icon);

	dbsf1_( &dimension, time_node, &node_num, imag_coeff, &isw,
			&time, &index, &imag_data, vw, &icon);

	*pcal_phs = atan2( imag_data, real_data );
/*
----------------------------------------------- ENDING
*/
	free( vw );
	return(0);
}
