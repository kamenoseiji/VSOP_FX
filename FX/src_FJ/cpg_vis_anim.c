/*********************************************************
**	CPG_BP_ANIM.C: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <cpgplot.h>
#include <math.h>
#include "obshead.inc"
#define	CORRDATA	4
#define	SECDAY		86400
#define	MAX_SS		32
#define	PI	3.14159265358979323846
#define	PI2	6.28318530717958647692

int	cpg_vis_anim( stn_name1, stn_name2, obs_name, obj_name,
			time_data_ptr, time_num, integ_time,
			ssnum, ssid_in_cfs, freq_num_ptr, rf_ptr, freq_incr_ptr,
			amp_max, vis_r_ptr, vis_i_ptr )

	char	*stn_name1;				/* Station Name #1				*/
	char	*stn_name2;				/* Station Name #1				*/
	char	*obs_name;				/* Observation Name				*/
	char	*obj_name;				/* Object Name					*/
	double	*time_data_ptr;			/* Pointer of Integ Start [MJD]	*/
	int		time_num;				/* Number of Time Data			*/
	double	integ_time;				/* Integ time [sec]				*/
	int		ssnum;					/* Number of Sub-Stream			*/
	int		*ssid_in_cfs;			/* SS ID in CODA				*/
	int		*freq_num_ptr;			/* Number of Freq. Channels		*/
	double	*rf_ptr;				/* RF Frequency [MHz]			*/
	double	*freq_incr_ptr;			/* Frequency Increment			*/
	float	amp_max;				/* Amplitude Max to Plot		*/
	float	**vis_r_ptr;			/* Pointer of Visibility Data	*/
	float	**vis_i_ptr;			/* Pointer of Visibility Data	*/
{
	FILE	*vis_file_ptr;			/* File to Save Visibility		*/
	float	xmin, xmax;
	float	ymin, ymax;
	int		ss_index;
	int		nxwin, nywin;
	int		nx_index, ny_index;
	int		err_code;
	int		freq_index;
	int		index_max;
	int		time_index;
	int		center_year;
	int		center_doy;
	int		center_hh;
	int		center_mm;
	double	center_ss;
	double	center_mjd;
	double	bldelay;
	double	blrate;
	float	*freq_ptr;
	float	*vis_amp_ptr;
	float	*vis_phs_ptr;
	double	*visr_sum[MAX_SS];
	double	*visi_sum[MAX_SS];
	float	*visr_ave[MAX_SS];
	float	*visi_ave[MAX_SS];
	float	xwin_incr,	ywin_incr;
	float	x_text, y_text;
	float	vis_max[MAX_SS];
	char	text[256];
	char	pg_device[32];

	for(ss_index=0; ss_index<ssnum; ss_index++){
		vis_max[ss_index]		= 0.0;
		visr_ave[ss_index] = (float *)malloc(
								freq_num_ptr[ss_index]* sizeof(float));
		visi_ave[ss_index] = (float *)malloc(
								freq_num_ptr[ss_index]* sizeof(float));

		visr_sum[ss_index] = (double *)malloc(
								freq_num_ptr[ss_index]* sizeof(double));
		visi_sum[ss_index] = (double *)malloc(
								freq_num_ptr[ss_index]* sizeof(double));
		memset( visr_sum[ss_index], 0, freq_num_ptr[ss_index]* sizeof(double) );
		memset( visi_sum[ss_index], 0, freq_num_ptr[ss_index]* sizeof(double) );
	}
	err_code	= 32;
	cpgqinf("FILE", pg_device, &err_code);

	err_code = 0;
	/*-------- OPEN PGPLOT DEVICE --------*/
	if( strstr( pg_device, "cps") != NULL ){
		cpgscrn(0, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(1, "Black", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(2, "ivory", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(3, "Blue", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(4, "Green", &err_code);			/* COLOR DEFINISHON */

	} else if( strstr( pg_device, "ps") == NULL ){
		cpgscrn(0, "DarkSlateGray", &err_code);	/* COLOR DEFINISHON */
		cpgscrn(1, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(2, "SlateGray", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(3, "Yellow", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(4, "Cyan", &err_code);			/* COLOR DEFINISHON */
	} else {
		cpgscrn(0, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(1, "Black", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(2, "LightGray", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(3, "Black", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(4, "Black", &err_code);			/* COLOR DEFINISHON */
	}


	for( time_index=0; time_index<time_num; time_index++){
		cpgbbuf();
		cpgeras();

		nywin	= (int)sqrt((double)ssnum);
		nxwin	= (ssnum + nywin - 1)/nywin;
		xwin_incr = 0.9 / (float)nxwin;
		ywin_incr = 0.9 / (float)nywin;

		cpgsvp( 0.0, 1.0, 0.0, 1.0 );
		cpgswin( 0.0, 1.0, 0.0, 1.0 );
		cpgsci(1); cpgsch(1.0);
		cpgtext( 0.45, 0.025, "Frequency [MHz]" );
		cpgsch(0.7);
		sprintf( text, "EXPER    :  %s", obs_name);cpgtext( 0.65, 0.980, text );
		sprintf( text, "SOURCE   : %s", obj_name); cpgtext( 0.65, 0.962, text );
		sprintf( text, "BASELINE : %s - %s", stn_name1, stn_name2);
												cpgtext( 0.65, 0.944, text );

		center_mjd	= time_data_ptr[time_index] + 0.5*integ_time/SECDAY;
		fmjd2doy( center_mjd,
			&center_year, &center_doy, &center_hh, &center_mm, &center_ss );

		sprintf( text, "TIME     : %04d %03dd %02d:%02d:%02d",
			center_year, center_doy, center_hh, center_mm, (int)center_ss );
		cpgtext( 0.65, 0.926, text );

		for(ss_index=0; ss_index<ssnum; ss_index++){
			cpgsch(1.0);
			nx_index	= ss_index % nxwin;
			ny_index	= ss_index / nxwin;

			/*-------- X-AXIS DATA --------*/
			freq_ptr	=(float *)malloc(freq_num_ptr[ss_index]* sizeof(float));
			vis_amp_ptr	=(float *)malloc(freq_num_ptr[ss_index]* sizeof(float));
			vis_phs_ptr	=(float *)malloc(freq_num_ptr[ss_index]* sizeof(float));

			#ifdef DEBUG
			printf("READING..SS=%d, Time=%d <- %X\n",
				ss_index, time_index, vis_r_ptr[ss_index]);
			#endif
			bldelay = 0.0;
			blrate  = 0.0;


			for(freq_index=0; freq_index< freq_num_ptr[ss_index]; freq_index++){
				*visr_sum[ss_index] += (double)(*vis_r_ptr[ss_index]);
				*visi_sum[ss_index] += (double)(*vis_i_ptr[ss_index]);
				visr_sum[ss_index] ++;	visi_sum[ss_index] ++;
				vis_r_ptr[ss_index] ++;	vis_i_ptr[ss_index] ++;
			}
			visr_sum[ss_index] -= freq_num_ptr[ss_index];
			visi_sum[ss_index] -= freq_num_ptr[ss_index];
			vis_r_ptr[ss_index] -= freq_num_ptr[ss_index];
			vis_i_ptr[ss_index] -= freq_num_ptr[ss_index];

			for(freq_index=0; freq_index< freq_num_ptr[ss_index]; freq_index++){
				*visr_ave[ss_index] = *visr_sum[ss_index] / (time_index + 1);
				*visi_ave[ss_index] = *visi_sum[ss_index] / (time_index + 1);
				visr_sum[ss_index] ++;	visi_sum[ss_index] ++;
				visr_ave[ss_index] ++;	visi_ave[ss_index] ++;
			}
			visr_sum[ss_index] -= freq_num_ptr[ss_index];
			visi_sum[ss_index] -= freq_num_ptr[ss_index];
			visr_ave[ss_index] -= freq_num_ptr[ss_index];
			visi_ave[ss_index] -= freq_num_ptr[ss_index];

			cp_vis( rf_ptr[ss_index], freq_incr_ptr[ss_index],
				integ_time* (time_index - time_num/2),
				bldelay, blrate,
				freq_num_ptr[ss_index],

				visr_ave[ss_index], visi_ave[ss_index],
/*
				vis_r_ptr[ss_index], vis_i_ptr[ss_index],
*/
				vis_amp_ptr, vis_phs_ptr, &vis_max[ss_index], &index_max);

			vis_r_ptr[ss_index] += freq_num_ptr[ss_index];
			vis_i_ptr[ss_index] += freq_num_ptr[ss_index];

			/*-------- PLOT WINDOW --------*/
			xmin = rf_ptr[ss_index] - 0.5* freq_incr_ptr[ss_index];
			xmax = xmin + ((double)(freq_num_ptr[ss_index]) - 0.5) 
					* freq_incr_ptr[ss_index];

			ymin = 0.0;		ymax = amp_max;
/*
------------------------------------------------- PLOT PHASE
*/
			cpgsvp(	0.067+xwin_incr*nx_index, 0.067+xwin_incr*(nx_index+0.9),
				0.075+ywin_incr*ny_index, 0.067+ywin_incr*(ny_index+0.95/2.0));

			cpgswin(xmin, xmax, -PI, PI);
			cpgsci(2);	cpgrect(xmin, xmax, -PI, PI);
			cpgsci(0);	cpgbox("G", 0.0, 0, "G", 0.0, 0);
			cpgsch(0.8);
			cpgsci(1);	cpgbox(	"BCNTS", 0.0, 0,
								"BCNTS", 0.0, 0 );
			if( ss_index == 0){
				cpglab("", "PHASE [rad]", "");
			}

			cpgsch(1.0);
			/*------------------- PLOT THE PHASE ---------------*/
			for( freq_index=0; freq_index<freq_num_ptr[ss_index]; freq_index++){
				freq_ptr[freq_index] =	xmin +
										freq_index * freq_incr_ptr[ss_index];
			}

			cpgsch(1.0);
			cpgsci(4);	cpgpt(freq_num_ptr[ss_index], freq_ptr, vis_phs_ptr, 1);
/*
------------------------------------------------- PLOT AMP
*/
			cpgsvp(	0.067+xwin_incr*nx_index, 0.067+xwin_incr*(nx_index+0.9),
					0.067+ywin_incr*(ny_index+0.95/2.0),
					0.059+ywin_incr*(ny_index+0.95));

			cpgswin(xmin, xmax, ymin, ymax);
			cpgsci(2);	cpgrect(xmin, xmax, ymin, ymax);
			cpgsci(0);	cpgbox("G", 0.0, 0, "G", 0.0, 0);
			cpgsch(0.8);
			cpgsci(1);	cpgbox(	"BCTS", 0.0, 0, "BCNTS", 0.0, 0 );
			if( ss_index == 0){
				cpglab("", "AMP [scaled by Tsys]", "");
			}

			cpgsch(1.0);
			cpgsci(3);	cpgline(freq_num_ptr[ss_index], freq_ptr, vis_amp_ptr);

			x_text = xmin*0.25+ xmax*0.75;
			y_text = ymin*0.1 + ymax*0.9;
			sprintf(text, "SS = %d", ssid_in_cfs[ss_index] );
			cpgsch(1.0); cpgsci(3);	cpgtext( x_text, y_text, text );

			free(freq_ptr);
			free(vis_amp_ptr);
			free(vis_phs_ptr);
		}
		cpgebuf();
	}

/*
------------------------------------------------- PLOT FINAL RESULT
*/
	for(ss_index=0; ss_index<ssnum; ss_index++){
		cpgsch(1.0);
		nx_index	= ss_index % nxwin;
		ny_index	= ss_index / nxwin;

		/*-------- X-AXIS DATA --------*/
		freq_ptr	=(float *)malloc(freq_num_ptr[ss_index]* sizeof(float));
		vis_amp_ptr	=(float *)malloc(freq_num_ptr[ss_index]* sizeof(float));
		vis_phs_ptr	=(float *)malloc(freq_num_ptr[ss_index]* sizeof(float));

		cp_vis( rf_ptr[ss_index], freq_incr_ptr[ss_index],
			integ_time* (time_index - time_num/2),
			bldelay, blrate,
			freq_num_ptr[ss_index],
			visr_ave[ss_index], visi_ave[ss_index],
			vis_amp_ptr, vis_phs_ptr, &vis_max[ss_index], &index_max);


		/*-------- PLOT WINDOW --------*/
		xmin = rf_ptr[ss_index] - 0.5* freq_incr_ptr[ss_index];
		xmax = xmin + ((double)(freq_num_ptr[ss_index]) - 0.5) 
				* freq_incr_ptr[ss_index];

		ymin = 0.0;		ymax = amp_max;
		cpgsvp(	0.067+xwin_incr*nx_index, 0.067+xwin_incr*(nx_index+0.9),
				0.075+ywin_incr*ny_index, 0.067+ywin_incr*(ny_index+0.95/2.0));

		cpgswin(xmin, xmax, -PI, PI);
		cpgsci(2);	cpgrect(xmin, xmax, -PI, PI);
		cpgsci(0);	cpgbox("G", 0.0, 0, "G", 0.0, 0);
		cpgsch(0.8);
		cpgsci(1);	cpgbox(	"BCNTS", 0.0, 0,
								"BCNTS", 0.0, 0 );
		if( ss_index == 0){
			cpglab("", "PHASE [rad]", "");
		}

		cpgsch(1.0);
		/*------------------- PLOT THE PHASE ---------------*/
		for( freq_index=0; freq_index<freq_num_ptr[ss_index]; freq_index++){
			freq_ptr[freq_index] =	xmin + freq_index * freq_incr_ptr[ss_index];
		}

		cpgsch(1.0);
		cpgsci(4);	cpgpt(freq_num_ptr[ss_index], freq_ptr, vis_phs_ptr, 1);

		cpgsvp(	0.067+xwin_incr*nx_index, 0.067+xwin_incr*(nx_index+0.9),
				0.067+ywin_incr*(ny_index+0.95/2.0),
				0.059+ywin_incr*(ny_index+0.95));

		cpgswin(xmin, xmax, ymin, ymax);
		cpgsci(2);	cpgrect(xmin, xmax, ymin, ymax);
		cpgsci(0);	cpgbox("G", 0.0, 0, "G", 0.0, 0);
		cpgsch(0.8);
		cpgsci(1);	cpgbox(	"BCTS", 0.0, 0, "BCNTS", 0.0, 0 );
		if( ss_index == 0){
			cpglab("", "AMP [scaled by Tsys]", "");
		}

		cpgsch(1.0);
		cpgsci(3);	cpgline(freq_num_ptr[ss_index], freq_ptr, vis_amp_ptr);

		x_text = xmin*0.25+ xmax*0.75;
		y_text = ymin*0.1 + ymax*0.9;
		sprintf(text, "SS = %d", ssid_in_cfs[ss_index] );
		cpgsch(1.0); cpgsci(3);	cpgtext( x_text, y_text, text );

		free(freq_ptr);
		free(vis_amp_ptr);
		free(vis_phs_ptr);
	}
	cpgebuf();





	vis_file_ptr = fopen("hidoi.vis", "w");
	if(vis_file_ptr == NULL){
		printf("Can't Open hidoi.vis\n");
	}
	for(ss_index=0; ss_index<ssnum; ss_index++){
		for(freq_index=0; freq_index< freq_num_ptr[ss_index]; freq_index++){
			*visr_sum[ss_index] /= (double)time_num;
			*visi_sum[ss_index] /= (double)time_num;
			visr_sum[ss_index] ++;	visi_sum[ss_index] ++;
		}
		visr_sum[ss_index] -= freq_num_ptr[ss_index];
		visi_sum[ss_index] -= freq_num_ptr[ss_index];
		fwrite(visr_sum[ss_index], 1, freq_num_ptr[ss_index]* sizeof(double),
			vis_file_ptr);
		fwrite(visi_sum[ss_index], 1, freq_num_ptr[ss_index]* sizeof(double),
			vis_file_ptr);
	}
	fclose(vis_file_ptr);
	return(0);
}
