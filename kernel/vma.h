struct vma{
  struct file *file;
  unsigned long prot;
  char flags;
  char used;
  char *addr;
  unsigned long length;
};