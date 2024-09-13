/*********************************************************
**	split_acorr : Split Power Spectra					**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include "aoslog.inc"
#include <math.h>
#define	ARG_INNAME	1
#define	ARG_STARTCH	2
#define	ARG_STOPCH	3
#define	ARG_OUTNAME	4
#define	MAX(a,b)	( (a>b)?a:b )


MAIN__(
	int		argc,			/* Number of Arguments 		*/
	char	**argv)			/* Pointer to Arguments 	*/
{

	int		file_num;							/* Number of Files			*/

	struct spec_info	inspec_hdr;				/* Header Information		*/
	struct spec_info	outspec_hdr;			/* Header Information		*/
	double	*in_spec;							/* Input Spectrum Data		*/
	double	*out_spec;							/* Output Spectrum Data		*/
	double	spec_max;							/* Spectral Max				*/
	int		file_index;							/* Index for Spectral File	*/

	/*-------- LOAD INPUT FILE --------*/
	info_spec(argv[ARG_INNAME], &inspec_hdr);
	in_spec = (double *)malloc( inspec_hdr.num_freq* sizeof(double) );	/* Input Spectrum	*/
	load_spec(argv[ARG_INNAME], &inspec_hdr, in_spec, &spec_max);
	in_spec += atoi(argv[ARG_STARTCH]);
	printf("IN: Freq. Num  : %d\n", inspec_hdr.num_freq);
	printf("IN: Freq. Incr : %lf\n", inspec_hdr.freq_incr);

	/*-------- TRIM SPECTRAL DATA --------*/
	outspec_hdr.num_freq = atoi(argv[ARG_STOPCH]) - atoi(argv[ARG_STARTCH]) + 1;
	out_spec = (double *)malloc( outspec_hdr.num_freq* sizeof(double) );	/* Output Storage	*/
	memcpy(out_spec, in_spec, (outspec_hdr.num_freq* sizeof(double)) );
	in_spec -= atoi(argv[ARG_STARTCH]); free(in_spec);

	/*-------- HEADER INFORMATION --------*/
	strcpy( outspec_hdr.obs_name, inspec_hdr.obs_name);
	strcpy( outspec_hdr.stn_name, inspec_hdr.stn_name);
	strcpy( outspec_hdr.src_name, inspec_hdr.src_name);
	outspec_hdr.type		= inspec_hdr.type;			/* File Type (MRG)	*/
	outspec_hdr.start_mjd	= inspec_hdr.start_mjd;		/* Time Range 		*/
	outspec_hdr.stop_mjd	= inspec_hdr.stop_mjd;		/* Time Range 		*/
	outspec_hdr.integ_sec	= inspec_hdr.integ_sec;		/* Time Range 		*/
	outspec_hdr.sefd		= inspec_hdr.sefd;			/* Time Range 		*/
	outspec_hdr.ss_id		= 0;						/* Whole SS 		*/
	outspec_hdr.freq_incr	= inspec_hdr.freq_incr;		/* Ref. Freq. 		*/
	outspec_hdr.ref_freq	= inspec_hdr.ref_freq;		/* Ref. Freq. 		*/
	outspec_hdr.ref_freq	+= (atoi(argv[ARG_STARTCH])* inspec_hdr.freq_incr);	/* Ref. Freq. 		*/

	printf("Freq. Num  : %d\n", outspec_hdr.num_freq);
	printf("Freq. Incr : %lf\n", outspec_hdr.freq_incr);
	printf("Freq. Range: %lf - %lfMHz\n",
				outspec_hdr.ref_freq,
				outspec_hdr.ref_freq + outspec_hdr.num_freq * outspec_hdr.freq_incr);

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
		outspec_hdr.ss_id,					/* SS ID	*/
		&(outspec_hdr.num_freq),
		&(outspec_hdr.ref_freq),
		&(outspec_hdr.freq_incr),
		&out_spec);

	free(out_spec);

	return(0);
}
