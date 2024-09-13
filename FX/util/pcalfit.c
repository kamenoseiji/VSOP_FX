/*********************************************************
**	PCAL.C : Read Pcal Data								**
**														**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/7/27									**
**********************************************************/
#include "cpgplot.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>

#define NCH 2		/* Number of Channel */
#define NBIT 1		/* Number of QUANTIZATION BIT */
#define NDATA 4096
#define NPARM 5
#define PI2   6.2836

MAIN__(argc, argv)
	long	argc;
	char	**argv;
{
	long	filenum;		/* Pcal File Number */
	char	fname[9];		/* Pcal File Name */
	FILE	*pc_file;		/* Pcal File Pointer */
	long	monit_spp;		/* Monitor SPP Number */
	char	pcdata[512];	/* PCAL Data Line */
	char	dum[256];		/* Dummy String */
	float	pr[NCH];		/* Pcal Real Part */
	float	pi[NCH];		/* Pcal Real Part */
	float	amp[NDATA];		/* Pcal Amplitude */
	float	phs[NDATA];		/* Pcal Phase */
	float	x[NDATA];		/* PP Number */
	float	re[NDATA];		/* Pcal Phase versus  PP Number */
	float	im[NDATA];		/* Pcal amp versus  PP Number */
	float	prbuf;
	float	pibuf;
	float	xmin, xmax, ymin, ymax;
	long	i, j, k, bpp, ndata;

	float	cs_phi, sn_phi;
	float	cs_th,	sn_th;
	float	a[NPARM];
	float	r[2*NDATA];
	float	residual;
	float	p[NPARM][2*NDATA];
	float	pwp[NPARM][NPARM];
	float	d[NPARM];
	long	nparm;
	long	ip[NPARM];
	long	icon, isw;
	float	epsz;
	float	vw[NPARM*2];
	long	niter;
	float	correction;
	float	radius;
	float	theta;
	float	plotx[NDATA], ploty[NDATA];

	/* float	tbc = 2097152.0/((float)NCH * (float)NBIT); for K-4 */
	float	tbc = 2097152.0/((float)NCH * (float)NBIT); /* for 32-1-2 */
	/* float	tbc = 2097152.0/((float)NCH * (float)NBIT)*1.0025;  for 16-2-2 */
	float	pfact = 45.0/atan(1.0);
	float	pofs;
/*
---------------------------------------------------- LOAD PARAMETERS
*/
	if(argc < 10){
		printf("USAGE : pcalfit [FILE_NUM] [CH_NUM] [BPP] [NPP] [X_0] [Y_0] [MAJ] [MIN] [PH] !!\n");
		exit(0);
	}
	filenum = atoi(argv[1]);
	sprintf(fname, "PC%06ld", filenum);
	pc_file = fopen(fname, "r");
	if(pc_file == NULL){
		printf("Can't Open Pcal Data File!\n");
		exit(0);
	}

	monit_spp = atoi(argv[2]);
	bpp		= atoi(argv[3]);
	ndata	= atoi(argv[4]);
/*
---------------------------------------------------- CALC PCAL
*/
	for(i=0; i<bpp; i++){ 
		/*-------- SKIP --------*/
		if( fgets(pcdata, sizeof(pcdata), pc_file) == 0){
			fclose(pc_file);
			break;
		}
	}

	xmax = -9999.0; xmin = 9999.0;
	ymax = -9999.0; ymin = 9999.0;
	for(j=0; j<ndata; j++){ 
		prbuf = pr[NCH-1];	pibuf = pi[NCH-1];

		/*-------- READ PCAL DATA FROM FILE --------*/
		if( fgets(pcdata, sizeof(pcdata), pc_file) == 0){
			fclose(pc_file);
			break;
		}
		sscanf(pcdata, "%8s%8f,%8f,%8f,%8f",
			dum, &pr[0],&pi[0],&pr[1],&pi[1]); 

		/*-------- DIFFERENTIAL DATA --------*/
		if(monit_spp == NCH){
			re[j] = (float)((int)(pr[0] - prbuf) & (0xffffff));
			im[j] = (float)((int)(pi[0] - pibuf) & (0xffffff));
		} else {
			re[j] = (float)((int)(pr[monit_spp] - pr[monit_spp-1])
					& (0xffffff));
			im[j] = (float)((int)(pi[monit_spp] - pi[monit_spp-1])
					& (0xffffff));
		}

		/*-------- NORMALIZATION --------*/
		re[j] = 100.0*(2.0*re[j] - tbc)/tbc;
		im[j] = 100.0*(2.0*im[j] - tbc)/tbc;
		if(re[j] > xmax){	xmax = re[j];}
		if(re[j] < xmin){	xmin = re[j];}
		if(im[j] > ymax){	ymax = im[j];}
		if(im[j] < ymin){	ymin = im[j];}

		/*-------- AMP and PHASE --------*/
		amp[j] = sqrt(re[j]*re[j] + im[j]*im[j]);
		if(amp[j] == 0){
			phs[j] = 0.0;
		} else {
			phs[j] = pfact*atan2(im[j], re[j]);
		}
		x[j] = (float)(j+bpp);
	}
/*
---------------------------------------------------- PGPLOT INIT
	cpgbeg(1, "?", 1, 1);
*/
	cpgbeg(1, "/xd", 1, 1);
	cpgbbuf();
	cpgenv((xmin*1.2-xmax*0.2), (xmax*1.2-xmin*0.2),
			(ymin*1.2-ymax*0.2), (ymax*1.2-ymin*0.2), 1, 1);
	cpglab("Real", "Imag", fname);
	cpgpt(j, re, im, 1);
	cpgebuf();

/*
---------------------------------------------------- ELIIPSE FIT
*/
	for(i=0; i<5; i++){
		a[i] = atof(argv[i+5]);
	}

	niter = 100;
	while(niter){
		cs_phi = cos(a[4]);	sn_phi = sin(a[4]);

		/*-------- RESIDUAL VECTOR --------*/
		residual = 0.0;
		for(i=0; i<ndata; i++){
			cs_th  = ( cs_phi*(re[i]-a[0]) + sn_phi*(im[i]-a[1]))/a[2];
			sn_th  = (-sn_phi*(re[i]-a[0]) + cs_phi*(im[i]-a[1]))/a[3];
			radius = sqrt(cs_th*cs_th + sn_th*sn_th);
			cs_th = cs_th / radius;
			sn_th = sn_th / radius;
/*
			printf("%7.4f %7.4f %7.4f %7.4f\n", re[i], im[i],  cs_th, sn_th );
*/

			r[2*i]		= re[i] -(a[0] + a[2]*cs_phi*cs_th - a[3]*sn_phi*sn_th);
			r[2*i+1]	= im[i] -(a[1] + a[2]*sn_phi*cs_th + a[3]*cs_phi*sn_th);
			residual = residual + r[2*i]*r[2*i] + r[2*i+1]*r[2*i+1];

			/*-------- PARTIAL MATRIX --------*/
			p[0][2*i] = 1.0;			p[0][2*i+1] = 0.0;
			p[1][2*i] = 0.0;			p[1][2*i+1] = 1.0;
			p[2][2*i] = cs_th*cs_phi;	p[2][2*i+1] = cs_th*sn_phi;
			p[3][2*i] =-sn_th*sn_phi;	p[3][2*i+1] = sn_th*cs_phi;
			p[4][2*i]	=-a[2]*cs_th*sn_phi - a[3]*sn_th*cs_phi;
			p[4][2*i+1]	= a[2]*cs_th*cs_phi - a[3]*sn_th*sn_phi;

		}
		residual = sqrt(residual/(float)(ndata*2));

		/*-------- PARTIAL MATRIX --------*/
		for(i=0; i<NPARM; i++){
			for(j=0; j<NPARM; j++){
				pwp[i][j] = 0.0;
				for(k=0; k<2*ndata; k++){
					pwp[i][j] = pwp[i][j] + p[i][k]*p[j][k];
				}
			}
		}

		/*-------- FEEDBACK VECTOR --------*/
		for(i=0; i<NPARM; i++){
			d[i] = 0.0;
			for(j=0; j<ndata; j++){
				d[i] = d[i] + p[i][2*j]*r[2*j] + p[i][2*j+1]*r[2*j+1];
			}
		}

		isw = 1; nparm = NPARM; epsz = 0.0;
		alu_(pwp, &nparm, &nparm, &epsz, ip, &isw, vw, &icon);
		if(icon > 0){
			printf("ITERATON HALTED. ICON = %d.\n", icon);
			exit(0);
		}
		isw = 1;
		lux_(d, pwp, &nparm, &nparm, &isw, ip, &icon);
		luiv_(pwp, &nparm, &nparm, ip, &icon);

		/*-------- CORRECTION --------*/
		correction = 0.0;
		for(i=0; i<NPARM; i++){
			a[i] = a[i] + d[i];
			correction = correction + d[i]*d[i];
		}
		niter--;
		correction = sqrt(correction/(float)NDATA);
/*
		printf("Iteration No. %d ... Residual = %13.6e\n", 100-niter, residual);
		printf("%7.3f %7.3f %7.3f %7.3f %7.3f\n",
			a[0], a[1], a[2], a[3], a[4]);
*/
		if(correction < 1.0e-9 ){	break;}
	}

	/*-------- PLOT SOLUTION --------*/
	cpgbbuf();
	cpgsci(3);
	for(i=0; i<100; i++){
		theta = PI2 * (float)i/(float)100;
		plotx[i] = a[0] + a[2]*cos(theta)*cos(a[4])-a[3]*sin(theta)*sin(a[4]); 
		ploty[i] = a[1] + a[2]*cos(theta)*sin(a[4])+a[3]*sin(theta)*cos(a[4]); 
		if(i == 0){
			cpgmove(plotx[i], ploty[i]);
		} else {
			cpgdraw(plotx[i], ploty[i]);
		}
	}
	cpgebuf();

	cpgend();
/*
---------------------------------------------------- PGPLOT INIT
	cpgbeg(1, "?", 1, 2);
*/
	cpgbeg(1, "/xd", 1, 2);
	ymin = 0.0;	ymax = sqrt(xmax*xmax+ymax*ymax)*1.2;
	xmin = (float)bpp;	xmax = (float)(bpp+ndata);
	cpgbbuf();
	cpgenv(xmin, xmax, ymin, ymax, 0, 0);
	cpglab("PP Number", "Pcal-Amp [%]", fname);
	cpgline(ndata, x, amp);
	cs_phi = cos(a[4]);	sn_phi = sin(a[4]);

	cpgsci(3);
	for(i=0; i<ndata; i++){
		cs_th  = ( cs_phi*(re[i]-a[0]) + sn_phi*(im[i]-a[1]))/a[2];
		sn_th  = (-sn_phi*(re[i]-a[0]) + cs_phi*(im[i]-a[1]))/a[3];
		radius = a[2]*sqrt(cs_th*cs_th + sn_th*sn_th);
		if(i==0){
			cpgmove(x[i], radius);
		} else {
			cpgdraw(x[i], radius);
		}
	}

	ymin = -180.0;	ymax = 180.0;
	cpgsci(1);
	cpgenv(xmin, xmax, ymin, ymax, 0, 0);
	cpglab("PP Number", "Pcal-Phase [deg]", fname);
	cpgpt(ndata, x, phs, 1);

	cpgsci(3);
	for(i=0; i<ndata; i++){
		cs_th  = ( cs_phi*(re[i]-a[0]) + sn_phi*(im[i]-a[1]))/a[2];
		sn_th  = (-sn_phi*(re[i]-a[0]) + cs_phi*(im[i]-a[1]))/a[3];

		theta = pfact*atan2(
			(sn_th*cs_phi + cs_th*sn_phi), (cs_th*cs_phi - sn_th*sn_phi));

		printf("%d %8.5f\n", (long)x[i], theta); 
		if(i==0){
			cpgmove(x[i], theta);
		} else {
			cpgdraw(x[i], theta);
		}
	}
	cpgebuf();

/*
---------------------------------------------------- ENDING
*/
	cpgend();
}
