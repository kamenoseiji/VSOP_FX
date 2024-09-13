/*************************************************************
**	D_INDEX_SORT.C : Make Index Table of DOUBLE Variables	**
**					in Increasing Order.					**
**															**
**	AUTHOR : Kameno S.										**
**	CREATED: 1997/2/27										**
*************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

int	d_index_sort( data_num, data_ptr, index_ptr )
	int		data_num;				/* Total Number of Data		*/
	double	*data_ptr;				/* Total Number of Data		*/
	int		*index_ptr;				/* Pointer of Index			*/
{
	int		seq_index;				/* Number of Data in Branch	*/
	int		level_index;
	int		i, j;
	double	*temp_ptr;				/* Temporary Data Pointer	*/
	double	curr_top_value;			/* Current Top (smallest)	*/
	int		curr_index;				/* Current Index Value		*/

	/*-------- Copy Data into Temporary Buffer --------*/
	temp_ptr = (double *)malloc( data_num * sizeof(double));
	memcpy( temp_ptr, data_ptr, data_num * sizeof(double) );

	seq_index	= (data_num + 1) / 2; 
	level_index	= data_num - 1; 

	while(1){
		if( seq_index > 0 ){		/* Not the top				*/ 
			seq_index --;
			curr_top_value = temp_ptr[seq_index];
			curr_index		= index_ptr[seq_index];

		} else {					/* In the Top				*/
			curr_top_value			= temp_ptr[level_index];
			temp_ptr[level_index]	= temp_ptr[0];

			curr_index				= index_ptr[level_index];
			index_ptr[level_index]	= index_ptr[0];

			level_index --;
			if( level_index == 0 ){
				temp_ptr[0]		= curr_top_value;
				index_ptr[0]	= curr_index;
				free(temp_ptr);
				return(0);
			}
		}

		i=seq_index;	j=seq_index << 1;
		while( j <= level_index){
			if( (j < level_index) && (temp_ptr[j] < temp_ptr[j+1])){	++j;}
			if( curr_top_value < temp_ptr[j] ){
				temp_ptr[i]		= temp_ptr[j];
				index_ptr[i]	= index_ptr[j];
				j += (i=j);
			} else {
				j = level_index + 1;
			}
			temp_ptr[i]		= curr_top_value;
			index_ptr[i]	= curr_index;
		}
	}
}
