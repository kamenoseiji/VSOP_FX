/*****************************************
**	FXAPRI.C: READ FX A PRIORI PARAMS	**
**				AND DISPLAY THEM.		**
**	AUTHOR:		KAMENO S.				**
**	CREATED:	1995/8/17				**
******************************************/

#define	APRFILE	"/home0/nao/nkameno/FX/DATA/apri.dmp"
#define	DISP_STREAM	1
#define	NRESOLV	100
#include	<stdio.h>
#include	<sys/types.h>

MAIN__()
{
	FILE	*fp_apri;		/* A PRIORI PARAM FILE */
	char	line_buf[256];
	char	data_mode[10];
	char	dum[256];
	long	apri_flag;
	long	eof_flag;
	long	itime;
	float	ap_time[20];
	float	a0delay[20];
	float	f0, f1, f2, f3, f4;
	double	a0[16][20];
	double	a1[16][20];
	double	a2[16][20];
	double	a3[16][20];
	double	a4[16][20];
	long	ndata	=0;
	long	istream	=0;
	long	dummy	=0;
	long	just	=0;
	long	axis	=0;
	long	nx		=1;
	long	ny		=1;
	long	pg_point=17;
	long	i;
	long	nresolv;
	float	ymin	= 9999.9;
	float	ymax	=-9999.9;
	double	t_resolv;	
	double	delta_t;	
	float	ref_time[NRESOLV];	
	float	delay[NRESOLV];	

	if((fp_apri	= fopen(APRFILE, "r")) == 0){
		printf("Can't Open APRI FILE !\n");
		exit(0);
	}

	apri_flag	= 0;
	eof_flag	= 0;
	itime 		= -1;
	
	while(1){
		if(fgets(line_buf, sizeof(line_buf), fp_apri) == 0){
			eof_flag	= 1;
			break;
		}

		if(line_buf[0] == '*'){
			sscanf(line_buf, "%s %s", dum, data_mode);
			if(strcmp(data_mode, "Apriori") == 0){
				apri_flag = 1;
			}
		}

		if(apri_flag = 1){
			sscanf(line_buf, "%s %s", dum, data_mode);

			if(strcmp(data_mode, "Time") == 0){
				itime	= itime + 1;
				sscanf(line_buf, "%s %s %s %08s%f",
					dum, data_mode, dum, dum, &ap_time[itime]);
				istream	= -1;
			}

			if(strcmp(data_mode, "Data") == 0){
				istream	= istream + 1;
				sscanf(line_buf, "%s %s %s %e %e %e %e %e",
					dum, data_mode, dum, &f0, &f1, &f2, &f3, &f4); 

				a0[istream][itime]	= (double)f0;
				a1[istream][itime]	= (double)f1;
				a2[istream][itime]	= (double)f2;
				a3[istream][itime]	= (double)f3;
				a4[istream][itime]	= (double)f4;

					if(istream == DISP_STREAM){
						a0delay[itime]	= f0;
						if( a0delay[itime] < ymin){
							ymin	= a0delay[itime];
						}
						if( a0delay[itime] > ymax){
							ymax	= a0delay[itime];
						}
					}
			}	/* End of Data Mode */
		}	/* End of Apri Flag */
	}	/* End of while */
	fclose(fp_apri);
	ndata	= itime + 1;
	pgbegin_(&dummy, "?", &nx, &ny, 1L);
	pgenv_(&ap_time[0], &ap_time[itime], &ymin, &ymax, &just, &axis);
	pgpoint_(&ndata, ap_time, a0delay, &pg_point);

	nresolv		= NRESOLV;
	t_resolv	= 1.0/((double)NRESOLV);
	for(itime=0; itime<ndata; itime++){
		for(i=0; i<NRESOLV; i++){
			delta_t	= (double)i * t_resolv;
			ref_time[i]	= (float)((double)ap_time[itime] + delta_t);
			delay[i]	= (float)((((a4[DISP_STREAM][itime] * delta_t
						+ a3[DISP_STREAM][itime]) * delta_t
						+ a2[DISP_STREAM][itime]) * delta_t
						+ a1[DISP_STREAM][itime]) * delta_t
						+ a0[DISP_STREAM][itime]) ;
		}
		pgline_(&nresolv, ref_time, delay);
	}
	pgend_();
}
