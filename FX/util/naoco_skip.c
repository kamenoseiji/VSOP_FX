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

#define HEADLINE 193
#define	SIZE_COR 4168

naoco_skip( corr_file, start, duration)

	FILE	**corr_file;			/* CORR file Pointer */
	long	start;				/* START Time from the Begin of the Day [sec] */
	long	duration;			/* DURATION [sec] */
{
	long	eof_flag;
	long	et_time;
	double	corr_time_tag;
	struct cordat	cor;		/* DATA FORMAT IN NAOCO FILE */

	et_time	= start + duration;
	eof_flag = 0;
	while(1){

		/*-------- CAN'T FIND START POINT --------*/
		if(fread(&cor, 1, SIZE_COR, *corr_file) != SIZE_COR){
			printf("Failed to SKIP Data.\n");
			eof_flag = 1;
			break;
		}

		corr_time_tag = (double)cor.time_sec + 1.0e-6*(double)cor.time_micro;

		/*-------- FIND START POINT --------*/
		if(corr_time_tag > start){
			eof_flag	= 0;
			break;
		}

		/*-------- START POINT PASSED --------*/
		if(corr_time_tag > et_time){
			eof_flag	= -1;
			break;
		}
	}

	return(eof_flag);
}
