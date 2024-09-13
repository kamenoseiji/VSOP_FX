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
#define		NSPEC	16
#define		NTIME	256	
#define		PI		3.14159265358979323846
#define		PI2		6.28318530717958647688

long	coarse(vr_ptr, vi_ptr,	spec_num, spec_dim, freq_init, freq_incr,
			time_num, time_incr,
			vis_amp_ptr, vis_phs_ptr, vis_snr_ptr,
			delay_ptr, delay_err_ptr, rate_ptr, rate_err_ptr )

	float	*vr_ptr;		/* INPUT : Pointer of Visibility (Real) Data */	 
	float	*vi_ptr;		/* INPUT : Pointer of Visibility (Real) Data */
	long	spec_num;		/* INPUT : Number of Spectrum */
	long	spec_dim;		/* INPUT : Dimension Number for Spectrum */
	double	freq_init;		/* INPUT : Initial Frequency [MHz] */
	double	freq_incr;		/* INPUT : Frequency Increment [MHz] */
	long	time_num;		/* INPUT : Number of Time */
	double	time_incr;		/* INPUT : Time Increment [sec] */
	double	*vis_amp_ptr;	/* OUTPUT: Pointer of Visibility amp */
	double	*vis_phs_ptr;	/* OUTPUT: Pointer of Visibility amp */
	double	*vis_snr_ptr;	/* OUTPUT: Pointer of Visibility S/N */
	double	*delay_ptr;		/* OUTPUT: Pointer of Delay [microsec] */
	double	*delay_err_ptr;	/* OUTPUT: Pointer of Delay [microsec] */
	double	*rate_ptr;		/* OUTPUT: Pointer of Delay Rate [picosec/sec] */
	double	*rate_err_ptr;	/* OUTPUT: Pointer of Delay Rate [picosec/sec] */
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
	long	icon;				/* Condition Code in SSL2 */
	long	nfft[2];
	long	ndir;				/* FFT DIRECTION */
	float	vis_max;
	long	irate;
	long	idelay;
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

	for(itime=time_num; itime<pow2round(time_num); itime++){
		for(ispec=0; ispec<NSPEC; ispec++){
			vr[itime][ispec]	= 0.0;
			vi[itime][ispec]	= 0.0;
		}
	}
/*
---------------------------------------------- FFT
*/
	nfft[0] = NSPEC;
	nfft[1] = pow2round(time_num);
	ndim = 2;	ndir = 1;
	cft_( vr, vi, nfft, &ndim, &ndir, &icon);
/*
---------------------------------------------- PEAK SEARCH
*/
	for(itime=0; itime<nfft[1]/2; itime++){
		index_time	= nfft[1]/2 + itime;
		for(ispec=0; ispec<nfft[0]/2; ispec++){
			index_spec	= nfft[0]/2 + ispec;
			vis_amp[itime][ispec]
				= sqrt(vr[index_time][index_spec]*vr[index_time][index_spec]
				+ vi[index_time][index_spec]*vi[index_time][index_spec]);

			vis_amp[itime][index_spec]
				= sqrt(vr[index_time][ispec]*vr[index_time][ispec]
				+ vi[index_time][ispec]*vi[index_time][ispec]);

			vis_amp[index_time][ispec]
				= sqrt(vr[itime][index_spec]*vr[itime][index_spec]
				+ vi[itime][index_spec]*vi[itime][index_spec]);

			vis_amp[index_time][index_spec]
				= sqrt(vr[itime][ispec]*vr[itime][ispec]
				+ vi[itime][ispec]*vi[itime][ispec]);

		}
	}

	for(itime=0; itime<nfft[1]; itime++){
		for(ispec=0; ispec<nfft[0]; ispec++){
			if( vis_amp[itime][ispec] > vis_max ){
				vis_max = vis_amp[itime][ispec];
				irate	= itime - nfft[1]/2;
				idelay	= ispec - nfft[0]/2;
			}
		}
	}

	*vis_amp_ptr= (double)vis_max / (double)(time_num * spec_num);
	*vis_phs_ptr= atan2((double)vi[irate + nfft[1]/2][idelay + nfft[0]/2],
						(double)vr[irate + nfft[1]/2][idelay + nfft[0]/2]);

	#ifdef DEBUG
	printf("REAL = %f, IMAG = %f\n, PHASE= %lf",
				vr[irate + nfft[1]/2][idelay + nfft[0]/2],
				vi[irate + nfft[1]/2][idelay + nfft[0]/2],
				*vis_phs_ptr); 
	#endif

	*vis_snr_ptr=*vis_amp_ptr*sqrt(1.0e6*spec_num*freq_incr*time_num*time_incr);
	*delay_ptr	= (double)idelay / (spec_num * freq_incr);
	*rate_ptr	= 1.0e6*(double)irate /(nfft[1] * time_incr * freq_init);

	if(*vis_snr_ptr > 7.0){
		*delay_err_ptr	= 1.0/(*vis_snr_ptr * spec_num * freq_incr);
		*rate_err_ptr	= 1.0/(*vis_snr_ptr * nfft[1] * time_incr * freq_init);
	} else {
		*delay_err_ptr	= 1.0;
		*rate_err_ptr	= 1.0;
		*vis_amp_ptr= *vis_amp_ptr * (*vis_snr_ptr - 1)/(*vis_snr_ptr);
	}

	return(0);
}
