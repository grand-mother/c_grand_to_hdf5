/** \file grand_binlib.c
 *  \brief library routines to read the GRAND binary file
 *
 *
 *  Date: 4/12/2020
 *
 *  Author: C. Timmermans
 */
#include<stdio.h>
#include<stdlib.h>
#include "grand_binlib.h"

/*! pointer to the binary file header information*/
int *file_hdr=NULL;

/*! pointer to the event data */
unsigned short *event=NULL;

/**
 * Fill the run tables as well as the events
 * @param[in] fp: the file pointer of the binary file
 * @param[out] size: pointer to the size of the file header
 * \return NULL:  could not read the file header
 * \return otherwise: valid pointer to the file header information
 */
int *grand_read_file_header(FILE *fp, int *size)
{
  int return_code;
  int isize;
  
  *size = -1; //default error
  if( !fread(&isize,INTSIZE,1,fp)) {
    printf("Cannot read the header length\n");
    return(NULL);
  }
  printf("The header length is %d bytes \n",isize);
  if(isize < FILE_HDR_ADDITIONAL){
    printf("The file header is too short, only %d integers\n",isize);
    return(NULL);
  }
  if(file_hdr != NULL) free((void *)file_hdr);
  file_hdr = (int *)malloc(isize+INTSIZE);
  if(file_hdr == NULL){
    printf("Cannot allocate enough memory to save the file header!\n");
    return(NULL);
  }
  file_hdr[0] = isize;
  if((return_code = fread(&(file_hdr[1]),1,isize,fp)) !=(isize)) {
    printf("Cannot read the full header (%d)\n",return_code);
    return(NULL);
  }
  *size = isize+INTSIZE;
  return(file_hdr);
}

/**
 * Fill the run tables as well as the events
 * @param[in] fp: the file pointer of the binary file
 * @param[out] size: pointer to the size of the event
 * \return NULL:  could not read the event
 * \return otherwise: valid pointer to the event data
 */
unsigned short *grand_read_event(FILE *fp, int *size)
{
  int isize,return_code;
  
  *size = -1;
  if( !fread(&isize,INTSIZE,1,fp)) {
    printf("Cannot read the Event length\n");
    return(NULL);
  }
  if(event != NULL) {
    if(event[0] != isize) {
      free((void *)event);
      event = (unsigned short *)malloc(isize+INTSIZE);
    }
  }
  else{
      event = (unsigned short *)malloc(isize+INTSIZE);
  }
  if(event == NULL){
    printf("Cannot allocate enough memory to save the event!\n");
    return(NULL);
  }
  event[0] = isize&0xffff;
  event[1] = isize>>16;
  if((return_code = fread(&(event[2]),1,isize,fp)) !=(isize)) {
    printf("Cannot read the full event (%d requested %d bytes) Error %d EOF %d\n",return_code,isize,ferror(fp),feof(fp));
    return(NULL);
  }
  *size = isize+INTSIZE;
  return(event);
}
