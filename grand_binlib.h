/** \file grand_binlib.h
 *  \brief description of the GRAND binary file
 *
 *
 *  Date: 4/12/2020
 *
 *  Author: C. Timmermans
 */
#include<stdio.h>

#define INTSIZE   4 //size of an integer
#define SHORTSIZE 2 //size of a short

//#define DECPRINT  1 //print ADC values in decimal 

#define USE_EVENT_VERSION    1

/* First define the header words */
#define FILE_HDR_LENGTH          0
#define FILE_HDR_RUNNR           1
#define FILE_HDR_RUN_MODE        2 
#define FILE_HDR_SERIAL          3
#define FILE_HDR_FIRST_EVENT     4
#define FILE_HDR_FIRST_EVENT_SEC 5
#define FILE_HDR_LAST_EVENT      6
#define FILE_HDR_LAST_EVENT_SEC  7
#define FILE_HDR_ADDITIONAL      8 //start of additional info to be defined

#define EVENT_HDR_LENGTH          0
#define EVENT_HDR_RUNNR           2
#define EVENT_HDR_EVENTNR         4 
#define EVENT_HDR_T3EVENTNR       6
#define EVENT_HDR_FIRST_LS        8
#define EVENT_HDR_EVENT_SEC      10
#define EVENT_HDR_EVENT_NSEC     12
#define EVENT_HDR_EVENT_TYPE     14
#ifdef USE_EVENT_VERSION
#define EVENT_HDR_EVENT_VERS     15
#endif
#define EVENT_HDR_AD1            16 //start of additional info to be defined
#define EVENT_HDR_AD2            18 //                    info to be defined
#define EVENT_HDR_LSCNT          20 //                    info to be defined
#define EVENT_LS                 22

/* Next part copied from scope.h */
#define EVENT_TRIGMASK    0
#define EVENT_GPS         2
#define EVENT_STATUS      9 
#define EVENT_CTD        10
#define EVENT_LENCH1     14
#define EVENT_LENCH2     16
#define EVENT_LENCH3     18
#define EVENT_LENCH4     20
#define EVENT_THRES1CH1  22
#define EVENT_THRES2CH1  24
#define EVENT_THRES1CH2  26
#define EVENT_THRES2CH2  28
#define EVENT_THRES1CH3  30
#define EVENT_THRES2CH3  32
#define EVENT_THRES1CH4  34
#define EVENT_THRES2CH4  36
#define EVENT_QUANT1     38
#define EVENT_QUANT2     42
#define EVENT_CTP        46
#define EVENT_SYNC       50 
#define PPS_GPS          52
#define PPS_CTRL         92
#define PPS_WINDOWS     104
#define PPS_CH1         120
#define PPS_CH2         132
#define PPS_CH3         144
#define PPS_CH4         156
#define PPS_TRIG1       168
#define PPS_TRIG2       180
#define PPS_TRIG3       192
#define PPS_TRIG4       204
#define PPS_FILT11      216
#define PPS_FILT12      232
#define PPS_FILT21      248
#define PPS_FILT22      264
#define PPS_FILT31      280
#define PPS_FILT32      296
#define PPS_FILT41      312
#define PPS_FILT42      328
#define EVENT_ADC       344 //472 // latest version, was 344

#define FIRMWARE_VERSION(x) (100*((x>>20)&0xf)+10*((x>>16)&0xf)+((x>>12)&0xf))
#define FIRMWARE_SUBVERSION(x) ((x>>9)&0x7)
#define SERIAL_NUMBER(x) (100*((x>>8)&0x1)+10*((x>>4)&0xf)+((x>>0)&0xf))


#define UINT16 unsigned short


typedef struct{
  short year;
  char month;
  char day;
  char hour;
  char minute;
  char second;
}ElectronicsGPS;

typedef struct{
  short gain;
  char offset;
  char integration;
  unsigned short base_max;
  unsigned short base_min;
  char pm_volt;
  char filter;
  unsigned short spare;
}ChannelProperties;

typedef struct{
  short sig_thres;
  short noise_thres;
  char tprev;
  char tper;
  char tcmax;
  char ncmax;
  char ncmin;
  char qmax;
  char qmin;
  char options;
}ChannelTrigger;

typedef struct{
  char trigmask[2];
  char event_year[2];
  char event_month;
  char event_day;
  char event_hour;
  char event_minute;
  char event_second;
  char status;
  char ctd[4];
  char length[8];
  char thres[16];
  char gps_quant[8];
  char ctp[4];
  char sync[2];
  char serialversion[4];
  char pps_year[2];
  char pps_month;
  char pps_day;
  char pps_hour;
  char pps_minute;
  char pps_second;
  char pps_status;
  char longitude[8];
  char latitude[8];
  char altitude[8];
  char temperature[4];
  char control[2];
  char trigger_enable[2];
  char channel_mask;
  char trigger_divider;
  char coinc_readout[2];
  char ctrl_spare[4];
  char prepost[16];
  ChannelProperties property[4];
  ChannelTrigger chtrigger[4];
  char filter_constants[8][16];
}ElectronicsHeader;



typedef struct{
  UINT16 length;
  UINT16 event_nr;
  UINT16 LS_id;
  UINT16 header_length;
  unsigned int GPSseconds;
  unsigned int GPSnanoseconds;
  UINT16 trigger_flag;
  UINT16 trigger_pos;
  UINT16 sampling_freq;
  UINT16 channel_mask;
  UINT16 ADC_resolution;
  UINT16 tracelength;
#ifdef USE_EVENT_VERSION
  UINT16 version;
#endif
  UINT16 info_ADCbuffer[];
}EventBody;

typedef struct{
  unsigned int length;
  unsigned int runnr;
  unsigned int eventnr;
  unsigned int t3_event;
  unsigned int first_ls;
  unsigned int second;
  unsigned int nanosecond;
  unsigned int version;
  unsigned int ad1;
  unsigned int ad2;
  unsigned int LSCNT;
}EventHeader;

int *grand_read_file_header(FILE *fp, int *size);
unsigned short *grand_read_event(FILE *fp, int *size);

