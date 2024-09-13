/*********************************************************************
**	TSS2SOY : Calculate SECOND OF YEAR for give TSSID and FRACTION	**
**	FUNCTION: TSS2SOY accepts TSSID, FRACTION and CLOCK. And 		**
**				calculate Second of Year.							**
**	AUTHOUR	: KAMENO Seiji											**
**	CREATED	: 1994/7/25												**
**********************************************************************/

#include <stdio.h>
#define		TSSP	144432			/* CLOCK per TSS */
#define		TSSM	8388608			/* CLOCK per TSS */

long	tss2soy( soy, tss, frc, clk, soy_ptr, frc_ptr )

	unsigned long	soy;			/* INPUT: Second of Year [Approx.] */
	unsigned long	tss;			/* INPUT: TSS ID */
	unsigned long	frc;			/* INPUT: TSS FRACTION */
	unsigned long	clk;			/* INPUT: Clock [32 or 16 or 8 MHz] */
	unsigned long	*soy_ptr;		/* OUTPUT: Second of Year */
	unsigned long	*frc_ptr;		/* OUTPUT: Clock Fraction */

{
	unsigned long	tss0;			/* TSS ID for SOY */
	unsigned long	frc0;			/* TSS FRACTION for SOY */
	unsigned long	tss_offset;		/* TSS OFFSET */
	unsigned long	frc_offset;		/* TSS OFFSET */
	unsigned long	soy_offset;		/* TSS OFFSET */
	long			delta_tss;		/* TSS OFFSET */
	long			delta_frc;		/* TSS OFFSET */
	long			delta_soy;		/* TSS OFFSET */

/*
---------------------------------------- CALCULATE TSS0 
*/
	soy2tss( soy, clk, &tss0, &frc0 );
	while(1){
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
		tss_offset = abs(delta_tss);
/*
---------------------------------------- CASE STUDY
*/
		if(tss_offset < TSSM/2){	break;
		} else if(delta_tss > 0){	tss0	+= TSSM;
		} else {					tss		+= TSSM;
		}
	}
	if(delta_tss < 0){
		tss_offset -= 1;
		delta_frc = TSSP - delta_frc; 
	}
/*
---------------------------------------- CONVERT TSS OFFSET TO SECOND
*/
	tss2sec( tss_offset, delta_frc, clk, &soy_offset, &frc_offset );
	clk = clk * 1000000;		/* CLK [MHz] -> [Hz] */
/*
---------------------------------------- OUTPUT
*/
	if( delta_tss < 0){
		*soy_ptr = soy - soy_offset - 1;
		*frc_ptr = clk - frc_offset;
	} else {
		*soy_ptr = soy + soy_offset;
		*frc_ptr = frc_offset;
	}
/*
---------------------------------------- LEAP for FRACTION
*/
	while( *frc_ptr >= clk ){
		*soy_ptr = *soy_ptr + 1;
		*frc_ptr = *frc_ptr - clk;
	}
/*
---------------------------------------- ENDING
*/
	return(0);
}
