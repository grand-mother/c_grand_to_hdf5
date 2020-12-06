/** \file grand_hdf5.h
 *  \brief structures and routines  used in GRAND HDF5 file format
 *
 *
 *  Date: 6/12/2020
 *
 *  Author: C. Timmermans
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hdf5.h"
#include "grand_binlib.h"

typedef struct{
  double longitude;
  double latitude;
  float altitude;
  float x,y;
}Center;

typedef struct{
  short id;
  double longitude;
  double latitude;
  float altitude;
  float x,y;
  char ant_model[20];
  short elec_id;
  char elec_model[20];
  ElectronicsHeader elec_setting;
}AntInfo;

typedef struct{
  unsigned short id;
  unsigned int seconds;
  unsigned int nano_seconds;
  unsigned int trigger_flag;
  short year;
  char month;
  char day;
  char hour;
  char minute;
  char sec;
  char status;
  unsigned int ctd;
  float gps_quant[2];
  unsigned int ctp;
  unsigned short sync;
  float temperature;
}AntHdr;

typedef struct{
  unsigned short elec_id;
  unsigned short elec_serial;
  unsigned short firmware;
  unsigned int second;
  unsigned short rate[5];
  float temp;
  float volt;
  float current;
  unsigned short status;
}MonInfo;

int grand_HDF5create_file(char *hdfname,int runnr,hid_t *file_id, hid_t *run_id);
void grand_HDF5close_file(hid_t run_id,hid_t file_id);
void grand_HDF5initiate_field();
void grand_HDF5fill_electronicsheader(int iant,char *Elechdr);
void grand_HDF5fill_event(hid_t run_id,unsigned short *event);
int grand_HDF5fill_run(char *filename, hid_t run_id);
int grand_HDF5create_run_structure(hid_t run_id);
void grand_HDF5fill_runheader(hid_t run_id);
int grand_HDF5fill_monitor(char *filename, hid_t run_id);






