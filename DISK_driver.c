#include <stdlib.h>
#include <stdio.h>
#include <string.h>
struct PARTITION {
 int total_blocks;
 int block_size;
} Partition;
char* partitionFileName;
struct FAT {
 char *filename;
 int file_length;
 int blockPtrs[10];
 int current_location;
} fat[20];

 char *block_buffer;
FILE *active_file_table[5];
int active_fat[20] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}; // first initialize to -1 to indicate there is nothing in that index
void initIO()
{
  Partition.block_size= 0;
  Partition.total_blocks=0;
  for(int i=0; i<20;i++)
  {
    fat[i].filename = "NULL";
    fat[i].file_length=0;
    for(int j = 0; j<10;j++)
    {
      fat[i].blockPtrs[j] = 0;
    }
    fat[i].current_location=0;
    
  }
  
}


int partition(char *name, int blocksize, int totalblocks)
{
  system("mkdir PARTITION");
  system("cd PARTITION");
  FILE *f = fopen(name, "w+");

  fprintf(f, "%i,", totalblocks);
  fprintf(f, "%i", blocksize);
  fputc(':', f);
  for(int i = 0; i< 20; i++)
  {
    fprintf(f, "%s,", fat[i].filename);

    fprintf(f, "%i,", fat[i].file_length);


    for(int j = 0; j < 10; j++)
    {
      fprintf(f, "%i,", fat[i].blockPtrs[j]);

    }

    fprintf(f, "%i:", fat[i].current_location);
    int numOfZeros = Partition.total_blocks*Partition.block_size;
    char c = '0';
    for(int i = 0; i<numOfZeros;i++)
    {

      putc(c, f);
    }

  }

  

  

  
}


int mountFS(char *name)
{
  FILE *f = fopen (name, "r");
    if (f == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit (1);
    }
  int totalblocks;
  fscanf(f, "%i", &totalblocks);
  int blocksize;
  fgetc(f);
  fscanf(f, "%i", &blocksize);
  fgetc(f);
  for (int i = 0; i<20; i++)
  {
   fscanf(f, "%s", fat[i].filename);
   fgetc(f);
   fscanf(f, "%i", &fat[i].file_length);
   fgetc(f);
   for(int j = 0; j<10; j++)
   {
     fscanf(f, "%i", &fat[i].blockPtrs[j]);
     fgetc(f);
   }
   fscanf(f, "%i", &fat[i].current_location);
  }
  int size = Partition.block_size*Partition.total_blocks;
  block_buffer = (char*)malloc(sizeof(char)*size);
  partitionFileName = name;
  return 0;

}


int openfile(char *name)
{
  int i = 0; // this i is an index to search through fat[]
  while(i<20)
  {
    // find the corresponding fat slot
    if (strcmp(fat[i].filename, name)==0)
    {
      int j = 0;
      while(active_file_table[j] != NULL)
      {
        // find an available active file pointer
        j++;
        if(j == 5)
        {
          return -1;
        }
      }
      FILE *g = fopen(name, "rt");
      while(getc(g)!=':');

      while(getc(g)!=':');
      // make the file pointer points to the first block of the file content part
      active_file_table[j] = g;
      active_fat[i] = j;   // store the corresponding index of the active file pointer to the array that stores information about which file corresponding to which pointer
      return i;
    }
    else
    {
      i++;
    }
  }
  if(i==20)
  {
    // if the file is not in fat already
    i = 0;
    while(i<20)
    {
      if(fat[i].filename==NULL)
      {
        fat[i].filename = name;
       // active_fat[i] = ;
        return i;
      }
      else
      {
        i++;
      }
    }
    return -1;
  }
  return 0;
}

char* readBlock(int file)
{
  if (file == -1)
  {
    return NULL;
  }
  FILE *par = fopen(partitionFileName, "r");
  if(active_fat[file]==-1)
  {
    // the file hasn't been opened yet
    return NULL;
  }
  while(getc(par)!=':');

  while(getc(par)!=':');
  // in order to facilitate non-contiguious disk, we get the location from blockPtrs every time 
  fseek(par, fat[file].blockPtrs[fat[file].current_location]*Partition.block_size, SEEK_SET);
  int i = 0;
  char buffer[1000];

  while(i<Partition.block_size)
  {
    buffer[i] = getc(par);
  }
  block_buffer = buffer;
  fat[file].current_location++;
  return block_buffer;
}

int writeBlock(int file, char *data)
{
  if (file == -1)
  {
    return -1;
  }
  FILE *par = fopen(partitionFileName, "w");
  int currloc = fat[file].blockPtrs[fat[file].current_location];

  while(getc(par)!=':');

  while(getc(par)!=':');
  if(currloc == 0)
  {
    // this case it means the slot in bloctPtr hasn't been set before
    int offset = 0;
    while(offset < Partition.total_blocks)
    {
       fseek(par, Partition.block_size, SEEK_SET);
       // find the first block where its first character is '0'
       if (fgetc(par) == '0')
       {
         fseek(par, -1, SEEK_SET);

       }else
       {
         offset ++;
       }
    }
    fat[file].blockPtrs[fat[file].current_location] = offset;

  }
  int i = 0;
  char buffer[1000];
  strcpy(buffer, data);
  while(i<Partition.block_size)
  {
    fprintf(par, "%c",buffer[i]);
    i++;
  }

  fat[i].current_location++;
  return 0;
}

