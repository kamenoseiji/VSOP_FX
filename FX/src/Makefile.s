#----------------- LIBLARY ------------------------
PGDIR = /sys01/pds/pgplot_f90
CPGDIR = /sys01/pds/pgplot
PGPLOT= $(PGDIR)/libpgplot.a
CPGPLOT= $(CPGDIR)/libcpgplot.a
MATH=/usr/lib/libm.a
INCLUDE= /sys01/pds/pgplot
#CFSLIB=	/sys01/custom/codatest/CODA/FS/LIB/BIN/libcfs.a /sys01/custom/codatest/CODA/FS/LIB/DSMD/d_link.a
CFSLIB=	/sys01/custom/coda/CODA/FS/LIB/BIN/libcfs.a /sys01/custom/coda/CODA/FS/LIB/DSMD/d_link.a
GRLIBS= -L/sys01/custom/bin -lFXCORR -lfssl2 -L/usr/openwin/lib -lX11 
#----------------- COMPILE OPTIONS ----------------
CFLAGS = -I$(INCLUDE) -O3
FFLAGS = -I$(INCLUDE) -O3
BINDIR = /sys01/custom/bin
SYSTEM = sol2
CCOMPL = fcc $(CFLAGS) $(CPPFLAGS)
FCOMPL = frt $(FFLAGS)
AR = ar ru
PROGS  = coda_bp \
station_bp			\
replay_bp			\
read_station_bp		\
coda_gfs			\
coda_gfs_bird		\
coda_gfs_delayslice	\
coda_gfs_rateslice	\
coda_acscl			\
refscl				\
gcal			\
plot_delay 			\
plot_gain 			\
plot_pcal 			\
coda_spec 			\
make_pcal			\
coda_fine 			\
coda_info 			\
fake_bp 			\
coda_mrg 			\
coda_visp 			\
coda_anim 			\
coda_copy 			\
sort_delay 			\
sort_obslog 		\
coda_gfs_acc 		\
coda_uv 			\
coda_gfs_spec		\
spline_delay 		\
coda_avail 			\
coda_amphi 			\
coda_tsys 			\
merge_mrg 			\
vis_comp 			\
coda_track
#----------------- OBJECTS ------------------------
OBJ_avail	=	coda_avail.o average_vis.o ave_bandpass.o \
				cpg_amphi.o cpg_clphs.o
OBJ_bp	=	coda_bp.o read_flag.o
OBJ_fakebp	=	fake_bp.o 
OBJ_rpbp=	replay_bp.o 
OBJ_rpbp=	replay_bp.o 
OBJ_stbp=	station_bp.o
OBJ_rdstbp=	read_station_bp.o

OBJ_gfs =	coda_gfs.o load_vis.o crs_search.o \
			save_gff.o  max_search.o

OBJ_gfs_bird =	coda_gfs_bird.o load_vis.o crs_search.o \
			save_gff.o max_search.o

OBJ_gfs_delayslice =	coda_gfs_delayslice.o load_vis.o crs_search.o \
			save_gff.o cpgdelayslice.o max_search.o

OBJ_gfs_rateslice =	coda_gfs_rateslice.o load_vis.o crs_search.o \
			save_gff.o cpgrateslice.o max_search.o

OBJ_gfs_acc =	coda_gfs_acc.o load_vis.o acc_search.o \
			save_gff.o bl_search.o plot_blsearch.o crs_search.o

OBJ_gfs_spec =	coda_gfs_spec.o load_vis.o acc_spec_search.o \
			save_gff.o

OBJ_acscl=	coda_acscl.o read_flag.o

OBJ_refscl=	refscl.o cpg_acorr.o \
			cal_offline.o cal_offset.o

OBJ_gcal=	gcal.o cpg_acorr.o search_coda.o \
			read_acorr.o cal_offset.o cal_exec.o cal_offline.o

OBJ_plotdelay	= plot_delay.o cpg_delay.o

OBJ_splinedelay	= spline_delay.o cpg_delay.o  cpg_spline_delay.o

OBJ_sortdelay	= sort_delay.o save_delay_sort.o

OBJ_sortobslog	= sort_obslog.o read_vb.o save_vb_index.o

OBJ_plotgain	= plot_gain.o cpg_gain.o

OBJ_plotpcal	= plot_pcal.o 

OBJ_coda_spec	= coda_spec.o read_fcal.o integ_vis.o cpg_vis.o ave_bandpass.o

OBJ_coda_anim	= coda_anim.o read_fcal.o  \
			integ_vis.o  cpg_vis_anim.o cpg_spec.o cp_vis.o \
			load_vis_anim.o phase_track.o

OBJ_coda_track	= coda_track.o read_fcal.o  \
			integ_vis.o  \
			load_vis_anim.o phase_track.o

OBJ_make_pcal	= make_pcal.o load_vis.o save_pcal.o

OBJ_amphi	=	coda_amphi.o average_vis.o ave_bandpass.o \
				cpg_amphi.o cpg_clphs.o

OBJ_fine	=	coda_fine.o load_visp.o \
				station_info.o atmgain_solve.o

OBJ_mrg		=	coda_mrg.o average_vis.o ave_bandpass.o \
				cpg_amphi.o cpg_clphs.o

OBJ_merge	=	merge_mrg.o read_merge.o count_merge.o

OBJ_visp	=	coda_visp.o load_visp.o \
				station_scan.o atmgain_solve.o

OBJ_info	=	coda_info.o 

OBJ_uv		=	coda_uv.o 

OBJ_copy	=	coda_copy.o 
OBJ_tsys	=	coda_tsys.o  read_tsys.o
OBJ_viscomp	=	vis_comp.o

#----------------- LINK ------------------------
coda_avail: $(OBJ_avail)
	$(FCOMPL) -o $@ $(OBJ_avail) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_bp: $(OBJ_bp)
	$(FCOMPL) -o $@ $(OBJ_bp) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

fake_bp: $(OBJ_fakebp)
	$(FCOMPL) -o $@ $(OBJ_fakebp) $(GRLIBS) $(CFSLIB) $(MATH)

replay_bp: $(OBJ_rpbp)
	$(FCOMPL) -o $@ $(OBJ_rpbp) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

station_bp: $(OBJ_stbp)
	$(FCOMPL) -o $@ $(OBJ_stbp) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

read_station_bp: $(OBJ_rdstbp)
	$(FCOMPL) -o $@ $(OBJ_rdstbp) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_gfs: $(OBJ_gfs)
	$(FCOMPL) -o $@ $(OBJ_gfs) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_gfs_bird: $(OBJ_gfs_bird)
	$(FCOMPL) -o $@ $(OBJ_gfs_bird) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_gfs_delayslice: $(OBJ_gfs_delayslice)
	$(FCOMPL) -o $@ $(OBJ_gfs_delayslice) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_gfs_rateslice: $(OBJ_gfs_rateslice)
	$(FCOMPL) -o $@ $(OBJ_gfs_rateslice) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_gfs_acc: $(OBJ_gfs_acc)
	$(FCOMPL) -o $@ $(OBJ_gfs_acc) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_gfs_spec: $(OBJ_gfs_spec)
	$(FCOMPL) -o $@ $(OBJ_gfs_spec) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_acscl: $(OBJ_acscl)
	$(FCOMPL) -o $@ $(OBJ_acscl) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)
refscl: $(OBJ_refscl)
	$(FCOMPL) -o $@ $(OBJ_refscl) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

gcal: $(OBJ_gcal)
	$(FCOMPL) -o $@ $(OBJ_gcal) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

plot_delay: $(OBJ_plotdelay)
	$(FCOMPL) -o $@ $(OBJ_plotdelay) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

spline_delay: $(OBJ_splinedelay)
	$(FCOMPL) -o $@ $(OBJ_splinedelay) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

sort_delay: $(OBJ_sortdelay)
	$(FCOMPL) -o $@ $(OBJ_sortdelay) $(GRLIBS) $(CFSLIB) $(MATH)

sort_obslog: $(OBJ_sortobslog)
	$(FCOMPL) -o $@ $(OBJ_sortobslog) $(GRLIBS) $(CFSLIB) $(MATH)

plot_gain: $(OBJ_plotgain)
	$(FCOMPL) -o $@ $(OBJ_plotgain) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

plot_pcal: $(OBJ_plotpcal)
	$(FCOMPL) -o $@ $(OBJ_plotpcal) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_spec: $(OBJ_coda_spec)
	$(FCOMPL) -o $@ $(OBJ_coda_spec) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_anim: $(OBJ_coda_anim)
	$(FCOMPL) -o $@ $(OBJ_coda_anim) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_track: $(OBJ_coda_track)
	$(FCOMPL) -o $@ $(OBJ_coda_track) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

make_pcal: $(OBJ_make_pcal)
	$(FCOMPL) -o $@ $(OBJ_make_pcal) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_amphi: $(OBJ_amphi)
	$(FCOMPL) -o $@ $(OBJ_amphi) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_fine: $(OBJ_fine)
	$(FCOMPL) -o $@ $(OBJ_fine) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_mrg: $(OBJ_mrg)
	$(FCOMPL) -o $@ $(OBJ_mrg) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

merge_mrg: $(OBJ_merge)
	$(CCOMPL) -o $@ $(OBJ_merge) $(GRLIBS) $(MATH)

coda_visp: $(OBJ_visp)
	$(FCOMPL) -o $@ $(OBJ_visp) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_info: $(OBJ_info)
	$(FCOMPL) -o $@ $(OBJ_info) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_uv: $(OBJ_uv)
	$(FCOMPL) -o $@ $(OBJ_uv) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

coda_copy: $(OBJ_copy)
	$(FCOMPL) -o $@ $(OBJ_copy) $(GRLIBS) $(CFSLIB) $(MATH)

coda_tsys: $(OBJ_tsys)
	$(FCOMPL) -o $@ $(OBJ_tsys) $(GRLIBS) $(CFSLIB) $(MATH)

vis_comp: $(OBJ_viscomp)
	$(FCOMPL) -o $@ $(OBJ_viscomp) $(GRLIBS) $(CFSLIB) $(CPGPLOT) $(PGPLOT) $(MATH)

all: $(PROGS)

install:
	@mv $(PROGS) $(BINDIR)

clean:
	$(RM) $(PROGS) *.o a.out core *.trace
#----------------- COMPILE ------------------------
.c.o:
	$(CCOMPL) -c $*.c

atmgain_solve.o:	atmgain_solve.c		obshead.inc
ave_bandpass.o:		ave_bandpass.c		obshead.inc
average_vis.o:		average_vis.c
bl_search.o:		bl_search.c			obshead.inc
cal_exec.o:			cal_exec.c
cal_offset.o:		cal_offset.c
cal_offline.o:		cal_offline.c
coda_acscl.o:		coda_acscl.c		obshead.inc
coda_avail.o:		coda_avail.c		obshead.inc
coda_bp.o:			coda_bp.c			obshead.inc
coda_gfs.o:			coda_gfs.c			obshead.inc
coda_gfs_bird.o:	coda_gfs_bird.c			obshead.inc
coda_gfs_acc.o:		coda_gfs_acc.c		obshead.inc
coda_gfs_spec.o:	coda_gfs_spec.c		obshead.inc
coda_spec.o:		coda_spec.c			obshead.inc
coda_anim.o:		coda_anim.c			obshead.inc
coda_amphi.o:		coda_amphi.c		obshead.inc
coda_fine.o:		coda_fine.c			obshead.inc
coda_mrg.o:			coda_mrg.c			obshead.inc merge.inc
coda_visp.o:		coda_visp.c			obshead.inc
coda_info.o:		coda_info.c			obshead.inc
coda_track.o:		coda_track.c		obshead.inc
coda_tsys.o:		coda_tsys.c			obshead.inc
coda_uv.o:			coda_uv.c			obshead.inc
coda_copy.o:		coda_copy.c			obshead.inc
count_merge.o:		count_merge.c		merge.inc
cp_vis.o:			cp_vis.c
cpg_acorr.o:		cpg_acorr.c
cpg_amphi.o:		cpg_amphi.c
cpg_clphs.o:		cpg_clphs.c
cpg_delay.o:		cpg_delay.c			obshead.inc
cpg_spline_delay.o:	cpg_spline_delay.c	obshead.inc
cpg_gain.o:			cpg_gain.c
cpgdelayslice.o:	cpgdelayslice.c
cpgrateslice.o:		cpgrateslice.c
cpg_vis.o:			cpg_vis.c
cpg_vis_anim.o:		cpg_vis_anim.c
cpg_spec.o:			cpg_spec.c
crs_search.o:		crs_search.c
acc_search.o:		acc_search.c
acc_spec_search.o:	acc_spec_search.c
fake_bp.o:			fake_bp.c			obshead.inc
gcal.o:				gcal.c				obshead.inc
integ_vis.o:		integ_vis.c
interp_real.o:		interp_real.c
load_vis.o:			load_vis.c			obshead.inc
load_visp.o:		load_visp.c			obshead.inc
make_pcal.o:		make_pcal.c			obshead.inc
max_search.o:		max_search.c
merge_mrg.o:		merge_mrg.c			merge.inc
plot_gain.o:		plot_gain.c			obshead.inc
plot_pcal.o:		plot_pcal.c			obshead.inc
plot_blsearch.o:	plot_blsearch.c		obshead.inc
phase_track.o:		phase_track.c		obshead.inc
read_acorr.o:		read_acorr.c
read_flag.o:		read_flag.c
read_fcal.o:		read_fcal.c
read_merge.o:		read_merge.c		merge.inc
read_station_bp.o:	read_station_bp.c	obshead.inc
read_vb.o:			read_vb.c			obshead.inc
read_vis.o:			read_vis.c
read_tsys.o:		read_tsys.c
refscl.o:			refscl.c			obshead.inc
replay_bp.o:		replay_bp.c			obshead.inc
save_bp.o:			save_bp.c
save_delay_sort.o:	save_delay_sort.c
save_gff.o:			save_gff.c			obshead.inc
search_coda.o:		search_coda.c
sort_delay.o:		sort_delay.c		obshead.inc
sort_obslog.o:		sort_obslog.c		obshead.inc
spline_delay.o:		spline_delay.c		obshead.inc
station_bp.o:		station_bp.c		obshead.inc
save_pcal.o:		save_pcal.c			obshead.inc
save_vb_index.o:	save_vb_index.c		obshead.inc
station_info.o:		station_info.c		obshead.inc
station_scan.o:		station_scan.c		obshead.inc
load_vis_anim.o: 	load_vis_anim.c		obshead.inc
vis_comp.o: 		vis_comp.c

#----------------- End of File --------------------
