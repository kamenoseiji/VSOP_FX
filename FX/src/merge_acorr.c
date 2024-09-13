/*********************************************************
**	show_acorr : Integrate Station-Based AutoCorr		**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include "aoslog.inc"
#include <math.h>
#define	ARG_OUTNAME	1
#define	ARG_INNAME	2
#define	MAX_FILE   32
#define	MAX(a,b)	( (a>b)?a:b )


MAIN__(
	int		argc,			/* Number of Arguments 		*/
	char	**argv)			/* Pointer to Arguments 	*/
{

	int		file_num;							/* Number of Files			*/

	struct spec_info	inspec_hdr;				/* Header Information		*/
	struct spec_info	outspec_hdr;			/* Header Information		*/
	double	*spec_ptr[MAX_FILE];				/* Spectrum Data			*/
	double	spec_max;							/* Spectral Max				*/
	double	weight;								/* Weight for Integration	*/
	double	weight_sum;							/* Sum of the Weight 		*/
	int		file_index;							/* Index for Spectral File	*/
	int		spec_index;							/* Index for Spectral Point	*/

	file_num = argc - ARG_INNAME;				/* Number of Files	*/
	info_spec(argv[ARG_INNAME], &inspec_hdr);
	spec_ptr[file_num] = (double *)malloc( inspec_hdr.num_freq* sizeof(double) );	/* Output Storage	*/
	memset(spec_ptr[file_num], 0, inspec_hdr.num_freq* sizeof(double) );

	weight_sum = 0.0;
	outspec_hdr.integ_sec  = 0.0;
	/*-------- LOAD DATA FROM FILE --------*/
	for(file_index = 0; file_index < file_num; file_index ++){
		info_spec(argv[ARG_INNAME + file_index], &inspec_hdr);
		spec_ptr[file_index] = (double *)malloc( inspec_hdr.num_freq* sizeof(double) );
		load_spec(argv[ARG_INNAME + file_index], &inspec_hdr, spec_ptr[file_index], &spec_max);

		/*-------- ACCUMULATE --------*/
		weight = inspec_hdr.integ_sec / (inspec_hdr.sefd* inspec_hdr.sefd);
		weight_sum += weight;
		outspec_hdr.integ_sec += inspec_hdr.integ_sec;
		printf("INTEG = %lf\n", inspec_hdr.integ_sec);
		for(spec_index=0; spec_index < inspec_hdr.num_freq; spec_index ++){
			*(spec_ptr[file_num]) += (weight* (*spec_ptr[file_index]) );
			spec_ptr[file_num] ++;
			spec_ptr[file_index] ++;
		}
		spec_ptr[file_index] -= inspec_hdr.num_freq;
		spec_ptr[file_num] 	 -= inspec_hdr.num_freq;

		/*-------- HEADER --------*/
		if(file_index == 0){
			outspec_hdr.start_mjd = inspec_hdr.start_mjd; }
		if(file_index == (file_num -1)){
			outspec_hdr.stop_mjd = inspec_hdr.stop_mjd; }
	}

	outspec_hdr.sefd = sqrt( outspec_hdr.integ_sec / weight_sum);
	printf("TOTAL INTEG = %lf   SEFD = %lf\n", outspec_hdr.integ_sec, outspec_hdr.sefd);

	for(spec_index=0; spec_index < inspec_hdr.num_freq; spec_index ++){
		*(spec_ptr[file_num]) /= weight_sum;
/*
		printf("SPEC = %lf\n", *spec_ptr[file_num]);
*/
		spec_ptr[file_num] ++;
	}
	spec_ptr[file_num] 	 -= inspec_hdr.num_freq;

	/*-------- SAVE DATA --------*/
	dump_spec(
		argv[ARG_OUTNAME],
		inspec_hdr.obs_name,
		inspec_hdr.stn_name,
		inspec_hdr.src_name,
		outspec_hdr.start_mjd,
		outspec_hdr.stop_mjd,
		CAL + 1,							/* FILE Descriptor	*/
		outspec_hdr.sefd,					/* SEFD [calibrated]*/
		&outspec_hdr.integ_sec,				/* INTEG	*/
		1,									/* SSNUM	*/
		inspec_hdr.ss_id,					/* SS ID	*/
		&(inspec_hdr.num_freq),
		&(inspec_hdr.ref_freq),
		&(inspec_hdr.freq_incr),

		&spec_ptr[file_num]);

	for(file_index = 0; file_index < file_num; file_index ++){
		free(spec_ptr[file_index]);
	}
	free(spec_ptr[file_num]);
	return(0);
}




