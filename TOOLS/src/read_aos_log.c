/*********************************************************
**	read_aos_log: Read AOS Log File 					**
**														**
**	FUNCTION: Open Drudge File and Read.				**
**				Data File.								**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include	"aoslog.inc"

int	read_aos_log(
	char	*fname,						/* Log File Name		*/
	int		*scan_addr)
{
	FILE	*file_ptr; 					/* Log File Pointer		*/
	int		scan_index = 0;				/* Index for Scan Record	*/
	int		scan_num;					/* Total Number of Scans	*/
	int		scan_id;					/* Scan Type ID				*/
	double	start_mjd;
	double	stop_mjd;
	struct scan_line	*first_scan;
	struct scan_line	*new_scan;
	struct scan_line	*prev_scan;
/*
------------------------------------------- OPEN LOG FILE
*/
	file_ptr = fopen(fname, "r");
	if(file_ptr == NULL){
		printf("Can't Open Log File [%s]\n", fname);
		return(-1);
	}

	scan_num = 0;
	while(fgets(line_buf, sizeof(line_buf), file_ptr) != 0){
		scan_num ++;

		scan2mjd(line_buf, &start_mjd, &stop_mjd);
		scan_id = scan2type(line_buf);

		new_scan = (struct scan_line *)malloc(sizeof(struct scan_line));
		if( scan_index == 0 ){
			first_scan = new_scan;
			prev_scan  = new_scan;
			printf("%X\n", new_scan);
		}

		new_scan->scan_type = scan_id;
		new_scan->start_mjd = start_mjd;
		new_scan->stop_mjd = stop_mjd;

		new_scan->next_scan_ptr = NULL;
		prev_scan->next_scan_ptr = new_scan;
		prev_scan = new_scan;

		scan_index ++;
	}
	fclose(file_ptr);
	*scan_addr = (int)first_scan;
	return(scan_num);
}
