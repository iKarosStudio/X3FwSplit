#include "X3FwSplit.h" 


#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

 

int Img2File (char *InputFilePath, char *OutputFilePath) 
{
   int FwFile;
  /*
    Load .fw file and create memory image.
   */
  printf ("*Load %s ...", InputFilePath) ;
  FwFile = open (InputFilePath, "RW")  ;
  if (FwFile == -1) {
    printf ("Load fw fail, EXIT\n") ;
    return -1;
  }
  
  return 0;
}
