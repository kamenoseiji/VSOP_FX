/*********************************************************
**	INTEG_VIS.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>
#define PI2			6.28318530717958647688
#define	SECDAY		86400
#define	RADDEG		57.29577951308232087721
#define	CORRDATA	4
#define	CORRFLAG	5
#define CHOFS 0

int	integ_vis( bl_index, bl_direction, obj_id, ss_index, position,
			rf, freq_incr, freq_num_ptr, bunch_num,
			time_num_cfs,	time_num,	time_incr,
			start_mjd, stop_mjd,
			node_num, time_node_ptr, delay_coeff_ptr, rate_coeff_ptr,
			bp_r_ptr, bp_i_ptr,
			work_ptr, vis_r_ptr, vis_i_ptr )

	int		bl_index;				/* Baseline Index */
	float	bl_direction;			/* Baseline Direction */
	int		obj_id;					/* Object ID Number */
	int		ss_index;				/* Sub-Stream Index */
	int		*position;				/* Start PP Position	*/
	double	rf;						/* Frequency at the Band Edge */
	double	freq_incr;				/* Frequency Increment */
	int		*freq_num_ptr;			/* Number of Freq. Channels */
	int		bunch_num;				/* Number of Bunching */
	int		time_num;				/* Number of Time */
	int		time_num_cfs;			/* Number of Time */
	double	time_incr;				/* Time Increment [sec] */
	double	start_mjd;				/* INTEG START TIME [MJD] */
	double	stop_mjd;				/* INTEG STOP TIME [MJD] */
	int		*node_num;				/* Number of Node for Station		*/
	int		*time_node_ptr;			/* Pointer of Time Node for Station	*/
	int		*delay_coeff_ptr;		/* Pointer of Delay Spline Coeff	*/
	int		*rate_coeff_ptr;		/* Baseline Rate at the Epoch		*/
	double	*bp_r_ptr;				/* Pointer of BP (real) in the SS	*/
	double	*bp_i_ptr;				/* Pointer of BP (imag) in the SS	*/
	float	*work_ptr;				/* Work Area of Visibility Data		*/
	float	*vis_r_ptr;				/* Pointer of Visibility (real) Data */
	float	*vis_i_ptr;				/* Pointer of Visibility (imag) Data */
{

	/*-------- IDENTIFIER --------*/
	int		corunit;				/* Logical Unit Number for CORR data */
	int		flgunit;				/* Logical Unit Number for FLAG data */
	int		first_data_flag;		/* -1:Not Yet, 0:1st Data, 1:2nd or later */
	int		current_obj;			/* Current Object ID				*/
	int		ret;					/* Return Code from CFS Library		*/
	int		isw;					/* Control Code in SSL2				*/
	int		icon;					/* Condition Code					*/

	/*-------- INDEX --------*/
	int		freq_index;				/* Index for Freq. CH in CFS		*/
	int		bunch_index;			/* Index for Freq. Bunching			*/
	int		node_index;				/* Index for Node Point				*/
	int		stn_index;				/* Index for Station				*/

	/*-------- TOTAL NUMBER --------*/
	int		valid_pp;				/* How Many DATA Was Valid			*/
	int		spline_dim;				/* Spline Dimension					*/
	int		freq_num;				/* Number of Frequency in SS		*/

	/*-------- VARIABLES for VANVLECK --------*/
	int		vanvnode_num;			/* Number of Nodes					*/
	double	*spline_node;			/* Pointer of Nodes					*/
	double	*spline_fact;			/* Pointer of Spline Factors		*/

	/*-------- GENERAL VARIABLE --------*/
	double	first_mjd;				/* MJD of the First Valid PP		*/
	double	mjd_data;				/* Current MJD in CFS Data			*/
	double	mjd_flag;				/* Current MJD in CFS Flag			*/
	double	uvw[3];					/* (u, v, w) Value					*/
	double	cs, sn;					/* cos(phase), sin(phase)			*/
	double	phase;					/* Phase to be corrected			*/
	double	omega;					/* Angular Frequency				*/
	double	d_omega;				/* Angular Frequency in the Band	*/
	double	bldelay;				/* Baseline-Based Delay				*/
	double	blrate;					/* Baseline-Based Rate				*/
	double	delay[2];				/* Delay for Each Station			*/
	double	rate[2];				/* Rate for Each Station			*/
	double	time_ip;				/* Time from Start of the Day [sec]	*/
	double	time_pp;				/* Time from Epoch [sec]			*/
	double	vw[4];					/* Working Area for SSL2			*/
	double	*tmp_vr, *tmp_vi;		/* Tempolary Visibility				*/
	double	power;					/* Total Power						*/
	double	bp_power;				/* Total Power of Bandpass			*/
	double	tmp_delay;

	double	amp, phs, ss;
	int		year, doy, hh, mm;

	char	omode[2];				/* CFS Access Mode */
	char	fname[32];				/* CFS File Name */
	unsigned char	flag[1024];

	vanvleck2_init( &vanvnode_num, &spline_node, &spline_fact );
/*
------------------------------- INITIALIZE MEMORY AREA FOR VISIBILITIES
*/
	freq_num = *freq_num_ptr;
	first_data_flag = -1;
	tmp_vr = (double *)malloc( freq_num* sizeof(double) );
	tmp_vi = (double *)malloc( freq_num* sizeof(double) );
	memset( tmp_vi , 0, freq_num* sizeof(double) );
	memset( tmp_vr , 0, freq_num* sizeof(double) );
	memset( vis_r_ptr, 0, freq_num* sizeof(float)/bunch_num );
	memset( vis_i_ptr, 0, freq_num* sizeof(float)/bunch_num );
/*
------------------------------- INITIALIZE MEMORY AREA FOR VISIBILITIES
*/
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
	skip_coda( corunit, flgunit, position, time_num_cfs, start_mjd,
				freq_num, time_incr);

	/*-------- READ VISIBILITY AND FLAG DATA --------*/
	valid_pp = 0;
	mjd_data = 0.0;
	node_index = 0;	spline_dim = 3;
	while( (mjd_data <= stop_mjd) && ( valid_pp < time_num)){


		/*-------- LOAD VISIBILITY TO WORK AREA --------*/
		cfs235_( &corunit, &mjd_data, work_ptr, freq_num_ptr, &ret );
		cfs_ret( 235, ret );

		cfs225_( &flgunit, &mjd_flag, &current_obj, uvw,
					flag, freq_num_ptr, &ret ); cfs_ret( 225, ret );

		if( SECDAY* fabs(mjd_data - mjd_flag) > time_incr ){
			printf(" DATA Error... MJD in DATA and FLAG are Different !\n"); 
			printf(" DATA MJD = %lf,  FLAG MJD = %lf!\n", mjd_data, mjd_flag); 
			return(0);
		}

/*
		amp = sqrt( work_ptr[CHOFS]*work_ptr[CHOFS]
				+	work_ptr[CHOFS+1]*work_ptr[CHOFS+1]);
		phs = RADDEG* atan2( work_ptr[CHOFS+1], work_ptr[CHOFS]);

		fmjd2doy( mjd_data, &year, &doy, &hh, &mm, &ss);
		printf("%02d:%02d:%02d  SS=%d : AMP=%lf, PHS=%lf\n",
			hh, mm, (int)ss, ss_index, amp, phs);
*/


		/*-------- IS THIS THE TARGET SOURCE ? --------*/
		if(		(current_obj ==  obj_id)
			&&	(mjd_data >= start_mjd)
			&&	(mjd_data <= stop_mjd)	){

			if(first_data_flag == 0){ first_data_flag = 1;}
			if(first_data_flag == -1){
				first_data_flag = 0;
				first_mjd	= mjd_data;
			}

			/*-------- INTERPOLATE DELAY and RATE --------*/
			time_pp	= (mjd_data - first_mjd)*SECDAY;
			time_ip	= (mjd_data - (int)first_mjd)*SECDAY;
			isw = 0;
			for( stn_index=0; stn_index<2; stn_index++){

				if(node_num[stn_index] > 1){
					dbsf1_( &spline_dim, time_node_ptr[stn_index],
						&node_num[stn_index], delay_coeff_ptr[stn_index],
						&isw, &time_ip, &node_index, &tmp_delay, vw, &icon);

					delay[stn_index] = tmp_delay;

					dbsf1_( &spline_dim, time_node_ptr[stn_index],
						&node_num[stn_index], rate_coeff_ptr[stn_index],
						&isw, &time_ip, &node_index, &tmp_delay, vw, &icon);

					rate[stn_index] = tmp_delay;

				} else {	/*-------- IN CASE OF REFANT --------*/
					delay[stn_index] = 0.0;
					rate[stn_index] = 0.0;
				}
			}
			bldelay	= (double)bl_direction* (delay[1] - delay[0]);
			blrate	= (double)bl_direction* (rate[1]  - rate[0]);

			/*-------- INTEGRATE VISIBILITY --------*/
			for(freq_index=0; freq_index<freq_num; freq_index++){
				omega	= PI2 * (rf + freq_incr* freq_num/2); 
				d_omega	= PI2 * freq_incr
							*(freq_index - freq_num / 2 ); 

				phase = (d_omega*bldelay + omega*blrate*time_pp);
				cs	= cos(phase);	sn = sin(phase);

				tmp_vr[freq_index] += ( cs * work_ptr[0] + sn * work_ptr[1]);
				tmp_vi[freq_index] += (-sn * work_ptr[0] + cs * work_ptr[1]);

				work_ptr+=2;
			}
			work_ptr	-= (2* freq_num );
			valid_pp++;
		}
	}

	/*-------- BANDPASS and VANVLECK CORRECTION --------*/
	power = vanvleck2(freq_num, vanvnode_num, spline_node, spline_fact,
				-1.0, bp_r_ptr, bp_i_ptr);
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

	/*-------- FREQUENCY CHANNEL BANCHING --------*/
	for(freq_index=0; freq_index< (freq_num/bunch_num); freq_index++){
		for(bunch_index=0; bunch_index<bunch_num; bunch_index++){
			vis_r_ptr[freq_index] += tmp_vr[bunch_num*freq_index+bunch_index];
			vis_i_ptr[freq_index] += tmp_vi[bunch_num*freq_index+bunch_index];
		}
		vis_r_ptr[freq_index] /= (double)bunch_num;
		vis_i_ptr[freq_index] /= (double)bunch_num;
	}
	free(tmp_vr);
	free(tmp_vi);

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
