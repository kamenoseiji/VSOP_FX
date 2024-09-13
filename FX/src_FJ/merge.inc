struct mrg_header{
	long	format_version;		/* 1 -> Without (u,v), 2 -> Include (u,v) */
	long	history_num;		/* Number of History */
	long	header_num;			/* Number of Header Records */
	long	ant_num;			/* Number of Antenna */
	long	bl_num;				/* Number of Baseline */
	long	cl_num;				/* Number of Closure Phase */
	long	record_len;			/* Length of records in 4-byte words */
	long	cal_flag;			/* 0 -> corr. coeff. 1 -> corr. flux den. */ 
	long	stokes_index;		/* -1=RR,	-2=LL,	-3=RL,	-4=LR	*/ 
								/*  1=I,	 2=Q,	 3=U,	 4=V	*/ 
								/*  0=UNDEFINED	*/ 
	long	dummy_int[11];		/* DUMMY */
};

struct	mrg_source{
	char	name[16];			/* Source Name */
	double	ra_1950, dec_1950;	/* Mean B1950 Source Position [rad] */
	double	ra_app, dec_app;	/* Apparent Source Position [rad] */
	double	total_flux;			/* Total Flux Density [Jy] */
	double	dummy[3];
};

struct	mrg_station1{
	char	name[8];			/* Station Name */
	double	x;					/* X-Coordinate [m] */
	double	y;					/* X-Coordinate [m] */
	double	z;					/* X-Coordinate [m] */
	double	dummy[6];
};

struct	mrg_station2{
	char	name[8];			/* Station Name */
	double	coord[9];			/* STATION POSITION PARAMETER */
	long	type;				/* 1=ground, 2=orbit */
};

struct	mrg_misc{
	long	year;				/* Observation Year */
	long	ut;					/* Universal Time */
	double	gst;				/* Greenwich Apparent Sidereal Time */
	double	rf;					/* Obsergvation Frequency [Hz] */
	double	coh_integ;			/* Coherent Integ Time [sec] */
	double	inc_integ;			/* Incoherent Integ Time [sec] */
	double	bw;					/* Bandwidth [Hz] */
	double	dummy[4];
};

struct mrg_vis1{
	float	amp;				/* Visibility Amplitude */
	float	amp_err;			/* Visibility Amplitude Error */
	float	phs;				/* Visibility Phase [deg] */
	float	phs_err;			/* Visibility Phase Error [deg] */
};

struct mrg_vis2{
	float	amp;				/* Visibility Amplitude */
	float	amp_err;			/* Visibility Amplitude Error */
	float	phs;				/* Visibility Phase [deg] */
	float	phs_err;			/* Visibility Phase Error [deg] */
	float	u, v, w;			/* (u,v,w) in Wavelength */
};

struct mrg_close{
	float	phs;				/* Closure Phase [deg] */
	float	phs_err;			/* Closure Phase Error [deg] */
};
