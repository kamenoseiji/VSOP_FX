/*****************************************************
**	TSS2IDR: TimeLine Run				 	**
**	AUTHOUR	: KAMENO Seiji							**
**	CREATED	: 1994/11/1								**
******************************************************/

long tss2idr( tss, idr_ptr ) 
	long	tss;			/* TSS ID to Play TLrun Counter */
	char	*idr_ptr;
{
/*
--------------------------------------- TSSSET for Both TCU
*/
	sprintf( idr_ptr, "%02X%02X%02X\0",
		 (tss & 0x000000ff),
		((tss & 0x0000ff00) >> 8),
		((tss & 0x00ff0000) >> 16));
}

long idr2tss( idr_ptr ) 
	char	*idr_ptr;
{
	long	h2, h1, h0;
	sscanf(idr_ptr, "%02X%02X%02X", &h2, &h1, &h0);
	return( h0<<16 | h1<<8 | h2 ); 
}
