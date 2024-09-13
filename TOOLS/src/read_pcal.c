/*********************************************************
**	READ_PCAL.C :	Plot Antenna-Based Delay Data		**
**														**
**	FUNCTON	: Open PCAL-DATA in CFS and Read Delay, 	**
**			Rate, and Acceleration Data. And store them	**
**			to Specified Linked-List.					**
**			READ_DELAY returns 0 when the specified		**
**			station is the reference antenna. In other	**
**			case, it returns the number of data.		**
**	AUTHOR  : KAMENO Seiji								**
**	CREATED : 1996/6/27									**
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cpgplot.h>
#include "obshead.inc"

#define	MAX_SS		32
#define	PCALUNIT	4

int	read_pcal( stn_id,	first_pcal_ptr )

	int		stn_id;							/* Station ID in CFS		*/
	int		*first_pcal_ptr;				/* Pointer of P-CAL data	*/
{
	struct	pcal_data	*pcal_ptr;			/* Pointer of P-CAL data	*/
	struct	pcal_data	*prev_pcal_ptr;		/* Pointer of P-CAL data	*/
	int		eof_flag;			/* 1 -> End of File,  0 -> Under way	*/
	int		ret;				/* Return Code from CFS Library			*/
	int		recid;				/* Record ID in CFS						*/
	char	fname[128];			/* File Name of CFS Files				*/
	char	omode[2];			/* Access Mode of CFS Files				*/
	int		ss_index;			/* Index for Sub-Stream					*/
	int		ssnum;				/* Number of Sub-Stream					*/
	int		data_num;			/* Number of Time 						*/
	int		stn_index;			/* Index for Station					*/
	int		tone_num[MAX_SS];	/* Number of Tones in each SS			*/
	int		pcunit;				/* Logical Unit for Delay File			*/
	int		*int_ptr;			/* Pointer for Integer data in CFS		*/
	double	*double_ptr;		/* Pointer for Double data in CFS		*/
	char	cvalue[64];			/* Character Data in CFS				*/
	int		nival;				/* Number of Integer Data in CFS		*/
	int		nrval;				/* Number of Double Data in CFS			*/
	int		ncval;				/* Byte Number of Character Data in CFS	*/
	int		ivalue;				/* Dummy Variable for Reading CFS		*/
	double	rvalue;				/* Dummy Variable for Reading CFS		*/
	double	phase[32];			/* P-Cal Phase of Each SS				*/
	double	phs_err[32];		/* P-Cal Phase Error of Each SS			*/
	double	start_mjd;			/* Start MJD							*/
	double	time_incr;			/* Time Increment [sec]					*/
	char	obs_name[32];
	char	stn_name[32];

	pcunit	= PCALUNIT;
	sprintf(omode, "r");

	/*-------- OPEN DELAY FILE --------*/
	sprintf(fname, "CALIB/PCAL.%d", stn_id );
	cfs287_(fname, &ret, strlen(fname) );	cfs_ret( 287, ret );
	cfs103_(&pcunit, fname, omode, &ret, strlen(fname), strlen(omode) );
	cfs_ret( 103, ret );

	/*-------- READ HEADER ITEMS --------*/
	recid = 0;	nival = MAX_SS+2;	nrval = 0;	ncval = 64;
	int_ptr	= (int *)malloc( nival*sizeof(int) );

	cfs125_( &pcunit, &recid, &nival, int_ptr, &nrval, &rvalue,
				&ncval, cvalue, &ret, ncval );
	cfs_ret( 125, ret );
	
	/*-------- READ CHARACTER DATA --------*/
	sscanf( &cvalue[0], "%s", obs_name);
	sscanf( &cvalue[32], "%s", stn_name);

	/*-------- READ INTEGER DATA --------*/
	stn_index	= int_ptr[0];
	ssnum		= int_ptr[1];
	for(ss_index=0; ss_index<ssnum; ss_index++){
		tone_num[ss_index] = int_ptr[ss_index + 2];
	}
	free(int_ptr);

	/*-------- READ DELAY DATA --------*/
	eof_flag = -1;
	data_num = 0;
	while(1){
		for(ss_index=0; ss_index<ssnum; ss_index++){

			/*-------- NUBER OF DATA --------*/
			nival	= 0;	nrval	= 2 + 8*tone_num[ss_index];
			ncval	= 32;	recid = ss_index+1;
			double_ptr	= (double *)malloc( nrval*sizeof(double) );

			/*-------- READ FROM CFS --------*/
			cfs125_( &pcunit, &recid, &nival, &ivalue, &nrval, double_ptr,
				&ncval, cvalue, &ret, ncval );

			/*-------- DETECT END OF FILE in CFS P-CAL TABLE --------*/
			if(ret == 55555){
				eof_flag = 1;
				free(double_ptr);
				break;
			}

			/*-------- STORE P-CAL PHASE DATA --------*/
			if( tone_num[ss_index] != 0 ){
				phase[ss_index]		= atan2( double_ptr[6], double_ptr[4] );
				phs_err[ss_index]	= double_ptr[5];
			}
			start_mjd	= double_ptr[0];
			time_incr	= double_ptr[1];

			free(double_ptr);
		}
		if( eof_flag == 1 ){	break;}
		data_num ++;

		pcal_ptr = (struct pcal_data *)malloc( sizeof(struct pcal_data));
		memset( pcal_ptr, 0, sizeof( struct pcal_data ) );

		if( eof_flag == -1){
			*first_pcal_ptr	= (int)pcal_ptr;
			prev_pcal_ptr	= pcal_ptr;
			eof_flag = 0;
		}

		pcal_ptr->mjd		=  start_mjd;
		pcal_ptr->time_incr	=  time_incr;

		strcpy( pcal_ptr->objnam, cvalue );
		for(ss_index=0; ss_index<ssnum; ss_index++){
			if( tone_num[ss_index] != 0 ){
				pcal_ptr->phs[ss_index] = phase[ss_index];
				pcal_ptr->err[ss_index] = phs_err[ss_index];
			}
		}
		prev_pcal_ptr->next_pcal_ptr	= pcal_ptr;
		pcal_ptr->next_pcal_ptr			= NULL;
		prev_pcal_ptr					= pcal_ptr;
	}

	/*-------- CLOSE PCAL FILE --------*/
	cfs104_( &pcunit, &ret );	cfs_ret( 104, ret );

	return(data_num);
}
