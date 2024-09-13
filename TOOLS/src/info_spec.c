/*********************************************************
**	info_spec.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include "aoslog.inc"
#define MAX(a,b)    ( (a>b)?a:b )

int	info_spec(
	char	*fname,					/* Spectrum File Name			*/
	struct spec_info	*spec_hdr)	/* Header Information			*/
{
	FILE	*file_ptr;				/* File Pointer					*/
	int		spec_index;				/* Index for Spectrum			*/

	file_ptr = fopen( fname, "r" );
	if( file_ptr == NULL ){
		printf("Can't Open Spectrum File [%s]\n", fname);
		return(-1);
	}

	fread( spec_hdr, 1, sizeof(struct spec_info), file_ptr);
	fclose(file_ptr);
	return(spec_hdr->num_freq);
}
