/*****************************************************************
**	SOY2TSS : Calculate TSSID and FRACTION						**
**	FUNCTION: SOY2TSS accepts UTC time and clock. And calcurate **
**				TSSID and FRACTION.								**
**	AUTHOUR	: KAMENO Seiji										**
**	CREATED	: 1994/7/25											**
******************************************************************/

#include <stdio.h>
#define		TSSP	144432			/* CLOCK per TSS */
#define		TSSM	8388608			/* TSS Counter Maximum (=2^23) */
#define		TSS1	354				/* 144432 = 354 * 408 */
#define		TSS2	408				/* TSSP = TSS1 * TSS2 */

long	soy2tss( soy, clk, tss_ptr, frc_ptr)
	unsigned long	soy;			/* INPUT: Second of Year */
	unsigned long	clk;			/* INPUT: Clock [32 or 16 or 8 MHz] */
	unsigned long	*tss_ptr;		/* OUTPUT:Pointer of TSS */
	unsigned long	*frc_ptr;		/* OUTPUT:Pointer of TSS */
{
	unsigned long	clk1;			/* CLK = CLK1 * TSS1 + CLK2 */
	unsigned long	clk2;			/* CLK = CLK1 * TSS1 + CLK2 */
	unsigned long	tss;			/* TSS ID */
	unsigned long	frc;			/* TSS Fraction */
	unsigned long	sc1;			/* SC  = SC1 * TSS2 + SC2 */
	unsigned long	sc2;			/* SC  = SC1 * TSS2 + SC2 */
	char	dum[20];		/* Input Dummy Character */

	/*----------- INITIAL PARAMETER SETTINGS ---------------------------*/
	clk		= clk * 1000000;			/* Clock MHz -> Hz */

	clk1= (clk% TSSP) / TSS1;
	clk2= (clk% TSSP) % TSS1;		/* CLK%TSSP = CLK1*TSS1 + CLK2 */
	sc1	= (soy % TSSP) / TSS2;
	sc2	= (soy % TSSP) % TSS2;		/* SC%TSSP  = SC1 *TSS2 + SC2  */

	/*----------- CALCULATE TSS FRACTION ---------------------------*/
	/* TSS Fraction	= (SC * CLK) % TSSP								*/
	/*				= ((SC%TSSP) * (CLK%TSSP)) % TSSP				*/
	/*				= ((CLK1*TSS1+CLK2) * (SC1*TSS2+SC2)) % TSSP	*/
	/*--------------------------------------------------------------*/
	frc	= (TSS1*clk1*sc2 + TSS2*clk2*sc1 + clk2*sc2)%TSSP;

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
	tss	= (((soy/TSSP)*TSSP)%TSSM * (clk/TSSP))%TSSM
		+ (soy/TSSP)*(clk%TSSP)
		+ (soy%TSSP)*(clk/TSSP)
		+ clk1*sc1 + (TSS1*clk1*sc2 + TSS2*clk2*sc1 + clk2*sc2)/TSSP;
	tss	= tss%TSSM;
/*
---------------------------------------- OUTPUT
*/
	*frc_ptr = frc;
	*tss_ptr = tss;
/*
---------------------------------------- ENDING
*/
	return(0);
}
