#----------------- LIBLARY ------------------------
PGDIR = /sys01/pds/pgplot_frt
CPGDIR = /sys01/pds/pgplot_frt
PGPLOT = $(PGDIR)/libpgplot.a
CPGPLOT = $(CPGDIR)/libcpgplot.a
INCLUDE = /sys01/pds/pgplot_frt
MATH=/usr/lib/libm.a
GPIB=	-lgpib
GRLIBS= -L/sys01/custom/bin -lFXCORR -lfssl2 -L/usr/openwin/lib -lX11 
#----------------- COMPILE OPTIONS ----------------
CFLAGS = -I$(INCLUDE) -O3
FFLAGS = -I$(INCLUDE) -O3
BINDIR = /sys01/custom/bin
SYSTEM = sol2
CCOMPL = fcc $(CFLAGS) $(CPPFLAGS)
FCOMPL = frt $(FFLAGS)
PROGS=	fxsync_shm fxsync_view read_pbsched tcu_master_cntl tcu_slave_cntl fxsync_start fxsync_init dms_cntl fxsync_ql fxsync_print fxsync_dump fxsync_stim fxsync_replay rewind_off rewind_start tcu_rewind_cntl tcu_reset
#----------------- MAPPING ------------------------
OBJ_SHM=	fxsync_shm.o

OBJ_VIEW=	fxsync_view.o

OBJ_REPLAY=	fxsync_replay.o

OBJ_PRINT=	fxsync_print.o

OBJ_DUMP=	fxsync_dump.o fsleep.o

OBJ_STIM=	fxsync_stim.o fsleep.o

OBJ_REWOFF=	rewind_off.o fsleep.o

OBJ_TCURESET=	tcu_reset.o fsleep.o

OBJ_QL=		fxsync_ql.o fsleep.o

OBJ_PBSCHED=	read_pbsched.o fxsync_info.o

OBJ_DMSCNTL=	dms_cntl.o board_gpini.o dms_gpini.o fsleep.o dmsbin_scan.o \
				dms_move.o

OBJ_DMSINIT=	dmsinit.o board_gpini.o dms_gpini.o fsleep.o 

OBJ_TCU_MASTER=	tcu_master_cntl.o fsleep.o tcu_init.o dirsend.o \
				tcu_utcset.o tcu_gpini.o tcu_utcrst.o tcu_tssset.o \
				get_TCU_utc.o tcu_fxstart.o check_utc_tss.o

OBJ_TCU_SLAVE=	tcu_slave_cntl.o fsleep.o tcu_init.o \
				tcu_utcset.o tcu_gpini.o tcu_utcrst.o tcu_tssset.o \
				get_TCU_utc.o tcu_fxstart.o scan_bindat.o dirsend.o \
				tss2idr.o get_TCU_tss.o tlplay.o tape_load_request.o \
				tape_rewind.o tape_search.o check_utc_tss.o check_tss_idr.o \
				store_tape.o sched_shift.o

OBJ_TCU_REWIND=	tcu_rewind_cntl.o fsleep.o tcu_init.o \
				tcu_gpini.o scan_bindat.o dirsend.o \
				tape_load_request.o tape_rewind.o  store_tape.o

OBJ_START=		fxsync_start.o fxsync_info.o

OBJ_REWIND_START=		rewind_start.o fxsync_info.o

OBJ_INIT=		fxsync_init.o
#----------------- Compile and link ------------------------
fxsync_shm : $(OBJ_SHM)
	$(CCOMPL) -o $@ $(OBJ_SHM)

fxsync_start : $(OBJ_START)
	$(CCOMPL) -o $@ $(OBJ_START)

rewind_start : $(OBJ_REWIND_START)
	$(CCOMPL) -o $@ $(OBJ_REWIND_START)

tcu_reset : $(OBJ_TCURESET)
	$(CCOMPL) -o $@ $(OBJ_TCURESET)

fxsync_init : $(OBJ_INIT)
	$(CCOMPL) -o $@ $(OBJ_INIT)

fxsync_view : $(OBJ_VIEW)
	$(FCOMPL) -o $@ $(OBJ_VIEW) $(GRLIBS) $(CPGPLOT) $(PGPLOT)

fxsync_replay : $(OBJ_REPLAY)
	$(FCOMPL) -o $@ $(OBJ_REPLAY) $(GRLIBS) $(CPGPLOT) $(PGPLOT)

fxsync_print : $(OBJ_PRINT)
	$(FCOMPL) -o $@ $(OBJ_PRINT) $(GRLIBS) $(CPGPLOT) $(PGPLOT)

fxsync_dump : $(OBJ_DUMP)
	$(CCOMPL) -o $@ $(OBJ_DUMP)

fxsync_stim : $(OBJ_STIM)
	$(CCOMPL) -o $@ $(OBJ_STIM)

rewind_off : $(OBJ_REWOFF)
	$(CCOMPL) -o $@ $(OBJ_REWOFF)

fxsync_ql : $(OBJ_QL)
	$(FCOMPL) -o $@ $(OBJ_QL) $(GRLIBS) $(CPGPLOT) $(PGPLOT)

dms_cntl : $(OBJ_DMSCNTL)
	$(CCOMPL) -o $@ $(OBJ_DMSCNTL) $(GPIB)

#dmsinit : $(OBJ_DMSINIT)
#	$(CCOMPL) -o $@ $(OBJ_DMSINIT) $(GPIB)

#dmstest : $(OBJ_DMSTEST)
#	$(CCOMPL) -o $@ $(OBJ_DMSTEST) $(GPIB)

#dmstest2 : $(OBJ_DMSTEST2)
#	$(CCOMPL) -o $@ $(OBJ_DMSTEST2) $(GPIB)

read_pbsched : $(OBJ_PBSCHED)
	$(CCOMPL) -o $@ $(OBJ_PBSCHED) $(GRLIBS)

tcu_master_cntl : $(OBJ_TCU_MASTER)
	$(CCOMPL) -o $@ $(OBJ_TCU_MASTER) $(GRLIBS) $(GPIB)

tcu_slave_cntl : $(OBJ_TCU_SLAVE)
	$(CCOMPL) -o $@ $(OBJ_TCU_SLAVE) $(GRLIBS) $(GPIB)

tcu_rewind_cntl : $(OBJ_TCU_REWIND)
	$(CCOMPL) -o $@ $(OBJ_TCU_REWIND) $(GRLIBS) $(GPIB)

clean :
	$(RM) $(PROGS) *.o a.out core *.trace

all :	$(PROGS)

install :
	@mv $(PROGS) $(BINDIR)
#----------------- Objects ------------------------
.c.o:
	$(CCOMPL) -c $*.c
board_gpini.o:		board_gpini.c
check_utc_tss.o:	check_utc_tss.c		fxsync.inc
check_tss_idr.o:	check_tss_idr.c		fxsync.inc
dirsend.o:			dirsend.c
dms_cntl.o:			dms_cntl.c			fxsync.inc
dmstest.o:			dmstest.c
dmstest2.o:			dmstest2.c
dmsinit.o:			dmsinit.c
dms_move.o:			dms_move.c
dms_gpini.o:		dms_gpini.c
dmsbin_scan.o:		dmsbin_scan.c
fsleep.o:			fsleep.c	
fxsync_ql.o:		fxsync_ql.c		fxsync.inc
fxsync_shm.o:		fxsync_shm.c		fxsync.inc
fxsync_start.o:		fxsync_start.c		fxsync.inc
fxsync_info.o:		fxsync_info.c		fxsync.inc
fxsync_init.o:		fxsync_init.c		fxsync.inc
fxsync_view.o:		fxsync_view.c		fxsync.inc
fxsync_replay.o:	fxsync_replay.c		fxsync.inc
fxsync_print.o:		fxsync_print.c		fxsync.inc
fxsync_dump.o:		fxsync_dump.c		fxsync.inc
fxsync_stim.o:		fxsync_stim.c		fxsync.inc
get_TCU_utc.o:		get_TCU_utc.c
get_TCU_tss.o:		get_TCU_tss.c
read_pbsched.o:		read_pbsched.c		fxsync.inc
rewind_off.o:		rewind_off.c		fxsync.inc
rewind_start.o:		rewind_start.c		fxsync.inc
sched_shift.o:		sched_shift.c		fxsync.inc
scan_bindat.o:		scan_bindat.c		fxsync.inc
store_tape.o:		store_tape.c		fxsync.inc
tape_load_request.o: tape_load_request.c fxsync.inc
tape_rewind.o:		tape_rewind.c		fxsync.inc
tape_search.o:		tape_search.c		fxsync.inc
tlplay.o:			tlplay.c			fxsync.inc
tcu_master_cntl.o:	tcu_master_cntl.c	fxsync.inc
tcu_slave_cntl.o:	tcu_slave_cntl.c	fxsync.inc
tcu_rewind_cntl.o:	tcu_rewind_cntl.c	fxsync.inc
tcu_init.o:			tcu_init.c
tcu_tcu_gpini.o:	tcu_tcu_gpini.c
tcu_utcset.o:		tcu_utcset.c
tcu_utcrst.o:		tcu_utcrst.c
tcu_tssset.o:		tcu_tssset.c
tcu_fxstart.o:		tcu_fxstart.c
tcu_reset.o:		tcu_reset.c			fxsync.inc
tss2idr.o:			tss2idr.c
#----------------- End of File --------------------
