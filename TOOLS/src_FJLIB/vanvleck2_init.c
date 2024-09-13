/*********************************************************
**	VANVLECK2_INIT:		Calculate Vanvleck Function		**
**						for 2-bit quantized data		**
**	FUNCTION: This function returns spline-factor of 	**
**		Van Vleck relation function for 2-bit quantized	**
**		data. If the file table exists, this function 	**
**		simply read the table and return it. Unless,	**
**		calcurate the function and create table.		**
**														**
**	AUTHOR :	KAMENO Sejii							**
**	CREATED:	1997 7/25								**	
*********************************************************/
#include	<stdio.h>

#define	FNAME	"/sys01/custom/bin/vanvleck.spline"
#define	NODENUM	16

int	vanvleck2_init( node_num_ptr, spline_node_ptr, spline_fact_ptr )
	int		*node_num_ptr;		/* Number of Nodes			*/
	int		*spline_node_ptr;	/* Pointer of Nodes			*/
	int		*spline_fact_ptr;	/* Pointer of Spline Factor	*/
{
	FILE	*table_ptr;			/* File Pointer of Vanvleck Spline Table	*/
	int		node_num;			/* Number of Nodes							*/
	int		fact_num;			/* Number of Spline Factor					*/
	double	*spline_fact;		/* Spline Factor							*/
	double	*spline_node;		/* Spline Nodes								*/
/*
-------------------------------------------------- CREATE SPLINE TABLE
*/
	table_ptr = fopen( FNAME, "r");
	if( table_ptr == NULL ){

		/*-------- CALC VANVLECK and CREATE TABLE --------*/
		printf("Spline Table %s does not found...Create It!\n", FNAME);
		table_ptr = fopen( FNAME, "w");
		if( table_ptr == NULL ){
			printf("Can't Create %s...ABORT!!\n", FNAME);
			return(-1);
		}

		/*-------- ALLOC MEMORY --------*/
		node_num	= 16;
		fact_num	= node_num + 2;
		spline_fact = (double *)malloc( fact_num* sizeof(double) );
		spline_node = (double *)malloc( node_num* sizeof(double) );

		/*-------- CALC SPLINE FACTOR --------*/
		vanvcalc_(&node_num, spline_node, spline_fact);

		/*-------- SAVE INTO FILE --------*/
		fwrite( &node_num, 1, sizeof(int), table_ptr);
		fwrite( spline_node, 1, node_num* sizeof(double), table_ptr);
		fwrite( spline_fact, 1, fact_num* sizeof(double), table_ptr);
		fclose( table_ptr );

		*spline_node_ptr = (int)spline_node;
		*spline_fact_ptr = (int)spline_fact;
		*node_num_ptr	 = node_num;
		return(1);
/*
-------------------------------------------------- READ SPLINE TABLE
*/
	} else {
		/*-------- READ NUMBER of NODES --------*/
		fread( &node_num, 1, sizeof(int), table_ptr);

		/*-------- ALLOC MEMORY --------*/
		fact_num	= node_num + 2;
		spline_fact = (double *)malloc( fact_num* sizeof(double) );
		spline_node = (double *)malloc( node_num* sizeof(double) );

		/*-------- READ SPLINE FACTOR TABLE --------*/
		fread( spline_node, 1, node_num* sizeof(double), table_ptr);
		fread( spline_fact, 1, fact_num* sizeof(double), table_ptr);

		fclose( table_ptr );
		*spline_node_ptr = (int)spline_node;
		*spline_fact_ptr = (int)spline_fact;
		*node_num_ptr	 = node_num;
		return(1);
	}
}
