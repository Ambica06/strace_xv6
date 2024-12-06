#include "types.h"
#include "stat.h"
#include "user.h"

#define CHUNK_SIZE (4096 * 4096) // Bytes to allocate in each iteration

int main() {
    int total_allocated = 0;

    while (1) {
        if (sbrk(CHUNK_SIZE) == (char*)-1) {
            printf(1, "failed after %d KB\n", total_allocated / 1024);
            break;
        }
        total_allocated += CHUNK_SIZE;
        sleep(10); // Sleep to slow the program
    }

    exit();
}
