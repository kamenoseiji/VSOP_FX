/*************************************************
**	EXCESS_ATM : Excess Path by Atmosphere 		**	
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

int	excess_atm( temp, press, humid, sin_el, dsecz,
				excess_path_ptr, excess_diff_ptr )
	double	temp;				/* Temperature [K]						*/
	double	press;				/* Atmospheric Pressure [hPa]			*/
	double	humid;				/* Relative Humidity [%]				*/
	double	sin_el;				/* sin( EL )							*/
	double	dsecz;				/* d[sec z]/dt							*/
	double	*excess_path_ptr;	/* Pointer of Excess Path [m]			*/
	double	*excess_diff_ptr;	/* Pointer of Excess Path Speed [m/s]	*/
{
	double	pvs;				/* Saturated Water Vapor Pressure [hPa]	*/ 
	double	pv;					/* Ground Water Vapor Pressure [hPa]	*/ 
	double	tanz2;				/* tan^2(z)								*/

	/*-------- sin(el) -> tan(z)[ 0 < el < pi/2] -----------*/ 
	tanz2	= 1.0/(sin_el*sin_el) - 1.0;

	/*-------- CALC Saturated Water Vapor Pressure ---------*/
	/* This formulation is based on Crane (1976). Please 	*/
	/* see Thompson, Moran and Swenson "Interferometry and 	*/
	/* Synthesys in Radio Astronomy" p.410					*/

	pvs = (6.11 * exp(-5.3*log(temp/TEMP_KELV))
				* exp(25.2 * (temp - TEMP_KELV) / temp ) );
	pv	= pvs * humid * 0.01;	/* Water Vapor Pressure on the Ground */

	/*-------- Excess Path by Wator Vapor ------------------*/
	/* This formulation assume that the atmosphere is 		*/
	/* isothermal and a scale height of water vapor is 2 km	*/ 

	*excess_path_ptr	= WET_FACT0* pv* (1.0 - WET_FACT1* tanz2)
						/ (temp* temp* sin_el);

	*excess_diff_ptr 	= WET_FACT0* pv
						* (dsecz *(1.0 - WET_FACT1* tanz2)
						- 2.0* WET_FACT1* dsecz/ (sin_el* sin_el) )
						/ (temp* temp) ;

	/*-------- Excess Path by Dry Atmosphere ---------------*/
	/* This formulation assumes isothermalatmosphere		*/ 

	*excess_path_ptr +=  (DRY_FACT0* press* (1.0 - DRY_FACT1* tanz2)/ sin_el ) ;
	*excess_diff_ptr += (DRY_FACT0* press*
						(dsecz* (1.0 - DRY_FACT1* tanz2)
						- 2.0* DRY_FACT1* dsecz/ (sin_el* sin_el) ));

	return(0);
}
