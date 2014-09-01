/*
  
function prototype and sturcture define

 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE 512 /* 1 Sector = n bytes */

#define FILE_HEADER_SIZE 512
#define SECTOR_HEADER_SIZE 64

typedef struct {
  char StartSignature[4];
  unsigned int SectorAmount;
  unsigned int Unknown1; /* should be 0x00000004 */
  unsigned int Unknown2; /* should be 0xFFFFF000 */
  char TimeStamp[12];
  unsigned int FileAmount;
  char FwVerion[0x10];
  char FwModule[0x10];
  char Zeros[444];
  unsigned int EndSignature;
} FileHeader ;

typedef struct {
  char FilePath[56];
  unsigned int SectorIndex;
  unsigned int FileSize;
} SectorHeader ;

typedef struct {
  char FilePath[56];
  unsigned int SectorIndex;
  unsigned int FileSize;
  void *NextFile;
} SplitFileList;


int Img2File (char *InputFilePath, char *OutputFilePath) ;

int DumpFileHeader (FILE *FwFileRam) ;
int DumpSecterHeader (void *FwFileRam, unsigned int FileAmount) ;

int SplitFilePath (char *FullPath, char Spliter) ;



