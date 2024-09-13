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
PROGS=  vlbalog 
MATH=	/usr/lib/libm.a
#----------------- MAPPING ------------------------
OBJ_VLBALOG=	vlbalog.o clg_input.o scan_log.o
#----------------- Compile and link ------------------------
vlbalog : $(OBJ_VLBALOG)
	$(CCOMPL) -o $@ $(OBJ_VLBALOG) $(MATH) $(GRLIBS)

clean :
	$(RM) $(PROGS) *.o *.a a.out core *.trace

all :	$(PROGS)

install :
	@mv $(PROGS) $(BINDIR)
#----------------- Objects ------------------------
.c.o:
	$(CCOMPL) -c $*.c

vlbalog.o:			vlbalog.c		vlbalog.inc
clg_input.o:		clg_input.c		vlbalog.inc
scan_log.o:			scan_log.c		vlbalog.inc
#----------------- End of File --------------------
