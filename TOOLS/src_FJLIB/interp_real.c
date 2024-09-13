/*********************************************************
**	INTERP_REAL.C	: Test Module to Read CODA File System	**
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

int	interp_real( time_ip, node_num, time_node_ptr, coeff_ptr, real_ptr)

	double	time_ip;				/* INPUT: Second of Day [sec]		*/
	int		node_num;				/* INPUT: Number of Nodes 			*/
	double	*time_node_ptr;			/* INPUT: Pointer of Time Node		*/
	double	*coeff_ptr;				/* INPUT: Pointer of Spline Coeff.	*/
	double	*real_ptr;				/* OUTPUT:Pointer of Interporated Value	*/
{
	/*-------- INDEX --------*/
	int		node_index;				/* Index for Node Point				*/
	int		spline_dim;				/* Spline Function Dimension		*/
	int		isw;					/* Control Code						*/
	int		icon;					/* Condition Code					*/

	/*-------- GENERAL VARIABLE --------*/
	double	vw[4];					/* Working Area for SSL2			*/
	double	tmp_value;

	/*-------- INTERPOLATE DELAY and RATE --------*/
	spline_dim = 3; isw = 0; node_index = 0;

	if(node_num > 1){
		dbsf1_( &spline_dim, time_node_ptr, &node_num, coeff_ptr,
				&isw, &time_ip, &node_index, real_ptr, vw, &icon);

	} else {	/*-------- IN CASE OF REFANT --------*/
		*real_ptr = 0.0;
	}
	return(0);
}
