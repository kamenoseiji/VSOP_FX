/*********************************************************
**	CPG_INCR.C :	Determin Proper Grid for PGPLOT		**
**														**
**	AUTHOR  : KAMENO Seiji								**
**	CREATED : 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>

int	cpg_incr( range, incr_ptr )
	double	range;
	double	*incr_ptr;
{
	double	factor;
	int		order;

	/*-------- AVOID NEGATIVE RANGE --------*/
	if(range < 0.0){
		range	*= -1.0;
	}

	if(range < 1.0){
		order	= (int)log10(range) - 1;
	} else {
		order	= (int)log10(range);
	}

	factor	= range / exp(M_LN10*order);
	if(factor < 1.4){
		*incr_ptr	= 2.0*exp(M_LN10*(order - 1));
	} else if(factor < 3.2){
		*incr_ptr	= 5.0*exp(M_LN10*(order - 1));
	} else if(factor < 7.0){
		*incr_ptr	= exp(M_LN10*order);
	} else {
		*incr_ptr	= 2.0*exp(M_LN10*order);
	}

	return(0);
}
