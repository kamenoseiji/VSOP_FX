/*********************************************************
**	dump_spec.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include "aoslog.inc"
#include <math.h>

int	dump_spec(
	char	*out_fname,				/* Output File Name				*/
	char	*obs_name,				/* Observation Name 			*/
	char	*stn_name,				/* Station Name 				*/
	char	*obj_name,				/* Object Name 					*/
	double	start_mjd,				/* Integ Start [MJD ] 			*/
	double	stop_mjd,				/* Integ Stop [MJD ] 			*/
	int		scan_type,				/* Scan Type					*/
	double	sefd,					/* SEFD [Jy] 					*/
	double	*integ_time,			/* Integ time [sec] 			*/
	int		ssnum,					/* Number of Sub-Stream 		*/
	int		ss_id,					/* SS ID						*/
	int		*freq_num_ptr,			/* Number of Freq. Channels 	*/
	double	*rf_ptr,				/* RF Frequency [MHz] 			*/
	double	*freq_incr_ptr,			/* Frequency Increment 			*/
	double	**vis_r_ptr)			/* Pointer of Visibility Data 	*/
{
	static char    ant_stat[][8] = {
			"ZERO", "R", "SKY", "ON", "OFF", "DUMM", "DUMM", "DUMM",
			"ON_OFF", "CAL", "MRG1", "MRG2", "MRG3", "MRG4", "MRG5", "MRG6",
			};
	FILE	*file_ptr;				/* File Pointer to Dump			*/
	char	fname[64];				/* File Name to Dump			*/
	struct	spec_info	spec_hdr;	/* Headder Information 			*/
	int		ss_index;

	for(ss_index=0; ss_index<ssnum; ss_index++){
		if(freq_num_ptr[ss_index] != 0){

			if( ss_id < 0){	spec_hdr.ss_id = ss_index;	
			} else {		spec_hdr.ss_id = ss_id; 	}

			if( out_fname == NULL ){
				sprintf(fname, "%s.%s.%s.SS%02d.%s.%12.6lf",
					obs_name, stn_name, obj_name, spec_hdr.ss_id,
					ant_stat[scan_type], start_mjd);
			} else {
				sprintf(fname, "%s", out_fname);
			}
			printf("OUTPUT FILE = %s\n", fname);
			file_ptr = fopen( fname, "w" );

			strcpy(spec_hdr.obs_name, obs_name);
			strcpy(spec_hdr.stn_name, stn_name);
			strcpy(spec_hdr.src_name, obj_name);
			spec_hdr.type = scan_type;
			spec_hdr.start_mjd = start_mjd; 
			spec_hdr.stop_mjd = stop_mjd; 
			spec_hdr.integ_sec = integ_time[ss_index]; 
			spec_hdr.sefd = sefd; 
			spec_hdr.num_freq = freq_num_ptr[ss_index]; 
			spec_hdr.ref_freq = rf_ptr[ss_index]; 
			spec_hdr.freq_incr = freq_incr_ptr[ss_index]; 

			fwrite(&spec_hdr, 1, sizeof(struct spec_info), file_ptr);
			fwrite(vis_r_ptr[ss_index], 1, freq_num_ptr[ss_index] * sizeof(double), file_ptr);

			fclose(file_ptr);
		}
	}
	return(0);
}
