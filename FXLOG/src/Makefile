#----------------- LINK OPTIONS -------------------
SYSTEM=sol2
INCLUDE=
CPPFLAGS= -I. $(INCLUDE)
CFLAGS= -g -I /usr/include -I. -DMAIN
FCOMPL=frt 
CCOMPL=fcc
#CCOMPL=fcc -g -DDEBUG
#------- Followings are PASS or DIRECTORY -------
BINDIR=	/sys01/custom/bin
PGDIR=	/sys01/pds/pgplot
PGPLOT=	$(PGDIR)/libpgplot.a
GRLIBS=	-L/usr/openwin/lib -lX11 -lfssl2 -L/sys01/custom/bin -lFXCORR
CODALIB= /sys01/custom/coda/CODA/FS/LIB/BIN/libcfs.a /sys01/custom/coda/CODA/FS/LIB/DSMD/d_link.a -lm -lc
PROGS=  fxvisp fxapri fxphase fxlog
MATH=	/usr/lib/libm.a
#----------------- MAPPING ------------------------
OBJ_FXVISP=		fxvisp.o fxvisp_param.o
OBJ_FXAPRI=		fxapri.o
OBJ_FXPHASE=	fxphase.o
OBJ_FXLOG=		fxlog.o fxlog_param.o fxlog_read_open.o fxlog_write_open.o \
				fxlog_read.o fxlog_selector.o fxlog_scan.o fxlog_input.o \
				fxlog_detect_chapter.o fxlog_detect_source.o \
				fxlog_drg_read.o fxlog_read_exper.o fxlog_read_source.o \
				fxlog_read_station.o fxlog_read_code.o fxlog_read_lo.o \
				fxlog_read_bbc.o fxlog_read_form.o fxlog_read_wx.o \
				fxlog_read_st.o fxlog_read_et.o fxlog_read_sked.o \
				fxlog_read_label.o fxlog_read_param.o \
				fxlog_write_exper.o fxlog_write_station.o fxlog_write_lo.o \
				fxlog_write_bbc.o fxlog_write_form.o fxlog_write_wx.o \
				fxlog_write_st.o fxlog_write_et.o fxlog_write_source.o \
				fxlog_write_label.o \
				pow2round.o
#----------------- Compile and link ------------------------
fxvisp : $(OBJ_FXVISP)
	$(FCOMPL) -o $@ $(OBJ_FXVISP) $(CODALIB) $(PGPLOT) $(MATH) $(GRLIBS)

fxapri : $(OBJ_FXAPRI)
	$(FCOMPL) -o $@ $(OBJ_FXAPRI) $(PGPLOT) $(MATH) $(GRLIBS)

fxphase : $(OBJ_FXPHASE)
	$(FCOMPL) -o $@ $(OBJ_FXPHASE) $(PGPLOT) $(MATH) $(GRLIBS)

fxlog : $(OBJ_FXLOG)
	$(CCOMPL) -o $@ $(OBJ_FXLOG) $(MATH) $(GRLIBS)

clean :
	$(RM) $(PROGS) *.o *.a a.out core *.trace

all :	$(PROGS)

install :
	@mv $(PROGS) $(BINDIR)
#----------------- Objects ------------------------
.c.o:
	$(CCOMPL) -c $*.c

fxvisp.o:			fxvisp.c		fxvisp_param.inc	coda.inc
fxvisp_param.o:		fxvisp_param.c	fxvisp_param.inc
fxapri.o:			fxapri.c
fxphase.o:			fxphase.c
fxlog.o:			fxlog.c				fxlog.inc
fxlog_param.o:		fxlog_param.c		fxlog.inc
fxlog_read_open.o:	fxlog_read_open.c
fxlog_write_open.o:	fxlog_write_open.c
fxlog_read.o:		fxlog_read.c		fxlog.inc
fxlog_selector.o:	fxlog_selector.c	fxlog.inc
fxlog_scan.o:		fxlog_scan.c		fxlog.inc
fxlog_input.o:		fxlog_input.c		fxlog.inc
fxlog_drg_read.o:	fxlog_drg_read.c	fxlog.inc
fxlog_detect_chapter.o:	fxlog_detect_chapter.c	fxlog.inc
fxlog_detect_source.o:	fxlog_detect_source.c	fxlog.inc
fxlog_read_exper.o:		fxlog_read_exper.c		fxlog.inc
fxlog_read_source.o:	fxlog_read_source.c		fxlog.inc
fxlog_read_station.o:	fxlog_read_station.c	fxlog.inc
fxlog_read_sked.o:		fxlog_read_sked.c		fxlog.inc
fxlog_read_code.o:		fxlog_read_code.c		fxlog.inc
fxlog_read_lo.o:		fxlog_read_lo.c			fxlog.inc
fxlog_read_bbc.o:		fxlog_read_bbc.c		fxlog.inc
fxlog_read_form.o:		fxlog_read_form.c		fxlog.inc
fxlog_read_wx.o:		fxlog_read_wx.c			fxlog.inc
fxlog_read_st.o:		fxlog_read_st.c			fxlog.inc
fxlog_read_et.o:		fxlog_read_et.c			fxlog.inc
fxlog_read_label.o:		fxlog_read_label.c		fxlog.inc
fxlog_write_exper.o:	fxlog_write_exper.c		fxlog.inc
fxlog_write_source.o:	fxlog_write_source.c	fxlog.inc
fxlog_write_station.o:	fxlog_write_station.c	fxlog.inc
fxlog_write_lo.o:		fxlog_write_lo.c		fxlog.inc
fxlog_write_bbc.o:		fxlog_write_bbc.c		fxlog.inc
fxlog_write_form.o:		fxlog_write_form.c		fxlog.inc
fxlog_write_wx.o:		fxlog_write_wx.c		fxlog.inc
fxlog_write_st.o:		fxlog_write_st.c		fxlog.inc
fxlog_write_et.o:		fxlog_write_et.c		fxlog.inc
fxlog_write_label.o:	fxlog_write_label.c		fxlog.inc
pow2round.o:			pow2round.c
#----------------- End of File --------------------
