/*********************************************************
**	CORASE_MULT.C: COARSE FRINGE SEARCH for Multi-Stream**
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
#define		NSPEC	32
#define		NTIME	512	
#define		PI		3.14159265358979323846
#define		PI2		6.28318530717958647688

long	coarse_mult( vr_ptr, vi_ptr, spec_num, spec_dim, freq_init,
					 freq_incr, time_num, time_incr, vis_amp_ptr)

	float	*vr_ptr;		/* INPUT : Pointer of Visibility (Real) Data */	 
	float	*vi_ptr;		/* INPUT : Pointer of Visibility (Real) Data */
	long	spec_num;		/* INPUT : Number of Spectrum */
	long	spec_dim;		/* INPUT : Dimension Number for Spectrum */
	double	freq_init;		/* INPUT : Initial Frequency [MHz] */
	double	freq_incr;		/* INPUT : Frequency Increment [MHz] */
	long	time_num;		/* INPUT : Number of Time */
	double	time_incr;		/* INPUT : Time Increment [sec] */
	float	*vis_amp_ptr;	/* OUTPUT : Pointer of Visibility Amplitude */
{
	long	i, j, k;
	long	itime;				/* Time Index */
	long	ispec;				/* Spectrum Index */
	long	ndim;				/* Number of Dimension of the Data */
	long	index_time;
	long	index_spec;
	float	vr[NTIME][NSPEC];
	float	vi[NTIME][NSPEC];
	float	vis_amp[NTIME][NSPEC];
	float	vis_max;
	long	icon;				/* Condition Code in SSL2 */
	long	nfft[2];
	long	ndir;				/* FFT DIRECTION */

	nfft[0] = spec_num*2;
	nfft[1] = pow2round(time_num)*2;
/*
---------------------------------------------- DATA STORE
*/
	/*-------- LOOP FOR TIME POINTS --------*/
	for(itime=0; itime<time_num; itime++){

		/*-------- LOOP FOR SPECTRAL POINTS --------*/
		for(ispec=0; ispec<spec_num; ispec++){
			vr[itime][ispec]	= *vr_ptr;	vr_ptr++;
			vi[itime][ispec]	= *vi_ptr;	vi_ptr++;
		}
		for(ispec=spec_num; ispec<NSPEC; ispec++){
			vr[itime][ispec]	= 0.0;
			vi[itime][ispec]	= 0.0;
		}
		vr_ptr = vr_ptr + (spec_dim - spec_num);
		vi_ptr = vi_ptr + (spec_dim - spec_num);
	}

	for(itime=time_num; itime<nfft[1]; itime++){
		for(ispec=0; ispec<NSPEC; ispec++){
			vr[itime][ispec]	= 0.0;
			vi[itime][ispec]	= 0.0;
		}
	}
/*
---------------------------------------------- FFT
*/
	ndim = 2;	ndir = 1;
	cft_( vr, vi, nfft, &ndim, &ndir, &icon);
/*
---------------------------------------------- INTEGRATE FOR SUB-STREAM
*/
	for(itime=nfft[1]/2; itime<nfft[1]; itime++){
		for(ispec=nfft[0]/2; ispec<nfft[0]; ispec++){
			*vis_amp_ptr	= *vis_amp_ptr
							+ sqrt(vr[itime][ispec]*vr[itime][ispec]
							+ vi[itime][ispec]*vi[itime][ispec]);
			vis_amp_ptr++;
		}
		for(ispec=0; ispec<nfft[0]/2; ispec++){
			*vis_amp_ptr	= *vis_amp_ptr
							+ sqrt(vr[itime][ispec]*vr[itime][ispec]
							+ vi[itime][ispec]*vi[itime][ispec]);
			vis_amp_ptr++;
		}
	}
	for(itime=0; itime<nfft[1]/2; itime++){
		for(ispec=nfft[0]/2; ispec<nfft[0]; ispec++){
			*vis_amp_ptr	= *vis_amp_ptr
							+ sqrt(vr[itime][ispec]*vr[itime][ispec]
							+ vi[itime][ispec]*vi[itime][ispec]);
			vis_amp_ptr++;
		}
		for(ispec=0; ispec<nfft[0]/2; ispec++){
			*vis_amp_ptr	= *vis_amp_ptr
							+ sqrt(vr[itime][ispec]*vr[itime][ispec]
							+ vi[itime][ispec]*vi[itime][ispec]);
			vis_amp_ptr++;
		}
	}

	return(0);
}
