#include <stdio.h>				/* Standard I/O					*/
#include <sys/types.h>			/* InterProcess Communication	*/ 
#include <sys/ipc.h>			/* InterProcess Communication	*/
#include <sys/shm.h>			/* Shared Memory				*/
#include <sys/msg.h>			/* Message Queue				*/

/* ----- FOR TURUGI
#define	READ_SOUND	"rsh vsopsv -l fxcorr audioplay -v 40 /home/fxcorr/Read.au"
#define	START_SOUND	"rsh vsopsv -l fxcorr audioplay -v 40 /home/fxcorr/Start.au"
#define	WARN_SOUND	"rsh vsopsv -l fxcorr audioplay -v 40 /home/fxcorr/Warning.au"
#define	ERROR_SOUND	"rsh vsopsv -l fxcorr audioplay -v 40 /home/fxcorr/Error.au"
#define	END_SOUND	"rsh vsopsv -l fxcorr audioplay -i -v 40 /home/fxcorr/Finish.au"
#define	REWEND_SOUND	"rsh vsopsv -l fxcorr audioplay -i -v 40 /home/fxcorr/RewindEnd.au"
------- */

/*-------- FOR FXSYNC --------*/
#define	READ_SOUND	"audioplay -v 70 /home/fxcorr/Read.au"
#define	START_SOUND	"audioplay -v 70 /home/fxcorr/Start.au"
#define	WARN_SOUND	"audioplay -v 70 /home/fxcorr/Warning.au"
#define	ERROR_SOUND	"audioplay -v 70 /home/fxcorr/Error.au"
#define	END_SOUND	"audioplay -i -v 70 /home/fxcorr/Finish.au"
#define	REWEND_SOUND	"audioplay -i -v 70 /home/fxcorr/RewindEnd.au"

#define	FXSYNC_KEY1	4096
#define	FXSYNC_KEY2	8192
#define	FXSYNC_KEY3	16384
#define	DMS_BIN		16
#define	MAX_BIN		24
#define	MAX_CART	5
#define	MAX_PB		16

#define	UNUSE		0
#define	USE			1

/*---------------------------------------------- UTCSET EVENT BIT --------*/

/*---------------------------------------------- Event Table --------*/
#define	UNDEF		0
#define	DISABLE		1
#define	ENABLE		2
#define	ACTIVE		4
#define	RESERVED	8
#define	DONE		16
#define	CONFIRMED	32
#define	FINISH		127
#define	ABSFIN		255

/*---------------------------------------------- Color Table --------*/
#define	WHITE_COL		1		/* WHITE	*/
#define	UNDEF_COL		14		/* GREY		*/
#define	DISABLE_COL		2		/* RED		*/
#define	ENABLE_COL		3		/* GREEN	*/
#define	RESERVED_COL	7		/* YELLOW	*/
#define	ACTIVE_COL		6		/* VIOLET	*/
#define	DONE_COL		8		/* ORCHERD	*/
#define	CONFIRMED_COL	5		/* LIGHT BLUE*/
#define	FINISH_COL		4		/* BLUE		*/

/*---------------------------------------------- DIR-1000 STATUS ----*/
#define	STANDBY_STAT	0x00000020
#define	FWD_STAT		0x00000001

/*---------------------------------------------- MESSAGE SIZE ----*/
#define MSG_SIZ			64


/*---------------------------------------------- COMMAND PATH ----*/
#define	FXSYNC_SHM	"/sys01/custom/bin/fxsync_shm"
#define	PBSCHED		"/sys01/custom/bin/read_pbsched"
#define	TCU_MASTER	"/sys01/custom/bin/tcu_master_cntl"
#define	TCU_SLAVE	"/sys01/custom/bin/tcu_slave_cntl"
#define	TCU_REWIND	"/sys01/custom/bin/tcu_rewind_cntl"
#define	DMS_CNTL	"/sys01/custom/bin/dms_cntl"

/*-------- STRUCT of SHARED MEMORY --------*/
struct	shared_mem1{
	int					validity;				/* Data Validity		*/
	int					work_enable;			/* Working Status		*/
	int					polarity;				/* Clock Polarity		*/
	int					pb_usage;				/* USE->1, UNUSE->0		*/
	int					cart_usage;				/* USE->1, UNUSE->0		*/
	int					start_soy;				/* SOY at START			*/
	int					stop_soy;				/* SOY at END			*/
	int					fxstart_utc;			/* UTC at FXSTART		*/
	int					fxstart_tss;			/* TSSID at FXSTART		*/
	int					fxstart_tss_256;		/* TSS at FXSTART		*/
	int					fxstart_tss_128;		/* TSS at FXSTART		*/
	int					fxstart_tss_64;			/* TSS at FXSTART		*/
	int					utcrst_soy;				/* UTCRESET Pulse Timing*/
	unsigned int		master_soy;				/* Current Master UTC	*/
	int					master_clk;				/* MASTER Clock			*/
	unsigned int		master_tss;				/* MASTER TSSID			*/
	unsigned int		master_frc;				/* MASTER FRACTION		*/
	unsigned int		slave_soy[MAX_PB];		/* Current Slave UTC	*/
	int					slave_clk[MAX_PB];		/* Current Slave UTC	*/
	int					curr_scan[MAX_PB];		/* Current Scan (/ST/)	*/
	int					curr_tape[MAX_PB];		/* Current Tape Number	*/

	/*-------- EVENT INFORMATION --------*/
	int		utcrst_active;			/* UTCRST Pulse Activity		*/
	int		tssset_active;			/* TSSSET Pulse Activity		*/
	int		fxstrt_active;			/* FXSTART Pulse Activity		*/
	int		cart_event[MAX_CART][2];	/* Event Status of Carts		*/
	int		cart_source[MAX_CART][2];	/* Source BIN ID				*/
	int		cart_dest[MAX_CART][2];		/* Destination DIR ID			*/
	int		load_pb_id[MAX_CART][2];	/* Currently Loading PB ID		*/
	int		tape_home[MAX_PB];		/* Home Bin of Current Tape		*/
	int		tape_dest[MAX_PB];		/* Current Tape Position		*/
	int		insert_enable;			/* Tape Load Enable in PB		*/
	int		insert_done;			/* Tape Load Finish				*/
	int		eject_enable;			/* Tape Eject Enable in PB		*/
	int		eject_done;				/* Tape Eject Finish			*/
	int		rewind_enable;			/* Initial Tape Rewind			*/
	int		rewind_done;			/* Tape Rewind Finish			*/
	int		search_enable;			/* Tape Search to ST point		*/
	int		search_done;			/* Tape Search Finish			*/
	int		utcset_enable;			/* UTCSET Enable Flag			*/
	int		utcset_done;			/* UTCSET Event Done			*/
	int		tlrun_enable;			/* Timeline Play Event Permission*/
	int		tlrun_done;				/* Timeline Play Event Reserved	*/
	int		tape_sync;				/* Synchronized Play Back		*/
	int		current_idr[MAX_PB];	/* Current IDR of the DIR		*/
};

struct	shared_mem2{
	/*-------- PROCESS INFORMATION --------*/
	int		tcu_cntl_exec;			/* 0 -> Can't Exec  1->Exec		*/
	int		tcu_pid[MAX_PB];		/* Process ID of tcu_cntl		*/
	int		dms_pid[MAX_CART];		/* Process ID of dms_cntl		*/

	/*-------- GPIB INFORMATION --------*/
	int		tcu_board[MAX_PB];		/* GPIB Address of TCU			*/
	int		tcu_addr[MAX_PB];		/* GPIB Address of TCU			*/
	int		dms_board[MAX_CART];	/* GPIB Address of TCU			*/
	int		dms_addr[MAX_CART];		/* GPIB Address of TCU			*/
	int		dms_id[MAX_PB];			/* DMS ID for Each PB			*/
	int		pos[MAX_PB];			/* DIR Position in DMS-16/24	*/

	/*-------- CART BIN INFORMATION --------*/
	char	binlabel[MAX_CART][MAX_BIN][16];	/* Tape Label in Bins		*/
	char	dirlabel[MAX_CART][2][16];			/* Tape Label in DIR		*/
	int		dir_id[MAX_CART][2];				/* PB ID of installed DIR	*/

	/*-------- TAPE INFORMATION --------*/
	int		tape_num[MAX_PB];					/* Number of Tapes		*/
	char	tape_label[MAX_PB][MAX_BIN][16];	/* Tape Label List		*/

	/*-------- SYNC SCHEDULE --------*/
	int		st_num[MAX_PB];				/* Number of ST Events				*/
	int		master_tssepc[MAX_PB][256];	/* TSS ID of Master at ST EPOCH		*/
	int		tape_id[MAX_PB][256];		/* Tape ID 							*/
	int		rec_rate[MAX_PB][256];		/* Recorded Rate [Mbps]				*/
	int		play_rate[MAX_PB][256];		/* Playback Rate [Mbps]				*/
	int		rec_utc[MAX_PB][256];		/* Recorded UTC at ST [DDDHHMMSS]	*/
	int		play_utc[MAX_PB][256];		/* Play UTC at ST [DDDHHMMSS]		*/
	int		st_idr[MAX_PB][256];		/* IDR at ST 						*/
	int		et_tss[MAX_PB][256];		/* IDR at ET 						*/
	int		et_idr[MAX_PB][256];		/* IDR at ET 						*/
	int		st_soy[MAX_PB][256];		/* Second of Year at /ST/			*/
	int		et_soy[MAX_PB][256];		/* Second of Year at /ET/			*/
	int		dur_sec[MAX_PB][256];		/* Duration in Sec					*/
};

/*-------- STRUCT of MESSAGE QUEUE --------*/
struct msg_buf{
	int		msg_type;
	char	msg_text[MSG_SIZ];
};
