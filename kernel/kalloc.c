// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run
{
  struct run *next;
};

struct
{
  struct spinlock lock;
  struct run *freelist;
  uint32 nfree;
} kmem[NCPU];
struct spinlock steal;

void kinit()
{
  for (int i = 0; i < NCPU; i++)
  {
    initlock(&kmem[i].lock, "kmem");
    kmem[i].freelist = 0;
    kmem[i].nfree = 0;
  }
  initlock(&steal, "steal");

  freerange(end, (void *)PHYSTOP);
}

void freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa)
{
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run *)pa;

  push_off();
  int cid = cpuid();
  pop_off();

  acquire(&kmem[cid].lock);
  r->next = kmem[cid].freelist;
  kmem[cid].freelist = r;
  kmem[cid].nfree++;
  release(&kmem[cid].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r, *p;

  push_off();
  int cid = cpuid();
  acquire(&kmem[cid].lock);
  pop_off();
  r = kmem[cid].freelist;
  if (r)
  {
    kmem[cid].freelist = r->next;
    kmem[cid].nfree--;
  }
  release(&kmem[cid].lock);

  if (r == 0)
  {
    int k;
    acquire(&steal);
    for (int i = 0; i < NCPU - 1; i++)
    {
      int j = (cid + i + 1) % NCPU;
      acquire(&kmem[j].lock);
      r = kmem[j].freelist;
      if (r)
      {
        k = 1;
        p = r;
        while (k < kmem[j].nfree / 2)
        {
          p = p->next;
          k++;
        }
        kmem[j].freelist = p->next;
        p->next = 0;
        kmem[j].nfree -= k;
      }
      release(&kmem[j].lock);
      if (r)
      {
        acquire(&kmem[cid].lock);
        kmem[cid].nfree += k - 1;
        p->next = kmem[cid].freelist;
        kmem[cid].freelist = r->next;
        release(&kmem[cid].lock);
        break;
      }
    }
    release(&steal);
  }

  if (r)
    memset((char *)r, 5, PGSIZE); // fill with junk
  return (void *)r;
}
