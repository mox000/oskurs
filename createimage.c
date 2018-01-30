#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_FILE "image"
#define ARGS "[--extended] [--vm] <bootblock> <executable-file> ..."

#define SECTOR_SIZE 512 //define the size of the sector


/* Variable to store pointer to program name */
char *progname; 

/* Variable to store pointer to the filename for the file being read. */
char *elfname;

/* Structure to store command line options */
static struct {
    int vm;
    int extended;
} options;

/* prototypes of local functions */
static void create_image(int nfiles, char *files[]);
static void error(char *fmt,...);

int main(int argc, char **argv)
{
    /* Process command line options */
    progname = argv[0];
    options.vm = 0;
    options.extended = 0;
    while ((argc > 1) && (argv[1][0] == '-') && (argv[1][1] == '-')) 
    {
        char *option = &argv[1][2];

        if (strcmp (option, "vm") == 0) {
            options.vm = 1;
        } else if (strcmp (option, "extended") == 0) {
            options.extended = 1;
        } else {
            error ("%s: invalid option\nusage: %s %s\n", progname, progname, ARGS);
        }
        argc--;
        argv++;
    }
    if (options.vm == 1) {
      /* This option is not needed in project 1 so we doesn't bother
       * implementing it*/
      error ("%s: option --vm not implemented\n", progname);
    }
    if (argc < 3) {
        /* at least 3 args (createimage bootblock kernel) */
        error ("usage: %s %s\n", progname, ARGS);
    }
    create_image (argc - 1, argv + 1);
    return 0;
}

static void create_image(int nfiles, char *files[])
{
    /* This is where you should start working on your own implemtation
     * of createimage.  Don't forget to structure the code into
     * multiple functions in a way whichs seems logical, otherwise the
     * solution will not be accepted. */
    //P_FILESZ is the size of the file
    //P_OFFSETis what we will read, the amount of bits to skip in the beginning
    //E_PHNUM holds the number of entries in the program header
    //E_PHOFF holds the program header table file offset in bytes, if there is no header, it holds zero
    //ELF is where the programs are placed. when entering an elfheader, it contains programheaders that know where the programs are placed
    //elf 32 is a struct and contains the "elfs" we are using later
   
    Elf32_Ehdr header; 
    FILE * open_file;
    int i, j;
    int read;               //the read file
    int leftover_bytes; 
    int totalbyte;
    int amount_of_sector;   //the amount of sectors



    //assembly file has 61 bytes
    //c-file has 4309 bytes 


    //opens the imagefile, w+ makes it able to open, read and overwrite and create a new file if it is needed
    FILE *imagefile = fopen(IMAGE_FILE, "w+");

    //declares that it will have the possibility to open the files and it will start at the first and work its way through. condition there has to be files.
    for(i = 0; i < nfiles; i++)
    {
        //opens the files on position "i", in this case we are opening non text files, therefore using rb
        open_file = fopen(files[i], "rb");
        
        //reads all the elf headers, their size containings
        read = fread(&header, sizeof(Elf32_Ehdr), 1, open_file);

        //allocates memory to the program header
        Elf32_Phdr *p_header = malloc(sizeof(Elf32_Phdr)*header.e_phnum);

        //runs through the program and e_phnum holds the number of entries.
        for(j = 0; j < header.e_phnum; j++)
            {
                //reads the program header of the files in elf
                read = fread(p_header, sizeof(Elf32_Phdr), 1, open_file);

                //seeks the file, reads from the beginning
                fseek(open_file, p_header->p_offset, SEEK_SET);

                //the segment size
                leftover_bytes = p_header->p_filesz;

            //for each segment start at beginning and go through
            for(int i = 0; i < leftover_bytes; i++)
            {
                //collects readfile
                char c = fgetc(open_file);
                //adds the readfile to the image
                fputc(c, imagefile);
            }

            //segment size
            totalbyte = p_header->p_filesz;

            //bootblock entry
            if(i==0)
            {
                //collects file
                fseek(imagefile, 0x1fe, SEEK_SET);
                //adds 55AA to the file, and works as a termination signal
                fputc(0x55, imagefile);
                fputc(0xAA,imagefile);

            }

            //inside the kernel-file
            if(i==1)
            {
                //if the total amount of bytes divided on the sectorsize is 0 
                if(totalbyte % SECTOR_SIZE == 0)
                {
                    //set the amount of sectors
                    amount_of_sector = (totalbyte/SECTOR_SIZE);

                }
                //if the modulo is anything else than 0,. since amount_of_sector is an int it will reduce down we have to add 1.
                else
                {
                      amount_of_sector = (totalbyte/SECTOR_SIZE) + 1;
                }

                //finding the total amount of bytes
                int totalsize = (amount_of_sector*SECTOR_SIZE);
                
                //finding the difference of the amount that will fill the blocks compared to the actual amount
                int dif = (totalsize - totalbyte);

                //if there is a difference use fputc to pad the remaining
                for(i = 0; i < dif; i++)
                {
                    fputc(0x0, imagefile);
                }

                //finds the correct place to put in amount of sectors and writes it in
                fseek(imagefile, 0x2, SEEK_SET);
                fputc(amount_of_sector, imagefile);
            }
        }
    }
}


/* print an error message and exit */
static void error(char *fmt, ...)
{
    va_list args;

    va_start (args, fmt);
    vfprintf (stderr, fmt, args);
    va_end (args);
    if (errno != 0) {
        perror (NULL);
    }
    exit (EXIT_FAILURE);
}

