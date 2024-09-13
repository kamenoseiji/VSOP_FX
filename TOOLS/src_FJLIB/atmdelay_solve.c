/*********************************************************
**	ATMDELAY_SOLVE.C: Solution for Atmospheric Delay 	**
**						at the Zenith					**
**														**
**	FUNCTION : Input Baseline-Based Data and Returns 	**
**				Antenna-Based Solution and Error		**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/2/8									**
**	COMMENT : This Module Needs SSL2 (Scientific Sub-	**
**				routine Library) Released by Fujitsu.	** 
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "obshead.inc"
#define RADDEG 57.29577951308232087721
#define	MAX_ANT	10
#define	MAX_PRM	3*MAX_ANT+1
#define	ATM_FACT	stn_index+1
#define	CLK_FACT	stn_num+stn_index+1
#define	RTE_FACT	2*stn_num+stn_index+1

int	atmdelay_solve(obs_ptr, obj_ptr, refant_id, fcal_ptr_ptr, stn_ptr,
			atm_ptr )

	struct	header		*obs_ptr;		/* Pointer of Obs Header */
	struct	head_obj	*obj_ptr;		/* Pointer of Obj Header */
	int		refant_id;					/* Refant ID in CFS */
	struct	fcal_data	**fcal_ptr_ptr;	/* Pointer of FCAL data */
	struct	head_stn	*stn_ptr;		/* Pointer of Station Header */
	double	*atm_ptr;					/* Pointer of Atmosperic Parameters */
{
	struct	head_obj	*first_obj_ptr;	/* Initial Pointer of Obj Header */
	struct	fcal_data	**first_fcal_ptr;/*Pointer of FCAL Data */
	struct	fcal_data	*fcal_ptr;		/*Pointer of FCAL Data */
	struct	head_stn	*first_stn_ptr;	/* Initial Pointer of STN Header */
	double	radius,	radius_ref;			/* Earth Radius */ 
	double	sin_phi, sin_phi_ref;		/* sin(latitude) */
	double	cos_phi, cos_phi_ref;		/* cos(latitude) */
	double	lambda, lambda_ref;			/* Longitude */
	double	sin_el, sin_el_ref;			/* sin(EL) */
	double	dsecz, dsecz_ref;			/* d[secz]/dt */
	double	gmst;						/* Greenwich Mean Sidereal Time */
	int		stn_index;					/* Index for Station*/
	int		stn_num;					/* Total Number of Stations */
	int		data_index;					/* Index for Delay Data */
	double	wgt_dl;						/* Weight */
	double	wgt_rt;						/* Weight */
	double	p[MAX_PRM][MAX_PRM];		/* PARTIAL MATRIX */
	long	icon;						/* CONDITION CODE */
	int		max_prm;					/* Max Number of PRM */
	int		prm_num;					/* Number of PRM */
	double	epsz;						/* PIVOT CRITICAL VALUE */
	double	vw[MAX_PRM];				/* WORK AREA */
	int		ip[MAX_PRM];				/* WORK AREA */
	int		is;							/* Factor for Determinant */
	int		isw;						/* Control Code */
	int		i, j;						/* General Index */

/*
----------------------------------------------- INIT
*/
	max_prm	= MAX_PRM;
	for(i=0; i<MAX_PRM; i++){
		for(j=0; j<MAX_PRM; j++){
			p[i][j] = 0.0;
		}
		atm_ptr[i] = 0.0;
	}
/*
----------------------------------------------- WEIGHT
*/
	first_obj_ptr	= obj_ptr;
	first_fcal_ptr	= fcal_ptr_ptr;
	first_stn_ptr	= stn_ptr;
/*
----------------------------------------------- SEARCH FOR REFANT
*/
	stn_index = 0;
	stn_ptr	= first_stn_ptr;
	while( stn_ptr != NULL){
		if( stn_ptr->stn_index == refant_id){

			printf("[ATMDLEAY] REFANT = %s\n", stn_ptr->stn_name );

			radius_ref	= sqrt( stn_ptr->stn_pos[0]*stn_ptr->stn_pos[0]
							+	stn_ptr->stn_pos[1]*stn_ptr->stn_pos[1]
							+	stn_ptr->stn_pos[2]*stn_ptr->stn_pos[2] );
			sin_phi_ref	= stn_ptr->stn_pos[2] / radius_ref;
			cos_phi_ref	= sqrt( 1.0 - sin_phi_ref*sin_phi_ref);
			lambda_ref	= atan2(stn_ptr->stn_pos[1]/(radius_ref*cos_phi_ref),
								stn_ptr->stn_pos[0]/(radius_ref*cos_phi_ref));
		} else if(stn_ptr->acorr_index != -1){
			stn_index++;
		}
		stn_ptr = stn_ptr->next_stn_ptr;
	}
	stn_num	= stn_index;
	prm_num	= 3*stn_num + 1;
/*
----------------------------------------------- SEARCH FOR REFANT
*/
	stn_index	= 0;
	data_index	= 0;
	stn_ptr	= first_stn_ptr;

	/*-------- LOOP FOR STATION --------*/
	while( *fcal_ptr_ptr != NULL){

		/*-------- AVOID REFANT --------*/
		if( stn_ptr->stn_index == refant_id){
			stn_ptr = stn_ptr->next_stn_ptr;
		}

		/*-------- STATION POSITION in SPHERICAL COORDINATE --------*/
		radius	 = sqrt( stn_ptr->stn_pos[0]*stn_ptr->stn_pos[0]
					+	stn_ptr->stn_pos[1]*stn_ptr->stn_pos[1]
					+	stn_ptr->stn_pos[2]*stn_ptr->stn_pos[2] );
		sin_phi = stn_ptr->stn_pos[2] / radius;
		cos_phi = sqrt(1.0 - sin_phi*sin_phi);
		lambda  = atan2(stn_ptr->stn_pos[1]/(radius*cos_phi),
		stn_ptr->stn_pos[0]/(radius*cos_phi) );

		/*-------- LOOP FOR TIME in FCAL DATA --------*/
		fcal_ptr	= *fcal_ptr_ptr;
		while( fcal_ptr != NULL ){

			/*-------- SEARCE FOR CURRENT OBJ in OBJ LIST --------*/
			obj_ptr	= first_obj_ptr;
			while(obj_ptr != NULL){

				if(strstr(fcal_ptr->objnam, obj_ptr->obj_name) != NULL){
					break;
				}
				obj_ptr = obj_ptr->next_obj_ptr;
			}

			/*-------- CALC. CURRENT ELEVATION --------*/
			mjd2gmst(fcal_ptr->mjd, obs_ptr->ut1utc, &gmst);

			gst2el(gmst, -lambda_ref, atan2(sin_phi_ref, cos_phi_ref),
				obj_ptr->obj_pos[0]/RADDEG,
				obj_ptr->obj_pos[1]/RADDEG,
				&sin_el_ref );

			gst2el(gmst, -lambda, atan2(sin_phi, cos_phi),
				obj_ptr->obj_pos[0]/RADDEG,
				obj_ptr->obj_pos[1]/RADDEG,
				&sin_el );

			gst2dsecz( obs_ptr->degpdy, gmst, -lambda_ref, 
				atan2(sin_phi_ref, cos_phi_ref),
				obj_ptr->obj_pos[0]/RADDEG,
				obj_ptr->obj_pos[1]/RADDEG,
				sin_el_ref,
				&dsecz_ref );

/*
			printf("LONG = %lf  \n", 57.29578*lambda);
			printf("LATI = %lf  \n", 57.29578*atan2(sin_phi, cos_phi) );
			printf("EL   = %lf  \n", 57.29578*asin(sin_el));
			printf("d SECZ / dt = %lf  \n", dsecz_ref);
*/

			gst2dsecz( obs_ptr->degpdy, gmst, -lambda, 
				atan2(sin_phi, cos_phi),
				obj_ptr->obj_pos[0]/RADDEG,
				obj_ptr->obj_pos[1]/RADDEG,
				sin_el,
				&dsecz );

			if(fcal_ptr->rate_err > 0.0){
				wgt_rt	= 1.0/(fcal_ptr->rate_err * fcal_ptr->rate_err);
			} else { wgt_rt	= 0.0; }

			if(fcal_ptr->delay_err > 0.0){
				wgt_dl	= 1.0/(fcal_ptr->delay_err * fcal_ptr->delay_err);
			} else { wgt_dl	= 0.0; }

			/*-------- PARTIAL MATRIX --------*/
			p[0][0]					+= wgt_dl/(sin_el_ref*sin_el_ref);
			p[0][0]					+= wgt_rt*dsecz_ref*dsecz_ref;
			p[0][ATM_FACT]			-= wgt_dl/(sin_el*sin_el_ref);
			p[0][ATM_FACT]			-= wgt_rt*dsecz_ref*dsecz;
			p[0][CLK_FACT]			-= wgt_dl/(sin_el_ref);
			p[0][RTE_FACT]			-= wgt_rt*dsecz_ref;

			p[ATM_FACT][0]			-= wgt_dl/(sin_el*sin_el_ref);
			p[ATM_FACT][0]			-= wgt_rt*dsecz_ref*dsecz;
			p[ATM_FACT][ATM_FACT]	+= wgt_dl/(sin_el*sin_el);
			p[ATM_FACT][ATM_FACT]	+= wgt_rt*dsecz*dsecz;
			p[ATM_FACT][CLK_FACT]	+= wgt_dl/sin_el;
			p[ATM_FACT][RTE_FACT]	+= wgt_rt*dsecz;

			p[CLK_FACT][0]			-= wgt_dl/(sin_el_ref);
			p[CLK_FACT][ATM_FACT]	+= wgt_dl/sin_el;
			p[CLK_FACT][CLK_FACT]	+= wgt_dl;

			p[RTE_FACT][0]			-= wgt_rt*dsecz_ref;
			p[RTE_FACT][ATM_FACT]	+= wgt_rt*dsecz;
			p[RTE_FACT][RTE_FACT]	+= wgt_rt;


			/*-------- RATE VECTOR --------*/
			atm_ptr[0]				-= wgt_dl/sin_el_ref * fcal_ptr->delay;
			atm_ptr[0]				-= wgt_rt*dsecz_ref * fcal_ptr->rate;

			atm_ptr[ATM_FACT]		+= wgt_dl/sin_el * fcal_ptr->delay;
			atm_ptr[ATM_FACT]		+= wgt_rt*dsecz * fcal_ptr->rate;

			atm_ptr[CLK_FACT]		+= wgt_dl*fcal_ptr->delay;
			atm_ptr[RTE_FACT]		+= wgt_rt*fcal_ptr->rate;

			data_index++;
			fcal_ptr = fcal_ptr->next_fcal_ptr;
		}	/*-------- End of Time Loop in FCAL DATA --------*/
		fcal_ptr_ptr++;
		stn_index++;
		stn_ptr	= stn_ptr->next_stn_ptr;
	}	/*-------- End of Station Loop --------*/
/*
----------------------------------------------- SOLVE FOR MATRIX
*/
	epsz = 0.0;	isw	= 1;
	dlax_( p, &max_prm, &prm_num, atm_ptr, &epsz, &isw, &is, vw, ip, &icon );

	#ifdef DEBUG
	#endif
	for(i=0; i<prm_num; i++){
		printf("PARAM [%d] = %e\n", i, atm_ptr[i]);
	}
	#ifdef DEBUG
	#endif

	obj_ptr	= first_obj_ptr;
	return(stn_num);
}
