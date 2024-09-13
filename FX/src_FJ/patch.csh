MAIN__(
	int		argc,			/* Number of Arguments			*/
	char	**argv,			/* Pointer to Arguments			*/
	char	**envp)			/* Pointer to Environments		*/
{

	char		ppblock_fname[128];	/* Block File Name				*/
	FILE		*ppblock_file;		/* Block File					*/
	struct block_info   ppblock[MAX_BLOCK];	/* Block Information	*/
	int			ppblock_num;			/* Total Number of Blocks		*/

	/*-------- BLOCK INFO --------*/
	sprintf(ppblock_fname, BLOCK_FMT, getenv("CFS_DAT_HOME"), argv[1], stn_ptr->acorr_index);
	printf("BLOCK FILE NAME = %s\n", ppblock_fname);
	ppblock_file = fopen(ppblock_fname, "r");
	if( ppblock_file == NULL){
		fclose(ppblock_file);
		printf("  Missing Block Record Information... Please Run codainfo in advance !\n");
		return(0);
	}
	fread(&ppblock_num, sizeof(int), 1, ppblock_file);
	fread(ppblock, ppblock_num* sizeof(struct block_info), 1, ppblock_file );
	fclose(ppblock_file);

	position = block_search(ppblock_num, ppblock, start_mjd);

