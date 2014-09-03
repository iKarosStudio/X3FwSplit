#include "X3FwSplit.h" 

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void *FwFileRam = NULL;
unsigned int FileAmount = 0;

int Img2File (char *InputFilePath, char *OutputFilePath) 
{
  int Res = 0;
  int Index = 0;
  int FwFileSize = 0;
  FILE *FwFileImg;
  /*
    Load .fw file and create memory image.
  */
  printf ("*Open %s ... ", InputFilePath) ;
  FwFileImg = fopen (InputFilePath, "rb") ;

  if (FwFileImg) {
    printf ("success!\n") ;

    /*
      Get fw file size
    */
    fseek (FwFileImg, 0, SEEK_END) ;
    FwFileSize = ftell (FwFileImg) ;
    fseek (FwFileImg, 0, SEEK_SET) ;
    printf ("*File Size : %d bytes\n", FwFileSize) ;
    
    /*
      Request ram space
    */
    FwFileRam = malloc (FwFileSize) ;
    if (FwFileRam) {
      printf ("*Create ram buffer @ 0x%08x\n", FwFileRam) ;
      /*
	Copy image to ram
       */
      Res = fread (FwFileRam, FwFileSize, 1, FwFileImg) ;
      printf ("*Copied %d bytes to ram\n", Res) ;

      /*
	Dump fw file header information to user
      */
      Res = DumpFileHeader (FwFileRam) ;

      /*
	Dump each sector header information to user
      */
      Res = DumpSecterHeader (FwFileRam, FileAmount) ;
      free (FwFileRam) ;
    }

    fclose (FwFileImg) ;
  } else {
    printf ("fail! (Path of Filename error?)\n") ;
  }
  return 0;
}


int DumpFileHeader (FILE *FwFileRam) 
{
  int Res = 0;
  FileHeader *FwFileHeader = NULL ;

  /*
    Request header buffer ram space
  */
  FwFileHeader = malloc (FILE_HEADER_SIZE ) ;
  if (FwFileHeader) {
    printf ("*Create header buffer @ 0x%08x\n", FwFileHeader) ;

    /*
      Copy .fw file header information form memory to buffer.
     */
    memcpy (FwFileHeader, FwFileRam, FILE_HEADER_SIZE) ;
    printf ("*Copied %d bytes to buffer\n", FILE_HEADER_SIZE) ;

    /*
      Start dump fw header information
    */
    printf ("-----------------------------------\n") ;
    printf ("--- .fw FILE HEADER INFORMATION ---\n") ;
    printf ("-----------------------------------\n") ;
    printf ("Start signature : %s\n", FwFileHeader->StartSignature) ;
    printf ("Sector amount   : %d\n", FwFileHeader->SectorAmount) ;
    printf ("Unknown1        : 0x%08X\n", FwFileHeader->Unknown1) ;
    printf ("Unknown2        : 0x%08X\n", FwFileHeader->Unknown2) ;
    printf ("Time stamp      : %s\n", FwFileHeader->TimeStamp) ;
    printf ("File amount     : %d\n", FwFileHeader->FileAmount) ;
    printf ("Fw Version      : %d.%02d\n", FwFileHeader->FwVerion[0], FwFileHeader->FwVerion[1]) ;
    printf ("Fw Module       : %s\n", FwFileHeader->FwModule) ;
    printf ("End signature   : 0x%08X\n", FwFileHeader->EndSignature) ;

    /*
      Report file amount
    */
    FileAmount = FwFileHeader->FileAmount;

    free (FwFileHeader) ;
  } else {
    printf ("*Create header buffer fail\n") ;
    return -1;
  }
  return 0;
}

int DumpSecterHeader (void *FwFileRam, unsigned int FileAmount) 
{
  void *FwSectorAddress = NULL;
  SectorHeader *FwSectorHeader = NULL;
  unsigned int SectorIndex = 0;
  int Res = 0;

  /*
    Move pointer to first file of file table.
  */
  FwSectorAddress = (FwFileRam + (FILE_HEADER_SIZE * 4) ) ;

  
  FwSectorHeader = malloc (SECTOR_HEADER_SIZE) ;
  //for (SectorIndex = 0; SectorIndex < FileAmount; SectorIndex++) {
  for (SectorIndex = 0; SectorIndex < 1; SectorIndex++) {
    memcpy (FwSectorHeader, FwSectorAddress, SECTOR_HEADER_SIZE) ;

    /*
      Dump sector information to user
    */
    printf ("---- File %04d ----\n", SectorIndex) ;
    printf ("Full Path    : %s \n", FwSectorHeader->FilePath) ;
    printf ("Sector Index : 0x%08X \n", FwSectorHeader->SectorIndex) ;
    printf ("File Size    : %d bytes\n", FwSectorHeader->FileSize) ;
    printf ("-------------------\n") ;

    /*
      Move pointer to next table offset
    */
    FwSectorAddress += SECTOR_HEADER_SIZE;

    /*
      Split file path
    */
    Res = SplitFilePath (FwSectorHeader->FilePath, '/') ;
  }

  free (FwSectorHeader) ;
  return 0;
}

int SplitFilePath (char *FullPath, char Spliter) 
{
  char *FilePath;
  
  printf ("File path spliter : \n") ;

  /*
    Split full file path with strtok
  */
  
  return 0;
}
