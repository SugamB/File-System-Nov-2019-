/*
Operating System
Assignment 4
Group Members:
              Name:   Bishesh Shrestha  ID: 1001556177
              Name:   Sugam Banskota ID: 1001552420
*/

 #include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include <errno.h>
#include <unistd.h>



#define _GNU_SOURCE


#define st_mtime st_mtim.tv_sec

#define NUM_BLOCKS 4226
#define BLOCK_SIZE 8192
int NUM_FILES =128;
int tot_files = 0;
#define MAX_FILE_SIZE 10240000
#define MAX_PATH_SIZE 4096
char Disk[32];
#define MAX_COMMAND_SIZE 255    // The maximum command-line size
#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

FILE *fd; 

uint8_t blocks[NUM_BLOCKS][BLOCK_SIZE];
unsigned char file_data[NUM_BLOCKS][BLOCK_SIZE];



struct Directory_Entry
{
    uint8_t valid;
    char filename [32];
    char filename2 [32];
    uint32_t inode;
};
struct Inode
{
    uint8_t valid;
    uint32_t attributeh;

    // 1 is  not hidden
    // 0 is  hidden

    uint32_t attributer;
    uint32_t size;
    uint32_t iblocks[1250];
};

struct Directory_Entry  * dir;
struct Inode            * inodes;
int                *freeBlockList;
int freeBlockarr[4226];

uint8_t                 * freeInodeList;

// This initialises the directory block[0] // 0 = free 1 = used
    void initializeDirectory()
    {
        int i;
        for(i=0;i<NUM_FILES;i++)
        {
            dir[i].valid = 1;//1 is free
            dir[i].inode = -1;
            memset( dir[i].filename , 0, 32);

        }
    }
// This initialiss the block list , 1 means its free
    void initializeBlockList()
    {
        int i;
        for(i=132;i<NUM_BLOCKS;i++)
        {
           freeBlockList[i] = 1;
        }
    }
    void initializeBlockarr()
    {
        int i;
        for(i=132;i<NUM_BLOCKS;i++)
        {
           freeBlockarr[i] = 1;// 1 means free
        }
    }
//This initialises the inode list, 1 means its free
    void initializeInodeList()
    {
        int i;
        for(i=0;i<NUM_FILES;i++)
        {
           
           freeInodeList[i] = 1;
        }
    }

    void initializeInode()
    {
        int i;
        for(i=0;i<NUM_FILES;i++)
        {
            inodes[i].valid = 1;//1 is free
            inodes[i].size = 0;
            inodes[i].attributeh = 1;

            int j;
            for(j=0;j<1250;j++)
            {
                inodes[i].iblocks[j] = 0;
            }
        }
    }
// calculate free block
    int df()
    {
        int i;
        int free_space = 0;
        for(i=132;i<NUM_BLOCKS;i++)
        {
            if(freeBlockarr[i]==1)
            {
                free_space = free_space + BLOCK_SIZE;
            }
        } 
        return free_space; 
    }
//findinf existing directory with a filename if not finds a free directory index

    int findDirectoryEntry(char *filename)
    {
        // check for existing entry 
        int i;
        int ret = -1;
                       // printf("disadddssr[?]valid %d\n",dir[i].valid);

        
        for(i=0;i<NUM_FILES;i++)
        {
            if(strcmp(filename,dir[i].filename)==0)
            {

                return i;
            }
        }
        
        //find a free space
        for(i=0;i<NUM_FILES;i++)
        {
            if(dir[i].valid == 1) // 1 is free
            {
                //printf("disadddssr[?]valid %d\n",dir[i].valid);
                dir[i].valid = 0; // 0 is used
                return i;

            }
        }
    return ret;

    }

// finds a free inodeput index
    int findFreeInode()
    {
        int i;
        int ret = -1;


         for(i=0;i<NUM_FILES;i++)
        {
            if(inodes[i].valid == 1) // 1 is free
            {
                inodes[i].valid = 0; // 0 is used
                return i;
            }
        }
        return ret;
    }
// FINDS free block
    int findFreeBlock()
    {
        int i;
        int ret = -1;
        int counter=0;

    for(i=132;i<NUM_BLOCKS;i++)
        {
            
            if(freeBlockList[counter] == 1) // 1 is free
            {
                freeBlockList[counter] = 0; // 0 is used
                return i;
            }
            counter++;
        }
        ret = 132;
        return ret;
    }
// put funtion
int put (char * filename)
{
    printf("inside put\n");
    struct stat buf;
    int status;
    
    status = stat(filename,&buf );
    int size = buf.st_size;

    if(status == -1)
    {
        printf("File doesnt exist\n");
        return -1;
    }

    if(size>MAX_FILE_SIZE)
    {
        printf("File size too big\n");
        return -1;

    }
    
    if(size > df())
    {
       // printf("Reading %d bytes from %s\n", (int) buf . st_size, filename );
       // printf("Disk space remaining %d\n",df());
        printf("File size exceeds the remaining space\n");
        return -1;

    }
    
    // put the file in the image;    
    int directoryIndex = findDirectoryEntry(filename);
    strcpy(dir[directoryIndex].filename,filename);
    printf("directory index %d\n",directoryIndex);
    int inodeIndex = directoryIndex;
    //printf("inode index %d\n",inodeIndex);
    //Example code goes here up to line 128
    
    if( status != -1 )
    {
        //Open the input file read-only 

        FILE *ifp = fopen ( filename, "r" ); 
        if (ifp == NULL) {
        perror("Failed: ");
        return 1;
        }
        printf("Reading %d bytes from %s\n", (int) buf . st_size, filename );


    // Save off the size of the input file since we'll use it in a couple of places and 

    // also initialize our index variables to zero. 

    int copy_size   = buf . st_size;



    // We want to copy and write in chunks of BLOCK_SIZE. So to do this 

    // we are going to use fseek to move along our file stream in chunks of BLOCK_SIZE.

    // We will copy bytes, increment our file pointer by BLOCK_SIZE and repeat.

    int offset      = 0;               



    // We are going to copy and store our file in BLOCK_SIZE chunks instead of one big 

    // memory pool. Why? We are simulating the way the file system stores file data in

    // blocks of space on the disk. block_index will keep us pointing to the area of

    // the area that we will read from or write to.

    int block_index = 0;

 

    // copy_size is initialized to the size of the input file so each loop iteration we

    // will copy BLOCK_SIZE bytes from the file then reduce our copy_size counter by

    // BLOCK_SIZE number of bytes. When copy_size is less than or equal to zero we know

    // we have copied all the data from the input file.
    

      while( copy_size > 0 )

        {



         // Index into the input file by offset number of bytes.  Initially offset is set to

         // zero so we copy BLOCK_SIZE number of bytes from the front of the file.  We 

         // then increase the offset by BLOCK_SIZE and continue the process.  This will

          // make us copy from offsets 0, BLOCK_SIZE, 2*BLOCK_SIZE, 3*BLOCK_SIZE, etc.

            int a =           fseek( ifp, offset, SEEK_SET );
          //  printf("fseel %d\n",a );
         // Read BLOCK_SIZE number of bytes from the input file and store them in our

      // data array. 
    //int block_index = findFreeBlock();
    int i;
    /*
      for(i=132;i<=4226;i++)
        {
            
            if(freeBlockList[counter] == 1) // 1 is free
            {
                freeBlockList[counter] = 0; // 0 is used
                block_index = i;
                
            }
            counter++;
        }
    */
 
   int counternode = 0;
   for(i=132;i<4226;i++){
        if(freeBlockarr[i] == 1){
        block_index = i;
        inodes[inodeIndex].iblocks[counternode] = i;
        counternode++;
        freeBlockarr[i]= 0; // 0 means used 
        break;           
       }
        
   }
    printf("block index :%d\n",block_index);
    int bytes  = fread(&blocks[block_index], BLOCK_SIZE, 1, ifp );
      //printf("%d bytes copied in file_data\n",bytes);


      // If bytes == 0 and we haven't reached the end of the file then something is 

      // wrong. If 0 is returned and we also have the EOF flag set then that is OK.

      // It means we've reached the end of our input file.

      if( bytes == 0 && !feof( ifp ) )

      {

        printf("An error occured reading from the input file.\n");
        return -1;

      }



      // Clear the EOF file flag.

      clearerr( ifp );



      // Reduce copy_size by the BLOCK_SIZE bytes.

      copy_size -= BLOCK_SIZE;

      

      // Increase the offset into our input file by BLOCK_SIZE.  This will allow

      // the fseek at the top of the loop to position us to the correct spot.

      offset    += BLOCK_SIZE;



      // Increment the index into the block array 

      block_index ++;

    }



    // We are done copying from the input file so close it out.

    fclose( ifp );
  
}
}

    void createfs( char * filname)
    {
        fd = fopen(filname,"w");

        memset(&blocks[10],0,NUM_BLOCKS * BLOCK_SIZE);
         
        fwrite(&blocks[0], NUM_BLOCKS, BLOCK_SIZE, fd);

        fclose(fd); 
    }

  int  calfiles(){
  	int i;
  	int filecounter = 0;
  	for(i=0;i<125;i++){
  		if(dir[i].valid == 0){
  			filecounter++;
  		}
  	}
  	return filecounter;
}

    void list()
    {
    	 

        int i;
        int sizevalue = calfiles();
        for(i=0;i<=sizevalue;i++)
        {
            //if valid and not hidden
            if(dir[i].valid == 0)
            {
              if(inodes[i].attributeh < 50){
                char arr[100] = "";
                struct stat buf;   
                stat(dir[i].filename, &buf);
                char borntime[100];
                strcpy(borntime, ctime(&buf.st_atime));
                int len = strlen(borntime);
				if( borntime[len-1] == '\n' ){
    			borntime[len-1] = 0;
    			strcat(arr, borntime);
                strcat(arr, " ");
                strcat(arr,  dir[i].filename);
                printf("%d ", buf.st_size);
                printf("%s \n",arr);

            }
                }
            }
            else{
            	printf("list: No files found.\n");
            }

        }

        return;
    }

void get(char * filename)
    {
    
    //
    
    int    status;                   // Hold the status of all return values.

  struct stat buf;                 // stat struct to hold the returns from the stat call



  // Call stat with out input filename to verify that the file exists.  It will also 

  // allow us to get the file size. We also get interesting file system info about the

  // file such as inode number, block size, and number of blocks.  For now, we don't 

  // care about anything but the filesize.

  status =  stat( Disk, &buf ); 

 
    // Now, open the output file that we are going to write the data to.
int k;

for(k=0;k<128;k++){
    if(strcmp(dir[k].filename,filename)==0){
        break;
    }
}
FILE *ofp = fopen(dir[k].filename,"w");
    


    if( ofp == NULL )

    {

      printf("Could not open output file: %s\n", dir[k].filename );

      perror("Opening output file returned");

      return -1;

    }



    // Initialize our offsets and pointers just we did above when reading from the file.

    int block_index = 0;

    int copy_size   = buf . st_size;

    int offset      = 0;



    printf("Writing %d bytes to %s\n", (int) buf . st_size,dir[k].filename );



    // Using copy_size as a count to determine when we've copied enough bytes to the output file.

    // Each time through the loop, except the last time, we will copy BLOCK_SIZE number of bytes from

    // our stored data to the file fp, then we will increment the offset into the file we are writing to.

    // On the last iteration of the loop, instead of copying BLOCK_SIZE number of bytes we just copy

    // how ever much is remaining ( copy_size % BLOCK_SIZE ).  If we just copied BLOCK_SIZE on the

    // last iteration we'd end up with gibberish at the end of our file. 
    int j=0;
    while( copy_size > 0 )

    { 



      int num_bytes;



      // If the remaining number of bytes we need to copy is less than BLOCK_SIZE then

      // only copy the amount that remains. If we copied BLOCK_SIZE number of bytes we'd

      // end up with garbage at the end of the file.

      if( copy_size < 8192 )

      {

        num_bytes = copy_size;

      }

      else 

      {

        num_bytes = BLOCK_SIZE;

      }



      // Write num_bytes number of bytes from our data array into our output file.
    int i;
    for(i=0;i<128;i++){
        if(strcmp(dir[i].filename,filename) == 0){
            if(inodes[i].iblocks[j]>2)
            {
              block_index = inodes[i].iblocks[j];
              j++;
                break;
            }
        

       }
    }

      fwrite( blocks[block_index], 8192, 1, ofp ); 



      // Reduce the amount of bytes remaining to copy, increase the offset into the file

      // and increment the block_index to move us to the next data block.

      copy_size -= BLOCK_SIZE;

      offset    += BLOCK_SIZE;

      block_index ++;



      // Since we've copied from the point pointed to by our current file pointer, increment

      // offset number of bytes so we will be ready to copy to the next area of our output file.

      fseek( ofp, offset, SEEK_SET );

    }



    // Close the output file, we're done. 

    fclose( ofp );

    }


void get2(char * filename, char *filename2)
    {
    
    int    status;                   // Hold the status of all return values.

  struct stat buf;                 // stat struct to hold the returns from the stat call


  status =  stat( Disk, &buf ); 

 
    // Now, open the output file that we are going to write the data to.
int k;

for(k=0;k<128;k++){
    if(strcmp(dir[k].filename,filename)==0){
        break;
    }
}
FILE *ofp = fopen(filename2,"w");
    


    if( ofp == NULL )

    {

      printf("Could not open output file: %s\n", dir[k].filename );

      perror("Opening output file returned");

      return -1;

    }



    // Initialize our offsets and pointers just we did above when reading from the file.

    int block_index = 0;

    int copy_size   = buf . st_size;

    int offset      = 0;



    printf("Writing %d bytes to %s\n", (int) buf . st_size,dir[k].filename );



    // Using copy_size as a count to determine when we've copied enough bytes to the output file.

    // Each time through the loop, except the last time, we will copy BLOCK_SIZE number of bytes from

    // our stored data to the file fp, then we will increment the offset into the file we are writing to.

    // On the last iteration of the loop, instead of copying BLOCK_SIZE number of bytes we just copy

    // how ever much is remaining ( copy_size % BLOCK_SIZE ).  If we just copied BLOCK_SIZE on the

    // last iteration we'd end up with gibberish at the end of our file. 
    int j=0;
    while( copy_size > 0 )

    { 



      int num_bytes;



      // If the remaining number of bytes we need to copy is less than BLOCK_SIZE then

      // only copy the amount that remains. If we copied BLOCK_SIZE number of bytes we'd

      // end up with garbage at the end of the file.

      if( copy_size < 8192 )

      {

        num_bytes = copy_size;

      }

      else 

      {

        num_bytes = BLOCK_SIZE;

      }



      // Write num_bytes number of bytes from our data array into our output file.
    int i;
    for(i=0;i<128;i++){
        if(strcmp(dir[i].filename,filename) == 0){
            if(inodes[i].iblocks[j]>2)
            {
              block_index = inodes[i].iblocks[j];
              j++;
                break;
            }
        

       }
    }

      fwrite( blocks[block_index], 8192, 1, ofp ); 



      // Reduce the amount of bytes remaining to copy, increase the offset into the file

      // and increment the block_index to move us to the next data block.

      copy_size -= BLOCK_SIZE;

      offset    += BLOCK_SIZE;

      block_index ++;



      // Since we've copied from the point pointed to by our current file pointer, increment

      // offset number of bytes so we will be ready to copy to the next area of our output file.

      fseek( ofp, offset, SEEK_SET );

    }



    // Close the output file, we're done. 

    fclose( ofp );

    }



void open(char *diskname){

        int    status;                   // Hold the status of all return values.

  struct stat buf;                 // stat struct to hold the returns from the stat call



  // Call stat with out input filename to verify that the file exists.  It will also 

  // allow us to get the file size. We also get interesting file system info about the

  // file such as inode number, block size, and number of blocks.  For now, we don't 

  // care about anything but the filesize.

  status =  stat( diskname, &buf ); 



  // If stat did not return -1 then we know the input file exists and we can use it.

  if( status != -1 )

  {

 

    // Open the input file read-only 

    FILE *ifp = fopen ( diskname, "r" ); 

    printf("Reading %d bytes from disk image\n", (int) buf . st_size  );

 

    // Save off the size of the input file since we'll use it in a couple of places and 

    // also initialize our index variables to zero. 

    int copy_size   = buf . st_size;



    // We want to copy and write in chunks of BLOCK_SIZE. So to do this 

    // we are going to use fseek to move along our file stream in chunks of BLOCK_SIZE.

    // We will copy bytes, increment our file pointer by BLOCK_SIZE and repeat.

    int offset      = 0;               



    // We are going to copy and store our file in BLOCK_SIZE chunks instead of one big 

    // memory pool. Why? We are simulating the way the file system stores file data in

    // blocks of space on the disk. block_index will keep us pointing to the area of

    // the area that we will read from or write to.

    int block_index = 0;

 

    // copy_size is initialized to the size of the input file so each loop iteration we

    // will copy BLOCK_SIZE bytes from the file then reduce our copy_size counter by

    // BLOCK_SIZE number of bytes. When copy_size is less than or equal to zero we know

    // we have copied all the data from the input file.

    while( copy_size > 0 )

    {



      // Index into the input file by offset number of bytes.  Initially offset is set to

      // zero so we copy BLOCK_SIZE number of bytes from the front of the file.  We 

      // then increase the offset by BLOCK_SIZE and continue the process.  This will

      // make us copy from offsets 0, BLOCK_SIZE, 2*BLOCK_SIZE, 3*BLOCK_SIZE, etc.

      fseek( ifp, offset, SEEK_SET );

 

      // Read BLOCK_SIZE number of bytes from the input file and store them in our

      // data array. 

      int bytes  = fread( blocks[block_index], 36257792, 1, ifp );



      // If bytes == 0 and we haven't reached the end of the file then something is 

      // wrong. If 0 is returned and we also have the EOF flag set then that is OK.

      // It means we've reached the end of our input file.

      if( bytes == 0 && !feof( ifp ) )

      {

        printf("An error occured reading from the input file.\n");

        return ;

      }



      // Clear the EOF file flag.

      clearerr( ifp );



      // Reduce copy_size by the BLOCK_SIZE bytes.

      copy_size -= BLOCK_SIZE;

      

      // Increase the offset into our input file by BLOCK_SIZE.  This will allow

      // the fseek at the top of the loop to position us to the correct spot.

      offset    += BLOCK_SIZE;



      // Increment the index into the block array 

      block_index ++;

    }



    // We are done copying from the input file so close it out.

    fclose( ifp );
  }




}
void Close()
{
    FILE *ofp;

    ofp = fopen(Disk, "w");



    if( ofp == NULL )

    {

     // printf("Could not open output file: %s\n", argv[2] );

      perror("Opening output file returned");

      return -1;

    }



    // Initialize our offsets and pointers just we did above when reading from the file.

    int block_index = 0;

    int copy_size   = 34619392;

    int offset      = 0;



    printf("Writing 34619392 bytes to 34619392\n");



    // Using copy_size as a count to determine when we've copied enough bytes to the output file.

    // Each time through the loop, except the last time, we will copy BLOCK_SIZE number of bytes from

    // our stored data to the file fp, then we will increment the offset into the file we are writing to.

    // On the last iteration of the loop, instead of copying BLOCK_SIZE number of bytes we just copy

    // how ever much is remaining ( copy_size % BLOCK_SIZE ).  If we just copied BLOCK_SIZE on the

    // last iteration we'd end up with gibberish at the end of our file. 

    while( copy_size > 0 )

    { 



      int num_bytes;



      // If the remaining number of bytes we need to copy is less than BLOCK_SIZE then

      // only copy the amount that remains. If we copied BLOCK_SIZE number of bytes we'd

      // end up with garbage at the end of the file.

      if( copy_size < BLOCK_SIZE )

      {

        num_bytes = copy_size;

      }

      else 

      {

        num_bytes = BLOCK_SIZE;

      }



      // Write num_bytes number of bytes from our data array into our output file.

      fwrite( blocks[block_index], num_bytes, 1, ofp ); 



      // Reduce the amount of bytes remaining to copy, increase the offset into the file

      // and increment the block_index to move us to the next data block.

      copy_size -= BLOCK_SIZE;

      offset    += BLOCK_SIZE;

      block_index ++;



      // Since we've copied from the point pointed to by our current file pointer, increment

      // offset number of bytes so we will be ready to copy to the next area of our output file.

      fseek( ofp, offset, SEEK_SET );

    }



    // Close the output file, we're done. 
    tot_files++;
    fclose( ofp );
}



int main()
{


    dir = (struct Directory_Entry *)&blocks[0]; 
    inodes = (struct Inode *)&blocks[9]; 
    freeInodeList = (uint8_t *)&blocks[7]; 
    freeBlockList = (int *)&blocks[8]; 
    

    initializeDirectory();
    int j;
   // for(j=0;j<NUM_FILES;j++){
    //printf("dir[] valid? %d\n", dir[j].valid );
    //}
    initializeBlockarr();
    initializeBlockList();
    initializeInodeList();
    int i;    
    
    //dir[0].valid = 1;
    //dir[1].valid = 1;
    //dir[127].valid = 1;
    
    //printf("%d %d %d %d\n",dir[1].inode,dir[2].inode,dir[22].inode,dir[127].inode);
        
    //freeInodeList[12]=0;
    //printf("%d %d\n", freeInodeList[0],freeInodeList[12] );

    //put("mfs.c");
    //put("doesnt_exits");
   
  
    //createfs("disk.image");
    //printf("Disk space remaining %d\n",df());
     
    printf("Disk space remaining %d\n",df());
    
      char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
      if(strcmp(cmd_str,"\n") ==0)
	    	continue;

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality
    int i;
    int j;
    // int token_index  = 0;
    // for( token_index = 0; token_index < token_count; token_index ++ ) 
    // {
    //   printf("token[%d] = %s\n", token_index, token[token_index] );  
    // }

   if(strcmp(token[0],"put")==0){
    put(token[1]);
   }
   if(strcmp(token[0],"get")==0){
   	if(token[1]==NULL){
   		continue;
   	}
   	else if(token[2] !=NULL){
	   	get2(token[1],token[2]);
   	}
   	else {
   		get(token[1]);
   	}
   }

   if(strcmp(token[0],"createfs")==0){
   	 if(token[1] ==NULL){
   	 	printf("createfs: File not found \n");
   	 }
   	 else{
   	 	createfs(token[1]);
   	 	strcpy(Disk,token[1]);
   	 	printf("hello %s \n",Disk);
   	 } 
   	}
    //quit and exit command exits the program
	if(strcmp(token[0],"quit")==0 || strcmp(token[0],"exit")==0) exit(EXIT_SUCCESS);

   if(strcmp(token[0],"list")==0){
    list(); 
   }

   if(strcmp(token[0],"open")==0){
   	if(token[1] == NULL){
   		continue;
   	}
   	else if((strcmp(token[1],Disk)>0) || (strcmp(token[1],Disk)<0)){
   		printf("open: File not found \n");
   	}
   	else{
   		open(token[1]);
   		strcpy(Disk,token[1]);
   	}
   }
   if(strcmp(token[0],"close")==0){
    Close();
   }
    if(strcmp(token[0],"df")==0){
      printf("Disk space remaining %d\n",df());
    }

    if(strcmp(token[0],"attrib")==0){
    
      if(strcmp(token[1],"+h")==0)
      {
         for(i=0;i<NUM_FILES;i++)
         {
           if(strcmp(token[2],dir[i].filename)==0)
           {
            inodes[i].attributeh = 100;
            //printf("%s attri %d\n",dir[i].filename,inodes[i].attributeh);
           }
         }
      }
      else if(strcmp(token[1],"-h")==0)
      {
        for(i=0;i<NUM_FILES;i++)
         {
           if(strcmp(token[2],dir[i].filename)==0)
           {
            inodes[i].attributeh = 0;
            //printf("%s attri %d\n",dir[i].filename,inodes[i].attributeh);
           }
         }
      }
      else if(strcmp(token[1], "+r")==0){
      	for(i=0;i<NUM_FILES;i++)
         {
           if(strcmp(token[2],dir[i].filename)==0)
           {
            inodes[i].attributer = 200;
            printf("%s attri %d\n",dir[i].filename,inodes[i].attributer);
           }
         }
      } 
      else if(strcmp(token[1], "-r")==0){

           for(i=0;i<NUM_FILES;i++)
         {
           if(strcmp(token[2],dir[i].filename)==0)
           {
            inodes[i].attributer = 0;
            printf("%s attri %d\n",dir[i].filename,inodes[i].attributer);
           }
         }
   }
}

   if(strcmp(token[0],"del")==0){
     //search token[1] file name
   	if(token[1] == NULL){
   		continue;
   	}
     else {
    for(i=0;i<NUM_FILES;i++)
       {
         if(strcmp(dir[i].filename,token[1])==0)
        {
        	if(inodes[i].attributer == 200){
        		printf("del: That file is marked read-only.\n");
        	}
        else{

          printf("%s\n",dir[i].filename);
           dir[i].valid = 1; // 1 is valid
           for(j=0;j<NUM_FILES;j++){
           if(inodes[i].iblocks[j] >2 ){
                freeBlockarr[inodes[i].iblocks[j]] = 1; // 1 is free
           }
           }
           for(j=0;j<NUM_FILES;j++){
           if(inodes[i].iblocks[j] >2 ){
               inodes[i].iblocks[j] = -1;
               // freeBlockarr[i] = 1; // 1 is free
           }
           }
           
        }
          }
       }
   }
}
  
    free( working_root );
	}
  return 0;
}

