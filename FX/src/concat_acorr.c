/*********************************************************
**	concat_acorr : Concatinate Power Spectra			**
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
	double	*out_spec;							/* Output Spectrum Data		*/
	double	spec_max;							/* Spectral Max				*/
	int		file_index;							/* Index for Spectral File	*/

	file_num = argc - ARG_INNAME;				/* Number of Files	*/

	/*-------- CALC NUM OF FREQ CHANNELs --------*/
	outspec_hdr.num_freq = 0;
	for(file_index = 0; file_index < file_num; file_index ++){
		info_spec(argv[ARG_INNAME + file_index], &inspec_hdr);
		outspec_hdr.num_freq += inspec_hdr.num_freq;
		if(file_index == 0){
			strcpy( outspec_hdr.obs_name, inspec_hdr.obs_name);
			strcpy( outspec_hdr.stn_name, inspec_hdr.stn_name);
			strcpy( outspec_hdr.src_name, inspec_hdr.src_name);
			outspec_hdr.type		= inspec_hdr.type;			/* File Type (MRG)	*/
			outspec_hdr.start_mjd	= inspec_hdr.start_mjd;		/* Time Range 		*/
			outspec_hdr.stop_mjd	= inspec_hdr.stop_mjd;		/* Time Range 		*/
			outspec_hdr.integ_sec	= inspec_hdr.integ_sec;		/* Time Range 		*/
			outspec_hdr.sefd		= inspec_hdr.sefd;			/* Time Range 		*/
			outspec_hdr.ss_id		= 0;						/* Whole SS 		*/
			outspec_hdr.ref_freq	= inspec_hdr.ref_freq;		/* Ref. Freq. 		*/
			outspec_hdr.freq_incr	= inspec_hdr.freq_incr;		/* Ref. Freq. 		*/
		}
		printf("Freq. Range: %lf - %lfMHz\n",
				inspec_hdr.ref_freq,
				inspec_hdr.ref_freq + inspec_hdr.num_freq * inspec_hdr.freq_incr);
	}


	/*-------- Prepere for Spectral Data Area --------*/
	out_spec = (double *)malloc( outspec_hdr.num_freq* sizeof(double) );	/* Output Storage	*/
	memset(out_spec, 0, outspec_hdr.num_freq* sizeof(double) );

	/*-------- LOAD DATA FROM FILE --------*/
	for(file_index = 0; file_index < file_num; file_index ++){
		info_spec(argv[ARG_INNAME + file_index], &inspec_hdr);
		load_spec(argv[ARG_INNAME + file_index], &inspec_hdr, out_spec, &spec_max);
		out_spec += inspec_hdr.num_freq;
	}
	out_spec -= outspec_hdr.num_freq;

	/*-------- SAVE DATA --------*/
	dump_spec(
		argv[ARG_OUTNAME],
		outspec_hdr.obs_name,
		outspec_hdr.stn_name,
		outspec_hdr.src_name,
		outspec_hdr.start_mjd,
		outspec_hdr.stop_mjd,
		outspec_hdr.type,					/* FILE Descriptor	*/
		outspec_hdr.sefd,					/* SEFD [calibrated]*/
		&outspec_hdr.integ_sec,				/* INTEG	*/
		1,									/* SSNUM	*/
		inspec_hdr.ss_id,					/* SS ID	*/
		&(outspec_hdr.num_freq),
		&(outspec_hdr.ref_freq),
		&(outspec_hdr.freq_incr),
		&out_spec);

	free(out_spec);

	return(0);
}
