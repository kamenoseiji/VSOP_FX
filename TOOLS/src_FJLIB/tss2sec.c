/*********************************************************************
**	TSS2SEC : Convert given TSSID and FRACTION to Unit of Second	**
**	FUNCTION: TSS2SEC accepts TSSID, FRACTION and CLOCK. And 		**
**				Convert to Unit of Second.							**
**	AUTHOUR	: KAMENO Seiji											**
**	CREATED	: 1994/7/25												**
**********************************************************************/

#include <stdio.h>
#define		TSSP	144432			/* CLOCK per TSS */
#define		TSSM	8388608			/* TSS Counter Maximum (=2^23) */
#define		TSS1	354				/* 144432 = 354 * 408 */
#define		TSS2	408				/* TSSP = TSS1 * TSS2 */
#define		TSS3	51				/* 408 = 51 * 8 */
#define		TSS4	8				/* TSS2 = TSS3 * TSS4 */

long	tss2sec( tss, frc, clk, sec_ptr, sec_frc_ptr)

	unsigned long	tss;			/* INPUT: TSS ID */
	unsigned long	frc;			/* INPUT: TSS FRACTION */
	unsigned long	clk;			/* INPUT: Clock [32 or 16 or 8 MHz] */
	unsigned long	*sec_ptr;		/* OUTPUT: Pointer of Second */
	unsigned long	*sec_frc_ptr;	/* OUTPUT: Pointer of Clock Fraction */
{
	unsigned long	sec;			/* Second */
	unsigned long	sec_frc;		/* Clock Fraction of Second */
	unsigned long	clk1;			/* tss * TSS1 */
	unsigned long	clk2;			/* (clk1 % clk) * TSS3 */

	/*----------- INITIAL PARAMETER SETTINGS ---------------------------*/
	clk		= clk * 1000000;			/* Clock MHz -> Hz */

	/*----------- CALCULATE TSS FRACTION ---------------------------*/
	/*	SOY = ( TSS * TSSP + FRC ) / CLK							*/
	/*		where TSSP = 144432										*/
	/*																*/
	/*	TO AVOID BIT OVERFLOW, TSSP will be split to TSS1 * TSS2	*/
	/*--------------------------------------------------------------*/

	clk1	= tss * TSS1;
	clk2	= (clk1 % clk) * TSS3;

	sec		= (clk1/clk)*TSS2 + (clk2/clk)*TSS4 + ((clk2%clk)*8 + frc)/clk;
	sec_frc = ((clk2%clk)*8 + frc)%clk;
/*
---------------------------------------- OUTPUT
*/
	*sec_ptr		= sec;
	*sec_frc_ptr	= sec_frc;
/*
---------------------------------------- ENDING
*/
	return(0);
}
