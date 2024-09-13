#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cpgplot.h"

char	line_buf[80];		/* HEADER VALUE */

struct cordat{
	long	sppnum;			/* SPP NUMBER */
	long	cppnum;			/* CPP NUMBER */
	long	errflg;			/* ERROR FLAG */
	long	status;			/* Status Word */
	long	time_1990;		/* Second from 1990 */
	long	time_sec;		/* Second from Observation Date */
	long	time_micro;		/* Micro Second from Observation Date */
	long	tauapr;			/* A PRIORI DELAY */
	long	ratapr;			/* A PRIORI DELAY RATE */
	long	accapr;			/* A PRIORI DELAY ACCELERATION */
	long	tbc_re;			/* REAL PART OF TOTAL BIT COUNTER */
	long	tbc_im;			/* IMAG PART OF TOTAL BIT COUNTER */
	long	pcalre_x;		/* REAL PART OF PCAL */
	long	pcalim_x;		/* IMAG PART OF PCAL */
	long	pcalph_x;		/* INITIAL PHASE OF PCAL */
	long	pcalre_y;		/* REAL PART OF PCAL */
	long	pcalim_y;		/* IMAG PART OF PCAL */
	long	pcalph_y;		/* INITIAL PHASE OF PCAL */
	long	data[1024];		/* CORR DATA */
};
