/*********************************************************
**	read_aos_log: Read AOS Log File 					**
**														**
**	FUNCTION: Open Drudge File and Read.				**
**				Data File.								**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include	"aoslog.inc"

int	detect_scan(
	int		scan_addr,		/* Pointer to the Scan Records		*/
	double	current_mjd)	/* MJD of the FX Data				*/
{
	struct scan_line	*this_scan;
	struct scan_line	*prev_scan;
/*
------------------------------------------- OPEN LOG FILE
*/
	this_scan = (struct scan_line *)scan_addr;
	prev_scan = this_scan;

	while(this_scan != NULL){
		if( this_scan->start_mjd > current_mjd){
#ifdef DEBUG
			printf("No Available Scan Found.\n");
			printf("Start MJD = %lf,  Last Scan End = %lf, Current MJD = %lf\n", 
				this_scan->start_mjd, prev_scan->stop_mjd, current_mjd);
#endif
			return(-1);
		}

		if( this_scan->stop_mjd > current_mjd){

#ifdef DEBUG
			printf("SCAN DETECTED!!  TYPE = %d\n", this_scan->scan_type);
			printf("Start MJD = %lf,  Current MJD = %lf\n", 
				this_scan->start_mjd, current_mjd);
#endif
			return(this_scan->scan_type);
		}

		this_scan = this_scan->next_scan_ptr;
		prev_scan = this_scan;
	}
#ifdef DEBUG
	printf("No Available Scan Found.\n");
#endif
	return(-1);
}
