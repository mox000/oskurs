#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_FILE "image"
#define ARGS "[--extended] [--vm] <bootblock> <executable-file> ..."

#define SECTOR_SIZE 512


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
    //P_FILESZ ER STØRRELSEN PÅ FILEN, ALTSÅ HELE LINJEN - OFFSET
    //P_OFFSET ER DER VI SKAL STARTE Å LESE, ALTSÅ HVOR MANGE BYTES VI MÅ HOPPE OVER PÅ STARTEN
    //E_PHNUM HOLDS THE NUMBER OF ENTRIES IN THE PROGRAM HEADER.
    //E_PHOFF HOLDS THE PROGRAM HEADER TABLE FILE OFFSET IN BYTES. IF THERE IS NO HEADER IT HOLDS ZER0
    // elf er der hvor programmene ligger. så når man går inn i en header, der er det programheadere som vet hvor programmen ligger.
    Elf32_Ehdr header; //er elf 32 og er en struct som inneholder elfene vi skal bruke senere
    FILE * open_file;
    int i, j;
    int lest;
    int leftover_bytes;
    int totalbyte;
    int bytesize;
    int amount_of_sector;

    char buffer[6000];

    //assemblyfil har 61 bytes
    //c-filen har 4309 bytes 

    FILE *imagefile = fopen(IMAGE_FILE, "w+");

    //lese igjennom filene for å finne riktig fil. dette gjøres så lenge i er mindre enn antall filer og forutsetter at det er filen i *files
    for(i = 0; i < nfiles; i++)
    {
        printf("step 1\n");

        open_file = fopen(files[i], "rb");
        //fprintf(stderr, "This version of %s doesn't do anything.\n", open_file);
        
        printf("step 2\n");
        lest = fread(&header, sizeof(Elf32_Ehdr), 1, open_file);


        
        //fprintf(stderr, "This version of %s doesn't do anything.\n", lest);
        printf("step3 \n");
        Elf32_Phdr *p_header = malloc(sizeof(Elf32_Phdr)*header.e_phnum);

        printf("step 4\n");

        for(j = 0; j < header.e_phnum; j++){
            printf("123 hei\n");
            
            lest = fread(p_header, sizeof(Elf32_Phdr), 1, open_file);

            printf("step 5\n");

            fseek(open_file, p_header->p_offset, SEEK_SET);

            leftover_bytes = p_header->p_filesz;


            for(int i = 0; i < leftover_bytes; i++){
                char c = fgetc(open_file);
                fputc(c, imagefile);
            }
            fprintf(stderr, "remaining bytes: %d\n", leftover_bytes);

            printf("step 6 \n");

            fprintf(stderr, "buffer inneholder: %d \n ", buffer);

            totalbyte = p_header->p_filesz;
            fprintf(stderr, "total bytes: %d\n", totalbyte);
            


            leftover_bytes -= lest;
            printf("du er fucked,%d \n", leftover_bytes);

            if(i==0)
            {
                printf("\nnå kjører if i==0\n");
                
                printf("I = %d\n", i);

                printf("\nhenter imagefile og sette 55AA på korrekt plass\n");
                fseek(imagefile, 0x1fe, SEEK_SET);
                fputc(0x55, imagefile);
                fputc(0xAA,imagefile);

            }

            if(i==1)
            {
                printf("\nnå kjører i==1\n");
                printf("I = %d\n", i);
                
                printf("total bytes er: %d\n", totalbyte);
                printf("SECTOR_SIZE er : %d\n", SECTOR_SIZE);
                if(totalbyte % SECTOR_SIZE == 0)
                {
                    amount_of_sector = (totalbyte/SECTOR_SIZE);
                    printf("amount_of_sector er %d\n", amount_of_sector);

                }
                else
                {
                      amount_of_sector = (totalbyte/SECTOR_SIZE) + 1;

                }
              
                printf("amount_of_sector er %d\n", amount_of_sector);

                int totalsize = (amount_of_sector*SECTOR_SIZE);

                int dif = (totalsize - totalbyte);

                for(i = 0; i < dif; i++)
                {
                    fputc(0x0, imagefile);
                }

                fseek(imagefile, 0x2, SEEK_SET);
                fputc(amount_of_sector, imagefile);

                


            }


        }


    }


    //fprintf (stderr, "This version of %s doesn't do anything.\n", progname);
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






/*

                int kernel = (totalbyte - bytesize);

                int sector = ceil(leftover_bytes/sector_size);
                int sector2 = (leftover_bytes/sector_size);

                int total_kernel = (sector*sector_size);

                int total_imagefile = (total_kernel + sector_size);

                fseek(imagefile, 0x2, SEEK_SET);
                fputc(sector, imagefile);
                printf("hvis den har kommet hit sjekk image");
*/