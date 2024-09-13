/*********************************************************
**	show_acorr : Integrate Station-Based AutoCorr		**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include "aoslog.inc"
#include <math.h>
#include <cpgplot.h>
#define	ARG_FNAME	1
#define	ARG_DEVICE	2
#define	MAX(a,b)	( (a>b)?a:b )


MAIN__(
	int		argc,			/* Number of Arguments 		*/
	char	**argv)			/* Pointer to Arguments 	*/
{
	struct spec_info	spec_hdr;		/* Header Information		*/
	int		spec_index;					/* Index for Spectral Point	*/
	double	*spec_ptr;					/* Spectrum Data			*/
	double	*dumm_ptr;					/* Dummy Spectrum Data		*/
	double	spec_max;					/* Spectral Max				*/
	double	spec_mean;					/* Mean of Flux Densities	*/
	double	spec_sd;					/* Standard Diviation of Flux Densities */

	/*-------- READ HEADER INFO --------*/
	info_spec( argv[ARG_FNAME], &spec_hdr);

	/*-------- LOAD SPECTRAL DATA --------*/
	spec_ptr = (double *)malloc(spec_hdr.num_freq* sizeof(double) );
	dumm_ptr = (double *)malloc(spec_hdr.num_freq* sizeof(double) );
	load_spec( argv[ARG_FNAME], &spec_hdr, spec_ptr, &spec_max);

#ifdef HIDOI
	for(spec_index=0; spec_index < 448; spec_index ++){
		spec_ptr[spec_index] += 0.08;
	}

	for(spec_index=2040; spec_index < spec_hdr.num_freq; spec_index ++){
		spec_ptr[spec_index] -= 0.025;
	}
#endif

	/*-------- STATISTICAL INFO --------*/
	for(spec_index=0; spec_index < spec_hdr.num_freq; spec_index ++){
		spec_sd = spec_hdr.sefd* 1.0e-3 / sqrt(spec_hdr.integ_sec* spec_hdr.freq_incr);
		dumm_ptr[spec_index] = spec_sd;
	}
	mean_flux(spec_hdr.num_freq, spec_ptr, dumm_ptr, &spec_mean, &spec_sd);
	printf(" FLUX Average = %lf,  SD = %lf\n", spec_mean, spec_sd);


	/*-------- DUMMY DATA for BANDPASS --------*/
	memset(dumm_ptr, 0, spec_hdr.num_freq* sizeof(double));

	cpgbeg(1, argv[ARG_DEVICE], 1, 1);
	/*-------- PLOT BANDPASS DATA --------*/
	cpg_bp( spec_hdr.obs_name,
			spec_hdr.stn_name,
			spec_hdr.src_name,
			spec_hdr.start_mjd,
			spec_hdr.stop_mjd,
			spec_hdr.integ_sec,
			1,						/* Num of SS		*/
			&spec_hdr.num_freq,
			&spec_hdr.ref_freq,
			&spec_hdr.freq_incr,
			&spec_max,
			&spec_ptr,
			&dumm_ptr);

	cpgend();
	free(spec_ptr);
	free(dumm_ptr);
	return(0);
}
