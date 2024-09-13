/*********************************************************
**	ATMGAIN_GET.C: Solution for Atmospheric GAIN		**
**							at the Zenith				**
**														**
**	FUNCTION : Input Baseline-Based Data and Return		**
**				Antenna-Based Solution and Error		**
**	AUTHOR	: KAZUMASA Suzuki							**
**	CREATED	: 1996/11/8									**
**	COMMENT : This Module Needs SSL2 (Scientific Sub-	**
**	routine Library) Released by Fujitsu.				** 
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
#define	MAX_PRM	3*MAX_ANT
#define	ATM_FACT	stn_index+1
#define	CLK_FACT	stn_num+stn_index+1
#define	RTE_FACT	2*stn_num+stn_index+1
#define E_EPS 10e-3

int	atmgain_get(obs_ptr, first_obj_ptr, first_stn_ptr,
			t_rx_ptr, a_e_ptr, heikatu_ptr, xt_ptr, nt_ptr,
			target_time, src_id, sefd_ptr )

	struct	header		*obs_ptr;		/* Pointer of Obs Header			*/
	struct	head_obj	*first_obj_ptr;	/* Pointer of Obj Header			*/
	struct	head_stn	*first_stn_ptr;	/* Pointer of Station Header		*/
    double  *t_rx_ptr;                  /* rx tempratura                    */
    double  *a_e_ptr;                   /* antena_e                         */
	double	*heikatu_ptr;				/* Pointer of heikatuka_keisuu		*/
	double	*xt_ptr;					/* Pointer of fusiten_no_atai		*/
	double	*nt_ptr;					/* Pointer of fusiten_no_kazu		*/
	double	target_time;				/* time								*/
	int		src_id;						/* Source Number from OBJ File		*/
	double	*sefd_ptr;					/* Pointer of sefd					*/
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
	int		i, j, k;					/* General Index */
	double	sefd;
	double	secz;
	double	tau_fit;

/* SPLINE HOKAN */

	double  xt[MAX_DATA];
	double  c1[MAX_DATA];
	double  r[MAX_DATA];
	int     iv;
	int     isw;
	int     m;
	int     nt;

/*
	printf("TARGET MJD = %lf\n", target_time);
----------------------------------------------- READ GAIN DATA
*/
	icon		= 0;
	stn_index	= 0;
	stn_ptr		= first_stn_ptr;
	j=0;	
	k=0;	

	/*-------- LOOP FOR STATION --------*/
	while( stn_ptr != NULL){
		radius	= sqrt( stn_ptr->stn_pos[0]*stn_ptr->stn_pos[0]
					+	stn_ptr->stn_pos[1]*stn_ptr->stn_pos[1]
					+	stn_ptr->stn_pos[2]*stn_ptr->stn_pos[2]);
		sin_phi		= stn_ptr->stn_pos[2] / radius;
		cos_phi		= sqrt(1.0 - sin_phi*sin_phi);
		lambda		= atan2(stn_ptr->stn_pos[1]/(radius*cos_phi),
							stn_ptr->stn_pos[0]/(radius*cos_phi) );

		/*-------- SPLINE HOKAN --------*/
		m = 3;
   	   	isw = 0;    iv  = 0;
		nt	= nt_ptr[ stn_index ];
		for(i=0; i<(nt+2); i++){
			c1[i] = heikatu_ptr[j]; 
			j++;
		}	

		for(i=0; i<nt; i++){
			xt[i] = xt_ptr[k]; 
			k++;
		}	

   	   	dbsf1_( &m, xt, &nt, c1, &isw, &target_time, &iv,
           		             &tau_fit, vw, &icon );

/*
		printf("TAU_0 = %lf\n", tau_fit);
*/

		/*-------- SEARCH CURRENT OBJECT from OBJECT LIST --------*/
		obj_ptr = first_obj_ptr;
		while( obj_ptr != NULL ){
			if( obj_ptr->obj_index == src_id ){
				break;
			}
			obj_ptr = obj_ptr->next_obj_ptr;
		}
			
		/*-------- CALCULATE SEFD --------*/
		mjd2gmst( target_time, obs_ptr->ut1utc, &gmst);
		gst2el( gmst, -lambda, atan2(sin_phi, cos_phi),
			obj_ptr->obj_pos[0]/RADDEG,
			obj_ptr->obj_pos[1]/RADDEG,
			&sin_el );

		secz	= 1.0/(sin_el);

       	sefd  = ( (2.0*K_BOLTZ)*(t_rx_ptr[ stn_index ] + T_ATM)
												/a_e_ptr[ stn_index ] )
       		             *exp(tau_fit*secz)
               	        - (2.0*K_BOLTZ)*T_ATM/a_e_ptr[ stn_index ];

	
		sefd_ptr[ stn_index ]		= sefd;

		stn_index++;
		stn_ptr = stn_ptr->next_stn_ptr;
	}	/*-------- End of Station Loop --------*/


	return(icon);
}
