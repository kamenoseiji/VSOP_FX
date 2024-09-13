/*****************************************************************
**	MEAN_FLUX.C	: CALCULATE the mean flux density and SD		**
**																**
**	FUNCTION : Calculate Weighted Mean and Standard Diviation	**
**		of the Input Flux Densities. The weight is given by 	**
**		1/(err*err). If the error is less than or equal to zero,**
**		the weight will be zero as an exception. This function	**
**		returns a count of the invalid weights.					**
**																**
**	AUTHOR	: Seiji Kameno										**
**	CREATED	: 1999 11.4											**
*****************************************************************/

#include <stdio.h>
#include <math.h>

int	mean_flux(
	int		data_num,		/* INPUT:	Total Number of Data						*/
	double	*flux_ptr,		/* INPUT:	Pointer of Flux Densities					*/
	double	*flux_err,		/* INPUT:	Error of Flux Densities						*/
	double	*mean_ptr,		/* OUTPUT:	Mean Flux Density							*/
	double	*sd_ptr ) 		/* OUTPUT:	Standard Diviation of the Flux Densities	*/
{
	int		data_index;				/* Index of Data Points				*/
	int		err_count = 0;			/* Error Counter					*/
	double	sum_flux = 0.0;			/* Summation of the Flux Densities	*/
	double	sum_weight = 0.0;		/* Summation of the Weight			*/
	double	weight = 0.0;			/* Weight							*/
	double	resid = 0.0;			/* Residuals						*/
	double	sum_resid = 0.0;			/* Summation of Residuals			*/


	/*-------- Calc Statictics --------*/
	for(data_index=0; data_index<data_num; data_index ++){
		/*-------- Weight --------*/
		if( flux_err[data_index] > 0.0 ){
			weight = 1.0/ (flux_err[data_index]* flux_err[data_index]);
		} else {
			/*---- Invalid Weight (to avoid division by zero) ----*/
			weight = 0.0;
			err_count ++;
		}
		sum_flux	+= (weight* flux_ptr[data_index]);
		sum_weight	+= weight;
	}
	/*-------- Mean --------*/
	*mean_ptr	= sum_flux / sum_weight;

	sum_weight	= 0.0;
	/*-------- Calc Statictics --------*/
	for(data_index=0; data_index<data_num; data_index ++){

		/*-------- Weight --------*/
		if( flux_err[data_index] > 0.0 ){
			weight = 1.0/ flux_err[data_index];
		} else {
			/*---- Invalid Weight (to avoid division by zero) ----*/
			weight = 0.0;
		}

		resid = weight* (flux_ptr[data_index] - *mean_ptr);
		sum_resid	+= (resid* resid);
		sum_weight  += (weight* weight);
	}

	*sd_ptr		= sqrt(sum_resid /  sum_weight);

	return(err_count);
}
