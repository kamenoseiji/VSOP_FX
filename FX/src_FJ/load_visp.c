/*****************************************************************
**	LOAD_VISP.C	: Load Visibility Phase with P-Cal Correction	**
**																**
**	AUTHOR	: KAMENO Seiji										**
**	CREATED	: 1996/6/27											**
*****************************************************************/
#include <stdio.h>
#include <math.h>
#define	CORRDATA	4
#define	CORRFLAG	5

int	load_visp( bl_index, bl_direction, obj_id, ss_index, position,
			freq_num_ptr, bunch_num, time_num_cfs, time_num, time_incr,
			obs_start_mjd, start_mjd, stop_mjd,
			bp1_ptr, bp2_ptr,
			real_coeff_ptr1, imag_coeff_ptr1, time_node_ptr1, node_num1,
			real_coeff_ptr2, imag_coeff_ptr2, time_node_ptr2, node_num2,
			work_ptr, vis_r_ptr, vis_i_ptr )

	int		bl_index;				/* Baseline Index 					*/
	float	bl_direction;			/* Baseline Direction				*/
	int		obj_id;					/* Object ID Number 				*/
	int		ss_index;				/* Sub-Stream Index 				*/
	int		*position;				/* Search Position					*/
	int		*freq_num_ptr;			/* Number of Freq. Channels			*/
	int		bunch_num;				/* Number of Bunching				*/
	int		time_num;				/* Number of Time					*/
	int		time_num_cfs;			/* Number of Time in CFS			*/
	double	time_incr;				/* Time Increment [sec]				*/
	double	obs_start_mjd;			/* OBS START TIME [MJD]				*/
	double	start_mjd;				/* INTEG START TIME [MJD]			*/
	double	stop_mjd;				/* INTEG STOP TIME [MJD]			*/
	double	*bp1_ptr;				/* Pointer of Bandpass for STN 1	*/
	double	*bp2_ptr;				/* Pointer of Bandpass for STN 2	*/
	double	*real_coeff_ptr1;		/* Pointer of P-Cal Spline Coeff	*/ 
	double	*imag_coeff_ptr1;		/* Pointer of P-Cal Spline Coeff	*/ 
	double	*time_node_ptr1;		/* Pointer of Time Node in Spline	*/
	int		node_num1;				/* Number of Node					*/
	double	*real_coeff_ptr2;		/* Pointer of P-Cal Spline Coeff	*/ 
	double	*imag_coeff_ptr2;		/* Pointer of P-Cal Spline Coeff	*/ 
	double	*time_node_ptr2;		/* Pointer of Time Node in Spline	*/
	int		node_num2;				/* Number of Node					*/
	float	*work_ptr;				/* Work Area of Visibility Data		*/
	float	*vis_r_ptr;				/* Pointer of Visibility (real) Data */
	float	*vis_i_ptr;				/* Pointer of Visibility (imag) Data */
{
	int		corunit;				/* Logical Unit Number for CORR data */
	int		flgunit;				/* Logical Unit Number for FLAG data */
	int		freq_index;
	int		bunch_index;
	int		skip_pp;				/* SKIP NUMBER */
	int		origin;					/* SKIP NUMBER */
	int		loop_counter;
	int		ret;					/* Return Code from CFS Library */
	int		first_data_flag;		/* -1:Not Yet, 0:1st Data, 1:2nd or later */
	char	omode[2];				/* CFS Access Mode */
	char	fname[32];				/* CFS File Name */
	double	first_mjd;
	double	mjd_data;
	double	mjd_flag;
	double	uvw[3];
	double	vis_r;					/* Temporary Visibility (real)		*/
	double	vis_i;					/* Temporary Visibility (real)		*/
	double	cs, sn;					/* cos(phase), sin(phase)			*/
	double	pcal_phs1;				/* P-Cal Phase of Station 1			*/
	double	pcal_phs2;				/* P-Cal Phase of Station 2			*/
	unsigned char	flag[1024];
	int		current_obj;
	int		valid_pp;				/* How Many DATA Was Valid */
	double	bp[1024];

	int		node_index;


#ifdef DEBUG
	printf("Number of Node : %d, %d\n", node_num1, node_num2);
	for(node_index=0; node_index<node_num1; node_index++){
		printf("STATION 1 : NODE[%3d] = %lf\n",
			node_index, time_node_ptr1[node_index] );
	}
	for(node_index=0; node_index<node_num2; node_index++){
		printf("STATION 2 : NODE[%3d] = %lf\n",
			node_index, time_node_ptr2[node_index] );
	}
#endif

/*
------------------------------- INITIALIZE MEMORY AREA FOR VISIBILITIES
*/
	first_data_flag = -1;
	memset( vis_r_ptr, 0, (*freq_num_ptr)*time_num*sizeof(float)/bunch_num );
	memset( vis_i_ptr, 0, (*freq_num_ptr)*time_num*sizeof(float)/bunch_num );
/*
------------------------------- INITIALIZE MEMORY AREA FOR VISIBILITIES
*/
	for(freq_index=0; freq_index < *freq_num_ptr; freq_index++){
		bp[freq_index] = sqrt( bp1_ptr[freq_index] * bp2_ptr[freq_index] );
	}

	corunit	= CORRDATA;		flgunit = CORRFLAG;
	sprintf(omode, "r"); 

	/*-------- OPEN CORR DATA --------*/
	sprintf(fname, "CORR.%d/SS.%d/DATA.1\0", bl_index, ss_index ); 

	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&corunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- OPEN CORR DATA --------*/
	sprintf(fname, "CORR.%d/SS.%d/FLAG.1\0", bl_index, ss_index ); 

	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&flgunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- SKIP TO THE START MJD --------*/
	skip_coda( corunit, flgunit, position, time_num_cfs, start_mjd,
				*freq_num_ptr, time_incr );

	/*-------- READ VISIBILITY AND FLAG DATA --------*/
	valid_pp = 0;
	mjd_data = 0.0;
	while( (mjd_data <= stop_mjd) && ( valid_pp < time_num)){

		/*-------- LOAD VISIBILITY TO WORK AREA --------*/
		cfs235_( &corunit, &mjd_data, work_ptr, freq_num_ptr, &ret );
		cfs225_( &flgunit, &mjd_flag, &current_obj, uvw,
					flag, freq_num_ptr, &ret );

		if(mjd_data != mjd_flag){
			printf(" DATA Error... MJD in DATA and FLAG are Different !\n"); 
			printf(" DATA MJD = %lf,  FLAG MJD = %lf!\n", mjd_data, mjd_flag); 
			return(0);
		}

		/*-------- IS THIS THE TARGET SOURCE ? --------*/
		if(		(current_obj ==  obj_id)
			&&	(mjd_data >= start_mjd)
			&&	(mjd_data <= stop_mjd)	){


			if(first_data_flag == 0){ first_data_flag = 1;}
			if(first_data_flag == -1){
				first_data_flag = 0;
				first_mjd	= mjd_data;
			}
			/*-------- GET P-CAL PHASE --------*/
			get_pcalphase( real_coeff_ptr1, imag_coeff_ptr1,
				time_node_ptr1, node_num1,
				(double)(mjd_data - (int)obs_start_mjd),
				&pcal_phs1);

			get_pcalphase( real_coeff_ptr2, imag_coeff_ptr2,
				time_node_ptr2, node_num2,
				(double)(mjd_data - (int)obs_start_mjd),
				&pcal_phs2);

#ifdef DEBUG
			printf("MJD = %lf:  P-CAL PHASE = %lf, %lf\n",
				(double)(mjd_data - (int)start_mjd), pcal_phs1, pcal_phs2);
#endif

			cs	= cos( pcal_phs2 - pcal_phs1 )/ bunch_num;
			sn	= sin( pcal_phs2 - pcal_phs1 )/ bunch_num;
			sn	*= (double)bl_direction;

			/*-------- INTEGRATE VISIBILITY --------*/
			for(freq_index=0; freq_index<*freq_num_ptr/bunch_num; freq_index++){
				vis_r = 0.0;	vis_i = 0.0;
				for(bunch_index=0; bunch_index<bunch_num; bunch_index++){
					vis_r += *work_ptr / bp[freq_index*bunch_num + bunch_index];
						work_ptr++;
					vis_i += *work_ptr / bp[freq_index*bunch_num + bunch_index];
						work_ptr++;
				}

				*vis_r_ptr	+= (float)( cs* vis_r + sn* vis_i); vis_r_ptr++;
				*vis_i_ptr	+= (float)(-sn* vis_r + cs* vis_i); vis_i_ptr++;
			}
			work_ptr -= 2*( *freq_num_ptr );
			valid_pp++;
		}
	}

	/*-------- FOUND NO TARGET SOURCE --------*/
	if( valid_pp == 0){
		printf(" CAUTION : TARGET SOURCE [ID=%d] WAS NOT FOUND...\n", obj_id );
		return(0);
	}

	/*-------- CLOSE SS-HEADER --------*/
	cfs104_( &corunit, &ret );	cfs_ret( 104, ret );
	cfs104_( &flgunit, &ret );	cfs_ret( 104, ret );

	*freq_num_ptr /= bunch_num;
	start_mjd	= first_mjd;
	return(valid_pp);
}
