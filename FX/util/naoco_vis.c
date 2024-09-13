/*************************************************************
**	BLDELAY.C	: BASELINE-BASED FRINGE SEARCH FUNCTION 	**
**															**
**	FUNCTION	: INPUT BASELINE-BASED CORRELATED DATA		**
**					and RETURNS DELAY, DELAY RATE, AMP,		**
**					PHASE, and SNR.							**
**	AUTHOR		: KAMENO Seiji								**
**	CREATED		: 1996/2/26									**
*************************************************************/

#include "naoco.inc"
#include "delaydata.inc"

#define HEADLINE 193
#define	SIZE_COR 4168
#define NLAG  512
#define NCPP  256
#define NCPP2 128
#define NSPEC 256
#define NSPEC2 128

naoco_vis( corr_file,	start,	duration,	lag_number,
			vis_r,		vis_i,	delay_ptr)

	FILE	**corr_file;		/* Pointer Data File */
	long	start;				/* START Time from the Begin of the Day [sec] */
	long	duration;			/* DURATION [sec] */
	long	lag_number;			/* LAG Number to be Used */
	float	*vis_r;				/* Visibility Function */
	float	*vis_i;				/* Visibility Function */
	struct	delay_data	delay_ptr;
{
	long	eof_flag;
	long	nfft;				/* FFT NUMBER */
	long	ndim;				/* FFT DIMENSION */
	long	ndir;				/* FFT DIRECTION */
	long	icon;				/* CONDITION CODE */
	long	nch;				/* CHANNEL NUMBER */
	long	i, j;				/* GENERAL COUNTER */
	long	et_time;
	long	ncpp;
	double	corr_time_tag;
	double	afact;
	double	cor_r[NLAG], cor_i[NLAG];	/* CORRELATION FUNCTION */
	struct cordat	cor;				/* DATA FORMAT IN NAOCO FILE */


	eof_flag = 0;
	et_time	 = start + duration;
	ncpp = 0;
	afact = 1.0/16777216;
	while(1){
		if(fread(&cor, 1, SIZE_COR, *corr_file) != SIZE_COR){
			printf("Failed to Read Data at CPP=%d.\n", ncpp);
			eof_flag = 1;
			break;
		}
		corr_time_tag = (double)cor.time_sec + 1.0e-6*(double)cor.time_micro;
		if(corr_time_tag > et_time){	break;	}
		if(corr_time_tag + delay_ptr.cpp_len > start){
			/*-------- SAVE LAG-DOMAIN DATA --------*/
			for(j=0; j<lag_number/2; j++){
				cor_r[j]				= cor.data[2*(j+NSPEC)];
				cor_i[j]				= cor.data[2*(j+NSPEC)+1];
				cor_r[j+lag_number/2]	= cor.data[2*(j+NSPEC)-lag_number];
				cor_i[j+lag_number/2]	= cor.data[2*(j+NSPEC)-lag_number+1];
			}
			nfft=lag_number; ndir=1; ndim=1;
			dcft_(cor_r, cor_i, &nfft, &ndim, &ndir, &icon);

			/*-------- SWITCH BY FRINGE RATE SIGN --------*/
			if(cor.ratapr < 0.0){

				/*-------- IN CASE OF USB --------*/
				for(j=0; j<lag_number/2; j++){
					*vis_r	=	(float)(afact*cor_r[j]);	vis_r++;
					*vis_i	=	(float)(afact*cor_i[j]);	vis_i++;
				}
			} else {

				/*-------- IN CASE OF LSB --------*/
				for(j=lag_number-1; j>=lag_number/2; j--){
					*vis_r	=	(float)(afact*cor_r[j]);	vis_r++;
					*vis_i	=	(float)(afact*cor_i[j]);	vis_i++;
				}
			}

			ncpp++;
		}
	}

	delay_ptr.ncpp		= ncpp;

	return(eof_flag);
}
