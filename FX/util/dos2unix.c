/*********************************************************
**	DOS2UNIX.C :	Convert End-of-Line Code on MS-DOS	**
**					To on UNIX.							**
**	FUNCTION	:	Substitute <CR> <LF> Code to <LF>.	**
**					The formar are End-of-Line code on	**
**					MS-DOS and the latter is on UNIX.	**
**	AUTHOR		:	KAMENO Seiji						**
**	CREATED		:	1996/5/14							**
**	USAGE		:	dos2unix [MS-DOS FILE] [OUTPUT FILE]**
*********************************************************/

#include <stdio.h>
main(argc, argv)
	int		argc;		/* Number of Arguments */
	char	**argv;		/* Pointer of Arguments */
{
	char	buf[1];				/* 1-Byte Buffer 				*/
	FILE	*out_file_ptr;		/* File Pointer of Output File 	*/
	FILE	*in_file_ptr;		/* File Pointer of Input File	*/

	/*-------- CHECK FOR COMMAND ARGUMENTS --------*/
	if(argc < 3){
		printf("USAGE : dos2unix [input file] [output file] !!\n");
		exit(0);
	}

	/*-------- OPEN OUTPUT FILE --------*/
	if( (out_file_ptr = fopen( argv[2], "w" )) == 0){
		printf("Can't Open Output File [%s]\n", argv[2]);
		exit(0);
	}

	/*-------- OPEN INPUT FILE --------*/
	if( (in_file_ptr = fopen( argv[1], "r" )) == 0){
			printf("Can't Open INPUT File [%s]\n", argv[1]);
			return(0);
	}

	/*-------- COPY BYTE-BY-BYTE --------*/
	printf("Copying [%s] ...\n", argv[1]);
	while(1){
		if( fread( buf, 1, 1, in_file_ptr ) != 1){
			fclose( in_file_ptr );
			break;
		}

		/*-------- DETECT <CR> CODE --------*/
		if(buf[0] != 0x0d){ 
			fwrite( buf, 1, 1, out_file_ptr );
		}
	}

	fclose( out_file_ptr );
}
