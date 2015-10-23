#include "X3FwSplit.h" 

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <zlib.h>

void *FwFileRam = NULL;
unsigned int FileAmount = 0;
unsigned int Checksum = 0;
unsigned int Crc32 = 0;


int Img2File (char *InputFilePath, char *OutputFilePath) 
{
  int Res = 0;
  int Index = 0;
  int FwFileSize = 0;
  FILE *FwFileImg;
  /*
    Load .fw file and create memory image.
  */
  printf ("Open %s ... ", InputFilePath) ;
  FwFileImg = fopen (InputFilePath, "rb") ;

  if (FwFileImg) {
    printf ("success!\n") ;

    /*
      Get fw file size
    */
    fseek (FwFileImg, 0, SEEK_END) ;
    FwFileSize = ftell (FwFileImg) ;
    fseek (FwFileImg, 0, SEEK_SET) ;
    printf ("File Size : %d Kb\n", FwFileSize / 1024) ;
    
    /*
      Request ram space
    */
    FwFileRam = malloc (FwFileSize) ;
    if (FwFileRam) {
      //printf ("*Allocate %d bytes @ %08p\n", FwFileSize, FwFileRam) ;
      /*
	Copy image to ram
       */
      Res = fread (FwFileRam, FwFileSize, 1, FwFileImg) ;
      Res = mkdir (OUTPUT_ROOT_FOLDER, FOLDER_PERMISSION) ;
      if (Res == -1) {
	perror ("Make folder fail, reason") ;
      }

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

    /*
      Find check sum
    */
    FindChecksum (InputFilePath) ;
    printf ("Try Checksum:%08X\n", Checksum) ;
    printf ("Try CRC32   :%08X\n", Crc32) ;
    
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
    //printf ("*Allocate %d bytes @ 0x%08x\n", FILE_HEADER_SIZE, FwFileHeader) ;

    /*
      Copy .fw file header information form memory to buffer.
     */
    memcpy (FwFileHeader, FwFileRam, FILE_HEADER_SIZE) ;
    //printf ("*Copied %d bytes to buffer\n", FILE_HEADER_SIZE) ;

    /*
      Start dump fw header information
    */
    printf ("-----------------------------------\n") ;
    printf ("--- .fw FILE HEADER INFORMATION ---\n") ;
    printf ("-----------------------------------\n") ;
    printf ("Start signature : %s\n", FwFileHeader->StartSignature) ;
    printf ("Sector amount   : 0x%08X\n", FwFileHeader->SectorAmount) ;
    printf ("Unknown1        : 0x%08X\n", FwFileHeader->Unknown1) ;
    printf ("Unknown2        : 0x%08X\n", FwFileHeader->Unknown2) ;
    printf ("Time stamp      : %s\n", FwFileHeader->TimeStamp) ;
    printf ("File amount     : %d\n", FwFileHeader->FileAmount) ;
    //printf ("Fw Version      : %d.%02d\n", FwFileHeader->FwVerion[0], FwFileHeader->FwVerion[1]) ;
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
  unsigned int TailCode = 0;
  int Res = 0;

  /*
    Move pointer to first file of file table.
  */
  FwSectorAddress = (FwFileRam + (FILE_HEADER_SIZE * 4) ) ;

  
  FwSectorHeader = malloc (sizeof (SectorHeader) ) ;
  for (SectorIndex = 1; SectorIndex <= FileAmount; SectorIndex++) {
    memcpy (FwSectorHeader, FwSectorAddress, SECTOR_HEADER_SIZE) ;

    /*
      Dump sector information to user
    */
    //printf ("[%04d]", SectorIndex) ;
    //printf ("---- File %04d ----\n", SectorIndex) ;
    //printf (" : %s \n", FwSectorHeader->FilePath) ;
    //printf ("Sector Index : 0x%08X \n", FwSectorHeader->SectorIndex) ;
    //printf ("File Size    : %d bytes\n", FwSectorHeader->FileSize) ;
    //printf ("-------------------\n") ;


    /*
      Dump code at file tail
    */
    if (SectorIndex == FileAmount) { /* Final file */
      memcpy (&TailCode,
	      (FwFileRam + (FwSectorHeader->SectorIndex * 512) + FwSectorHeader->FileSize) ,
	      4) ;
      //printf ("Final : 0x%08X\n", TailCode) ;
    }

    /*
      Create file from sector header.
    */
    Res = CreateFile (FwSectorHeader) ;

    /*
      Move pointer to next table offset
    */
    FwSectorAddress += SECTOR_HEADER_SIZE;

    /*
      Clear Header Buffer
    */
    memset (FwSectorHeader, 0, SECTOR_HEADER_SIZE) ;
  }

  free (FwSectorHeader) ;
  return 0;
}

int CreateFile (SectorHeader *SourceFile) {
  char *OutputFilePath = NULL;
  FILE *OutputFile = NULL;

  OutputFilePath = PharseFilePath (SourceFile->FilePath) ;
  OutputFile = fopen (OutputFilePath, "wb") ;
  
  fwrite ((FwFileRam + (SourceFile->SectorIndex << 9) ), /* << 9 -> *512 sector size */
	  1,
	  SourceFile->FileSize,
	  OutputFile) ;

  printf ("[0x%08X] - \"%s\" - [%d] bytes\n",
	SourceFile->SectorIndex * SECTOR_SIZE,
	OutputFilePath,
	SourceFile->FileSize) ;
	  
  fclose (OutputFile) ;
  free (OutputFilePath) ;
  return 0;
}

char *PharseFilePath (char *TargetPath) 
{
  char PathStringBuffer[128] = {'\0'} ;
  char *PathStringSlice = NULL;
  char *OutputFilePath = NULL;
  int Res = 0;

  /*
    Set root ouput folder
  */
  strcat (PathStringBuffer, OUTPUT_ROOT_FOLDER) ;
  #ifdef LINUX
  strcat (PathStringBuffer, "/") ;
  #endif

  PathStringSlice = strtok (TargetPath, "\\") ; /* Pharse '\' */
  while (PathStringSlice != NULL) {
    /*
      Append to Buffer
    */
    strcat (PathStringBuffer, PathStringSlice) ;


    /*
      Next Slice
    */
    PathStringSlice = strtok (NULL, "\\") ;
    if (PathStringSlice != NULL) {
      /*
	Create folder for path
      */
      #ifdef LINUX
      Res = mkdir (PathStringBuffer, FOLDER_PERMISSION) ;
      if (Res == -1) {
	//perror (PathStringBuffer) ;
      }
      #endif

      /*
	Append directory sepherator
      */
      #ifdef LINUX
      strcat (PathStringBuffer, "/") ;
      #endif
      
    } else {
      /*
	Create path file
      */
    }
  }
  
  //OutputFilePath = malloc (strlen (PathStringBuffer) ) ;
  OutputFilePath = malloc (128) ;
  memset (OutputFilePath, '\0', 128) ;
  free (PathStringSlice) ;
  strcpy (OutputFilePath, PathStringBuffer) ;
  return OutputFilePath ;
}

/*
 Just for test
 */
int FindChecksum (char *FileImg) 
{
  FILE *fi = NULL;
  unsigned int Size = 0;
  unsigned int Buffer = 0;
  //unsigned int *Buffer = NULL;
  int Res = 0;

  fi = fopen (FileImg, "rb") ;

  fseek (fi, 0, SEEK_SET) ;
  fseek (fi, 0, SEEK_END) ;
  Size = ftell (fi) ;
  

  fseek (fi, 4, SEEK_SET) ;
  /*
  while (!feof (fi) ) {
    fread (&Buffer, 4, 1, fi) ;
    Checksum += Buffer;
    Crc32 = crc32 (Crc32, (const Bytef *) &Buffer, 1) ;
    if (Crc32 == 0x626A29EC || Crc32 == 0xec296a62 ||
	Checksum == 0x626A29EC || Checksum == 0xEC296A62) {
      printf ("found\n") ;
    }
  }
  */
  
  fclose (fi) ;
  return 0;
}
