/*
 * Skeleton code for CSC 360, Spring 2024,  Assignment #4
 *
 * Prepared by: Michael Zastre (University of Victoria) 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * Some compile-time constants.
 */

#define REPLACE_NONE 0
#define REPLACE_FIFO 1
#define REPLACE_LRU  2
#define REPLACE_CLOCK 3
#define REPLACE_OPTIMAL 4


#define TRUE 1
#define FALSE 0
#define PROGRESS_BAR_WIDTH 60
#define MAX_LINE_LEN 100


/*
 * Some function prototypes to keep the compiler happy.
 */
int setup(void);
int teardown(void);
int output_report(void);
long resolve_address(long, int);
void error_resolve_address(long, int);


/*
 * Variables used to keep track of the number of memory-system events
 * that are simulated.
 */
int page_faults = 0;
int mem_refs    = 0;
int swap_outs   = 0;
int swap_ins    = 0;

//Added variables to help with helper functions
long fifo_counter = 0;
long lru_counter = 0;
int clock_hands = 0;
int found = FALSE; //Used for clock 


/*
 * Page-table information. You are permitted to modify this in order to
 * implement schemes such as CLOCK. However, you are not required
 * to do so.
 */
struct page_table_entry *page_table = NULL;
struct page_table_entry {
    long page_num;
    int dirty;
    int free;
    // A number that LRU can reference to see the least recently used page.
    // The lower the value the more recently used
    long frequency;
    //For CLOCK, to indicate if the page has been recently used
    int use_bit;
};


/*
 * These global variables will be set in the main() function. The default
 * values here are non-sensical, but it is safer to zero out a variable
 * rather than trust to random data that might be stored in it -- this
 * helps with debugging (i.e., eliminates a possible source of randomness
 * in misbehaving programs).
 */

int size_of_frame = 0;  /* power of 2 */
int size_of_memory = 0; /* number of frames */
int page_replacement_scheme = REPLACE_NONE;


/*
 * Function to convert a logical address into its corresponding 
 * physical address. The value returned by this function is the
 * physical address (or -1 if no physical address can exist for
 * the logical address given the current page-allocation state.
 */

long resolve_address(long logical, int memwrite)
{
    int i;
    long page, frame;
    long offset;
    long mask = 0;
    long effective;

    long least_recently_used;
    int lru_index = -1;
    
    /* Get the page and offset */
    page = (logical >> size_of_frame);

    for (i=0; i<size_of_frame; i++) {
        mask = mask << 1;
        mask |= 1;
    }
    offset = logical & mask;

    /* Find page in the inverted page table. */
    frame = -1;
    for ( i = 0; i < size_of_memory; i++ ) {
        if (!page_table[i].free && page_table[i].page_num == page) {
            frame = i;
            break;
        }
    }

    /* If frame is not -1, then we can successfully resolve the
     * address and return the result. */
    if (frame != -1) {
        
	// Update the dirty bit when writing to a page.
	if (memwrite){
		page_table[frame].dirty = TRUE;
	}

	// Update page table enteries
	switch (page_replacement_scheme){
		case REPLACE_FIFO:
        
		case REPLACE_LRU:
			page_table[frame].frequency = lru_counter;
			lru_counter++; 
            break;
        
		case REPLACE_CLOCK:
			page_table[frame].use_bit = 1;
            break;
	}	

        effective = (frame << size_of_frame) | offset;
        return effective;
    }


    /* If we reach this point, there was a page fault. Find
     * a free frame. */
    page_faults++;

    for ( i = 0; i < size_of_memory; i++) {
        if (page_table[i].free) {
            frame = i;
            break;
        }
    }

    /* If we found a free frame, then patch up the
     * page table entry and compute the effective
     * address. Otherwise return -1.
     */
     if (frame != -1){
        page_table[frame].page_num = page;
        page_table[i].free = FALSE;
        swap_ins++;

        page_table[frame].dirty = memwrite;
    }
    else if (frame == -1){
        switch (page_replacement_scheme){
            case REPLACE_FIFO:
                frame = fifo_counter; // Selects the frame to replace
    
                if (page_table[fifo_counter].dirty){
                        swap_outs++;
                        page_table[fifo_counter].dirty = FALSE;
                }
    
                // Reset the page table entry for the replaced page
                page_table[frame].page_num = page; 
                page_table[frame].free = FALSE;
                page_table[frame].dirty = memwrite; //set dirty based on current operation
                page_table[frame].frequency = 0;
                page_table[frame].use_bit = 0;
    
                fifo_counter = (fifo_counter + 1) % size_of_memory;
                swap_ins++;
                break;
        
            case REPLACE_LRU:
                least_recently_used = lru_counter;
                // Loop through the page table to find the lowest frequency value
                for (int i = 0; i < size_of_memory; i++){
                    if (!page_table[i].free && page_table[i].frequency < least_recently_used){
                        least_recently_used = page_table[i].frequency;
                        lru_index = i;
                    }
                }

                // If a LRU page is found, replace it
                if (lru_index != -1){
                    if (page_table[lru_index].dirty){
                        swap_outs++;
                        page_table[lru_index].dirty = FALSE; //Resetting dirty bit as it's being swaped out
                    }
                    // Reset the page table entry for the replaced page
                    page_table[lru_index].page_num = page;
                    page_table[lru_index].free = FALSE;
                    page_table[lru_index].frequency = lru_counter++; //Marks as most recently used
                    page_table[lru_index].dirty = memwrite;
                    
                    swap_ins++;
                    frame = lru_index;
                }
                break;
        
            case REPLACE_CLOCK:
                found = FALSE;
                while (!found){
                    //Check to see if current page is free, if not, check the use bit
                    if (!page_table[clock_hands].free){
                        if (page_table[clock_hands].use_bit == 0){
                            //Found page to replace
                            if (page_table[clock_hands].dirty){
                                swap_outs++;
                                page_table[clock_hands].dirty = FALSE;
                            }
                            // Reset the page table entry for the replaced page
                            page_table[clock_hands].page_num = page;
                            page_table[clock_hands].use_bit = 1; //Marks as most recently used
                            page_table[clock_hands].dirty = memwrite;
                            swap_ins++;
                            
                            frame = clock_hands;
                            found = TRUE;
                        }
                        else{
                            //Page has already been accessed
                            page_table[clock_hands].use_bit = 0;
                        }
                    }
                    clock_hands = (clock_hands + 1) % size_of_memory; //Moves to the next frame
                }
                break;
        }
    }

    if(frame != -1){
        effective = (frame << size_of_frame) | offset;
        return effective;
    } 
    else {
        return -1;
    }
}

/*
 * Super-simple progress bar.
 */
void display_progress(int percent)
{
    int to_date = PROGRESS_BAR_WIDTH * percent / 100;
    static int last_to_date = 0;
    int i;

    if (last_to_date < to_date) {
        last_to_date = to_date;
    } else {
        return;
    }

    printf("Progress [");
    for (i=0; i<to_date; i++) {
        printf(".");
    }
    for (; i<PROGRESS_BAR_WIDTH; i++) {
        printf(" ");
    }
    printf("] %3d%%", percent);
    printf("\r");
    fflush(stdout);
}


int setup()
{
    int i;

    page_table = (struct page_table_entry *)malloc(
        sizeof(struct page_table_entry) * size_of_memory
    );

    if (page_table == NULL) {
        fprintf(stderr,
            "Simulator error: cannot allocate memory for page table.\n");
        exit(1);
    }

    for (i=0; i<size_of_memory; i++) {
        page_table[i].free = TRUE;
    }

    return -1;
}


int teardown()
{

    return -1;
}


void error_resolve_address(long a, int l)
{
    fprintf(stderr, "\n");
    fprintf(stderr, 
        "Simulator error: cannot resolve address 0x%lx at line %d\n",
        a, l
    );
    exit(1);
}


int output_report()
{
    printf("\n");
    printf("Memory references: %d\n", mem_refs);
    printf("Page faults: %d\n", page_faults);
    printf("Swap ins: %d\n", swap_ins);
    printf("Swap outs: %d\n", swap_outs);

    return -1;
}


int main(int argc, char **argv)
{
    /* For working with command-line arguments. */
    int i;
    char *s;

    /* For working with input file. */
    FILE *infile = NULL;
    char *infile_name = NULL;
    struct stat infile_stat;
    int  line_num = 0;
    int infile_size = 0;

    /* For processing each individual line in the input file. */
    char buffer[MAX_LINE_LEN];
    long addr;
    char addr_type;
    int  is_write;

    /* For making visible the work being done by the simulator. */
    int show_progress = FALSE;

    /* Process the command-line parameters. Note that the
     * REPLACE_OPTIMAL scheme is not required for A#3.
     */
    for (i=1; i < argc; i++) {
        if (strncmp(argv[i], "--replace=", 9) == 0) {
            s = strstr(argv[i], "=") + 1;
            if (strcmp(s, "fifo") == 0) {
                page_replacement_scheme = REPLACE_FIFO;
            } else if (strcmp(s, "lru") == 0) {
                page_replacement_scheme = REPLACE_LRU;
            } else if (strcmp(s, "clock") == 0) {
                page_replacement_scheme = REPLACE_CLOCK;
            } else if (strcmp(s, "optimal") == 0) {
                page_replacement_scheme = REPLACE_OPTIMAL;
            } else {
                page_replacement_scheme = REPLACE_NONE;
            }
        } else if (strncmp(argv[i], "--file=", 7) == 0) {
            infile_name = strstr(argv[i], "=") + 1;
        } else if (strncmp(argv[i], "--framesize=", 12) == 0) {
            s = strstr(argv[i], "=") + 1;
            size_of_frame = atoi(s);
        } else if (strncmp(argv[i], "--numframes=", 12) == 0) {
            s = strstr(argv[i], "=") + 1;
            size_of_memory = atoi(s);
        } else if (strcmp(argv[i], "--progress") == 0) {
            show_progress = TRUE;
        }
    }

    if (infile_name == NULL) {
        infile = stdin;
    } else if (stat(infile_name, &infile_stat) == 0) {
        infile_size = (int)(infile_stat.st_size);
        /* If this fails, infile will be null */
        infile = fopen(infile_name, "r");  
    }


    if (page_replacement_scheme == REPLACE_NONE ||
        size_of_frame <= 0 ||
        size_of_memory <= 0 ||
        infile == NULL)
    {
        fprintf(stderr, 
            "usage: %s --framesize=<m> --numframes=<n>", argv[0]);
        fprintf(stderr, 
            " --replace={fifo|lru|optimal} [--file=<filename>]\n");
        exit(1);
    }


    setup();

    while (fgets(buffer, MAX_LINE_LEN-1, infile)) {
        line_num++;
        if (strstr(buffer, ":")) {
            sscanf(buffer, "%c: %lx", &addr_type, &addr);
            if (addr_type == 'W') {
                is_write = TRUE;
            } else {
                is_write = FALSE;
            }

            if (resolve_address(addr, is_write) == -1) {
                error_resolve_address(addr, line_num);
            }
            mem_refs++;
        } 

        if (show_progress) {
            display_progress(ftell(infile) * 100 / infile_size);
        }
    }
    

    teardown();
    output_report();

    fclose(infile);

    exit(0);
}