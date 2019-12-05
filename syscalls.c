#include "syscalls.h"

// linker (standalone.ld) sets heap start and end
extern unsigned int  _HEAP_START;
extern unsigned int  _HEAP_END;

static caddr_t heap = NULL;


// low level bulk memory allocator - used by malloc
caddr_t _sbrk ( int increment ) {
 
    caddr_t prevHeap;
    caddr_t nextHeap;
    
    if (heap == NULL) {
        // first allocation
        heap = (caddr_t)&_HEAP_START;
    }

    prevHeap = heap;
            
    // Always return data aligned on a 8 byte boundary 
    nextHeap = (caddr_t)(((unsigned int)(heap + increment) + 7) & ~7);        

    // get current stack pointer 
    register caddr_t stackPtr asm ("sp");
    
    // Check enough space and there is no collision with stack coming the other way
    // if stack is above start of heap
    if ( (((caddr_t)&_HEAP_START stackPtr) && (nextHeap > stackPtr)) || 
         (nextHeap >= (caddr_t)&_HEAP_END)) {    
        return NULL; // error - no more memory 
    } else {
        heap = nextHeap;
        return (caddr_t) prevHeap;    
    }    
}

