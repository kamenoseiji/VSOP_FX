/*********************************************************
**	CLOSURE_SOLVE.C	: Solution for Antenna-Based Value	**
**						From Baseline-Based Data		**
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
#define MAX_ANT 20
#define MAX_BL	190

long	closure_solve(ant_num, bl_data, bl_err,
						ant_data, ant_err)

	long	ant_num;			/* INPUT : NUMBER OF ANTENNA */
	double	*bl_data;			/* INPUT : BASELINE-BASED DATA */
	double	*bl_err;			/* INPUT : BASELINE-BASED ERROR */
	double	*ant_data;			/* OUTPUT: Pointer of ANTENNA-BASED DATA */
	double	*ant_err;			/* OUTPUT: Pointer of ANTENNA-BASED ERROR */
{
	long	i, j, k;			/* GENERAL COUNTER */
	long	niter;				/* SUFFIX OF NUMBER OF ITERATION */
	long	bl_suffix;			/* SUFFIX OF BASELINE */
	long	bl_num;				/* NUMBER OF BASELINE */
	long	ant_suffix;			/* SUFFIX OF BASELINE */
	long	ivw[MAX_ANT-1];		/* WORK AREA */
	long 	max_bl;				/* MAXIMUM BASELINE */
	long	max_ant;			/* MAXIMUM INDEPENDENT ANTTENA */
	long 	isw;				/* WORK AREA */
	long	icon;				/* CONDITION CODE */
	long	ip[MAX_ANT-1];		/* WORK AREA */
	double	initial_data[MAX_BL];		/* INITIAL SOLUTION */
	double	p[MAX_ANT-1][MAX_BL];		/* PARTIAL MATRIX */
	double	pwp[MAX_ANT-1][MAX_ANT-1];	/* WEIGHTED PARTIAL MATRIX */
	double	vw[2*(MAX_ANT-1)];	/* WORK AREA */
	double	epsz = 0.0;			/* PIVOT CRITICAL VALUE */
	double	w[MAX_BL];			/* WEIGHT */

	bl_num = ant_num * (ant_num -1)/2;
/*
----------------------------------------------- WEIGHT
*/
	for(i=0; i<bl_num; i++){
		if( *bl_err > 0.0){
			w[i] = 1.0/ (*bl_err);
		} else {
			w[i] = 0.0;
		}
		bl_err++;
	}
/*
----------------------------------------------- MAKE PARTIAL MATRIX
*/
	ant_suffix = 0;		bl_suffix = 0;
	/*-------- LOOP FOR Y-STATION --------*/
	for(i=1; i<ant_num; i++){

		/*-------- LOOP FOR X-STATION --------*/
		for(j=0; j<i; j++){

			/*-------- LOOP FOR ANTENNA TO SOLVE --------*/
			for( ant_suffix=1; ant_suffix<ant_num; ant_suffix++){
				p[ant_suffix-1][bl_suffix] = 0;
				if(ant_suffix == i){p[ant_suffix-1][bl_suffix] =-w[bl_suffix];}
				if(ant_suffix == j){p[ant_suffix-1][bl_suffix] = w[bl_suffix];}
			}
			bl_suffix++;
		}
	}

	ant_num	= ant_num -1;			/* NUMBER OF INDEPENDENT DELAY */
	max_bl	= MAX_BL;
	max_ant	= MAX_ANT-1;
	isw		= 1;
/*
----------------------------------------------- INITIAL SOLUTION
*/
	for(i=0; i<bl_num; i++){
		initial_data[i] = *bl_data * w[i];
		bl_data++;
	}
	dlaxl_(p, &max_bl, &bl_num, &ant_num, initial_data, &isw, vw, ivw, &icon);

	*ant_data = 0.0;					/* FOR REFERENCE ANT */
	for(i=0; i<ant_num; i++){
		ant_data++;
		*ant_data = initial_data[i];
	}
/*
----------------------------------------------- ERROR ESTIMATION
*/

		/*------- PARTIAL MATRIX --------*/
		for(i=0; i<ant_num; i++){
			for(j=0; j<ant_num; j++){
				pwp[i][j] = 0.0;
				for(k=0; k<bl_num; k++){
					pwp[i][j] = pwp[i][j] + p[i][k]*p[j][k];
				}
			}
		}

		/*------------------------ CAUTION ------------------------*/
		/* dalu_() is a function in SSL2 released by Fujitsu
			Corporation. It requires libssl2.a (for Sparc
			Compiler) or libfssl2.a (for Fujitsu Compiler)
			to be linked.									*/
		/*------------------------ CAUTION ------------------------*/
		dalu_(pwp, &max_ant, &ant_num, &epsz, ip, &isw, vw, &icon);
		if(icon > 0){
			printf("ITERATION HALTED.... ICON=%d\n", icon);
			return(icon);
		}

		/*------------------------ CAUTION ------------------------*/
		/* dluiv_() is a function in SSL2 released by Fujitsu
			Corporation. It requires libssl2.a (for Sparc
			Compiler) or libfssl2.a (for Fujitsu Compiler)
			to be linked.									*/
		/*------------------------ CAUTION ------------------------*/
		isw = 1;
		dluiv_(pwp, &max_ant, &ant_num, ip, &icon);

		/*------- ERROR ESTIMATION --------*/
		*ant_err = 0.0;				/* FOR REF ANT */
		for(i=0; i<ant_num; i++){
			ant_err++;
			*ant_err	= sqrt(pwp[i][i]);
		}

	return(icon);
}
