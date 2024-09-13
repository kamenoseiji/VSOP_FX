/*************************************************************
**	SEARCH_FINE.C : Visibility Integration for Multi-		**
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

int	search_fine(vr_ptr_ptr,	vi_ptr_ptr,		ss_num,		spec_num,
			freq_init_ptr,	freq_incr_ptr,
			time_num,	integ_num,
			start_offset,	time_incr,		sum_delay_ptr,
			ant_num,		afact_ptr,		ss_id )

	float	**vr_ptr_ptr;	/* INPUT : Pointer of Visibility (REAL) Data	*/
	float	**vi_ptr_ptr;	/* INPUT : Pointer of Visibility (IMAG) Data	*/
	int		ss_num;			/* INPUT : Number of Sub-Stream					*/
	int		spec_num;		/* INPUT : Number of Spectrum for Each SS		*/
	double	*freq_init_ptr;	/* INPUT : Initial Frequency [MHz] for Each SS	*/
	double	*freq_incr_ptr;	/* INPUT : Freq. Increment [MHz] for Each SS	*/
	int		time_num;		/* INPUT : Number of Time						*/
	int		integ_num;		/* INPUT : PP Number to Integrate				*/
	int		start_offset;	/* INPUT : Start Offset PP Number				*/
	double	time_incr;		/* INPUT : Time Increment [sec]					*/
	double	*sum_delay_ptr;	/* INPUT : Summation of Residual Delay			*/
	int		ant_num;		/* INPUT : Number of Antenna					*/
	double	*afact_ptr;		/* IN/OUT: Pointer of Visibility amp			*/
	int		*ss_id;			/* INPUT : SS ID in CFS							*/
{
	int		i, j;
	int		blnum;		/* Baseline Number					*/
	int		bl_index;			/* Baseline Index					*/
	int		ss_index;			/* Sub-Stream Index					*/
	int		time_index;			/* Time Index						*/
	int		spec_index;			/* Spectrum Index					*/
	int		ant1, ant2;			/* Antenna Index for Each Baseline	*/
	int		amp_index;			/* Amplitude Index in afact[]		*/
	int		phs_index;			/* Phase Index in afact[]			*/
	int		delay_index1;		/* Delay Index in afact[]			*/
	int		delay_index2;		/* Delay Index in afact[]			*/
	int		rate_index1;		/* Delay Rate Index in afact[]		*/
	int		rate_index2;		/* Delay Rate Index in afact[]		*/
	int		afact_num;			/* Number of Variable to Determined	*/
	float	**bl_vr_ptr_ptr;	/* Pointer of Visibility (real)		*/
	float	**bl_vi_ptr_ptr;	/* Pointer of Visibility (imag)		*/
	float	*vr_ptr;			/* Pointer of Visibility (real)		*/
	float	*vi_ptr;			/* Pointer of Visibility (imag)		*/
	double	afact[MAX_VAR];		/* Variable to be Determined		*/
	double	phase;				/* (trial) Visibility Phase			*/
	double	omega;				/* Frequency						*/
	double	omega_ref;			/* Frequency						*/
	double	d_omega;			/* Frequency						*/
	double	time;				/* Time from the start [sec]		*/
	double	trial_vr[MAX_BL];	/* Trial Visibility (real)			*/
	double	trial_vi[MAX_BL];	/* Trial Visibility (imag)			*/
	double	blphs;				/* Baseline-based Initial Phase		*/
	double	bldelay;			/* Baseline-based Delay 			*/
	double	blrate;				/* Baseline-based Delay Rate		*/
	double	cs, sn;
	long	time_ptr_offset;

/*
---------------------------------------------- INITIAL SETTINGS
*/
	blnum= ant_num*(ant_num-1)/2;			/* Number of Baseline */
	afact_num	= 2*(blnum + ant_num - 1);	/* Number of Variable */

	printf("ANTNUM = %d, BLNUM = %d, AFACTNUM = %d, SSNUM = %d\n",
		ant_num, blnum, afact_num, ss_num );
/*
---------------------------------------------- INITIAL PARAMETER
*/
	omega_ref	= 0.0;
	for(ss_index=0; ss_index<ss_num; ss_index++){
		omega_ref	+= (PI2 * (freq_init_ptr[ss_index]
					+ 0.5*(double)spec_num* freq_incr_ptr[ss_index]) );
	}
	omega_ref /= ss_num;

	for(i=0; i<afact_num; i++){
		afact[i]	= afact_ptr[i];
	}
	for(i=0; i<blnum; i++){
		trial_vr[i]	= 0.0;
		trial_vi[i]	= 0.0;
	}
/*
---------------------------------------------- DATA STORE
*/
	/*-------- LOOP FOR BASELINE  --------*/
	bl_vr_ptr_ptr	= vr_ptr_ptr;
	bl_vi_ptr_ptr	= vi_ptr_ptr;
	for(bl_index=0; bl_index < blnum; bl_index++){
		bl2ant(bl_index, &ant2, &ant1);	/* GET ANTENNA NUMBER */

		/*-------- INDEX in AFACT --------*/
		amp_index	= bl_index;
		phs_index	= amp_index + blnum;
		delay_index1=  2*blnum + ant1 - 1;
		delay_index2=  2*blnum + ant2 - 1;
		rate_index1	=  delay_index1 + ant_num - 1;
		rate_index2	=  delay_index2 + ant_num - 1;

		/*-------- BASELINE INCLUDES REFANT --------*/
		if(ant1 == 0){
			bldelay	= afact[delay_index2];
			blrate	= afact[rate_index2];
			blphs	= omega_ref* sum_delay_ptr[ant2 - 1];
			blphs	= blphs - PI2*(int)(blphs/PI2);

		/*-------- BASELINE DOES NOT INCLUDES REFANT --------*/
		} else {
			bldelay	= afact[delay_index2] - afact[delay_index1];
			blrate	= afact[rate_index2] - afact[rate_index1];
			blphs	= omega_ref*(sum_delay_ptr[ant2-1]-sum_delay_ptr[ant1-1]);
			blphs	= blphs - PI2*(int)(blphs/PI2);
		}

		/*-------- LOOP FOR SUB-STREAM  --------*/
		for(ss_index=0; ss_index<ss_num; ss_index++){
			time_ptr_offset	= 0;

			/*-------- LOOP FOR TIME POINTS --------*/
			for(time_index=0; time_index<integ_num; time_index++){
				time= time_incr*(time_index - integ_num/2);	/* TIME in SEC */

				/*-------- LOOP FOR SPECTRAL POINTS  --------*/
				for(spec_index=0; spec_index < spec_num; spec_index++){

					omega	= PI2*( (*freq_init_ptr)
								+ (*freq_incr_ptr)*spec_index );
					d_omega	= omega - omega_ref;

					phase	= d_omega * bldelay
							+ omega * time* blrate + blphs;

					/*-------- PHASE TO BE COMPENSATED --------*/
					cs = cos( phase );	sn = sin( phase );

					/*-------- ACCESS POINTER to the VISIBILITY --------*/

					vr_ptr 	= *bl_vr_ptr_ptr
							+ (time_ptr_offset + spec_index);
					vi_ptr 	= *bl_vi_ptr_ptr
							+ (time_ptr_offset + spec_index);

					/*-------- INTEGRATE TO TRIAL VISIBILITY --------*/
					trial_vr[bl_index] += ( cs*(*vr_ptr)+ sn*(*vi_ptr));
					trial_vi[bl_index] += (-sn*(*vr_ptr)+ cs*(*vi_ptr));


				}	/*-------- END OF SPECTRAL LOOP --------*/

				time_ptr_offset	+= spec_num;
			}	/*-------- END OF TIME LOOP --------*/

			freq_init_ptr++;
			freq_incr_ptr++;

			bl_vr_ptr_ptr++;
			bl_vi_ptr_ptr++;

		}	/*-------- END OF SUB-STREAM LOOP --------*/

		afact[amp_index] = sqrt(trial_vr[bl_index]*trial_vr[bl_index]
						+		trial_vi[bl_index]*trial_vi[bl_index])
						/ 		(spec_num*ss_num*integ_num);
		afact[phs_index] = atan2(trial_vi[bl_index], trial_vr[bl_index]);



		#ifdef DEBUG
		#endif
		printf("VIS[%02d] = %lf, %lf\n",
			bl_index, afact[amp_index]*100.0, afact[phs_index]);
		#ifdef DEBUG
		#endif

		freq_init_ptr	-= ss_num;
		freq_incr_ptr	-= ss_num;

	}	/*-------- END OF BASELINE LOOP --------*/

	for(i=0; i<afact_num; i++){
		afact_ptr[i]	= afact[i];
	}

	return(0);
}
