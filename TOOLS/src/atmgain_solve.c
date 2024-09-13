/*********************************************************
**	ATMGAIN_SOLVE.C: Solution for Atmospheric GAIN		**
**						at the Zenith					**
**														**
**	FUNCTION : Input Baseline-Based Data and Return		**
**				Antenna-Based Solution and Error		**
**	AUTHOR	: KAZUMASA Suzuki							**
**	CREATED	: 1996/11/8									**
**	COMMENT : This Module Needs SSL2 (Scientific Sub-	**
**				routine Library) Released by Fujitsu.	** 
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "obshead.inc"
#define RADDEG 57.29577951308232087721
#define K_BOLTZ 1380.0658
#define	T_ATM	285	
#define	MAX_ANT	10
#define	MAX_DATA	100
#define	MAX_PRM		3*MAX_ANT
#define	SNR_LIMIT	5
#define	ATM_FACT	stn_index+1
#define	CLK_FACT	stn_num+stn_index+1
#define	RTE_FACT	2*stn_num+stn_index+1
#define E_EPS 10e-3
int	atmgain_solve(obs_ptr, first_obj_ptr, gcal_ptr_ptr, first_stn_ptr,
			solint, t_rx_ptr, a_e_ptr, heikatu_ptr, xt_ptr, nt_ptr )

	struct	header		*obs_ptr;		/* Pointer of Obs Header			*/
	struct	head_obj	*first_obj_ptr;	/* Pointer of Obj Header			*/
	struct	gcal_data	**gcal_ptr_ptr;	/* Pointer of GCAL data				*/
	struct	head_stn	*first_stn_ptr;	/* Pointer of Station Header		*/
	double	solint;						/* solution intaval					*/
	double	*t_rx_ptr;					/* rx tempratura					*/
	double	*a_e_ptr;					/* antena_e							*/
	double	*heikatu_ptr;				/* Pointer of heikatuka_keisuu		*/
	double	*xt_ptr;					/* Pointer of fusiten_no_atai		*/
	int		*nt_ptr;					/* Pointer of fusiten_no_kazu		*/
{
	struct	head_obj	*obj_ptr;		/* Initial Pointer of Obj Header	*/
	struct	gcal_data	**first_gcal_ptr;/*Pointer of GCAL Data				*/
	struct	gcal_data	*gcal_ptr;		/*Pointer of GCAL Data				*/
	struct	head_stn	*stn_ptr;		/* Initial Pointer of STN Header	*/
	double	radius,	radius_ref;			/* Earth Radius						*/ 
	double	sin_phi, sin_phi_ref;		/* sin(latitude)					*/
	double	cos_phi, cos_phi_ref;		/* cos(latitude)					*/
	double	lambda, lambda_ref;			/* Longitude						*/
	double	sin_el, sin_el_ref;			/* sin(EL)							*/
	double	dsecz, dsecz_ref;			/* d[secz]/dt						*/
	double	gmst;						/* Greenwich Mean Sidereal Time		*/
	int		stn_index;					/* Index for Station				*/
	int		stn_num;					/* Total Number of Stations			*/
	int		data_index;					/* Index for Delay Data				*/
	double	wgt_dl;						/* Weight							*/
	double	wgt_rt;						/* Weight							*/
	long	icon;						/* CONDITION CODE */
	double	vw[MAX_DATA];				/* WORK AREA */
	int		i, j, k;						/* General Index */
	double	weight[MAX_DATA];
	double	weight1[MAX_DATA];
	double	time[MAX_DATA];
	double	time1[MAX_DATA];
	double	sefd[MAX_DATA];
	double	sefd1[MAX_DATA];
	double	sefd_sum[MAX_DATA];
	double	sefd_max[MAX_DATA];
	double	sefd_min;
	double	secz_1;
	double	sefd_tau;
	double	tau_0[MAX_DATA];
	int     data1_num[MAX_DATA];
	int     data2_num[MAX_DATA];
	int     data_num;

/* SPLINE HOKAN */
	double  c1[MAX_DATA];
	double  xt[MAX_DATA];
	double  r[MAX_DATA];
	double  rnor;
	int     ivw[MAX_DATA];
	int     m;
	int     nt;

/*
----------------------------------------------- READ GAIN DATA
*/
	icon		= 0;
	stn_index	= 0;
	stn_ptr		= first_stn_ptr;

	/*-------- LOOP FOR STATION --------*/
	while( gcal_ptr_ptr[stn_index] != NULL){

		gcal_ptr	= gcal_ptr_ptr[stn_index];
		radius	= sqrt( stn_ptr->stn_pos[0]*stn_ptr->stn_pos[0]
					+	stn_ptr->stn_pos[1]*stn_ptr->stn_pos[1]
					+	stn_ptr->stn_pos[2]*stn_ptr->stn_pos[2]);
		sin_phi		= stn_ptr->stn_pos[2] / radius;
		cos_phi		= sqrt(1.0 - sin_phi*sin_phi);
		lambda		= atan2(stn_ptr->stn_pos[1]/(radius*cos_phi),
							stn_ptr->stn_pos[0]/(radius*cos_phi) );

		/*-------- CALC SEFD_MIN[j] in Groupe[j] --------*/
		gcal_ptr	= gcal_ptr_ptr[stn_index];
		i=0;
		j=0;
		data_num=1;
		while( gcal_ptr != NULL ){
			time[i] = gcal_ptr->mjd;
			sefd[i] = gcal_ptr->real;
			weight[i] = gcal_ptr->weight;
			if( time[0]+solint >= time[i] ){
		        sefd_min    =  1.0e36;
           		if( sefd_min > sefd[i] ){
					sefd_min	= sefd[i];	
               		sefd_max[j]	= sefd[i] + SNR_LIMIT * weight[i];
					weight1[j]	= 1.0/(weight[i]*weight[i]);
				}	
				i++;
				data1_num[j] = i;
				gcal_ptr = gcal_ptr->next_gcal_ptr;
	
			}else{
				i=0;
				j++;
				data_num++;
				gcal_ptr = gcal_ptr->next_gcal_ptr;
			}
		}

		/*-------- CALC SEFD in Groupe[j] --------*/
		gcal_ptr	= gcal_ptr_ptr[stn_index];
		sefd_sum[0] = 0;
		i=0;
		j=0;
		k=0;
		while( gcal_ptr != NULL ){
			time[i] = gcal_ptr->mjd;
			sefd[i] = gcal_ptr->real;
			weight[i] = gcal_ptr->weight;
			if( time[0]+solint >= time[i] ){
				if( sefd_max[j] > sefd[i] ){
  	            	k ++;
   		        	sefd_sum[j]	+= sefd[i];
				}
       			data2_num[j] = k; 
				i++;
				gcal_ptr = gcal_ptr->next_gcal_ptr;
	
			}else{
				time1[j] = time[ data1_num[j] / 2 ]; 	
				i=0;
				k=0;
				j++;
				sefd_sum[j] = 0;
				gcal_ptr = gcal_ptr->next_gcal_ptr;
			}
		}
		time1[j] = time[ data1_num[j] / 2 ]; 	

		for(j=0; j<data_num; j++){
			sefd1[j] = sefd_sum[j] / data2_num[j];
		}

		/*-------- SEARCH CURRENT OBJECT from OBJECT LIST --------*/
		obj_ptr = first_obj_ptr;
		gcal_ptr	= gcal_ptr_ptr[stn_index];
		while( obj_ptr != NULL ){
			if(strstr(gcal_ptr->objnam, obj_ptr->obj_name) != NULL){
				break;
			}
			obj_ptr = obj_ptr->next_obj_ptr;
		}

		/*-------- CALCULATE TAU_0 --------*/
		for(j=0; j<data_num; j++){
			mjd2gmst( time1[j], obs_ptr->ut1utc, &gmst);
			gst2el( gmst, -lambda, atan2(sin_phi, cos_phi),
				obj_ptr->obj_pos[0]/RADDEG,
				obj_ptr->obj_pos[1]/RADDEG,
				&sin_el );
	
			secz_1	= 1.0/(sin_el);
			sefd_tau	= ( sefd1[j] * a_e_ptr[ stn_index ] )
							/ (2.0 * K_BOLTZ *( t_rx_ptr[ stn_index ] + T_ATM ))
							+ T_ATM / ( t_rx_ptr[ stn_index ] + T_ATM );

			tau_0[j]	= log( sefd_tau ) / secz_1;
		}	 

		/*-------- SPLINE HOKAN --------*/
		m = 3;  nt  = data_num/4 + 2;

		/*-------- SPLINE HOKAN --------*/
    	xt[0] = (double)time1[0]*0.8;
    	for(i=1; i<(nt-1); i++){
        	xt[i] = time1[4*i];
    	}
    	xt[nt-1] = (double)time1[data_num]*1.2;

    	dbsc1_( time1, tau_0, weight1, &data_num, &m, xt, &nt,
        	    c1, r, &rnor, vw, ivw, &icon);
	
		nt_ptr[ stn_index ]	= nt;

		for(i=0; i<nt; i++){
			xt_ptr[ stn_index * nt + i ]		= xt[i];
		}
		for(i=0; i<(nt+2); i++){
			heikatu_ptr[ stn_index * (nt+2) + i ]		= c1[i];
		}

		stn_index++;
		stn_ptr = stn_ptr->next_stn_ptr;
	}	/*-------- End of Station Loop --------*/


	return(icon);
}
