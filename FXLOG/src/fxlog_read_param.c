/*********************************************************
**	FXLOG_READ_PARAM.C : Read Param Information.		**
**														**
**	FUNCTION: Read Frequency Chapter.					**
**	AUTHOR	: KAMENO Seiji								**
**	CREATED	: 1994/8/15									**
**********************************************************/

#include "fxlog.inc"
fxlog_read_param()
{
	int		form_counter;				/* Index Number of FORM				*/
	int		data_rate;					/* Data Rate [Mbps]					*/
	int		quant_bit;					/* Quantization Bit [bits/sample]	*/
	int		form_num;					/* Total Number of Formatters		*/
	int		fedge;						/* Edge Frequency [MHz]				*/
	int		fsample;					/* Sampling Frequency per FORM [kHz]*/
	char	sideb_str[8];				/* SideBand Selection				*/
/*
---------------------------------------------------- READ CODE CHAP. 
*/
	form_counter = 0;
	while(1){
		/*--------- Next Chapter or End Of File ----------*/
		if((drg_chapter != CODE) || (drg_eof_flag == 1)) break;

		/*-------- SAMPLE sub-chapter --------*/
		if( strstr(line_buf, "SAMPLE") != NULL ){

			sscanf(line_buf,"%s %d %d %d",
				dum, &data_rate, &quant_bit, &form_num );
			fsample = data_rate* 1000 / (quant_bit* form_num);

			drg_param.form_num	= form_num;
			drg_param.data_rate	= data_rate;
			drg_param.quant_bit	= quant_bit;
			drg_param.fsample	= fsample;
		}

		/*-------- FORM sub-chapter --------*/
		if( strstr(line_buf, "FORM") != NULL ){

			sscanf(line_buf,"%s %d %d %s",
				dum, &form_counter, &fedge, sideb_str);

			drg_param.fedge[form_counter - 1] = fedge;

			if( strstr( sideb_str, "LSB" ) != NULL ){
				drg_param.sideb[form_counter - 1]	= -1; }

			if( strstr( sideb_str, "USB" ) != NULL ){
				drg_param.sideb[form_counter - 1]	= 1; }

		}

		fxlog_detect_chapter();			/* Read 1-Line */
	}	/* while */
/*
---------------------------------------------------- ENDING
*/
	return(1);
}
