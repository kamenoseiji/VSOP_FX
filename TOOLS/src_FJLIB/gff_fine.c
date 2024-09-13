/*********************************************************
**	GFF_MULT.C	: GLOBAL FRINGE FITTING module for		**
**					Multi-Stream Visibility Data		**
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
#define		MAX_ITER	7
#define		COR_LIMIT	1.0e-5
#define		PI		3.14159265358979323846
#define		PI2		6.28318530717958647688
#define		MAX_ANT	10
#define		MAX_VAR 108

int		gff_fine(vr_ptr_ptr,vi_ptr_ptr,		ss_id,		ss_num,
			spec_num,	freq_init_ptr,	freq_incr_ptr,	time_num,
			time_incr,	ant_num,		afact_ptr,		afact_err_ptr,
			resid )

	float	**vr_ptr_ptr;	/* INPUT : Pointer of Visibility (REAL) Data	*/
	float	**vi_ptr_ptr;	/* INPUT : Pointer of Visibility (IMAG) Data	*/
	int		*ss_id;			/* INPUT : SS ID in CFS							*/
	int		ss_num;			/* INPUT : Number of Sub-Stream					*/
	int		spec_num;		/* INPUT : Number of Spectrum for Each SSi		*/
	double	*freq_init_ptr;	/* INPUT : Initial Frequency [MHz] for Each SS	*/
	double	*freq_incr_ptr;	/* INPUT : Freq. Increment [MHz] for Each SS	*/
	int		time_num;		/* INPUT : Number of Time						*/
	double	time_incr;		/* INPUT : Time Increment [sec]					*/
	int		ant_num;		/* INPUT : Number of Antenna					*/
	double	*afact_ptr;		/* IN/OUT: Pointer of Parameters				*/
	double	*afact_err_ptr;	/* OUTPUT: Pointer of Error of Parameters		*/
	double	*resid;			/* OUTPUT: Pointer of Residual 					*/
{
	int		i, j, k;
	int		baseline_num;		/* Baseline Number */
	int		bl_index;			/* Baseline Index */
	int		ss_index;			/* Sub-Stream Index */
	int		time_index;			/* Time Index */
	int		spec_index;			/* Spectrum Index */
	int		ndim;				/* Number of Dimension of the Data */
	int		ant1, ant2;			/* Antenna Index for Each Baseline */
	int		amp_index;			/* Amplitude Index in afact[] */
	int		phs_index;			/* Phase Index in afact[] */
	int		delay_index1;		/* Delay Index in afact[] */
	int		delay_index2;		/* Delay Index in afact[] */
	int		rate_index1;		/* Delay Rate Index in afact[] */
	int		rate_index2;		/* Delay Rate Index in afact[] */
	int		afact_num;			/* Number of Variable to be Determined */
	float	**bl_vr_ptr_ptr;	/* Pointer of Visibility (real) */
	float	**bl_vi_ptr_ptr;	/* Pointer of Visibility (imag) */
	float	*vr_ptr;			/* Pointer of Visibility (real) */
	float	*vi_ptr;			/* Pointer of Visibility (imag) */
	double	afact[MAX_VAR];		/* Variable to be Determined */
	double	d[MAX_VAR];			/* Correction Vector */
	double	pwp[MAX_VAR][MAX_VAR];	/* Weighted Partial Matrix */
	double	pr[MAX_VAR];		/* PARTIAL MATRIX (real) */
	double	pi[MAX_VAR];		/* PARTIAL MATRIX (imag) */
	double	cor;				/* RSS of the Correction Vector */
	double	residual;			/* Residual */
	double	phase;				/* (trial) Visibility Phase */
	double	omega;				/* Angular Frequency */
	double	omega_ref;			/* Reference Angular Frequency */
	double	d_omega;			/* Angular Frequency in Video Band */
	double	time;				/* Time from the start [sec] */
	double	trial_vr;			/* Trial Visibility (real) */
	double	trial_vi;			/* Trial Visibility (imag) */
	double	rr;					/* Residual (real) */
	double	ri;					/* Residual (imag) */
	double	vis_sigma;			/* Standard Deviation of Visibility */
	double	bldelay;			/* Baseline-based Delay				*/
	double	blrate;				/* Baseline-based Delay Rate		*/
	int		datanum;			/* Number of Data */ 
	int		pdim;				/* Dimension of the Partial Matrix */
	double	epsz;				/* Pibotting Factor */
	int		isw;				/* Control Code in SSL2 */
	int		is;					/* Control Code in SSL2 */
	double	vw[MAX_VAR];		/* Work Area in SSL2 */
	int		ip[MAX_VAR];		/* Work Area in SSL2 */
	int		index[6];
	int		icon;				/* Condition Code in SSL2 */
	int		time_ptr_offset;
	int		niter;				/* Iteration Counter */

/*
---------------------------------------------- INITIAL SETTINGS
*/
	baseline_num= ant_num*(ant_num-1)/2;			/* Number of Baseline */
	afact_num	= 2*(baseline_num + ant_num - 1);	/* Number of Variable */
	if(afact_num > MAX_VAR){
		printf("MEMORY ERROR !!  Please Reduce Sub-Stream or Baseline.\n");
		*resid	= 9999.0;
		return(-1);
	}
/*
---------------------------------------------- INITIAL PARAMETER
*/
	for(i=0; i<afact_num; i++){
		afact[i] = afact_ptr[i];
	}

	/*-------- WEIGHT CENTER OF FREQUENCY as REF_FREQ --------*/
	omega_ref	= 0.0;
	for(ss_index=0; ss_index<ss_num; ss_index++){
		omega_ref	+= (PI2 * (freq_init_ptr[ss_index]
					+ 0.5*(double)spec_num* freq_incr_ptr[ss_index]) );
	}
	omega_ref /= ss_num;
/*
---------------------------------------------- DATA STORE
*/
	niter = 0;
	/*-------- ITERATION LOOP --------*/
	while(niter < MAX_ITER){
		residual		= 0.0;

		/*-------- INITIALIZE PARTIAL MATRIX and RESIDUAL VECTOR --------*/
		for(i=0; i<afact_num; i++){
			for(j=0; j<afact_num; j++){ pwp[i][j] = 0.0; }
			d[i] = 0.0;
		}

		/*-------- LOOP FOR BASELINE  --------*/

		bl_vr_ptr_ptr	= vr_ptr_ptr;
		bl_vi_ptr_ptr	= vi_ptr_ptr;

		for(bl_index=0; bl_index < baseline_num; bl_index++){
			bl2ant(bl_index, &ant2, &ant1);	/* GET ANTENNA NUMBER */

			/*-------- INDEX in AFACT --------*/
			amp_index	= bl_index;
			phs_index	= amp_index + baseline_num;
			delay_index2=  2*baseline_num + ant2 - 1;
			rate_index2	=  delay_index2 + ant_num - 1;

			/*-------- BASELINE INCLUDES REFANT --------*/
			if( ant1 == 0 ){
				delay_index1=  - 1;
				rate_index1	=  - 1;
				bldelay	= afact[delay_index2];
				blrate	= afact[rate_index2];

			/*-------- BASELINE DOES NOT INCLUDES REFANT --------*/
			} else {
				delay_index1=  2*baseline_num + ant1 - 1;
				rate_index1	=  delay_index1 + ant_num - 1;
				bldelay	= afact[delay_index2] - afact[delay_index1];
				blrate	= afact[rate_index2] - afact[rate_index1];
			}

			/*-------- LOOP FOR SUB-STREAM  --------*/
			for(ss_index=0; ss_index<ss_num; ss_index++){

				vis_sigma	= 2.0e6*(*freq_incr_ptr)*time_incr;
				time_ptr_offset	= 0;

				index[0]	=  amp_index;
				index[1]	=  phs_index;
				index[2]	=  delay_index1;
				index[3]	=  delay_index2;
				index[4]	=  rate_index1;
				index[5]	=  rate_index2;

				/*-------- LOOP FOR TIME POINTS --------*/
				for(time_index=0; time_index<time_num; time_index++){

					time	= time_incr
							* (time_index - time_num/2);	/* TIME in SEC */

					/*-------- LOOP FOR SPECTRAL POINTS  --------*/
					for(spec_index=0; spec_index < spec_num; spec_index++){

						omega	= PI2 * ((*freq_init_ptr)
									+ (*freq_incr_ptr) * spec_index);
						d_omega	= omega - omega_ref;
						phase	= afact[phs_index]
								+ d_omega* bldelay + omega* time* blrate;

						/*-------- TRIAL VISIBILITY --------*/
						trial_vr	= afact[amp_index]*cos( phase );
						trial_vi	= afact[amp_index]*sin( phase );

						/*-------- ACCESS POINTER to the VISIBILITY --------*/
						vr_ptr 	= (*bl_vr_ptr_ptr)
								+ (time_ptr_offset+ spec_index);
						vi_ptr 	= (*bl_vi_ptr_ptr)
								+ (time_ptr_offset+ spec_index);

						/*-------- RESIDUAL VECTOR --------*/
						rr	= (*vr_ptr) - trial_vr; 
						ri	= (*vi_ptr) - trial_vi; 

						residual += vis_sigma*(rr*rr + ri*ri);

						/*-------- PARTIAL MATRIX COMPONENTS --------*/

						pr[amp_index]		= trial_vr/afact[amp_index];
						pi[amp_index]		= trial_vi/afact[amp_index];

						pr[phs_index]		=-trial_vi;
						pi[phs_index]		= trial_vr;

						pr[delay_index2]	=-d_omega*trial_vi;
						pi[delay_index2]	= d_omega*trial_vr;

						pr[rate_index2]		=-omega*time*trial_vi;
						pi[rate_index2]		= omega*time*trial_vr;

						if(ant1 != 0){
							pr[delay_index1]= d_omega*trial_vi;
							pi[delay_index1]=-d_omega*trial_vr;
							pr[rate_index1]	= omega*time*trial_vi;
							pi[rate_index1]	=-omega*time*trial_vr;
						} else {
							pr[delay_index1]= 0.0;
							pi[delay_index1]= 0.0;
							pr[rate_index1]	= 0.0;
							pi[rate_index1]	= 0.0;
						}

						for(i=0; i<6; i++){
							if( index[i] != -1 ){
							for(j=0; j<6; j++){
								if( index[j] != -1 ){
								pwp[(index[i])][(index[j])] += vis_sigma * 
									( pr[(index[i])]*pr[(index[j])]
									+ pi[(index[i])]*pi[(index[j])] );
								}
							}
							d[index[i]]	+= vis_sigma
							* ( pr[(index[i])] * rr + pi[(index[i])] * ri );
							}
						}

					}	/*-------- END OF SPECTRAL LOOP --------*/

					time_ptr_offset	+= spec_num;

				}	/*-------- END OF TIME LOOP --------*/

				freq_init_ptr++;
				freq_incr_ptr++;
	
				bl_vr_ptr_ptr++;
				bl_vi_ptr_ptr++;

			}	/*-------- END OF SUB-STREAM LOOP --------*/

			freq_init_ptr	-= ss_num;
			freq_incr_ptr	-= ss_num;

		}	/*-------- END OF BASELINE LOOP --------*/

		#ifdef DEBUG
		for(i=0; i<afact_num; i++){
			for(j=0; j<afact_num; j++){
				printf("%7.1e ", pwp[i][j]);
			}
			printf(" | %7.1e\n", d[i]);
		}
		#endif

		/*-------- SOLVE FOR PARTIAL MATRIX --------*/
		pdim = afact_num;	ndim = MAX_VAR;	epsz = 0.0;	isw = 1;
		dalu_( pwp, &ndim, &pdim, &epsz, ip, &is, vw, &icon);
		if(icon != 0){
			printf(" ITERATION HALTED. ICON = %d\n", icon);
			*resid	= 9999.0;
			return(icon);
		}
		isw = 1;
		dlux_( d, pwp, &ndim, &pdim, &isw, ip, &icon);
		if(icon != 0){
			printf(" ITERATION HALTED. ICON = %d\n", icon);
			*resid	= 9999.0;
			return(icon);
		}

		/*-------- ERROR ESTIMATION --------*/
		dluiv_(pwp, &ndim, &pdim, ip, &icon);

		/*-------- CORRECTION INCLUDING AMPLITUDE --------*/
		for(i=0; i<afact_num; i++){
			afact[i]+= d[i];
			cor		+= d[i]*d[i];
		}
		cor = sqrt(cor)/afact_num;

		residual/= (2* ss_num* spec_num* time_num* baseline_num - afact_num);

		/*-------- IN CASE OF AMP < 0 --------*/
		for(bl_index=0; bl_index<baseline_num; bl_index++){
			amp_index	= bl_index;
			phs_index	= amp_index + baseline_num;
			if(afact[amp_index] < 0.0){
				afact[amp_index] *= -1;
				afact[phs_index] = atan2(
					-sin(afact[phs_index]), -cos(afact[phs_index]));
			} else {
				afact[phs_index] = atan2(
					sin(afact[phs_index]), cos(afact[phs_index]));
			}
		}

		/*-------- CONVERGED ? --------*/
		#ifdef DEBUG
		printf("ITER %2d [AMP FREE] : CHI^2=%9.6f AMP=%e DELAY=%e\n",
			niter, residual, afact[0], afact[2*baseline_num]);
		#endif
		*resid = residual;

		if(cor < COR_LIMIT){ break; }

		niter++;
	}	/*-------- END OF ITERATION LOOP --------*/

	for(i=0; i<afact_num; i++){
		afact_ptr[i]		= afact[i];
		afact_err_ptr[i]	= sqrt(pwp[i][i]);
	}

	return(0);
}
