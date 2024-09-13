/*************************************************************
**	INTEG_AMPHI.C : Visibility Integration for Multi-Stream	**
**					Stream Data Using P-CAL Data.			**
**															**
**	FUNCTION: INPUT VISIILITY DATA, DELAY, and DELAY RATE	**
**				and INTEGRATE.								**
**	AUTHOR	: KAMENO Seiji									**
**	CREATED	: 1996/3/14										**
*************************************************************/
#include	"cpgplot.h"
#include	<stdio.h>
#include	<math.h>
#define		PI		3.14159265358979323846
#define		PI2		6.28318530717958647688
#define		SECDAY	86400
#define		MAX_SS	32

long	integ_amphi( vr_ptr_ptr,	vi_ptr_ptr,
			ss_num,		spec_num_ptr,
			freq_init_ptr,	freq_incr_ptr,
			start_offset,	time_num,	time_incr,
			bldelay,	blrate,	pcphs_ptr,
			integ_vr_ptr,	integ_vi_ptr)

	float	**vr_ptr_ptr;	/* INPUT : Pointer of Visibility (REAL) Data */	 
	float	**vi_ptr_ptr;	/* INPUT : Pointer of Visibility (IMAG) Data */
	int		ss_num;			/* INPUT : Number of Sub-Stream */
	int		*spec_num_ptr;	/* INPUT : Number of Spectrum for Each SS */
	double	*freq_init_ptr;	/* INPUT : Initial Frequency [MHz] for Each SS */
	double	*freq_incr_ptr;	/* INPUT : Frequency Increment [MHz] for Each SS */
	double	start_offset;	/* INPUT : Start Time Offset [sec]*/
	int		time_num;		/* INPUT : Number of Time */
	double	time_incr;		/* INPUT : Time Increment [sec] */
	double	bldelay;		/* INPUT : Baseline Delay [microsec]*/
	double	blrate;			/* INPUT : Baseline Delay Rate [microsec/sec]*/
	double	*pcphs_ptr;		/* INPUT : Pointer of P-Cal phase [rad]*/
	double	*integ_vr_ptr;	/* OUTPUT: Pointer of Visibility [real] */
	double	*integ_vi_ptr;	/* OUTPUT: Pointer of Visibility [real] */
{
	float	*vr_ptr;
	float	*vi_ptr;
	long	ss_index;			/* Sub-Stream Index */
	long	time_index;			/* Time Index */
	long	spec_index;			/* Spectrum Index */
	double	phase;				/* (trial) Visibility Phase */
	double	omega;				/* Angular Frequency */
	double	d_omega;			/* Angular Frequency in Video Band */
	double	time;				/* Time from the start [sec] */
	double	trial_vr[MAX_SS];	/* Trial Visibility (real) */
	double	trial_vi[MAX_SS];	/* Trial Visibility (imag) */
	double	cs, sn;

/*
---------------------------------------------- INITIAL PARAMETER
*/
	for(ss_index=0; ss_index<ss_num; ss_index++){
		trial_vr[ss_index]	= 0.0;
		trial_vi[ss_index]	= 0.0;
	}
/*
---------------------------------------------- DATA STORE
*/
	/*-------- LOOP FOR SUB-STREAM  --------*/
	for(ss_index=0; ss_index<ss_num; ss_index++){
		vr_ptr	= *vr_ptr_ptr;
		vi_ptr	= *vi_ptr_ptr;

		#ifdef DEBUG
		printf("PCAL PHASE = %lf\n", pcphs_ptr[ss_index]);
		#endif

		/*-------- LOOP FOR TIME POINTS --------*/
		for(time_index=0; time_index<time_num; time_index++){
			time= time_incr*(time_index - time_num/2 );	/* TIME in SEC */
			time += start_offset;	

			/*-------- LOOP FOR SPECTRAL POINTS  --------*/
			for(spec_index=0; spec_index < *spec_num_ptr; spec_index++){

				d_omega	= PI2	* (*freq_incr_ptr)
								* (spec_index - *spec_num_ptr/2);
				omega	= PI2	* ((*freq_init_ptr)
								+ (*freq_incr_ptr) * spec_index);

				phase	= d_omega* bldelay
						+ omega*time*blrate
						+ pcphs_ptr[ss_index];

				/*-------- PHASE TO BE COMPENSATED --------*/
				cs	= cos( phase );
				sn	= sin( phase );

				/*-------- INTEGRATE TO TRIAL VISIBILITY --------*/
				trial_vr[ss_index]	+= ( cs*(*vr_ptr)+ sn*(*vi_ptr));
				trial_vi[ss_index]	+= (-sn*(*vr_ptr)+ cs*(*vi_ptr));

				vr_ptr++;
				vi_ptr++;

			}	/*-------- END OF SPECTRAL LOOP --------*/


		}	/*-------- END OF TIME LOOP --------*/

		trial_vr[ss_index] /= (double)((*spec_num_ptr)*time_num);
		trial_vi[ss_index] /= (double)((*spec_num_ptr)*time_num);

		#ifdef DEBUG
		printf("VIS[%02d] = %lf, %lf\n",ss_index,
			sqrt(trial_vr[ss_index] * trial_vr[ss_index]
				+trial_vi[ss_index] * trial_vi[ss_index])*100.0,
			atan2( trial_vi[ss_index], trial_vr[ss_index]) );
		#endif

		*integ_vr_ptr	+= trial_vr[ss_index];
		*integ_vi_ptr	+= trial_vi[ss_index];

		vr_ptr_ptr++;
		vi_ptr_ptr++;
		spec_num_ptr++;
		freq_init_ptr++;
		freq_incr_ptr++;

	}	/*-------- END OF SUB-STREAM LOOP --------*/
	*integ_vr_ptr /= ss_num;
	*integ_vi_ptr /= ss_num;

	return(0);
}
