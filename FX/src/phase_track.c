/*********************************************************
**	LOAD_VIS_ANIM.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <cpgplot.h>
#include <math.h>
#define	CORRDATA	4
#define	CORRFLAG	5
#define	SECDAY		86400
#define	PI			3.14159265358979323846
#define	PI2			6.28318530717958647692

int	phase_track(time_num, time_data_ptr, mjd_min,
				freq_num, solint, ss_index, ssnum,
				xmin, xmax, amp_min, amp_max, phs_min, phs_max,
				vis_r_ptr, vis_i_ptr )

	int		time_num;				/* Number of Time					*/
	double	*time_data_ptr;			/* MJD of the data					*/
	double	mjd_min;				/* Minumu MJD						*/
	int		freq_num;				/* Number of Freq. Channels			*/
	int		solint;					/* Solution Interval [sec]			*/
	int		ss_index;				/* Index Number of SS				*/
	int		ssnum;					/* Total Number of SS				*/
	float	xmin, xmax;				/* Timerange to Plot				*/
	float	amp_min, amp_max;		/* Amplitude Range to Plot			*/
	float	phs_min, phs_max;		/* Phase Range to Plot				*/
	float	*vis_r_ptr;				/* Pointer of Visibility (real) Data */
	float	*vis_i_ptr;				/* Pointer of Visibility (imag) Data */
{
	int		time_index;				/* Index for Time					*/
	int		freq_index;				/* Index for Frequency				*/
	int		node_index;				/* Index of Node Points				*/
	int		vis_index;				/* Index of Visibility				*/
	int		nx_index, ny_index;		/* Index of Plot Sub-Window			*/
	int		nxwin, nywin;			/* Number of Window					*/
	double	init_phase;				/* Initial Smoothed Phase			*/
	double	current_real;			/* Current Smoothed Phase			*/
	double	current_imag;			/* Current Smoothed Phase			*/
	double	current_phase;			/* Current Smoothed Phase			*/
	double	time_init;				/* MJD at Start of the Day			*/
	double	*time_data;				/* Second from Start of the Day		*/
	double	*visr_sum, *visi_sum;	/* Visibility averaged in bandwidth	*/
	double	*vis_wgt;				/* Visibility Weight				*/
	float	*amp_plot, *phs_plot;	/* Pointer of Amp and Phase to Plot	*/
	float	*time_plot;				/* Pointer of Time to Plot			*/
	float	xwin_incr, ywin_incr;	/* Increment of Window Corner Pos	*/
	int		node_real_num;			/* Number of Nodes					*/
	int		node_imag_num;			/* Number of Nodes					*/
	double	*real_coeff;			/* Spline Coefficient				*/
	double	*imag_coeff;			/* Spline Coefficient				*/
	double	*time_real_node;		/* Spline Node						*/
	double	*time_imag_node;		/* Spline Node						*/
	double	cs, sn;					/* cos(phase), sin(phase)			*/
	double	vis_r, vis_i;			/* Temporal Visibility				*/

	/*-------- SSL2 VARIABLE --------*/
	int		spline_dim;				/* Dimension of SPLINE				*/
	int		icon;					/* Condition Code					*/
	int		isw;					/* Control Code						*/
	double	vw[4];					/* Work Space						*/


	int		hour, min, sec;
	float	phase_deg;
/*
------------------------------- INITIALIZE MEMORY AREA FOR VISIBILITIES
*/
	visr_sum = (double *)malloc( time_num* sizeof(double) );
	visi_sum = (double *)malloc( time_num* sizeof(double) );
	time_data= (double *)malloc( time_num* sizeof(double) );
	vis_wgt  = (double *)malloc( time_num* sizeof(double) );
	time_plot= (float *)malloc( time_num* sizeof(float) );
	amp_plot = (float *)malloc( time_num* sizeof(float) );
	phs_plot = (float *)malloc( time_num* sizeof(float) );
	memset(visr_sum, 0, time_num* sizeof(double) );
	memset(visi_sum, 0, time_num* sizeof(double) );
	memset(amp_plot, 0, time_num* sizeof(float) );
	memset(phs_plot, 0, time_num* sizeof(float) );
/*
------------------------------- INTEGRATE VISIBILITIES ALONG BANDWIDTH
*/
#ifdef KIMURA
	printf("--TIME--  ---AMPL---  --PHS[DEG]--\n");
#endif
	time_init = (double)((int)mjd_min);
	for(time_index=0; time_index<time_num; time_index++){
		time_data[time_index] = SECDAY* time_data_ptr[time_index];
		time_plot[time_index] = (float)
			((time_data_ptr[time_index] - time_init)* SECDAY);

#ifdef KIMURA
		printf("INDEX=%d  TIME=%lf %f\n", time_index, time_data[time_index], time_plot[time_index]);
#endif

		/*-------- INTEGRATE VISIBILITY --------*/
		visr_sum[time_index] = 0.0;
		visi_sum[time_index] = 0.0;
		for(freq_index=0; freq_index< freq_num; freq_index++){
			vis_index = time_index* freq_num + freq_index;
			/*-------- INTEG VISIBILITY for BANDWIDTH --------*/
			visr_sum[time_index] += (double)(vis_r_ptr[vis_index]);
			visi_sum[time_index] += (double)(vis_i_ptr[vis_index]);
		}

		visr_sum[time_index] /= (double)freq_num;
		visi_sum[time_index] /= (double)freq_num;

		if( (visi_sum[time_index] == 0) && (visr_sum[time_index] == 0)){
			phs_plot[time_index] = 0.0;
		} else {
			phs_plot[time_index] = (float)atan2(
				visi_sum[time_index], visr_sum[time_index]);
		}

		vis_wgt[time_index]  = sqrt(
			visr_sum[time_index]* visr_sum[time_index]
		  +	visi_sum[time_index]* visi_sum[time_index] );

		amp_plot[time_index] = (float)vis_wgt[time_index];


#ifdef KIMURA
		hour = (int)time_plot[time_index] / 3600;
		min  = ((int)time_plot[time_index] % 3600) / 60;
		sec  =  (int)time_plot[time_index] % 60;
		phase_deg = 360.0* phs_plot[time_index] / PI2;
		if(phase_deg < 0.0){	phase_deg += 360.0; }
		printf("%02d:%02d:%02d  %10.4e  %8.4lf\n",
			hour, min, sec, vis_wgt[time_index], phase_deg);
#endif

	}
/*
------------------------------- SPLINE FIT for PHASE
*/

#ifdef SPLINE

	/*-------- SPLINE Coefficient of Visibilities --------*/
	real_spline(time_data, visr_sum, vis_wgt, time_num,
		(double)solint, &node_real_num, &real_coeff, &time_real_node );
	real_spline(time_data, visi_sum, vis_wgt, time_num,
		(double)solint, &node_imag_num, &imag_coeff, &time_imag_node );
#endif

	/*-------- Plot PH Phase --------*/
	nywin = (int)sqrt((double)(ssnum));
	nxwin = (ssnum + nywin - 1)/nywin;

	xwin_incr = 0.9 / (float)nxwin;
	ywin_incr = 0.9 / (float)nywin;

	nx_index = (ss_index) % nxwin;
	ny_index = (ss_index) / nxwin;
	cpgsvp(	0.067+xwin_incr*nx_index, 0.067+xwin_incr*(nx_index+0.9),
			0.067+ywin_incr*ny_index, 0.067+ywin_incr*(ny_index+0.45) );

	/*-------- Plot Integrated Phase --------*/
	cpgswin( xmin, xmax, phs_min, phs_max);
	cpgsci(4); cpgpt( time_num, time_plot, phs_plot, 1 );

#ifdef SPLINE
	/*-------- Smoothed Phase --------*/
	cpgsci(3);
	isw = 0;	spline_dim = 3;	node_index = 0;
	for(time_index=0; time_index<time_num; time_index++){

		/*-------- SPLINE interporation of Phase --------*/
		dbsf1_( &spline_dim, time_real_node, &node_real_num, 
			real_coeff, &isw, &time_data[time_index], &node_index,
			&current_real, vw, &icon);

		dbsf1_( &spline_dim, time_imag_node, &node_imag_num, 
			imag_coeff, &isw, &time_data[time_index], &node_index,
			&current_imag, vw, &icon);

		current_phase = atan2(current_imag, current_real);
		phs_plot[time_index] = current_phase;

		cs = cos(current_phase);	sn = sin(current_phase);
		for(freq_index=0; freq_index< freq_num; freq_index++){
			vis_index = time_index* freq_num + freq_index;
			/*-------- PHASE CORRECTION --------*/
			vis_r = vis_r_ptr[vis_index];	vis_i = vis_i_ptr[vis_index];
			vis_r_ptr[vis_index] = cs* vis_r + sn* vis_i;
			vis_i_ptr[vis_index] = cs* vis_i - sn* vis_r;
		}
	}

	/*-------- Plot Phase --------*/
	cpgbbuf();
	cpgmove(time_plot[0], phs_plot[0]);
	for(time_index=1; time_index<time_num; time_index++){
		if( fabs(phs_plot[time_index] - phs_plot[time_index-1]) < PI){
			cpgdraw(time_plot[time_index], phs_plot[time_index]);
		} else if(phs_plot[time_index] > phs_plot[time_index-1]){
			cpgdraw(time_plot[time_index], phs_plot[time_index]-PI2);
			cpgmove(time_plot[time_index-1], phs_plot[time_index-1]+PI2);
			cpgdraw(time_plot[time_index], phs_plot[time_index]);
		} else {
			cpgdraw(time_plot[time_index], phs_plot[time_index]+PI2);
			cpgmove(time_plot[time_index-1], phs_plot[time_index-1]-PI2);
			cpgdraw(time_plot[time_index], phs_plot[time_index]);
		}
	}
	cpgebuf();
#endif

	nx_index = (ss_index) % nxwin;
	ny_index = (ss_index) / nxwin;
	cpgsvp(	0.067+xwin_incr*nx_index, 0.067+xwin_incr*(nx_index+0.9),
			0.067+ywin_incr*(ny_index+0.45), 0.067+ywin_incr*(ny_index+0.9) );
	cpgswin( xmin, xmax, amp_min, amp_max);
	cpgsci(4); cpgpt( time_num, time_plot, amp_plot, 17 );


	return(time_num);
}
