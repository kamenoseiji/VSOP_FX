#----------------- LIBLARY ------------------------
INCLUDE= /sys01/pds/pgplot_frt
PGDIR = /sys01/pds/pgplot_frt
PGPLOT= $(PGDIR)/libpgplot.a
MATH=/usr/lib/libm.a
#CFSLIB= /sys01/custom/coda/CODA/FS/LIB/BIN/libcfs.a /sys01/custom/coda/CODA/FS/LIB/DSMD/d_link.a
CFSLIB= /home/veracorr/CODA/LIB/libcfs.a
GRLIBS= -L/sys01/custom/bin -lFXCORR -L/usr/openwin/lib -lX11 -lfssl2
#----------------- COMPILE OPTIONS ----------------
BINDIR = /sys01/custom/bin
SYSTEM = sol2
CCOMPL = fcc -I$(INCLUDE) -O4 
FCOMPL = frt -O3
#FCOMPL = frt -DDEBUG
AR = ar ru
LIBS	= libFXCORR_FJ.a
#----------------- OBJECTS ------------------------
OBJ_FXLIBS=		soy2dhms.o dhms2soy.o soy2tss.o utc2tss.o tss2sec.o tss2soy.o \
				doy2mjd.o mjd2doy.o mjd2gmst.o fmjd2doy.o doy2fmjd.o soy2mdhms.o \
				f_sort.o d_sort.o f_index_sort.o d_index_sort.o \
				fastcos.o fastsin.o \
				gst2el.o gst2dsecz.o excess_atm.o fort_excessatm.o \
				detect_chapter.o scan_source.o scan_sked.o scan_station.o \
				scan_code.o closure_solve.o \
				clphs_solve.o calc_el.o calc_delay.o atmgain_solve.o \
				atmgain_get.o calc_bl_delay.o fsleep.o \
				restore_delay.o read_delay.o restore_gain.o read_gain.o \
				restore_pcal.o read_pcal.o interp_real.o \
				ant2bl.o bl2ant.o ant2cl.o cl2ant.o pow2round.o \
				cpgbird.o cpg_incr.o cpgpseud.o cpgpseud_win.o cpg_bp.o \
				coarse.o gff.o \
				coarse_mult.o gff_mult.o gff_mult_acc.o gff_acc.o \
				gff_fine.o integ_mult.o integ_mult_com.o integ_mult_acc.o \
				comp2vis.o memalloc.o memfree.o \
				b1950toj2000.o j2000tob1950.o  sqr_fit.o vanvleck.o \
				fvanvleck.o vanvcalc.o vanvleck2_init.o vanvleck2.o \
				integ_amphi.o integ_fine.o \
				pcal_spline.o get_pcalphase.o real_spline.o \
				fort_doy2mjd.o fort_doy2fmjd.o fort_mjd2doy.o \
				fort_fmjd2doy.o integ_rawbp.o \
				integ_bp.o read_obshead.o read_corhead.o read_sshead.o \
				objct_id.o acorr_pair.o xcorr_pair.o atmdelay_solve.o \
				skip_coda.o skip_flag.o time_span_coda.o \
				block_search.o time_block_coda.o \
				write_sshead.o cfs_ret.o read_bp.o save_bp.o
#----------------- LINK ------------------------
libFXCORR_FJ.a:	$(OBJ_FXLIBS)
	$(AR) $@ $(OBJ_FXLIBS)

install:
	@mv $(LIBS) $(BINDIR)

clean:
	$(RM) *.o a.out core *.trace
#----------------- COMPILE ------------------------
.c.o:
	$(CCOMPL) -c $*.c

acorr_pair.o:		acorr_pair.c		obshead.inc
ant2bl.o:			ant2bl.c
ant2cl.o:			ant2cl.c
atmdelay_solve.o:	atmdelay_solve.c	obshead.inc
atmgain_solve.o:	atmgain_solve.c		obshead.inc
atmgain_get.o:		atmgain_get.c		obshead.inc
bl2ant.o:			bl2ant.c
b1950toj2000.o:		b1950toj2000.c
block_search.o:		block_search.c		obshead.inc
calc_el.o:			calc_el.c			obshead.inc
calc_delay.o:		calc_delay.c		obshead.inc
calc_bl_delay.o:	calc_bl_delay.c		obshead.inc
cfs_ret.o:			cfs_ret.c
closure_solve.o:	closure_solve.c
clphs_solve.o:		clphs_solve.c
cl2ant.o:			cl2ant.c
coarse.o:			coarse.c
coarse_mult.o:		coarse_mult.c
comp2vis.o:			comp2vis.c
cpgbird.o:			cpgbird.c
cpg_incr.o:			cpg_incr.c
cpg_bp.o:			cpg_bp.c
cpgpseud.o:			cpgpseud.c
cpgpseud_win.o:		cpgpseud_win.c
dhms2soy.o:			dhms2soy.c
detect_chapter.o:	detect_chapter.c	drudge.inc
d_sort.o:			d_sort.c
d_index_sort.o:		d_index_sort.c
doy2mjd.o:			doy2mjd.c
doy2fmjd.o:			doy2fmjd.c
excess_atm.o:		excess_atm.c
fastcos.o:			fastcos.c
fastsin.o:			fastsin.c
f_sort.o:			f_sort.c
f_index_sort.o:		f_index_sort.c
fmjd2doy.o:			fmjd2doy.c
fvanvleck.o:		fvanvleck.c
fort_doy2mjd.o:		fort_doy2mjd.c
fort_doy2fmjd.o:	fort_doy2fmjd.c
fort_mjd2doy.o:		fort_mjd2doy.c
fort_fmjd2doy.o:	fort_fmjd2doy.c
fort_excessatm.o:	fort_excessatm.c
fsleep.o:			fsleep.c
gff.o:				gff.c
gff_mult.o:			gff_mult.c
gff_mult_acc.o:		gff_mult_acc.c
gff_fine.o:			gff_fine.c
gff_acc.o:			gff_acc.c
gst2el.o:			gst2el.c
gst2dsecz.o:		gst2dsecz.c
get_pcalphase.o:	get_pcalphase.c
integ_mult.o:		integ_mult.c
integ_mult_acc.o:	integ_mult_acc.c
integ_mult_com.o:	integ_mult_com.c
integ_amphi.o:		integ_amphi.c
integ_fine.o:		integ_fine.c
integ_bp.o:			integ_bp.c
integ_rawbp.o:		integ_rawbp.c
interp_real.o:		interp_real.c
j2000tob1950.o:		j2000tob1950.c
mjd2doy.o:			mjd2doy.c
mjd2gmst.o:			mjd2gmst.c
memalloc.o:			memalloc.c			obshead.inc
memfree.o:			memfree.c			obshead.inc
objct_id.o:			objct_id.c			obshead.inc
pow2round.o:		pow2round.c
pcal_spline.o:		pcal_spline.c
read_bp.o:			read_bp.c			obshead.inc
read_obshead.o:		read_obshead.c		obshead.inc
read_corhead.o:		read_corhead.c
read_sshead.o:		read_sshead.c
read_delay.o:		read_delay.c
read_gain.o:		read_gain.c
read_pcal.o:		read_pcal.c			obshead.inc
real_spline.o:		real_spline.c
real_delay.o:		real_delay.c
restore_delay.o:	restore_delay.c
restore_gain.o:		restore_gain.c
restore_pcal.o:		restore_pcal.c
save_bp.o:			save_bp.c
soy2dhms.o:			soy2dhms.c
soy2mdhms.o:		soy2mdhms.c
soy2tss.o:			soy2tss.c
scan_source.o:		scan_source.c		drudge.inc
scan_code.o:		scan_code.c			drudge.inc
scan_sked.o:		scan_sked.c			drudge.inc
scan_station.o:		scan_station.c		drudge.inc
sqr_fit.o:			sqr_fit.c
skip_coda.o:		skip_coda.c
skip_flag.o:		skip_flag.c
tss2sec.o:			tss2sec.c
tss2soy.o:			tss2soy.c
time_span_coda.o:	time_span_coda.c
time_block_coda.o:	time_block_coda.c	obshead.inc
utc2tss.o:			utc2tss.c
vanvleck.o:			vanvleck.c
vanvleck2_init.o:	vanvleck2_init.c
xcorr_pair.o:		xcorr_pair.c		obshead.inc
write_sshead.o:		write_sshead.c		obshead.inc

.f.o:
	$(FCOMPL) -c $*.f
vanvcalc.o:			vanvcalc.f
#----------------- End of File --------------------
