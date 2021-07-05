/** \file grand_hdf5lib.c
 *  \brief library routines to convert the GRAND binary file into HDF5 format
 *
 *
 *  Date: 14/12/2020
 *
 *  Author: C. Timmermans
 */
#include "grand_misc.h"
#include "grand_hdf5.h"

#define FIELDSIZE 4 /**< hardcoded maximal size of the antenna field (to be changed!) */

/*! Storage of the detector setup*/
AntInfo *field;
int field_size = 0;

/*! the center point of the detector*/
Center center;

/*! HDF5 types for GRAND */
hid_t t_run_header = -1;
/*! HDF5 types for GRAND */
hid_t t_field_center = -1;
/*! HDF5 types for GRAND */
hid_t t_elec_setting = -1;
/*! HDF5 types for GRAND */
hid_t t_event_header = -1;
/*! HDF5 types for GRAND */
hid_t t_antenna_header = -1;
/*! HDF5 types for GRAND */
hid_t t_monitor_info = -1;
/**! Chunked property */
hid_t p_chunked;


/**
 \brief Creates the HDF5 run header structure
 * \return 1: all ok
 * \return 0: nothing needs to be done
 * \return -1: Structure cannot be created
 * \return -2: items cannot be added
* */
int grand_HDF5create_compound_run_header()
{
  hid_t mem_type;
  int return_code = 1;
  hsize_t dim[1];
  
  if(t_run_header>0) return(0);
  if((t_run_header = H5Tcreate( H5T_COMPOUND, sizeof(AntInfo)))<0) return(-1);
  if(H5Tinsert(t_run_header, "antenna_id", HOFFSET(AntInfo,id), H5T_NATIVE_USHORT)<0) return_code = -2;
  if(H5Tinsert(t_run_header, "latitude", HOFFSET(AntInfo,latitude), H5T_NATIVE_DOUBLE)<0) return_code = -2;
  if(H5Tinsert(t_run_header, "longitude", HOFFSET(AntInfo,longitude), H5T_NATIVE_DOUBLE)<0) return_code = -2;
  if(H5Tinsert(t_run_header, "altitude", HOFFSET(AntInfo,altitude), H5T_NATIVE_FLOAT)<0) return_code = -2;
  if(H5Tinsert(t_run_header, "x", HOFFSET(AntInfo,x), H5T_NATIVE_FLOAT)<0) return_code = -2;
  if(H5Tinsert(t_run_header, "y", HOFFSET(AntInfo,y), H5T_NATIVE_FLOAT)<0) return_code = -2;
  mem_type = H5Tcopy (H5T_C_S1); //create string of length 20
  if(H5Tset_size (mem_type, 20)<0) return_code = -2;
  if(H5Tinsert(t_run_header, "antenna_model", HOFFSET(AntInfo,ant_model), mem_type)<0) return_code = -2;
  if(H5Tinsert(t_run_header, "electronics_id", HOFFSET(AntInfo,elec_id), H5T_NATIVE_USHORT)<0) return_code = -2;
  if(H5Tinsert(t_run_header, "electronics_model", HOFFSET(AntInfo,elec_model), mem_type)<0) return_code = -2;
  H5Tclose(mem_type);
  dim[0] = 4;
  if((mem_type = H5Tarray_create(H5T_C_S1,1,dim))<0) return_code = -2;
  if(H5Tinsert(t_run_header, "channel_connections", HOFFSET(AntInfo,channel), mem_type)<0) return_code = -2;
  H5Tclose(mem_type);

  if(return_code < 0){
    H5Tclose(t_run_header);
    t_run_header = -1;
    return(return_code);
  }
  return(1);
}

/**
 \brief Creates the HDF5 field center structure
 * \return 1: all ok
 * \return 0: nothing needs to be done
 * \return -1: Structure cannot be created
 * \return -2: items cannot be added
* */
int grand_HDF5create_compound_field_center()
{
  int return_code = 1;
  
  if(t_field_center >0) return(0);
  if((t_field_center = H5Tcreate( H5T_COMPOUND, sizeof(Center)))<0) return(-1);
  if(H5Tinsert(t_field_center, "latitude", HOFFSET(Center,latitude), H5T_NATIVE_DOUBLE)<0)
    return_code = -2;
  if(H5Tinsert(t_field_center, "longitude", HOFFSET(Center,longitude), H5T_NATIVE_DOUBLE)<0)
    return_code = -2;
  if(H5Tinsert(t_field_center, "altitude", HOFFSET(Center,altitude), H5T_NATIVE_FLOAT)<0)
    return_code = -2;
  if(H5Tinsert(t_field_center, "x", HOFFSET(Center,x), H5T_NATIVE_FLOAT)<0) return_code = -2;
  if(H5Tinsert(t_field_center, "y", HOFFSET(Center,y), H5T_NATIVE_FLOAT)<0) return_code = -2;
  if(return_code < 0){
    H5Tclose(t_field_center);
    t_field_center = -1;
    return(return_code);
  }
  return(1);
}

/**
 \brief Creates the HDF5 electronics settings structure
 * \return 1: all ok
 * \return 0: nothing needs to be done
 * \return -1: Structure cannot be created
 * \return -2: items cannot be added
* */
int grand_HDF5create_compound_elec_setting()
{
  hid_t mem_type;
  hsize_t dim[2]={1,1};
  static hid_t t_channel_prop=-1;
  static hid_t t_channel_trig=-1;
  int return_code = 1;
  
  if(t_channel_prop>0) return(0);
  if((t_channel_prop = H5Tcreate( H5T_COMPOUND, sizeof(ChannelProperties)))<0) return(-1);
  if(H5Tinsert(t_channel_prop, "gain", HOFFSET(ChannelProperties,gain), H5T_NATIVE_SHORT)<0) return_code = -2;
  if(H5Tinsert(t_channel_prop, "offset", HOFFSET(ChannelProperties,offset), H5T_NATIVE_CHAR)<0) return_code = -2;
  if(H5Tinsert(t_channel_prop, "integration", HOFFSET(ChannelProperties,integration), H5T_NATIVE_UCHAR)<0)
    return_code = -2;
  if(H5Tinsert(t_channel_prop, "base_max", HOFFSET(ChannelProperties,base_max), H5T_NATIVE_USHORT)<0)
    return_code = -2;
  if(H5Tinsert(t_channel_prop, "base_min", HOFFSET(ChannelProperties,base_min), H5T_NATIVE_USHORT)<0)
    return_code = -2;
  if(H5Tinsert(t_channel_prop, "pm_volt", HOFFSET(ChannelProperties,pm_volt), H5T_NATIVE_CHAR)<0)
    return_code = -2;
  if(H5Tinsert(t_channel_prop, "filter", HOFFSET(ChannelProperties,filter), H5T_NATIVE_CHAR)<0)
    return_code = -2;
  if(t_channel_trig>0) return(0);
  if((t_channel_trig = H5Tcreate( H5T_COMPOUND, sizeof(ChannelTrigger)))<0) return(-1);
  if(H5Tinsert(t_channel_trig, "signal_threshold", HOFFSET(ChannelTrigger,sig_thres), H5T_NATIVE_USHORT)<0)
    return_code = -2;
  if(H5Tinsert(t_channel_trig, "noise_threshold", HOFFSET(ChannelTrigger,noise_thres), H5T_NATIVE_USHORT)<0)
    return_code = -2;
  if(H5Tinsert(t_channel_trig, "time_previous", HOFFSET(ChannelTrigger,tprev), H5T_NATIVE_UCHAR)<0)
    return_code = -2;
  if(H5Tinsert(t_channel_trig, "time_period", HOFFSET(ChannelTrigger,tper), H5T_NATIVE_UCHAR)<0)
    return_code = -2;
  if(H5Tinsert(t_channel_trig, "time_max", HOFFSET(ChannelTrigger,tcmax), H5T_NATIVE_UCHAR)<0)
    return_code = -2;
  if(H5Tinsert(t_channel_trig, "n_max", HOFFSET(ChannelTrigger,ncmax), H5T_NATIVE_UCHAR)<0)
    return_code = -2;
  if(H5Tinsert(t_channel_trig, "c_min", HOFFSET(ChannelTrigger,ncmin), H5T_NATIVE_UCHAR)<0)
    return_code = -2;
  if(H5Tinsert(t_channel_trig, "charge_max", HOFFSET(ChannelTrigger,qmax), H5T_NATIVE_UCHAR)<0)
    return_code = -2;
  if(H5Tinsert(t_channel_trig, "charge_min", HOFFSET(ChannelTrigger,qmin), H5T_NATIVE_UCHAR)<0)
    return_code = -2;
  if(H5Tinsert(t_channel_trig, "options", HOFFSET(ChannelTrigger,options), H5T_NATIVE_UCHAR)<0)
    return_code = -2;

  if(t_elec_setting >0) return(0);
  if((t_elec_setting = H5Tcreate( H5T_COMPOUND, sizeof(AntInfo)))<0) return(-1);
  if(H5Tinsert(t_elec_setting, "electronics_id", HOFFSET(AntInfo,elec_id), H5T_NATIVE_USHORT)<0) return_code = -2;
  if(H5Tinsert(t_elec_setting, "trigger_mask", HOFFSET(AntInfo,elec_setting.trigmask), H5T_NATIVE_SHORT)<0) return_code = -2;
  dim[0] = 4;
  if((mem_type = H5Tarray_create(H5T_NATIVE_USHORT,1,dim))<0) return_code = -2;
  if(H5Tinsert(t_elec_setting, "trace_lengths", HOFFSET(AntInfo,elec_setting.length), mem_type)<0) return_code = -2;
  H5Tclose(mem_type);
  dim[0] = 4;
  dim[1] = 2;
  if((mem_type = H5Tarray_create(H5T_NATIVE_USHORT,2,dim))<0) return_code = -2;
  if(H5Tinsert(t_elec_setting, "thresholds", HOFFSET(AntInfo,elec_setting.thres), mem_type)<0)
    return_code = -2;
  H5Tclose(mem_type);
  if(H5Tinsert(t_elec_setting, "serial_version", HOFFSET(AntInfo,elec_setting.serialversion),
               H5T_NATIVE_UINT)<0) return_code = -2;
  if(H5Tinsert(t_elec_setting, "longitude", HOFFSET(AntInfo,elec_setting.longitude), H5T_NATIVE_DOUBLE)<0)
    return_code = -2;
  if(H5Tinsert(t_elec_setting, "latitude", HOFFSET(AntInfo,elec_setting.latitude), H5T_NATIVE_DOUBLE)<0)
    return_code = -2;
  if(H5Tinsert(t_elec_setting, "altitude", HOFFSET(AntInfo,elec_setting.altitude), H5T_NATIVE_DOUBLE)<0)
    return_code = -2;
  if(H5Tinsert(t_elec_setting, "control", HOFFSET(AntInfo,elec_setting.control), H5T_NATIVE_USHORT)<0)
    return_code = -2;
  if(H5Tinsert(t_elec_setting, "trigger_enable", HOFFSET(AntInfo,elec_setting.trigger_enable),
               H5T_NATIVE_USHORT)<0) return_code = -2;
  if(H5Tinsert(t_elec_setting, "channel_mask", HOFFSET(AntInfo,elec_setting.channel_mask), H5T_NATIVE_CHAR)<0)
    return_code = -2;
  if(H5Tinsert(t_elec_setting, "trigger_divider", HOFFSET(AntInfo,elec_setting.trigger_divider),
               H5T_NATIVE_CHAR)<0) return_code = -2;
  if(H5Tinsert(t_elec_setting, "coincidence_readout", HOFFSET(AntInfo,elec_setting.coinc_readout),
               H5T_NATIVE_USHORT)<0) return_code = -2;
  if(H5Tinsert(t_elec_setting, "ctrl", HOFFSET(AntInfo,elec_setting.ctrl_spare), H5T_NATIVE_USHORT)<0)
    return_code = -2;
  dim[0] = 4;
  dim[1] = 2;
  if((mem_type = H5Tarray_create(H5T_NATIVE_USHORT,2,dim))<0) return_code = -2;
  if(H5Tinsert(t_elec_setting, "pre_post_length", HOFFSET(AntInfo,elec_setting.prepost), mem_type)<0)
    return_code = -2;
  H5Tclose(mem_type);
  dim[0] = 4;
  if((mem_type = H5Tarray_create(t_channel_prop,1,dim))<0) return_code = -2;
  if(H5Tinsert(t_elec_setting, "channel_properties", HOFFSET(AntInfo,elec_setting.property), mem_type)<0)
    return_code = -2;
  H5Tclose(mem_type);
  dim[0] = 4;
  if((mem_type = H5Tarray_create(t_channel_trig,1,dim))<0) return_code = -2;
  if(H5Tinsert(t_elec_setting, "channel_trigger", HOFFSET(AntInfo,elec_setting.chtrigger), mem_type)<0)
    return_code = -2;
  H5Tclose(mem_type);
  dim[0] = 8;
  dim[1] = 16;
  if((mem_type = H5Tarray_create(H5T_NATIVE_UCHAR,2,dim))<0) return_code = -2;
  if(H5Tinsert(t_elec_setting, "filter_setting", HOFFSET(AntInfo,elec_setting.filter_constants), mem_type)<0)
    return_code = -2;
  H5Tclose(mem_type);
  if(return_code < 0){
    H5Tclose(t_elec_setting);
    t_elec_setting = -1;
    return(return_code);
  }
  return(1);
}

/**
 \brief Creates the HDF5 antenna structure
 * \return 1: all ok
 * \return 0: nothing needs to be done
 * \return -1: Structure cannot be created
 * \return -2: items cannot be added
* */
int grand_HDF5create_compound_antenna_header()
{
  hid_t mem_type;
  hsize_t dim[1]={1};
  int return_code = 1;
  
  if(t_antenna_header > 0) return(0);
  if((t_antenna_header = H5Tcreate( H5T_COMPOUND, sizeof(AntHdr)))<0) return(-1);
  if(H5Tinsert(t_antenna_header, "antenna_id", HOFFSET(AntHdr,id), H5T_NATIVE_SHORT)<0) return_code = -2;
  if(H5Tinsert(t_antenna_header, "gps_sec", HOFFSET(AntHdr,seconds), H5T_NATIVE_UINT)<0) return_code = -2;
  if(H5Tinsert(t_antenna_header, "nanosec", HOFFSET(AntHdr,nano_seconds), H5T_NATIVE_UINT)<0) return_code = -2;
  if(H5Tinsert(t_antenna_header, "trigger_flag", HOFFSET(AntHdr,trigger_flag), H5T_NATIVE_UINT)<0)
    return_code = -2;
  if(H5Tinsert(t_antenna_header, "year", HOFFSET(AntHdr,year), H5T_NATIVE_SHORT)<0) return_code = -2;
  if(H5Tinsert(t_antenna_header, "month", HOFFSET(AntHdr,month), H5T_NATIVE_CHAR)<0) return_code = -2;
  if(H5Tinsert(t_antenna_header, "day", HOFFSET(AntHdr,day), H5T_NATIVE_CHAR)<0) return_code = -2;
  if(H5Tinsert(t_antenna_header, "hour", HOFFSET(AntHdr,hour), H5T_NATIVE_CHAR)<0) return_code = -2;
  if(H5Tinsert(t_antenna_header, "minute", HOFFSET(AntHdr,minute), H5T_NATIVE_CHAR)<0) return_code = -2;
  if(H5Tinsert(t_antenna_header, "second", HOFFSET(AntHdr,sec), H5T_NATIVE_CHAR)<0) return_code = -2;
  if(H5Tinsert(t_antenna_header, "elec_status", HOFFSET(AntHdr,status), H5T_NATIVE_CHAR)<0) return_code = -2;
  if(H5Tinsert(t_antenna_header, "ctd", HOFFSET(AntHdr,ctd), H5T_NATIVE_UINT)<0) return_code = -2;
  dim[0] = 2;
  if((mem_type = H5Tarray_create(H5T_NATIVE_FLOAT,1,dim))<0) return_code = -2;
  if(H5Tinsert(t_antenna_header, "gps_quant", HOFFSET(AntHdr,gps_quant), mem_type)<0) return_code = -2;
  H5Tclose(mem_type);
  if(H5Tinsert(t_antenna_header, "ctp", HOFFSET(AntHdr,ctp), H5T_NATIVE_UINT)<0) return(-2);
  if(H5Tinsert(t_antenna_header, "synchronization", HOFFSET(AntHdr,sync), H5T_NATIVE_USHORT)<0) return_code = -2;
  if(H5Tinsert(t_antenna_header, "temperature", HOFFSET(AntHdr,temperature), H5T_NATIVE_FLOAT)<0)
    return_code = -2;
  if(return_code < 0){
    H5Tclose(t_antenna_header);
    t_antenna_header = -1;
    return(return_code);
  }
  return(1);
}

/**
 \brief Creates the HDF5 event header structure
 * \return 1: all ok
 * \return 0: no action needed
 * \return -1: Structure cannot be created
 * \return -2: items cannot be added
* */
int grand_HDF5create_compound_event_header()
{
  int return_code = 1;
  if(t_event_header > 0) return(0); //it already exists
  if((t_event_header = H5Tcreate( H5T_COMPOUND, sizeof(EventHeader)))<0) return(-1);
  if(H5Tinsert(t_event_header, "run_nr", HOFFSET(EventHeader,runnr), H5T_NATIVE_UINT)<0) return_code = -2;
  if(H5Tinsert(t_event_header, "event_nr", HOFFSET(EventHeader,eventnr), H5T_NATIVE_UINT)< 0) return_code = -2;
  if(H5Tinsert(t_event_header, "t3_nr", HOFFSET(EventHeader,t3_event), H5T_NATIVE_UINT)<0) return_code = -2;
  if(H5Tinsert(t_event_header, "second", HOFFSET(EventHeader,second), H5T_NATIVE_UINT)<0) return_code = -2;
  if(H5Tinsert(t_event_header, "nanosec", HOFFSET(EventHeader,nanosecond), H5T_NATIVE_UINT)<0) return_code = -2;
  if(H5Tinsert(t_event_header, "n_detector", HOFFSET(EventHeader,LSCNT), H5T_NATIVE_UINT)<0) return_code = -2;
  if(return_code < 0){
    H5Tclose(t_event_header);
    t_event_header = -1;
    return(return_code);
  }
  return(1);
}

/**
 \brief Creates the HDF5 Monitor structure
 * \return 1: all ok
 * \return 0: no action needed
 * \return -1: Structure cannot be created
 * \return -2: items cannot be added
* */
int grand_HDF5create_compound_monitor_info()
{
  int return_code = 1;
  
  if(t_monitor_info>0) return(0); //it already exists
  if((t_monitor_info = H5Tcreate( H5T_COMPOUND, sizeof(MonInfo)))<0) return(-1);
  if(H5Tinsert(t_monitor_info, "second", HOFFSET(MonInfo,second), H5T_NATIVE_UINT)<0) return_code = -2;
  if(H5Tinsert(t_monitor_info, "total_rate", HOFFSET(MonInfo,rate[0]), H5T_NATIVE_USHORT)<0) return_code = -2;
  if(H5Tinsert(t_monitor_info, "rate_ch_0", HOFFSET(MonInfo,rate[1]), H5T_NATIVE_USHORT)<0) return_code = -2;
  if(H5Tinsert(t_monitor_info, "rate_ch_1", HOFFSET(MonInfo,rate[2]), H5T_NATIVE_USHORT)<0) return_code = -2;
  if(H5Tinsert(t_monitor_info, "rate_ch_2", HOFFSET(MonInfo,rate[3]), H5T_NATIVE_USHORT)<0) return_code = -2;
  if(H5Tinsert(t_monitor_info, "rate_ch_3", HOFFSET(MonInfo,rate[4]), H5T_NATIVE_USHORT)<0) return_code = -2;
  if(H5Tinsert(t_monitor_info, "temperature", HOFFSET(MonInfo,temp), H5T_NATIVE_FLOAT)<0) return_code = -2;
  if(H5Tinsert(t_monitor_info, "voltage", HOFFSET(MonInfo,volt), H5T_NATIVE_FLOAT)<0) return_code = -2;
  if(H5Tinsert(t_monitor_info, "current", HOFFSET(MonInfo,current), H5T_NATIVE_FLOAT)<0) return_code = -2;
  if(H5Tinsert(t_monitor_info, "status", HOFFSET(MonInfo,status), H5T_NATIVE_USHORT)<0) return_code = -2;
  if(return_code < 0){
    H5Tclose(t_monitor_info);
    t_monitor_info = -1;
    return(return_code);
  }
  return(1);
}

/**
 * \brief create all compound structures used by GRAND in HDF5 format
 */
void grand_HDF5create_compounds()
{
  grand_HDF5create_compound_run_header();
  grand_HDF5create_compound_field_center();
  grand_HDF5create_compound_elec_setting();
  grand_HDF5create_compound_event_header();
  grand_HDF5create_compound_antenna_header();
  grand_HDF5create_compound_monitor_info();
}

/**
 * \brief close all compound structures used by GRAND in HDF5 format
 */
void grand_HDF5close_compounds()
{
  H5Tclose(t_run_header);
  t_run_header = -1;
  H5Tclose(t_field_center);
  t_field_center= -1;
  H5Tclose(t_elec_setting);
  t_elec_setting = -1;
  H5Tclose(t_event_header);
  t_event_header = -1;
  H5Tclose(t_antenna_header);
  t_antenna_header = -1;
  H5Tclose(t_monitor_info);
  t_monitor_info = -1;
}

/**
 * \brief close all compound structures used by GRAND in HDF5 format
 * \return 1: all ok
 * \return 0: no action needed
 * \return -1: Property cannot be created
 * \return -2: items cannot be added
 *  */
int grand_HDF5create_chunked_property()
{
  int rank = 1; //dimensions of the matrix to follow
  hsize_t chunk_dims[1] = {1};
  int return_code = 1;

  if(p_chunked>0) return(0);
  if((p_chunked = H5Pcreate(H5P_DATASET_CREATE))<0) return(-1);
  if(H5Pset_layout(p_chunked, H5D_CHUNKED)<0) return_code = -2;
  if(H5Pset_chunk(p_chunked, rank, chunk_dims)<0)return_code = -2;
  if(H5Pset_deflate(p_chunked,6)<0) return_code = -2; //compression level 6
  if(return_code<0) {
    H5Pclose(p_chunked);
    p_chunked = -1;
  }
  return(return_code);
}


/**
 * \brief Open file, create the run group and compound types
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
  //create compound types
  grand_HDF5create_compounds();
  grand_HDF5create_chunked_property();
  return(1);
}

/**
 * \brief Close file and run group
 * @param[in] file_id: the HDF5 file identifier
 * @param[in] run_id: the run group in the HDF5 file
 */
void grand_HDF5close_file(hid_t run_id,hid_t file_id)
{
  grand_HDF5close_compounds();
  H5Gclose (run_id);
  H5Fclose(file_id);
}

/**
 * \brief Create the field configuration
 * @param[in] fieldname: name of the file containing the field configuration
 *  \return 1: all ok
 *  \return -1: cannot open the file "fieldname"
 *  \return -2: cannot create the memory to hold the field configuration
 */
int grand_HDF5initiate_field(char *fieldname)
{
  FILE *fp;
  double r_earth;
  char line[300];
  int i_field;
  
  if((fp = fopen(fieldname,"r"))== NULL)return(-1);
  field_size = 0;
  while(fgets(line,299,fp)>0){
    if(line[0] != '#') field_size++;
  }
  fseek(fp,0,SEEK_SET);
  if((field = (AntInfo *)malloc(field_size*sizeof(AntInfo)))<0) {
    fclose(fp);
    return(-2);
  }
  i_field = 0;
  while(fgets(line,299,fp)>0){
    if(line[0] == '#') continue;
    sscanf(line,"%hd %hd %lg %lg %g %s %s %c %c %c %c",
           &field[i_field].id,&field[i_field].elec_id,
           &field[i_field].longitude,&field[i_field].latitude,&field[i_field].altitude,
           field[i_field].ant_model,field[i_field].elec_model,
           &field[i_field].channel[0],&field[i_field].channel[1],
           &field[i_field].channel[2],&field[i_field].channel[3]);
    i_field++;
  }
  fclose(fp);
  
  memset((void *)&center,0,sizeof(Center));
  for(int i=0;i<field_size;i++){
    center.latitude +=field[i].latitude/field_size;
    center.longitude +=field[i].longitude/field_size;
    center.altitude +=field[i].altitude/field_size;
  }
  center.x = 0;
  center.y = 0;
  r_earth = rad_earth(center.latitude);
  for(int i=0;i<field_size;i++){
    field[i].y = cos(center.latitude/RADTODEG)*(center.longitude-field[i].longitude)*r_earth/RADTODEG;
    field[i].x =(field[i].latitude-center.latitude)*r_earth/RADTODEG;
  }
  return(1);
}


/**
 \brief Fills the run header with the appropriate electronics info
* @param[in] iant: identifier of the antenna to be filled
* @param[in] electronics_header: The electronics header information
* */
void grand_HDF5fill_electronicsheader(int iant,char *electronics_header)
{
  memcpy(&(field[iant].elec_setting),electronics_header,sizeof(ElectronicsHeader));
}




/**
 * \brief Create and fill the event tables
 * @param[in] run_id: the run group in the HDF5 file
 * @param[in] *event: buffer containing the raw event
 * \return 1: all ok
 * \return -1: Cannot create the event or raw group
 * \return -2: Event data problem
  */
int grand_HDF5fill_event(hid_t run_id,unsigned short *event)
{
  hid_t event_id,raw_id,antenna_id;
  hid_t data_set,space,trspace;   /* file identifier */
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
  int iused[field_size];
  char *trname[3]={"ADC_X","ADC_Y","ADC_Z"};
  int itrace;

  sprintf(grpname,"Event_%d",eh->eventnr);
  if((event_id = H5Gcreate(run_id, grpname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))<0){
    return(-1);
  }
  if((raw_id = H5Gcreate(event_id, "raw", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))<0){
    printf("Cannot create group %s\n",grpname);
    H5Gclose(event_id);
    return(-1);
  }
  if((space = H5Screate_simple(rank, dim, NULL))< 0){
    H5Gclose(raw_id);
    H5Gclose(event_id);
    return(-2);
  }
  if((data_set = H5Dcreate(raw_id, "EventHeader", t_event_header, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))<0){
    H5Sclose(space);
    H5Gclose(raw_id);
    H5Gclose(event_id);
    return(-2);

  }
  status = H5Dwrite(data_set, t_event_header, H5S_ALL, H5S_ALL, H5P_DEFAULT, (const void *)event);
  H5Sclose(space);
  H5Dclose(data_set);
  if(status<0) return(-2);
    
  dim[0] = eh->LSCNT;
  space = H5Screate_simple(rank, dim, NULL);

  ah= malloc(eh->LSCNT*sizeof(AntHdr));
  ic = 0;
  for(iant=0;iant<field_size;iant++)iused[iant] = 0;
  while(ils<ev_end){
    eb = (EventBody *)(&event[ils]);
    raw = (char *)eb->info_ADCbuffer;
    elh = (ElectronicsHeader *)raw;
    iant = -1;
    for(int i=0;i<field_size;i++){
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
    if((antenna_id = H5Gcreate(raw_id, grpname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))<0){
      printf("Cannot create group %s\n",grpname);
      break;
    }
    grand_HDF5fill_electronicsheader(iant,raw);
    /* Write Traces */
    for(int itr=0;itr<4;itr++){
      itrace = -1;
      if(field->channel[itr] == 'X' || field->channel[itr] == 'x') itrace = 0;
      if(field->channel[itr] == 'Y' || field->channel[itr] == 'y') itrace = 1;
      if(field->channel[itr] == 'Z' || field->channel[itr] == 'z') itrace = 2;
      trlen = *(unsigned short *)&raw[EVENT_LENCH1+2*itr];
      if(itrace <0) {
        ioff+=trlen;
        continue;
      }
      if(trlen != 0){
        dim[0] = trlen;
        trspace = H5Screate_simple(rank, dim, NULL);
        if((data_set = H5Dcreate(antenna_id,trname[itrace], H5T_NATIVE_SHORT, trspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))<0){
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
  data_set = H5Dcreate(raw_id, "AntennaInfo", t_antenna_header, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Dwrite(data_set, t_antenna_header, H5S_ALL, H5S_ALL, H5P_DEFAULT, (const void *)ah);
  free(ah);
  H5Dclose(data_set);
  H5Sclose(space);
  
  status = H5Gclose (raw_id);
  status = H5Gclose (event_id);
  return(1);
}

/**
 * \brief Create and fill the event tables
 * @param[in] run_id: the run group in the HDF5 file
 * @param[in] *event: buffer containing the raw event
 * \return 1: all ok
 * \return -1: Cannot create the event or raw group
 * \return -2: Event data problem
  */
int grand_HDF5fill_periodic_event(hid_t run_id,unsigned short *event)
{
  int ret_code;
  hid_t per_id = H5Gopen(run_id,"Periodic",H5P_DEFAULT);
  if(per_id<0) return(-1);
  ret_code = grand_HDF5fill_event(per_id,event);
  H5Gclose(per_id);
  return(ret_code);
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
  hid_t per_id,mon_id, data_set[field_size];
  int rank = 1; //dimensions of the matrix to follow
  hsize_t dim[1]={0}; //length of each of the dimensions!
  hsize_t max_dim[1]={H5S_UNLIMITED}; //maximal length of dimensions
  int iant;
  char mon_name[100];
  
  if((per_id = H5Gcreate(run_id, "Periodic", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))<0){
    return(-2);
  }
  H5Gclose(per_id);
  if((mon_id = H5Gcreate(run_id, "Monitor", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT))<0){
    return(-2);
  }
  rank = 1;
  dim[0] = 1; //should be 0
  if((mem_space = H5Screate_simple(rank, dim, max_dim))<0){
    H5Gclose(mon_id);
    return(-3);
  }
  for(iant=0;iant<field_size;iant++){
    sprintf(mon_name,"MonDetector_%d",field[iant].id);
    if((data_set[iant] = H5Dcreate(mon_id, mon_name, t_monitor_info, mem_space, H5P_DEFAULT, p_chunked, H5P_DEFAULT))<0){
      for(int i=0;i<iant;i++) H5Dclose(data_set[i]);
      H5Gclose(mon_id);
      H5Sclose(mem_space);
      return(-2);

    }
  }
  H5Sclose(mem_space);
  for(iant=0;iant<field_size;iant++) H5Dclose(data_set[iant]);
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
  hid_t space,data_set;
  int rank = 1; //dimensions of the matrix to follow
  hsize_t dim[2]={1,1}; //length of each of the dimensions!
  
  rank = 1;
  dim[0] = 4;
  space = H5Screate_simple(rank, dim, NULL);
  data_set = H5Dcreate(run_id, "DetectorInfo", t_run_header, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Dwrite(data_set, t_run_header, H5S_ALL, H5S_ALL, H5P_DEFAULT, (const void *)field);
  H5Dclose(data_set);

  data_set = H5Dcreate(run_id, "ElectronicsSettings", t_elec_setting, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Dwrite(data_set, t_elec_setting, H5S_ALL, H5S_ALL, H5P_DEFAULT, (const void *)field);
  H5Dclose(data_set);

  H5Sclose(space);
  rank = 1;
  dim[0] = 1;
  space = H5Screate_simple(rank, dim, NULL);
  data_set = H5Dcreate(run_id, "CenterField", t_field_center, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Dwrite(data_set, t_field_center, H5S_ALL, H5S_ALL, H5P_DEFAULT, (const void *)&center);
  H5Dclose(data_set);
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
  hid_t data_set[field_size];
  int rank = 1; //dimensions of the matrix to follow
  hsize_t dim[1]={0}; //length of each of the dimensions!
  hsize_t max_dim[1]={H5S_UNLIMITED}; //maximal length of dimensions
  hsize_t start[1],count[1];
  hid_t mem_space,file_space;
  char mon_name[100],mon_line[200];
  int iant;
  int return_code;
  MonInfo monitor;

  fp = fopen(filename,"r");
  if(fp == NULL){ // no monitor info
    return(-1);
  }

  for(iant=0;iant<field_size;iant++){
    sprintf(mon_name,"Monitor/MonDetector_%d",field[iant].id);
    data_set[iant] = H5Dopen(run_id,mon_name, H5P_DEFAULT);
  }
  sprintf(mon_name,"Monitor/MonDetector");
  rank = 1;
  dim[0] = 1;
  if((mem_space = H5Screate_simple(rank, dim, NULL))<0){
    for(iant=0;iant<field_size;iant++) H5Dclose(data_set[iant]);
    fclose(fp);
    return(-3);
  }
  count[0] = 1;
  return_code = 1;
  while(fgets(mon_line,199,fp) == mon_line && return_code>0){
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
    for(int iant=0;iant<field_size &&return_code>0;iant++){
      if(monitor.elec_id == field[iant].elec_id){
        if((file_space = H5Dget_space(data_set[iant]))<0)return_code = -1;
        if(H5Sget_simple_extent_dims(file_space,dim,max_dim)<0) return_code = -2;
        if(return_code != -1) H5Sclose(file_space);
        dim[0] +=1;
        if(H5Dset_extent(data_set[iant], dim)< 0) return_code = -2;
        if((file_space = H5Dget_space(data_set[iant]))<0)return_code = -1;
        start[0] = dim[0]-1;
        if(H5Sselect_hyperslab(file_space, H5S_SELECT_SET, start, NULL, count, NULL)<0)return_code = -2;
        if(H5Dwrite(data_set[iant], t_monitor_info, mem_space, file_space, H5P_DEFAULT, &monitor)<0)
          return_code = -2;
        H5Sclose(file_space);
        break;
      }
    }
  }
  if(return_code == -2) H5Sclose(file_space);
  for(iant=0;iant<field_size;iant++) H5Dclose(data_set[iant]);
  H5Sclose(mem_space);
  fclose(fp);
  return(return_code);
}

