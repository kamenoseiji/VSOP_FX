/*********************************************************
**	PCAL_SPLINE.C: Solution for Atmospheric GAIN 		**
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
#define	DATANUM_LIMIT	3

int	pcal_spline( time_ptr, pcal_ptr, weight_ptr, pcal_num, solint,
		real_coeff_ptr,	imag_coeff_ptr, time_node, node_num_ptr )

	double	*time_ptr;					/* Pointer of Time[sec]				*/
	double	*pcal_ptr;					/* Pointer of P-Cal Phase			*/
	double	*weight_ptr;				/* Pointer of Weight				*/
	int		pcal_num;					/* Number of P-Cal Data				*/
	double	solint;						/* Solution Interval [sec]			*/
	int		*real_coeff_ptr;			/* Pointer of Spline Coefficient	*/
	int		*imag_coeff_ptr;			/* Pointer of Spline Coefficient	*/
	double	*time_node;					/* Node Points for Time				*/
	int		*node_num_ptr;				/* Pointer of Number of Node 		*/
{
	double	*real_ptr;					/* Pointer of P-Cal (Real)			*/
	double	*imag_ptr;					/* Pointer of P-Cal (Imag)			*/
	double	*real_coeff;				/* Pointer of Spline Coeff (Real)	*/
	double	*imag_coeff;				/* Pointer of Spline Coeff (Imag)	*/
	double	real_data;					/* Real Part						*/
	double	imag_data;					/* Imag Part						*/
	double	ref_time;					/* Reference Time at Node			*/
	double	rel_time;					/* Relative Time from ref_time[sec]	*/
	double	current_time;				/* Current Time						*/
	int		node_index;					/* Index of Node Points				*/
	int		data_index;					/* Index of P-Cal Data				*/
	int		data_content;				/* Data Points in Each Node			*/
	int		node_num;					/* Total Number of Node				*/
	double	*resid;						/* Spline Residual					*/
	double	rnor;						/* Spline Residual					*/
	double	*vw;						/* Work Area						*/
	int		*ivw;						/* Work Area						*/
	int		icon;						/* Condition Code					*/
	int		isw;						/* Control Code						*/
	int		dimension;					/* Spline Dimension					*/
	int		index;						/* General Index					*/
/*
----------------------------------------------- CALC NODE POINTS
*/
	dimension	= 3;
	ref_time	= 0.0;
	node_index	= 0;
	data_content= 0;

	for( data_index=0; data_index < pcal_num - DATANUM_LIMIT; data_index++){
		rel_time	= time_ptr[data_index] - ref_time;

		if( (rel_time > solint) && (data_content > DATANUM_LIMIT) ){
			node_index++;
			ref_time =  0.5*(time_ptr[data_index] + time_ptr[data_index - 1]);
			time_node[node_index] =  ref_time;
			data_content	= 0;

		} else {
			data_content ++;
		}
	}

	time_node[0]			= time_ptr[0] 			- solint/2;
	time_node[node_index+1]	= time_ptr[pcal_num-1]	+ solint/2;

	node_num	= node_index + 2;
	*node_num_ptr	= node_num;
/*
----------------------------------------------- MEMORY ALLOCATION
*/
	/*-------- MEMORY AREA FOR P-CAL DATA --------*/
	real_ptr	= (double *)malloc( pcal_num * sizeof(double) );
	imag_ptr	= (double *)malloc( pcal_num * sizeof(double) );

	/*-------- MEMORY AREA FOR SPLINE COEFFICIENT --------*/
	real_coeff	= (double *)malloc((node_num + dimension - 1)* sizeof(double));
	imag_coeff	= (double *)malloc((node_num + dimension - 1)* sizeof(double));

	/*-------- STORE MEMORY ADDRESS --------*/
	*real_coeff_ptr = (int)real_coeff;
	*imag_coeff_ptr = (int)imag_coeff;

	/*-------- WORKING MEMORY AREA FOR SSL2 --------*/
	resid		= (double *)malloc( pcal_num * sizeof(double) );
	vw			= (double *)malloc((node_num + dimension)*(dimension + 1)
									* sizeof(double));
	ivw			= (int *)malloc( pcal_num * sizeof(int));
/*
----------------------------------------------- PHASE -> (real, imag)
*/
	for( data_index=0; data_index < pcal_num; data_index++){
		real_ptr[data_index]	=  cos( pcal_ptr[data_index] );
		imag_ptr[data_index]	=  sin( pcal_ptr[data_index] );
	}
/*
----------------------------------------------- SPLINE
*/
	dbsc1_( time_ptr, real_ptr, weight_ptr, &pcal_num, &dimension,
		time_node, &node_num,
		real_coeff, resid, &rnor, vw, ivw, &icon);
	if( icon != 0 ){
		printf(" CONDITION CODE = %d\n", icon );
		release_memory(real_ptr, imag_ptr, resid, vw, ivw);
		return(-1);
	}

	dbsc1_( time_ptr, imag_ptr, weight_ptr, &pcal_num, &dimension,
		time_node, &node_num,
		imag_coeff, resid, &rnor, vw, ivw, &icon);
	if( icon != 0 ){
		printf(" CONDITION CODE = %d\n", icon );
		release_memory();
		return(-1);
	}
/*
----------------------------------------------- ENDING
*/
	release_memory( real_ptr, imag_ptr, resid, vw, ivw );
	return(0);
}

release_memory( real_ptr, imag_ptr, resid, vw, ivw )
	double	*real_ptr;					/* Pointer of P-Cal (Real)			*/
	double	*imag_ptr;					/* Pointer of P-Cal (Imag)			*/
	double	*resid;						/* Spline Residual					*/
	double	*vw;						/* Work Area						*/
	int		*ivw;						/* Work Area						*/
{
	free( real_ptr );
	free( imag_ptr );
	free( resid );
	free( vw );
	free( ivw );
	return;
}
