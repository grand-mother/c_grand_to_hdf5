/** \file grand_hdf5lib.c
 *  \brief library routines to convert the GRAND binary file into HDF5 format
 *
 *
 *  Date: 4/12/2020
 *
 *  Author: C. Timmermans
 */
#include "grand_misc.h"
#include "grand_hdf5.h"

#define FIELDSIZE 4 /**< hardcoded maximal size of the antenna field (to be changed!) */

/*! Storage of the detector setup*/
AntInfo field[FIELDSIZE];

/*! the center point of the detector*/
Center center;

/**
 * \brief Open file and create the run group
 * @param[in] hdfname: the pathname of the binary GRAND file
 * @param[in] runnr: the run number
 * @param[out] *file_id: pointer to the HDF5 file identifier
 * @param[out] *run_id: pointer to the run group in the HDF5 file
 * \return 1: all ok
 * \return -2: the HDF5 file cannot be opened/created
 */
int grand_HDF5create_file(char *hdfname,int runnr,hid_t *file_id, hid_t *run_id)
{
  char buf[100];
  
  if((*file_id = H5Fcreate(hdfname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT))<0){
    return(-2);
  }
  //next: create the run as a group
  sprintf(buf,"/Run_%d",runnr);
  if((*run_id = H5Gcreate(*file_id, buf, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))<0){
    H5Fclose(*file_id);
    return(-2);
  }
  return(1);
}

/**
 * \brief Close file and run group
 * @param[in] file_id: the HDF5 file identifier
 * @param[in] run_id: the run group in the HDF5 file
 */
void grand_HDF5close_file(hid_t run_id,hid_t file_id)
{
  H5Gclose (run_id);
  H5Fclose(file_id);
}

/**
 * \brief Create the field configuration
 * currently a fully hardcoded routine to initiate the setup of the detector. To be replaced!
 */
void grand_HDF5initiate_field()
{
  double r_earth;

  field[0].id = 1;
  field[0].longitude = 92.3454847;
  field[0].latitude = 38.8522560;
  field[0].altitude = 2712.06;
  strcpy(field[0].ant_model,"GP35");
  field[0].elec_id = 127;
  strcpy(field[0].elec_model,"AERA");
  field[1].id = 2;
  field[1].longitude = 92.3441787;
  field[1].latitude = 38.8524178;
  field[1].altitude = 2712.11;
  strcpy(field[1].ant_model,"GP35");
  field[1].elec_id = 105;
  strcpy(field[1].elec_model,"AERA");
  field[2].id = 3;
  field[2].longitude = 92.3446684;
  field[2].latitude = 38.8533595;
  field[2].elec_id = 126;
  field[2].altitude = 2711.16;
  strcpy(field[2].ant_model,"GP35");
  strcpy(field[2].elec_model,"AERA");
  field[3].id = 4;
  field[3].longitude = 92.344811;
  field[3].latitude = 38.852674;
  field[3].altitude = 2712.0;
  strcpy(field[3].ant_model,"GP35");
  field[3].elec_id = 0;
  strcpy(field[3].elec_model,"R&S SCOPE");
  memset((void *)&center,0,sizeof(Center));
  for(int i=0;i<4;i++){
    center.latitude +=field[i].latitude/4.;
    center.longitude +=field[i].longitude/4.;
    center.altitude +=field[i].altitude/4.;
  }
  center.x = 0;
  center.y = 0;
  r_earth = rad_earth(center.latitude);
  for(int i=0;i<4;i++){
    field[i].y = cos(center.latitude/RADTODEG)*(center.longitude-field[i].longitude)*r_earth/RADTODEG;
    field[i].x =(field[i].latitude-center.latitude)*r_earth/RADTODEG;
  }
}


/**
 \brief Fills the run header with the appropriate electronics info
* @param[in] iant: identifier of the antenna to be filled
* @param[in] elechdr: The electronics header information
* */
void grand_HDF5fill_electronicsheader(int iant,char *Elechdr)
{
  memcpy(&(field[iant].elec_setting),Elechdr,sizeof(ElectronicsHeader));
}


/**
 * \brief Create and fill the event tables
 * @param[in] run_id: the run group in the HDF5 file
 * @param[in] *event: buffer containing the raw event
  */
void grand_HDF5fill_event(hid_t run_id,unsigned short *event)
{
  hid_t event_id,event_tid,antenna_id,antenna_tid;
  hid_t data_set,space,trspace,mem_type;   /* file identifier */
  herr_t      status;
  int rank = 1; //dimensions of the matrix to follow
  hsize_t dim[1]={1}; //length of each of the dimensions!
  char grpname[50];
  EventHeader *eh = (EventHeader *)event;
  int ils = EVENT_LS;
  EventBody *eb;
  int ev_end = ((int)(event[EVENT_HDR_LENGTH+1]<<16)+(int)(event[EVENT_HDR_LENGTH]))/SHORTSIZE;
  AntHdr *ah;
  ElectronicsHeader *elh;
  int iant,ic;
  char *raw;
  int trlen,ioff;
  int iused[FIELDSIZE];
  char *trname[3]={"ADC_X","ADC_Y","ADC_Z"};

  if(eh->LSCNT<3)return;
  sprintf(grpname,"Event_%d",eh->eventnr);
  if((event_id = H5Gcreate(run_id, grpname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))<0){
    printf("Cannot create group %s\n",grpname);
    return;
  }
  space = H5Screate_simple(rank, dim, NULL);
  event_tid = H5Tcreate( H5T_COMPOUND, sizeof(EventHeader) );
  H5Tinsert(event_tid, "evt_run_nr", HOFFSET(EventHeader,runnr), H5T_NATIVE_UINT);
  H5Tinsert(event_tid, "evt_event_nr", HOFFSET(EventHeader,eventnr), H5T_NATIVE_UINT);
  H5Tinsert(event_tid, "evt_t3__nr", HOFFSET(EventHeader,t3_event), H5T_NATIVE_UINT);
  H5Tinsert(event_tid, "evt_second", HOFFSET(EventHeader,second), H5T_NATIVE_UINT);
  H5Tinsert(event_tid, "evt_nanosec", HOFFSET(EventHeader,nanosecond), H5T_NATIVE_UINT);
  H5Tinsert(event_tid, "evt_n_detector", HOFFSET(EventHeader,LSCNT), H5T_NATIVE_UINT);

  data_set = H5Dcreate(event_id, "EventHeader", event_tid, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Dwrite(data_set, event_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, (const void *)eh);
  H5Tclose(event_tid);
  H5Sclose(space);
  H5Dclose(data_set);
  
  dim[0] = eh->LSCNT;
  space = H5Screate_simple(rank, dim, NULL);
  antenna_tid = H5Tcreate( H5T_COMPOUND, sizeof(AntHdr) );
  H5Tinsert(antenna_tid, "evt_antenna_id", HOFFSET(AntHdr,id), H5T_NATIVE_SHORT);
  H5Tinsert(antenna_tid, "evt_gps_sec", HOFFSET(AntHdr,seconds), H5T_NATIVE_UINT);
  H5Tinsert(antenna_tid, "evt_nanosec", HOFFSET(AntHdr,nano_seconds), H5T_NATIVE_UINT);
  H5Tinsert(antenna_tid, "evt_trigger_flag", HOFFSET(AntHdr,trigger_flag), H5T_NATIVE_UINT);
  H5Tinsert(antenna_tid, "evt_year", HOFFSET(AntHdr,year), H5T_NATIVE_SHORT);
  H5Tinsert(antenna_tid, "evt_month", HOFFSET(AntHdr,month), H5T_NATIVE_CHAR);
  H5Tinsert(antenna_tid, "evt_day", HOFFSET(AntHdr,day), H5T_NATIVE_CHAR);
  H5Tinsert(antenna_tid, "evt_hour", HOFFSET(AntHdr,hour), H5T_NATIVE_CHAR);
  H5Tinsert(antenna_tid, "evt_minute", HOFFSET(AntHdr,minute), H5T_NATIVE_CHAR);
  H5Tinsert(antenna_tid, "evt_second", HOFFSET(AntHdr,sec), H5T_NATIVE_CHAR);
  H5Tinsert(antenna_tid, "evt_elec_status", HOFFSET(AntHdr,status), H5T_NATIVE_CHAR);
  //printf("%08x (%08x)\n",ee->ctd,*(int *)&Elechdr[EVENT_CTD]);
  H5Tinsert(antenna_tid, "evt_ctd", HOFFSET(AntHdr,ctd), H5T_NATIVE_UINT);
  dim[0] = 2;
  mem_type = H5Tarray_create(H5T_NATIVE_FLOAT,1,dim);
  H5Tinsert(antenna_tid, "evt_gps_quant", HOFFSET(AntHdr,gps_quant), mem_type);
  H5Tinsert(antenna_tid, "evt_ctp", HOFFSET(AntHdr,ctp), H5T_NATIVE_UINT);
  H5Tinsert(antenna_tid, "evt_synchronization", HOFFSET(AntHdr,sync), H5T_NATIVE_USHORT);
  H5Tinsert(antenna_tid, "evt_temperature", HOFFSET(AntHdr,temperature), H5T_NATIVE_FLOAT);
  ah= malloc(eh->LSCNT*sizeof(AntHdr));
  ic = 0;
  for(iant=0;iant<FIELDSIZE;iant++)iused[iant] = 0;
  while(ils<ev_end){
    eb = (EventBody *)(&event[ils]);
    raw = (char *)eb->info_ADCbuffer;
    elh = (ElectronicsHeader *)raw;
    iant = -1;
    for(int i=0;i<FIELDSIZE;i++){
      if((eb->LS_id&0xff) == field[i].elec_id) {
        iant = i;
        iused[iant] += 1;
      }
    }
    if(iant == -1) {
      ils+=(eb->length);
      continue;
    }
    ah[ic].id = iant+1;
    ah[ic].seconds = eb->GPSseconds;
    ah[ic].nano_seconds = eb->GPSnanoseconds;
    ah[ic].trigger_flag = eb->trigger_flag;
    ah[ic].year =*(short *)(elh->event_year);
    ah[ic].month =elh->event_month;
    ah[ic].day =elh->event_day;
    ah[ic].hour =elh->event_hour;
    ah[ic].minute =elh->event_minute;
    ah[ic].sec =elh->event_second;
    ah[ic].status =elh->status;
    ah[ic].ctd =*(unsigned int *)elh->ctd;
    ah[ic].ctp =*(unsigned int *)elh->ctp;
    ah[ic].gps_quant[0] =*(float *)&elh->gps_quant[0];
    ah[ic].gps_quant[1] =*(float *)&elh->gps_quant[4];
    ah[ic].sync =*(unsigned short *)elh->sync;
    ah[ic].temperature =*(float *)elh->temperature;
    //printf("         GPS(%d): %02d-%02d-%d %02d:%02d:%02d Status 0x%02x Long %10.7f Lat %10.7f Alt %g Temp %g\n",
    //       (eb->LS_id&0xff),raw[PPS_GPS+7],raw[PPS_GPS+6],*((unsigned short *)&raw[PPS_GPS+4]),
    //       raw[PPS_GPS+8],raw[PPS_GPS+9],raw[PPS_GPS+10],raw[PPS_GPS+11],
    // RADTODEG*(*(double *)&raw[PPS_GPS+12]),RADTODEG*(*(double *)&raw[PPS_GPS+20]),*(double *)&raw[PPS_GPS+28],*(float *)&raw[PPS_GPS+36]);
    ioff = EVENT_ADC;
    if(iused[iant] == 1)  sprintf(grpname,"Traces_%d",iant+1);
    else  sprintf(grpname,"Traces_Antenna_%d_%d",iant+1,iused[iant]);
    if((antenna_id = H5Gcreate(event_id, grpname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))<0){
      printf("Cannot create group %s\n",grpname);
      break;
    }
    grand_HDF5fill_electronicsheader(iant,raw);
    /* Write Traces */
    for(int itr=0;itr<3;itr++){
      trlen = *(unsigned short *)&raw[EVENT_LENCH1+2*itr];
      if(trlen != 0){
        dim[0] = trlen;
        trspace = H5Screate_simple(rank, dim, NULL);
        if((data_set = H5Dcreate(antenna_id,trname[itr], H5T_NATIVE_SHORT, trspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))<0){
          printf("Cannot create data_set %s %s\n",grpname,trname[itr]);
          break;
        }
        status = H5Dwrite(data_set,H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, (const void *)&raw[ioff]);
        H5Dclose(data_set);
        H5Sclose(trspace);
      }
      ioff+=trlen;
    }
    H5Gclose(antenna_id);
    ic++;
    if(ic>=eh->LSCNT) break;
    ils+=(eb->length);
  }
  data_set = H5Dcreate(event_id, "AntennaInfo", antenna_tid, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Dwrite(data_set, antenna_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, (const void *)ah);
  free(ah);
  H5Dclose(data_set);
  H5Tclose(antenna_tid);
  H5Sclose(space);
  
  status = H5Gclose (event_id);

}

/**
 * \brief Fill the run tables as well as the events
 * @param[in] filename: the pathname of the binary GRAND file
 * @param[in] run_id: the run group in the HDF5 file
 * \return 1: all ok
 * \return -1: the binary data file cannot be opened
 */
int grand_HDF5fill_run(char *filename, hid_t run_id)
{
  FILE *fp;
  int filelength,readlength;
  unsigned short *event;

  fp = fopen(filename,"r");
  if(fp == NULL) {
    return(-1);
  }
  fseek(fp, 0, SEEK_END);
  filelength = ftell(fp);
  rewind(fp);
  grand_read_file_header(fp,&readlength);
  grand_HDF5initiate_field();
  while((event = grand_read_event(fp,&readlength))!= NULL){
    grand_HDF5fill_event(run_id,event);

  }
  fclose(fp);
  return(1);
}



/**
 \brief create the tables and groups in the run
* @param[in] run_id: identifier of the run-group inside the hdf5 file
* \return 1: all ok
* \return -2: if there is a problem with hdf5 file
* \return -3: other problems
* */
int grand_HDF5create_run_structure(hid_t run_id)
{
  hid_t mem_space;
  hid_t mon_id, mon_tid, data_set[FIELDSIZE];
  hid_t plist;
  int rank = 1; //dimensions of the matrix to follow
  hsize_t dim[1]={0}; //length of each of the dimensions!
  hsize_t max_dim[1]={H5S_UNLIMITED}; //maximal length of dimensions
  int iant;
  char mon_name[100];
  
  if((mon_id = H5Gcreate(run_id, "Monitor", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))<0){
    return(-2);
  }
  rank = 1;
  dim[0] = 1; //should be 0
  if((mem_space = H5Screate_simple(rank, dim, max_dim))<0){
    H5Gclose(mon_id);
    return(-3);
  }
  // create property list
  if((plist = H5Pcreate(H5P_DATASET_CREATE))<0){
    H5Gclose(mon_id);
    H5Sclose(mem_space);
    return(-3);
  }
  if(H5Pset_layout(plist, H5D_CHUNKED)<0){ // make it chunked!
    H5Pclose(plist);
    H5Gclose(mon_id);
    H5Sclose(mem_space);
    return(-3);
  }
  hsize_t chunk_dims[1] = {1};
  if(H5Pset_chunk(plist, rank, chunk_dims)<0){
    H5Pclose(plist);
    H5Gclose(mon_id);
    H5Sclose(mem_space);
    return(-3);
  }
  if((mon_tid = H5Tcreate( H5T_COMPOUND, sizeof(MonInfo)))<0){
    H5Pclose(plist);
    H5Gclose(mon_id);
    H5Sclose(mem_space);
    return(-3);
  }
  H5Tinsert(mon_tid, "mon_second", HOFFSET(MonInfo,second), H5T_NATIVE_UINT);
  H5Tinsert(mon_tid, "mon_total_rate", HOFFSET(MonInfo,rate[0]), H5T_NATIVE_USHORT);
  H5Tinsert(mon_tid, "mon_rate_ch_0", HOFFSET(MonInfo,rate[1]), H5T_NATIVE_USHORT);
  H5Tinsert(mon_tid, "mon_rate_ch_1", HOFFSET(MonInfo,rate[2]), H5T_NATIVE_USHORT);
  H5Tinsert(mon_tid, "mon_rate_ch_2", HOFFSET(MonInfo,rate[3]), H5T_NATIVE_USHORT);
  H5Tinsert(mon_tid, "mon_rate_ch_3", HOFFSET(MonInfo,rate[4]), H5T_NATIVE_USHORT);
  H5Tinsert(mon_tid, "mon_temperature", HOFFSET(MonInfo,temp), H5T_NATIVE_FLOAT);
  H5Tinsert(mon_tid, "mon_voltage", HOFFSET(MonInfo,volt), H5T_NATIVE_FLOAT);
  H5Tinsert(mon_tid, "mon_current", HOFFSET(MonInfo,current), H5T_NATIVE_FLOAT);
  H5Tinsert(mon_tid, "mon_status", HOFFSET(MonInfo,status), H5T_NATIVE_USHORT);
  H5Tcommit(mon_id,"MonDetector",mon_tid,H5P_DEFAULT, H5P_DEFAULT,H5P_DEFAULT);
  for(iant=0;iant<FIELDSIZE;iant++){
    sprintf(mon_name,"MonDetector_%d",field[iant].id);
    if((data_set[iant] = H5Dcreate(mon_id, mon_name, mon_tid, mem_space, H5P_DEFAULT, plist, H5P_DEFAULT))<0){
      for(int i=0;i<iant;i++) H5Dclose(data_set[i]);
      H5Tclose(mon_tid);
      H5Pclose(plist);
      H5Gclose(mon_id);
      H5Sclose(mem_space);
      return(-2);

    }
  }
  H5Pclose(plist);
  H5Sclose(mem_space);
  for(iant=0;iant<FIELDSIZE;iant++) H5Dclose(data_set[iant]);
  H5Tclose(mon_tid);
  H5Gclose(mon_id);
  return(1);
}

/**
 \brief create and fill the run header tables, field and center must have been filled already!
* @param[in] run_id: identifier of the run-group inside the hdf5 file
* */
void grand_HDF5fill_runheader(hid_t run_id)
{
  int status;
  hid_t run_tid,space,mem_type,data_set;
  int rank = 1; //dimensions of the matrix to follow
  hsize_t dim[2]={1,1}; //length of each of the dimensions!
  
  rank = 1;
  dim[0] = 4;
  space = H5Screate_simple(rank, dim, NULL);
  run_tid = H5Tcreate( H5T_COMPOUND, sizeof(AntInfo) );
  H5Tinsert(run_tid, "run_antenna_id", HOFFSET(AntInfo,id), H5T_NATIVE_USHORT);
  H5Tinsert(run_tid, "run_latitude", HOFFSET(AntInfo,latitude), H5T_NATIVE_DOUBLE);
  H5Tinsert(run_tid, "run_longitude", HOFFSET(AntInfo,longitude), H5T_NATIVE_DOUBLE);
  H5Tinsert(run_tid, "run_altitude", HOFFSET(AntInfo,altitude), H5T_NATIVE_FLOAT);
  H5Tinsert(run_tid, "run_x", HOFFSET(AntInfo,x), H5T_NATIVE_FLOAT);
  H5Tinsert(run_tid, "run_y", HOFFSET(AntInfo,y), H5T_NATIVE_FLOAT);
   mem_type = H5Tcopy (H5T_C_S1); //create string of length 20
  status = H5Tset_size (mem_type, 20);
  H5Tinsert(run_tid, "run_antenna_model", HOFFSET(AntInfo,ant_model), mem_type);
  H5Tinsert(run_tid, "run_electronics_id", HOFFSET(AntInfo,elec_id), H5T_NATIVE_USHORT);
  H5Tinsert(run_tid, "run_electronics_model", HOFFSET(AntInfo,elec_model), mem_type);
  data_set = H5Dcreate(run_id, "DetectorInfo", run_tid, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Dwrite(data_set, run_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, (const void *)field);
  H5Dclose(data_set);
  H5Tclose(run_tid);
  run_tid = H5Tcreate( H5T_COMPOUND, sizeof(AntInfo) );
  H5Tinsert(run_tid, "run_electronics_id", HOFFSET(AntInfo,elec_id), H5T_NATIVE_USHORT);
  H5Tinsert(run_tid, "run_trigger_mask", HOFFSET(AntInfo,elec_setting.trigmask), H5T_NATIVE_SHORT);
  dim[0] = 4;
  mem_type = H5Tarray_create(H5T_NATIVE_USHORT,1,dim);
  H5Tinsert(run_tid, "run_trace_lengths", HOFFSET(AntInfo,elec_setting.length), mem_type);
  dim[0] = 4;
  dim[1] = 2;
  mem_type = H5Tarray_create(H5T_NATIVE_USHORT,2,dim);
  H5Tinsert(run_tid, "run_thresholds", HOFFSET(AntInfo,elec_setting.thres), mem_type);
  H5Tinsert(run_tid, "run_serial_version", HOFFSET(AntInfo,elec_setting.serialversion), H5T_NATIVE_UINT);
  H5Tinsert(run_tid, "run_longitude", HOFFSET(AntInfo,elec_setting.longitude), H5T_NATIVE_DOUBLE);
  H5Tinsert(run_tid, "run_latitude", HOFFSET(AntInfo,elec_setting.latitude), H5T_NATIVE_DOUBLE);
  H5Tinsert(run_tid, "run_altitude", HOFFSET(AntInfo,elec_setting.altitude), H5T_NATIVE_DOUBLE);
  H5Tinsert(run_tid, "run_control", HOFFSET(AntInfo,elec_setting.control), H5T_NATIVE_USHORT);
  H5Tinsert(run_tid, "run_trigger_enable", HOFFSET(AntInfo,elec_setting.trigger_enable), H5T_NATIVE_USHORT);
  H5Tinsert(run_tid, "run_channel_mask", HOFFSET(AntInfo,elec_setting.channel_mask), H5T_NATIVE_CHAR);
  H5Tinsert(run_tid, "run_trigger_divider", HOFFSET(AntInfo,elec_setting.trigger_divider), H5T_NATIVE_CHAR);
  H5Tinsert(run_tid, "run_coincidence_readout", HOFFSET(AntInfo,elec_setting.coinc_readout), H5T_NATIVE_USHORT);
  H5Tinsert(run_tid, "run_ctrl", HOFFSET(AntInfo,elec_setting.ctrl_spare), H5T_NATIVE_USHORT);
  dim[0] = 4;
  dim[1] = 2;
  mem_type = H5Tarray_create(H5T_NATIVE_USHORT,2,dim);
  H5Tinsert(run_tid, "run_pre_post_length", HOFFSET(AntInfo,elec_setting.prepost), mem_type);

  data_set = H5Dcreate(run_id, "ElectronicsSettings", run_tid, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Dwrite(data_set, run_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, (const void *)field);
  H5Dclose(data_set);
  H5Tclose(run_tid);

  H5Sclose(space);
  rank = 1;
  dim[0] = 1;
  space = H5Screate_simple(rank, dim, NULL);
  run_tid = H5Tcreate( H5T_COMPOUND, sizeof(AntInfo) );
  H5Tinsert(run_tid, "center_latitude", HOFFSET(Center,latitude), H5T_NATIVE_DOUBLE);
  H5Tinsert(run_tid, "center_longitude", HOFFSET(Center,longitude), H5T_NATIVE_DOUBLE);
  H5Tinsert(run_tid, "center_altitude", HOFFSET(Center,altitude), H5T_NATIVE_FLOAT);
  H5Tinsert(run_tid, "center_x", HOFFSET(Center,x), H5T_NATIVE_FLOAT);
  H5Tinsert(run_tid, "center_y", HOFFSET(Center,y), H5T_NATIVE_FLOAT);
  data_set = H5Dcreate(run_id, "CenterField", run_tid, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Dwrite(data_set, run_tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, (const void *)&center);
  H5Dclose(data_set);
  H5Tclose(run_tid);
  H5Sclose(space);

}


/**
 \brief fill the monitoring information
* @param[in]  filename: name of the ascii monitor file defined by the DAQ
* @param[in]  run_id: identifier of the run-group inside the hdf5 file
* \return 1: all ok
* \return -1: if there is a problem with monitor file
* \return -2: if there is a problem with hdf5 file
* \return -3: other problems
 */
int grand_HDF5fill_monitor(char *filename, hid_t run_id)
{
  FILE *fp;
  hid_t mon_tid,data_set[FIELDSIZE];
  int rank = 1; //dimensions of the matrix to follow
  hsize_t dim[1]={0}; //length of each of the dimensions!
  hsize_t max_dim[1]={H5S_UNLIMITED}; //maximal length of dimensions
  hsize_t start[1],count[1];
  hid_t mem_space,file_space;
  char mon_name[100],mon_line[200];
  int iant;
  MonInfo monitor;

  fp = fopen(filename,"r");
  if(fp == NULL){ // no monitor info
    return(-1);
  }

  for(iant=0;iant<FIELDSIZE;iant++){
    sprintf(mon_name,"Monitor/MonDetector_%d",field[iant].id);
    data_set[iant] = H5Dopen(run_id,mon_name, H5P_DEFAULT);
  }
  sprintf(mon_name,"Monitor/MonDetector");
  mon_tid = H5Topen(run_id,mon_name,H5P_DEFAULT);
  rank = 1;
  dim[0] = 1;
  if((mem_space = H5Screate_simple(rank, dim, NULL))<0){
    for(iant=0;iant<FIELDSIZE;iant++) H5Dclose(data_set[iant]);
    H5Tclose(mon_tid);
    fclose(fp);
    return(-3);
  }
  count[0] = 1;
  while(fgets(mon_line,199,fp) == mon_line){
    sscanf(mon_line,"%hd %hd %hd %d %hd %hd %hd %hd %hd %g %g %g %hd",
           &(monitor.elec_id),
           &(monitor.elec_serial),
           &(monitor.firmware),
           &(monitor.second),
           &(monitor.rate[0]),
           &(monitor.rate[1]),
           &(monitor.rate[2]),
           &(monitor.rate[3]),
           &(monitor.rate[4]),
           &(monitor.temp),
           &(monitor.volt),
           &(monitor.current),
           &(monitor.status)
           );
    for(int iant=0;iant<FIELDSIZE;iant++){
      if(monitor.elec_id == field[iant].elec_id){
        if((file_space = H5Dget_space(data_set[iant]))<0){
          for(iant=0;iant<FIELDSIZE;iant++) H5Dclose(data_set[iant]);
          H5Sclose(mem_space);
          H5Tclose(mon_tid);
          fclose(fp);
          return(-2);
        }
        if(H5Sget_simple_extent_dims(file_space,dim,max_dim)<0){
          for(iant=0;iant<FIELDSIZE;iant++) H5Dclose(data_set[iant]);
          H5Sclose(file_space);
          H5Sclose(mem_space);
          H5Tclose(mon_tid);
          fclose(fp);
          return(-2);
        }
        H5Sclose(file_space);
        dim[0] +=1;
        if(H5Dset_extent(data_set[iant], dim)< 0){
          for(iant=0;iant<FIELDSIZE;iant++) H5Dclose(data_set[iant]);
          H5Sclose(mem_space);
          H5Tclose(mon_tid);
          fclose(fp);
          return(-2);
        }
        if((file_space = H5Dget_space(data_set[iant]))<0){
          for(iant=0;iant<FIELDSIZE;iant++) H5Dclose(data_set[iant]);
          H5Sclose(mem_space);
          H5Tclose(mon_tid);
          fclose(fp);
          return(-2);
        }
        start[0] = dim[0]-1;
        if(H5Sselect_hyperslab(file_space, H5S_SELECT_SET, start, NULL, count, NULL)<0){
          for(iant=0;iant<FIELDSIZE;iant++) H5Dclose(data_set[iant]);
          H5Sclose(file_space);
          H5Sclose(mem_space);
          H5Tclose(mon_tid);
          fclose(fp);
          return(-2);
        }
        if(H5Dwrite(data_set[iant], mon_tid, mem_space, file_space, H5P_DEFAULT, &monitor)<0){
          for(iant=0;iant<FIELDSIZE;iant++) H5Dclose(data_set[iant]);
          H5Sclose(file_space);
          H5Sclose(mem_space);
          H5Tclose(mon_tid);
          fclose(fp);
          return(-2);
        }
        H5Sclose(file_space);
        break;
      }
    }
  }
  for(iant=0;iant<FIELDSIZE;iant++) H5Dclose(data_set[iant]);
  H5Sclose(mem_space);
  H5Tclose(mon_tid);
  fclose(fp);
  return(1);
}

