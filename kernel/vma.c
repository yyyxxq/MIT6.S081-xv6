#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "vma.h"
struct file;

struct vma_table{
    struct spinlock lock;
    struct vma vma[NOFILE];
}vma_table;
void
vma_init(void){
    initlock(&vma_table.lock,"vma_table");
}
struct vma*
vma_alloc(void){
    acquire(&vma_table.lock);
    for(uint64 i=0;i<NOFILE;i++){
        if(vma_table.vma[i].used==0){
            vma_table.vma[i].used=1;
            release(&vma_table.lock);
            return &vma_table.vma[i];
        }
    }
    release(&vma_table.lock);
    return 0;
}
void
vma_free(struct vma *x){
    acquire(&vma_table.lock);
    x->file=0;
    x->used=0;
    release(&vma_table.lock);
}