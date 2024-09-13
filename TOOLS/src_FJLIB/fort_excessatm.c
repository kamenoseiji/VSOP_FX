/*************************************************
**	FORT_EXCESSATM : Excess Path by Atmosphere	**	
**												**
**	AUTHOR : KAMENO Seiji						**	
**************************************************/

#include <stdio.h>
#include <math.h>

#define	TEMP_KELV	273.16	
#define	DRY_FACT0	0.00228	
#define	DRY_FACT1	0.0013	
#define	WET_FACT0	750.0	
#define	WET_FACT1	0.0003	

int	fort_excessatm_( temp_ptr, press_ptr, humid_ptr, sin_el_ptr, dsecz_ptr,
				excess_path_ptr, excess_diff_ptr )
	double	*temp_ptr;			/* Temperature [K]						*/
	double	*press_ptr;			/* Atmospheric Pressure [hPa]			*/
	double	*humid_ptr;			/* Relative Humidity [%]				*/
	double	*sin_el_ptr;		/* sin( EL )							*/
	double	*dsecz_ptr;			/* d[sec z]/dt							*/
	double	*excess_path_ptr;	/* Pointer of Excess Path [m]			*/
	double	*excess_diff_ptr;	/* Pointer of Excess Path Speed [m/s]	*/
{
	double	pvs;				/* Saturated Water Vapor Pressure [hPa]	*/ 
	double	pv;					/* Ground Water Vapor Pressure [hPa]	*/ 
	double	tanz2;				/* tan^2(z)								*/

	/*-------- sin(el) -> tan(z)[ 0 < el < pi/2] -----------*/ 
	tanz2	= 1.0/((*sin_el_ptr)*(*sin_el_ptr)) - 1.0;

	/*-------- CALC Saturated Water Vapor Pressure ---------*/
	/* This formulation is based on Crane (1976). Please 	*/
	/* see Thompson, Moran and Swenson "Interferometry and 	*/
	/* Synthesys in Radio Astronomy" p.410					*/

	pvs = (6.11 * exp(-5.3*log((*temp_ptr)/TEMP_KELV))
				* exp(25.2 * ((*temp_ptr) - TEMP_KELV) / (*temp_ptr) ) );
	pv	= pvs * (*humid_ptr) * 0.01;	/* Water Vapor Pressure on the Ground */

	/*-------- Excess Path by Wator Vapor ------------------*/
	/* This formulation assume that the atmosphere is 		*/
	/* isothermal and a scale height of water vapor is 2 km	*/ 

	*excess_path_ptr	= WET_FACT0* pv* (1.0 - WET_FACT1* tanz2)
						/ ((*temp_ptr)* (*temp_ptr)* (*sin_el_ptr));

	*excess_diff_ptr 	= WET_FACT0* pv
				* ((*dsecz_ptr) *(1.0 - WET_FACT1* tanz2)
				- 2.0* WET_FACT1* (*dsecz_ptr)/ ((*sin_el_ptr)* (*sin_el_ptr)) )
				/ ((*temp_ptr)* (*temp_ptr)) ;

	/*-------- Excess Path by Dry Atmosphere ---------------*/
	/* This formulation assumes isothermalatmosphere		*/ 

	*excess_path_ptr +=  (DRY_FACT0* (*press_ptr)
						* (1.0 - DRY_FACT1* tanz2)/ (*sin_el_ptr) ) ;
	*excess_diff_ptr += (DRY_FACT0* (*press_ptr)*
			((*dsecz_ptr)* (1.0 - DRY_FACT1* tanz2)
			- 2.0* DRY_FACT1* (*dsecz_ptr)/ ((*sin_el_ptr)* (*sin_el_ptr)) ));
	return(0);
}
