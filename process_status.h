#define RESOLUTION 100

extern int process_count;

struct process_status {
  struct process_status *next_process;
  int pid, ppid;
  unsigned char comm[18];
  char state;
  unsigned cpu;
  unsigned cpuhist[RESOLUTION];
  int histidx;
  unsigned long long
    utime,		/* user-mode CPU time accumulated by process */
    stime,		/* kernel-mode CPU time accumulated by process */
    cutime,		/* cumulative utime of process and reaped children */
    cstime,		/* cumulative stime of process and reaped children */
    start_time;	/* start time of process -- seconds since 1-1-70 */
  long
  priority,	/* kernel scheduling priority */
    timeout,	/* ? */
    nice,		/* standard unix nice level of process */
    rss,		/* resident set size from /proc/#/stat (pages) */
    it_real_value,	/* ? */
    /* the next 7 members come from /proc/#/statm */
    size,		/* total # of pages of memory */
    resident,	/* number of resident set (non-swapped) pages (4k) */
    share,		/* number of pages of shared (mmap'd) memory */
    trs,		/* text resident set size */
    lrs,		/* shared-lib resident set size */
    drs,		/* data resident set size */
    dt;		/* dirty pages */
  unsigned long
  vm_size,        /* same as vsize in kb */
    vm_lock,        /* locked pages in kb */
    vm_rss,         /* same as rss in kb */
    vm_data,        /* data size */
    vm_stack,       /* stack size */
    vm_exe,         /* executable size */
    vm_lib,         /* library size (all pages, not just used ones) */
    rtprio,		/* real-time priority */
    sched,		/* scheduling class */
    vsize,		/* number of pages of virtual memory ... */
    rss_rlim,	/* resident set size limit? */
    flags,		/* kernel flags for the process */
    min_flt,	/* number of minor page faults since process start */
    maj_flt,	/* number of major page faults since process start */
    cmin_flt,	/* cumulative min_flt of process and child processes */
    cmaj_flt,	/* cumulative maj_flt of process and child processes */
    nswap,		/* ? */
    cnswap,		/* cumulative nswap ? */
    start_code,	/* address of beginning of code segment */
    end_code,	/* address of end of code segment */
    start_stack,	/* address of the bottom of stack for the process */
    kstk_esp,	/* kernel stack pointer */
    kstk_eip,	/* kernel instruction pointer */
    wchan;		/* address of kernel wait channel proc is sleeping in */
  int
  ruid, rgid,     /* real      */
    euid, egid,     /* effective */
    suid, sgid,     /* saved     */
    fuid, fgid,     /* fs (used for file access only) */
    pgrp,		/* process group id */
    session,	/* session id */
    tty,		/* full device number of controlling terminal */
    tpgid,		/* terminal process group id */
    exit_signal,	/* might not be SIGCHLD */
    processor;      /* current (or most recent?) CPU */
};

struct process_status *read_processes_status(void);
void free_processes_status(struct process_status *this);
void diff(struct process_status *newp, struct process_status *oldp);
