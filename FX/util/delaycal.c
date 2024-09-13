#include <stdio.h>
#include <math.h>
#include "cpgplot.h"
#include "delaydata.inc"
#include "merge.inc"
#define    TOTALLAG    512
#define    NSPEC    16
#define    NSS        16
#define    NCPP    256    
#define    MAX_SS    16
#define    MAX_ANT 5
#define    MAX_BL    MAX_ANT*(MAX_ANT-1)/2
#define    MAX_VAR    2*(MAX_BL*MAX_SS + MAX_ANT - 1)
#define    SEC2RAD    4.84813681109535993589e-6
#define    RAD2DEG    57.29577951308232087721

MAIN__(argc, argv)
    long    argc;                /* Number of Arguments */
    char    **argv;                /* Pointer of Arguments */
{
    FILE    *corr_file[MAX_BL][MAX_SS];
    FILE    *mrg_file_ptr[NSS];    /* File Pointer to Save Delay Data */
    FILE    *delay_file_ptr;    /* File Pointer to Save Delay Data */
    char    out_file_name[32];
    long    eof_flag;
    long    first_flag;
    long    start;
    long    integ;
    long    stop_time;
    long    end_time;
    long    start_integ;
    double    rf[MAX_SS];            /* Observing Frequency [MHz] */
    double    freq_incr[MAX_SS];    /* Frequency increment [MHz] */
    double    fsample;            /* Sampling Freq. [MHz] */
    double    cpp_len;            /* Duration of Compressed PP [sec] */
    float    vis_r[MAX_BL*MAX_SS*NCPP*NSPEC];
    float    vis_i[MAX_BL*MAX_SS*NCPP*NSPEC];
    float    *vr_ptr[MAX_BL*MAX_SS];
    float    *vi_ptr[MAX_BL*MAX_SS];
    long    ncpp;
    struct    delay_file_header    delay_header;
    struct    delay_data            delay_bl[MAX_BL];
    struct    ant_delay_data        delay_ant;
    long    blnum;
    long    antnum;
    long    clnum;
    long    icon;                /* CONDITION CODE */
    long    ant1,ant2,ant3;
    long    ant_index;
    long    bl_index;
    long    cl_index;                /* Index of Closure */
    long    ss_index;
    long    time_index;
    long    spec_index;
    long    delay_index;
    long    rate_index;
    long    amp_index;
    long    phs_index;
    long    spec_num[NSS];
    long    var_num;
    float    gff_result[MAX_VAR];
    float    gff_err[MAX_VAR];
    float    rate[MAX_ANT];
    float    amp_err;
    float    afact;
    long    i, j;
    double    ra_2000;
    double    dec_2000;

    long    byte_len;
    long    history_index;            /* Index of History */
    long    bl_id;                    /* ID Number of Baseline */
    long    cl_id;                    /* ID Number of Closure */
    long    mjd;
    long    ut;                        /* From Begin of the Year, [1/60 sec] */
    char    history[80];            /* History */
    struct mrg_header    header;        /* Header in Merge File */
    struct mrg_source    source;        /* Source Information */
    struct mrg_station1    stn_1;        /* Station Format 1 */
    struct mrg_station2    stn_2;        /* Station Format 2 */
    struct mrg_misc        misc;        /* Miscellaneous Information */
    struct mrg_vis1        vis1;        /* Visivility in Format 1 */
    struct mrg_vis2        vis2;        /* Visivility in Format 1 */
    struct mrg_close    cls;        /* Closure Phase */

/*
-------------------------------- INPUT PARAMETERS
*/
    if(argc < 7){
        printf("USAGE : delaycal [START] [STOP] [INTEG] [DELAY FILE] [MRG FILE] [PATH1] [NUM1] [PATH2] [NUM2] ... !!\n");
        exit(0);
    }

    if( (delay_file_ptr = fopen(argv[4], "r")) == NULL){
        printf("Can't Open Delay File [%s] !!\n", argv[4]);
    }

    for(ss_index=0; ss_index<NSS; ss_index++){
        sprintf( out_file_name, "%s.%02d", argv[5], ss_index+1);
        if( (mrg_file_ptr[ss_index] = fopen(out_file_name, "w")) == NULL){
            printf("Can't Open Output File [%s] !!\n", out_file_name);
            exit(0);
        }
    }

    /*-------- READ FILE HEADER FROM DELAY FILE --------*/
    if( fread(&delay_header, 1, sizeof(delay_header), delay_file_ptr)
        != sizeof(delay_header) ){
        printf("Can't Read File Header from File !!\n");
        fclose(delay_file_ptr);
        return(0);
    }

    antnum    = delay_header.antnum;
    blnum = (antnum-1)*antnum/2;
    clnum = (antnum-2)*(antnum-1)*antnum/6;
    var_num    = 2*(blnum * NSS + antnum - 1);
    printf("%d BASELINEs, %d ANTENNAE\n", blnum, antnum);

    /*-------- READ DELAY HEADER FROM DELAY FILE --------*/
    for(ant_index=0; ant_index < antnum; ant_index++){
        if( fread(&delay_ant, 1, sizeof(delay_ant), delay_file_ptr)
            != sizeof(delay_ant) ){
            printf("Can't Read Ant Header from File [ANT = %d]!!\n", ant_index);
            fclose(delay_file_ptr);
            return(0);
        }
    }

    fread(&gff_result, 1, sizeof(gff_result), delay_file_ptr);
    for(ant_index=0; ant_index < antnum-1; ant_index++){
        rate[ant_index] = gff_result[98+ant_index];
        gff_result[98+ant_index] = 0.0;
    }
    fclose(delay_file_ptr);
/*
-------------------------------- START TIME
*/
    start    = 3600*(atoi(argv[1])/10000)
            + 60*((atoi(argv[1])/100)%100)
            + atoi(argv[1])%100;

    stop_time    = 3600*(atoi(argv[2])/10000)
            + 60*((atoi(argv[2])/100)%100)
            + atoi(argv[2])%100;

    integ    = atoi(argv[3]);
    end_time= start + integ;
/*
-------------------------------- WRITE HEADER TO MERGE FILE
*/
    header.format_version    = 1;
    header.history_num        = 1;
    header.ant_num            = antnum;
    header.bl_num            = blnum;
    header.cl_num            = 0;
    header.record_len        = 4*header.bl_num + 2*header.cl_num + 1;
    header.cal_flag            = 0;
    header.stokes_index        = 0;
    header.header_num        = header.history_num + antnum + 3;
    if(header.cl_num > 0){
        header.header_num++;
    }

    /*-------- WRITE FILE HEADER --------*/
    byte_len = sizeof(header);
    for(ss_index=0; ss_index<NSS; ss_index++){
        fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
        fwrite( &header, 1, sizeof(header), mrg_file_ptr[ss_index]);
        fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
    }

    /*-------- WRITE BASELINE ID --------*/
    byte_len = sizeof(bl_id)*header.bl_num;
    for(ss_index=0; ss_index<NSS; ss_index++){
        fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
        for(bl_index=0; bl_index<header.bl_num; bl_index++){
            bl2ant( bl_index, &ant1, &ant2);
            bl_id    = (ant2+1)*100 + (ant1+1);
            fwrite( &bl_id, 1, sizeof(bl_id), mrg_file_ptr[ss_index]);
        }
        fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
    }

    /*-------- WRITE CLOSURE ID --------*/
    for(ss_index=0; ss_index<NSS; ss_index++){
        if(header.cl_num > 0){
            byte_len = sizeof(cl_id)*header.cl_num;
            fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
            for(cl_index=0; cl_index<header.cl_num; cl_index++){
                cl2ant( cl_index, &ant1, &ant2, &ant3);
                cl_id    = (ant3+1)*10000 + (ant2+1)*100 + (ant1+1);
                fwrite( &cl_id, 1, sizeof(cl_id), mrg_file_ptr[ss_index]);
            }
            fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
        }
    }

    /*-------- WRITE CLOSURE ID --------*/
    byte_len    = 80;
    for(ss_index=0; ss_index<NSS; ss_index++){
        fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
        sprintf(history, "NAOCOMRG ");
        fwrite( history, 1, byte_len, mrg_file_ptr[ss_index]);
        fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
    }
/*
-------------------------------- OPEN NAOCO DATA FILE AND READ HEADER
*/
    /*-------- LOOP FOR BASELINE --------*/
    for(bl_index=0; bl_index<blnum; bl_index++){

        /*-------- LOOP FOR SUB-STREAM --------*/
        for(ss_index=0; ss_index<NSS; ss_index++){

            eof_flag = naoco_open(
                argv[2*bl_index+6],            /* DIRECTORY POINTER */
                atoi(argv[2*bl_index+7]),    /* FILE NUMBER */
                ss_index+1,                    /* Sub-Stream NUMBER */
                &corr_file[bl_index][ss_index]);
            if(eof_flag == -1 ){
                return(0);
            }

            naoco_header(
                &corr_file[bl_index][ss_index],
                ss_index,                    /* Sub-Stream Index */
                &delay_bl[bl_index]);        /* HEADER INFORMATION */

            eof_flag = naoco_skip(
                &corr_file[bl_index][ss_index],
                start,                        /* START TIME */
                integ);                        /* INTEGRATION TIME */

            if(eof_flag != 0 ){
                fclose(corr_file[bl_index][ss_index]);
                return(0);
            }
        }
    }

    /*-------- WRITE SOURCE INFORMATION TO MERGE FILE  --------*/
    sprintf(source.name, "%-016s", delay_bl[0].source_name);

    source.ra_1950    = (60.0*(double)delay_bl[0].rh 
                    + (double)delay_bl[0].rm) * 60.0
                    + delay_bl[0].rs ;
    source.ra_1950    *= 15.0*SEC2RAD;

    if(delay_bl[0].sign[0] == '-'){
        source.dec_1950    = (-60.0*(double)delay_bl[0].dd 
                        - (double)delay_bl[0].dm) * 60.0
                        - delay_bl[0].ds ;
    } else {
        source.dec_1950    = (60.0*(double)delay_bl[0].dd 
                        + (double)delay_bl[0].dm) * 60.0
                        + delay_bl[0].ds ;
    }
    source.dec_1950    *= SEC2RAD;
    if(delay_bl[0].epoch    == 2000000){
        ra_2000    = source.ra_1950;
        dec_2000= source.dec_1950;
        j2000tob1950(ra_2000, dec_2000,
            &source.ra_1950, &source.dec_1950 );
    }

    source.ra_app    = ra_2000;
    source.dec_app    = dec_2000;
    source.total_flux    = 1.0;
    byte_len    = sizeof(source);
    for(ss_index=0; ss_index<NSS; ss_index++){
        fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
        fwrite( &source, 1, sizeof(source), mrg_file_ptr[ss_index]);
        fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
    }

    /*-------- WRITE STATION INFORMATION TO MERGE FILE  --------*/
    for(ss_index=0; ss_index<NSS; ss_index++){

        strcpy(stn_1.name, delay_bl[0].site_name_x);
        stn_1.x    = delay_bl[0].x_pos_x * 1.0e-3;
        stn_1.y    =-delay_bl[0].x_pos_y * 1.0e-3;        /* CIT DEFINITION */
        stn_1.z    = delay_bl[0].x_pos_z * 1.0e-3;
        byte_len    = sizeof(stn_1);

        fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
        fwrite( &stn_1, 1, sizeof(stn_1), mrg_file_ptr[ss_index]);
        fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
        for(ant_index=1; ant_index < antnum; ant_index++){
            bl_index = ant2bl( ant_index, 0);
            strcpy(stn_1.name, delay_bl[bl_index].site_name_y);
            stn_1.x    = delay_bl[bl_index].y_pos_x * 1.0e-3;
            stn_1.y    =-delay_bl[bl_index].y_pos_y * 1.0e-3;
            stn_1.z    = delay_bl[bl_index].y_pos_z * 1.0e-3;
            byte_len    = sizeof(stn_1);
            fwrite(&byte_len,1,sizeof(byte_len),mrg_file_ptr[ss_index]);
            fwrite(&stn_1, 1, sizeof(stn_1), mrg_file_ptr[ss_index]);
            fwrite(&byte_len,1,sizeof(byte_len),mrg_file_ptr[ss_index]);
        }
    }

    /*-------- WRITE MISC INFORMATION TO MERGE FILE  --------*/
    doy2mjd( delay_bl[0].year, delay_bl[0].doy, &mjd ); 
    mjd2gmst( (double)mjd, delay_bl[0].input_ut1utc, &misc.gst );
    misc.year    = delay_bl[0].year;
    misc.ut        = 5184000*(delay_bl[0].doy - 1);
    misc.coh_integ    = (double)integ;
    misc.inc_integ    = 0.0;
    misc.bw        = delay_bl[0].fsample*0.5e6;
    byte_len    = sizeof(misc);
    for(ss_index=0; ss_index<NSS; ss_index++){
        misc.rf        = 1.0e6*delay_bl[0].rf[ss_index];
        fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
        fwrite( &misc, 1, sizeof(misc), mrg_file_ptr[ss_index]);
        fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
    }

/*
-------------------------------- LOOP FOR START TIME
*/
    eof_flag = 0;
    first_flag = 1;
    while(eof_flag != 1){
        printf("INTEG %02d:%02d:%02d - %02d:%02d:%02d\n",
            start/3600, (start/60)%60, start%60,
            (start+integ)/3600, ((start+integ)/60)%60, (start+integ)%60);

        /*-------- LOOP FOR BASELINE --------*/
        for(bl_index=0; bl_index<blnum; bl_index++){

            /*-------- LOOP FOR SUB-STREAM --------*/
            for(ss_index=0; ss_index<NSS; ss_index++){

                vr_ptr[bl_index*NSS + ss_index]
                    = &vis_r[(bl_index*NSS+ss_index)*NCPP*NSPEC];
                vi_ptr[bl_index*NSS + ss_index]
                    = &vis_i[(bl_index*NSS+ss_index)*NCPP*NSPEC];

                /*-------- READ VISIBILITY FROM FILE --------*/

                eof_flag    = naoco_vis(
                    &corr_file[bl_index][ss_index],
                    start,                        /* START TIME */
                    integ,                        /* INTEGRATION TIME */
                    2*NSPEC,            /* LAG NUMBER */
                    vr_ptr[bl_index*NSS + ss_index],
                    vi_ptr[bl_index*NSS + ss_index],
                    &delay_bl[bl_index]);        /* HEADER INFORMATION */

                rf[ss_index]        = delay_bl[bl_index].rf[ss_index];
                spec_num[ss_index]    = NSPEC;
                freq_incr[ss_index]    = delay_bl[bl_index].fsample/NSPEC/2;

            } /*-------- END OF SUB-STREAM LOOP --------*/

        } /*-------- END OF BASELINE LOOP --------*/

        ncpp    = delay_bl[0].ncpp;
        cpp_len    = delay_bl[0].cpp_len;

        amp_err    = 1.0/sqrt( delay_bl[0].fsample*1.0e6 * cpp_len * ncpp);

        printf(" DELAY = %e   %e\n", gff_result[96], gff_result[97]);
        printf(" RATE  = %e   %e\n", gff_result[98], gff_result[99]);

        integ_mult( vr_ptr, vi_ptr, NSS, spec_num, rf, freq_incr,
                ncpp, cpp_len, antnum, gff_result);

        /*-------- WRITE VISIBILITY DATA TO MERGE FILE  --------*/
        ut    = ((delay_bl[0].doy - 1)*86400 + start + integ/2) * 60;
        byte_len    = sizeof(ut) + blnum*sizeof(vis1);
        for(ss_index=0; ss_index<NSS; ss_index++){
            fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
            fwrite( &ut, 1, sizeof(ut), mrg_file_ptr[ss_index]);
            for(bl_index=0; bl_index<blnum; bl_index++){
                amp_index        = bl_index*NSS    + ss_index;
                phs_index        = amp_index + blnum*NSS;
                vis1.amp        = gff_result[amp_index];
                vis1.amp_err    = amp_err;
                vis1.phs        = gff_result[phs_index]*RAD2DEG;
                vis1.phs_err    = amp_err*RAD2DEG/vis1.amp;
                fwrite( &vis1, 1, sizeof(vis1), mrg_file_ptr[ss_index]);
            }
            fwrite( &byte_len, 1, sizeof(byte_len), mrg_file_ptr[ss_index]);
        }

        start    += integ;
        gff_result[96] += rate[0]*(float)integ;
        gff_result[97] += rate[1]*(float)integ;

        if( start >= stop_time ){    break;}
    }

    /*-------- LOOP FOR SUB_STREAM --------*/
    for(ss_index=0; ss_index<NSS; ss_index++){
        fclose(mrg_file_ptr[ss_index]);

        /*-------- LOOP FOR BASELINE --------*/
        for(bl_index=0; bl_index<blnum; bl_index++){

            fclose(corr_file[bl_index][ss_index]);

        }
    }

    return(0);
}
