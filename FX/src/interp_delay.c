/*********************************************************
**	INTERP_DELAY.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>
#define PI2			6.28318530717958647688
#define	SECDAY		86400
#define	RADDEG		57.29577951308232087721
#define	CORRDATA	4
#define	CORRFLAG	5
#define CHOFS 0

int	interp_delay( time_ip, node_num, time_node_ptr,
				delay_coeff_ptr, rate_coeff_ptr,
				delay_ptr, rate_ptr)

	double	time_ip;				/* Time from Start of the Day [sec]	*/
	int		node_num;				/* Number of Node for Station		*/
	double	*time_node_ptr;			/* Pointer of Time Node for Station	*/
	double	*delay_coeff_ptr;		/* Pointer of Delay Spline Coeff	*/
	double	*rate_coeff_ptr;		/* Baseline Rate at the Epoch		*/
	double	*delay_ptr;
	double	*rate_ptr;
{

	/*-------- INDEX --------*/
	int		node_index;				/* Index for Node Point				*/
	int		spline_dim;				/* Spline Function Dimension		*/
	int		isw;					/* Control Code						*/
	int		icon;					/* Condition Code					*/

	/*-------- GENERAL VARIABLE --------*/
	double	vw[4];					/* Working Area for SSL2			*/
	double	tmp_delay;

	/*-------- INTERPOLATE DELAY and RATE --------*/
	spline_dim = 3; isw = 0; node_index = 0;

	if(node_num > 1){
		dbsf1_( &spline_dim, time_node_ptr, &node_num, delay_coeff_ptr,
				&isw, &time_ip, &node_index, delay_ptr, vw, &icon);

		dbsf1_( &spline_dim, time_node_ptr, &node_num, rate_coeff_ptr,
				&isw, &time_ip, &node_index, rate_ptr, vw, &icon);

	} else {	/*-------- IN CASE OF REFANT --------*/
		*delay_ptr = 0.0;
		*rate_ptr = 0.0;
	}
	return(0);
}
