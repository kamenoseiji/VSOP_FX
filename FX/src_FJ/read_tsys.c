/*****************************************************************
**	READ_TSYS.C	: Read Satellite Position and Velocity Data	**
**																**
**	AUTHOR	: Kameno Seiji										**
**	CREATED	: 1997/3/26											**
*****************************************************************/

#include	<stdio.h>
#include	<math.h>
#define	TSYS_FMT	"%03d%02d%02d%02d%*s %lf %lf"
#define	KFACT	2761.324		/* 2k in Jy Unit						*/

int	read_tsys( year,  fname, init_time_ptr, init_sefd_ptr )
	int		year;				/* Observation Year						*/
	char	*fname;				/* File Name 							*/
	int		*init_time_ptr;		/* Pointer of Time [MJD]				*/
	int		*init_sefd_ptr;		/* Pointer of SEFD 						*/
{
	FILE	*file_ptr;			/* Pointer of Satellite Position File	*/
	char	line_buf[256];		/* 1-Line Buffer						*/

	/*-------- DATA POINTER --------*/
	double	*time_ptr;			/* Pointer of Time [MJD]				*/
	double	*sefd_ptr;			/* Pointer of Amplitude 				*/

	/*-------- INDEX ---------*/
	int		data_index;			/* Index for Data Records				*/
	int		data_num;			/* Total Number of Data Records			*/

	/*-------- TIME DATA ---------*/
	int		doy;				/* Day of Year							*/
	int		hour;				/* Hour (UT)							*/
	int		min;				/* Minute								*/
	int		sec;				/* Second								*/
	double	curr_mjd;			/* MJD									*/

	/*-------- TSYS DATA ---------*/
	double	tsys;				/* Amplitude							*/
	double	ae;					/* Amplitude							*/
	double	sefd;				/* Amplitude							*/
/*
------------------------------------------- OPEN DATA FILE
*/
	file_ptr	= fopen( fname, "r" );
	if( file_ptr == NULL ){
		printf(" Can't Open Data File [%s] !!\n", fname);
		fclose(file_ptr);
		return(-1);
	}
/*
------------------------------------------- SCAN DATA FILE 
*/
	/*-------- SCAN DATA FILE AND GET NUMBER OF DATA --------*/
	data_index = 0;
	while( fgets(line_buf, sizeof(line_buf), file_ptr ) != 0){
		if( strstr(line_buf, "/TSYS/") != NULL){
			data_index ++;
		}
	}
	fclose( file_ptr );
	data_num = data_index;

	/*-------- ALLOCATE MEMORY --------*/
	time_ptr= (double *)malloc( data_num* sizeof(double) );
	sefd_ptr= (double *)malloc( data_num* sizeof(double) );
/*
------------------------------------------- SCAN DATA FILE 
*/
	file_ptr	= fopen( fname, "r" );
	data_index = 0;

	while(data_index<data_num){
		if(fgets(line_buf, sizeof(line_buf), file_ptr ) == NULL){	break;}
		if( strstr(line_buf, "/TSYS/") != NULL ){

			/*-------- READ START TIME --------*/
			sscanf( line_buf, TSYS_FMT, &doy, &hour, &min, &sec, &ae, &tsys); 
			doy2fmjd( year, doy, hour, min, (double)sec, &curr_mjd);
			time_ptr[data_index] = curr_mjd;
			sefd_ptr[data_index] = KFACT* tsys / ae;

			data_index ++;
		}
	}
	fclose(file_ptr);
	printf("READ TOTAL %d RECORD DATA...\n", data_num);
/*
------------------------------------------- STORE INTO ARRAY 
*/
	*init_time_ptr	= (int)time_ptr;
	*init_sefd_ptr	= (int)sefd_ptr;

	return(data_num);
}
