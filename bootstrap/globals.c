#include <errno.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <unistd.h>

typedef unsigned int u32;

int pf(const char *fmt, ...) {
    int res;
    va_list ap;
    va_start(ap, fmt);
    res = va_arg(ap, int);
    va_end(ap);
    return res;
}

void nothing(void *p) {
}

int main() {
    FILE *stdin_ = stdin;
    FILE *stdout_ = stdout;
    FILE *stderr_ = stderr;
    jmp_buf buf;
    setjmp(buf);

    nothing(stdin_);
    nothing(stdout_);
    nothing(stderr_);

    printf("// libc/c_errno.c2i\n\n");
#ifdef EPERM
    printf("const i32 %-13s= %3d;  /* %s */\n", "EPERM", EPERM, "Operation not permitted");
#endif
#ifdef ENOENT
    printf("const i32 %-13s= %3d;  /* %s */\n", "ENOENT", ENOENT, "No such file or directory");
#endif
#ifdef ESRCH
    printf("const i32 %-13s= %3d;  /* %s */\n", "ESRCH", ESRCH, "No such process");
#endif
#ifdef EINTR
    printf("const i32 %-13s= %3d;  /* %s */\n", "EINTR", EINTR, "Interrupted system call");
#endif
#ifdef EIO
    printf("const i32 %-13s= %3d;  /* %s */\n", "EIO", EIO, "I/O error");
#endif
#ifdef ENXIO
    printf("const i32 %-13s= %3d;  /* %s */\n", "ENXIO", ENXIO, "No such device or address");
#endif
#ifdef E2BIG
    printf("const i32 %-13s= %3d;  /* %s */\n", "E2BIG", E2BIG, "Argument list too long");
#endif
#ifdef ENOEXEC
    printf("const i32 %-13s= %3d;  /* %s */\n", "ENOEXEC", ENOEXEC, "Exec format error");
#endif
#ifdef EBADF
    printf("const i32 %-13s= %3d;  /* %s */\n", "EBADF", EBADF, "Bad file number");
#endif
#ifdef ECHILD
    printf("const i32 %-13s= %3d;  /* %s */\n", "ECHILD", ECHILD, "No child processes");
#endif
#ifdef EAGAIN
    printf("const i32 %-13s= %3d;  /* %s */\n", "EAGAIN", EAGAIN, "Try again");
#endif
#ifdef ENOMEM
    printf("const i32 %-13s= %3d;  /* %s */\n", "ENOMEM", ENOMEM, "Out of memory");
#endif
#ifdef EACCES
    printf("const i32 %-13s= %3d;  /* %s */\n", "EACCES", EACCES, "Permission denied");
#endif
#ifdef EFAULT
    printf("const i32 %-13s= %3d;  /* %s */\n", "EFAULT", EFAULT, "Bad address");
#endif
#ifdef ENOTBLK
    printf("const i32 %-13s= %3d;  /* %s */\n", "ENOTBLK", ENOTBLK, "Block device required");
#endif
#ifdef EBUSY
    printf("const i32 %-13s= %3d;  /* %s */\n", "EBUSY", EBUSY, "Device or resource busy");
#endif
#ifdef EEXIST
    printf("const i32 %-13s= %3d;  /* %s */\n", "EEXIST", EEXIST, "File exists");
#endif
#ifdef EXDEV
    printf("const i32 %-13s= %3d;  /* %s */\n", "EXDEV", EXDEV, "Cross-device link");
#endif
#ifdef ENODEV
    printf("const i32 %-13s= %3d;  /* %s */\n", "ENODEV", ENODEV, "No such device");
#endif
#ifdef ENOTDIR
    printf("const i32 %-13s= %3d;  /* %s */\n", "ENOTDIR", ENOTDIR, "Not a directory");
#endif
#ifdef EISDIR
    printf("const i32 %-13s= %3d;  /* %s */\n", "EISDIR", EISDIR, "Is a directory");
#endif
#ifdef EINVAL
    printf("const i32 %-13s= %3d;  /* %s */\n", "EINVAL", EINVAL, "Invalid argument");
#endif
#ifdef ENFILE
    printf("const i32 %-13s= %3d;  /* %s */\n", "ENFILE", ENFILE, "File table overflow");
#endif
#ifdef EMFILE
    printf("const i32 %-13s= %3d;  /* %s */\n", "EMFILE", EMFILE, "Too many open files");
#endif
#ifdef ENOTTY
    printf("const i32 %-13s= %3d;  /* %s */\n", "ENOTTY", ENOTTY, "Not a typewriter");
#endif
#ifdef ETXTBSY
    printf("const i32 %-13s= %3d;  /* %s */\n", "ETXTBSY", ETXTBSY, "Text file busy");
#endif
#ifdef EFBIG
    printf("const i32 %-13s= %3d;  /* %s */\n", "EFBIG", EFBIG, "File too large");
#endif
#ifdef ENOSPC
    printf("const i32 %-13s= %3d;  /* %s */\n", "ENOSPC", ENOSPC, "No space left on device");
#endif
#ifdef ESPIPE
    printf("const i32 %-13s= %3d;  /* %s */\n", "ESPIPE", ESPIPE, "Illegal seek");
#endif
#ifdef EROFS
    printf("const i32 %-13s= %3d;  /* %s */\n", "EROFS", EROFS, "Read-only file system");
#endif
#ifdef EMLINK
    printf("const i32 %-13s= %3d;  /* %s */\n", "EMLINK", EMLINK, "Too many links");
#endif
#ifdef EPIPE
    printf("const i32 %-13s= %3d;  /* %s */\n", "EPIPE", EPIPE, "Broken pipe");
#endif
#ifdef EDOM
    printf("const i32 %-13s= %3d;  /* %s */\n", "EDOM", EDOM, "Math argument out of domain of func");
#endif
#ifdef ERANGE
    printf("const i32 %-13s= %3d;  /* %s */\n", "ERANGE", ERANGE, "Math result not representable");
#endif
#ifdef EALREADY
    printf("const i32 %-13s= %3d;  /* %s */\n", "EALREADY", EALREADY, "Operation already in progress");
#endif
#ifdef EINPROGRESS
    printf("const i32 %-13s= %3d;  /* %s */\n", "EINPROGRESS", EINPROGRESS, "Operation now in progress");
#endif
#ifdef ESTALE
    printf("const i32 %-13s= %3d;  /* %s */\n", "ESTALE", ESTALE, "Stale file handle");
#endif

    printf("\n");
    printf("// libc/fcntl.c2i\n\n");
#ifdef O_RDONLY
    printf("const u32 O_RDONLY    =%#9o;\n",  O_RDONLY);
#endif
#ifdef O_WRONLY
    printf("const u32 O_WRONLY    =%#9o;\n",  O_WRONLY);
#endif
#ifdef O_RDWR
    printf("const u32 O_RDWR      =%#9o;\n",  O_RDWR);
#endif
#ifdef O_CREAT
    printf("const u32 O_CREAT     =%#9o;\n",  O_CREAT);
#endif
#ifdef O_EXCL
    printf("const u32 O_EXCL      =%#9o;\n",  O_EXCL);
#endif
#ifdef O_NOCTTY
    printf("const u32 O_NOCTTY    =%#9o;\n",  O_NOCTTY);
#endif
#ifdef O_TRUNC
    printf("const u32 O_TRUNC     =%#9o;\n",  O_TRUNC);
#endif
#ifdef O_APPEND
    printf("const u32 O_APPEND    =%#9o;\n",  O_APPEND);
#endif
#ifdef O_NONBLOCK
    printf("const u32 O_NONBLOCK  =%#9o;\n",  O_NONBLOCK);
#endif
#ifdef O_DIRECT
    printf("const u32 O_DIRECT    =%#9o;\n",  O_DIRECT);
#endif
#ifdef O_LARGEFILE
    printf("const u32 O_LARGEFILE =%#9o;\n",  O_LARGEFILE);
#endif
#ifdef O_DIRECTORY
    printf("const u32 O_DIRECTORY =%#9o;\n",  O_DIRECTORY);
#endif
#ifdef O_NOFOLLOW
    printf("const u32 O_NOFOLLOW  =%#9o;\n",  O_NOFOLLOW);
#endif
#ifdef O_SYNC
    printf("const u32 O_SYNC      =%#9o;\n",  O_SYNC);
#endif
#ifdef O_NOATIME
    printf("const u32 O_NOATIME   =%#9o;\n",  O_NOATIME);
#endif
#ifdef O_CLOEXEC
    printf("const u32 O_CLOEXEC   =%#9o;\n",  O_CLOEXEC);
#endif
#ifdef O_PATH
    printf("const u32 O_PATH      =%#9o;\n",  O_PATH);
#endif
#ifdef O_TMPFILE
    printf("const u32 O_TMPFILE   =%#9o;\n",  O_TMPFILE);
#endif
    printf("\n");
#ifdef F_DUPFD
    printf("const u32 F_DUPFD = %d;\n",  F_DUPFD);
#endif
#ifdef F_GETFD
    printf("const u32 F_GETFD = %d;\n",  F_GETFD);
#endif
#ifdef F_SETFD
    printf("const u32 F_SETFD = %d;\n",  F_SETFD);
#endif
#ifdef F_GETFL
    printf("const u32 F_GETFL = %d;\n",  F_GETFL);
#endif
#ifdef F_SETFL
    printf("const u32 F_SETFL = %d;\n",  F_SETFL);
#endif
    printf("\n");
#ifdef AT_FDCWD
    printf("const i32 AT_FDCWD = %d;\n",  AT_FDCWD);
#endif
#ifdef FD_CLOEXEC
    printf("const u32 FD_CLOEXEC = %u;\n",  FD_CLOEXEC);
#endif
    printf("\n");
    printf("// libc/dirent.c2i\n\n");
#ifdef DT_UNKNOWN
    printf("const c_uint DT_UNKNOWN = %d;\n", DT_UNKNOWN);
#endif
#ifdef DT_FIFO
    printf("const c_uint DT_FIFO = %d;\n", DT_FIFO);
#endif
#ifdef DT_CHR
    printf("const c_uint DT_CHR = %d;\n", DT_CHR);
#endif
#ifdef DT_DIR
    printf("const c_uint DT_DIR = %d;\n", DT_DIR);
#endif
#ifdef DT_BLK
    printf("const c_uint DT_BLK = %d;\n", DT_BLK);
#endif
#ifdef DT_REG
    printf("const c_uint DT_REG = %d;\n", DT_REG);
#endif
#ifdef DT_LNK
    printf("const c_uint DT_LNK = %d;\n", DT_LNK);
#endif
#ifdef DT_SOCK
    printf("const c_uint DT_SOCK = %d;\n", DT_SOCK);
#endif
#ifdef DT_WHT
    printf("const c_uint DT_WHT = %d;\n", DT_WHT);
#endif
    printf("\n");

    printf("// libc/unistd.c2i\n\n");
#ifdef R_OK
    printf("const u8 R_OK = %d;\n", R_OK);
#endif
#ifdef W_OK
    printf("const u8 W_OK = %d;\n", W_OK);
#endif
#ifdef X_OK
    printf("const u8 X_OK = %d;\n", X_OK);
#endif
#ifdef F_OK
    printf("const u8 F_OK = %d;\n", F_OK);
#endif
    printf("\n");

#ifdef _SC_ARG_MAX
    printf("u32 _SC_ARG_MAX = %d;\n", _SC_ARG_MAX);
#endif
#ifdef _SC_CHILD_MAX
    printf("u32 _SC_CHILD_MAX = %d;\n", _SC_CHILD_MAX);
#endif
#ifdef _SC_CLK_TCK
    printf("u32 _SC_CLK_TCK = %d;\n", _SC_CLK_TCK);
#endif
#ifdef _SC_NGROUPS_MAX
    printf("u32 _SC_NGROUPS_MAX = %d;\n", _SC_NGROUPS_MAX);
#endif
#ifdef _SC_OPEN_MAX
    printf("u32 _SC_OPEN_MAX = %d;\n", _SC_OPEN_MAX);
#endif
#ifdef _SC_JOB_CONTROL
    printf("u32 _SC_JOB_CONTROL = %d;\n", _SC_JOB_CONTROL);
#endif
#ifdef _SC_SAVED_IDS
    printf("u32 _SC_SAVED_IDS = %d;\n", _SC_SAVED_IDS);
#endif
#ifdef _SC_VERSION
    printf("u32 _SC_VERSION = %d;\n", _SC_VERSION);
#endif
#ifdef _SC_PAGESIZE
    printf("u32 _SC_PAGESIZE = %d;\n", _SC_PAGESIZE);
#endif
#ifdef _SC_PAGE_SIZE
    printf("u32 _SC_PAGE_SIZE = %d;\n", _SC_PAGE_SIZE);
#endif
#ifdef _SC_NPROCESSORS_CONF
    printf("u32 _SC_NPROCESSORS_CONF = %d;\n", _SC_NPROCESSORS_CONF);
#endif
#ifdef _SC_NPROCESSORS_ONLN
    printf("u32 _SC_NPROCESSORS_ONLN = %d;\n", _SC_NPROCESSORS_ONLN);
#endif
#ifdef _SC_PHYS_PAGES
    printf("u32 _SC_PHYS_PAGES = %d;\n", _SC_PHYS_PAGES);
#endif
#ifdef _SC_AVPHYS_PAGES
    printf("u32 _SC_AVPHYS_PAGES = %d;\n", _SC_AVPHYS_PAGES);
#endif
#ifdef _SC_MQ_OPEN_MAX
    printf("u32 _SC_MQ_OPEN_MAX = %d;\n", _SC_MQ_OPEN_MAX);
#endif
#ifdef _SC_MQ_PRIO_MAX
    printf("u32 _SC_MQ_PRIO_MAX = %d;\n", _SC_MQ_PRIO_MAX);
#endif
#ifdef _SC_RTSIG_MAX
    printf("u32 _SC_RTSIG_MAX = %d;\n", _SC_RTSIG_MAX);
#endif
#ifdef _SC_SEM_NSEMS_MAX
    printf("u32 _SC_SEM_NSEMS_MAX = %d;\n", _SC_SEM_NSEMS_MAX);
#endif
#ifdef _SC_SEM_VALUE_MAX
    printf("u32 _SC_SEM_VALUE_MAX = %d;\n", _SC_SEM_VALUE_MAX);
#endif
#ifdef _SC_SIGQUEUE_MAX
    printf("u32 _SC_SIGQUEUE_MAX = %d;\n", _SC_SIGQUEUE_MAX);
#endif
#ifdef _SC_TIMER_MAX
    printf("u32 _SC_TIMER_MAX = %d;\n", _SC_TIMER_MAX);
#endif
#ifdef _SC_TZNAME_MAX
    printf("u32 _SC_TZNAME_MAX = %d;\n", _SC_TZNAME_MAX);
#endif
#ifdef _SC_ASYNCHRONOUS_IO
    printf("u32 _SC_ASYNCHRONOUS_IO = %d;\n", _SC_ASYNCHRONOUS_IO);
#endif
#ifdef _SC_FSYNC
    printf("u32 _SC_FSYNC = %d;\n", _SC_FSYNC);
#endif
#ifdef _SC_MAPPED_FILES
    printf("u32 _SC_MAPPED_FILES = %d;\n", _SC_MAPPED_FILES);
#endif
#ifdef _SC_MEMLOCK
    printf("u32 _SC_MEMLOCK = %d;\n", _SC_MEMLOCK);
#endif
#ifdef _SC_MEMLOCK_RANGE
    printf("u32 _SC_MEMLOCK_RANGE = %d;\n", _SC_MEMLOCK_RANGE);
#endif
#ifdef _SC_MEMORY_PROTECTION
    printf("u32 _SC_MEMORY_PROTECTION = %d;\n", _SC_MEMORY_PROTECTION);
#endif
#ifdef _SC_MESSAGE_PASSING
    printf("u32 _SC_MESSAGE_PASSING = %d;\n", _SC_MESSAGE_PASSING);
#endif
#ifdef _SC_PRIORITIZED_IO
    printf("u32 _SC_PRIORITIZED_IO = %d;\n", _SC_PRIORITIZED_IO);
#endif
#ifdef _SC_REALTIME_SIGNALS
    printf("u32 _SC_REALTIME_SIGNALS = %d;\n", _SC_REALTIME_SIGNALS);
#endif
#ifdef _SC_SEMAPHORES
    printf("u32 _SC_SEMAPHORES = %d;\n", _SC_SEMAPHORES);
#endif
#ifdef _SC_SHARED_MEMORY_OBJECTS
    printf("u32 _SC_SHARED_MEMORY_OBJECTS = %d;\n", _SC_SHARED_MEMORY_OBJECTS);
#endif
#ifdef _SC_SYNCHRONIZED_IO
    printf("u32 _SC_SYNCHRONIZED_IO = %d;\n", _SC_SYNCHRONIZED_IO);
#endif
#ifdef _SC_TIMERS
    printf("u32 _SC_TIMERS = %d;\n", _SC_TIMERS);
#endif
#ifdef _SC_AIO_LISTIO_MAX
    printf("u32 _SC_AIO_LISTIO_MAX = %d;\n", _SC_AIO_LISTIO_MAX);
#endif
#ifdef _SC_AIO_MAX
    printf("u32 _SC_AIO_MAX = %d;\n", _SC_AIO_MAX);
#endif
#ifdef _SC_AIO_PRIO_DELTA_MAX
    printf("u32 _SC_AIO_PRIO_DELTA_MAX = %d;\n", _SC_AIO_PRIO_DELTA_MAX);
#endif
#ifdef _SC_DELAYTIMER_MAX
    printf("u32 _SC_DELAYTIMER_MAX = %d;\n", _SC_DELAYTIMER_MAX);
#endif
#ifdef _SC_THREAD_KEYS_MAX
    printf("u32 _SC_THREAD_KEYS_MAX = %d;\n", _SC_THREAD_KEYS_MAX);
#endif
#ifdef _SC_THREAD_STACK_MIN
    printf("u32 _SC_THREAD_STACK_MIN = %d;\n", _SC_THREAD_STACK_MIN);
#endif
#ifdef _SC_THREAD_THREADS_MAX
    printf("u32 _SC_THREAD_THREADS_MAX = %d;\n", _SC_THREAD_THREADS_MAX);
#endif
#ifdef _SC_TTY_NAME_MAX
    printf("u32 _SC_TTY_NAME_MAX = %d;\n", _SC_TTY_NAME_MAX);
#endif
#ifdef _SC_THREADS
    printf("u32 _SC_THREADS = %d;\n", _SC_THREADS);
#endif
#ifdef _SC_THREAD_ATTR_STACKADDR
    printf("u32 _SC_THREAD_ATTR_STACKADDR = %d;\n", _SC_THREAD_ATTR_STACKADDR);
#endif
#ifdef _SC_THREAD_ATTR_STACKSIZE
    printf("u32 _SC_THREAD_ATTR_STACKSIZE = %d;\n", _SC_THREAD_ATTR_STACKSIZE);
#endif
#ifdef _SC_THREAD_PRIORITY_SCHEDULING
    printf("u32 _SC_THREAD_PRIORITY_SCHEDULING = %d;\n", _SC_THREAD_PRIORITY_SCHEDULING);
#endif
#ifdef _SC_THREAD_PRIO_INHERIT
    printf("u32 _SC_THREAD_PRIO_INHERIT = %d;\n", _SC_THREAD_PRIO_INHERIT);
#endif
#ifdef _SC_THREAD_PRIO_PROTECT
    printf("u32 _SC_THREAD_PRIO_PROTECT = %d;\n", _SC_THREAD_PRIO_PROTECT);
#endif
#ifdef _SC_THREAD_PRIO_CEILING
    printf("u32 _SC_THREAD_PRIO_CEILING = %d;\n", _SC_THREAD_PRIO_CEILING);
#endif
#ifdef _SC_THREAD_PROCESS_SHARED
    printf("u32 _SC_THREAD_PROCESS_SHARED = %d;\n", _SC_THREAD_PROCESS_SHARED);
#endif
#ifdef _SC_THREAD_SAFE_FUNCTIONS
    printf("u32 _SC_THREAD_SAFE_FUNCTIONS = %d;\n", _SC_THREAD_SAFE_FUNCTIONS);
#endif
#ifdef _SC_GETGR_R_SIZE_MAX
    printf("u32 _SC_GETGR_R_SIZE_MAX = %d;\n", _SC_GETGR_R_SIZE_MAX);
#endif
#ifdef _SC_GETPW_R_SIZE_MAX
    printf("u32 _SC_GETPW_R_SIZE_MAX = %d;\n", _SC_GETPW_R_SIZE_MAX);
#endif
#ifdef _SC_LOGIN_NAME_MAX
    printf("u32 _SC_LOGIN_NAME_MAX = %d;\n", _SC_LOGIN_NAME_MAX);
#endif
#ifdef SC_THREAD_DESTRUCTOR_ITERATIONS
    printf("u32 SC_THREAD_DESTRUCTOR_ITERATIONS = %d;\n", SC_THREAD_DESTRUCTOR_ITERATIONS);
#endif
#ifdef _SC_ADVISORY_INFO
    printf("u32 _SC_ADVISORY_INFO = %d;\n", _SC_ADVISORY_INFO);
#endif
#ifdef _SC_ATEXIT_MAX
    printf("u32 _SC_ATEXIT_MAX = %d;\n", _SC_ATEXIT_MAX);
#endif
#ifdef _SC_BARRIERS
    printf("u32 _SC_BARRIERS = %d;\n", _SC_BARRIERS);
#endif
#ifdef _SC_BC_BASE_MAX
    printf("u32 _SC_BC_BASE_MAX = %d;\n", _SC_BC_BASE_MAX);
#endif
#ifdef _SC_BC_DIM_MAX
    printf("u32 _SC_BC_DIM_MAX = %d;\n", _SC_BC_DIM_MAX);
#endif
#ifdef _SC_BC_SCALE_MAX
    printf("u32 _SC_BC_SCALE_MAX = %d;\n", _SC_BC_SCALE_MAX);
#endif
#ifdef _SC_BC_STRING_MAX
    printf("u32 _SC_BC_STRING_MAX = %d;\n", _SC_BC_STRING_MAX);
#endif
#ifdef _SC_CLOCK_SELECTION
    printf("u32 _SC_CLOCK_SELECTION = %d;\n", _SC_CLOCK_SELECTION);
#endif
#ifdef _SC_COLL_WEIGHTS_MAX
    printf("u32 _SC_COLL_WEIGHTS_MAX = %d;\n", _SC_COLL_WEIGHTS_MAX);
#endif
#ifdef _SC_CPUTIME
    printf("u32 _SC_CPUTIME = %d;\n", _SC_CPUTIME);
#endif
#ifdef _SC_EXPR_NEST_MAX
    printf("u32 _SC_EXPR_NEST_MAX = %d;\n", _SC_EXPR_NEST_MAX);
#endif
#ifdef _SC_HOST_NAME_MAX
    printf("u32 _SC_HOST_NAME_MAX = %d;\n", _SC_HOST_NAME_MAX);
#endif
#ifdef _SC_IOV_MAX
    printf("u32 _SC_IOV_MAX = %d;\n", _SC_IOV_MAX);
#endif
#ifdef _SC_IPV6
    printf("u32 _SC_IPV6 = %d;\n", _SC_IPV6);
#endif
#ifdef _SC_LINE_MAX
    printf("u32 _SC_LINE_MAX = %d;\n", _SC_LINE_MAX);
#endif
#ifdef _SC_MONOTONIC_CLOCK
    printf("u32 _SC_MONOTONIC_CLOCK = %d;\n", _SC_MONOTONIC_CLOCK);
#endif
#ifdef _SC_RAW_SOCKETS
    printf("u32 _SC_RAW_SOCKETS = %d;\n", _SC_RAW_SOCKETS);
#endif
#ifdef _SC_READER_WRITER_LOCKS
    printf("u32 _SC_READER_WRITER_LOCKS = %d;\n", _SC_READER_WRITER_LOCKS);
#endif
#ifdef _SC_REGEXP
    printf("u32 _SC_REGEXP = %d;\n", _SC_REGEXP);
#endif
#ifdef _SC_RE_DUP_MAX
    printf("u32 _SC_RE_DUP_MAX = %d;\n", _SC_RE_DUP_MAX);
#endif
#ifdef _SC_SHELL
    printf("u32 _SC_SHELL = %d;\n", _SC_SHELL);
#endif
#ifdef _SC_SPAWN
    printf("u32 _SC_SPAWN = %d;\n", _SC_SPAWN);
#endif
#ifdef _SC_SPIN_LOCKS
    printf("u32 _SC_SPIN_LOCKS = %d;\n", _SC_SPIN_LOCKS);
#endif
#ifdef _SC_SPORADIC_SERVER
    printf("u32 _SC_SPORADIC_SERVER = %d;\n", _SC_SPORADIC_SERVER);
#endif
#ifdef _SC_SS_REPL_MAX
    printf("u32 _SC_SS_REPL_MAX = %d;\n", _SC_SS_REPL_MAX);
#endif
#ifdef _SC_SYMLOOP_MAX
    printf("u32 _SC_SYMLOOP_MAX = %d;\n", _SC_SYMLOOP_MAX);
#endif
#ifdef _SC_THREAD_CPUTIME
    printf("u32 _SC_THREAD_CPUTIME = %d;\n", _SC_THREAD_CPUTIME);
#endif
#ifdef _SC_THREAD_SPORADIC_SERVER
    printf("u32 _SC_THREAD_SPORADIC_SERVER = %d;\n", _SC_THREAD_SPORADIC_SERVER);
#endif
#ifdef _SC_TIMEOUTS
    printf("u32 _SC_TIMEOUTS = %d;\n", _SC_TIMEOUTS);
#endif
#ifdef _SC_TRACE
    printf("u32 _SC_TRACE = %d;\n", _SC_TRACE);
#endif
#ifdef _SC_TRACE_EVENT_FILTER
    printf("u32 _SC_TRACE_EVENT_FILTER = %d;\n", _SC_TRACE_EVENT_FILTER);
#endif
#ifdef _SC_TRACE_EVENT_NAME_MAX
    printf("u32 _SC_TRACE_EVENT_NAME_MAX = %d;\n", _SC_TRACE_EVENT_NAME_MAX);
#endif
#ifdef _SC_TRACE_INHERIT
    printf("u32 _SC_TRACE_INHERIT = %d;\n", _SC_TRACE_INHERIT);
#endif
#ifdef _SC_TRACE_LOG
    printf("u32 _SC_TRACE_LOG = %d;\n", _SC_TRACE_LOG);
#endif
#ifdef _SC_TRACE_NAME_MAX
    printf("u32 _SC_TRACE_NAME_MAX = %d;\n", _SC_TRACE_NAME_MAX);
#endif
#ifdef _SC_TRACE_SYS_MAX
    printf("u32 _SC_TRACE_SYS_MAX = %d;\n", _SC_TRACE_SYS_MAX);
#endif
#ifdef _SC_TRACE_USER_EVENT_MAX
    printf("u32 _SC_TRACE_USER_EVENT_MAX = %d;\n", _SC_TRACE_USER_EVENT_MAX);
#endif
#ifdef _SC_TYPED_MEMORY_OBJECTS
    printf("u32 _SC_TYPED_MEMORY_OBJECTS = %d;\n", _SC_TYPED_MEMORY_OBJECTS);
#endif
#ifdef _SC_V7_ILP32_OFF32
    printf("u32 _SC_V7_ILP32_OFF32 = %d;\n", _SC_V7_ILP32_OFF32);
#endif
    printf("\n");

    printf("// libc/sys_mman.c2i\n\n");
#ifdef PROT_NONE
    printf("const u32 PROT_NONE = %d;\n", PROT_NONE);
#endif
#ifdef PROT_READ
    printf("const u32 PROT_READ = %d;\n", PROT_READ);
#endif
#ifdef PROT_WRITE
    printf("const u32 PROT_WRITE = %d;\n", PROT_WRITE);
#endif
#ifdef PROT_EXEC
    printf("const u32 PROT_EXEC = %d;\n", PROT_EXEC);
#endif
    printf("\n");
#ifdef MAP_SHARED
    printf("const u32 MAP_SHARED = 0x%02x;\n", MAP_SHARED);
#endif
#ifdef MAP_PRIVATE
    printf("const u32 MAP_PRIVATE = 0x%02x;\n", MAP_PRIVATE);
#endif
#ifdef MAP_FIXED
    printf("const u32 MAP_FIXED = 0x%02x;\n", MAP_FIXED);
#endif
#ifdef MAP_ANONYMOUS
    printf("const u32 MAP_ANONYMOUS = 0x%02x;\n", MAP_ANONYMOUS);
#endif
#ifdef MAP_POPULATE
    printf("const u32 MAP_POPULATE = 0x%02x;\n", MAP_POPULATE);
#endif
#ifdef MAP_FAILED
    printf("const usize MAP_FAILED = %lld;\n", (long long)MAP_FAILED);
#endif
    printf("\n");

    printf("// libc/sys_ioctl.c2i\n\n");
#ifdef TCGETS
    printf("const c_uint TCGETS = %d;\n", TCGETS);
#endif
#ifdef TCSETS
    printf("const c_uint TCSETS = %d;\n", TCSETS);
#endif
#ifdef TCSETSW
    printf("const c_uint TCSETSW = %d;\n", TCSETSW);
#endif
#ifdef TCSETSF
    printf("const c_uint TCSETSF = %d;\n", TCSETSF);
#endif
#ifdef TCGETA
    printf("const c_uint TCGETA = %d;\n", TCGETA);
#endif
#ifdef TCSETA
    printf("const c_uint TCSETA = %d;\n", TCSETA);
#endif
    printf("\n");

    printf("// pthread/pthread.c2i\n\n");
    printf("const u32 SIZEOF_ATTR_T = %zu;\n", sizeof(pthread_attr_t));
    printf("const u32 SIZEOF_MUTEX_T = %zu;\n", sizeof(pthread_mutex_t));
    printf("const u32 SIZEOF_MUTEXATTR_T = %zu;\n", sizeof(pthread_mutexattr_t));
    printf("const u32 SIZEOF_COND_T = %zu;\n", sizeof(pthread_cond_t));
    printf("const u32 SIZEOF_CONDATTR_T = %zu;\n", sizeof(pthread_condattr_t));
    printf("\n");

    return errno;
}
