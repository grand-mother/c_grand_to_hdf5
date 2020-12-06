#include "grand_hdf5.h"


int main(int argc, char **argv) {
  //First: The binary data
  char filename[200];
  int runnr,fileseq;
  //HDF5 stuff
  char hdfname[100];
  hid_t       file_id,run_id;

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
  sprintf(filename,"%s/AD/ad%06d.f%04d",argv[1],runnr,fileseq);
  grand_HDF5fill_run(filename,run_id);
  grand_HDF5create_run_structure(run_id);
  // Next: monitoring data
  sprintf(filename,"%s/MON/MO%06d.f%04d",argv[1],runnr,fileseq);
  grand_HDF5fill_monitor(filename,run_id);
  //place 4 antennas in the run
  grand_HDF5fill_runheader(run_id);
  grand_HDF5close_file(run_id,file_id);
}
