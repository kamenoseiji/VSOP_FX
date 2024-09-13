/*********************************************************
**	REAL_SPLINE.C: Spline smoothing and interpolation 	**
**						for general data				**
**														**
**	FUNCTION : Input data versus time and calculate the	**
**				spline coefficient.	If proper spline	**
**				function has not been calculated, it	**
**				returns -1.	In successful cas, it re-	** 
**				turns 0.								**
**														**
**	CAUTION	: Time data must be sorted in time order.	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/2/8									**
**	COMMENT : This Module Needs SSL2 (Scientific Sub-	**
**				routine Library) Released by Fujitsu.	** 
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define	DATANUM_LIMIT	3

int	real_spline( time_ptr, data_ptr, weight_ptr, data_num, solint, node_num_ptr,
				spline_coeff_ptr,	time_node_ptr )
	double	*time_ptr;					/* Pointer of Time[sec]				*/
	double	*data_ptr;					/* Pointer of Data					*/
	double	*weight_ptr;				/* Pointer of Weight				*/
	int		data_num;					/* Number of Data					*/
	double	solint;						/* Solution Interval [sec]			*/
	int		*node_num_ptr;				/* Pointer of Number of Node 		*/
	int		*spline_coeff_ptr;			/* Pointer of Spline Coefficient	*/
	int		*time_node_ptr;				/* Node Points for Time				*/
{

	double	*spline_coeff;				/* Pointer of Spline Coefficient	*/
	double	*time_node;					/* Pointer of Node Points			*/
	double	ref_time;					/* Reference Time at Node			*/
	double	rel_time;					/* Relative Time from ref_time[sec]	*/
	double	current_time;				/* Current Time						*/
	double	*resid;						/* Spline Residual					*/
	double	rnor;						/* Spline Residual					*/
	double	*vw;						/* Work Area						*/
	double	*valid_timeptr;				/* Time Pointer of Valid Data		*/
	double	*valid_dataptr;				/* Data Pointer of Valid Data		*/
	double	*valid_wgtptr;				/* Weight Pointer of Valid Data		*/
	int		node_index;					/* Index of Node Points				*/
	int		data_index;					/* Index of P-Cal Data				*/
	int		valid_data_index;			/* Index of Valid Data				*/
	int		data_content;				/* Data Points in Each Node			*/
	int		node_num;					/* Total Number of Node				*/
	int		valid_datanum;				/* Number of Valid Data				*/
	int		*ivw;						/* Work Area						*/
	int		icon;						/* Condition Code					*/
	int		dimension;					/* Spline Dimension					*/
	int		index;						/* General Index					*/
	int		isw;						/* Control Code						*/
/*
----------------------------------------------- CALC NODE POINTS
*/
	dimension	= 3;			/* Third-order B-spline function	*/
	ref_time	= time_ptr[0];	/* Reference Time, i.e Prev. Node	*/
	node_index	= 0;			/* Init Index of Nodes				*/
	data_content= 0;			/* Init Data Index from the Node	*/

#ifdef DEBUG
	printf("  USING REAL_SPLINE : INIT TIME=%lf   END TIME=%lf\n",
		time_ptr[0], time_ptr[data_num - 1]);
#endif

	/*-------- CHECK DATA (Reject Weight = 0.0e0)  --------*/
	valid_datanum = 0;
	for( data_index=0; data_index < data_num; data_index++){
		if( weight_ptr[data_index] > 0.0 ){ valid_datanum ++; }
	}
	valid_timeptr = (double *)malloc( valid_datanum* sizeof(double) );
	valid_dataptr = (double *)malloc( valid_datanum* sizeof(double) );
	valid_wgtptr  = (double *)malloc( valid_datanum* sizeof(double) );

	valid_data_index = -1;
	/*-------- RESTORE DATA  --------*/
	for( data_index=0; data_index < data_num; data_index++){
		if( weight_ptr[data_index] > 0.0 ){
			valid_data_index ++;
			valid_timeptr[valid_data_index] = time_ptr[data_index];
			valid_dataptr[valid_data_index] = data_ptr[data_index];
			valid_wgtptr[valid_data_index]  = weight_ptr[data_index];
		}
	}
	data_num = valid_datanum;
#ifdef DEBUG
	printf("  VALID TIME RANGE  : INIT TIME=%lf   END TIME=%lf\n",
		valid_timeptr[0], valid_timeptr[valid_datanum - 1]);
#endif
/*
----------------------------------------------- CHECK NUMBER of DATA POINTS
*/
	if(data_num <= DATANUM_LIMIT){ return(-1); }

	/*-------- COUNT HOW MANY NODES ARE NECESSARY --------*/
	for( data_index=0; data_index < data_num - DATANUM_LIMIT; data_index++){

		rel_time	= valid_timeptr[data_index] - ref_time;
		if( (rel_time > solint) && (data_content > DATANUM_LIMIT) ){
			node_index++;
			ref_time =  0.5*
				(valid_timeptr[data_index] + valid_timeptr[data_index - 1]);
			data_content	= 0;

		} else if(valid_wgtptr[data_index] > 0.0){
			data_content ++;
		}
	}

	/*-------- MEMORY AREA FOR NODES --------*/
	node_num		= node_index + 2;
	*node_num_ptr	= node_num;
	time_node		= (double *)malloc( node_num* sizeof(double) );
	*time_node_ptr	= (int)time_node;

	node_index	= 0;
	data_content= 0;
	ref_time	= valid_timeptr[0];

	/*-------- STORE NODE DATA --------*/
	for( data_index=0; data_index < data_num - DATANUM_LIMIT; data_index++){
		rel_time	= valid_timeptr[data_index] - ref_time;
		if( (rel_time > solint) && (data_content > DATANUM_LIMIT) ){
			node_index++;
			ref_time =  0.5*
				(valid_timeptr[data_index] + valid_timeptr[data_index - 1]);
			time_node[node_index] =  ref_time;
			data_content	= 0;

		} else {
			data_content ++;
		}
	}
	time_node[0]	= valid_timeptr[0] 	- solint;				/* First Node */
	time_node[node_index+1]	= valid_timeptr[data_num-1]	+ solint;/* Last Node */
/*
----------------------------------------------- MEMORY ALLOCATION
*/
	/*-------- MEMORY AREA FOR SPLINE COEFFICIENT --------*/
	spline_coeff = (double *)malloc((node_num + dimension - 1)* sizeof(double));

	/*-------- STORE MEMORY ADDRESS --------*/
	*spline_coeff_ptr = (int)spline_coeff;

	/*-------- WORKING MEMORY AREA FOR SSL2 --------*/
	resid		= (double *)malloc( data_num * sizeof(double) );
	vw			= (double *)malloc((node_num + dimension)*(dimension + 1)
									* sizeof(double));
	ivw			= (int *)malloc( data_num * sizeof(int));
/*
----------------------------------------------- SPLINE FACTOR by SSL2
*/
	dbsc1_( valid_timeptr, valid_dataptr, valid_wgtptr, &data_num, &dimension,
		time_node, &node_num,
		spline_coeff, resid, &rnor, vw, ivw, &icon);

	if( icon != 0 ){
		printf(" CONDITION CODE = %d in REAL_SPLINE !!\n", icon );
		free( resid );	free( vw );	free( ivw );
		free( valid_timeptr );	free( valid_dataptr );	free( valid_wgtptr );
		return(-1);
	}

#ifdef DEBUG
	for( node_index=0; node_index<node_num; node_index ++ ){
		printf("  SPLINE NODE[%d]  :  TIME=%lf  COEFF=%e\n",
		node_index, time_node[node_index], spline_coeff[node_index]);
	}
#endif
/*
----------------------------------------------- ENDING
*/
	free( resid );	free( vw );	free( ivw );
	free( valid_timeptr );	free( valid_dataptr );	free( valid_wgtptr );
	return(0);
}
