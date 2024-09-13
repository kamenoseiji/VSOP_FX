/*********************************************************
**	CORASE.C: COARSE FRINGE SEARCH for Each Baseline	**
**														**
**	FUNCTION: INPUT VISIILITY DATA and SOLVE DELAY, 	**
**				DELAY RATE, DELAY ACCERELATION for		**
**				each STATION							**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/3/14									**
*********************************************************/
#include	"cpgplot.h"
#include	<stdio.h>
#include	<math.h>
#define		PI		3.14159265358979323846
#define		PI2		6.28318530717958647688

long	coarse(vr_ptr, vi_ptr,	spec_num, freq_init, freq_incr,
			time_num, time_incr,
			vis_amp_ptr, vis_phs_ptr,
			delay_ptr, rate_ptr )

	float	*vr_ptr;		/* INPUT : Pointer of Visibility (Real) Data */	 
	float	*vi_ptr;		/* INPUT : Pointer of Visibility (Real) Data */
	long	spec_dim;		/* INPUT : Dimension Number for Spectrum */

	double	freq_init;		/* INPUT : Initial Frequency [MHz] */
	double	freq_incr;		/* INPUT : Frequency Increment [MHz] */

	long	time_num;		/* INPUT : Number of Time */
	double	time_incr;		/* INPUT : Time Increment [sec] */

	float	*vis_amp_ptr;	/* OUTPUT: Pointer of Visibility amp */
	float	*vis_phs_ptr;	/* OUTPUT: Pointer of Visibility amp */

	double	*delay_ptr;		/* OUTPUT: Pointer of Delay [microsec] */
	double	*rate_ptr;		/* OUTPUT: Pointer of Delay Rate [picosec/sec] */
{
	long	ndim;				/* Number of Dimension of the Data */
	long	icon;				/* Condition Code in SSL2 */
	long	nfft[2];
	long	ndir;				/* FFT DIRECTION */
	long	irate;
	long	idelay;
/*
---------------------------------------------- FFT
*/
	nfft[0] = spec_num;
	nfft[1] = time_num;
	ndim = 2;	ndir = 1;
	cft_( vr, vi, nfft, &ndim, &ndir, &icon);

	return(0);
}
