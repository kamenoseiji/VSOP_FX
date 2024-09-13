/*********************************************************
**	FXSYNC_REPLAY.C : PLAY BACK FXSYNC STATUS			**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1998/6/15									**
**********************************************************/

#include "fxsync.inc"
#include <cpgplot.h>
#include <unistd.h>
#define ARG_NUM 2
#define DEVICE 1
#define TSS_MASK 0x7fffff
#define TRUE_MASK   (1 << pb_index)
#define FALSE_MASK  ~(1 << pb_index)
#define	SCHED_FMT	"%s %02d:%02d:%02d (%07d) - %02d:%02d:%02d (%07d) [%d sec]"
#define	FINISH_MAX	200
#define	SECDAY		86400
#define	DUMP_FILE1	"fxsync.dump1"
#define	DUMP_FILE2	"fxsync.dump2"

float	world_bottom;			/* Bottom in World Coordinate	*/
float	world_top;				/* Top in World Coordinate		*/
float	world_left;				/* Left in World Coordinate		*/
float	world_right;			/* Right in World Coordinate	*/
float	xmin, xmax;				/* Min and Max of X-Axis		*/
float	ymin, ymax;				/* Min and Max of Y-Axis		*/
float	x_text, y_text;			/* Text Position				*/

MAIN__(argc, argv)
	long	argc;		/* Number of Argument */
	char	**argv;		/* Pointer of Argument */
{

	/*-------- SHARED MEMORY FOR CURRENT STATUS --------*/
	FILE	*dump_file1;		/* File Pointer of Dump File	*/
	FILE	*dump_file2;		/* File Pointer of Dump File	*/
	struct shared_mem1 *fxsync_ptr;  /* Pointer of Shared Memory	*/
	struct shared_mem2 *fxsched_ptr;  /* Pointer of Shared Memory	*/
/*
---------------------------------------------------- ACCESS TO SHARED MEMORY
*/
	if(argc < ARG_NUM){
		printf("USAGE : fxsync_replay [PGPLOT DEVICE] !!\n");
		exit(0);
	}

	/*-------- ACCESS TO DUMP FILE 1--------*/
	fxsync_ptr = (struct shared_mem1 *)malloc( sizeof(struct shared_mem1) );
	dump_file1 = fopen( DUMP_FILE1, "r" );
	if( dump_file1 == NULL ){
		printf("Can't Open Dump File [%s]!!\n", DUMP_FILE1 );
		exit(0);
	}

	/*-------- ACCESS TO DUMP FILE --------*/
	fxsched_ptr = (struct shared_mem2 *)malloc( sizeof(struct shared_mem2) );
	dump_file2 = fopen( DUMP_FILE2, "r" );
	if( dump_file2 == NULL ){
		printf("Can't Open Dump File [%s]!!\n", DUMP_FILE2 );
		exit(0);
	}
/*
---------------------------------------------------- OPEN PGPLOT DEVICE
*/
	/*-------- INITIALIZE PGPLOT WINDOW --------*/
	cpgbeg( 1, argv[DEVICE], 1, 1 );
	cpgeras();
/*
---------------------------------------------------- LOOP FOR TIME
*/

	/*-------- READ PB SCHEDULE (fixed) --------*/
	if( fread(fxsched_ptr, 1, sizeof(struct shared_mem2), dump_file2)
		!= sizeof(struct shared_mem2) ){
		memset( fxsched_ptr, 0, sizeof(struct shared_mem2) );
	}

	/*-------- READ TIME-VARIABLE STATUS --------*/
	while(1){
		if( fread(fxsync_ptr, 1, sizeof(struct shared_mem1), dump_file1)
			!= sizeof(struct shared_mem1) ){
			fclose( dump_file1 );
			break;
		}

		cpgbbuf();
		disp_current_time(fxsync_ptr);

		disp_pb_stat(fxsync_ptr, fxsched_ptr);
		disp_bin_tape(fxsync_ptr, fxsched_ptr);
		if( fxsync_ptr->start_soy > 0 ){
			disp_pb_sched(fxsync_ptr, fxsched_ptr);
		}

		cpgebuf();
	}

	cpgend();
	return(0);
}
/*
---------------------------------------------------- DISP PB STATUS
*/
disp_pb_stat(fxsync_ptr, fxsched_ptr)
	struct shared_mem1 *fxsync_ptr;  /* Pointer of Shared Memory	*/
	struct shared_mem2 *fxsched_ptr;  /* Pointer of Shared Memory	*/
{
	int		pb_index;				/* Index for PlayBacks			*/
	int		st_index;				/* Index for /ST/ Scan			*/
	int		tape_index;				/* Index for Tape				*/
	int		tape_num;				/* Total Number of Tape			*/
	int		st_idr;					/* IDR at current /ST/			*/
	int		et_idr;					/* IDR at current /ET/			*/
	int		st_doy, et_doy;			/* Day of Year at /ST/ and /ET/	*/
	int		st_hh, et_hh;			/* Hour at /ST/ and /ET/		*/
	int		st_mm, et_mm;			/* Min at /ST/ and /ET/			*/
	int		st_ss, et_ss;			/* Sec at /ST/ and /ET/			*/
	char	pg_text[64];
	static	int		stat_col[2] = {DISABLE_COL, ENABLE_COL};

	world_bottom = 0.500;	world_top  = 0.880;
	world_left	 = 0.05;	world_right= 0.480;
	xmin = 0.0;		xmax = 9.0;
	ymin = 11.0;	ymax = -1.5;

	cpgsvp( world_left, world_right, world_bottom, world_top ); 
	cpgswin(xmin, xmax, ymin, ymax);

	cpgsci(UNDEF_COL);	cpgrect(xmin, xmax, ymin, ymax);
	cpgsci(WHITE_COL);	cpgbox("BC", 0.0, 0, "BC", 0.0, 0);
	cpgsch(0.7);

	cpgsci(WHITE_COL);
	cpgptxt(1.4, 1.0, 30.0, 0.0, "SYNC" );
	cpgptxt(2.4, 1.0, 30.0, 0.0, "UTC SET" );
	cpgptxt(3.4, 1.0, 30.0, 0.0, "TAPE INSIDE" );
	cpgptxt(4.4, 1.0, 30.0, 0.0, "REW DONE" );
	cpgptxt(5.4, 1.0, 30.0, 0.0, "SEARCH DONE" );
	cpgptxt(6.4, 1.0, 30.0, 0.0, "TL ACTIVE" );
	cpgptxt(7.4, 1.0, 30.0, 0.0, "POLARITY" );
	for( pb_index=1; pb_index<11; pb_index++){

		if( fxsched_ptr->st_num[pb_index] > 0 ){
			/*-------- SYNC --------*/
			cpgsci(stat_col[(fxsync_ptr->tape_sync >> pb_index)& 1 ]);
			cpgrect(1.25, 1.75, (float)(pb_index)+0.9, (float)(pb_index)+0.3);

			/*-------- UTC RESET --------*/
			cpgsci(stat_col[(fxsync_ptr->utcset_done >> pb_index)& 1]);
			cpgrect(2.25, 2.75, (float)(pb_index)+0.9, (float)(pb_index)+0.3);

			/*-------- TAPE INSERT --------*/
			cpgsci(stat_col[(fxsync_ptr->insert_done >> pb_index)& 1]);
			cpgrect(3.25, 3.75, (float)(pb_index)+0.9, (float)(pb_index)+0.3);

			/*-------- TAPE REWIND DONE --------*/
			cpgsci(stat_col[(fxsync_ptr->rewind_done >> pb_index)& 1]);
			cpgrect(4.25, 4.75, (float)(pb_index)+0.9, (float)(pb_index)+0.3);

			/*-------- TAPE SEARCH DONE --------*/
			cpgsci(stat_col[(fxsync_ptr->search_done >> pb_index)& 1]);
			cpgrect(5.25, 5.75, (float)(pb_index)+0.9, (float)(pb_index)+0.3);

			/*-------- TLRUN ENABLE --------*/
			cpgsci(stat_col[(fxsync_ptr->tlrun_enable >> pb_index)& 1]);
			cpgrect(6.25, 6.75, (float)(pb_index)+0.9, (float)(pb_index)+0.3);

			/*-------- TLRUN ENABLE --------*/
			cpgsci(stat_col[(fxsync_ptr->polarity >> pb_index)& 1]);
			cpgrect(7.25, 7.75, (float)(pb_index)+0.9, (float)(pb_index)+0.3);
		}

		cpgsci(WHITE_COL);
		x_text	= (float)0.25;
		y_text	= (float)pb_index + 0.75;
		sprintf( pg_text, "PB%02d", pb_index );
		cpgtext(x_text, y_text, pg_text );
	}
	return(0);
}

/*
---------------------------------------------------- DISP PB SCHEDULE
*/
disp_pb_sched(fxsync_ptr, fxsched_ptr)
	struct shared_mem1 *fxsync_ptr;  /* Pointer of Shared Memory	*/
	struct shared_mem2 *fxsched_ptr;  /* Pointer of Shared Memory	*/
{
	int		pb_index;				/* Index for PlayBacks			*/
	int		st_index;				/* Index for /ST/ Scan			*/
	int		tape_index;				/* Index for Tape				*/
	int		prev_tape_index;		/* Previous Index for Tape		*/
	int		tape_num;				/* Total Number of Tape			*/
	int		st_idr;					/* IDR at current /ST/			*/
	int		et_idr;					/* IDR at current /ET/			*/
	int		st_soy, et_soy;			/* Sec of Year at /ST/ and /ET/	*/
	int		st_doy, et_doy;			/* Day of Year at /ST/ and /ET/	*/
	int		st_hh, et_hh;			/* Hour at /ST/ and /ET/		*/
	int		st_mm, et_mm;			/* Min at /ST/ and /ET/			*/
	int		st_ss, et_ss;			/* Sec at /ST/ and /ET/			*/
	float	duration;				/* Duration (Stop - Start) [sec]*/
	char	pg_text[64];

	world_bottom = 0.05;	world_top  = 0.475;
	world_left	 = 0.05;	world_right= 0.99;
	xmin = (float)(fxsync_ptr->start_soy + SECDAY);
	xmax = (float)(fxsync_ptr->stop_soy + SECDAY);
	duration = xmax - xmin;
	ymin = 10.2;	ymax = -0.2;

	cpgsvp( world_left, world_right, world_bottom, world_top ); 
	cpgswin(xmin, xmax, ymin, ymax);

	cpgsci(UNDEF_COL);	cpgrect(xmin, xmax, ymin, ymax);
	cpgsci(WHITE_COL);	cpgtbox("BCNSTZH", 0.0, 0, "BC", 0.0, 0);
	cpgsci(RESERVED_COL);
	if( fxsync_ptr->utcset_done != 0 ){
		x_text = (float)(fxsync_ptr->master_soy + SECDAY);
		cpgmove(x_text, ymin);
		cpgdraw(x_text, ymax);
	}
	cpgsch(0.7);

	for( pb_index=1; pb_index<11; pb_index++){

		cpgsci(WHITE_COL);
		if( fxsched_ptr->st_num[pb_index] > 0 ){

			/*-------- SHOW CURRENT SCHEDULE --------*/
			st_index = fxsync_ptr->curr_scan[pb_index];
			if( st_index < fxsched_ptr->st_num[pb_index] ){
				soy2dhms( fxsched_ptr->st_soy[pb_index][st_index],
					&st_doy, &st_hh, &st_mm, &st_ss );
				soy2dhms( fxsched_ptr->et_soy[pb_index][st_index],
					&et_doy, &et_hh, &et_mm, &et_ss );

				sprintf(pg_text, SCHED_FMT, "Current",
					st_hh, st_mm, st_ss,
					fxsched_ptr->st_idr[pb_index][st_index],
					et_hh, et_mm, et_ss,
					fxsched_ptr->et_idr[pb_index][st_index],
					fxsched_ptr->dur_sec[pb_index][st_index] );

				x_text = 0.02* xmax + 0.98* xmin;	y_text = pb_index - 0.1;
				cpgtext(x_text, y_text, pg_text );
			}

			/*-------- SHOW NEXT SCHEDULE --------*/
			st_index ++; 
			if( st_index < fxsched_ptr->st_num[pb_index] ){
				soy2dhms( fxsched_ptr->st_soy[pb_index][st_index],
					&st_doy, &st_hh, &st_mm, &st_ss );
				soy2dhms( fxsched_ptr->et_soy[pb_index][st_index],
					&et_doy, &et_hh, &et_mm, &et_ss );

				sprintf(pg_text, SCHED_FMT, "Next",
					st_hh, st_mm, st_ss, fxsched_ptr->st_idr[pb_index][st_index],
					et_hh, et_mm, et_ss, fxsched_ptr->et_idr[pb_index][st_index],
					fxsched_ptr->dur_sec[pb_index][st_index] );

				x_text = 0.55* xmax + 0.45* xmin;	y_text = pb_index - 0.1;
				cpgtext(x_text, y_text, pg_text );
			}

			/*-------- PLOT ALL SCHEDULE --------*/
			for(st_index=0; st_index< fxsched_ptr->st_num[pb_index]; st_index++){
				st_soy = fxsched_ptr->st_soy[pb_index][st_index] + SECDAY;
				et_soy = fxsched_ptr->et_soy[pb_index][st_index] + SECDAY;

				if( fxsync_ptr->master_soy + SECDAY > et_soy ){ 
					cpgsci(15);
				} else if(fxsync_ptr->master_soy + SECDAY > st_soy){
					cpgsci(13);
				} else {
					cpgsci(DONE_COL);
				}
				if( fxsync_ptr->utcset_done == 0 ){ cpgsci(DONE_COL);}
				cpgrect((float)st_soy, (float)et_soy,
						(float)(pb_index)-0.5, (float)(pb_index)-0.9 );
			}

			/*-------- PLOT TAPE LABEL --------*/
			prev_tape_index = -1;
			for(st_index=0; st_index<fxsched_ptr->st_num[pb_index]; st_index++){
				st_soy = fxsched_ptr->st_soy[pb_index][st_index] + SECDAY;
				cpgsci(WHITE_COL);
				tape_index = fxsched_ptr->tape_id[pb_index][st_index];
				if( tape_index != prev_tape_index ){
					x_text = (float)st_soy;
					y_text = (float)(pb_index)-0.5;
					cpgtext( x_text, y_text,
					fxsched_ptr->tape_label[pb_index][tape_index]);
				}
				prev_tape_index = tape_index;
			}
		}
	}
	return(0);
}
/*
---------------------------------------------------- DISP CURRENT STATUS
*/
disp_bin_tape(fxsync_ptr, fxsched_ptr)
	struct shared_mem1 *fxsync_ptr;  /* Pointer of Shared Memory	*/
	struct shared_mem2 *fxsched_ptr;  /* Pointer of Shared Memory	*/
{
	int		dms_index;				/* Index for DMS-16/24			*/
	int		bin_index;				/* Index for Tape Bin			*/
	int		coltable;				/* Color Table					*/
	int		cart_stat;				/* Status of the Cart			*/

	world_bottom = 0.50;	world_top  = 0.95;
	xmin = 0.0;		xmax = 1.0;
	ymin = 17.0;	ymax = -1.0;

	for( dms_index=0; dms_index<5; dms_index++){
		world_left	= 0.5 + 0.1* dms_index;
		world_right	= world_left + 0.09;

		cpgsvp( world_left, world_right, world_bottom, world_top ); 
		cpgswin(xmin, xmax, ymin, ymax);

		cart_stat = fxsync_ptr->cart_event[dms_index][0];
		if(fxsync_ptr->cart_event[dms_index][1] > cart_stat){
			cart_stat = fxsync_ptr->cart_event[dms_index][1];	}

		switch( cart_stat ){
			case UNDEF		: coltable = UNDEF_COL;		break;
			case DISABLE	: coltable = DISABLE_COL;	break;
			case ENABLE		: coltable = CONFIRMED_COL;	break;
			case ACTIVE		: coltable = ACTIVE_COL;	break;
			case DONE		: coltable = DONE_COL;		break;
			case CONFIRMED	: coltable = CONFIRMED_COL;	break;
			case FINISH		: coltable = FINISH_COL;	break;
			case ABSFIN		: coltable = FINISH_COL;	break;
			default			: coltable = UNDEF_COL;		break;
		}

		cpgsci(coltable);
		cpgrect(xmin, xmax, ymin, ymax);
		cpgsci(1);	cpgbox("BC", 0.0, 0, "BC", 0.0, 0);

		cpgsch(0.7); cpgsci(0);
		for(bin_index=0; bin_index<16; bin_index++){
			x_text	= 0.1;		y_text	= (float)bin_index;
			cpgtext(x_text, y_text,
				fxsched_ptr->binlabel[dms_index][bin_index]);
		}
	}
	return(0);
}
/*
---------------------------------------------------- DISP CURRENT TIME
*/
disp_current_time(fxsync_ptr)
	struct shared_mem1 *fxsync_ptr;  /* Pointer of Shared Memory	*/
{
	unsigned int	doy;			/* Day of Year				*/
	unsigned int	hh;				/* Hour						*/
	unsigned int	mm;				/* Minute					*/
	unsigned int	ss;				/* Second					*/
	char	pg_text[64];			/* Text to be Plot			*/

	world_bottom= 0.90;	world_top  = 0.95;
	world_left	= 0.05; world_right= 0.48;

	xmin = 0.0;		xmax = 1.0;
	ymin = 0.0;		ymax = 1.0;

	cpgsvp( world_left, world_right, world_bottom, world_top ); 
	cpgswin(xmin, xmax, ymin, ymax);

	cpgsci(UNDEF_COL);	cpgrect(xmin, xmax, ymin, ymax);
	cpgsci(WHITE_COL);	cpgbox("BC", 0.0, 0, "BC", 0.0, 0);

	x_text	= 0.05;		y_text	= 0.1;

	soy2dhms( fxsync_ptr->master_soy, &doy, &hh, &mm, &ss );
	sprintf( pg_text, "MASTER UTC  %03d %02d:%02d:%02d", doy, hh, mm, ss);

	cpgsch(1.2);
	cpgtext(x_text, y_text, pg_text);

	return(0);
}
