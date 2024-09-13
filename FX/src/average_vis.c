/*************************************************************
**	AVERAGE_VIS.C	: Test Module to Read CODA File System	**
**															**
**	AUTHOR	: KAMENO Seiji									**
**	CREATED	: 1996/6/27										**
*************************************************************/
#include <stdio.h>
#include <math.h>
#define	SECDAY		86400
#define	RADDEG		57.29577951308232087721
#define	CORRDATA	4
#define	CORRFLAG	5
#define	AMP			0
#define	PHS			1

int	average_vis(
		vis_ptr, err_ptr,
		bp_r_ptr, bp_i_ptr, power,
		cor_id, ss_id,
		bldelay, blrate, blsefd,
		rf, ref_freq, freq_incr, freq_num,
		start_mjd, integ, time_incr, time_num_cfs,
		position, first_mjd_ptr, uvw_ptr,
		work_ptr)

	double	bldelay;				/* Baseline-Based Delay [microsec]	*/
	double	blrate;					/* Baseline-Based Rate [microsec/sec]*/
	double	blsefd;					/* Baseline-Based SEFD [Jy]			*/
	double	rf;						/* Frequency at the Band Edge [MHz]	*/
	double	ref_freq;				/* Frequency at the Band Edge [MHz]	*/
	double	freq_incr;				/* Frequency Increment [MHz]		*/
	double	start_mjd;				/* Integ Start MJD					*/
	double	integ;					/* Integ Time [sec]					*/
	double	time_incr;				/* Time Increment [sec]				*/
	double	power;					/* Total Power						*/
	int		cor_id;					/* Corr Pair ID in CFS				*/
	int		ss_id;					/* SS ID in CFS						*/
	int		freq_num;				/* Number of Freq. Channels			*/
	int		time_num_cfs;			/* Number of Time					*/
	double	*bp_r_ptr;				/* Pointer of Baseline-Based BP		*/
	double	*bp_i_ptr;				/* Pointer of Baseline-Based BP		*/
	double	*first_mjd_ptr;			/* Start PP MJD						*/
	double	*uvw_ptr;				/* Pointer of (u, v, w) coord.		*/
	float	*vis_ptr;				/* Averaged Visibility (Amp, Phase)	*/
	float	*err_ptr;				/* Averaged Visibility (Amp, Phase)	*/
	int		*position;				/* Start PP Position				*/
	float	*work_ptr;				/* Work Area to Read CFS Data		*/
{

	/*-------- IDENTIFIER --------*/
	int		corunit;				/* Logical Unit Number for CORR data */
	int		flgunit;				/* Logical Unit Number for FLAG data */
	int		first_data_flag;		/* -1:Not Yet, 0:1st Data, 1:2nd or later */
	int		current_obj;			/* Current Object ID				*/
	int		ret;					/* Return Code from CFS Library		*/
	int		isw;					/* Control Code in SSL2				*/
	int		icon;					/* Condition Code					*/
	int		start_ch;				/* Integration Start CH				*/
	int		end_ch;					/* Integration End CH				*/

	/*-------- INDEX --------*/
	int		freq_index;				/* Index for Freq. CH in CFS		*/
	int		bunch_index;			/* Index for Freq. Bunching			*/
	int		stn_index;				/* Index for Station				*/

	/*-------- TOTAL NUMBER --------*/
	int		valid_pp;				/* How Many DATA Was Valid			*/

	/*-------- VARIABLES for VANVLECK --------*/
	int		vanvnode_num;			/* Number of Nodes					*/
	double	*spline_node;			/* Pointer of Nodes					*/
	double	*spline_fact;			/* Pointer of Spline Factors		*/

	/*-------- GENERAL VARIABLE --------*/
	double	mjd_data;				/* Current MJD in CFS Data			*/
	double	mjd_flag;				/* Current MJD in CFS Flag			*/
	double	uvw[3];					/* (u, v, w) Value					*/
	double	valid_time;				/* Valid Integrated Time [sec]		*/	
	double	cs, sn;					/* cos(phase), sin(phase)			*/
	double	phase;					/* Phase to be corrected			*/
	double	omega;					/* Angular Frequency				*/
	double	d_omega;				/* Angular Frequency in the Band	*/
	double	delay[2];				/* Delay for Each Station			*/
	double	rate[2];				/* Rate for Each Station			*/
	double	time_ip;				/* Time from Start of the Day [sec]	*/
	double	time_pp;				/* Time from Epoch [sec]			*/
	double	vw[4];					/* Working Area for SSL2			*/
	double	*tmp_vr, *tmp_vi;		/* Tempolary Visibility				*/
	double	vis_r, vis_i;			/* Averaged Visibility				*/
	double	vis_sigma;				/* Standard Diviation of Visibility	*/
	double	bp_power;				/* Total Power of Bandpass			*/
	double	tmp_delay;
	double	pcalphs;
	char	omode[2];				/* CFS Access Mode */
	char	fname[32];				/* CFS File Name */
	unsigned char	flag[1024];

#ifdef HIDOI
	printf(" BL ID = %d \n", cor_id );
	printf(" SS ID = %d \n", ss_id );
	printf(" DELAY = %e \n", bldelay );
	printf(" RATE  = %e \n", blrate );
	printf(" SEFD  = %lf \n", blsefd );
	printf(" POWER = %lf \n", power );
	printf(" RF    = %lf \n", rf );
	printf(" REF   = %lf \n", ref_freq );
	printf(" F_INCR= %lf \n", freq_incr );
	printf(" F_NUM = %d \n", freq_num );
	printf(" MJD   = %lf\n", start_mjd );
	printf(" INTEG = %lf\n", integ );
	printf(" T_INCR= %lf\n", time_incr );
	printf(" T_NUM = %d\n", time_num_cfs );
	printf(" POSI  = %d\n", *position );
#endif


	vanvleck2_init( &vanvnode_num, &spline_node, &spline_fact );
/*
------------------------------- INITIALIZE MEMORY AREA FOR VISIBILITIES
*/
	first_data_flag = -1;
	tmp_vr = (double *)malloc( freq_num* sizeof(double) );
	tmp_vi = (double *)malloc( freq_num* sizeof(double) );
	memset( tmp_vi , 0, freq_num* sizeof(double) );
	memset( tmp_vr , 0, freq_num* sizeof(double) );
/*
------------------------------- INITIALIZE MEMORY AREA FOR VISIBILITIES
*/
	corunit	= CORRDATA;		flgunit = CORRFLAG;
	sprintf(omode, "r"); 

	/*-------- OPEN CORR DATA --------*/
	sprintf(fname, "CORR.%d/SS.%d/DATA.1\0", cor_id, ss_id ); 
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&corunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- OPEN FLAG DATA --------*/
	sprintf(fname, "CORR.%d/SS.%d/FLAG.1\0", cor_id, ss_id ); 
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&flgunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- SKIP TO THE START MJD --------*/
	skip_coda( corunit, flgunit, position, time_num_cfs, start_mjd,
				freq_num, time_incr );

	/*-------- READ VISIBILITY AND FLAG DATA --------*/
	valid_pp = 0;	valid_time = 0.0;
	mjd_data = 0.0;

	while( valid_time < integ ){

		/*-------- LOAD VISIBILITY TO WORK AREA --------*/
		cfs235_( &corunit, &mjd_data, work_ptr, &freq_num, &ret );
		cfs_ret( 235, ret );

		cfs225_( &flgunit, &mjd_flag, &current_obj, uvw,
					flag, &freq_num, &ret ); cfs_ret( 225, ret );

		if(valid_time < integ* 0.5){
			uvw_ptr[0] = uvw[0];
			uvw_ptr[1] = uvw[1];
			uvw_ptr[2] = uvw[2];
		}

		if( SECDAY* fabs(mjd_data - mjd_flag) > time_incr ){
			printf(" DATA Error... MJD in DATA and FLAG are Different !\n"); 
			printf(" DATA MJD = %lf,  FLAG MJD = %lf!\n", mjd_data, mjd_flag); 
			return(0);
		}

		if(first_data_flag == 0){ first_data_flag = 1;}
		if(first_data_flag == -1){
			first_data_flag = 0;
			*first_mjd_ptr	= mjd_data;
		}

		time_pp = SECDAY*(mjd_data - (*first_mjd_ptr)) - integ/2.0;
		/*-------- INTEGRATE VISIBILITY --------*/
		for(freq_index=0; freq_index<freq_num; freq_index++){
			omega	= 2.0* M_PI* ( rf + freq_incr* (double)freq_index );
			d_omega	= omega - (2.0* M_PI* ref_freq);
			phase = d_omega* bldelay + omega* blrate* time_pp;

			cs	= cos(phase);	sn = sin(phase);

			tmp_vr[freq_index] += ( cs * work_ptr[2* freq_index]
								+   sn * work_ptr[2* freq_index + 1]);
			tmp_vi[freq_index] += (-sn * work_ptr[2* freq_index]
								+   cs * work_ptr[2* freq_index + 1]);
		}
		valid_pp ++;
		valid_time += time_incr;
	}
	/*-------- FOUND NO TARGET SOURCE --------*/
	if( valid_pp == 0){
		printf(" CAUTION : TARGET SOURCE WAS NOT FOUND...\n");
		return(0);
	}

	/*-------- BANDPASS and VANVLECK CORRECTION --------*/
	for(freq_index=0; freq_index<freq_num; freq_index++){

		cs = tmp_vr[freq_index] / (power* valid_pp);
		sn = tmp_vi[freq_index] / (power* valid_pp);

		bp_power = bp_r_ptr[freq_index]*bp_r_ptr[freq_index]
				 + bp_i_ptr[freq_index]*bp_i_ptr[freq_index];

		tmp_vr[freq_index]  = ( cs* bp_r_ptr[freq_index]
							+   sn* bp_i_ptr[freq_index] ) / bp_power;
		tmp_vi[freq_index]  = ( sn* bp_r_ptr[freq_index]
							-   cs* bp_i_ptr[freq_index] ) / bp_power;
	}

	vanvleck2(freq_num, vanvnode_num, spline_node, spline_fact,
				1.0, tmp_vr, tmp_vi);

	if( freq_num > 4 ){
		start_ch= (int)((double)freq_num * 0.05 );
		end_ch	= (int)((double)freq_num * 0.95 );
	} else {
		start_ch= 0;
		end_ch	= freq_num;
	}
	vis_r = 0.0;	vis_i = 0.0;
	/*-------- FREQUENCY CHANNEL BANCHING --------*/
	for(freq_index=start_ch; freq_index< end_ch; freq_index++){
		vis_r += tmp_vr[freq_index];
		vis_i += tmp_vi[freq_index];
	}
	free(tmp_vr);
	free(tmp_vi);

#ifdef HIDOI
	printf("BL SEFD    = %lf\n", blsefd);
	printf("CORR COEFF = %lf\n", (float)( sqrt(vis_r* vis_r + vis_i* vis_i)
                 / (double)(end_ch - start_ch)) );
#endif

	vis_ptr[AMP] = (float)( blsefd* sqrt(vis_r* vis_r + vis_i* vis_i)
				 / (double)(end_ch - start_ch) );
	vis_ptr[PHS] = (float)atan2(vis_i, vis_r);

	vis_sigma = blsefd /
			sqrt(2.0e6* freq_incr* time_incr* ((end_ch - start_ch) * valid_pp));

	err_ptr[AMP] = (float)vis_sigma;
	err_ptr[PHS] = (float)(vis_sigma / vis_ptr[AMP]);


	/*-------- CLOSE SS-HEADER --------*/
	cfs104_( &corunit, &ret );	cfs_ret( 104, ret );
	cfs104_( &flgunit, &ret );	cfs_ret( 104, ret );

	return(valid_pp);
}
