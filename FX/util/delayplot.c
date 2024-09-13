#include <stdio.h>
#include <math.h>
#include "cpgplot.h"
#include "delaydata.inc"
#define	PI		3.1415926535897932384626433
#define	TOTALLAG	512
#define	NSPEC	16
#define	NSS		16
#define	NCPP	1024	
#define	MAX_SS	16
#define	MAX_ANT 5
#define	MAX_BL	MAX_ANT*(MAX_ANT-1)/2
#define	MAX_VAR	2*(MAX_BL*MAX_SS + MAX_ANT - 1)

MAIN__(argc, argv)
	long	argc;				/* Number of Arguments */
	char	**argv;				/* Pointer of Arguments */
{
	FILE	*delay_file_ptr;		/* File Pointer of Delay Data */
	FILE	*out_file_ptr;			/* File Pointer to Save Delay Data */
	struct	delay_file_header	delay_header;
	struct	ant_delay_data		delay_ant;
	float	gff_result[MAX_VAR];
	float	gff_err[MAX_VAR];
	long	time_index;
	long	ant_index;
	long	file_index;
	long	ss_index;
	long	time_num;
	float	start_time;
	float	current_time;
	float	end_time;
	float	time_data[NCPP];
	float	delay[MAX_ANT][NCPP];
	float	delay_u[MAX_ANT][NCPP];
	float	delay_l[MAX_ANT][NCPP];
	float	amp[NSS][NCPP];
	float	phs[NSS][NCPP];
	float	phs_base[NCPP];
	float	phs_ss[NSS];
	long	delay_index_offset;
	float	rate[MAX_ANT][NCPP];
	float	rate_u[MAX_ANT][NCPP];
	float	rate_l[MAX_ANT][NCPP];
	long	rate_index_offset;
	double	sum_weight[MAX_ANT];
	double	sum_time[MAX_ANT];
	double	sum_time2[MAX_ANT];
	double	sum_time_delay[MAX_ANT];
	double	sum_delay[MAX_ANT];
	double	matrix[2][2];
	double	vect[2];
	long	ndim;
	double	epsz;
	long	isw;
	long	is;
	double	vw[2];
	long	ip[2];
	long	icon;
/*
-------------------------------- OUTPUT FILE
*/
	if(argc < 3){
		printf("USAGE : delayplot [DELAY_FILE] [...] [OUT FILE] !!\n");
		exit(0);
	}

/*
-------------------------------- INIT PARAMETERS
*/
	for(ant_index=0; ant_index < MAX_ANT; ant_index++){
		sum_weight[ant_index]		= 0.0;
		sum_time[ant_index]			= 0.0;
		sum_time2[ant_index]		= 0.0;
		sum_time_delay[ant_index]	= 0.0;
		sum_delay[ant_index]		= 0.0;
	}

	if( (out_file_ptr = fopen(argv[argc-1], "w")) == NULL){
		printf("Can't Open Output File [%s] !!\n", argv[argc-1]);
		exit(0);
	}

	time_index	= 0;
	for(file_index=0; file_index<argc-2; file_index++){

		if( (delay_file_ptr = fopen(argv[file_index+1], "r")) == NULL){
			printf("Can't Open Input File [%s] !!\n", argv[file_index+1]);
			exit(0);
		}

/*
-------------------------------- READ HEADER FROM FILE
*/
		if( fread(&delay_header, 1, sizeof(delay_header), delay_file_ptr)
			!= sizeof(delay_header) ){
			printf("Can't Read Header from File !!\n");
			fclose(delay_file_ptr);
			break;
		}

		/*-------- SAVE FILE HEADER TO OUTPUT FILE --------*/
		if(file_index == 0){
			fwrite(&delay_header, 1, sizeof(delay_header), out_file_ptr);
		}

		#ifdef DEBUG
		printf("ANTENNA NUMBER = %d\n", delay_header.antnum);
		printf("VAR     NUMBER = %d\n", delay_header.var_num);
		printf("START TIME     = %d\n", delay_header.start);
		printf("INTEG TIME     = %d\n", delay_header.integ);
		printf("TIME INCREMENT = %d\n", delay_header.time_incr);
		#endif

		for(ant_index=0; ant_index < delay_header.antnum; ant_index++){
			if( fread(&delay_ant, 1, sizeof(delay_ant), delay_file_ptr)
				!= sizeof(delay_ant) ){
				printf("Can't Read Header from File !!\n");
				fclose(delay_file_ptr);
				return(0);
			}
			#ifdef DEBUG
			printf("ANTENNA [%d] : %s\n", ant_index, delay_ant.site_code);
			#endif

			/*-------- SAVE ANTENNA HEADER TO OUTPUT FILE --------*/
			if(file_index == 0){
				fwrite(&delay_ant, 1, sizeof(delay_ant), out_file_ptr);
			}
		}
/*
-------------------------------- READ FILE
*/
		delay_index_offset= delay_header.antnum*(delay_header.antnum-1)*MAX_SS;
		rate_index_offset = delay_header.antnum*(delay_header.antnum-1)*MAX_SS
							+ delay_header.antnum -1;
		if(file_index == 0){
			start_time	= (float)delay_header.start;
		}
		current_time= (float)delay_header.start;
		while(1){

			/*-------- READ DELAY DATA --------*/
			if( fread(gff_result, 1, 4*delay_header.var_num, delay_file_ptr)
				!= 4*delay_header.var_num ){
				printf("Failed to Read Delay Data !! \n");
				break;
			}

			/*-------- READ DELAY ERROR --------*/
			if( fread(gff_err, 1, 4*delay_header.var_num, delay_file_ptr)
				!= 4*delay_header.var_num ){
				printf("Failed to Read Delay Error Data !! \n");
				break;
			}

			time_data[time_index]	= current_time
									+ (float)delay_header.integ/2;


			for(ant_index=0; ant_index < delay_header.antnum-1; ant_index++){
				delay[ant_index][time_index]
					= gff_result[delay_index_offset + ant_index];
				delay_u[ant_index][time_index] = delay[ant_index][time_index]
					+ gff_err[delay_index_offset + ant_index];
				delay_l[ant_index][time_index] = delay[ant_index][time_index]
					- gff_err[delay_index_offset + ant_index];

				/*-------- PARAMETERS FOR LINEAR FIT --------*/
				sum_time[ant_index]
					+= time_data[time_index]
					/ (gff_err[delay_index_offset+ant_index]
					*  gff_err[delay_index_offset+ant_index]);

				sum_time2[ant_index]
					+= (time_data[time_index] * time_data[time_index])
					/ (gff_err[delay_index_offset+ant_index]
					*  gff_err[delay_index_offset+ant_index]);

				sum_weight[ant_index] += 1.0 /
					( gff_err[delay_index_offset+ant_index]
					* gff_err[delay_index_offset+ant_index]);

				sum_time_delay[ant_index]
					+= time_data[time_index] * delay[ant_index][time_index]
					/ (gff_err[delay_index_offset+ant_index]
					*  gff_err[delay_index_offset+ant_index]);

				sum_delay[ant_index]
					+= delay[ant_index][time_index]
					/ (gff_err[delay_index_offset+ant_index]
					*  gff_err[delay_index_offset+ant_index]);

				rate[ant_index][time_index]
					= gff_result[rate_index_offset + ant_index];
				rate_u[ant_index][time_index] = rate[ant_index][time_index]
					+ gff_err[rate_index_offset + ant_index];
				rate_l[ant_index][time_index] = rate[ant_index][time_index]
					- gff_err[rate_index_offset + ant_index];

			}

			for(ss_index=0; ss_index < NSS; ss_index++){
				amp[ss_index][time_index] = gff_result[ss_index];
				phs[ss_index][time_index] = gff_result[ss_index + 48];
			}

			time_index++;
			current_time	+= (float)delay_header.time_incr;
		}
		fclose(delay_file_ptr);
		printf("READ %d DATA POINTS. \n", time_index);
	}

	time_num	= time_index;
	end_time	= current_time + (float)delay_header.time_incr;
/*
-------------------------------- PLOT DELAY
*/

	cpgbeg(1, "?", 2, 2);
	cpgenv( start_time, end_time, -0.1, 0.1, 0, 1);
	for(ant_index=0; ant_index < delay_header.antnum-1; ant_index++){
		cpgsci(ant_index+2);
		cpgpt( time_num, time_data, delay[ant_index], 17);
		cpgsci(1);
		cpgerry( time_num, time_data,
			delay_l[ant_index], delay_u[ant_index], 1.0);
	}
/*
-------------------------------- LINEAR FIT
*/
	for(ant_index=0; ant_index < delay_header.antnum-1; ant_index++){
		vect[0]	= sum_time_delay[ant_index];
		vect[1]	= sum_delay[ant_index];
		matrix[0][0]	= sum_time2[ant_index];
		matrix[0][1]	= sum_time[ant_index];
		matrix[1][0]	= sum_time[ant_index];
		matrix[1][1]	= sum_weight[ant_index];

		ndim	= 2;	isw		= 1;	epsz	= 0.0;
		dlax_(matrix, &ndim, &ndim, vect, &epsz, &isw, &is, vw, ip, &icon);

		gff_result[delay_index_offset + ant_index]
			= vect[0]*start_time + vect[1];
		gff_result[rate_index_offset + ant_index]
			= vect[0];

		printf("DELAY = %e, RATE = %e\n", vect[0]*start_time + vect[1], vect[0]); 

		cpgmove( start_time, vect[0]*start_time + vect[1]);
		cpgdraw( end_time, vect[0]*end_time + vect[1]);
	}
/*
-------------------------------- PLOT RATE
*/
	cpgenv( start_time, end_time, -2.0e-6, 2.0e-6, 0, 1);
	for(ant_index=0; ant_index < delay_header.antnum-1; ant_index++){
		cpgsci(ant_index+2);
		cpgpt( time_num, time_data, rate[ant_index], 17);
		cpgline( time_num, time_data, rate[ant_index]);
		cpgsci(1);
		cpgerry( time_num, time_data,
			rate_l[ant_index], rate_u[ant_index], 1.0);
	}
/*
-------------------------------- PLOT AMP
*/
	cpgsci(1);
	cpgenv( start_time, end_time, 0.0, 0.01, 0, 1);
	for(ss_index=0; ss_index < NSS; ss_index++){
		cpgsci(ss_index%7 + 1);
		cpgpt( time_num, time_data, amp[ss_index], 17);
		cpgline( time_num, time_data, amp[ss_index]);
	}

/*
-------------------------------- PLOT PHASE
*/
	cpgsci(1);
	cpgenv( start_time, end_time, -PI, PI, 0, 1);
	for(ss_index=0; ss_index < NSS; ss_index++){
		cpgsci(ss_index%7 + 1);
		cpgpt( time_num, time_data, phs[ss_index], 17);
		cpgline( time_num, time_data, phs[ss_index]);
	}


	fwrite(&gff_result, 1, sizeof(gff_result), out_file_ptr);
	fclose(out_file_ptr);
	cpgend();
	return(0);
}
