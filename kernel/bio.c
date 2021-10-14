// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct {
  struct spinlock lock;

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head;
} bcache[NBUF];
int nowtime;
struct buf buf[NBUF];
void
binit(void)
{
  struct buf *b;

  for(int i=0;i<NBUF;i++){
    initlock(&bcache[i].lock, "bcache");
    bcache[i].head.prev = &bcache[i].head;
    bcache[i].head.next = &bcache[i].head;
  }
  for(b = buf; b < buf+NBUF; b++){
    b->next = bcache[0].head.next;
    b->prev = &bcache[0].head;
    initsleeplock(&b->lock, "buffer");
    bcache[0].head.next->prev = b;
    bcache[0].head.next = b;
  }
  // Create linked list of buffers
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
#define min(x,y) ((x)>(y)?y:x)
static struct buf*
bget(uint dev, uint blockno)
{
  nowtime++;
  struct buf *b;
  int k=blockno%29;
  acquire(&bcache[k].lock);

  // Is the block already cached?
  struct buf *mi=0;
  for(b = bcache[k].head.next; b != &bcache[k].head; b = b->next){
    if((mi==0||b->tick<mi->tick)&&b->refcnt==0)mi=b;
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache[k].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }
  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  if(mi!=0){
//    mi->prev->next=mi->next;
//    mi->next->prev=mi->prev;
    mi->dev = dev;
    mi->blockno = blockno;
    mi->valid = 0;
    mi->refcnt = 1;
    mi->tick=nowtime;
    release(&bcache[k].lock);
    acquiresleep(&mi->lock);
    return mi;
  }
  for(int i=0;i<29;i++){
    if(i==k)continue;
    acquire(&bcache[i].lock);
    for(b = bcache[i].head.next; b != &bcache[i].head; b = b->next)
    if((mi==0||b->tick<mi->tick)&&b->refcnt==0)mi=b;
    if(mi){
      mi->prev->next=mi->next;
      mi->next->prev=mi->prev;
      mi->next = bcache[k].head.next;
      mi->prev = &bcache[k].head;
      bcache[k].head.next->prev = mi;
      bcache[k].head.next = mi;
      mi->dev = dev;
      mi->blockno = blockno;
      mi->valid = 0;
      mi->refcnt = 1;
      mi->tick=nowtime;
      release(&bcache[i].lock);
      release(&bcache[k].lock);
      acquiresleep(&mi->lock);
      return mi;
    }
    release(&bcache[i].lock);
  }
  release(&bcache[k].lock);

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  int k=b->blockno%29;
  acquire(&bcache[k].lock);
  b->refcnt--;
/*  if (b->refcnt == 0) {
    // no one is waiting for it.    
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache[k].head.next;
    b->prev = &bcache[k].head;
    bcache[k].head.next->prev = b;
    bcache[k].head.next = b;
  }*/
  release(&bcache[k].lock);
}

void
bpin(struct buf *b) {
  int k=b->blockno%29;
  acquire(&bcache[k].lock);
  b->refcnt++;
  release(&bcache[k].lock);
}

void
bunpin(struct buf *b) {
  int k=b->blockno%29;
  acquire(&bcache[k].lock);
  b->refcnt--;
  release(&bcache[k].lock);
}

