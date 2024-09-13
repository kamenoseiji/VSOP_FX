/*********************************************************
**	dump_spec.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include "aoslog.inc"
#define MAX(a,b)    ( (a>b)?a:b )

int	load_spec(
	char	*fname,					/* Spectrum File Name			*/
	struct spec_info	*spec_hdr,	/* Header Information			*/
	double	*vis_ptr,				/* Pointer of Spectrum Data 	*/
	double	*spec_max)				/* Max of the spectrum			*/
{
	FILE	*file_ptr;				/* File Pointer					*/
	int		spec_index;				/* Index for Spectrum			*/

	file_ptr = fopen( fname, "r" );
	if( file_ptr == NULL ){
		printf("Can't Open Spectrum File [%s]\n", fname);
		return(-1);
	}

	fread( spec_hdr, 1, sizeof(struct spec_info), file_ptr);
	fread( vis_ptr, 1, spec_hdr->num_freq* sizeof(double), file_ptr);
	*spec_max = -1.0e38;
    for(spec_index=(0.2* spec_hdr->num_freq); spec_index < (0.8* spec_hdr->num_freq); spec_index ++){
        *spec_max = MAX( vis_ptr[spec_index], *spec_max);
    }


	fclose(file_ptr);
	return(spec_hdr->num_freq);
}
