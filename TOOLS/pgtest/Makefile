#----------------- LINK OPTIONS -------------------
SYSTEM=sol2
INCLUDE= /sys01/pds/pgplot_frt
#CFLAGS= -I$(INCLUDE) -O4
CFLAGS= -I$(INCLUDE)
#FCOMPL=frt -O3
FCOMPL=frt
CCOMPL=fcc $(CFLAGS)
#------- Followings are PASS or DIRECTORY -------
ACDIR=	../bin
PGDIR=	/sys01/pds/pgplot_frt
CPGDIR=	/sys01/pds/pgplot_frt
PGPLOT=	$(PGDIR)/libpgplot.a
CPGPLOT=$(CPGDIR)/libcpgplot.a
GRLIBS=	-L/usr/openwin/lib -lX11 -lfssl2 
MATH=	/usr/lib/libm.a
#----------------- EXECUTABLE MODULES ------------------------
PROGS=	pgtest spectest lev4 beam erftest resolv_freq
#----------------- OBJECTS ------------------------
OBJ_TEST=	pgtest.o
OBJ_SPEC=	spectest.o cpg_incr.o
OBJ_LEV4=	lev4.o cpg_incr.o lev4corr.o
OBJ_BEAM=	beam.o cpg_incr.o
OBJ_ELFTEST=	erftest.o
OBJ_RESOLV=	resolv_freq.o
#----------------- LINK ------------------------
pgtest : $(OBJ_TEST)
	$(FCOMPL) -o $@ $(OBJ_TEST) $(CPGPLOT) $(PGPLOT) $(MATH) $(GRLIBS)

spectest : $(OBJ_SPEC)
	$(FCOMPL) -o $@ $(OBJ_SPEC) $(CPGPLOT) $(PGPLOT) $(MATH) $(GRLIBS)

lev4 : $(OBJ_LEV4)
	$(FCOMPL) -o $@ $(OBJ_LEV4) $(CPGPLOT) $(PGPLOT) $(MATH) $(GRLIBS)

beam : $(OBJ_BEAM)
	$(FCOMPL) -o $@ $(OBJ_BEAM) $(CPGPLOT) $(PGPLOT) $(MATH) $(GRLIBS)

erftest : $(OBJ_ELFTEST)
	$(CCOMPL) -o $@ $(OBJ_ELFTEST) $(MATH) $(GRLIBS)

resolv_freq : $(OBJ_RESOLV)
	$(FCOMPL) -o $@ $(OBJ_RESOLV) $(CPGPLOT) $(PGPLOT) $(MATH) $(GRLIBS)

clean :
	$(RM) $(PROGS) *.o a.out core *.trace

all :	$(PROGS)

install :
	@mv $(PROGS) $(ACDIR)
#----------------- COMPILE ------------------------
.c.o:
	$(CCOMPL) -c $*.c
pgtest.o:	pgtest.c
spectest.o:	spectest.c
lev4.o:		lev4.c
beam.o:		beam.c
cpg_incr.o:	cpg_incr.c
erftest.o:		erftest.c
resolv_freq.o:	resolv_freq.c

.f.o:
	$(FCOMPL) -c $*.f
lev4corr.o:	lev4corr.f
#----------------- End of File --------------------
