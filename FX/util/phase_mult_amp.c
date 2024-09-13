/*********************************************************
**	GFF.C	: GLOBAL FRINGE FITTING module				**
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
#define		MAX_ITER	10
#define		PI		3.14159265358979323846
#define		PI2		6.28318530717958647688
#define		MAX_ANT	10
#define		MAX_BL	(MAX_ANT*(MAX_ANT-1))/2
#define		MAX_SS	16
#define		MAX_VAR 684

long	phase_mult(vr_ptr_ptr,	vi_ptr_ptr,		ss_num,		spec_num_ptr,
			freq_init_ptr,	freq_incr_ptr,	time_num,	time_incr,
			ant_num,		afact_ptr)

	float	**vr_ptr_ptr;	/* INPUT : Pointer of Visibility (REAL) Data */	 
	float	**vi_ptr_ptr;	/* INPUT : Pointer of Visibility (IMAG) Data */
	long	ss_num;			/* INPUT : Number of Sub-Stream */
	long	*spec_num_ptr;	/* INPUT : Number of Spectrum for Each SS */
	double	*freq_init_ptr;	/* INPUT : Initial Frequency [MHz] for Each SS */
	double	*freq_incr_ptr;	/* INPUT : Frequency Increment [MHz] for Each SS */
	long	time_num;		/* INPUT : Number of Time */
	double	time_incr;		/* INPUT : Time Increment [sec] */
	long	ant_num;		/* INPUT : Number of Antenna */
	float	*afact_ptr;		/* IN/OUT: Pointer of Visibility amp */
{
	long	i, j;
	long	baseline_num;		/* Baseline Number */
	long	bl_index;			/* Baseline Index */
	long	ss_index;			/* Sub-Stream Index */
	long	time_index;			/* Time Index */
	long	spec_index;			/* Spectrum Index */
	long	ant1, ant2;			/* Antenna Index for Each Baseline */
	long	amp_index;			/* Amplitude Index in afact[] */
	long	phs_index;			/* Phase Index in afact[] */
	long	delay_index1;		/* Delay Index in afact[] */
	long	delay_index2;		/* Delay Index in afact[] */
	long	rate_index1;		/* Delay Rate Index in afact[] */
	long	rate_index2;		/* Delay Rate Index in afact[] */
	long	afact_num;			/* Number of Variable to be Determined */
	float	**bl_vr_ptr_ptr;	/* Pointer of Visibility (real) */
	float	**bl_vi_ptr_ptr;	/* Pointer of Visibility (imag) */
	float	*vr_ptr;			/* Pointer of Visibility (real) */
	float	*vi_ptr;			/* Pointer of Visibility (imag) */
	float	afact[MAX_VAR];		/* Variable to be Determined */
	float	phase;				/* (trial) Visibility Phase */
	float	omega;				/* Angular Frequency */
	float	d_omega;			/* Angular Frequency in Video Band */
	float	time;				/* Time from the start [sec] */
	float	trial_vr[MAX_BL][MAX_SS];		/* Trial Visibility (real) */
	float	trial_vi[MAX_BL][MAX_SS];		/* Trial Visibility (imag) */
	float	cs, sn;
	long	time_ptr_offset;
	long	ss_ptr_offset;

/*
---------------------------------------------- INITIAL SETTINGS
*/
	baseline_num= ant_num*(ant_num-1)/2;				/* Number of Baseline */
	afact_num	= 2*(baseline_num*ss_num + ant_num - 1);/* Number of Variable */
/*
---------------------------------------------- INITIAL PARAMETER
*/
	for(i=0; i<afact_num; i++){
		afact[i]	= *afact_ptr;
		afact_ptr++;
	}
	afact_ptr	-= afact_num;
	for(i=0; i<baseline_num; i++){
		for(j=0; j<ss_num; j++){
			trial_vr[i][j]	= 0.0;
			trial_vi[i][j]	= 0.0;
		}
	}
/*
---------------------------------------------- DATA STORE
*/
	ss_ptr_offset	= 0;
	/*-------- LOOP FOR SUB-STREAM  --------*/
	for(ss_index=0; ss_index<ss_num; ss_index++){

		time_ptr_offset	= 0;
		/*-------- LOOP FOR TIME POINTS --------*/
		for(time_index=0; time_index<time_num; time_index++){

			time= time_incr*(time_index - time_num/2);	/* TIME in SEC */

			/*-------- LOOP FOR SPECTRAL POINTS  --------*/
			for(spec_index=0; spec_index < *spec_num_ptr; spec_index++){

				d_omega	= PI2	* (*freq_incr_ptr)
								* (spec_index - *spec_num_ptr/2);
				omega	= PI2 * ((*freq_init_ptr)
							+ (*freq_incr_ptr) * spec_index);

				/*-------- LOOP FOR BASELINE  --------*/
				bl_vr_ptr_ptr	= vr_ptr_ptr;
				bl_vi_ptr_ptr	= vi_ptr_ptr;
				for(bl_index=0; bl_index < baseline_num; bl_index++){

					bl2ant(bl_index, &ant2, &ant1);	/* GET ANTENNA NUMBER */

					/*-------- INDEX in AFACT --------*/
					amp_index	= bl_index*ss_num + ss_index;
					phs_index	= amp_index + baseline_num*ss_num;
					delay_index1=  2*baseline_num*ss_num + ant1 - 1;
					delay_index2=  2*baseline_num*ss_num + ant2 - 1;
					rate_index1	=  delay_index1 + ant_num - 1;
					rate_index2	=  delay_index2 + ant_num - 1;

					if(ant1 == 0){

						/*------- INCLUDE REF ANT --------*/
						phase	= d_omega*afact[delay_index2]
								+ omega*time*afact[rate_index2];

					} else{

						/*------- NOT INCLUDE REF ANT --------*/
						phase	=
						+d_omega*(afact[delay_index2]- afact[delay_index1])
						+omega*time*(afact[rate_index2]-afact[rate_index1]);
					}

					/*-------- TRIAL VISIBILITY --------*/
					cs	= (float)cos( phase );
					sn	= (float)sin( phase );

					/*-------- ACCESS POINTER to the VISIBILITY --------*/
					vr_ptr 	= *bl_vr_ptr_ptr
							+ (time_ptr_offset+ ss_ptr_offset+ spec_index);
					vi_ptr 	= *bl_vi_ptr_ptr
							+ (time_ptr_offset+ ss_ptr_offset+ spec_index);

					trial_vr[bl_index][ss_index]+= cs*(*vr_ptr)	+ sn*(*vi_ptr);
					trial_vi[bl_index][ss_index]+=-sn*(*vr_ptr)	+ cs*(*vi_ptr);

					bl_vr_ptr_ptr++;
					bl_vi_ptr_ptr++;

				}	/*-------- END OF BASELINE LOOP --------*/

			}	/*-------- END OF SPECTRAL LOOP --------*/

			time_ptr_offset	+= *spec_num_ptr;

		}	/*-------- END OF TIME LOOP --------*/

		for(bl_index=0; bl_index<baseline_num; bl_index++){
			amp_index	= bl_index*ss_num + ss_index;
			phs_index	= amp_index + baseline_num*ss_num;
			afact[amp_index] =
			sqrt(trial_vr[bl_index][ss_index]*trial_vr[bl_index][ss_index]
				+trial_vi[bl_index][ss_index]*trial_vi[bl_index][ss_index])
				/ ((*spec_num_ptr)*time_num);
			afact[phs_index] = atan2(trial_vr[bl_index][ss_index],
									 trial_vi[bl_index][ss_index]);
		}

		ss_ptr_offset	+= (*spec_num_ptr)*time_num;
		spec_num_ptr++;
		freq_init_ptr++;
		freq_incr_ptr++;

	}	/*-------- END OF SUB-STREAM LOOP --------*/

	for(i=0; i<afact_num; i++){
		*afact_ptr	= afact[i];
		afact_ptr++;
	}
	afact_ptr	-= afact_num;

	return(0);
}
