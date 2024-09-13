/*********************************************************
**	LOAD_VIS_ANIM.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>
#define	CORRDATA	4
#define	CORRFLAG	5
#define	SECDAY		86400
#define	RADDEG		57.29577951308232087721

int	load_vis_anim( bl_index, bl_direction, obj_id, ss_index, position,
			freq_num_ptr, bunch_num, freq_incr_ptr, freq_center, time_num_cfs,
			time_num_ptr, integ_num, time_incr, time_data_ptr, mjd_init,
			node_num, time_node_ptr, delay_coeff_ptr, rate_coeff_ptr,
			bp1_ptr, bp2_ptr,
			work_ptr, vis_r_ptr, vis_i_ptr )

	int		bl_index;				/* Baseline Index 					*/
	float	bl_direction;			/* Baseline Direction				*/
	int		obj_id;					/* Object ID Number 				*/
	int		ss_index;				/* Sub-Stream Index 				*/
	int		*position;				/* Search Position					*/
	int		*freq_num_ptr;			/* Number of Freq. Channels			*/
	int		bunch_num;				/* Number of Bunching				*/
	double	*freq_incr_ptr;			/* Frequency Increment of SS [MHz]	*/
	double	freq_center;			/* Frequency Center [MHz]			*/
	int		*time_num_ptr;			/* Number of Time					*/
	int		time_num_cfs;			/* Number of Time in CFS			*/
	int		integ_num;				/* Integration PP Number			*/
	double	time_incr;				/* Time Increment [sec]				*/
	double	*time_data_ptr;			/* Time Data [sec]					*/
	double	mjd_init;				/* MJD at Start of Day				*/
	int		*node_num;				/* Number of node for station		*/
	int		*time_node_ptr;			/* Pointer of Time Node for Station	*/
	int		*delay_coeff_ptr;		/* Pointer of Delay SPLINE Coeff.	*/
	int		*rate_coeff_ptr;		/* Pointer of Rate SPLINE Coeff.	*/
	double	*bp1_ptr;				/* Pointer of Bandpass for STN 1	*/
	double	*bp2_ptr;				/* Pointer of Bandpass for STN 2	*/
	float	*work_ptr;				/* Work Area of Visibility Data		*/
	float	*vis_r_ptr;				/* Pointer of Visibility (real) Data */
	float	*vis_i_ptr;				/* Pointer of Visibility (imag) Data */
{
	int		corunit;				/* Logical Unit Number for CORR data */
	int		flgunit;				/* Logical Unit Number for FLAG data */
	int		spec_index;
	int		freq_index;
	int		bunch_index;
	int		time_index;				/* Index in Time Integration		*/
	int		skip_pp;				/* SKIP NUMBER */
	int		node_index;				/* Index of Node Points				*/
	int		ch_num;					/* Freq. Channel Number after BUNCH	*/
	int		origin;					/* SKIP NUMBER */
	int		loop_counter;
	int		ret;					/* Return Code from CFS Library */
	int		first_data_flag;		/* -1:Not Yet, 0:1st Data, 1:2nd or later */
	char	omode[2];				/* CFS Access Mode */
	char	fname[32];				/* CFS File Name */
	double	init_phase;				/* Initial Smoothed Phase			*/
	double	current_delay;			/* Current Smoothed Delay			*/
	double	current_phase;			/* Current Smoothed Phase			*/
	double	ref_delay;				/* Reference Smoothed Delay			*/
	double	ref_phase;				/* Reference Smoothed Phase			*/
	double	current_time;			/* Current Time [sec from SOD]		*/
	double	freq_incr;				/* Frequency Increment [MHz]		*/
	double	mjd_data;
	double	mjd_flag;
	double	sum_mjd;
	double	uvw[3];
	unsigned char	flag[1024];
	int		current_obj;
	int		integ_pp;				/* How Many Data were integrated	*/
	int		valid_pp;				/* How Many DATA Were Valid			*/
	double	bp[1024];
	double	rel_freq;
	double	phase;
	double	cs, sn;
	double	vr, vi;
	double	visr_sum, visi_sum;

	/*-------- SSL2 VARIABLE --------*/
	int		spline_dim;				/* Dimension of SPLINE				*/
	int		icon;					/* Condition Code					*/
	int		isw;					/* Control Code						*/
	double	vw[4];					/* Work Space						*/

	int		hour, min, sec;
/*
------------------------------- INITIALIZE MEMORY AREA FOR VISIBILITIES
*/
	freq_incr = *freq_incr_ptr;
	first_data_flag = -1;
	ch_num = (*freq_num_ptr) / bunch_num;
	memset( vis_r_ptr, 0, ch_num* (*time_num_ptr)*sizeof(float) );
	memset( vis_i_ptr, 0, ch_num* (*time_num_ptr)*sizeof(float) );
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

	/*-------- OPEN FLAG DATA --------*/
	sprintf(fname, "CORR.%d/SS.%d/FLAG.1\0", bl_index, ss_index ); 
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&flgunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- SKIP TO THE START MJD --------*/
	skip_coda( corunit, flgunit, position, time_num_cfs, 0.0,
				*freq_num_ptr, time_incr );

	/*-------- READ VISIBILITY AND FLAG DATA --------*/
	valid_pp = 0;
	mjd_data = 0.0;
	spline_dim	= 3;
	node_index	= 0;

#ifdef SUZUYAMA
	printf("--TIME--  --DELAY[microsec]-- ---PHS[DEG]---\n");
#endif

	while( valid_pp < *time_num_ptr ){

		sum_mjd = 0.0;
		integ_pp= 0;
		for(time_index=0; time_index < integ_num; time_index++){

			if( valid_pp >= *time_num_ptr ){	break;	}
			/*-------- LOAD VISIBILITY TO WORK AREA --------*/
			cfs235_( &corunit, &mjd_data, work_ptr, freq_num_ptr, &ret );
			cfs225_( &flgunit, &mjd_flag, &current_obj, uvw,
						flag, freq_num_ptr, &ret );

			if( SECDAY* fabs(mjd_data - mjd_flag) > time_incr ){
				printf("DATA Error...MJD in DATA and FLAG are Different !\n");
				printf("DATA MJD=%lf, FLAG MJD = %lf!\n", mjd_data, mjd_flag);
				return(0);
			}
			sum_mjd += mjd_data;

			/*-------- Smoothed Delay and Phase --------*/
			current_time = SECDAY* (mjd_data - (int)mjd_init);
			isw = 0;
			dbsf1_( &spline_dim, time_node_ptr[1], &node_num[1],
				delay_coeff_ptr[1], &isw, &current_time, &node_index,
				&current_delay, vw, &icon);

			isw = -1;
			dbsf1_( &spline_dim, time_node_ptr[1], &node_num[1],
				rate_coeff_ptr[1], &isw, &current_time, &node_index,
				&current_phase, vw, &icon);

			if(node_num[0] > 0){
				isw = 0;
				dbsf1_( &spline_dim, time_node_ptr[0], &node_num[0],
					delay_coeff_ptr[0], &isw, &current_time, &node_index,
					&ref_delay, vw, &icon);

				isw = -1;
				dbsf1_( &spline_dim, time_node_ptr[0], &node_num[0],
					rate_coeff_ptr[0], &isw, &current_time, &node_index,
					&ref_phase, vw, &icon);
			} else {
				ref_delay = 0.0;
				ref_phase = 0.0;
			}
			current_delay -= ref_delay;
			current_phase -= ref_phase;
			current_phase *= freq_center;
			current_phase -= (int)current_phase;
			current_phase *= (2.0* M_PI);

#ifdef SUZUYAMA
			hour = (int)(current_time / 3600);
			min  = ((int)current_time % 3600) / 60;
			sec  =  (int)current_time % 60;
			printf("%02d:%02d:%02d  %16.10e  %9.4lf\n",
				hour, min, sec, current_delay, current_phase* RADDEG );
#endif

 

			/*-------- Store Visibility into Memory Area --------*/
			for(spec_index=0; spec_index< ch_num; spec_index++){
				for(bunch_index=0; bunch_index<bunch_num; bunch_index++){

					/*-------- Phase Correction --------*/
					freq_index = spec_index* bunch_num + bunch_index;
					rel_freq = freq_incr* (freq_index - (*freq_num_ptr) / 2);
					phase = 2.0* M_PI* rel_freq* current_delay;
					phase += current_phase;

					cs = cos(phase);	sn = sin(phase);

#ifdef MIYOPPI
/*------------ SPECIAL TREATMENT by MIYOSHI */
				phase = 0.0;
				if( freq_index == 112 ){
					cs = 1.0;	sn = 0.0;
				} else {
					cs = 0.0; sn = 0.0;
				}
/*------------ SPECIAL TREATMENT by MIYOSHI */
#endif

					vr = work_ptr[2* freq_index];
					vi = work_ptr[2* freq_index + 1];
					vi *= bl_direction;

					/*-------- Bandpass Correction --------*/
					*vis_r_ptr += (vr* cs + vi* sn) / bp[freq_index];
					*vis_i_ptr += (vi* cs - vr* sn) / bp[freq_index];
				}
				vis_r_ptr ++;
				vis_i_ptr ++;
			}
			integ_pp ++;
			valid_pp++;
			vis_r_ptr -= ch_num;
			vis_i_ptr -= ch_num;
		}

		/*-------- Normalization along frequency and time --------*/
		for(spec_index=0; spec_index< ch_num; spec_index++){
			*vis_r_ptr /= (float)(bunch_num* integ_pp);	vis_r_ptr ++;
			*vis_i_ptr /= (float)(bunch_num* integ_pp);	vis_i_ptr ++;
		}
		*time_data_ptr = sum_mjd / (double)integ_pp;	time_data_ptr ++;
	}
	*time_num_ptr = (valid_pp - 1) / integ_num + 1;
	*freq_incr_ptr *= bunch_num;

	/*-------- FOUND NO TARGET SOURCE --------*/
	if( valid_pp == 0){
		printf(" CAUTION : TARGET SOURCE [ID=%d] WAS NOT FOUND...\n", obj_id );
		return(0);
	}

	/*-------- CLOSE SS-HEADER --------*/
	cfs104_( &corunit, &ret );	cfs_ret( 104, ret );
	cfs104_( &flgunit, &ret );	cfs_ret( 104, ret );

	*freq_num_ptr /= bunch_num;
	return(valid_pp);
}
