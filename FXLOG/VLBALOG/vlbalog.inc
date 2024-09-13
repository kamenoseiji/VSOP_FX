/*****************************************************
**	VLBALOG.INC : Include File for FXLOG CONVERTER	**
**													**
**	AUTHOR	: KAMENO Seiji							**
**	CREATED	: 1995/11/26							**
******************************************************/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>

#define VSNCLG_FMT	"%04d%03d%02d%02d%02d%02d%*5s %s"

#define EQUIP_FMT   "%02d%03d%02d%02d%02d%02d;EQUIP1,*,*,*,*,*,*,VLBA,VLBA\n"
#define OBSCODE_FMT	"%02d%03d%02d%02d%02d%02d/OBSCODE/%s\n"
#define BBC_FMT		"%02d%03d%02d%02d%02d%02d/BBC%d/%d,,16,16,L\n"
#define FORM_FMT	"%02d%03d%02d%02d%02d%02d/FORM=V,32,1:4,off\n"
#define BIT_DENSITY_FMT	"%02d%03d%02d%02d%02d%02d/BIT_DENSITY=56700\n"
#define VSN_FMT		"%02d%03d%02d%02d%02d%02d/VSN/%s\n"
#define LABEL_FMT	"%02d%03d%02d%02d%02d%02d/LABEL=%s,1\n"
#define TRACK_FMT   "%02d%03d%02d%02d%02d%02d/TRACKFORM=2,1US,10,1UM,18,2US,26,2UM\n"
#define SOURCE_FMT	"%02d%03d%02d%02d%02d%02d/SOURCE/%s\n"
#define PASS_FMT	"%02d%03d%02d%02d%02d%02d%s"
#define TAPE_FMT	"%02d%03d%02d%02d%02d%02d%s"
#define ST_FMT		"%02d%03d%02d%02d%02d%02d%s"
#define ET_FMT		"%02d%03d%02d%02d%02d%02d%s"
