/*********************************************************
**	CRS_SEARCH.C: COARSE FRINGE SEARCH for Each Baseline	**
**														**
**	FUNCTION: INPUT VISIILITY DATA and SOLVE DELAY, 	**
**				DELAY RATE, DELAY ACCERELATION for		**
**				each STATION							**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/3/14									**
*********************************************************/
#include	"cpgplot.h"
#include	<stdio.h>
#include	<math.h>
#define		PI		3.14159265358979323846
#define		PI2		6.28318530717958647688

int	acc_search(vr_ptr, vi_ptr,	acc, freq_num,
			time_num, wind_x, wind_y, visamp_ptr)

	float	*vr_ptr;		/* INPUT : Pointer of Visibility (Real) Data */	 
	float	*vi_ptr;		/* INPUT : Pointer of Visibility (Real) Data */
	double	acc;			/* INPUT : Fringe Acceleration				*/
	int		freq_num;		/* INPUT : Number for Spectrum */
	int		time_num;		/* INPUT : Number of Time */
	int		wind_x;			/* INPUT : SEARCH WINDOW GRID */
	int		wind_y;			/* INPUT : SEARCH WINDOW GRID */
	float	*visamp_ptr;	/* OUTPUT: Pointer of Visibility amp */
{
	float	*vis_r;
	float	*vis_i;
	float	*first_vis_r;
	float	*first_vis_i;
	float	afact;				/* Normalizing Factor					*/
	double	phase;				/* Modulation Phase by Acceleration		*/
	double	cs, sn;				/* cos(phase), sin(phase)				*/
	double	rel_time;			/* Relative Time						*/
	int		ndim;				/* Number of Dimension of the Data		*/
	int		icon;				/* Condition Code in SSL2				*/
	int		nfft[2];
	int		ndir;				/* FFT DIRECTION */
	int		irate;
	int		idelay;
	int		time_index;
	int		freq_index;
/*
---------------------------------------------- STORE DATA
*/

	afact	= 1.0/(float)(time_num*freq_num);

	/*-------- FFT POINTS [2-times overlap] --------*/
	nfft[1] = pow2round(time_num)*2;
	nfft[0] = freq_num*2;

	/*-------- ALLOC MEMORY FOR FFT --------*/
	vis_r	= (float *)malloc( nfft[0] * nfft[1] * sizeof(float) );
	vis_i	= (float *)malloc( nfft[0] * nfft[1] * sizeof(float) );
	if( (vis_r == NULL) || (vis_i == NULL) ){
		printf("Failed to Alloc Memory in COARSE !!\n");
		return(-1);
	}

	/*-------- REMEMBER THE FIRST POINTER --------*/
	first_vis_r = vis_r;
	first_vis_i = vis_i;

	/*-------- STORE VISIBITLITY DATA --------*/
	for(time_index=0; time_index<time_num; time_index++){

		rel_time	= (double)(time_index - time_num/2);
		phase		= 0.5*acc* rel_time* rel_time;
		cs	= cos(phase);	sn	= sin(phase);

		for(freq_index=0; freq_index<freq_num; freq_index++){
			*vis_r = *vr_ptr * cs - *vi_ptr * sn;
			*vis_i = *vr_ptr * sn + *vi_ptr * cs;
			vr_ptr++;	vi_ptr++;
			vis_r ++;	vis_i ++;
		}
		for(freq_index=freq_num; freq_index<nfft[0]; freq_index++){
			*vis_r = 0.0;	vis_r ++;
			*vis_i = 0.0;	vis_i ++;
		}
	}
	for(time_index=time_num; time_index<nfft[1]; time_index++){
		for(freq_index=0; freq_index<nfft[0]; freq_index++){
			*vis_r = 0.0;	vis_r++;
			*vis_i = 0.0;	vis_i++;
		}
	}

	vis_r	= first_vis_r;
	vis_i	= first_vis_i;

/*
---------------------------------------------- FFT
*/
	ndim = 2;	ndir = 1;
	cft_( vis_r, vis_i, nfft, &ndim, &ndir, &icon);
/*
------------------------------------------ RESTORE CROSS-CORR FUNCTION 
*/
	vis_r = first_vis_r	+ (nfft[1] - wind_y/2 + 1)*nfft[0] - wind_x/2;
	vis_i = first_vis_i	+ (nfft[1] - wind_y/2 + 1)*nfft[0] - wind_x/2;

	for(time_index=0; time_index<wind_y/2; time_index++){
		for(freq_index=0; freq_index<wind_x/2; freq_index++){
			*vis_r	*= afact;	*vis_i	*= afact;
			*visamp_ptr	+= sqrt( (*vis_r)*(*vis_r) + (*vis_i)*(*vis_i) ); 
			visamp_ptr++; vis_r++; vis_i++;
		}
		vis_r -= nfft[0];	vis_i -= nfft[0];
		for(freq_index=0; freq_index<wind_x/2; freq_index++){
			*vis_r	*= afact;	*vis_i	*= afact;
			*visamp_ptr	+= sqrt( (*vis_r)*(*vis_r) + (*vis_i)*(*vis_i) ); 
			visamp_ptr++; vis_r++; vis_i++;
		}
		vis_r += (2*nfft[0]-wind_x);	vis_i += (2*nfft[0]-wind_x);
	}

	vis_r = first_vis_r	+ nfft[0] - wind_x/2;
	vis_i = first_vis_i	+ nfft[0] - wind_x/2;

	for(time_index=0; time_index<wind_y/2; time_index++){
		for(freq_index=0; freq_index<wind_x/2; freq_index++){
			*vis_r	*= afact;	*vis_i	*= afact;
			*visamp_ptr	+= sqrt( (*vis_r)*(*vis_r) + (*vis_i)*(*vis_i) ); 
			visamp_ptr++; vis_r++; vis_i++;
		}
		vis_r -= nfft[0];	vis_i -= nfft[0];
		for(freq_index=0; freq_index<wind_x/2; freq_index++){
			*vis_r	*= afact;	*vis_i	*= afact;
			*visamp_ptr	+= sqrt( (*vis_r)*(*vis_r) + (*vis_i)*(*vis_i) ); 
			visamp_ptr++; vis_r++; vis_i++;
		}
		vis_r += (2*nfft[0]-wind_x);	vis_i += (2*nfft[0]-wind_x);
	}

	vis_r = first_vis_r;
	vis_i = first_vis_i;

	free(vis_r);
	free(vis_i);

	return(0);
}
