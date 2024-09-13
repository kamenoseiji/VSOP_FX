/*********************************************************
**	CLPHS_SOLVE.C	: Solution for Antenna-Based Phase	**
**						From Baseline-Based Phase		**
**														**
**	FUNCTION : Input Baseline-Based Phase and Returns 	**
**				Antenna-Based Solution and Error		**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/2/8									**
**	COMMENT : This Module Needs SSL2 (Scientific Sub-	**
**				routine Library) Released by Fujitsu.	** 
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define SNR_LIMIT	4	
#define MAX_ITER	20	

long	clphs_solve(ant_num, bl_phs, bl_err,
						ant_phs, ant_err)

	int		ant_num;			/* INPUT : NUMBER OF ANTENNA */
	double	*bl_phs;			/* INPUT : BASELINE-BASED Phase */
	double	*bl_err;			/* INPUT : BASELINE-BASED ERROR */
	double	*ant_phs;			/* OUTPUT: Pointer of ANTENNA-BASED Phase */
	double	*ant_err;			/* OUTPUT: Pointer of ANTENNA-BASED ERROR */
{
	int		niter;				/* SUFFIX OF NUMBER OF ITERATION */
	int		ant1, ant2;			/* Baseline -> ant index */
	int		bl_index;			/* SUFFIX OF BASELINE */
	int		bl_num;				/* NUMBER OF BASELINE */
	int		ant_index;			/* SUFFIX OF BASELINE */
	int 	isw;				/* WORK AREA */
	int		icon;				/* CONDITION CODE */
	int		*ip;				/* WORK AREA */
	double	*a;					/* SOLUTION */
	double	*p;					/* PARTIAL MATRIX */
	double	*pwp;				/* WEIGHTED PARTIAL MATRIX */
	double	*vw;				/* WORK AREA */
	double	*w;					/* WEIGHT */
	double	*r;					/* Residual Vector */
	double	*d;					/* Correction Vector */
	double	epsz;				/* PIVOT CRITICAL VALUE */
	double	cor;				/* Correction */
	double	residual;			/* Chi Square */

	ant_num	--;
	bl_num = ((ant_num+1) * ant_num)/2;

	/*-------- THE MEMORY AREA USED IN MATRIX CALCULATION --------*/ 
	p	= (double *)malloc( ant_num*bl_num * sizeof(double) ); 
	a	= (double *)malloc( ant_num*sizeof(double) );
	pwp	= (double *)malloc( ant_num*ant_num * sizeof(double) ); 
	vw	= (double *)malloc( ant_num * sizeof(double) ); 
	r	= (double *)malloc( bl_num * sizeof(double) ); 
	d	= (double *)malloc( ant_num * sizeof(double) ); 
	w	= (double *)malloc( bl_num * sizeof(double) ); 
	ip	= (int *)malloc( ant_num*sizeof(int) );
/*
----------------------------------------------- INITIAL PARAMETER
*/
	for(ant_index=0; ant_index<ant_num; ant_index++){
		bl_index = ant2bl( 0, ant_index+1 );
		a[ant_index]			= bl_phs[bl_index];
	}

#ifdef DEBUG
	for(bl_index=0; bl_index<bl_num; bl_index++){
		printf("PHS[BL=%d] : %6.3lf +/- %6.3lf\n",
			bl_index, bl_phs[bl_index], bl_err[bl_index]);
	}
#endif

/*
----------------------------------------------- WEIGHT
*/
	for(bl_index=0; bl_index<bl_num; bl_index++){
		if( (bl_err[bl_index] > 0.0) && (bl_err[bl_index] < 1.0/SNR_LIMIT) ){
			w[bl_index] = 1.0 / (bl_err[bl_index]*bl_err[bl_index]);
		} else if( bl_err[bl_index] > 0.0 ){
			w[bl_index] = 0.1 / (bl_err[bl_index]*bl_err[bl_index]);
		} else {
			w[bl_index] = 0.0;
		}
		#ifdef DEBUG
		printf("WEIGHT[%d] = %lf\n", bl_index, w[bl_index]);
		#endif
	}

	niter=0;
	while(1){
/*
----------------------------------------------- RESIDUAL VECTOR
*/
#ifdef DEBUG
		printf("ITER[%d] : PHS= ", niter);
		for(ant_index=0; ant_index< ant_num; ant_index++){
			printf("%6.3lf ", a[ant_index] );
		}
#endif

		/*-------- RESIDUAL VECTOR --------*/
		residual	= 0.0;
		for(bl_index=0; bl_index<bl_num; bl_index++){
			bl2ant(bl_index, &ant2, &ant1);

			/*-------- INCLUDE REFANT --------*/
			if(ant1 == 0){
				r[bl_index]	= atan2(sin(bl_phs[bl_index] - a[ant2-1]),
									cos(bl_phs[bl_index] - a[ant2-1]))
							* w[bl_index];

			/*-------- NOT INCLUDE REFANT --------*/
			} else {
				r[bl_index]	= atan2(sin(bl_phs[bl_index]- a[ant2-1]+ a[ant1-1]),
							cos(bl_phs[bl_index] - a[ant2-1] + a[ant1-1]))
							* w[bl_index];
			}

			residual	+= r[bl_index]*r[bl_index];
#ifdef DEBUG
			printf("RESID [BL=%d] = %6.3lf\n",
				bl_index, r[bl_index] ); 
#endif
		}

		residual	/= (bl_num - ant_num);
/*
		printf("   TOTAL RESIDUAL = %lf\n", residual);
----------------------------------------------- MAKE PARTIAL MATRIX
*/
		/*-------- LOOP FOR Antenna --------*/
		for(ant_index=0; ant_index<ant_num; ant_index++){

			/*-------- LOOP FOR Baseline --------*/
			for(bl_index=0; bl_index<bl_num; bl_index++){

				p[ant_index*bl_num + bl_index]			= 0.0;
				bl2ant( bl_index, &ant2, &ant1);

				/*-------- PARTIAL MATRIX --------*/
				if(ant1 == ant_index + 1){
					p[ant_index*bl_num + bl_index] = -1.0;
				}

				if(ant2 == ant_index+1){
					p[ant_index*bl_num + bl_index] = 1.0;
				}
			}
		}

#ifdef DEBUG
		for(bl_index=0; bl_index<bl_num; bl_index++){
			for(ant_index=0; ant_index< ant_num; ant_index++){
				printf("%5.2lf ", p[ant_index*bl_num + bl_index]);
			}
			printf("\n");
		}
#endif
/*
----------------------------------------------- ERROR ESTIMATION
*/
		for(ant1=0; ant1<(ant_num*ant_num); ant1++){
			pwp[ant1] = 0.0;
		}

		/*------- PARTIAL MATRIX --------*/
		for(ant1=0; ant1<ant_num; ant1++){
			d[ant1]	= 0.0;
			for(ant2=0; ant2<ant_num; ant2++){
				for(bl_index=0; bl_index<bl_num; bl_index++){
					pwp[ant1*ant_num + ant2]+=	(p[ant1*bl_num + bl_index]
											*	p[ant2*bl_num + bl_index]
											*	w[bl_index]);
				}
			}
			for(bl_index=0; bl_index<bl_num; bl_index++){
				d[ant1]	+= p[ant1*bl_num + bl_index] * r[bl_index];
			}
		}


		/*------------------------ CAUTION ------------------------*/
		/* dalu_() is a function in SSL2 released by Fujitsu
			Corporation. It requires libssl2.a (for Sparc
			Compiler) or libfssl2.a (for Fujitsu Compiler)
			to be linked.									*/
		/*------------------------ CAUTION ------------------------*/
		epsz = 0.0;	isw = 0; cor = 0.0;
		dalu_(pwp, &ant_num, &ant_num, &epsz, ip, &isw, vw, &icon);
		if(icon > 0){
			printf("CLPHS_SOLVE: ITERATION HALTED.... ICON=%d\n", icon);
			return(icon);
		}

		/*------------------------ CAUTION ------------------------*/
		/* dlux_() is a function in SSL2 released by Fujitsu
			Corporation. It requires libssl2.a (for Sparc
			Compiler) or libfssl2.a (for Fujitsu Compiler)
			to be linked.									*/
		/*------------------------ CAUTION ------------------------*/
		isw = 1;
		dlux_( d, pwp, &ant_num, &ant_num, &isw, ip, &icon);
		if(icon > 0){
			printf("ITERATION HALTED.... ICON=%d\n", icon);
			return(icon);
		}

		for(ant_index=0; ant_index<ant_num; ant_index++){
			a[ant_index]		+= d[ant_index];
			cor	+= d[ant_index]*d[ant_index];
		}
		cor = sqrt(cor)/ant_num;
		if( (cor < 1.0e-6) || (niter >= MAX_ITER) ){	break;}

		niter++;
	}


	/*------------------------ CAUTION ------------------------*/
	/* dluiv_() is a function in SSL2 released by Fujitsu
		Corporation. It requires libssl2.a (for Sparc
		Compiler) or libfssl2.a (for Fujitsu Compiler)
		to be linked.									*/
	/*------------------------ CAUTION ------------------------*/
	isw = 1;
	dluiv_(pwp, &ant_num, &ant_num, ip, &icon);

	/*------- ERROR ESTIMATION --------*/
	ant_phs[0] = 0.0;				/* FOR REF ANT */
	ant_err[0] = 0.0;				/* FOR REF ANT */
	for(ant_index=0; ant_index<ant_num; ant_index++){
		ant_phs[ant_index+1]	= a[ant_index];
		ant_err[ant_index+1]	= sqrt(pwp[ant_index*ant_num+ant_index]);
		#ifdef DEBUG
		printf("PHS ERROR [%d] = %6.3lf\n", ant_index+1, ant_err[ant_index+1]);
		#endif
	}

	free(p);
	free(a);
	free(pwp);
	free(vw);
	free(w);

	return(icon);
}
