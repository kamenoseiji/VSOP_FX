/*********************************************************
**	SEARCH_CODA.C: Integrate Bandpass Data for Each		**
**					Antenna and Each Sub-Stream			**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1996/6/27									**
*********************************************************/
#include <stdio.h>
#define	CORRDATA	4
#define	CORRFLAG	5

int	search_coda( corunit, flgunit, cor_id, obj_id, ss_index, position,
			freq_num, start_mjd, time_num_cfs, time_incr,
			work_ptr )

	int		corunit;				/* Logical Unit Number */
	int		flgunit;				/* Logical Unit Number */
	int		cor_id;					/* Baseline Index */
	int		obj_id;					/* Object ID Number */
	int		ss_index;				/* Sub-Stream Index */
	int		*position;				/* Start PP Position	*/
	int		freq_num;				/* Number of Freq. Channels */
	double	start_mjd;				/* INTEG START TIME [MJD] */
	int		time_num_cfs;			/* Number of Time in CFS */
	double	time_incr;				/* INTEG TIME [SEC] */
	float	*work_ptr;				/* Pointer of Visibility Data */
{
	int		ret;					/* Return Code from CFS Library */
	char	omode[2];				/* CFS Access Mode */
	char	fname[32];				/* CFS File Name */

	corunit	= CORRDATA;		flgunit = CORRFLAG;
	sprintf(omode, "r"); 

	/*-------- OPEN CORR DATA --------*/
	sprintf(fname, "CORR.%d/SS.%d/DATA.1\0", cor_id, ss_index ); 
	#ifdef DEBUG
	printf("%s\n", fname ); 
	#endif
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&corunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- OPEN CORR DATA --------*/
	sprintf(fname, "CORR.%d/SS.%d/FLAG.1\0", cor_id, ss_index ); 
	#ifdef DEBUG
	printf("%s\n", fname ); 
	#endif
	cfs287_(fname, &ret, strlen(fname));	cfs_ret( 287, ret );
	cfs103_(&flgunit, fname, omode, &ret, strlen(fname), strlen(omode));
	cfs_ret( 103, ret );

	/*-------- READ VISIBILITY AND FLAG DATA --------*/

	#ifdef DEBUG
	printf("TIME NUM IN CFS IS %d\n", time_num_cfs ); 
	#endif
	skip_coda( corunit, flgunit, position, time_num_cfs, start_mjd,
				freq_num, time_incr );
	#ifdef DEBUG
	printf("FREQ NUM IN CFS IS %d\n", freq_num ); 
	#endif

	return(0);
}
