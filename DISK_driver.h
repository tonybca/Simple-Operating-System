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
int active_fat[5];

int partition(char *name, int blocksize, int totalblocks);
int mountFS(char *name);
int openfile(char *name);
int writeBlock(int file, char *data);
char* readBlock(int file);