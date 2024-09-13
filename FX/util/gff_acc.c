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
#define		NSPEC	16
#define		NTIME	256	
#define		PI		3.14159265358979323846
#define		PI2		6.28318530717958647688
#define		MAX_ANT	20
#define		MAX_BL	MAX_ANT*(MAX_ANT-1)/2
#define		MAX_VAR (MAX_ANT+3)*(MAX_ANT-1)	

long	gff_acc(vr_ptr, vi_ptr,	spec_num, spec_dim, freq_init, freq_incr,
			time_num, time_dim, time_incr, ant_num, bl_weight_ptr,
			vis_amp_ptr, vis_phs_ptr, ant_delay_ptr, ant_rate_ptr, ant_acc_ptr)

	float	*vr_ptr;		/* INPUT : Pointer of Visibility (Real) Data */	 
	float	*vi_ptr;		/* INPUT : Pointer of Visibility (Real) Data */
	long	spec_num;		/* INPUT : Number of Spectrum */
	long	spec_dim;		/* INPUT : Dimension Number for Spectrum */
	double	freq_init;		/* INPUT : Initial Frequency [MHz] */
	double	freq_incr;		/* INPUT : Frequency Increment [MHz] */
	long	time_num;		/* INPUT : Number of Time */
	long	time_dim;		/* INPUT : Dimention Number for Time */
	double	time_incr;		/* INPUT : Time Increment [sec] */
	long	ant_num;		/* INPUT : Number of Antenna */
	double	*bl_weight_ptr;/* INPUT : Pointer of Weight for Antenna */
	double	*vis_amp_ptr;	/* OUTPUT: Pointer of Visibility amp */
	double	*vis_phs_ptr;	/* OUTPUT: Pointer of Visibility phase [rad] */
	double	*ant_delay_ptr;	/* OUTPUT: Pointer of Delay [microsec] */
	double	*ant_rate_ptr;	/* OUTPUT: Pointer of Delay Rate [picosec/sec] */
	double	*ant_acc_ptr;	/* OUTPUT: Pointer of Delay Acc. [femtsec/sec/sec]*/
{
	long	i, j, k;
	long	baseline_num;	/* Baseline Number */
	long	ibaseline;		/* Baseline Index */
	long	itime;			/* Time Index */
	long	ispec;			/* Spectrum Index */
	long	ndim;				/* Number of Dimension of the Data */
	float	afact[MAX_VAR];		/* Variable to be Determined */
	long	afact_num;			/* Number of Variable to be Determined */
	long	ant1, ant2;			/* Antenna Index for Each Baseline */
	long	amp_index;			/* Amplitude Index in afact[] */
	long	phs_index;			/* Phase Index in afact[] */
	long	delay_index1;		/* Delay Index in afact[] */
	long	delay_index2;		/* Delay Index in afact[] */
	long	rate_index1;		/* Delay Rate Index in afact[] */
	long	rate_index2;		/* Delay Rate Index in afact[] */
	long	acc_index1;			/* Delay Acceleration Rate Index in afact[] */
	long	acc_index2;			/* Delay Acceleration Rate Index in afact[] */
	float	pr[MAX_VAR][MAX_BL];/* Partial Matrix */
	float	pi[MAX_VAR][MAX_BL];/* Partial Matrix */
	float	pwp[MAX_VAR][MAX_VAR];		/* Weighted Partial Matrix */
	float	bl_weight[MAX_BL];	/* Baseline Visibility Weight */
	float	rr[MAX_BL];			/* Residual Vector */
	float	ri[MAX_BL];			/* Residual Vector */
	float	d[MAX_VAR];			/* Correction Vector */
	float	cor;				/* RSS of the Correction Vector */
	float	phase;				/* (trial) Visibility Phase */
	float	omega;				/* Angular Frequency */
	float	d_omega;			/* Angular Frequency in Video Band */
	float	time;				/* Time from the start [sec] */
	float	trial_vr;			/* Trial Visibility (real) */
	float	trial_vi;			/* Trial Visibility (imag) */
	long	datanum;			/* Number of Data */ 
	long	pdim;				/* Dimension of the Partial Matrix */
	float	epsz;				/* Pibotting Factor */
	long	isw;				/* Control Code in SSL2 */
	long	is;					/* Control Code in SSL2 */
	float	vw[MAX_VAR];		/* Work Area in SSL2 */
	long	ip[MAX_VAR];		/* Work Area in SSL2 */
	long	icon;				/* Condition Code in SSL2 */
	float	*init_vr_ptr;
	float	*init_vi_ptr;
	long	niter;				/* Iteration Counter */

/*
---------------------------------------------- INITIAL SETTINGS
*/
	baseline_num	= ant_num*(ant_num-1)/2;		/* Number of Baseline */
	afact_num		= (ant_num + 3)*(ant_num - 1);	/* Number of Variable */
	init_vr_ptr	= vr_ptr;
	init_vi_ptr	= vi_ptr;
	for(ibaseline=0; ibaseline<baseline_num; ibaseline++){
		bl_weight[ibaseline] = *bl_weight_ptr;
		bl_weight_ptr++;
	}
/*
---------------------------------------------- INITIAL PARAMETER
*/
	for(i=0; i<baseline_num; i++){
		afact[i]				= (float)(*vis_amp_ptr);
		afact[baseline_num + i]	= (float)(*vis_phs_ptr);
		vis_amp_ptr++;
		vis_phs_ptr++;
	}
	vis_amp_ptr	-= baseline_num;
	vis_phs_ptr	-= baseline_num;

	for(i=0; i<ant_num; i++){
		if(i != 0){
			afact[ant_num*(ant_num - 1) + i - 1]= (float)(*ant_delay_ptr);
			afact[ant_num*ant_num + i - 2]		= 1.0e-6*(float)(*ant_rate_ptr);
			afact[ant_num*(ant_num + 1) + i - 3]= 1.0e-9*(float)(*ant_acc_ptr);
		}
		ant_delay_ptr++;
		ant_rate_ptr++;
		ant_acc_ptr++;
	}
	ant_delay_ptr	-= ant_num;
	ant_rate_ptr	-= ant_num;
	ant_acc_ptr		-= ant_num;
/*
---------------------------------------------- DATA STORE
*/
	niter = 0;
	/*-------- ITERATION LOOP --------*/
	while(1){

		/*-------- INITIALIZE PARTIAL MATRIX and RESIDUAL VECTOR --------*/
		for(i=0; i<afact_num; i++){
			for(j=0; j<afact_num; j++){ pwp[i][j] = 0.0; }
			d[i] = 0.0;
		}

		/*-------- LOOP FOR SPECTRAL POINTS --------*/
		for(itime=0; itime<time_num; itime++){
			/* time	= time_incr*itime*1.0e6; */	/* TIME in MICROSEC */
			time	= time_incr*itime;		/* TIME in SEC */

			/*-------- LOOP FOR SPECTRAL POINTS --------*/
			for(ispec=0; ispec<spec_num; ispec++){

				vr_ptr	= init_vr_ptr + itime*spec_dim + ispec;
				vi_ptr	= init_vi_ptr + itime*spec_dim + ispec;

				d_omega	= PI2 * freq_incr * ispec;
				omega	= PI2 * (freq_init + freq_incr * ispec);

				/*-------- INITIALIZE PARTIAL MATRIX --------*/
				for(i=0; i<afact_num; i++){
					for(k=0; k<baseline_num; k++){
						pr[i][k] = 0.0;
						pi[i][k] = 0.0;
					}
				}

				/*-------- LOOP FOR BASELINE --------*/
				for(ibaseline=0; ibaseline<baseline_num; ibaseline++){
					bl2ant(ibaseline, &ant2, &ant1);	/* GET ANTENNA NUMBER */

					/*-------- INDEX in AFACT --------*/
					amp_index	= ibaseline;
					phs_index	= baseline_num + ibaseline;
					delay_index1=  2*baseline_num + ant1 - 1;
					delay_index2=  2*baseline_num + ant2 - 1;
					rate_index1	=  delay_index1 + ant_num - 1;
					rate_index2	=  delay_index2 + ant_num - 1;
					acc_index1	=  rate_index1 + ant_num - 1;
					acc_index2	=  rate_index2 + ant_num - 1;

					#ifdef DEBUG
					printf("BL_index     = %d\n",ibaseline);
					printf("amp_index    = %d\n",amp_index);
					printf("phs_index    = %d\n",phs_index);
					printf("delay_index1 = %d\n",delay_index1);
					printf("delay_index2 = %d\n",delay_index2);
					printf("rate_index1  = %d\n",rate_index1);
					printf("rate_index2  = %d\n",rate_index2);
					printf("acc_index1   = %d\n",acc_index1);
					printf("acc_index2   = %d\n",acc_index2);
					#endif


					if(ant1 == 0){

						/*------- INCLUDE REF ANT --------*/
						phase	= afact[phs_index]
						+ d_omega*afact[delay_index2]
						+ omega*time*afact[rate_index2]
						+ 0.5*omega*time*time*afact[acc_index2];

					} else{

						/*------- NOT INCLUDE REF ANT --------*/
						phase	= afact[phs_index]
				+ d_omega*(afact[delay_index2] - afact[delay_index1])
				+ omega*time*(afact[rate_index2] - afact[rate_index1])
				+ 0.5*omega*time*time*(afact[acc_index2] - afact[acc_index1]);
					}

					/*-------- TRIAL VISIBILITY --------*/
					trial_vr	= afact[amp_index]*cos( phase );
					trial_vi	= afact[amp_index]*sin( phase );

					/*-------- RESIDUAL VECTOR --------*/
					rr[ibaseline]	= *vr_ptr - trial_vr; 
					vr_ptr	+= time_dim*spec_dim;
					ri[ibaseline]	= *vi_ptr - trial_vi; 
					vi_ptr	+= time_dim*spec_dim;

					/*-------- PARTIAL MATRIX COMPONENTS --------*/
					pr[amp_index][ibaseline]	= trial_vr/afact[amp_index]; 
					pi[amp_index][ibaseline]	= trial_vi/afact[amp_index]; 
					pr[phs_index][ibaseline]	= -trial_vi; 
					pi[phs_index][ibaseline]	=  trial_vr; 
					pr[delay_index2][ibaseline]	= -d_omega*trial_vi; 
					pi[delay_index2][ibaseline]	=  d_omega*trial_vr; 
					pr[rate_index2][ibaseline]	= -omega*time*trial_vi; 
					pi[rate_index2][ibaseline]	=  omega*time*trial_vr; 
					pr[acc_index2][ibaseline]	=-0.5*omega*time*time*trial_vi; 
					pi[acc_index2][ibaseline]	= 0.5*omega*time*time*trial_vr; 

					if(ant1 != 0){
						/*------- NOT INCLUDE REF ANT --------*/
						pr[delay_index1][ibaseline]	=  d_omega*trial_vi; 
						pi[delay_index1][ibaseline]	= -d_omega*trial_vr; 
						pr[rate_index1][ibaseline]	=  omega*time*trial_vi; 
						pi[rate_index1][ibaseline]	= -omega*time*trial_vr; 
				pr[acc_index1][ibaseline]	= 0.5*omega*time*time*trial_vi; 
				pi[acc_index1][ibaseline]	=-0.5*omega*time*time*trial_vr; 
					}

				}	/* END OF BASELINE LOOP */

				/*-------- MAKING WEIGHTED PARTIAL MATRIX --------*/
				for(i=0; i<afact_num; i++){
					for(j=0; j<afact_num; j++){
						for(k=0; k<baseline_num; k++){
							pwp[i][j]	= pwp[i][j] + bl_weight[k]
										*(pr[i][k]*pr[j][k] + pi[i][k]*pi[j][k]);
						}
					}

					/*-------- MAKING CORRECTION VECTOR --------*/
					for(k=0; k<baseline_num; k++){
						d[i]	= d[i] + bl_weight[k]
								* (pr[i][k]*rr[k] + pi[i][k]*ri[k]);
					}
				}
			}	/* END OF SPEC LOOP */
		}	/* END OF TIME LOOP */

		/*-------- SOLUTION FOR CORRECTION VECTOR --------*/
		pdim=afact_num;	ndim=MAX_VAR; epsz=0.0; isw=1;
		lax_( pwp, &ndim, &pdim, d, &epsz, &isw, &is, vw, ip, &icon);
		if(icon != 0){
			#ifdef DEBUG
			printf(" ITERATION HALTED. ICON=%d.\n", icon);
			#endif
			return(-1);
		}

		/*-------- CORRECTION FOR AFACT --------*/
		cor = 0.0;
		for(i=0; i<afact_num; i++){
			afact[i] = afact[i] + d[i];
			cor = cor + d[i] * d[i];
		}
		cor = sqrt(cor)/afact_num;

		for(i=0; i<baseline_num; i++){
			if(afact[i] < 0.0 ){
				afact[i]				= -afact[i];
				afact[baseline_num + i]	= afact[baseline_num + i] - PI;
			}
			afact[baseline_num + i]	= (float)atan2(
				sin((double)afact[baseline_num + i]),
				cos((double)afact[baseline_num + i]));
		}

		#ifdef DEBUG
		for(i=0; i<afact_num; i++){ printf(" %7.1e", afact[i]); }
		printf("\n");
		#endif

		/*-------- CONVERGED OR NOT --------*/
		if( (cor < 1.0e-4) || (niter > 10) ){ break;}
		niter++;
	}

	for(i=0; i<baseline_num; i++){
		*vis_amp_ptr	= (double)afact[i];
		*vis_phs_ptr	= (double)afact[baseline_num + i];
			vis_amp_ptr++;
			vis_phs_ptr++;
	}

	for(i=0; i<ant_num; i++){
		if(i == 0){
			*ant_delay_ptr	= 0.0;
			*ant_rate_ptr	= 0.0;
			*ant_acc_ptr	= 0.0;
				ant_delay_ptr++;
				ant_rate_ptr++;
				ant_acc_ptr++;
		} else {
			*ant_delay_ptr	= (double)afact[ant_num*(ant_num - 1) + i - 1];
			*ant_rate_ptr	= 1.0e6*(double)afact[ant_num*ant_num  + i - 2];
			*ant_acc_ptr	= 1.0e9*(double)afact[ant_num*(ant_num + 1) + i - 3];
				ant_delay_ptr++;
				ant_rate_ptr++;
				ant_acc_ptr++;
		}
	}

	return(0);
}
