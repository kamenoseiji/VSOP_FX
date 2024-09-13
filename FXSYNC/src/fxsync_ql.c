/*********************************************************
**	FXSYNC_QL.C : QLOOK MONITOR CURRENT FXSYNC STATUS	**
**														**
**	FUNCTION: Open Drudge File and Read.				**
**				Data File.								**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxsync.inc"
#include <cpgplot.h>
#include <unistd.h>
#define ARG_NUM 2
#define DEVICE 1
#define	BACK_COL	12
#define	CATEG_COL	13
#define	VALUE_COL	14
#define	FRAME_COL	15
#define	PBMASK  (1<<index)

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

	/*-------- INDEX --------*/
	int		dms_index;		/* Cart Index		*/
	int		bin_index;		/* BIN Index		*/
	int		err_code;		/* Error Code in PGPLOT	*/
	int		index;			/* General Index		*/

	/*-------- SHARED MEMORY FOR CURRENT STATUS --------*/
	key_t	fxsync_key;			/* Keyword for Shared Memory	*/
	key_t	fxsched_key;		/* Keyword for Shared Memory	*/
	int		shrd_fxsync_id;		/* Shared Memory ID				*/
	int		shrd_fxsched_id;	/* Shared Memory ID				*/
	struct shared_mem1 *fxsync_ptr;		/* Pointer of Shared Memory	*/
	struct shared_mem2 *fxsched_ptr;	/* Pointer of Shared Memory	*/

/*
---------------------------------------------------- ACCESS TO SHARED MEMORY
*/
	if(argc < ARG_NUM){
		printf("USAGE : fxsync_view [PGPLOT DEVICE] !!\n");
		exit(0);
	}

	/*-------- ACCESS TO CURRENT STATUS --------*/
	fxsync_key	= FXSYNC_KEY1;
	shrd_fxsync_id = shmget(fxsync_key, sizeof(struct shared_mem1), 0444);
	if( shrd_fxsync_id  < 0 ){
		printf("Can't Access to the Shared Memory : %s !!\n", argv[0]);
		printf("RUN fxsync_shm at first.\n");
		exit(1);
	}
	fxsync_ptr	= (struct shared_mem1 *)shmat(shrd_fxsync_id, NULL, 0);

	/*-------- ACCESS TO PB SCHEDULE --------*/
	fxsched_key	= FXSYNC_KEY2;
	shrd_fxsched_id = shmget(fxsched_key, sizeof(struct shared_mem2), 0444);
	if( shrd_fxsched_id  < 0 ){
		printf("Can't Access to the Shared Memory : %s !!\n", argv[0]);
		printf("RUN fxsync_shm at first.\n");
		exit(1);
	}
	fxsched_ptr	= (struct shared_mem2 *)shmat(shrd_fxsched_id, NULL, 0);
/*
---------------------------------------------------- OPEN PGPLOT DEVICE
*/
	/*-------- INITIALIZE PGPLOT WINDOW --------*/
	cpgbeg( 1, argv[DEVICE], 1, 1 );
	cpgscrn(BACK_COL,	"DarkSlateGray",	&err_code );
	cpgscrn(CATEG_COL,	"Yellow",			&err_code );
	cpgscrn(VALUE_COL,	"aquamarine",		&err_code );
	cpgscrn(FRAME_COL,	"White",			&err_code );
	cpgeras();
/*
---------------------------------------------------- LOOP FOR TIME
*/

	index = -1;
	while(1){

		cpgbbuf();
		disp_pilot(&index);
		disp_general_info(fxsync_ptr);
		disp_gpib_info(fxsched_ptr);
		disp_event_info(fxsync_ptr);
		cpgebuf();
		fsleep(500);

		if(fxsync_ptr->validity == FINISH){	break;}
		if(fxsync_ptr->validity == ABSFIN){
			return(0);
		}
	}

	sleep(60);
	return(0);
}
/*
---------------------------------------------------- DISP PILOT LAMP
*/
disp_pilot(index_ptr)
	int		*index_ptr;
{
	world_bottom= 0.90;	world_top  = 0.95;
	world_left	= 0.95; world_right= 1.00;
	xmin = 0.0;		xmax = 1.0;
	ymin = 0.0;		ymax = 1.0;
	cpgsvp( world_left, world_right, world_bottom, world_top ); 
	cpgswin(xmin, xmax, ymin, ymax);
	cpgsci(0);	cpgrect(xmin, xmax, ymin, ymax);
	cpgsci(VALUE_COL);	cpgsch(2.0);
	if( *index_ptr == -1){
		cpgtext(0.0, 0.0, "^.^");
	} else {
		cpgtext(0.0, 0.0, ".^.");
	}

	*index_ptr *= -1;
	return;
}
/*
---------------------------------------------------- DISP CURRENT TIME
*/
disp_general_info(fxsync_ptr)
	struct shared_mem1 *fxsync_ptr;  /* Pointer of Shared Memory	*/
{
	int		index;					/* General Index			*/
	char	pg_text[64];			/* Text to be Plot			*/
	static char categ[11][16] = {
		"validity",		"fxstart_utc",	"fxstart_tss",	"fxstart_256",
		"fxstart_128",	"fxstart_64",	"utcrst_soy",	"master_soy",
		"master_clk",	"master_tss",	"master_frc"};

	world_bottom= 0.60;	world_top  = 0.95;
	world_left	= 0.05; world_right= 0.30;
	xmin = 0.0;		xmax = 1.0;
	ymin = 1.0;		ymax = 0.0;
	cpgsvp( world_left, world_right, world_bottom, world_top ); 
	cpgswin(xmin, xmax, ymin, ymax);

	cpgsci(BACK_COL);	cpgrect(xmin, xmax, ymin, ymax);
	cpgsci(FRAME_COL);	cpgbox("BC", 0.0, 0, "BC", 0.0, 0);

	cpgsci(CATEG_COL);	x_text = 0.05; cpgsch(1.0);

	for(index=0; index<11; index++){
		y_text = (float)(index+1)/12;	sprintf( pg_text, categ[index]);
					cpgtext(x_text, y_text, pg_text);
	}

	cpgsci(VALUE_COL);	x_text = 0.55; cpgsch(1.0);
	y_text = 1.0/12;	sprintf( pg_text, "%03d", fxsync_ptr->validity);
					cpgtext(x_text, y_text, pg_text);

	y_text = 2.0/12;	sprintf( pg_text, "%09d", fxsync_ptr->fxstart_utc);
					cpgtext(x_text, y_text, pg_text);

	y_text = 3.0/12;	sprintf( pg_text, "%08d", fxsync_ptr->fxstart_tss);
					cpgtext(x_text, y_text, pg_text);

	y_text = 4.0/12;	sprintf( pg_text, "%08d", fxsync_ptr->fxstart_tss_256);
					cpgtext(x_text, y_text, pg_text);

	y_text = 5.0/12;	sprintf( pg_text, "%08d", fxsync_ptr->fxstart_tss_128);
					cpgtext(x_text, y_text, pg_text);

	y_text = 6.0/12;	sprintf( pg_text, "%08d", fxsync_ptr->fxstart_tss_64);
					cpgtext(x_text, y_text, pg_text);

	y_text = 7.0/12;	sprintf( pg_text, "%09d", fxsync_ptr->utcrst_soy);
					cpgtext(x_text, y_text, pg_text);

	y_text = 8.0/12;	sprintf( pg_text, "%09d", fxsync_ptr->master_soy);
					cpgtext(x_text, y_text, pg_text);

	y_text = 9.0/12;	sprintf( pg_text, "%02d", fxsync_ptr->master_clk);
					cpgtext(x_text, y_text, pg_text);

	y_text =10.0/12;	sprintf( pg_text, "%07d", fxsync_ptr->master_tss);
					cpgtext(x_text, y_text, pg_text);

	y_text =11.0/12;	sprintf( pg_text, "%08d", fxsync_ptr->master_frc);
					cpgtext(x_text, y_text, pg_text);

	return(0);
}

/*
---------------------------------------------------- DISP GPIB INFO
*/
disp_gpib_info(fxsched_ptr)
	struct shared_mem2 *fxsched_ptr;  /* Pointer of Shared Memory	*/
{
	int		index;					/* General Index			*/
	char	pg_text[64];			/* Text to be Plot			*/

	world_bottom= 0.80;	world_top  = 0.95;
	world_left	= 0.35; world_right= 0.90;
	xmin = 0.0;		xmax = 1.0;
	ymin = 1.0;		ymax = 0.0;
	cpgsvp( world_left, world_right, world_bottom, world_top ); 
	cpgswin(xmin, xmax, ymin, ymax);

	cpgsci(BACK_COL);	cpgrect(xmin, xmax, ymin, ymax);
	cpgsci(FRAME_COL);	cpgbox("BC", 0.0, 0, "BC", 0.0, 0);

	cpgsci(CATEG_COL);	x_text = 0.02; cpgsch(1.0);

	y_text = 2.0/6;	sprintf( pg_text, "tcu_addr");
					cpgtext(x_text, y_text, pg_text);

	y_text = 3.0/6;	sprintf( pg_text, "dms_addr");
					cpgtext(x_text, y_text, pg_text);

	y_text = 4.0/6;	sprintf( pg_text, "dms_id");
					cpgtext(x_text, y_text, pg_text);

	y_text = 5.0/6;	sprintf( pg_text, "pos_id");
					cpgtext(x_text, y_text, pg_text);


	y_text = 1.0/6;
		for(index=0; index<11; index++){
			x_text = (float)index/14.0 + 0.2;
			sprintf( pg_text, "%2d", index);
			cpgtext(x_text, y_text, pg_text);
		}

	cpgsci(VALUE_COL);	cpgsch(1.0);
	y_text = 2.0/6;
		for(index=0; index<11; index++){
			x_text = (float)index/14.0 + 0.2;
			sprintf( pg_text, "%02d", fxsched_ptr->tcu_addr[index]);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 3.0/6;
		for(index=0; index<5; index++){
			x_text = (float)index/14.0 + 0.2;
			sprintf( pg_text, "%02d", fxsched_ptr->dms_addr[index]);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 4.0/6;
		for(index=1; index<11; index++){
			x_text = (float)index/14.0 + 0.2;
			sprintf( pg_text, "%02d", fxsched_ptr->dms_id[index]);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 5.0/6;
		for(index=1; index<11; index++){
			x_text = (float)index/14.0 + 0.2;
			sprintf( pg_text, "%02d", fxsched_ptr->pos[index]);
			cpgtext(x_text, y_text, pg_text);
		}

	return(0);
}

/*
---------------------------------------------------- DISP GPIB INFO
*/
disp_event_info(fxsync_ptr)
	struct shared_mem1 *fxsync_ptr;  /* Pointer of Shared Memory	*/
{
	int		index;					/* General Index			*/
	int		cart_stat;				/* Cart Status				*/
	char	pg_text[64];			/* Text to be Plot			*/

	static char categ[20][16] = {
		"utcrst_active","tssset_active","fxstrt_active","cart_event",
		"cart_source",	"cart_dest",	"tape_home",	"tape_dest",
		"load_pb_id",	"insert_enable","insert_done",	"rewind_enable",
		"rewind_done",	"search_enable","search_done",	"utcset_enable",
		"utcset_done",	"tlrun_enable",	"current_idr",	"HIDOI"};

	world_bottom= 0.35;	world_top  = 0.75;
	world_left	= 0.35; world_right= 0.90;
	xmin = 0.0;		xmax = 1.0;
	ymin = 1.0;		ymax = 0.0;
	cpgsvp( world_left, world_right, world_bottom, world_top ); 
	cpgswin(xmin, xmax, ymin, ymax);

	cpgsci(BACK_COL);	cpgrect(xmin, xmax, ymin, ymax);
	cpgsci(FRAME_COL);	cpgbox("BC", 0.0, 0, "BC", 0.0, 0);

	cpgsci(CATEG_COL);	x_text = 0.02; cpgsch(1.0);

	for(index=0; index<20; index++){
		y_text = (float)(index+2.0)/21;	sprintf( pg_text, categ[index]);
						cpgtext(x_text, y_text, pg_text);
	}


	y_text = 1.0/21;
		for(index=0; index<11; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%2d", index);
			cpgtext(x_text, y_text, pg_text);
		}

	cpgsci(VALUE_COL);	cpgsch(1.0);
	y_text = 2.0/21; x_text = 0.30;
		sprintf( pg_text, "%02d", fxsync_ptr->utcrst_active);
		cpgtext(x_text, y_text, pg_text);

	y_text = 3.0/21; x_text = 0.30;
		sprintf( pg_text, "%02d", fxsync_ptr->tssset_active);
		cpgtext(x_text, y_text, pg_text);

	y_text = 4.0/21; x_text = 0.30;
		sprintf( pg_text, "%02d", fxsync_ptr->fxstrt_active);
		cpgtext(x_text, y_text, pg_text);

	y_text = 5.0/21;
		for(index=0; index<5; index++){
			x_text = (float)index/16.0 + 0.30;
			cart_stat = fxsync_ptr->cart_event[index][0];
			if(fxsync_ptr->cart_event[index][1] > cart_stat){
				cart_stat = fxsync_ptr->cart_event[index][1]; }
			sprintf( pg_text, "%02d", cart_stat);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 6.0/21;
		for(index=0; index<5; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%02d", fxsync_ptr->cart_source[index][0]);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 7.0/21;
		for(index=0; index<5; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%02d", fxsync_ptr->cart_dest[index][0]);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 8.0/21;
		for(index=0; index<11; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%02d", fxsync_ptr->tape_home[index]);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 9.0/21;
		for(index=0; index<11; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%02d", fxsync_ptr->tape_dest[index]);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 10.0/21;
		for(index=0; index<5; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%02d", fxsync_ptr->load_pb_id[index][0]);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 11.0/21;
		for(index=0; index<11; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%1d",
					(fxsync_ptr->insert_enable & PBMASK) >> index);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 12.0/21;
		for(index=0; index<11; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%1d",
					(fxsync_ptr->insert_done & PBMASK) >> index);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 13.0/21;
		for(index=0; index<11; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%1d",
					(fxsync_ptr->rewind_enable & PBMASK) >> index);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 14.0/21;
		for(index=0; index<11; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%1d",
					(fxsync_ptr->rewind_done & PBMASK) >> index);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 15.0/21;
		for(index=0; index<11; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%1d",
					(fxsync_ptr->search_enable & PBMASK) >> index);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 16.0/21;
		for(index=0; index<11; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%1d",
					(fxsync_ptr->search_done & PBMASK) >> index);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 17.0/21;
		for(index=0; index<11; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%1d",
					(fxsync_ptr->utcset_enable & PBMASK) >> index);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 18.0/21;
		for(index=0; index<11; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%1d",
					(fxsync_ptr->utcset_done & PBMASK) >> index);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 19.0/21;
		for(index=0; index<11; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%1d",
					(fxsync_ptr->tlrun_enable & PBMASK) >> index);
			cpgtext(x_text, y_text, pg_text);
		}

	y_text = 20.0/21;
		for(index=0; index<11; index++){
			x_text = (float)index/16.0 + 0.30;
			sprintf( pg_text, "%02d", fxsync_ptr->current_idr[index]);
			cpgtext(x_text, y_text, pg_text);
		}

	return(0);
}
