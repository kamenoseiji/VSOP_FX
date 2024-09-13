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
#define		MAX_ITER	20
#define		COR_LIMIT1	1.0e-3
#define		COR_LIMIT2	1.0e-6
#define		PI		3.14159265358979323846
#define		PI2		6.28318530717958647688
#define		MAX_ANT	10
#define		MAX_SS	16
#define		MAX_VAR 684

long	gff_mult(vr_ptr_ptr,	vi_ptr_ptr,		ss_num,		spec_num_ptr,
			freq_init_ptr,	freq_incr_ptr,	time_num,	time_incr,
			ant_num,		afact_ptr,		afact_err_ptr)

	float	**vr_ptr_ptr;	/* INPUT : Pointer of Visibility (REAL) Data */	 
	float	**vi_ptr_ptr;	/* INPUT : Pointer of Visibility (IMAG) Data */
	long	ss_num;			/* INPUT : Number of Sub-Stream */
	long	*spec_num_ptr;	/* INPUT : Number of Spectrum for Each SS */
	double	*freq_init_ptr;	/* INPUT : Initial Frequency [MHz] for Each SS */
	double	*freq_incr_ptr;	/* INPUT : Frequency Increment [MHz] for Each SS */
	long	time_num;		/* INPUT : Number of Time */
	double	time_incr;		/* INPUT : Time Increment [sec] */
	long	ant_num;		/* INPUT : Number of Antenna */
	float	*afact_ptr;		/* IN/OUT: Pointer of Parameters */
	float	*afact_err_ptr;	/* OUTPUT: Pointer of Error of Parameters */
{
	long	i, j, k;
	long	baseline_num;		/* Baseline Number */
	long	bl_index;			/* Baseline Index */
	long	ss_index;			/* Sub-Stream Index */
	long	time_index;			/* Time Index */
	long	spec_index;			/* Spectrum Index */
	long	ndim;				/* Number of Dimension of the Data */
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
	float	d[MAX_VAR];			/* Correction Vector */
	float	pwp[MAX_VAR][MAX_VAR];	/* Weighted Partial Matrix */
	float	pr[MAX_VAR];		/* PARTIAL MATRIX (real) */
	float	pi[MAX_VAR];		/* PARTIAL MATRIX (imag) */
	float	cor;				/* RSS of the Correction Vector */
	float	residual;			/* Residual */
	float	phase;				/* (trial) Visibility Phase */
	float	omega;				/* Angular Frequency */
	float	d_omega;			/* Angular Frequency in Video Band */
	float	time;				/* Time from the start [sec] */
	float	trial_vr;			/* Trial Visibility (real) */
	float	trial_vi;			/* Trial Visibility (imag) */
	float	rr;					/* Residual (real) */
	float	ri;					/* Residual (imag) */
	float	vis_sigma;
	long	datanum;			/* Number of Data */ 
	long	pdim;				/* Dimension of the Partial Matrix */
	float	epsz;				/* Pibotting Factor */
	long	isw;				/* Control Code in SSL2 */
	long	is;					/* Control Code in SSL2 */
	float	vw[MAX_VAR];		/* Work Area in SSL2 */
	long	ip[MAX_VAR];		/* Work Area in SSL2 */
	long	index[6];
	long	icon;				/* Condition Code in SSL2 */
	long	time_ptr_offset;
	long	niter;				/* Iteration Counter */
	long	amp_cor_flag;

/*
---------------------------------------------- INITIAL SETTINGS
*/
	baseline_num= ant_num*(ant_num-1)/2;				/* Number of Baseline */
	afact_num	= 2*(baseline_num*ss_num + ant_num - 1);/* Number of Variable */
	if(afact_num > MAX_VAR){
		printf("MEMORY ERROR !!  Please Reduce Sub-Stream or Baseline.\n");
		return(-1);
	}
/*
---------------------------------------------- INITIAL PARAMETER
*/
	for(i=0; i<afact_num; i++){
		afact[i]	= *afact_ptr;
		afact_ptr++;
	}
	afact_ptr	-= afact_num;
/*
---------------------------------------------- DATA STORE
*/
	niter = 0;
	amp_cor_flag = 0;
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

			/*-------- LOOP FOR SUB-STREAM  --------*/
			for(ss_index=0; ss_index<ss_num; ss_index++){

				time_ptr_offset	= 0;
				vis_sigma	= 1.0e6*(*freq_incr_ptr)*time_incr;

				/*-------- INDEX in AFACT --------*/
				amp_index	= bl_index*ss_num + ss_index;
				phs_index	= amp_index + baseline_num*ss_num;
				delay_index1=  2*baseline_num*ss_num + ant1 - 1;
				delay_index2=  2*baseline_num*ss_num + ant2 - 1;
				rate_index1	=  delay_index1 + ant_num - 1;
				rate_index2	=  delay_index2 + ant_num - 1;

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
					for(spec_index=0; spec_index < *spec_num_ptr; spec_index++){
						d_omega	= PI2	* (*freq_incr_ptr)
										* (spec_index - *spec_num_ptr/2);
						omega	= PI2	* ((*freq_init_ptr)
										+ (*freq_incr_ptr) * spec_index);

						if(ant1 == 0){

							/*------- INCLUDE REF ANT --------*/
							phase	= afact[phs_index]
							+ d_omega*afact[delay_index2]
							+ omega*time*afact[rate_index2];
	
						} else{

							/*------- NOT INCLUDE REF ANT --------*/
							phase	= afact[phs_index]
							+d_omega*(afact[delay_index2]- afact[delay_index1])
							+omega*time*(afact[rate_index2]-afact[rate_index1]);
						}

						/*-------- TRIAL VISIBILITY --------*/
						trial_vr	= afact[amp_index]*cos( phase );
						trial_vi	= afact[amp_index]*sin( phase );

						/*-------- ACCESS POINTER to the VISIBILITY --------*/
						vr_ptr 	= *bl_vr_ptr_ptr
								+ (time_ptr_offset+ spec_index);
						vi_ptr 	= *bl_vi_ptr_ptr
								+ (time_ptr_offset+ spec_index);

						/*-------- RESIDUAL VECTOR --------*/
						rr	= *vr_ptr - trial_vr; 
						ri	= *vi_ptr - trial_vi; 

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
							for(j=0; j<6; j++){
								pwp[(index[i])][(index[j])] += vis_sigma * 
									( pr[(index[i])]*pr[(index[j])]
									+ pi[(index[i])]*pi[(index[j])] );
							}
							d[index[i]]	+= vis_sigma
								* ( pr[(index[i])] * rr + pi[(index[i])] * ri );
						}

					}	/*-------- END OF SPECTRAL LOOP --------*/

					time_ptr_offset	+= *spec_num_ptr;

				}	/*-------- END OF TIME LOOP --------*/

				spec_num_ptr++;
				freq_init_ptr++;
				freq_incr_ptr++;
	
				bl_vr_ptr_ptr++;
				bl_vi_ptr_ptr++;

			}	/*-------- END OF SUB-STREAM LOOP --------*/

			spec_num_ptr	-= ss_num;
			freq_init_ptr	-= ss_num;
			freq_incr_ptr	-= ss_num;

		}	/*-------- END OF BASELINE LOOP --------*/

		/*-------- SOLVE FOR PARTIAL MATRIX --------*/
		pdim = afact_num;	ndim = MAX_VAR;	epsz = 0.0;	isw = 1;
		alu_( pwp, &ndim, &pdim, &epsz, ip, &is, vw, &icon);
		if(icon != 0){
			printf(" ITERATION HALTED. ICON = %d\n", icon);
			return(icon);
		}
		isw = 1;
		lux_( d, pwp, &ndim, &pdim, &isw, ip, &icon);
		if(icon != 0){
			printf(" ITERATION HALTED. ICON = %d\n", icon);
			return(icon);
		}

		/*-------- ERROR ESTIMATION --------*/
		luiv_(pwp, &ndim, &pdim, ip, &icon);

		/*-------- FEEDBACK TO INITIAL PARAMETER --------*/
		cor = 0.0;

		if(amp_cor_flag == 0){

			/*-------- CORRECTION EXCEPT AMPLITUDE --------*/
			for(i=ss_num*baseline_num; i<afact_num; i++){
				afact[i]+= d[i];
				cor		+= d[i]*d[i];
			}
		} else {

			/*-------- CORRECTION INCLUDING AMPLITUDE --------*/
			for(i=0; i<afact_num; i++){
				afact[i]+= d[i];
				cor		+= d[i]*d[i];
			}
		}

		cor = sqrt(cor)/afact_num;
		residual/= (2*ss_num*(*spec_num_ptr)*time_num*baseline_num - afact_num);

		/*-------- IN CASE OF AMP < 0 --------*/
		for(ss_index=0; ss_index<ss_num; ss_index++){
			for(bl_index=0; bl_index<baseline_num; bl_index++){
				amp_index	= bl_index*ss_num + ss_index;
				phs_index	= amp_index + baseline_num*ss_num;
				if(afact[amp_index] < 0.0){
					afact[amp_index] *= -1;
					afact[phs_index] = (float)atan2(
						-sin((double)afact[phs_index]),
						-cos((double)afact[phs_index]));
				} else {
					afact[phs_index] = (float)atan2(
						sin((double)afact[phs_index]),
						cos((double)afact[phs_index]));
				}
			}
		}


		/*-------- CONVERGED ? --------*/
		if(cor > COR_LIMIT1){
			amp_cor_flag = 0;
			printf("ITERATION %2d [FIXED AMP]: CHI^2 = %6.3f \n",
				niter, residual);
		}
		if(cor < COR_LIMIT1){
			amp_cor_flag = 1;
			printf("ITERATION %2d [AMP FREE] : CHI^2 = %6.3f \n",
				niter, residual);
		}
		if(cor < COR_LIMIT2){ break; }

		niter++;
	}	/*-------- END OF ITERATION LOOP --------*/

	for(i=0; i<afact_num; i++){
		*afact_ptr		= afact[i];
		*afact_err_ptr	= sqrt(pwp[i][i]);
		afact_ptr++;
		afact_err_ptr++;
	}

	return(0);
}
