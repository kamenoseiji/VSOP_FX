/*********************************************************
**	POLY_FIT.C: Polynominal Fitting for Genaral Data 	**
**														**
**	FUNCTION : Input data versus time and calculate the	**
**				Mean, Rate, Acc.				 		**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/2/8									**
**	COMMENT : This Module Needs SSL2 (Scientific Sub-	**
**				routine Library) Released by Fujitsu.	** 
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define	DATANUM_LIMIT	3

int	poly_fit( time_ptr, data_ptr, weight_ptr, data_num,
				poly_coeff )
	double	*time_ptr;					/* Pointer of Time[sec]				*/
	double	*data_ptr;					/* Pointer of Data					*/
	double	*weight_ptr;				/* Pointer of Weight				*/
	int		data_num;					/* Number of Data					*/
	int		*poly_coeff;				/* Polynominal Coefficient			*/
{
	double	*poly_coeff;				/* Pointer of Spline Coefficient	*/
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
*/
----------------------------------------------- SPLINE
*/
	sum_w	= 0;	sum_wt	= 0;	sum_wtt	= 0;
	sum_wttt= 0;	sum_wtttt=0;	sum_wx	= 0;
	sum_wxt	= 0;	sum_wxtt= 0;
	for(index=0; index<data_num; index++){
		wgt = weight_ptr[index]* weight_ptr[index];
		tm	= time_ptr[index];
		x	= data_ptr[index];

		sum_w		+= wgt;
		sum_wt		+= (wgt* tm);
		sum_wtt		+= (wgt* tm* tm);
		sum_wttt	+= (wgt* tm* tm* tm);
		sum_wtttt	+= (wgt* tm* tm* tm* tm);
		sum_wx		+= (wgt* x);
		sum_wxt		+= (wgt* x* tm);
		sum_wxtt	+= (wgt* x* tm* tm);
	}



	dbsc1_( time_ptr, data_ptr, weight_ptr, &data_num, &dimension,
		time_node, &node_num,
		spline_coeff, resid, &rnor, vw, ivw, &icon);

	if( icon != 0 ){
		printf(" CONDITION CODE = %d\n", icon );
		free( resid );	free( vw );	free( ivw );
		return(-1);
	}
/*
----------------------------------------------- ENDING
*/
	free( resid );	free( vw );	free( ivw );
	return(0);
}
