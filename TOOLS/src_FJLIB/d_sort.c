/*********************************************************
**	D_SORT.C : Sort DOUBLE data by increasing order		**
**														**
**	AUTHOR :	KAMENO Seiji							**
**	CREATED:	1997/2/26								**
**														**
*********************************************************/

int	d_sort( data_num, data_ptr )
	int		data_num;				/* Total Number of Data		*/
	double	*data_ptr;				/* Total Number of Data		*/
{
	int		seq_index;				/* Number of Data in Branch	*/
	int		level_index;
	int		i, j;
	double	curr_top_value;			/* Current Top (smallest)	*/


	seq_index	= (data_num + 1) / 2; 
	level_index	= data_num - 1; 

	for(;;){
		if( seq_index > 0 ){		/* Not the top				*/ 
			curr_top_value = data_ptr[--seq_index];

		} else {					/* In the Top				*/
			curr_top_value	= data_ptr[level_index];
			data_ptr[level_index] = data_ptr[0];

			if( --level_index == 0 ){
				data_ptr[0] = curr_top_value;
				return(0);
			}
		}


		i=seq_index;	j=seq_index << 1;
		while( j <= level_index){
			if( (j < level_index) && (data_ptr[j] < data_ptr[j+1])){	++j;}
			if( curr_top_value < data_ptr[j] ){
				data_ptr[i] = data_ptr[j];
				j += (i=j);
			} else {
				j = level_index + 1;
			}
			data_ptr[i] = curr_top_value;
		}
	}
}
