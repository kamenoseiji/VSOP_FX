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

MAIN__(argc, argv)
	long	argc;
	char	**argv;
{
	char	pcfile1[16];
	char	pcfile2[16];
	char	pcfile3[16];
	char	pcfile4[16];
	char	line_buf[256];
	FILE	*pcfile1_ptr;
	FILE	*pcfile2_ptr;
	FILE	*pcfile3_ptr;
	FILE	*pcfile4_ptr;
	float	pp;
	float	phs[3600], phs1, phs2, phs3, phs4;
	long	i;
/*
---------------------------------------------------- LOAD PARAMETERS
*/
	if(argc < 5){
		printf("USAGE : pcaldiff [FILE_NAME] [FIENAME] [FILENAME] [FILENAME] !!\n");
		exit(0);
	}
	strcpy(pcfile1, argv[1]); strcpy(pcfile2, argv[2]);
	strcpy(pcfile3, argv[3]); strcpy(pcfile4, argv[4]);

	pcfile1_ptr = fopen(pcfile1, "r");
	if(pcfile1_ptr == NULL){
		printf("Can't Open Pcal Data File [%s] !\n", pcfile1);
		exit(0);
	}
	pcfile2_ptr = fopen(pcfile2, "r");
	if(pcfile2_ptr == NULL){
		printf("Can't Open Pcal Data File [%s] !\n", pcfile2);
		exit(0);
	}
	pcfile3_ptr = fopen(pcfile3, "r");
	if(pcfile3_ptr == NULL){
		printf("Can't Open Pcal Data File [%s] !\n", pcfile3);
		exit(0);
	}
	pcfile4_ptr = fopen(pcfile4, "r");
	if(pcfile4_ptr == NULL){
		printf("Can't Open Pcal Data File [%s] !\n", pcfile4);
		exit(0);
	}
/*
---------------------------------------------------- CALC PCAL
*/
	cpgbeg(1, "?", 1, 1);
	cpgbbuf();
	cpgenv(00.0, 3600.0, -30.0, 30.0, 0, 0);
	cpglab("PP Number", "Phase DIfference [deg]", "PCAL PHASE (ref = 16MHz)");

	i = 0;
	while(1){
		if(fgets(line_buf, sizeof(line_buf), pcfile1_ptr) == NULL){
			break;
		}
		sscanf(line_buf, "%f %f", &pp, &phs[i]);
		i++;
	}

	i = 0;
	cpgsci(2);
	while(1){
		if(fgets(line_buf, sizeof(line_buf), pcfile2_ptr) == NULL){
			break;
		}
		sscanf(line_buf, "%f %f", &pp, &phs2);
		phs2 = phs2 - phs[i];
		if(phs2 < -180.0){	phs2 = phs2 + 360.0;}
		if(phs2 > 180.0){	phs2 = phs2 - 360.0;}
		if(i == 0){
			cpgmove(pp, phs2);
		} else {
			cpgdraw(pp, phs2);
		}
		i++;
	}

	i = 0;
	cpgsci(3);
	while(1){
		if(fgets(line_buf, sizeof(line_buf), pcfile3_ptr) == NULL){
			break;
		}
		sscanf(line_buf, "%f %f", &pp, &phs3);
		phs3 = phs3 - phs[i];
		if(phs3 < -180.0){	phs3 = phs3 + 360.0;}
		if(phs3 > 180.0){	phs3 = phs3 - 360.0;}
		if(i == 0){
			cpgmove(pp, phs3);
		} else {
			cpgdraw(pp, phs3);
		}
		i++;
	}

	i = 0;
	cpgsci(4);
	while(1){
		if(fgets(line_buf, sizeof(line_buf), pcfile4_ptr) == NULL){
			break;
		}
		sscanf(line_buf, "%f %f", &pp, &phs4);
		phs4 = phs4 - phs[i];
		if(phs4 < -180.0){	phs4 = phs4 + 360.0;}
		if(phs4 > 180.0){	phs4 = phs4 - 360.0;}
		if(i == 0){
			cpgmove(pp, phs4);
		} else {
			cpgdraw(pp, phs4);
		}
		i++;
	}



	cpgebuf();
	cpgend();
	fclose(pcfile1_ptr);
	fclose(pcfile2_ptr);
	fclose(pcfile3_ptr);
	fclose(pcfile4_ptr);
/*
---------------------------------------------------- PGPLOT INIT
*/
}
