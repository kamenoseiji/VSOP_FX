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
#define	MSG_SIZ	64
#define	MAX_TIME 86400
#define	MAX_ANT	5
#define	MAX_BL	10
#define	MAX_CL	10

struct	msg_buf{
	long	msg_type;
	char	msg_text[MSG_SIZ];
};

MAIN__(argc, argv)
	long	argc;		/* Number of Arguments */
	char	**argv;		/* Pointer of Arguments */
{
	double	*gaus_comp_ptr;	/* Gaussian Component Parameter */
	long	ncomp;			/* Number of Gaussian Components */
	long	comp_index;		/* Index of Gaussian Components */
	long	i;
	char	command[8];

	key_t	keyword;		/* KeyWord for Shared Memory */
	long	shrd_mem_id;	/* Shared Memory ID Number */
	long	shrd_mem_size;	/* Shared Memory Size */
	long	msg_id;
	key_t	msg_key;
	struct	msg_buf	rcv_buf;
	long	receive;
	long	first_time;

	long	ant_index;
	long	bl_index;
	long	cl_index;
	float	y_bottom;
	float	y_top;
	long	ant_num;
	long	bl_num;
	long	cl_num;
	long	ant1, ant2, ant3;
	long	bl12, bl23, bl13;
	long	time_index;
	long	time_num;
	double	time_incr;
	double	mjd_max, mjd_min;
	double	x[MAX_ANT],	y[MAX_ANT],	z[MAX_ANT];
	double	u[MAX_ANT],	v[MAX_ANT],	w[MAX_ANT];
	double	ra;					/* Right Ascension [rad] */
	double	dec;				/* Declination [rad] */
	double	ha;					/* Hour Angle [rad] */
	double	gmst;				/* Greenwidge Sidereal Time [rad] */
	double	ut1utc;				/* UT1 - UTC [sec] */
	double	mjd;				/* Modified Julian Date */
	double	lambda;				/* Wavelength [m] */
	float	u_bl[MAX_BL][MAX_TIME/100];
	float	v_bl[MAX_BL][MAX_TIME/100];
	float	tm_plot[MAX_TIME/100];
	double	uv[2];
	double	vis[2];
	double	gaus_comp[6];
	double	vis_re, vis_im;
	float	vis_amp[MAX_TIME/100];
	float	vis_phs[MAX_BL][MAX_TIME/100];
	float	cl_phs[MAX_TIME/100];
	long	gaus_index;
	char	station_name[MAX_ANT][4];
	char	bl_name[10];
	char	cl_name[16];
/*
---------------------------------------- INITIALIZE MASSAGE QUEUE
*/
	msg_key = 5678;
/*
---------------------------------------- CALC (u,v) v.s. time
*/
	x[0]=-3871023.4900;	y[0]= 3428106.8000;	z[0]= 3724039.5000;	/* NOBEYAMA */
	x[1]=-3997649.2400;	y[1]= 3276690.8100;	z[1]= 3724278.8900;	/* KASHIMA34 */ 
	x[2]=-3855354.0000;	y[2]= 3427426.0000;	z[2]= 3740965.0000;	/* USUDA64 */
	x[3]=-3857236.0300;	y[3]= 3108803.3100;	z[3]= 4003883.1200;	/* MIZUSAWA */
	x[4]=-3537007.8900;	y[4]= 4140258.0200;	z[4]= 3309951.0700;	/* KAGOSHIMA */
	strcpy(station_name[0], "NRO");
	strcpy(station_name[1], "CRL");
	strcpy(station_name[2], "UDS");
	strcpy(station_name[3], "MIZ");
	strcpy(station_name[4], "KAG");

	ra = 4.5;
	dec= 1.5;
	lambda = 0.01348290793793568697;
	ut1utc = 27.0;
	time_incr = 120.0;

	ant_num = 5;
	bl_num	= ant_num*(ant_num - 1)/2;
	cl_num	= ant_num*(ant_num - 1)*(ant_num - 2)/6;
	time_num= (long)MAX_TIME/time_incr;

	mjd_min = 51544.0; 
	for(time_index=0; time_index<time_num; time_index++){
		tm_plot[time_index] = (float)(time_index)*time_incr/3600;
		mjd = (double)mjd_min + (double)(time_index)*time_incr/86400;
		mjd2gmst(mjd, ut1utc, &gmst);
		ha	= gmst - ra;
		for(ant_index=0; ant_index<ant_num; ant_index++){
			xyz2uvw(x[ant_index],	y[ant_index],	z[ant_index],
					ha,				dec,			lambda,
					&u[ant_index],	&v[ant_index],	&w[ant_index]);
		}

		for(bl_index=0; bl_index<bl_num; bl_index++){
			bl2ant(bl_index, &ant2, &ant1);
			u_bl[bl_index][time_index]	= u[ant2] - u[ant1];
			v_bl[bl_index][time_index]	= v[ant2] - v[ant1];
		}
	}

/*
---------------------------------------- PLOT
*/
	receive = cpgbeg( 1, "/xw", 1, 1 );
	cpgsch(0.5);

	if((msg_id = msgget(msg_key, IPC_CREAT|0644))<0){
		printf("Error in [msgget] !!");
		exit(1);
	}

	first_time = 1;
/*
---------------------------------------- LOOP FOR MASSAGE QUEUE
*/
	while(1){
		cpgbbuf();
		/*-------- RECEIVE MESSAGE QUEUE from SERVER --------*/
		if((receive = msgrcv(msg_id, &rcv_buf, MSG_SIZ, 0, 1)) < 0){
			printf("Error in [msgrcv] !!");
			exit(1);
		}

		sscanf(rcv_buf.msg_text, "%s %d %d %d",
			command,	&ncomp, &keyword, &shrd_mem_size);

		if(strcmp(command, "fin") == 0){ break; }

		if(first_time == 1){
			if(( shrd_mem_id = shmget(keyword, shrd_mem_size, 0644)) < 0){
				printf("Error in [shmget] !!");
				exit(1);
			}
			gaus_comp_ptr = shmat( shrd_mem_id, NULL, 0);
			first_time = 0;
		}
/*
---------------------------------------- INIT IMAP
*/

		/*-------- BEGIN OF PLOT --------*/
		for(bl_index=0; bl_index<bl_num; bl_index++){
			for(time_index=0; time_index<time_num; time_index++){
				uv[0] = u_bl[bl_index][time_index];
				uv[1] = v_bl[bl_index][time_index];

				vis_re = 0.0;
				vis_im = 0.0;
				for(comp_index=0; comp_index<ncomp; comp_index++){
					for(gaus_index=0; gaus_index<6; gaus_index++){
						gaus_comp[gaus_index] = *gaus_comp_ptr;
						gaus_comp_ptr++;

					}
					gaus_comp[0]	*= PI2
									*gaus_comp[3]
									*gaus_comp[3]
									*gaus_comp[4];
					gaus_comp[1]	*= MAS2RAD;
					gaus_comp[3]	*= MAS2RAD;

					comp2vis(gaus_comp, uv, vis);

					vis_re	+= vis[0] * cos(vis[1]);
					vis_im	+= vis[0] * sin(vis[1]);
				}
				gaus_comp_ptr -= 6*ncomp;

				vis_amp[time_index] = sqrt(vis_re*vis_re + vis_im*vis_im);
				vis_phs[bl_index][time_index] = atan2(vis_im, vis_re);

			}

			y_bottom= 0.95 - (float)(bl_index+1)*0.9/MAX_BL;
			y_top	= 0.95 - (float)(bl_index)*0.9/MAX_BL;

			cpgsvp( 0.05, 0.475, y_bottom, y_top );
			cpgswin( 0.0, 24.0, 0.0, 2.5 );
			cpgsci(0);
			cpgrect( 0.0, 24.0, 0.0, 2.5 );
			cpgsci(1);
			if(bl_index == 0){
				cpglab("", "", "Visibility Amplitude");
			}
			if(bl_index == bl_num - 1){
				cpgbox( "BCNTS", 4.0, 4,  "BCNTSV", 0.0, 0);
				cpglab("UT", "", "");
			} else {
				cpgbox( "BCTS", 4.0, 4,  "BCNTSV", 0.0, 0);
			}
			cpgsci(5);
			bl2ant(bl_index, &ant2, &ant1);
			sprintf(bl_name, "%s - %s", station_name[ant1], station_name[ant2]);
			cpgtext( 0.5, 2.0, bl_name);

			cpgsci(3);
			cpgline(time_num, tm_plot, vis_amp);

		}

		for(cl_index=0; cl_index<cl_num; cl_index++){
			cl2ant( cl_index, &ant3, &ant2, &ant1 );
			bl12 = ant2bl(ant2, ant1);
			bl23 = ant2bl(ant3, ant2);
			bl13 = ant2bl(ant3, ant1);

			for(time_index=0; time_index<time_num; time_index++){

				cl_phs[time_index]	=	-vis_phs[bl12][time_index]
										- vis_phs[bl23][time_index]
										+ vis_phs[bl13][time_index];
				cl_phs[time_index] = 
					atan2( sin(cl_phs[time_index]), cos(cl_phs[time_index]) );
			}

			y_bottom= 0.95 - (float)(cl_index+1)*0.9/MAX_CL;
			y_top	= 0.95 - (float)(cl_index)*0.9/MAX_CL;
			cpgsvp( 0.525, 0.975, y_bottom, y_top );
			cpgswin( 0.0, 24.0, -PI, PI );
			cpgsci(0);
			cpgrect( 0.0, 24.0, -PI, PI );
			cpgsci(1);
			if(cl_index == 0){
				cpglab("", "", "Closure Phase");
			}
			if(cl_index == cl_num - 1){
				cpgbox( "BCNTS", 4.0, 4,  "BCNTSV", 0.0, 0);
				cpglab("UT", "", "");
			} else {
				cpgbox( "BCTS", 4.0, 4,  "BCNTSV", 0.0, 0);
			}

			cpgsci(5);
			sprintf(cl_name, "%s - %s - %s",
				station_name[ant1], station_name[ant2], station_name[ant3]);
			cpgtext( 0.5, PI*0.6, cl_name);

			cpgsci(3);
			cpgline(time_num, tm_plot, cl_phs);
		}

		/*-------- END OF PLOT --------*/
		cpgebuf();
	}
/*
---------------------------------------- END OF LOOP
	cpgend();
*/
	
	/*-------- RELEASE MESSAGE QUEUE --------*/
	if(msgctl(msg_id, IPC_RMID, 0) < 0 ){
		printf("Error in [msgctl] !!");
		exit(1);
	}

	return(0);
}
