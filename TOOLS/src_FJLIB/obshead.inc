#define	KEY_DIR	"/"
#define	OBJ_KEY 1234	
#define	STN_KEY 5678	
#define	SPC_KEY 9012	
#define	NORMAL	1	
#define	INVERT	-1	

struct	header{
	char	obscode[32];		/* Observation Code */
	int		cor_num;			/* Total Number of Correlation Pair */
	int		obj_num;			/* Total Number of Objects */
	int		stn_num;			/* Total Number of Stations */
	int		real_stn_num;		/* Number of Real Stations (exclude Dummy)*/
	int		sat_num;			/* Number of Sattelite */
	double	start_mjd;			/* Start Time [MJD] */
	double	stop_mjd;			/* Stop Time [MJD] */
	double	epoch;				/* Epoch of the Clock Params [MJD] */
	double	gstia0;				/* GST at the Epoch [rad] */
	double	degpdy;				/* Earth Rotaton Speed [Degree per Day] */
	double	polarx;				/* Polar X [rad] */
	double	polary;				/* Polar Y [rad] */
	double	ut1utc;				/* UT1 - UTC [sec] */
	double	iatutc;				/* TAI - UTC [sec] */
};

struct	head_obj{
	struct	head_obj	*next_obj_ptr;
	int		obj_index;			/* Index of Sources */
	char	obj_name[16];		/* Source Name */
	double	obj_pos[3];			/* RA [rad], DEC [rad], EPOCH */
};

struct	head_stn{
	struct	head_stn	*next_stn_ptr;
	int		stn_index;			/* Index of Station */
	int		acorr_index;		/* Correlation Index for Autocorr */
	char	stn_name[16];		/* Station Name */
	double	stn_pos[3];			/* Station Position (x, y, z) [m] */
};

struct	head_cor{
	struct	head_cor	*next_cor_ptr;
	int		cor_id;				/* CORR Pair ID */
	int		ant1;				/* Antenna ID 1 */
	int		ant2;				/* Antenna ID 2 */
	int		nss;				/* Number of Sub-Stream */
	double	loss;				/* Quantize  Efficiency */
};

struct	fcal_data{
	struct	fcal_data	*next_fcal_ptr;
	double	mjd;				/* Epoch of the Clock Data [MJD] */
	double	time_incr;			/* Integration Time [sec] */
	char	objnam[32];			/* Object Name */
	int		refant;				/* Reference Ant */
	double	rate;				/* Clock Rate [sec/sec] */
	double	rate_err;			/* Clock Rate Error [sec/sec] */
	double	delay;				/* Clock Offset [sec] */
	double	delay_err;			/* Clock Offset Error [sec] */
	double	acc;				/* Clock Acceleration Rate [sec/sec] */
	double	acc_err;			/* Clock Acceleration Rate Error [sec/sec] */
};

struct	gcal_data{
	struct	gcal_data	*next_gcal_ptr;
	double	mjd;				/* Epoch of the Clock Data [MJD] */
	double	time_incr;			/* Integration Time [sec] */
	char	objnam[32];			/* Object Name */
	double	phase;				/* Phase [rad] */
	double	real;				/* Real Part of SEFD [Jy] */
	double	imag;				/* Imag Part of SEFD [Jy] */
	double	weight;				/* Weight */
};

struct	pcal_data{
	struct	pcal_data	*next_pcal_ptr;
	double	mjd;				/* Epoch of the Clock Data [MJD]	*/
	double	time_incr;			/* Integration Time [sec]			*/
	char	objnam[32];			/* Object Name						*/
	double	dcal[32];			/* Pointer of Cable Delay [sec]		*/
	double	freq[32];			/* Pointer of Frequency [MHz]		*/
	double	phs[32];			/* Pointer of Real Part 			*/
	double	err[32];			/* Pointer of Real Part Error		*/
};

struct	vb_list{
	struct	vb_list	*next_vb_ptr;
	int		data_index;			/* Data Index						*/
	double	mjd;				/* Epoch of the Clock Data [MJD]	*/
	int		data_size;			/* Data Size [Byte]					*/
	char	*data_ptr;			/* Pointer of the Data				*/
};

struct	bl_list{
	int		cor_id;				/* Corr. Pair ID in CFS				*/
	int		stn_id1;			/* Station (1) ID in CFS			*/
	int		stn_id2;			/* Station (1) ID in CFS			*/
	int		bl_dir;				/* 1 -> Normal, -1 -> Invert		*/
	char	stn_name1[16];		/* Station (1) Name in CFS			*/
	char	stn_name2[16];		/* Station (1) Name in CFS			*/
};

struct	obj_list{
	int		obj_id;				/* Source ID in CFS					*/
	char	obj_name[16];		/* Source Name						*/
	double	obj_pos[3];			/* RA [rad], DEC [rad], EPOCH		*/
};

struct	block_info{
	double	st;					/* MJD at Start of the Block		*/
	double	et;					/* MJD at End of the Block			*/
	int		st_pp;				/* PP Number at Start of the Block	*/
	int		et_pp;				/* PP Number at End of the Block	*/
	int		obj;				/* Object Number of the Block		*/
};
