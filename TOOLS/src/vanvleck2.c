/*********************************************************
**	VANVLECK2.C :Conduct Van-Vleck Correction for 2-bit	**
**		Quantized Data									**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>

double	vanvleck2( freq_num, node_num, spline_node, spline_fact,
					power, re_ptr, im_ptr )
	int		freq_num;			/* Number of Spectrum Channels */
	int		node_num;			/* Number of Node					*/
	double	*spline_node;		/* Pointer of Spline Node			*/
	double	*spline_fact;		/* Pointer of Spline Factor			*/
	double	power;				/* Power at the LAG = 0 */
	double	*re_ptr;			/* Pointer of Real Part of Cross Spectrum */
	double	*im_ptr;			/* Pointer of Imag Part of Cross Spectrum */
{
	double	*vis_i_ptr;			/* Pointer of Visibility Real Part	*/
	double	*vis_r_ptr;			/* Pointer of Visibility IMAG Part	*/
	double	afact;				/* Amplitude Normalizing Factor		*/
	double	rho;				/* Correlation Coefficient			*/
	double	rho2;				/* Correlation Coefficient			*/
	double	vw[64];				/* Work Area for SSL2				*/
	int		spline_dim;			/* Dimension of Spline Function		*/
	int		node_index;			/* Index for Node					*/
	int		nfft;				/* Number of FFT Points				*/
	int		ndim = 1;			/* Data Dimension Used in cfs()		*/
	int		ndir;				/* FFT Direction : -1 -> inverse FFT */
	int		icon;				/* Condition Code					*/
	int		isw;				/* Control Code in SSL2				*/
	int		freq_index;			/* Index of Frequency				*/

	nfft	= 2*freq_num;

	/*-------- PREPARE MEMORY AREA FOR FFT --------*/
	vanvleck2_init( &node_num, &spline_node, &spline_fact );

	/*-------- PREPARE MEMORY AREA FOR FFT --------*/
	vis_r_ptr = (double *)malloc( nfft * sizeof(double) );
	vis_i_ptr = (double *)malloc( nfft * sizeof(double) );
	memset( vis_r_ptr, 0, nfft*sizeof(double));
	memset( vis_i_ptr, 0, nfft*sizeof(double));

	/*-------- STORE DATA TO THE MEMORY AREA FOR FFT --------*/
	for(freq_index=0; freq_index<freq_num; freq_index++){
		vis_r_ptr[freq_index]	= re_ptr[freq_index];
		vis_i_ptr[freq_index]	= im_ptr[freq_index];
	}

	vis_r_ptr[freq_num]	=  re_ptr[freq_num - 1];
	vis_i_ptr[freq_num]	= -im_ptr[freq_num - 1];
	for(freq_index=1; freq_index<freq_num; freq_index++){
		vis_r_ptr[freq_num + freq_index]	=  re_ptr[freq_num - freq_index];
		vis_i_ptr[freq_num + freq_index]	= -im_ptr[freq_num - freq_index];
	}

	/*-------- FFT: SPECTRUM -> CORRELATION FN. --------*/
	ndir = 1;
	dcft_( vis_r_ptr, vis_i_ptr, &nfft, &ndim, &ndir, &icon );

	/*-------- NORMALIZING FACTOR --------*/
	if(power <= 0){
		/*-------- IN CASE OF AUTOCORRELATION --------*/
		afact	= 1.0 / vis_r_ptr[0];

	} else {
		/*-------- USE GIVEN NORMALIZING FACTOR --------*/
		afact	= 1.0 / (power* nfft);
	}

	/*-------- VAN VLECK 2-bit CORRECTION --------*/
	node_index	= 0;
	spline_dim	= 3;
	isw			= 0;
	for(freq_index=0; freq_index<nfft; freq_index++){

		rho2 = afact* vis_r_ptr[freq_index];
		dbsf1_(&spline_dim, spline_node, &node_num, spline_fact,
			&isw, &rho2, &node_index, &rho, vw, &icon);
		vis_r_ptr[freq_index] = rho;

		rho2 = afact* vis_i_ptr[freq_index];
		dbsf1_(&spline_dim, spline_node, &node_num, spline_fact,
			&isw, &rho2, &node_index, &rho, vw, &icon);
		vis_i_ptr[freq_index] = rho;
	}

	/*-------- INVERSE FFT: CORRELATION FN. -> SPECTRUM --------*/
	ndir = -1;
	dcft_( vis_r_ptr, vis_i_ptr, &nfft, &ndim, &ndir, &icon );

	/*-------- STORE DATA  --------*/
	for(freq_index=0; freq_index<freq_num; freq_index++){
		re_ptr[freq_index]	= vis_r_ptr[freq_index];
		im_ptr[freq_index]	= vis_i_ptr[freq_index];
	}

	free(vis_r_ptr);
	free(vis_i_ptr);
	return(1.0/((double)nfft *afact) );
}
