/*************************************************************
**	INTEG_MULT.C : Visibility Integration for Multi-		**
**					Stream Data.							**
**															**
**	FUNCTION: INPUT VISIILITY DATA, DELAY, and DELAY RATE	**
**				and INTEGRATE.								**
**	AUTHOR	: KAMENO Seiji									**
**	CREATED	: 1996/3/14										**
*************************************************************/
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

long	integ_mult_com(vr_ptr_ptr,	vi_ptr_ptr,		ss_num,		spec_num,
			rf,	freq_incr,	time_num,	time_incr,
			ant_num,		afact_ptr)

	float	**vr_ptr_ptr;	/* INPUT : Pointer of Visibility (REAL) Data */	 
	float	**vi_ptr_ptr;	/* INPUT : Pointer of Visibility (IMAG) Data */
	long	ss_num;			/* INPUT : Number of Sub-Stream				*/
	long	*spec_num;		/* INPUT : Number of Spectrum for Each SS	*/
	double	*rf;			/* INPUT : Initial Freq [MHz] for Each SS	*/
	double	*freq_incr;		/* INPUT : Freq Increment [MHz] for Each SS */
	long	time_num;		/* INPUT : Number of Time					*/
	double	time_incr;		/* INPUT : Time Increment [sec]				*/
	long	ant_num;		/* INPUT : Number of Antenna				*/
	double	*afact_ptr;		/* IN/OUT: Pointer of Visibility amp		*/
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
	double	phase;				/* (trial) Visibility Phase */
	double	omega;				/* Angular Frequency */
	double	d_omega;			/* Angular Frequency */
	double	omega_ref;			/* Reference Angular Frequency			*/
	double	time;				/* Time from the start [sec] */
	double	bldelay;			/* Baseline-Based Delay					*/
	double	blrate;				/* Baseline-Based Rate					*/
	double	trial_vr[MAX_BL][MAX_SS];		/* Trial Visibility (real)	*/
	double	trial_vi[MAX_BL][MAX_SS];		/* Trial Visibility (imag)	*/
	double	cs, sn;
	long	time_ptr_offset;

/*
---------------------------------------------- INITIAL SETTINGS
*/
	baseline_num= ant_num*(ant_num-1)/2;				/* Number of Baseline */
	afact_num	= 2*(baseline_num*ss_num + ant_num);	/* Number of Variable */
/*
---------------------------------------------- INITIAL PARAMETER
*/
	omega_ref = 0.0;
	for(ss_index=0; ss_index<ss_num; ss_index++){
		omega_ref	+=	(PI2 * (rf[ss_index]
					+	(0.5*(double)spec_num[ss_index] - 0.5)
					*	freq_incr[ss_index]) );
	}
	omega_ref /= ss_num;
	for(i=0; i<baseline_num; i++){
		for(j=0; j<ss_num; j++){
			trial_vr[i][j]	= 0.0;
			trial_vi[i][j]	= 0.0;
		}
	}
/*
---------------------------------------------- DATA STORE
*/
	/*-------- LOOP FOR BASELINE  --------*/
	bl_vr_ptr_ptr	= vr_ptr_ptr;
	bl_vi_ptr_ptr	= vi_ptr_ptr;
	for(bl_index=0; bl_index < baseline_num; bl_index++){

		bl2ant(bl_index, &ant2, &ant1);	/* GET ANTENNA NUMBER */

		delay_index1=  2*baseline_num*ss_num + ant1;
		delay_index2=  2*baseline_num*ss_num + ant2;
		rate_index1	=  delay_index1 + ant_num;
		rate_index2	=  delay_index2 + ant_num;

		bldelay = afact_ptr[delay_index2] - afact_ptr[delay_index1];
		blrate	= afact_ptr[rate_index2]  - afact_ptr[rate_index1];

/*
		printf(" BL%d: DELAY = %e, RATE = %e\n", bl_index, bldelay, blrate);
*/

		/*-------- LOOP FOR SUB-STREAM  --------*/
		for(ss_index=0; ss_index<ss_num; ss_index++){
			time_ptr_offset	= 0;

			/*-------- INDEX in AFACT --------*/
			amp_index	= bl_index*ss_num + ss_index;
			phs_index	= amp_index + baseline_num*ss_num;

			/*-------- LOOP FOR TIME POINTS --------*/
			for(time_index=0; time_index<time_num; time_index++){
				time= time_incr*(time_index - time_num/2);	/* TIME in SEC */

				/*-------- LOOP FOR SPECTRAL POINTS  --------*/
				for(spec_index=0; spec_index<spec_num[ss_index]; spec_index++){

					omega	= PI2 * (rf[ss_index]
							+ freq_incr[ss_index] * spec_index);
					d_omega	= omega - omega_ref;
					phase = d_omega* bldelay + omega* time* blrate;

#ifdef HIDOI
					omega = PI2 * (rf[ss_index] + freq_incr[ss_index]
								* spec_index);
					d_omega = PI2* freq_incr[ss_index]
								* (spec_index - spec_num[ss_index] / 2);
					phase = (d_omega*bldelay + omega*blrate*time);
#endif

					/*-------- PHASE TO BE COMPENSATED --------*/
					cs	= cos( phase );
					sn	= sin( phase );

					/*-------- ACCESS POINTER to the VISIBILITY --------*/

					vr_ptr 	= *bl_vr_ptr_ptr
							+ (time_ptr_offset + spec_index);
					vi_ptr 	= *bl_vi_ptr_ptr
							+ (time_ptr_offset + spec_index);

					/*-------- INTEGRATE TO TRIAL VISIBILITY --------*/
					trial_vr[bl_index][ss_index]+=( cs*(*vr_ptr)+ sn*(*vi_ptr));
					trial_vi[bl_index][ss_index]+=(-sn*(*vr_ptr)+ cs*(*vi_ptr));

				}	/*-------- END OF SPECTRAL LOOP --------*/

				time_ptr_offset	+= spec_num[ss_index];

			}	/*-------- END OF TIME LOOP --------*/

			afact_ptr[amp_index] =
			sqrt(trial_vr[bl_index][ss_index]*trial_vr[bl_index][ss_index]
				+trial_vi[bl_index][ss_index]*trial_vi[bl_index][ss_index])
				/ (spec_num[ss_index]*time_num);
			afact_ptr[phs_index] = atan2(trial_vi[bl_index][ss_index],
								 trial_vr[bl_index][ss_index]);

			bl_vr_ptr_ptr++;
			bl_vi_ptr_ptr++;

		}	/*-------- END OF SUB-STREAM LOOP --------*/

	}	/*-------- END OF BASELINE LOOP --------*/
	return(0);
}
