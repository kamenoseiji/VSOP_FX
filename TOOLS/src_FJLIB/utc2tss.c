/*****************************************************************
**	UTC2TSS : Calculate TSSID and FRACTION						**
**	FUNCTION: UTC2TSS accepts UTC time and clock. And calcurate **
**				TSSID and FRACTION.								**
**	AUTHOUR	: KAMENO Seiji										**
**	CREATED	: 1994/7/25											**
******************************************************************/

#include <stdio.h>
#define		TSSP	144432			/* CLOCK per TSS */
#define		TSSM	8388608			/* TSS Counter Maximum (=2^23) */
#define		TSS1	354				/* 144432 = 354 * 408 */
#define		TSS2	408				/* TSSP = TSS1 * TSS2 */

long	utc2tss( doy, hh, mm, ss, clk,
			tss_ptr, frk_ptr)
	unsigned long	doy;			/* INPUT: Day of Year */
	unsigned long	hh;				/* INPUT: Hour (UTC)*/
	unsigned long	mm;				/* INPUT: Minute */
	unsigned long	ss;				/* INPUT: Second */
	unsigned long	clk;			/* INPUT: Clock [32 or 16 or 8 MHz] */
	unsigned long	*tss_ptr;		/* OUTPUT:Pointer of TSS */
	unsigned long	*frk_ptr;		/* OUTPUT:Pointer of TSS */
{
	unsigned long	clk1;			/* CLK = CLK1 * TSS1 + CLK2 */
	unsigned long	clk2;			/* CLK = CLK1 * TSS1 + CLK2 */
	unsigned long	tss;			/* TSS ID */
	unsigned long	frk;			/* TSS Fraction */
	unsigned long	sc;				/* Second of Year */
	unsigned long	sc1;			/* SC  = SC1 * TSS2 + SC2 */
	unsigned long	sc2;			/* SC  = SC1 * TSS2 + SC2 */
	char	dum[20];		/* Input Dummy Character */

	/*----------- INITIAL PARAMETER SETTINGS ---------------------------*/
	clk		= clk * 1000000;						/* Clock MHz -> Hz */
	sc	= (((doy-1)*24 + hh)*60 + mm)*60 + ss;		/* Second of Year */

	clk1= (clk% TSSP) / TSS1;
	clk2= (clk% TSSP) % TSS1;		/* CLK%TSSP = CLK1*TSS1 + CLK2 */
	sc1	= (sc % TSSP) / TSS2;
	sc2	= (sc % TSSP) % TSS2;		/* SC%TSSP  = SC1 *TSS2 + SC2  */

	/*----------- CALCULATE TSS FRACTION ---------------------------*/
	/* TSS Fraction	= (SC * CLK) % TSSP								*/
	/*				= ((SC%TSSP) * (CLK%TSSP)) % TSSP				*/
	/*				= ((CLK1*TSS1+CLK2) * (SC1*TSS2+SC2)) % TSSP	*/
	/*--------------------------------------------------------------*/
	frk	= (TSS1*clk1*sc2 + TSS2*clk2*sc1 + clk2*sc2)%TSSP;

	/*------------ CALCULATE TSS ID --------------------------------*/
	/* TSSID	= (SC * CLK / TSSP) % TSSM							*/
	/*																*/
	/*	SC	= (SC/TSSP)*TSSP + SC%TSSP		--- (1)					*/
	/*	CLK	= (CLK/TSSP)*TSSP + CLK%TSSP	--- (2)					*/
	/*	(1), (2) ->													*/
	/*	SC*CLK/TSSP = (SC/TSSP)*(CLK/TSSP)*TSSP	... (a)				*/
	/*				+ (SC/TSSP)*(CLK%TSSP)							*/ 
	/*				+ (SC%TSSP)*(CLK/TSSP)							*/
	/*				+ (SC%TSSP)*(CLK%TSSP)/TSSP	... (b)				*/
	/*																*/
	/*	(a) <- ((SC/TSSP)*(CLK/TSSP)*TSSP)%TSSM						*/
	/*			= (((SC/TSSP)*TSSP)%TSSM * (CLK/TSSP))%TSSM			*/
	/*																*/
	/*	(b) <- (SC%TSSP)*(CLK*TSSP)/TSSP)							*/
	/*			= CLK1*SC1											*/
	/*			+ (TSS1*CLK1*SC2 + TSS2*CLK2*SC1 + CLK2*SC2)/TSSP	*/
	/*--------------------------------------------------------------*/
	tss	= (((sc/TSSP)*TSSP)%TSSM * (clk/TSSP))%TSSM
		+ (sc/TSSP)*(clk%TSSP)
		+ (sc%TSSP)*(clk/TSSP)
		+ clk1*sc1 + (TSS1*clk1*sc2 + TSS2*clk2*sc1 + clk2*sc2)/TSSP;
	tss	= tss%TSSM;

	#ifdef DEBUG
	printf("TSS ID  = %ld\n", tss);
	printf("TSS FRK = %ld\n", frk);
	#endif
/*
---------------------------------------- OUTPUT
*/
	*frk_ptr = frk;
	*tss_ptr = tss;
/*
---------------------------------------- ENDING
*/
	return(0);
}
