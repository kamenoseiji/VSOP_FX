/*********************************************************
**	show_acorr : Integrate Station-Based AutoCorr		**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include "aoslog.inc"
#include <cpgplot.h>
#include <math.h>
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
	double	sum_spec;					/* Summation of the spectral data	*/
	double	ave_spec;					/* Average of the spectral data	*/
	double	sqrsum_spec;				/* Sum of the spectral data squared	*/
	double	sigma_spec;					/* Standard Diviatoin of the spectral data */

	/*-------- SAVE BANDPASS DATA --------*/
	info_spec( argv[ARG_FNAME], &spec_hdr);

	spec_ptr = (double *)malloc(spec_hdr.num_freq* sizeof(double) );
	dumm_ptr = (double *)malloc(spec_hdr.num_freq* sizeof(double) );
	load_spec( argv[ARG_FNAME], &spec_hdr, spec_ptr, &spec_max);
	memset(dumm_ptr, 0, spec_hdr.num_freq* sizeof(double));

	sum_spec = 0;
	sqrsum_spec = 0;
	for(spec_index=0; spec_index < spec_hdr.num_freq; spec_index ++){
		sum_spec += spec_ptr[spec_index];
		sqrsum_spec += (spec_ptr[spec_index]* spec_ptr[spec_index]);
	}
	printf("SUM = %lf\n", sum_spec);
	ave_spec = sum_spec / spec_hdr.num_freq;
	sigma_spec = sqrt(sqrsum_spec/spec_hdr.num_freq - ave_spec* ave_spec );
	printf("AVE = %lf\n", ave_spec);
	printf("SQRSUM = %lf\n", sqrsum_spec);
	printf("SIGMA = %lf\n", sigma_spec);

#ifdef PLOT
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
#endif
	free(spec_ptr);
	free(dumm_ptr);
	return(0);
}
