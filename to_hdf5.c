#include "grand_hdf5.h"


int main(int argc, char **argv) {
  //First: The binary data
  char filename[200];
  int runnr,fileseq;
  int readlength;
  unsigned short *event;
  //HDF5 stuff
  char hdfname[100];
  FILE *fp;
  hid_t       file_id,run_id;
  int nevt;

  if(argc != 4){
    printf("Use: to_hdf5 [basedir] [runnr] [fileseq]\n");
    return(-1);
  }
  if(sscanf(argv[2],"%d",&runnr)!= 1){
    printf("Use: to_hdf5 [basedir] [runnr] [fileseq]\n");
    return(-1);
  }
  if(sscanf(argv[3],"%d",&fileseq) != 1){
    printf("Use: to_hdf5 [basedir] [runnr] [fileseq]\n");
    return(-1);
  }
  sprintf(hdfname,"Run%d.hdf5",runnr);
  grand_HDF5create_file(hdfname,runnr, &file_id,&run_id);
  grand_HDF5initiate_field("field_run22.txt");

  sprintf(filename,"%s/AD/ad%06d.f%04d",argv[1],runnr,fileseq);
  fp = fopen(filename,"r");
  if(fp != NULL) {
    nevt = 0;
    grand_read_file_header(fp,&readlength);
    while((event = grand_read_event(fp,&readlength))!= NULL){
      if(((EventHeader *)event)->LSCNT<1)continue;
      grand_HDF5fill_event(run_id,event);
      nevt++;
    }
    fclose(fp);
  }
  grand_HDF5create_run_structure(run_id);
  /*sprintf(filename,"%s/TD/td%06d.f%04d",argv[1],runnr,fileseq);
  fp = fopen(filename,"r");
  if(fp != NULL) {
    grand_read_file_header(fp,&readlength);
    while((event = grand_read_event(fp,&readlength))!= NULL){
      if(((EventHeader *)event)->LSCNT<1)continue;
      grand_HDF5fill_periodic_event(run_id,event);
      nevt++;
    }
    fclose(fp);
  }
  printf("Wrote %d events\n",nevt);
  // Next: monitoring data
  sprintf(filename,"%s/MON/MO%06d.f%04d",argv[1],runnr,fileseq);
  grand_HDF5fill_monitor(filename,run_id);*/
  //place 4 antennas in the run
  grand_HDF5fill_runheader(run_id);
  grand_HDF5close_file(run_id,file_id);
}
