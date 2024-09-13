/*********************************************************
**	SAVE_PCAL.C	: Save P-CAL Data to CODA File System	**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <math.h>
#define	PCDATA	3
#define	MAX_SS	32	

int	save_pcal( obs_name, stn_name, stn_id, obj_name,
			cfs_ssnum, ssnum, cfs_ssid,	rf,
			start_mjd,	integ_time, pcal_ptr, pcerr_ptr )

	char	*obs_name;				/* Observation Code						*/
	char	*stn_name;				/* Station Name							*/
	int		stn_id;					/* Station ID							*/
	char	*obj_name;				/* Object Name							*/
	int		cfs_ssnum;				/* Number of Sub-Stream in CFS			*/
	int		ssnum;					/* Number of Sub-Stream					*/
	int		*cfs_ssid;				/* Sub-Stream Mapping					*/
	double	*rf;					/* RF Frequency							*/
	double	start_mjd;				/* INTEG START TIME [MJD]				*/
	double	integ_time;				/* INTEGRATION TIME [sec]				*/
	double	*pcal_ptr;				/* Pointer of PCAL Phase [rad]			*/
	double	*pcerr_ptr;				/* Pointer of PCAL Phase Error [rad]	*/
{
	int		ss_index;				/* Index of Sub-Stream					*/
	int		*tone_num;				/* Number of Tones in each SS			*/
	int		pcunit;					/* Logical Unit Number for CORR data	*/
	int		recid;					/* Record ID in CFS Data				*/
	int		origin;					/* Origin used in CFS400				*/
	int		nrcd;					/* Record Shift Number in CFS400		*/
	int		ret;					/* Return Code from CFS Library			*/
	int		nival;					/* Number of Integer Data in CFS Lib	*/
	int		nrval;					/* Number of Real Data in CFS Library	*/
	int		ncval;					/* Number of Char Data in CFS Library	*/
	int		double_index;			/* Parameter Index of DOUBLE			*/
	int		ivalue;					/* Int Data in CFS Library				*/
	double	rvalue;					/* Real Data in CFS Library				*/
	char	cvalue[64];				/* Char Data in CFS Library				*/
	int		*int_ptr;				/* Pointer of Integer Data				*/
	double	*double_ptr;			/* Pointer of Double Data				*/
	char	fmode[2];				/* CFS Access Mode						*/
	char	fname[32];				/* CFS File Name						*/
	int		i;

	pcunit	= PCDATA;
	sprintf(fmode, "u");

	/*-------- OPEN PCAL DATA --------*/
	sprintf(fname, "CALIB/PCAL.%d", stn_id ); 
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&pcunit, fname, fmode, &ret, strlen(fname), strlen(fmode));
	cfs_ret( 103, ret );
	cfs401_(&pcunit, &ret);	cfs_ret(401, ret);
/*
--------------------------------------- WRITE HEADER ITEMS
*/
	recid = 0;			/* Record ID for HEADER is 0 */
	nival = 2+MAX_SS;	nrval  = 0;	ncval = 64;

	/*-------- SET CHATACTER DATA --------*/
	memset(cvalue, 0x20, ncval);
	sprintf( &cvalue[0], "%s", obs_name);
	sprintf( &cvalue[32], "%s", stn_name);

	int_ptr = (int *)malloc( nival*sizeof(int) );
	tone_num= (int *)malloc( cfs_ssnum*sizeof(int) );
	memset(tone_num, 0, sizeof(tone_num));
	/*-------- SET INTEGER DATA --------*/
	int_ptr[0]	= stn_id;
	int_ptr[1]	= cfs_ssnum;

	/*-------- NUMBER OF TONES in EACH SS --------*/
	for( ss_index=0; ss_index<ssnum; ss_index++){
		int_ptr[ss_index + 2]	= 1;
		tone_num[ss_index] 	= 1;
	}

	for( ss_index=ssnum; ss_index<MAX_SS; ss_index++){
		int_ptr[ss_index + 2]	= 0;
	}

	/*-------- WRITE TO CFS DATA FILE --------*/
	cfs126_( &pcunit, &recid, &nival, int_ptr, &nrval, &rvalue,
			&ncval, cvalue, &ret, ncval );
	cfs_ret( 126, ret );
	free(int_ptr);

	/*-------- CLOSE PCAL FILE to SAVE --------*/
	cfs104_( &pcunit, &ret );	cfs_ret( 104, ret );
/*
--------------------------------------- OPEN CFS AGAIN
*/
	sprintf(fmode, "w");

	/*-------- OPEN PCAL DATA --------*/
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&pcunit, fname, fmode, &ret, strlen(fname), strlen(fmode));
	cfs_ret( 103, ret );
	cfs402_(&pcunit, &ret);	cfs_ret(402, ret);
/*
--------------------------------------- WRITE DATA ITEMS
*/
	/*-------- SET REAL DATA --------*/
	nival = 0;	ivalue = 0;	ncval = 32;
	memset(cvalue, 0x20, 64);
	sprintf( cvalue, "%s", obj_name );

	for(ss_index=0; ss_index<cfs_ssnum; ss_index++){

		recid = cfs_ssid[ss_index];
		nrval = 2 + 8*tone_num[ss_index];	/* MJD, INCR, + 8 X ntone */

		double_ptr = (double *)malloc(nrval*sizeof(double));
		memset(double_ptr, 0, nrval*sizeof(double) );

		/*-------- SET TIME DATA --------*/
		double_ptr[0]	= start_mjd;
		double_ptr[1]	= integ_time;

		if(tone_num[ss_index] == 1){
			/*-------- SET D-CAL DATA --------*/
			double_ptr[2]	= 0.0;

			/*-------- SET P-CAL FREQ --------*/
			double_ptr[3]	= rf[ss_index];

			/*-------- SET P-CAL REAL --------*/
			double_ptr[4]	= cos( pcal_ptr[ss_index] ); 

			/*-------- SET P-CAL REAL ERR --------*/
			double_ptr[5]	= pcerr_ptr[ss_index]; 

			/*-------- SET P-CAL IMAG --------*/
			double_ptr[6]	= sin( pcal_ptr[ss_index] ); 

			/*-------- SET P-CAL IMAG ERR --------*/
			double_ptr[7]	= pcerr_ptr[ss_index]; 

			/*-------- SET P-CAL RATE --------*/
			double_ptr[8]	= 0.0; 

			/*-------- SET P-CAL RATE ERR --------*/
			double_ptr[9]	= 0.0; 
		}

		/*-------- WRITE TO CFS FILE --------*/
		cfs126_( &pcunit, &recid, &nival, &ivalue, &nrval, double_ptr,
				&ncval, cvalue, &ret, ncval );
		cfs_ret( 126, ret );
		free(double_ptr);
	}

	/*-------- CLOSE PCALL FILE --------*/
	cfs104_( &pcunit, &ret );	cfs_ret( 104, ret );

	return(0);
}
