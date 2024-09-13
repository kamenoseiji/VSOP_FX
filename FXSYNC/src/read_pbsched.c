/*********************************************************
**	READ_PBSCHED.C : DISPLAY CURRENT FXSYNC STATUS		**
**														**
**	FUNCTION: Open Drudge File and Read.				**
**				Data File.								**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxsync.inc"
#include <unistd.h>
#define PBSCHED_INFO	"/apr01/pbs/info"
#define POLARITY_INFO	".polarity"
#define HD_FMT	"%d %*02s%d %s"
#define HD2_FMT	"%d %d %d %d %02d%d %02d%d %d"
#define SC1_FMT	"%d %d %*s %d %*02s%d %d %d %d"
#define SC2_FMT	"%d %d %s %d %*02s%d %*02s%d %d %d"
#define	TSS_MASK	0x7fffff
#define	TSSFACT		0.86545917802149108230	/* 1.0e6 / (8 * 144432) */
#define	MAX_PB	16

main(argc, argv)
	int		argc;			/* Number of Arguments				*/
	char	**argv;			/* Pointer of Arguments				*/
{
	/*-------- INDEX --------*/
	int		index;			/* General Index					*/
	int		pb_index;		/* PB Index							*/
	int		dms_index;		/* CART Index						*/
	int		tape_index;		/* Tape Index						*/
	int		tape_counter;	/* Tape Number for Each PB			*/
	int		scan_counter;	/* Scan Number for Each PB			*/

	/*-------- SHARED MEMORY FOR CURRENT STATUS --------*/
	key_t	fxsync_key;		/* Keyword for Shared Memory		*/
	key_t	fxsched_key;	/* Keyword for Shared Memory		*/
	int		shrd_fxsync_id;	/* Shared Memory ID					*/
	int		shrd_fxsched_id;/* Shared Memory ID					*/
	struct shared_mem1 *fxsync_ptr;		/* Pointer of Shared Memory	*/
	struct shared_mem2 *fxsched_ptr;	/* Pointer of Shared Memory	*/

	/*-------- STRING BUFFER --------*/
	FILE	*pbsched_file_ptr;	/* File Pointer of PB Schedule			*/
	FILE	*polar_file_ptr;	/* File Pointer of Polarity				*/
	char	*fname;		/* File Name of Sync Schedule */
	char	pbsched_fname[256];	/* PB Schedule File Name (Full Path)	*/
	char	line_buf[256];		/* Line Buffer of PB Schedule			*/
	char	tape_label[16];		/* Tape Label							*/
	char	prev_tape_label[16];/* Previous Tape Label					*/

	/*-------- TSS or IDR VARIABLE --------*/
	int		fxstart_tss;		/* Master TSS ID at FX-Start Event		*/
	int		fxstart_utc;		/* Master UTC (DDHHMMSS) at FX-Start	*/
	int		st_tss;				/* Master TSS ID at Start Event			*/
	int		rec_utc;			/* DDDHHMMSS of Master at Start Event	*/
	int		play_utc;			/* DDDHHMMSS of Slave at Start Event	*/
	int		frc;				/* TSS Fraction	[clock]					*/
	int		doy, hh, mm, ss;	/* DDD, HH, MM, SS						*/
	int		st_idr;				/* IDR at Start Event					*/
	int		et_tss;				/* TSSID at Stop Event					*/
	int		et_idr;				/* IDR at Stop Event					*/
	int		master_clk;			/* Master Clock Rate [256/128/64 Mbps]	*/
	int		fxstart_tss_256;	/* TSS ID at FXSTART in 256 Mbps		*/
	int		fxstart_tss_128;	/* TSS ID at FXSTART in 128 Mbps		*/
	int		fxstart_tss_64;		/* TSS ID at FXSTART in 64 Mbps			*/

	/*-------- GENERAL VARIABLE --------*/
	int		polarity;			/* Clock Polarity						*/
	int		newest_flag;		/* Newest Flag of the Schedule			*/
	int		sched_version;		/* Schedule Creation Date				*/
	int		pb_id;				/* PB ID Number							*/
	int		prev_pb_id;			/* Previous PB ID Number				*/
	int		pb_flag;			/* PB Flag  0 -> Unuse, 1 -> Use		*/
	int		rec_rate[MAX_PB];	/* Recording Rate [Mbps]				*/
	int		pb_rate;			/* Play-Back Rate [Mbps]				*/
	int		start_year;			/* Start Year [YY]						*/
	int		start_utc;			/* Start UTC [DDDHHMMSS]				*/
	int		stop_year;			/* Stop Year [YY]						*/
	int		stop_utc;			/* Stop UTC [DDDHHMMSS]					*/
	int		tape_num;			/* Number of Tapes						*/
	int		tape_id;			/* ID Number of Tapes					*/
	int		st_soy, et_soy;		/* Second of Year at /ST/ and /ET/		*/
	int		rough_st_soy;		/* Rough /ST/ Timing					*/
	int		dur_sec;			/* Plaback Duration in [sec]			*/
	int		dur_tss;			/* Plaback Duration in [tss]			*/
	char	sched_dir[64];		/* Schedlue File Directory				*/
	char	*line_ptr;
/*
---------------------------------------------------- ACCESS TO SHARED MEMORY
*/
	/*-------- ACCESS TO CURRENT STATUS --------*/
	fxsync_key	= FXSYNC_KEY1;
	shrd_fxsync_id = shmget(fxsync_key, sizeof(struct shared_mem1), 0666);
	if( shrd_fxsync_id  < 0 ){
		printf("Can't Access to the Shared Memory : %s !!\n", argv[0]);
		printf("RUN fxsync_shm at first.\n");
		exit(1);
	}
	fxsync_ptr	= (struct shared_mem1 *)shmat(shrd_fxsync_id, NULL, 0);
	memset( fxsync_ptr, 0, sizeof(struct shared_mem1) );

	/*-------- ACCESS TO PB SCHEDULE --------*/
	fxsched_key	= FXSYNC_KEY2;
	shrd_fxsched_id = shmget(fxsched_key, sizeof(struct shared_mem2), 0666);
	if( shrd_fxsched_id  < 0 ){
		printf("Can't Access to the Shared Memory : %s !!\n", argv[0]);
		printf("RUN fxsync_shm at first.\n");
		exit(1);
	}
	fxsched_ptr	= (struct shared_mem2 *)shmat(shrd_fxsched_id, NULL, 0);
	memset( fxsched_ptr, 0, sizeof(struct shared_mem2) );

    fxsync_info( fxsched_ptr );
/*
---------------------------------------------------- OPEN PB Schedule File
*/
	/*-------- SENSE PATH NAME OF THE SCHEDULE FILE --------*/
	if( (pbsched_file_ptr = fopen(PBSCHED_INFO, "r")) == NULL){
		printf("Can't Open %s !!\n", PBSCHED_INFO);	exit(0);
	}
	fgets(line_buf, sizeof(line_buf), pbsched_file_ptr);
	sscanf( line_buf, "%s", pbsched_fname);
	fclose( pbsched_file_ptr );

	/*-------- OPEN THE SCHEDULE FILE --------*/
	if( (pbsched_file_ptr = fopen(pbsched_fname, "r")) == NULL){
		printf("Can't Open PB Schedule File [%s] !!\n", pbsched_fname );
		fclose(pbsched_file_ptr);	return(-1);
	}

	/*-------- OPEN THE POLARITY FILE --------*/
	if( (polar_file_ptr = fopen(POLARITY_INFO, "r")) == NULL){
		printf("Can't Open Polarity File [%s] !!\n", POLARITY_INFO );
		fxsync_ptr->polarity = 0;
	} else {
		if(fgets(line_buf, sizeof(line_buf), polar_file_ptr) == 0){
			printf("Can't Read Polarity File [%s]!!\n", POLARITY_INFO );
			fxsync_ptr->polarity = 0;
		} else {
			sscanf( line_buf, "%X", &polarity );
			fxsync_ptr->polarity = polarity;
		}
		fclose(polar_file_ptr);
	}
	fxsync_ptr->polarity *= 2;
/*
---------------------------------------------------- READ FIRST ID
*/
	if(fgets(line_buf, sizeof(line_buf), pbsched_file_ptr) == 0){
		printf("Can't Read PB Schedule Headder [%s]!!\n", pbsched_fname );
		fclose(pbsched_file_ptr);	return(-1);
	}
	sscanf( line_buf, HD_FMT, &newest_flag, &sched_version, sched_dir );
#ifdef DEBUG
	printf("NEWEST  = %d\n", newest_flag);
	printf("VERSION = %d\n", sched_version);
#endif
/*
---------------------------------------------------- READ SECOND HEADDER
*/
	fxsync_ptr->pb_usage = 0x00;
	for( pb_index=0; pb_index<11; pb_index++){

		/*-------- READ DATA RATE and NUMBER OF TAPES --------*/
		if(fgets(line_buf, sizeof(line_buf), pbsched_file_ptr) == 0){
			printf("Can't Read PB Schedule Headder 2 [%s]!!\n", pbsched_fname );
			fclose(pbsched_file_ptr);	return(-1);
		}
		sscanf( line_buf, HD2_FMT, &pb_id, &pb_flag,
				&rec_rate[pb_index], &pb_rate,
				&start_year, &start_utc, &stop_year, &stop_utc, &tape_num );
		fxsched_ptr->tape_num[pb_index] = tape_num;

		/*-------- START and END Time from PB0 Entry --------*/
		if(pb_id == 0){
			fxsync_ptr->start_soy
				= 86400* (int)(start_utc / 1000000 - 1)
				+ 3600*  (int)((start_utc % 1000000) / 10000)
				+ 60*    (int)((start_utc % 10000) / 100)
				+        (int)(start_utc % 100);

			fxsync_ptr->stop_soy
				= 86400* (int)(stop_utc / 1000000 - 1)
				+ 3600*  (int)((stop_utc % 1000000) / 10000)
				+ 60*    (int)((stop_utc % 10000) / 100);
		}

		if(pb_flag == 1){
			if( tape_num > 0){
				fxsync_ptr->pb_usage |= (0x01 << pb_index);
				dms_index = fxsched_ptr->dms_id[pb_index] - 1;
				fxsync_ptr->cart_usage |= (0x01 << dms_index);
			}

			/*-------- READ TAPE LABEL LIST --------*/
			line_ptr = (char *)strchr( line_buf, ' ');	line_ptr++;
			for(index=0; index<5; index++){
				line_ptr = (char *)strchr( line_ptr, ' ');	line_ptr++;
			}
			for(tape_index=0; tape_index<tape_num; tape_index++){
				line_ptr = (char *)strchr( line_ptr, ' ');	line_ptr++;
				sscanf( line_ptr, "%s", tape_label );
				sprintf( fxsched_ptr->tape_label[pb_index][tape_index],
					"%s", tape_label);
#ifdef DEBUG
				printf(" TAPE LIST [%2d]: %s\n", tape_index+1, tape_label);
#endif
			}
		}
	}
/*
---------------------------------------------------- READ FXSTART TIMING
*/
	if(fgets(line_buf, sizeof(line_buf), pbsched_file_ptr) == 0){
		printf("Can't Read PB Schedule 1 [%s]!!\n", pbsched_fname );
		fclose(pbsched_file_ptr);	return(-1);
	}
	sscanf( line_buf, SC1_FMT, &pb_id, &fxstart_tss, &master_clk, &fxstart_utc,
			&fxstart_tss_256, &fxstart_tss_128, &fxstart_tss_64);
	fxsync_ptr->fxstart_utc = fxstart_utc;
	fxsync_ptr->fxstart_tss = fxstart_tss;
	fxsync_ptr->master_clk	= master_clk;
	fxsync_ptr->fxstart_tss_256	= fxstart_tss_256;
	fxsync_ptr->fxstart_tss_128	= fxstart_tss_128;
	fxsync_ptr->fxstart_tss_64	= fxstart_tss_64;

#ifdef DEBUG
	printf("FX START TIMING ... UTC = %d  TSS[32] = %d\n",
			fxstart_utc, fxstart_tss);
#endif
/*
---------------------------------------------------- READ TAPE PB SCHEDULE
*/
	scan_counter = 0;
	tape_counter = 0;
	prev_pb_id = -1;
	sprintf(prev_tape_label, "hidosugi");
	while(1){
		/*-------- READ 1-LINE --------*/
		if(fgets(line_buf, sizeof(line_buf), pbsched_file_ptr) == 0){ break; }
		sscanf( line_buf, SC2_FMT, &pb_id, &st_tss, tape_label, &pb_rate,
				&rec_utc, &play_utc, &st_idr, &et_tss);

		/*-------- CHECK FOR PLAYBACK CHANGE --------*/
		if( pb_id == prev_pb_id ){
			scan_counter ++;
		} else {
			if( prev_pb_id != -1 ){
				fxsched_ptr->st_num[prev_pb_id]   = scan_counter+1;
			}
			scan_counter = 0;
			tape_counter = 0;
			prev_pb_id = pb_id;
		}

		/*-------- INITIAL TAPE LABEL --------*/
		if( scan_counter == 0 ){
			strcpy( prev_tape_label, tape_label );
		}

		/*-------- CHECK FOR TAPE CHANGE --------*/
		if( strstr(tape_label, prev_tape_label) == NULL ){
			tape_counter ++;
		}
		strcpy( prev_tape_label, tape_label );

		/*-------- ET IDR CALCULATION --------*/
		et_idr = (st_idr + et_tss - st_tss) & TSS_MASK;

		/*-------- ACCURATE UTC TIMING --------*/
		dhms2soy( play_utc/1000000, (play_utc/10000)%100,
			(play_utc/100)%100, play_utc%100, &rough_st_soy );
		tss2soy( rough_st_soy, st_tss, 0, pb_rate/8, &st_soy, &frc );
		soy2dhms( st_soy, &doy, &hh, &mm, &ss );
		play_utc = doy*1000000 + hh*10000 + mm*100 + ss;

		/*-------- SET PARAMETERS into MEMORY --------*/
		fxsched_ptr->master_tssepc[pb_id][scan_counter]	= st_tss;
		fxsched_ptr->tape_id[pb_id][scan_counter]   = tape_counter;
		fxsched_ptr->rec_rate[pb_id][scan_counter] 	= rec_rate[pb_id];
		fxsched_ptr->play_rate[pb_id][scan_counter] = pb_rate;
		fxsched_ptr->rec_utc[pb_id][scan_counter] 	= rec_utc;
		fxsched_ptr->play_utc[pb_id][scan_counter] 	= play_utc;
		fxsched_ptr->st_idr[pb_id][scan_counter] 	= st_idr;
		fxsched_ptr->et_tss[pb_id][scan_counter] 	= et_tss;
		fxsched_ptr->et_idr[pb_id][scan_counter] 	= et_idr;

		/*-------- ADDITIONAL INFORMATION --------*/
		dur_tss = (et_idr - st_idr) & TSS_MASK;
		dur_sec = (int)( (float)dur_tss / (TSSFACT* pb_rate) );
		dhms2soy( play_utc/1000000, (play_utc/10000)%100,
			(play_utc/100)%100, play_utc%100, &st_soy );
		et_soy = st_soy + dur_sec;
		fxsched_ptr->st_soy[pb_id][scan_counter] = st_soy;
		fxsched_ptr->et_soy[pb_id][scan_counter] = et_soy;
		fxsched_ptr->dur_sec[pb_id][scan_counter] = dur_sec;

#ifdef DEBUG
		printf("PB[%d]  ST[%d] ST_IDR=%d  ET_IDR=%d  DUR_TSS=%d  DUR_SEC=%d\n",
			pb_id, scan_counter, st_idr, et_idr, dur_tss, dur_sec);
#endif

		fxsched_ptr->st_num[pb_id]   = scan_counter+1;

	}
/*
---------------------------------------------------- LOOP FOR TIME
*/
	system(READ_SOUND);

	fclose(pbsched_file_ptr);
	return(0);
}
