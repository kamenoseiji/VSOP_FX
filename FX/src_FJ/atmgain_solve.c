/*********************************************************
**	ATMGAIN_SOLVE.C: Solution for Atmospheric GAIN 		**
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
#define	MAX_PRM	3*MAX_ANT
#define	ATM_FACT	stn_index+1
#define	CLK_FACT	stn_num+stn_index+1
#define	RTE_FACT	2*stn_num+stn_index+1
#define LOOP_LIMIT 40
#define E_EPS 10e-3
int	atmgain_solve(obs_ptr, first_obj_ptr, gcal_ptr_ptr, first_stn_ptr,
			sefd_ptr )

	struct	header		*obs_ptr;		/* Pointer of Obs Header			*/
	struct	head_obj	*first_obj_ptr;	/* Pointer of Obj Header			*/
	struct	gcal_data	**gcal_ptr_ptr;	/* Pointer of GCAL data				*/
	struct	head_stn	*first_stn_ptr;	/* Pointer of Station Header		*/
	double	*sefd_ptr;					/* Pointer of SEFD Parameters		*/
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
	double	p[3][3];					/* PARTIAL MATRIX */
	double	r[3];						/* RESIDUAL VECTOR */
	long	icon;						/* CONDITION CODE */
	int		max_prm;					/* Max Number of PRM */
	int		prm_num;					/* Number of PRM */
	double	epsz;						/* PIVOT CRITICAL VALUE */
	double	vw[3];						/* WORK AREA */
	int		ip[3];						/* WORK AREA */
	int		col_num;					/* Total Column Number */
	int		is;							/* Factor for Determinant */
	int		isw;						/* Control Code */
	int		i, j;						/* General Index */
	int		loop_index;
	double	weight;
	double	sefd_max,sefd_min;
	double	ex_tau, secz;
	double	ex_0, ex_1, ex_2, fx; 
	double	fit_prm[3];

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

		sefd_max	= 0.0;

		/*-------- INITIALIZE FIT_PRM --------*/
		while( gcal_ptr != NULL ){
			if( sefd_max < gcal_ptr->real){	
				sefd_max	= gcal_ptr->real;
			}

			sefd_min	= sefd_max;
			if( sefd_min > gcal_ptr->real ){
				sefd_min	= gcal_ptr->real;
			}

			gcal_ptr = gcal_ptr->next_gcal_ptr;
		}
	
		fit_prm[0]	= (sefd_max + sefd_min)/2.0;
		fit_prm[1]	= -((sefd_max + sefd_min)/2.0)*0.2;
		fit_prm[2]	= 0.1; 	

		/*-------- LOOP CALC BEST-FIT PARAM --------*/
		loop_index = 0;
		while(loop_index < LOOP_LIMIT){
			gcal_ptr	= gcal_ptr_ptr[stn_index];
		
			/*-------- INITIALIZE PARTIAL MATRIX and RESIDUAL VECTOR --------*/
			for(i=0; i<3; i++){
				for(j=0; j<3; j++){
					p[i][j] = 0.0;
				}
				r[i] = 0.0;
			}

			/*-------- LOOP FOR DATA --------*/
			while( gcal_ptr != NULL ){

				/*-------- SEARCH CURRENT OBJECT from OBJECT LIST --------*/
				obj_ptr = first_obj_ptr;
				while( obj_ptr != NULL ){
					if(strstr(gcal_ptr->objnam, obj_ptr->obj_name) != NULL){
						break;
					}
					obj_ptr = obj_ptr->next_obj_ptr;
				}
			
				/*-------- CALCULATE CURRENT ELEVATION --------*/
				mjd2gmst( gcal_ptr->mjd, obs_ptr->ut1utc, &gmst);
				gst2el( gmst, -lambda, atan2(sin_phi, cos_phi),
					obj_ptr->obj_pos[0]/RADDEG,
					obj_ptr->obj_pos[1]/RADDEG,
					&sin_el );
				#ifdef DEBUG
				printf("GMST = %lf, EL = %lf, SEFD = %lf +/- %lf\n",
					gmst, RADDEG*asin(sin_el), gcal_ptr->real,
					gcal_ptr->weight );
				#endif
		
				/*-------- MAKE PARTIAL MATRIX --------*/
				secz	= 1.0/(sin_el);
				ex_tau = exp( fit_prm[2] * secz );
 
				ex_0 = ex_tau;
				ex_1 = 1.0;
				ex_2 = fit_prm[0] * secz * ex_tau;
				
				if( gcal_ptr->weight > 0.0 ){
					weight	= 1.0/(gcal_ptr->weight* gcal_ptr->weight);
				} else {
					weight = 0.0;
				}

				p[0][0] 	+= ( (ex_0)*(ex_0) )* weight;

				p[1][0]		+= ( (ex_0)*(ex_1) )* weight;
				p[0][1]		= p[1][0];
				
				p[2][0]		+= ( (ex_0)*(ex_2) )* weight;
				p[0][2]		= p[2][0];

				p[1][1]		+= ( (ex_1)*(ex_1) )* weight;

				p[1][2]		+= ( (ex_1)*(ex_2) )* weight;
				p[2][1]		= p[1][2];

				p[2][2]		+= ( (ex_2)*(ex_2) )* weight;  		

				/*-------- MAKE RESIDUAL VECTOR --------*/
				fx	= fit_prm[0] * ex_tau + fit_prm[1];		
			
				r[0]	-= ( ex_0*(fx - gcal_ptr->real) )* weight;	
				r[1]	-= ( ex_1*(fx - gcal_ptr->real) )* weight;
				r[2]	-= ( ex_2*(fx - gcal_ptr->real) )* weight;

				gcal_ptr = gcal_ptr->next_gcal_ptr;
			}	/*-------- End of DATA Loop --------*/

			/*-------- SOLVED SIMULTAMEOUS EQUETION --------*/
			col_num = 3;	epsz = 0.0;	isw = 1;
			dlax_( p, &col_num, &col_num, r, &epsz, &isw, &is, vw, ip, &icon);

			for(i=0; i<3; i++){
				fit_prm[i] += r[i];
			}

			if( 	(fabs(r[0])<E_EPS) 
				&&  (fabs(r[1])<E_EPS) 
				&&  (fabs(r[2])<E_EPS) ){
				break;
			}

			loop_index ++;
		} /*-------- End of CALK BEST-FIT PARAM Loop --------*/

		sefd_ptr[ 3* stn_index ]		= fit_prm[0];
		sefd_ptr[ 3* stn_index + 1 ]	= fit_prm[1];
		sefd_ptr[ 3* stn_index + 2 ]	= fit_prm[2];

		/*-------- RESULT --------*/
		printf("RESULT : A0 = %9.5lf, A1 = %9.5lf, A2 = %9.5lf \n",
						fit_prm[0], fit_prm[1], fit_prm[2] );
		stn_index++;
		stn_ptr = stn_ptr->next_stn_ptr;
	}	/*-------- End of Station Loop --------*/


	return(icon);
}
