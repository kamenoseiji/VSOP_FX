#include <stdio.h>
#include <math.h>
#include "cpgplot.h"
#include "delaydata.inc"
#define	TOTALLAG	512
#define	NSPEC	16
#define	NSS		16
#define	NCPP	256	
#define	MAX_SS	16
#define	MAX_ANT 5
#define	MAX_BL	MAX_ANT*(MAX_ANT-1)/2
#define	MAX_VAR	2*(MAX_BL*MAX_SS + MAX_ANT - 1)

MAIN__(argc, argv)
	long	argc;				/* Number of Arguments */
	char	**argv;				/* Pointer of Arguments */
{
	FILE	*corr_file[MAX_BL][MAX_SS];	/* File Pointer of CORR. File */
	FILE	*out_file_ptr;				/* File Pointer to Save Delay Data */
	long	eof_flag;
	long	first_flag;
	long	start;
	long	integ;
	long	stop_time;
	long	end_time;
	long	start_integ;
	double	rf[MAX_SS];			/* Observing Frequency [MHz] */
	double	freq_incr[MAX_SS];	/* Frequency increment [MHz] */
	double	fsample;			/* Sampling Freq. [MHz] */
	double	cpp_len;			/* Duration of Compressed PP [sec] */
	float	vis_r[MAX_BL*MAX_SS*NCPP*NSPEC];
	float	vis_i[MAX_BL*MAX_SS*NCPP*NSPEC];
	float	*vr_ptr[MAX_BL*MAX_SS];
	float	*vi_ptr[MAX_BL*MAX_SS];
	float	vis_amp[NCPP*2][NSPEC*2];
	double	vis_max;
	long	ncpp;
	struct	delay_file_header	delay_header;
	struct	delay_data			delay_bl[MAX_BL];
	struct	ant_delay_data		delay_ant;
	long	blnum;
	long	antnum;
	long	icon;				/* CONDITION CODE */
	long	ant_index;
	long	bl_index;
	long	ss_index;
	long	time_index;
	long	spec_index;
	long	delay_index;
	long	rate_index;
	long	spec_num[NSS];
	long	var_num;
	float	vis_snr[MAX_BL];
	float	gff_result[MAX_VAR];
	float	gff_err[MAX_VAR];
	float	afact;
	double	coeff[5];
	double	bl_delay[MAX_BL];
	double	bl_rate[MAX_BL];
	double	bl_delay_err[MAX_BL];
	double	bl_rate_err[MAX_BL];
	double	ant_delay[MAX_ANT];
	double	ant_rate[MAX_ANT];
	double	ant_delay_err[MAX_ANT];
	double	ant_rate_err[MAX_ANT];
	long	i, j;
/*
-------------------------------- INPUT PARAMETERS
*/
	if(argc < 7){
		printf("USAGE : gfftest [START] [STOP] [INTEG] [OUT_FILE] [PATH1] [NUM1] [PATH2] [NUM2] ... !!\n");
		exit(0);
	}
	if( (out_file_ptr = fopen(argv[4], "w")) == NULL){
		printf("Can't Open Output File [%s] !!\n", argv[4]);
		exit(0);
	}
	blnum = (argc-4)/2;
	antnum	= (1+(long)sqrt(1.0 + 8*blnum))/2;
	var_num	= 2*(blnum * NSS + antnum - 1);
	printf("%d BASELINEs, %d ANTENNAE\n", blnum, antnum);
/*
-------------------------------- START TIME
*/
	start	= 3600*(atoi(argv[1])/10000)
			+ 60*((atoi(argv[1])/100)%100)
			+ atoi(argv[1])%100;

	stop_time	= 3600*(atoi(argv[2])/10000)
			+ 60*((atoi(argv[2])/100)%100)
			+ atoi(argv[2])%100;

	integ	= atoi(argv[3]);
	end_time= start + integ;
/*
-------------------------------- WRITE HEADER TO DELAY DATA FILE
*/
	delay_header.antnum	= antnum;
	delay_header.blnum	= blnum;
	delay_header.ss_num	= NSS;
	delay_header.var_num= var_num;
	delay_header.start	= start;
	delay_header.integ	= integ;
	delay_header.time_incr	= integ/2;
	fwrite( &delay_header, 1, sizeof(delay_header), out_file_ptr );
/*
-------------------------------- OPEN NAOCO DATA FILE AND READ HEADER
*/
	/*-------- LOOP FOR BASELINE --------*/
	for(bl_index=0; bl_index<blnum; bl_index++){

		/*-------- LOOP FOR SUB-STREAM --------*/
		for(ss_index=0; ss_index<NSS; ss_index++){

			/*-------- OPEN CORR. DATA FILE --------*/
			eof_flag = naoco_open(
				argv[2*bl_index+5],			/* DIRECTORY POINTER */
				atoi(argv[2*bl_index+6]),	/* FILE NUMBER */
				ss_index+1,					/* Sub-Stream NUMBER */
				&corr_file[bl_index][ss_index]);
			if(eof_flag == -1 ){
				return(0);
			}


			/*-------- READ HEADER FROM CORR. DATA FILE --------*/
			naoco_header(
				&corr_file[bl_index][ss_index],
				ss_index,					/* Sub-Stream Index */
				&delay_bl[bl_index]);		/* HEADER INFORMATION */

			/*-------- SKIP TO THE START TIME --------*/
			eof_flag = naoco_skip(
				&corr_file[bl_index][ss_index],
				start,						/* START TIME */
				integ);						/* INTEGRATION TIME */

			if(eof_flag != 0 ){
				fclose(corr_file[bl_index][ss_index]);
				return(0);
			}
		}	/*-------- END OF SUB-STREAM LOOP --------*/
	}	/*-------- END OF BASELINE LOOP --------*/

/*
-------------------------------- LOOP FOR START TIME
*/
	eof_flag = 0;
	first_flag = 1;
	while(eof_flag != 1){
		printf("USING %02d:%02d:%02d - %02d:%02d:%02d\n",
			start/3600, (start/60)%60, start%60,
			(start+integ)/3600, ((start+integ)/60)%60, (start+integ)%60);

		/*-------- LOOP FOR BASELINE --------*/
		for(bl_index=0; bl_index<blnum; bl_index++){

			/*-------- INITIALIZE VISIBILITY AMP --------*/
			for(time_index=0; time_index<NCPP*2; time_index++){
				for(spec_index=0; spec_index<NSPEC*2; spec_index++){
					vis_amp[time_index][spec_index] = 0.0;
				}
			}

			/*-------- LOOP FOR SUB-STREAM --------*/
			for(ss_index=0; ss_index<NSS; ss_index++){

				vr_ptr[bl_index*NSS + ss_index]
					= &vis_r[(bl_index*NSS+ss_index)*NCPP*NSPEC];
				vi_ptr[bl_index*NSS + ss_index]
					= &vis_i[(bl_index*NSS+ss_index)*NCPP*NSPEC];

				/*-------- READ VISIBILITY FROM FILE --------*/
				eof_flag	= naoco_vis(
					&corr_file[bl_index][ss_index],
					start,						/* START TIME */
					integ,						/* INTEGRATION TIME */
					2*NSPEC,					/* LAG NUMBER */
					vr_ptr[bl_index*NSS + ss_index],
					vi_ptr[bl_index*NSS + ss_index],
					&delay_bl[bl_index]);		/* HEADER INFORMATION */

				rf[ss_index]		= delay_bl[bl_index].rf[ss_index];
				spec_num[ss_index]	= NSPEC;
				freq_incr[ss_index]	= delay_bl[bl_index].fsample/NSPEC/2;
				ncpp				= delay_bl[bl_index].ncpp;
				cpp_len				= delay_bl[bl_index].cpp_len;

				#ifdef DEBUG
				printf("SS=%d READ %d CPP DATA. \n",ss_index+1, ncpp);
				#endif

				/*-------- COARSE SEARCH FOR EACH BASELINE --------*/
				icon = coarse_mult(
					vr_ptr[bl_index*NSS + ss_index],
					vi_ptr[bl_index*NSS + ss_index],
					NSPEC,				/* SPECTRUM POINT */
					NSPEC,				/* SPECTRUM DIMENSION */
					rf[ss_index],		/* INITIAL FREQUENCY [MHz] */
					freq_incr[ss_index],/* FREQUENCY INCREMENT [MHz] */
					ncpp,				/* TIME POINT */
					cpp_len,			/* TIME INCREMENT */
					vis_amp
				);

			} /*-------- END OF SUB-STREAM LOOP --------*/
			afact	= 1.0/(float)(ncpp * NSPEC * NSS);

			/*-------- PEAK SEARCH [GRID] --------*/
			vis_max = 0.0;
			for(time_index=0; time_index<NCPP*2; time_index++){
				for(spec_index=0; spec_index<NSPEC*2; spec_index++){
					vis_amp[time_index][spec_index] *= afact;
					if( vis_amp[time_index][spec_index] > vis_max ){
						vis_max = vis_amp[time_index][spec_index];
						delay_index	= spec_index;
						rate_index	= time_index;
					}
				}
			}

			/*-------- PEAK SEARCH [SQUARE-MEHOD] --------*/
			sqr_fit((double)(delay_index-NSPEC-1),
				(double)(rate_index-pow2round(ncpp)),
				(double)vis_amp[rate_index][delay_index-1],

				(double)(delay_index-NSPEC),
				(double)(rate_index-pow2round(ncpp)),
				(double)vis_amp[rate_index][delay_index],

				(double)(delay_index-NSPEC+1),
				(double)(rate_index-pow2round(ncpp)),
				(double)vis_amp[rate_index][delay_index+1],

				(double)(delay_index-NSPEC),
				(double)(rate_index-pow2round(ncpp)-1),
				(double)vis_amp[rate_index-1][delay_index],

				(double)(delay_index-NSPEC),
				(double)(rate_index-pow2round(ncpp)+1),
				(double)vis_amp[rate_index+1][delay_index],
				coeff );

			/*-------- RESULTS FROM PEAK SEARCH --------*/
			bl_delay[bl_index]	= -coeff[2]/(coeff[0]*4*delay_bl[bl_index].fsample);
			bl_rate[bl_index]	= -1.0e6*coeff[3]
							/ (coeff[1]*4*rf[0]*pow2round(ncpp)*cpp_len);
			vis_max	= -(coeff[2]*coeff[2]/coeff[0]+coeff[3]*coeff[3]/coeff[1])/4
					+ coeff[4];
			vis_snr[bl_index]= vis_max
					*sqrt(5.0e5*delay_bl[bl_index].fsample*ncpp*cpp_len*sqrt(NSS));
			bl_delay_err[bl_index] = 1.0/(delay_bl[bl_index].fsample * vis_snr[bl_index]);
			bl_rate_err[bl_index] = 1.0e6
					/ (rf[0]*ncpp*cpp_len*vis_snr[bl_index]);

			printf("FMAX = %10.4e SNR = %5.1f DELAY = %lf, RATE = %lf\n",
			vis_max, vis_snr[bl_index], bl_delay[bl_index], bl_rate[bl_index]);


			/*-------- DISPLAY BASELINE-BASED SEARCH FUNCTION --------*/
			cpgbeg(1, "/xd", 1, 1);
			cpgenv(-cos(0.5), sin(0.5), 0.0, 1.5, 0, -2);
			cpgbbuf();
			cpgbird( vis_amp, pow2round(ncpp)*2, NSPEC*2, NSPEC*2, vis_max,
					0.5, 0.3, 1);
			cpgebuf();
			cpgend();
		} /*-------- END OF BASELINE LOOP --------*/

		/*-------- CONVERT BASELINE-BASED RESULTS -> ANTENNA-BASED --------*/
		closure_solve( antnum,bl_delay, bl_delay_err, ant_delay, ant_delay_err);
		closure_solve( antnum,bl_rate, bl_rate_err, ant_rate, ant_rate_err);

		for(i=0; i<antnum; i++){
			printf(" ANT %d : DELAY = %lf,  RATE=%lf\n",
					i, ant_delay[i], ant_rate[i]);
		}

		/*-------- STORE INITIAL PARAMETERS --------*/
/*
		for(i=0; i<antnum-1; i++){
			gff_result[blnum*(NSS + 1) + i]	= -ant_delay[i+1];
			gff_result[blnum*(NSS + 1) + antnum + i - 1]= -1.0e-6*ant_rate[i+1];
		}
*/
		for(i=0; i<antnum-1; i++){
			gff_result[2*blnum*NSS + i]	= -ant_delay[i+1];
			gff_result[2*blnum*NSS + antnum + i - 1]= -1.0e-6*ant_rate[i+1];
		}

		integ_mult( vr_ptr, vi_ptr, NSS, spec_num, rf, freq_incr,
				ncpp, cpp_len, antnum, gff_result);

/*
-------------------------------- GLOBAL FRINGE SEARCH
*/
		gff_mult( vr_ptr, vi_ptr, NSS, spec_num, rf, freq_incr,
				ncpp, cpp_len, antnum, gff_result, gff_err);

		#ifdef DEBUG
		printf("DELAY [microsec]    = %8.2e %8.2e\n", 
							gff_result[96], gff_result[97]);
		printf("ERR   [microsec]    = %8.2e %8.2e\n",
							gff_err[96], gff_err[97]);
		printf("RATE  [picosec/sec] = %8.2e %8.2e\n",
							gff_result[98]*1.0e6, gff_result[99]*1.0e6);
		printf("ERR   [picosec/sec] = %8.2e %8.2e\n",
							gff_err[98]*1.0e6, gff_err[99]*1.0e6);
		#endif

/*
-------------------------------- WRITE DELAY RESULTS TO FILE
*/
		if(first_flag == 1){

			/*-------- STATION INFOMATION FOR ANT0 --------*/
			strcpy(delay_ant.site_code, delay_bl[0].site_code_x);
			printf(" ANTENNA NAME = %s\n", delay_bl[0].site_code_x);
			delay_ant.input_offset	= delay_bl[0].input_rate_x;
			delay_ant.input_rate	= delay_bl[0].input_rate_x;
			delay_ant.clock_epoch	= delay_bl[0].clock_epoch;
			fwrite( &delay_ant, 1, sizeof(delay_ant), out_file_ptr );

			/*-------- STATION INFOMATION FOR OTHER ANT --------*/
			for( ant_index=1; ant_index< antnum; ant_index++){
				bl_index = ant2bl( ant_index, 0 );
				printf("ANT = %d, BL=%d\n", ant_index, bl_index);
				printf(" ANTENNA NAME = %s\n", delay_bl[bl_index].site_code_y);
				strcpy(delay_ant.site_code, delay_bl[bl_index].site_code_y);
				delay_ant.input_offset	= delay_bl[bl_index].input_rate_y;
				delay_ant.input_rate	= delay_bl[bl_index].input_rate_y;
				delay_ant.clock_epoch	= delay_bl[bl_index].clock_epoch;
				fwrite( &delay_ant, 1, sizeof(delay_ant), out_file_ptr );
			}

			first_flag = 0;
		}
		fwrite( gff_result, 1, 4*var_num, out_file_ptr );
		fwrite( gff_err, 1, 4*var_num, out_file_ptr );
		start += integ/2 ;
		if( start >= stop_time ){	break;}
	}

	/*-------- LOOP FOR SUB_STREAM --------*/
	for(ss_index=0; ss_index<NSS; ss_index++){
		/*-------- LOOP FOR BASELINE --------*/
		for(bl_index=0; bl_index<blnum; bl_index++){
			fclose(corr_file[bl_index][ss_index]);
		}
	}


	fclose(out_file_ptr);

	return(0);
}
