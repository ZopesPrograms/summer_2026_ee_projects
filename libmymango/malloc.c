/* File: malloc.c
 * --------------
 * ***** Dynamic heap allocator functions -- Sebastian's Implementation! *****
 */


 /*
 * The code given below is simple "bump" allocator from lecture.
 * An allocation request is serviced by using sbrk to extend
 * the heap segment.
 * It does not recycle memory (free is a no-op) so when all the
 * space set aside for the heap is consumed, it will not be able
 * to service any further requests.
 *
 * This code is given here just to show the very simplest of
 * approaches to dynamic allocation. You will replace this code
 * with your own heap allocator implementation.
 */

#include "malloc.h"
#include "memmap.h"
#include "printf.h"
#include <stddef.h>  // for NULL
#include <stdbool.h> // for true, false
#include "strings.h"

/*
 * Data variables private to this module used to track
 * statistics for debugging/validate heap:
 *    count_allocs, count_frees, total_bytes_requested
 */
static int count_allocs, count_frees, total_bytes_requested;

// Global type register for specifying the desired type of a pointer in its heap header.
enum MALLOC_TYPES { UNSPECIFIED, CHAR, INT, LONG, PTR, STR };
uint8_t __TYPE_REGISTER = UNSPECIFIED;

#define TOO_FAR_REVERSE                 -1 // payload_size_rev value when the previous header's payload_size is >INT16_MAX, so reverse freeing impossible.
#define DEBUG_MODE                      0 // Define to switch whether I want to print out the values returned by the malloc function
#define FREE_BACKTRACKING               1 // Define to switch whether free coalesces memory through backtracking.
#define DEBUG_BACKTRACK                 0 // Variable to turn on/off backtracking debug printfs
#define BYPASS_HEADERSKIP_REVERSE_BUG   1 // Sometimes the header previous payload skips over a few headers because
                                          // I didn't update its reverse payload somewhere; so it ignores new
                                          // allocations behind it and skips to its older antecedent; but as the 
                                          // error is consistent and can be bypassed, I will bypass it until I have
                                          // more time to fix.

void heap_dump(const char *label);

/*
 * Header struct for use in heap allocator memory tracking
*/

typedef struct {
    int payload_size;         // In bytes
    int16_t payload_size_rev; // In bytes (limited to INT16_MAX at most reverse)
    uint8_t status;           // 0 if free, 1 if in use
    uint8_t type;
} header_t;

/*
 * The segment of memory available for the heap runs from HEAP_START
 * to HEAP_MAX. Markers placed in memmap.ld establish these boundaries
 * and the pointer constants are declared in memmap.h
 */

// Helper functions which go from one header in the heap allocator to the next, or to the previous
void *next_header(void *cur_header)
{
    void* next = (char*)cur_header+((header_t*)cur_header)->payload_size+8;
    return (next < HEAP_MAX) ? next : NULL; // Return null if have reached end of heap.
}
void *prev_header(void *cur_header)
{
    void* previous = (char*)cur_header-((header_t*)cur_header)->payload_size_rev-8;
    if(BYPASS_HEADERSKIP_REVERSE_BUG) { // Error handling for reverse payload header skip bug (see above)
        if(((header_t*)previous)->payload_size != ((header_t*)cur_header)->payload_size_rev) ((header_t*)cur_header)->payload_size_rev = TOO_FAR_REVERSE;
    }
    if(((header_t*)cur_header)->payload_size_rev == TOO_FAR_REVERSE) return NULL; // Can't reverse far enough due to narrow int16_t address range.
    return previous;
}

/*
 * The sbrk function is used to extend the size of the in-use heap.
 * It internally tracks the current end of the heap using the pointer
 * cur_heap_end. The pointer is initialized to HEAP_START and is
 * moved upward as the in-use portion of heap segment enlarges.
 * Because cur_head_end is qualified as static, it is not stored
 * as local variable in stack frame, instead it is placed in global data.
 * The one variable is shared and retains its value between calls.
 *
 * The call sbrk(n) bumps up the cur_heap_end pointer, extending the in-use heap
 * by n bytes and returns a pointer to that new space.
 * Note that sbrk(0) is a means to access current end of heap without
 * making changes to it.
 */
void *sbrk(size_t nbytes) {
    void *heap_end = HEAP_START;
    if(total_bytes_requested == 0) return HEAP_START;
    while(next_header(heap_end) != NULL)
    {
        heap_end = next_header(heap_end);
    }
    return heap_end;
}

// round up size to multiple of n (for malloc, will use to round up n = 8)
// the expression below uses a clever efficient bitwise approach that relies on
// n being a power of two. Trace through the expression to learn how it works!
static inline size_t roundup(size_t val, size_t n) {
    return (val + n-1) & ~(n-1);
}

// Helper function to reset the heap if anything goes wrong
void heap_reset() {
    void *heap_index = HEAP_START;

    // Initializes first heap header with the size of the entirety of heap save for the first header
    header_t first = { roundup((size_t)((char*)HEAP_MAX-(char*)HEAP_START-8), 8), TOO_FAR_REVERSE, 0, UNSPECIFIED };
    memcpy(heap_index, &first, 8);
    count_allocs = 0;
    count_frees = 0;
    total_bytes_requested = 0;
    printf("\n\n\n\n\nERROR DETECTED IN HEAP STRUCTURE; HEAP RESET TO INITIAL CONDITIONS. \n\n\n\n\n\n");
}

void *malloc(size_t nbytes) {
    void *heap_index = HEAP_START;

    // Calculates heap space to allocate
    total_bytes_requested += nbytes;
    nbytes = roundup(nbytes, 8);
    // Initializes first heap header with the size of the entirety of heap save for the first header
    if(count_allocs == 0) {
        header_t first = { roundup((size_t)((char*)HEAP_MAX-(char*)HEAP_START-8), 8), TOO_FAR_REVERSE, 0, UNSPECIFIED };
        memcpy(heap_index, &first, 8);
    }

    // Iterates through headers to next heap memory block large enough to contain our allocation in 8-byte alignment
    header_t *head = (header_t*)heap_index;
    header_t *prev_head = NULL;
    while(head->payload_size < nbytes || head->status == 1) {
        heap_index = next_header(heap_index);
        if(heap_index == NULL) return NULL; // We were unable to find any available allocation sites within the heap, so terminate unsuccessfully (return NULL).
        prev_head = head;
        head = (header_t*)heap_index;
    }
    if(prev_head != NULL) head->payload_size_rev = prev_head->payload_size;

    // Places free block memory header at location where block splits between memory allocated and that left free.
    header_t split = { head->payload_size - nbytes - 8, TOO_FAR_REVERSE, 0, UNSPECIFIED };
    if(split.payload_size > 0) {
        heap_index = (char*)heap_index+(nbytes+8); // To new header position!
        if(nbytes < INT16_MAX) split.payload_size_rev = nbytes; // If it is possible to reverse from the next header to the just-allocated header, specify reverse payload size.
        
        if(DEBUG_BACKTRACK) printf("\nnew split header at %p with payload size %d and reverse payload size %d\n", heap_index, split.payload_size, split.payload_size_rev);
        *(header_t*)heap_index = split;
        heap_index = (char*)heap_index-(nbytes+8); // Back to previous header.
    }
    heap_index = (char*)heap_index+8; // To start of allocated memory block (what we return).

    // Sets header size, status, and type (specified by TYPE_REGISTER, default UNSPECIFIED) as requested by user of malloc
    head->status = 1;
    head->type = __TYPE_REGISTER;
    head->payload_size = nbytes;
    if(DEBUG_BACKTRACK) printf("allocated header at %p with payload size %d\n\n", head, head->payload_size);
    if(split.payload_size == 0) head->payload_size += 8; // If there is no space for a header, just increase payload size.

    count_allocs++;
    __TYPE_REGISTER = UNSPECIFIED; // Resets type register to unspecified type.
    if(DEBUG_MODE) printf("\n\n\n\nmalloc returned address %p\n\n\n\n\n", heap_index);
    if(DEBUG_BACKTRACK) {
        char printbuf[32];
        snprintf(printbuf, 32, "after malloc at %p", (char*)heap_index-8);
        heap_dump(printbuf);
    }
    return heap_index;
}

void free(void *ptr) {
    if(DEBUG_BACKTRACK) printf("\n\ncount_frees at %d\n\n", count_frees);    
    // Terminate if ptr does not point anywhere.
    if(ptr == NULL) return;
    // Locates header for ptr, sets its status as free and sets its type as unspecified.
    ptr = (char*)ptr - 8;
    header_t *ref_head = (header_t*)ptr;
    ref_head->status = 0;
    ref_head->type = UNSPECIFIED;
    count_frees++;
    
    // Forward memory recycling loop (consolidates free memory blocks in forwards direction).

    int new_payload = ref_head->payload_size;
    header_t *head = next_header(ref_head);
    while(head != NULL && head->status == 0) {
        new_payload += head->payload_size+8;
        head = next_header(head);
    }
    ref_head->payload_size = new_payload;
    // Ensures succeeding header (if not at end of heap) is correctly linked back to the new mega-free-block header.
    if(head != NULL) {
        head->payload_size_rev = TOO_FAR_REVERSE;
        if(new_payload < INT16_MAX) head->payload_size_rev = new_payload;
    }

    if(FREE_BACKTRACKING) {
        // Backwards memory recycling loop (consolidates free memory blocks in backwards direction).
        if(ref_head->payload_size_rev != TOO_FAR_REVERSE) {
            // Reset for backwards memory freeing trip
            new_payload = ref_head->payload_size;
            if(DEBUG_BACKTRACK) printf("\n\nbacktracking free start at %p with payload %d\n", ref_head, new_payload);

            // Backtrace size of free blocks in memory if they were consolidated backwards to the "earliest" unallocated block.
            header_t *prev_tail = ref_head;
            header_t *tail = prev_header(ref_head); 
            while(tail != NULL && tail->status == 0) { 
                new_payload += tail->payload_size+8;
                if(DEBUG_BACKTRACK) {
                    printf("prev tail at address %p\n", prev_tail);
                    printf("prev tail payload reverse: %d\n", prev_tail->payload_size_rev);
                    printf("\nreturn tail at address %p\n", tail);
                    printf("return tail payload: %d\n\n", tail->payload_size);
                }
                prev_tail = tail;
                tail = prev_header(tail);
            }
            prev_tail->payload_size = new_payload;
            if(DEBUG_BACKTRACK) {
                printf("for prev_tail at address %p, new payload %d", prev_tail, new_payload);
            }
            // Ensures succeeding header (if not at end of heap) is correctly linked back to the new mega-free-block header.
            if(head != NULL) {
                head->payload_size_rev = TOO_FAR_REVERSE;
                if(new_payload < INT16_MAX) head->payload_size_rev = new_payload;
            }
        }
    }
    if(DEBUG_BACKTRACK) {
        char printbuf[32];
        printf("\n\n\n");
        snprintf(printbuf, 32, "after free at %p", (char*)ptr-8);
        heap_dump(printbuf);
        printf("\n\n");
    }
}

/*
 * Heap dump function -- dumps the state of the entire heap, from HEAP_START to HEAP_MAX. For allocated blocks, it also shows
 * their content, up to 16 bytes.
*/
void heap_dump(const char *label) {
    void *heap_index = HEAP_START;
    printf("\n---------- HEAP DUMP (%s) ----------\n", label);
    printf("Heap segment at %p - %p\n", HEAP_START, HEAP_MAX); 
    if(count_allocs != 0) {
        // Iterates through headers to next heap memory block large enough to contain our allocation in 8-byte alignment, then
        // sets its status as allocated
        header_t *head = (header_t*)heap_index;
        while(true) {
            // Determines whether heap memory block is in use (allocated), and prints that alongside its memory range
            const char* allocated_s = "ALLOCATED";
            const char* free_s = "FREE";
            const char* use = (head->status == 1) ? allocated_s : free_s;
            printf("block of size %d from %p - %p %s",head->payload_size,heap_index,(char*)heap_index+head->payload_size+8,use); 

            // Prints contents of memory block being checked, if allocated (up to a max of 16 bytes).
            if(use == allocated_s) {
                printf(" with contents up to 32 bytes of: ");
                if(head->type == UNSPECIFIED) {
                    char contents[32];
                    size_t nbytes = (head->payload_size < 32) ? head->payload_size : 32;
                    memcpy(contents, (char*)heap_index+8, nbytes);
                    for(int i = 0; i < nbytes; i++) {
                        printf("%x ", (int)contents[i]);
                    }
                } else if(head->type == CHAR) {
                    char contents[33];
                    size_t nbytes = (head->payload_size < 32) ? head->payload_size : 32;
                    memcpy(contents, (char*)heap_index+8, nbytes);
                    contents[nbytes] = '\0';
                    printf("%s", contents);
                } else if(head->type == INT) {
                    int contents[8];
                    size_t nbytes = (head->payload_size < 32) ? head->payload_size : 32;
                    memcpy(contents, (char*)heap_index+8, nbytes);
                    for(int i = 0; i < nbytes/4; i++) {
                        printf("%d ", contents[i]);
                    }
                } else if(head->type == LONG) {
                    long contents[4];
                    size_t nbytes = (head->payload_size < 32) ? head->payload_size : 32;
                    memcpy(contents, (char*)heap_index+8, nbytes);
                    for(int i = 0; i < nbytes/8; i++) {
                        printf("%ld ", contents[i]);
                    }
                } else if(head->type == PTR) {
                    uintptr_t contents[4];
                    size_t nbytes = (head->payload_size < 32) ? head->payload_size : 32;
                    memcpy(contents, (char*)heap_index+8, nbytes);
                    for(int i = 0; i < nbytes/8; i++) {
                        printf("%p ", (void *)contents[i]);
                    }
                } else if(head->type == STR) {
                    uintptr_t contents[4];
                    size_t nbytes = (head->payload_size < 32) ? head->payload_size : 32;
                    memcpy(contents, (char*)heap_index+8, nbytes);
                    for(int i = 0; i < nbytes/8; i++) {
                        printf("\"%s\" ", (char *)contents[i]);
                    }
                }
            }
            printf("\n");

            // Goes to next header, breaks if that results in a non-existent "header" outside of the heap.
            heap_index = (char*)heap_index+(head->payload_size+8);
            if(heap_index >= HEAP_MAX) {
                break;
            } else {
                head = (header_t*)heap_index;
            }
        }
    }
    printf("----------  END DUMP (%s) ----------\n", label);
    printf("Stats: %d in-use (%d allocs, %d frees), %d total payload bytes requested\n\n",
        count_allocs - count_frees, count_allocs, count_frees, total_bytes_requested);
}

/*
 * Heap dump function -- dumps the state of the entire heap, from HEAP_START to HEAP_MAX. For allocated blocks, it also shows
 * their content, up to 16 bytes.
*/
void heap_dump_str(char* buf, size_t bufsize, const char *label) {
    void *heap_index = HEAP_START;
    snprintf(buf, bufsize, "\n---------- HEAP DUMP (%s) ----------\n", label);
    snprintf(buf+strlen(buf), bufsize-strlen(buf), "Heap segment at %p - %p\n", HEAP_START, HEAP_MAX); 
    if(count_allocs != 0) {
        // Iterates through headers to next heap memory block large enough to contain our allocation in 8-byte alignment, then
        // sets its status as allocated
        header_t *head = (header_t*)heap_index;
        while(true) {
            // Determines whether heap memory block is in use (allocated), and prints that alongside its memory range
            const char* allocated_s = "ALLOCATED";
            const char* free_s = "FREE";
            const char* use = (head->status == 1) ? allocated_s : free_s;
            snprintf(buf+strlen(buf), bufsize-strlen(buf), "block of size %d from %p - %p %s",head->payload_size,heap_index,(char*)heap_index+head->payload_size+8,use); 

            // Prints contents of memory block being checked, if allocated (up to a max of 16 bytes).
            if(use == allocated_s) {
                snprintf(buf+strlen(buf), bufsize-strlen(buf), " with contents up to 32 bytes of: ");
                if(head->type == UNSPECIFIED) {
                    char contents[32];
                    size_t nbytes = (head->payload_size < 32) ? head->payload_size : 32;
                    memcpy(contents, (char*)heap_index+8, nbytes);
                    for(int i = 0; i < nbytes; i++) {
                        snprintf(buf+strlen(buf), bufsize-strlen(buf), "%x ", (int)contents[i]);
                    }
                } else if(head->type == CHAR) {
                    char contents[33];
                    size_t nbytes = (head->payload_size < 32) ? head->payload_size : 32;
                    memcpy(contents, (char*)heap_index+8, nbytes);
                    contents[nbytes] = '\0';
                    snprintf(buf+strlen(buf), bufsize-strlen(buf), "%s", contents);
                } else if(head->type == INT) {
                    int contents[8];
                    size_t nbytes = (head->payload_size < 32) ? head->payload_size : 32;
                    memcpy(contents, (char*)heap_index+8, nbytes);
                    for(int i = 0; i < nbytes/4; i++) {
                        snprintf(buf+strlen(buf), bufsize-strlen(buf), "%d ", contents[i]);
                    }
                } else if(head->type == LONG) {
                    long contents[4];
                    size_t nbytes = (head->payload_size < 32) ? head->payload_size : 32;
                    memcpy(contents, (char*)heap_index+8, nbytes);
                    for(int i = 0; i < nbytes/8; i++) {
                        snprintf(buf+strlen(buf), bufsize-strlen(buf), "%ld ", contents[i]);
                    }
                } else if(head->type == PTR) {
                    uintptr_t contents[4];
                    size_t nbytes = (head->payload_size < 32) ? head->payload_size : 32;
                    memcpy(contents, (char*)heap_index+8, nbytes);
                    for(int i = 0; i < nbytes/8; i++) {
                        snprintf(buf+strlen(buf), bufsize-strlen(buf), "%p ", (void *)contents[i]);
                    }
                } else if(head->type == STR) {
                    uintptr_t contents[4];
                    size_t nbytes = (head->payload_size < 32) ? head->payload_size : 32;
                    memcpy(contents, (char*)heap_index+8, nbytes);
                    for(int i = 0; i < nbytes/8; i++) {
                        snprintf(buf+strlen(buf), bufsize-strlen(buf), "\"%s\" ", (char *)contents[i]);
                    }
                }
            }
            snprintf(buf+strlen(buf), bufsize-strlen(buf), "\n");

            // Goes to next header, breaks if that results in a non-existent "header" outside of the heap.
            heap_index = (char*)heap_index+(head->payload_size+8);
            if(heap_index >= HEAP_MAX) {
                break;
            } else {
                head = (header_t*)heap_index;
            }
        }
    }
    snprintf(buf+strlen(buf), bufsize-strlen(buf), "----------  END DUMP (%s) ----------\n", label);
}

// Malloc report function
void malloc_report(void) {
    printf("number of allocations: %d; number of frees: %d; total bytes requested: %d\n", count_allocs, count_frees, total_bytes_requested);
}

// Malloc report function
void malloc_report_str(char* buf, size_t bufsize) {
    snprintf(buf, bufsize, "number of allocations: %d; number of frees: %d; total bytes requested: %d\n", count_allocs, count_frees, total_bytes_requested);
}

// Helper functions to access internal debug info
int get_num_allocs() {
    return count_allocs;
}

int get_num_frees() {
    return count_frees;
}

int get_total_bytes_requested() {
    return total_bytes_requested;
}
