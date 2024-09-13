#include <stdio.h>
main(argc, argv)
	int		argc;		/* Number of Arguments */
	char	**argv;		/* Pointer of Arguments */
{
	char	buf[1];
	int		i;
	FILE	*out_file_ptr;
	FILE	*in_file_ptr;

	if(argc < 3){
		printf("USAGE : bincat [file1] [file2] .... [output file] !!\n");
		exit(0);
	}

	/*-------- OPEN OUTPUT FILE --------*/
	if( (out_file_ptr = fopen( argv[argc-1], "w" )) == 0){
		printf("Can't Open Output File [%s]\n", argv[argc-1]);
		exit(0);
	}

	/*-------- OPEN INPUT FILE --------*/
	for(i=1; i<argc-1; i++){
		printf("Copying [%s] ...\n", argv[i]);
		if( (in_file_ptr = fopen( argv[i], "r" )) == 0){
			printf("Can't Open INPUT File [%s]\n", argv[i]);
			break;
		}
		while(1){
			if( fread( buf, 1, 1, in_file_ptr ) != 1){
				fclose( in_file_ptr );
				break;
			}
			fwrite( buf, 1, 1, out_file_ptr );
		}
	}
	fclose( out_file_ptr );
}
