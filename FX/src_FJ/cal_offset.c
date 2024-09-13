/*********************************************************
**	CAL_OFFSET.C : Subtract Tsys Offset from Spectrum	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>

int	cal_offset( control, integ_flux, freq_num, rf, freq_incr, xpos_ptr,
			first_vis_r_ptr, first_vis_i_ptr,
			vis_max_ptr, sefd_ptr )

	int		control;			/* 0-> refant 1-> calib				*/
	double	integ_flux;			/* Integrated Flux density [Jy MHz]	*/
	int		freq_num;			/* Number of Spectral Points		*/
	double	rf;					/* Initial Frequency [MHz]			*/
	double	freq_incr;			/* Increment of Frequency [MHz]		*/
	double	*xpos_ptr;			/* Pointer of Cursor Positoin [MHz]	*/
	double	*first_vis_r_ptr;	/* Pointer of Visibility (real)		*/
	double	*first_vis_i_ptr;	/* Pointer of Visibility (imag)		*/
	double	*vis_max_ptr;		/* Maximum Visibility Amplitude		*/
	double	*sefd_ptr;			/* Pointer of SEFD [Jy]				*/
{
	double	*vis_r_ptr;
	double	*vis_i_ptr;
	double	freq;				/* Frequency						*/
	double	scal_freq;			/* Scaled Frequency = freq / rf		*/
	double	flux;				/* Flux Density						*/
	double	d_fact;				/* (Tsys + Ta) / Tsys				*/
	double	p[2][2];			/* Differential Matrix				*/
	double	offline[2];			/* Off-Line Parameter				*/
	double	online;				/* Integrated Flux Density			*/
	double	epsz;				/* Epsiron Zero in Pivotting		*/
	double	vw[2];				/* Work Area in SSL II				*/
	int		ip[2];				/* Control Code						*/
	int		ndim;				/* Dimention of Matrix				*/
	int		isw;				/* Control Code						*/
	int		icon;				/* Condition Code					*/
	int		col_index;			/* Column Index in Matrix			*/
	int		row_index;			/* Row Index in Matrix				*/
	int		freq_index;			/* Index for Frequency				*/

/*
----------------------------------- INITIAL POINTER
*/
	vis_r_ptr = first_vis_r_ptr;
	vis_i_ptr = first_vis_i_ptr;
/*
----------------------------------- INITIALIZE CUMULATIVE VARIABLES
*/
	freq		= rf;
	scal_freq	= 0.0;
	for(col_index=0; col_index<3; col_index++){
		for(row_index=0; row_index<3; row_index++){
			p[col_index][row_index] = 0.0;
		}
		offline[col_index] = 0.0;
	}
/*
----------------------------------- CALC BASELINE BY LINEAR FIT
*/
	/*-------- EMPTY AREA 1 --------*/
	while( freq < xpos_ptr[0] ){
		vis_i_ptr++; vis_r_ptr++;
		scal_freq += freq_incr;
		freq	= rf + scal_freq;
	}

	/*-------- BASELINE AREA 1 --------*/
	while( freq <= xpos_ptr[1] ){
		flux		= sqrt((*vis_r_ptr)*(*vis_r_ptr)
						+  (*vis_i_ptr)*(*vis_i_ptr));
		p[0][0]		+= 1.0;
		p[0][1]		+= scal_freq;
		p[1][0]		+= scal_freq;
		p[1][1]		+= (scal_freq * scal_freq);

		offline[0]	+= flux;
		offline[1]	+= (scal_freq * flux);

		vis_i_ptr++; vis_r_ptr++;
		scal_freq += freq_incr;
		freq	= rf + scal_freq;
	}

	/*-------- EMPTY AREA 2 --------*/
	while( freq < xpos_ptr[2] ){
		vis_i_ptr++; vis_r_ptr++;
		scal_freq += freq_incr;
		freq	= rf + scal_freq;
	}

	/*-------- SIGNAL AREA --------*/
	while( freq <= xpos_ptr[3] ){
		vis_i_ptr++; vis_r_ptr++;
		scal_freq += freq_incr;
		freq	= rf + scal_freq;
	}

	/*-------- EMPTY AREA 3 --------*/
	while( freq < xpos_ptr[4] ){
		vis_i_ptr++; vis_r_ptr++;
		scal_freq += freq_incr;
		freq	= rf + scal_freq;
	}

	/*-------- BASELINE AREA 2 --------*/
	while( freq <= xpos_ptr[5] ){
		flux		= sqrt((*vis_r_ptr)*(*vis_r_ptr)
						+  (*vis_i_ptr)*(*vis_i_ptr));
		p[0][0]		+= 1.0;
		p[0][1]		+= scal_freq;
		p[1][0]		+= scal_freq;
		p[1][1]		+= (scal_freq * scal_freq);

		offline[0]	+= flux;
		offline[1]	+= (scal_freq * flux);

		vis_i_ptr++; vis_r_ptr++;
		scal_freq += freq_incr;
		freq	= rf + scal_freq;
	}
/*
----------------------------------- CALC BASELINE PAREMETER
*/
	#ifdef DEBUG
	for(col_index=0; col_index<2; col_index++){
		for(row_index=0; row_index<2; row_index++){
			printf("%10.2e ",p[col_index][row_index] );
		}
		printf("%10.2e \n", offline[col_index] );
	}
	#endif

	ndim = 2;	epsz = 0.0;
	dalu_( p, &ndim, &ndim, &epsz, ip, &isw, vw, &icon );
	ndim = 2; isw = 1;
	dlux_( offline, p, &ndim, &ndim, &isw, ip, &icon );

	#ifdef DEBUG
	printf("OFFLINE [0] = %e\n", offline[0] );
	printf("OFFLINE [1] = %e\n", offline[1] );
	#endif
/*
----------------------------------- SUBTRACT BASELINE OFFSET
*/
	vis_r_ptr = first_vis_r_ptr;
	vis_i_ptr = first_vis_i_ptr;

	freq		= rf;
	scal_freq	= 0.0;
	online		= 0.0;

	if( control == 1 ){

/*
----------------------------------- CALC SEFD for EVERY STATION
*/
		for(freq_index=0; freq_index< freq_num; freq_index++){

			/*-------- SUBTRACT BASELINE OFFSET --------*/
			d_fact	= 1.0 / (offline[0] + scal_freq* offline[1] );

			vis_r_ptr[freq_index]	-= (1.0 / d_fact);

			/*-------- INTEGRATE FLUX DENSITY --------*/
			if( (freq >= xpos_ptr[2]) && (freq <= xpos_ptr[3] )){

				online	+= (freq_incr * vis_r_ptr[freq_index]);

				/*-------- PEAK SEARCH --------*/
				if( vis_r_ptr[freq_index] > *vis_max_ptr ){
					*vis_max_ptr	= vis_r_ptr[freq_index];
				}

			}
			scal_freq += freq_incr;
			freq	= rf + scal_freq;
		}
		#ifdef DEBUG
		printf("SEFD = %lf [Jy/%%]\n", integ_flux*0.01 / online);
		#endif
		*sefd_ptr	= integ_flux / online;
/*
----------------------------------- CALC INTEGRATED FLUX DENSITY
*/
	} else {

		for(freq_index=0; freq_index< freq_num; freq_index++){
			/*-------- SUBTRACT BASELINE OFFSET --------*/
			d_fact	= 1.0 / (offline[0] + scal_freq* offline[1] );
			vis_r_ptr[freq_index]	*= d_fact;
			vis_r_ptr[freq_index]	-= 1.0;
			vis_r_ptr[freq_index]	*= integ_flux;

			/*-------- INTEGRATE FLUX DENSITY --------*/
			if( (freq >= xpos_ptr[2]) && (freq <= xpos_ptr[3] )){

				online	+= (freq_incr * vis_r_ptr[freq_index] );

				/*-------- PEAK SEARCH --------*/
				if( vis_r_ptr[freq_index] > *vis_max_ptr ){
					*vis_max_ptr	= vis_r_ptr[freq_index];
				}

			}
			scal_freq += freq_incr;
			freq	= rf + scal_freq;
		}
		printf(" INTEGRATED FLUX DENSYTY = %7.2lf Jy MHz [ %lf - %lf MHz]\n",
			online, xpos_ptr[2], xpos_ptr[3] );
    	sefd_ptr[6] = online;
	}
/*
----------------------------------- ENDING
*/

	vis_r_ptr = first_vis_r_ptr;
	vis_i_ptr = first_vis_i_ptr;
	return( 0 );
}
