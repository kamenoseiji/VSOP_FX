#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cpgplot.h"
#define PI 		3.1415926535897932384626433832795
#define PI2 	6.2831853071795864769252867665590
#define MAS2RAD 4.84813681109535993589e-9
#define DEG2RAD 1.74532925199432957692e-2
#define FWHM2SIGMA 1.177410022515474691
#define	NGRID	64
#define	NCONT	12
#define	MAX_COMP	40	
#define	WINDOW_SIZE	10
#define MSG_SIZ 64

struct msg_buf{
	long	msg_type;
	char	msg_text[MSG_SIZ];
};

MAIN__(argc, argv)
	long	argc;		/* Number of Arguments */
	char	**argv;		/* Pointer of Arguments */
{
	double	gaus_comp[MAX_COMP][6];	/* Gaussian Component Parameter */
	long	ncomp;					/* Number of Gaussian Components */
	FILE	*mod_file_ptr;			/* File Pointer of the Model File */
	FILE	*out_file_ptr;			/* File Pointer of New Model File */
	char	line_buf[256];			/* Line Buffer for Model File */
	float	imag[NGRID][NGRID];		/* Image Memory */
	long	comp_index;				/* Index for Gaussian Components */
	long	x_index;				/* Index for X-Axis */
	long	y_index;				/* Index for Y-Axis */
	long	cont_index;				/* Index for Contour */
	float	xmin, xmax;				/* Max and Min for X-Axis */
	float	ymin, ymax;				/* Max and Min for Y-Axis */
	float	levs[NCONT];			/* Contour Levels */
	float	tr[6];					/* Matrix for Contour */
	float	max_imag;				/* Maximum Brightness */
	float	x_corner[4];			/* X-Coordinate of FWHM Corner */
	float	y_corner[4];			/* Y-Coordinate of FWHM Corner */
	float	x_curs, y_curs;			/* Cursor Position */
	float	x_position, y_position;	/* Position of Selected Component */
	float	x_size, y_size;			/* Window Size */
	float	x_incr, y_incr;			/* Grid Cellsize */
	char	curs_key[4];			/* Cursor Key */
	long	edit_status;
	long	selected_comp;
	float	beam_size;				/* BEAM SIZE */

	key_t	keyword;		/* KeyWord for Shared Memory */
	long	shrd_mem_id;	/* Shared Memory ID Number */
	long	shrd_mem_size;	/* Shared Memory Size */
	char	*shrd_mem_addr;

	long	msg_id;
	key_t	msg_key;
	struct	msg_buf	send_buf;

	msg_key	= 5678;
	msg_id = msgget(msg_key, 0644);

	/*-------- CHECK FOR INPUT PARAMETER --------*/
	if(argc < 5){
		printf("USAGE : modedit [INPUT FILE] [OUTPUT FILE] [WINDOW SIZE] [BEAM SIZE]!!\n");
		exit(0);
	}

	x_size	= atof(argv[3]);
	y_size	= x_size;
	beam_size = atof(argv[4]);
/*
---------------------------------------- INIT IMAP
*/
	if(msg_id > 0){
		keyword = 1234;
		if((shrd_mem_id=shmget(keyword,sizeof(gaus_comp),IPC_CREAT|0644)) < 0){
			printf("Error in [shmget] !!");
			exit(1);
		}
		shrd_mem_addr = shmat(shrd_mem_id, NULL, 0);
	}
	memset(imag, 0, sizeof(imag));
/*
---------------------------------------- INIT IMAP
*/
	x_incr	= x_size/NGRID;
	y_incr	= y_size/NGRID;
/*
---------------------------------------- OPEN MODEL FILE
*/
	if( (mod_file_ptr	= fopen( argv[1], "r" )) == NULL){
		printf("Can't Open Model File [%s] !!\n", argv[1]);
		exit(0);
	}
	if( (out_file_ptr	= fopen( argv[2], "w" )) == NULL){
		printf("Can't Open OutPut Model File [%s] !!\n", argv[2]);
		exit(0);
	}
/*
---------------------------------------- READ MODEL FILE
*/
	comp_index	= 0;
	while(1){
		if( fgets( line_buf, sizeof(line_buf), mod_file_ptr ) == NULL ){ break;}
		if( line_buf[0] != '!' ){
			sscanf( line_buf, "%lf %lf %lf %lf %lf %lf",
				&gaus_comp[comp_index][0],	&gaus_comp[comp_index][1],
				&gaus_comp[comp_index][2],	&gaus_comp[comp_index][3],
				&gaus_comp[comp_index][4],	&gaus_comp[comp_index][5]);

			gaus_comp[comp_index][2] *= DEG2RAD;	/* degree -> rad 		*/
			gaus_comp[comp_index][5] *= DEG2RAD;	/* degree -> rad 		*/
			gaus_comp[comp_index][3] += beam_size;	/* BEAM COMVOLUTION		*/
			gaus_comp[comp_index][4] += beam_size/gaus_comp[comp_index][3];
			gaus_comp[comp_index][0] *= 1.0
				/ (PI2*gaus_comp[comp_index][3]		/* Total Flux -> Peak	*/
				* gaus_comp[comp_index][3]			/* [Jy / mas^2]			*/
				* gaus_comp[comp_index][4]);

			comp2imag(NGRID, NGRID, x_incr, y_incr, gaus_comp[comp_index], imag );
			comp_index++;
		}
	}
	fclose( mod_file_ptr );
	ncomp = comp_index;
/*
---------------------------------------- PEAK BRIGHTNESS
*/
	max_imag	= 0.0;
	for(x_index=0; x_index<NGRID; x_index++){
		for(y_index=0; y_index<NGRID; y_index++){

			if(max_imag < imag[x_index][y_index]){
				max_imag = imag[x_index][y_index];
			}
		}
	}
/*
---------------------------------------- DISP IMAGE
*/
	max_imag	*= 0.9;
	for(cont_index=0; cont_index<NCONT; cont_index++){
		levs[cont_index] = max_imag;
		max_imag	*= 0.5;
	}
	xmax	= x_incr*NGRID/2;	xmin	= -xmax;
	ymax	= y_incr*NGRID/2;	ymin	= -ymax;

	tr[0] =-y_incr*(NGRID/2+1);	tr[1] = 0.00;	tr[2] = y_incr;
	tr[3] =-x_incr*(NGRID/2+1);	tr[4] = x_incr;	tr[5] = 0.00;

	cpgbeg( 1, "/xd", 1, 1 );
	cpgsvp( 0.1, 0.9, 0.1, 0.9 );
	cpgswin( xmax, xmin, ymin, ymax );
	cpgbox( "BCNTS", 0.0, 0,  "BCNTSV", 0.0, 0 );
	cpglab( "Relative R.A.", "Relative DEC.", "MODEL EDITOR");
	cpgcont( &imag[0][0], NGRID, NGRID, 1, NGRID, 1, NGRID, levs, NCONT, tr);
/*
---------------------------------------- DISP GAUSS COMPONENTS
*/
	cpgsci(2);
	cpgsfs(2);
	for(comp_index=0; comp_index<ncomp; comp_index++){
		comp2corner( gaus_comp[comp_index], x_corner, y_corner );
		cpgpoly( 4, x_corner, y_corner );
	}

/*
---------------------------------------- DISP VISIBILITY in MODELVIS
*/
	if(msg_id > 0){
		memcpy( shrd_mem_addr, gaus_comp, sizeof(gaus_comp) );
		send_buf.msg_type = 1;
		sprintf(send_buf.msg_text, "disp %d %d %d",
			ncomp, keyword, sizeof(gaus_comp) );

		if(msgsnd(msg_id, &send_buf, strlen(send_buf.msg_text)+1, 0) < 0){
			printf("Error in [msgsnd] !!");
			exit(0);
		}
	}
/*
---------------------------------------- EDIT MODEL
*/
	edit_status = 1;
	while(edit_status){
		cpgbbuf();
		cpgcurs( &x_curs, &y_curs, curs_key);
		selected_comp = select_comp( x_curs, y_curs, ncomp, gaus_comp );
		switch( curs_key[0] ){
			case 'k':		/* SHIFT UP */
				x_position	= gaus_comp[selected_comp][1]
							* cos(gaus_comp[selected_comp][2]);
				y_position	= gaus_comp[selected_comp][1]
							* sin(gaus_comp[selected_comp][2]);
				x_position += x_incr;
				gaus_comp[selected_comp][1] =
					sqrt( x_position*x_position + y_position*y_position);
				gaus_comp[selected_comp][2] =
					atan2( y_position, x_position );
				break;

			case 'h':		/* SHIFT RIGHT */
				x_position	= gaus_comp[selected_comp][1]
							* cos(gaus_comp[selected_comp][2]);
				y_position	= gaus_comp[selected_comp][1]
							* sin(gaus_comp[selected_comp][2]);
				y_position += y_incr;
				gaus_comp[selected_comp][1] =
					sqrt( x_position*x_position + y_position*y_position);
				gaus_comp[selected_comp][2] =
					atan2( y_position, x_position );
				break;

			case 'l':		/* SHIFT LEFT */
				x_position	= gaus_comp[selected_comp][1]
							* cos(gaus_comp[selected_comp][2]);
				y_position	= gaus_comp[selected_comp][1]
							* sin(gaus_comp[selected_comp][2]);
				y_position -= y_incr;
				gaus_comp[selected_comp][1] =
					sqrt( x_position*x_position + y_position*y_position);
				gaus_comp[selected_comp][2] =
					atan2( y_position, x_position );
				break;

			case 'j':		/* SHIFT DOWN */
				x_position	= gaus_comp[selected_comp][1]
							* cos(gaus_comp[selected_comp][2]);
				y_position	= gaus_comp[selected_comp][1]
							* sin(gaus_comp[selected_comp][2]);
				x_position -= x_incr;
				gaus_comp[selected_comp][1] =
					sqrt( x_position*x_position + y_position*y_position);
				gaus_comp[selected_comp][2] =
					atan2( y_position, x_position );
				break;

			case 'r':		/* ROTATE RIGHT */
				gaus_comp[selected_comp][5] += 0.1;
				break;

			case 't':		/* ROTATE LEFT */
				gaus_comp[selected_comp][5] -= 0.1;
				break;

			case 'w':		/* WIDER */
				gaus_comp[selected_comp][4] += 0.05;
				break;

			case 'q':		/* NARROWER */
				gaus_comp[selected_comp][4] -= 0.05;
				break;

			case 'a':		/* AMPLITUDE 10% BRIGHTER */
				gaus_comp[selected_comp][0] *= 1.1;
				break;

			case 'f':		/* AMPLITUDE 10% FAINTER */
				gaus_comp[selected_comp][0] *= 0.9;
				break;

			case 'e':		/* ENLARGE 10% */
				gaus_comp[selected_comp][3] *= 1.1;
				break;

			case 's':		/* ENLARGE -10% */
				gaus_comp[selected_comp][3] *= 0.9;
				break;

			case '?':		/* HELP */
				printf(" l  ->  Shift Left \n");
				printf(" h  ->  Shift Right \n");
				printf(" k  ->  Shift Up \n");
				printf(" j  ->  Shift Down \n");
				printf(" r  ->  Rotate Right \n");
				printf(" t  ->  Rotate Left \n");
				printf(" w  ->  Wider \n");
				printf(" q  ->  Narrower \n");
				printf(" a  ->  Brighter \n");
				printf(" f  ->  Fainter \n");
				printf(" e  ->  Enlarge \n");
				printf(" s  ->  Smaller \n");
				break;

			case 'x':		/* EXIT FROM EDIT */
				edit_status = 0;
				break;
		}

		/*-------- DELETE OLD LINES --------*/

		cpgsci(0);	cpgsfs(1);					/* Black, Fill */
		cpgrect( xmax, xmin, ymin, ymax );		/* Clear up */
		cpgsci(1);
		cpgbox( "BCNTS", 0.0, 0,  "BCNTSV", 0.0, 0 );

		/*-------- PLOT NEW LINES --------*/
		memset(imag, 0, sizeof(imag));
		for(comp_index=0; comp_index<ncomp; comp_index++){
			comp2imag(NGRID,NGRID, x_incr, y_incr, gaus_comp[comp_index], imag);
		}

		/*-------- PEAK SEARCH --------*/
		max_imag	= 0.0;
		for(x_index=0; x_index<NGRID; x_index++){
			for(y_index=0; y_index<NGRID; y_index++){
				if(max_imag < imag[x_index][y_index]){
					max_imag = imag[x_index][y_index];
				}
			}
		}
		max_imag	*= 0.9;
		for(cont_index=0; cont_index<NCONT; cont_index++){
			levs[cont_index] = max_imag;
			max_imag *= 0.5;
		}

		/*-------- PLOT NEW LINES --------*/
		cpgsci(1);
		cpgcont( &imag[0][0], NGRID, NGRID, 1, NGRID, 1, NGRID,
				levs, NCONT, tr);

		cpgsci(2); cpgsfs(2);
		for(comp_index=0; comp_index<ncomp; comp_index++){
			comp2corner( gaus_comp[comp_index], x_corner, y_corner );
			cpgpoly( 4, x_corner, y_corner );
		}

		/*-------- PLOT SELECTED BOX --------*/
		cpgsci(3);
		comp2corner( gaus_comp[selected_comp], x_corner, y_corner );
		cpgpoly( 4, x_corner, y_corner );

		cpgebuf();

		/*-------- DISP VISIBILITY in MODELVIS --------*/
		if(msg_id > 0){
			memcpy( shrd_mem_addr, gaus_comp, sizeof(gaus_comp) );
			send_buf.msg_type = 1;
			sprintf(send_buf.msg_text, "disp %d %d %d",
				ncomp, keyword, sizeof(gaus_comp) );

			if(msgsnd(msg_id, &send_buf, strlen(send_buf.msg_text)+1, 0) < 0){
				printf("Error in [msgsnd] !!");
				exit(0);
			}
		}

	}
	cpgend();

	if(msg_id > 0){
		send_buf.msg_type = 1;
		sprintf(send_buf.msg_text, "fin %d %d %d",
			ncomp, keyword, sizeof(gaus_comp) );

		if(msgsnd(msg_id, &send_buf, strlen(send_buf.msg_text)+1, 0) < 0){
			printf("Error in [msgsnd] !!");
			exit(0);
		}
	}


/*
---------------------------------------- SAVE MODEL to FILE
*/
	for(comp_index=0; comp_index<ncomp; comp_index++){
		gaus_comp[comp_index][2] /= DEG2RAD;	/* rad	-> degree	*/
		gaus_comp[comp_index][5] /= DEG2RAD;	/* rad	-> degree	*/
		gaus_comp[comp_index][0] *= 
				(PI2*gaus_comp[comp_index][3]	/* Peak -> Total Flux [Jy]*/
				* gaus_comp[comp_index][3]
				* gaus_comp[comp_index][4]);

		fprintf(out_file_ptr,
			"%8.3lf  %8.3lf  %8.3lf  %8.3lf  %8.3lf  %8.3lf  1\n", 
			gaus_comp[comp_index][0],	gaus_comp[comp_index][1],
			gaus_comp[comp_index][2],	gaus_comp[comp_index][3],
			gaus_comp[comp_index][4],	gaus_comp[comp_index][5]);
	}
	fclose( out_file_ptr );

	if(msg_id > 0){
		if(shmctl(shrd_mem_id, IPC_RMID, 0 ) < 0){
			printf("Error in [shmctl]!!");
			exit(1);
		}
	}

	return(0);
}
