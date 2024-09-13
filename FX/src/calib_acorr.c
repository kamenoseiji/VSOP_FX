/*********************************************************
**	show_acorr : Integrate Station-Based AutoCorr		**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include "aoslog.inc"
#include <math.h>
#define	ARG_ON_FNAME	1
#define	ARG_OFF_FNAME	2
#define	MAX_SPEC		1024
#define	FILE_NUM		3
#define	FILE_ON			0
#define	FILE_OFF		1
#define	FILE_CAL		2
#define	MAX(a,b)	( (a>b)?a:b )


MAIN__(
	int		argc,			/* Number of Arguments 		*/
	char	**argv)			/* Pointer to Arguments 	*/
{
	struct spec_info	spec_hdr[FILE_NUM];	/* Header Information		*/
	int		spec_index;						/* Index for Spectral Point	*/
	double	*spec_ptr[FILE_NUM];			/* Spectrum Data			*/
	double	spec_max[FILE_NUM];				/* Spectral Max				*/
	int		file_index;						/* Index for Files			*/

	/*-------- MEMORY ALLOCATION --------*/
	for(file_index=0; file_index< FILE_NUM; file_index ++){
		spec_ptr[file_index] = (double *)malloc(MAX_SPEC* sizeof(double) ); }

	/*-------- LOAD SPECTRAL DATA --------*/
	if(load_spec(argv[ARG_ON_FNAME], &spec_hdr[FILE_ON], spec_ptr[FILE_ON], &spec_max[FILE_ON]) <0){
		exit(0); }
	if(load_spec(argv[ARG_OFF_FNAME], &spec_hdr[FILE_OFF], spec_ptr[FILE_OFF], &spec_max[FILE_OFF]) <0){
		exit(0); }

	/*-------- CALIB SPECTRAL DATA --------*/
	memcpy( spec_ptr[FILE_CAL], spec_ptr[FILE_ON], MAX_SPEC* sizeof(double) ); 
	for(spec_index=0; spec_index< spec_hdr[FILE_ON].num_freq; spec_index ++){
		*spec_ptr[FILE_CAL]	-= *spec_ptr[FILE_OFF];
		*spec_ptr[FILE_CAL]	/= *spec_ptr[FILE_OFF];
		*spec_ptr[FILE_CAL] *= spec_hdr[FILE_ON].sefd;
		spec_ptr[FILE_CAL] ++; spec_ptr[FILE_OFF] ++;
	}
	spec_ptr[FILE_CAL] -= spec_hdr[FILE_ON].num_freq;
	spec_ptr[FILE_OFF] -= spec_hdr[FILE_ON].num_freq;

	/*-------- HEADER INFO --------*/
	spec_hdr[FILE_CAL] = spec_hdr[FILE_ON];
	spec_hdr[FILE_CAL].integ_sec = sqrt(
		spec_hdr[FILE_ON].integ_sec * spec_hdr[FILE_OFF].integ_sec);

	printf("SPEC N CH = %d  INTEG = %lf\n", spec_hdr[FILE_CAL].num_freq, spec_hdr[FILE_CAL].integ_sec);

	/*-------- SAVE DATA --------*/
	dump_spec(
		NULL,
		spec_hdr[FILE_CAL].obs_name,
		spec_hdr[FILE_CAL].stn_name,
		spec_hdr[FILE_CAL].src_name,
		spec_hdr[FILE_CAL].start_mjd,
		spec_hdr[FILE_CAL].stop_mjd,
		CAL,							/* FILE Descriptor	*/
		spec_hdr[FILE_CAL].sefd,		/* SEFD [calibrated]*/
		&spec_hdr[FILE_CAL].integ_sec,	/* INTEG	*/
		1,								/* SSNUM	*/
		spec_hdr[FILE_CAL].ss_id,		/* SS ID	*/
		&(spec_hdr[FILE_CAL].num_freq),
		&spec_hdr[FILE_CAL].ref_freq,
		&spec_hdr[FILE_CAL].freq_incr,
		&spec_ptr[FILE_CAL]);

	for(file_index=0; file_index< FILE_NUM; file_index ++){
		free(spec_ptr[file_index]); }

	return(0);
}
