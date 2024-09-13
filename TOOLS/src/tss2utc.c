/*********************************************************************
**	TSS2SOY : Calculate SECOND OF YEAR for give TSSID and FRACTION	**
**	FUNCTION: UTC2TSS accepts TSSID, FRACTION and CLOCK. And 		**
**				calculate Second of Year.							**
**	AUTHOUR	: KAMENO Seiji											**
**	CREATED	: 1994/7/25												**
**********************************************************************/

#include <stdio.h>
#define		TSSP	144432			/* CLOCK per TSS */
#define		TSSM	8388608			/* TSS Counter Maximum (=2^23) */
#define		TSS1	354				/* 144432 = 354 * 408 */
#define		TSS2	408				/* TSSP = TSS1 * TSS2 */

long	tss2soy( soy, clk, tss, frc, soy_ptr, frc_ptr );

	unsigned long	soy;			/* INPUT: Second of Year [Approx.] */
	unsigned long	tss;			/* INPUT: TSS ID */
	unsigned long	frc;			/* INPUT: TSS FRACTION */
	unsigned long	clk;			/* INPUT: Clock [32 or 16 or 8 MHz] */
	unsigned long	*soy_ptr;		/* OUTPUT: Second of Year */
	unsigned long	*frc_ptr;		/* OUTPUT: Clock Fraction */

{
	unsigned long	tss0;			/* TSS ID for SOY */
	unsigned long	frc0;			/* TSS FRACTION for SOY */

/*
---------------------------------------- CALCULATE TSS0 
*/
	soy2tss( soy, clk, &tss0, &frc0 );

	delta_tss = tss - tss0;			/* TSS offset */
	delta_frc = frc - frc0;			/* TSS offset */
/*
---------------------------------------- LEAP for FRACTION
*/
	while(delta_frc >= TSSP){
		delta_tss = delta_tss + 1;
		delta_frc = delta_frc - TSSP;
	}

	while(delta_frc < 0){
		delta_tss = delta_tss - 1;
		delta_frc = delta_frc + TSSP;
	}
/*
---------------------------------------- CONVERT TSS OFFSET TO SECOND
*/
	delta_tss = delta_tss && 0x807fffff;
	tss2soy( delta_tss, delta_frc, clk, &delta_soy, &delta_frc );
/*
---------------------------------------- LEAP for FRACTION
*/
	while(delta_frc >= clk){
		delta_soy = delta_soy + 1;
		delta_frc = delta_frc - clk;
	}

	while(delta_frc < 0){
		delta_soy = delta_soy - 1;
		delta_frc = delta_frc + clk;
	}
/*
---------------------------------------- OUTPUT
*/
	*soy_ptr = soy + delta_soy;
	*frc_ptr = frc + delta_frc;
/*
---------------------------------------- ENDING
*/
	return(0);
}
