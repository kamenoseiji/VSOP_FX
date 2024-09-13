/*********************************************************
**	FVANVLECK.C :Conduct Van-Vleck Correction for 1-bit	**
**		Quantized Data (FORTRAN VERSION)				**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>

double	fvanvleck_( freq_num, power, re_ptr, im_ptr )
	int		*freq_num;			/* Number of Spectrum Channels */
	double	*power;				/* Power at the LAG = 0 */
	double	*re_ptr;			/* Pointer of Real Part of Cross Spectrum */
	double	*im_ptr;			/* Pointer of Imag Part of Cross Spectrum */
{
	double	*vis_i_ptr;			/* Pointer of Visibility Real Part */
	double	*vis_r_ptr;			/* Pointer of Visibility IMAG Part */
	double	afact;				/* Amplitude Normalizing Factor */
	int		nfft;				/* Number of FFT Points */
	int		ndim = 1;			/* Data Dimension Used in cfs() */
	int		ndir = 1;			/* FFT Direction : -1 -> inverse FFT */
	int		icon;				/* Condition Code */
	int		freq_index;			/* Index of Frequency */

	nfft	= 2*(*freq_num);

	/*-------- PREPARE MEMORY AREA FOR FFT --------*/
	vis_r_ptr = (double *)malloc( nfft * sizeof(double) );
	vis_i_ptr = (double *)malloc( nfft * sizeof(double) );
	memset( vis_r_ptr, 0, nfft*sizeof(double));
	memset( vis_i_ptr, 0, nfft*sizeof(double));

	/*-------- STORE DATA TO THE MEMORY AREA FOR FFT --------*/
	for(freq_index=0; freq_index<(*freq_num); freq_index++){
		vis_r_ptr[freq_index]	= re_ptr[freq_index];
		vis_i_ptr[freq_index]	= im_ptr[freq_index];
	}
	vis_r_ptr[*freq_num]	= re_ptr[*freq_num - 1];
	vis_i_ptr[*freq_num]	= im_ptr[*freq_num - 1];
	for(freq_index=1; freq_index<*freq_num; freq_index++){
		vis_r_ptr[*freq_num + freq_index]	= re_ptr[*freq_num - freq_index];
		vis_i_ptr[*freq_num + freq_index]	= im_ptr[*freq_num - freq_index];
	}

	/*-------- FFT: SPECTRUM -> CORRELATION FN. --------*/
	dcft_( vis_r_ptr, vis_i_ptr, &nfft, &ndim, &ndir, &icon );

	/*-------- NORMALIZING FACTOR --------*/
	if(*power <= 0){
		/*-------- IN CASE OF AUTOCORRELATION --------*/
		afact	= M_PI_2 / vis_r_ptr[0];
	} else {
		/*-------- USE GIVEN NORMALIZING FACTOR --------*/
		afact	= M_PI_2 / (*power);
	}

	/*-------- VAN VLECK CORRECTION --------*/
	for(freq_index=0; freq_index<nfft; freq_index++){
		vis_r_ptr[freq_index]	= sin(afact * vis_r_ptr[freq_index]);
	}

	/*-------- INVERSE FFT: CORRELATION FN. -> SPECTRUM --------*/
	ndir = -1;
	memset( vis_i_ptr, 0, nfft*sizeof(double));
	dcft_( vis_r_ptr, vis_i_ptr, &nfft, &ndim, &ndir, &icon );

	/*-------- STORE DATA  --------*/
	for(freq_index=0; freq_index<*freq_num; freq_index++){
		re_ptr[freq_index]	= vis_r_ptr[freq_index];
		im_ptr[freq_index]	= vis_i_ptr[freq_index];
	}

	free(vis_r_ptr);
	free(vis_i_ptr);
	if(*power <= 0){ *power	= M_PI_2/afact;}
	return(M_PI_2/afact);
}
