#----------------- LIBLARY ------------------------
PGDIR = /sys01/pds/pgplot_frt
CPGDIR = /sys01/pds/pgplot_frt
PGPLOT= $(PGDIR)/libpgplot.a
CPGPLOT= $(CPGDIR)/libcpgplot.a
MATH=/usr/lib/libm.a
INCLUDE= /sys01/pds/pgplot
GRLIBS= -L/sys01/custom/bin -lFXCORR -lfssl2 -L/usr/openwin/lib -lX11 
#----------------- COMPILE OPTIONS ----------------
CFLAGS = -I $(INCLUDE) -O3
FFLAGS = -O4
BINDIR = /sys01/custom/bin
SYSTEM = sol2
CCOMPL = fcc $(CFLAGS) $(CPPFLAGS)
FCOMPL = frt $(FFLAGS)
AR = ar ru
PROGS  = pcalfit pcaldiff bincat dos2unix unix2dos gfftest gff_mult naoco_visp visib visedit uvtest read_mrg modedit modelvis delayplot delaycal b1950test j2000test redshift tss
#----------------- OBJECTS ------------------------
OBJ_j2000test=	j2000test.o
OBJ_b1950test=	b1950test.o
OBJ_delaycal=	delaycal.o naoco_open.o naoco_header.o naoco_skip.o naoco_vis.o
OBJ_delayplot=	delayplot.o
OBJ_merge=	read_mrg.o
OBJ_uvtest=	uvtest.o xyz2uvw.o
OBJ_visib=	visib.o
OBJ_modedit=	modedit.o comp2imag.o comp2corner.o select_comp.o
OBJ_modelvis=	modelvis.o comp2imag.o comp2corner.o xyz2uvw.o
OBJ_pcalfit=	pcalfit.o
OBJ_pcaldiff=	pcaldiff.o
OBJ_bincat=	bincat.o
OBJ_dos2unix=	dos2unix.o
OBJ_unix2dos=	unix2dos.o
OBJ_naoco_visp=	naoco_visp.o blvis.o coarse_mult.o phase_mult.o gff_mult.o sqr_fit.o
#OBJ_gfftest=	gfftest.o blvis.o
OBJ_gfftest=	gfftest.o blvis.o coarse.o gff.o
OBJ_gff_mult=	gfftest_mult.o naoco_open.o naoco_header.o naoco_skip.o naoco_vis.o sqr_fit.o
#OBJ_gff_mult=	gfftest_mult.o blvis.o coarse_mult.o integ_mult.o gff_mult.o sqr_fit.o
OBJ_sqr=		sqrtest.o sqr_fit.o
OBJ_redshift=	redshift.o
OBJ_tss=	tss.o
#----------------- LINK ------------------------
j2000test: $(OBJ_j2000test)
	$(CCOMPL) -o $@ $(OBJ_j2000test) $(GRLIBS) $(CPGPLOT) $(PGPLOT) $(MATH)

b1950test: $(OBJ_b1950test)
	$(CCOMPL) -o $@ $(OBJ_b1950test) $(GRLIBS) $(CPGPLOT) $(PGPLOT) $(MATH)

delaycal: $(OBJ_delaycal)
	$(FCOMPL) -o $@ $(OBJ_delaycal) $(GRLIBS) $(CPGPLOT) $(PGPLOT) $(MATH)

delayplot: $(OBJ_delayplot)
	$(FCOMPL) -o $@ $(OBJ_delayplot) $(GRLIBS) $(CPGPLOT) $(PGPLOT) $(MATH)

read_mrg: $(OBJ_merge)
	$(CCOMPL) -o $@ $(OBJ_merge) $(GRLIBS) $(CPGPLOT) $(PGPLOT) $(MATH)

uvtest: $(OBJ_uvtest)
	$(FCOMPL) -o $@ $(OBJ_uvtest) $(GRLIBS) $(CPGPLOT) $(PGPLOT) $(MATH)

modedit: $(OBJ_modedit)
	$(FCOMPL) -o $@ $(OBJ_modedit) $(GRLIBS) $(CPGPLOT) $(PGPLOT) $(MATH)

modelvis: $(OBJ_modelvis)
	$(FCOMPL) -o $@ $(OBJ_modelvis) $(GRLIBS) $(CPGPLOT) $(PGPLOT) $(MATH)

visib: $(OBJ_visib)
	$(FCOMPL) -o $@ $(OBJ_visib) $(GRLIBS) $(CPGPLOT) $(PGPLOT) $(MATH)

gfftest: $(OBJ_gfftest) $(PGPLOT)
	$(FCOMPL) -o $@ $(OBJ_gfftest) $(GRLIBS) $(CPGPLOT) $(PGPLOT) $(MATH)

gff_mult: $(OBJ_gff_mult) $(PGPLOT)
	$(FCOMPL) -o $@ $(OBJ_gff_mult) $(GRLIBS) $(CPGPLOT) $(PGPLOT) $(MATH)

naoco_visp: $(OBJ_naoco_visp) $(PGPLOT)
	$(FCOMPL) -o $@ $(OBJ_naoco_visp) $(GRLIBS) $(CPGPLOT) $(PGPLOT) $(MATH)

bincat: $(OBJ_bincat)
	$(CCOMPL) -o $@ $(OBJ_bincat)

dos2unix: $(OBJ_dos2unix)
	$(CCOMPL) -o $@ $(OBJ_dos2unix)

unix2dos: $(OBJ_unix2dos)
	$(CCOMPL) -o $@ $(OBJ_unix2dos)

sqrtest: $(OBJ_sqr)
	$(CCOMPL) -o $@ $(OBJ_sqr)

pcalfit: $(OBJ_pcalfit) $(PGPLOT)
	$(FCOMPL) -o $@ $(OBJ_pcalfit) $(CPGPLOT) $(PGPLOT) $(GRLIBS) $(MATH)

pcaldiff: $(OBJ_pcaldiff) $(PGPLOT)
	$(FCOMPL) -o $@ $(OBJ_pcaldiff) $(CPGPLOT) $(PGPLOT) $(GRLIBS) $(MATH)

redshift: $(OBJ_redshift)
	$(CCOMPL) -o $@ $(OBJ_redshift) $(GRLIBS) $(MATH)

tss: $(OBJ_tss)
	$(CCOMPL) -o $@ $(OBJ_tss) $(GRLIBS) $(MATH)

install:
	@mv $(PROGS) $(BINDIR)

clean:
	$(RM) $(PROGS) *.o a.out core *.trace
#----------------- COMPILE ------------------------
.c.o:
	$(CCOMPL) -c $*.c
bincat.o:		bincat.c
dos2unix.o:		dos2unix.c
unix2dos.o:		unix2dos.c
pcalfit.o:		pcalfit.c
pcaldiff.o:		pcaldiff.c
gfftest.o:		gfftest.c		delaydata.inc
gfftest_mult.o:	gfftest_mult.c	delaydata.inc
blvis.o:		blvis.c			naoco.inc delaydata.inc
naoco_open.o:	naoco_open.c	naoco.inc delaydata.inc
naoco_header.o:	naoco_header.c	naoco.inc delaydata.inc
naoco_skip.o:	naoco_skip.c	naoco.inc delaydata.inc
naoco_vis.o:	naoco_vis.c		naoco.inc delaydata.inc
gff.o:			gff.c
naoco_visp.o:	naoco_visp.c
gff_acc.o:		gff_acc.c
coarse.o:		coarse.c
sqrtest.o:		sqrtest.c
visib.o:		visib.c		pgplot.inc
modedit.o:		modedit.c	pgplot.inc
modelvis.o:		modelvis.c	pgplot.inc
comp2imag.o:	comp2imag.c
comp2corner.o:	comp2corner.c
select_comp.o:	select_comp.c
xyz2uvw.o:		xyz2uvw.c
uvtest.o:		uvtest.c
read_mrg.o:		read_mrg.c	merge.inc
delayplot.o:	delayplot.c
delaycal.o:		delaycal.c
b1950test.o:	b1950test.c
j2000test.o:	j2000test.c
b1950toj2000.o:	b1950toj2000.c
j2000tob1950.o:	j2000tob1950.c
redshift.o:	redshift.c
tss.o:	tss.c
#----------------- End of File --------------------
