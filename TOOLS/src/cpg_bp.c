/*********************************************************
**	CPG_BP.C	: Test Module to Read CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <cpgplot.h>
#include <math.h>
#define	CORRDATA	4
#define	PI	3.14159265358979323846
#define	PI2	6.28318530717958647692

int	cpg_bp(
	char	*obs_name,				/* Observation Name				*/
	char	*stn_name,				/* Station Name					*/
	char	*obj_name,				/* Object Name					*/
	double	start_mjd,				/* Integ Start [MJD ]			*/
	double	stop_mjd,				/* Integ Stop [MJD ]			*/
	double	integ_time,				/* Integ time [sec]				*/
	int		ssnum,					/* Number of Sub-Stream			*/
	int		*freq_num_ptr,			/* Number of Freq. Channels		*/
	double	*rf_ptr,				/* RF Frequency [MHz]			*/
	double	*freq_incr_ptr,			/* Frequency Increment			*/
	double	*vis_max_ptr,			/* Maximum Visibility			*/
	double	**vis_r_ptr,			/* Pointer of Visibility Data	*/
	double	**vis_i_ptr)			/* Pointer of Visibility Data	*/
{
	float	xmin, xmax;
	float	ymin, ymax;
	double	x_incr, y_incr;
	int		ss_index;
	int		nxwin, nywin;
	int		nx_index, ny_index;
	int		err_code;
	int		freq_index;
	float	*freq_ptr;
	float	*vis_amp_ptr;
	float	xwin_incr,	ywin_incr;
	float	x_text, y_text;
	char	text[32];
	char	pg_device[32];

	err_code	= 32;
	cpgqinf("FILE", pg_device, &err_code);
	err_code = 0;

	/*-------- OPEN PGPLOT DEVICE --------*/
	if( strstr( pg_device, "cps") != NULL ){
		cpgscrn(0, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(1, "Black", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(2, "ivory", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(3, "Blue", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(4, "Pink", &err_code);			/* COLOR DEFINISHON */
	} else if( strstr( pg_device, "ps") == NULL ){
		cpgscrn(0, "DarkSlateGray", &err_code);	/* COLOR DEFINISHON */
		cpgscrn(1, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(2, "SlateGrey", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(3, "Yellow", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(4, "Cyan", &err_code);			/* COLOR DEFINISHON */
	} else {
		cpgscrn(0, "White", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(1, "Black", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(2, "LightGrey", &err_code);		/* COLOR DEFINISHON */
		cpgscrn(3, "Black", &err_code);			/* COLOR DEFINISHON */
		cpgscrn(4, "Black", &err_code);			/* COLOR DEFINISHON */
	}
	cpgeras();

	cpgbbuf();
	nxwin	= (int)sqrt((double)ssnum);
	nywin	= (ssnum + nxwin - 1)/nxwin;
	xwin_incr = 0.9 / (float)nxwin;
	ywin_incr = 0.9 / (float)nywin;

	cpgsvp( 0.0, 1.0, 0.0, 1.0 );
	cpgswin( 0.0, 1.0, 0.0, 1.0 );
	cpgsci(1); cpgsch(1.0);
	sprintf( text, "%s Bandpass", stn_name );
	cpgtext( 0.42, 0.975, text );
	cpgtext( 0.45, 0.025, "Frequency [MHz]" );
	cpgsch(0.5);
	sprintf( text, "EXPER :  %s", obs_name ); cpgtext( 0.65, 0.990, text );
	sprintf( text, "SOURCE : %s", obj_name ); cpgtext( 0.65, 0.975, text );
	sprintf( text, "INTEG : %8.2lf - %8.2lf [MJD] (Valid %6.2lf sec)",
					start_mjd, stop_mjd, integ_time);
	cpgtext( 0.65, 0.960, text );

	cpgsch(0.5);
	for(ss_index=0; ss_index<ssnum; ss_index++){

		nx_index	= ss_index % nxwin;
		ny_index	= ss_index / nxwin;

		if(*freq_num_ptr != 0){

		/*-------- PLOT WINDOW --------*/
		xmin = *rf_ptr - 0.5*(*freq_incr_ptr);
		xmax = xmin + ((double)(*freq_num_ptr) - 0.5) * (*freq_incr_ptr);

		ymin = 0.0;		ymax = (float)(*vis_max_ptr)*1.2;

		cpg_incr( xmax-xmin, &x_incr);
		cpg_incr( ymax-ymin, &y_incr);

		cpgsvp(	0.067+xwin_incr*nx_index, 0.067+xwin_incr*(nx_index+0.9),
				0.067+ywin_incr*ny_index, 0.067+ywin_incr*(ny_index+0.9));
		cpgswin(xmin, xmax, ymin, ymax);

		/*-------- X-AXIS DATA --------*/
		freq_ptr	= (float *)malloc( *freq_num_ptr * 2* sizeof(float) );
		vis_amp_ptr	= (float *)malloc( *freq_num_ptr * 2* sizeof(float) );

		cp_vis( *freq_num_ptr, *vis_r_ptr, *vis_i_ptr, vis_amp_ptr);

		for( freq_index=0; freq_index<*freq_num_ptr; freq_index++){
			freq_ptr[2* freq_index] = xmin + freq_index * (*freq_incr_ptr);
			freq_ptr[2* freq_index] -= 0.5 * (*freq_incr_ptr);
			freq_ptr[2* freq_index + 1] = freq_ptr[2* freq_index] + (*freq_incr_ptr);
		}

		cpgsci(2);	cpgrect(xmin, xmax, ymin, ymax);
		cpgsci(0);	cpgbox("G", (float)x_incr, 1, "G", (float)y_incr, 1);
		cpgsci(1);	cpgbox(	"BCNTS", (float)x_incr, 10,
							"BCNTS", (float)y_incr*2, 5 );

		cpgsci(3);	cpgline( *freq_num_ptr* 2, freq_ptr, vis_amp_ptr );

		x_text = xmin*0.2 + xmax*0.8;
		y_text = ymin*0.1 + ymax*0.9;
		sprintf(text, "SS = %d", ss_index);
		cpgsci(3);	cpgtext( x_text, y_text, text );
		free(freq_ptr);
		free(vis_amp_ptr);
		}

		vis_r_ptr++;
		vis_i_ptr++;

		vis_max_ptr++;
		freq_num_ptr++;
		rf_ptr++;
		freq_incr_ptr++;
	}

	cpgebuf();

	return(0);
}

cp_vis(
	int		freq_num,
	double	*org_real_ptr,
	double	*org_imag_ptr,
	float	*dest_amp_ptr)
{
	int		freq_index;

	for(freq_index=0; freq_index<freq_num; freq_index++){
		dest_amp_ptr[2* freq_index] = (float)sqrt(
			  (*org_real_ptr)*(*org_real_ptr)
			+ (*org_imag_ptr)*(*org_imag_ptr) );
		dest_amp_ptr[2* freq_index + 1] = dest_amp_ptr[2* freq_index];
		org_real_ptr++;	org_imag_ptr++;
	}
	return;
}
