struct	delay_data{
	char	site_name_x[8];	/* SITE NAME for X-Station */
	char	site_name_y[8];	/* SITE NAME for Y-Station */
	char	site_code_x[2];	/* SITE CODE for X-Station */
	char	site_code_y[2];	/* SITE CODE for Y-Station */
	char	source_name[16];/* COMMON SOURCE NAME */
	long	rh;				/* RIGHT ASCENTION [HOUR] */
	long	rm;				/* RIGHT ASCENTION [MIN] */
	double	rs;				/* RIGHT ASCENTION [SEC] */
	char	sign[2];		/* SIGN OF DECLINATION [+/-]*/
	long	dd;				/* DECLINATION [deg] */
	long	dm;				/* DECLINATION [min] */
	double	ds;				/* DECLINATION [sec] */
	long	epoch;			/* EPOCH */
	long	year;			/* YEAR */
	long	doy;			/* DAY OF YEAR */
	long	st;				/* START TIME FROM THE DAY */
	double	x_pos_x;		/* X-Position of Station X [m] */
	double	x_pos_y;		/* Y-Position of Station X [m] */
	double	x_pos_z;		/* Z-Position of Station X [m] */
	double	y_pos_x;		/* X-Position of Station Y [m] */
	double	y_pos_y;		/* Y-Position of Station Y [m] */
	double	y_pos_z;		/* Z-Position of Station Y [m] */
	double	fsample;		/* Sampling Frequency [MHz] */
	double	cpp_len;		/* Compressed PP Length [sec] */
	double	rf[16];			/* RF Frequency [MHz] */
	long	start;			/* Start Time [sec] */ 
	long	ncpp;			/* Number of CPP */ 
	double	vis_amp;		/* VISIBILIRY AMPLITUDE */
	double	vis_snr;		/* VISIBILIRY SIGNAL-TO-NOISE RATIO */
	double	input_offset_x;	/* INPUT CLOCK OFFSET for X [mirrosec] */
	double	input_rate_x;	/* INPUT CLOCK RATE for X [picosec/sec] */
	double	input_offset_y;	/* INPUT CLOCK OFFSET for Y [mirrosec] */
	double	input_rate_y;	/* INPUT CLOCK RATE for Y [picosec/sec] */
	double	input_ut1utc;	/* INPUT UT1-UTC [sec] */
	double	clock_epoch;	/* CLOCK EPOCH in [MJD] */
	double	res_delay;		/* RESIDUAL DELAY [mirrosec] */
	double	res_rate;		/* RESIDUAL DELAY RATE [picosec/sec] */
	double	err_delay;		/* DELAY ERROR */
	double	err_rate;		/* DELAY RATE ERROR */
};

struct	delay_file_header{
	long	antnum;			/* Number of Antenna */
	long	blnum;			/* Number of Baseline */
	long	ss_num;			/* Number of Sub-Stream */
	long	var_num;		/* Number of Variable */
	long	start;			/* Start Time [sec] */
	long	integ;			/* Integration Time [sec] */
	long	time_incr;		/* Increment of Time [sec] */
};

struct	ant_delay_data{
	char	site_code[2];	/* SITE CODE */
	double	input_offset;	/* INPUT CLOCK OFFSET [mirrosec] */
	double	input_rate;		/* INPUT CLOCK RATE [picosec/sec] */
	double	clock_epoch;	/* CLOCK EPOCH in [MJD] */
};
