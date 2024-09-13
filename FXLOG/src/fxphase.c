/*****************************************
**	FXAPRI.C: READ FX A PRIORI PARAMS	**
**				AND DISPLAY THEM.		**
**	AUTHOR:		KAMENO S.				**
**	CREATED:	1995/8/17				**
******************************************/

#define	NAOCOPHASE	"/home0/nao/nkameno/FX/DATA/naoco.phs"
#define	FXPHASE		"/home0/nao/nkameno/FX/DATA/fx.phs"
#define	NCH		64
#include	<stdio.h>
#include	<sys/types.h>

MAIN__()
{
	FILE	*naoco_phs;		/* A PRIORI PARAM FILE */
	FILE	*fx_phs;		/* A PRIORI PARAM FILE */
	char	line_buf[256];
	char	dum[256];
	float	fx_phase[NCH];
	float	phs_diff[NCH];
	float	naoco_phase[NCH];
	float	freq[NCH];
	float	amp;
	long	ndata	=0;
	long	dummy	=0;
	long	just	=0;
	long	axis	=0;
	long	nx		=1;
	long	ny		=1;
	long	pg_point=17;
	long	icolor;
	long	j;
	float	phs_min	=-3.14149265358979;
	float	phs_max	= 3.14149265358979;

	if((fx_phs	= fopen(FXPHASE, "r")) == 0){
		printf("Can't Open FX PHASE FILE !\n");
		exit(0);
	}

	if((naoco_phs	= fopen(NAOCOPHASE, "r")) == 0){
		printf("Can't Open NAOCO PHASE FILE !\n");
		exit(0);
	}

	j=0;
	while(1){
		if(fgets(line_buf, sizeof(line_buf), fx_phs) == 0){
			break;
		}
		sscanf(line_buf, "%f %f", &freq[j], &fx_phase[j]);

		if(fgets(line_buf, sizeof(line_buf), naoco_phs) == 0){
			break;
		}
		sscanf(line_buf, "%f %f %f", &freq[j], &amp, &naoco_phase[j]);
		if(naoco_phase[j] < phs_min){
			naoco_phase[j] = naoco_phase[j] + 2.0*phs_max;
		}
		phs_diff[j] = fx_phase[j] - naoco_phase[j];
		j = j + 1;
	}
	fclose(fx_phs);
	fclose(naoco_phs);

	ndata	= j;
	printf("READ %d DATA POINTS.\n", ndata);
	pgbegin_(&dummy, "?", &nx, &ny, 1L);
	pgenv_(&freq[0], &freq[NCH-1], &phs_min, &phs_max, &just, &axis);
	pglabel_("NAOCO PHASE [RAD]", "FX PHASE [RAD]", "PHASE COMPARATION", 17L, 14L, 17L);
	icolor=2;
	pgsci_(&icolor);
	pgpoint_(&ndata, freq, fx_phase, &pg_point);
	icolor=4;
	pg_point=8;
	pgsci_(&icolor);
	pgpoint_(&ndata, freq, naoco_phase, &pg_point);
	icolor=1;
	pgsci_(&icolor);
	pgline_(&ndata, freq, phs_diff);
	pgend_();
}
