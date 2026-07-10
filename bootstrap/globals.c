#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

typedef unsigned int u32;
int verbose;

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

void preprocess_header(const char *header_name) {
    char cmd[200];
    printf("// preprocessor output of <%s>\n{\n", header_name);
    fflush(stdout);
    snprintf(cmd, sizeof(cmd), "echo '#include <%s>' | cc -E -", header_name);
    if (!verbose) {
        strcat(cmd, " | grep -v '^#'");
        strcat(cmd, " | tr -s '\n'");
    }
    system(cmd);
    printf("}\n");
}

int main(int argc, char *argv[]) {
    FILE *stdin_ = stdin;
    FILE *stdout_ = stdout;
    FILE *stderr_ = stderr;
    jmp_buf buf;
    setjmp(buf);

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) {
            verbose = 1;
            continue;
        }
        fprintf(stderr, "globals: invalid argument '%s'\n", argv[i]);
        return 1;
    }
    errno = 0;
    nothing(stdin_);
    nothing(stdout_);
    nothing(stderr_);

    printf("// ----------------------------------------------------------------\n");
    printf("// Output of global header processor\n{\n");
    //printf("// preprocessor output of cc -E - < globals.c\n{\n");
    //fflush(stdout);
    //system("cc -E - < globals.c");
    //printf("} // end of preprocessor output\n\n");

#define xstr(x) #x
#define str(x) xstr(x)

    preprocess_header("sys/types.h");

    printf("// libc/c2_assert.c2i\n{\n"); {
        preprocess_header("assert.h");
    }
    printf("}\n");
    printf("// libc/c_errno.c2i\n{\n"); {
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
#ifdef ENAMETOOLONG
        printf("const i32 %-13s= %3d;  /* %s */\n", "ENAMETOOLONG", ENAMETOOLONG, "File name too long");
#endif
#ifdef ESTALE
        printf("const i32 %-13s= %3d;  /* %s */\n", "ESTALE", ESTALE, "Stale file handle");
#endif
        printf("\n");
        preprocess_header("errno.h");
    }
    printf("}\n");
    printf("// libc/csetjmp.c2i\n{\n"); {
        printf("const u32 JMP_BUF_SIZE = %zu;\n", sizeof(jmp_buf));
        //printf("const u32 SIGJMP_BUF_SIZE = %zu;\n", sizeof(sigjmp_buf));
        printf("\n");
        preprocess_header("setjmp.h");
    }
    printf("}\n");
    printf("// libc/csignal.c2i\n{\n"); {
#ifdef SA_NOCLDSTOP
        printf("const int SA_NOCLDSTOP = %d;\n", SA_NOCLDSTOP);
#endif
#ifdef SA_RESTART
        printf("const int SA_RESTART   = %d;\n", SA_RESTART);
#endif
        printf("\n");
#ifdef SIGHUP
        printf("const int SIGHUP      = %d;\n", SIGHUP);
#endif
#ifdef SIGINT
        printf("const int SIGINT      = %d;\n", SIGINT);
#endif
#ifdef SIGQUIT
        printf("const int SIGQUIT     = %d;\n", SIGQUIT);
#endif
#ifdef SIGILL
        printf("const int SIGILL      = %d;\n", SIGILL);
#endif
#ifdef SIGTRAP
        printf("const int SIGTRAP     = %d;\n", SIGTRAP);
#endif
#ifdef SIGABRT
        printf("const int SIGABRT     = %d;\n", SIGABRT);
#endif
#ifdef SIGIOT
        printf("const int SIGIOT      = %d;\n", SIGIOT);
#endif
#ifdef SIGBUS
        printf("const int SIGBUS      = %d;\n", SIGBUS);
#endif
#ifdef SIGFPE
        printf("const int SIGFPE      = %d;\n", SIGFPE);
#endif
#ifdef SIGKILL
        printf("const int SIGKILL     = %d;\n", SIGKILL);
#endif
#ifdef SIGUSR1
        printf("const int SIGUSR1     = %d;\n", SIGUSR1);
#endif
#ifdef SIGSEGV
        printf("const int SIGSEGV     = %d;\n", SIGSEGV);
#endif
#ifdef SIGUSR2
        printf("const int SIGUSR2     = %d;\n", SIGUSR2);
#endif
#ifdef SIGPIPE
        printf("const int SIGPIPE     = %d;\n", SIGPIPE);
#endif
#ifdef SIGALRM
        printf("const int SIGALRM     = %d;\n", SIGALRM);
#endif
#ifdef SIGTERM
        printf("const int SIGTERM     = %d;\n", SIGTERM);
#endif
#ifdef SIGSTKFLT
        printf("const int SIGSTKFLT   = %d;\n", SIGSTKFLT);
#endif
#ifdef SIGCLD
        printf("const int SIGCLD      = SIGCLD;\n");
#endif
#ifdef SIGCHLD
        printf("const int SIGCHLD     = %d;\n", SIGCHLD);
#endif
#ifdef SIGCONT
        printf("const int SIGCONT     = %d;\n", SIGCONT);
#endif
#ifdef SIGSTOP
        printf("const int SIGSTOP     = %d;\n", SIGSTOP);
#endif
#ifdef SIGTSTP
        printf("const int SIGTSTP     = %d;\n", SIGTSTP);
#endif
#ifdef SIGTTIN
        printf("const int SIGTTIN     = %d;\n", SIGTTIN);
#endif
#ifdef SIGTTOU
        printf("const int SIGTTOU     = %d;\n", SIGTTOU);
#endif
#ifdef SIGURG
        printf("const int SIGURG      = %d;\n", SIGURG);
#endif
#ifdef SIGXCPU
        printf("const int SIGXCPU     = %d;\n", SIGXCPU);
#endif
#ifdef SIGXFSZ
        printf("const int SIGXFSZ     = %d;\n", SIGXFSZ);
#endif
#ifdef SIGVTALRM
        printf("const int SIGVTALRM   = %d;\n", SIGVTALRM);
#endif
#ifdef SIGPROF
        printf("const int SIGPROF     = %d;\n", SIGPROF);
#endif
#ifdef SIGWINCH
        printf("const int SIGWINCH    = %d;\n", SIGWINCH);
#endif
#ifdef SIGPOLL
        printf("const int SIGPOLL     = SIGIO;\n");
#endif
#ifdef SIGIO
        printf("const int SIGIO       = %d;\n", SIGIO);
#endif
#ifdef SIGPWR
        printf("const int SIGPWR      = %d;\n", SIGPWR);
#endif
#ifdef SIGSYS
        printf("const int SIGSYS      = %d;\n", SIGSYS);
#endif
#ifdef SIGUNUSED
        printf("const int SIGUNUSED   = %d;\n", SIGUNUSED);
#endif
        printf("\n");
#ifdef SIG_SETMASK
        printf("const u32 SIG_SETMASK = %d;   /* set mask with sigprocmask() */\n", SIG_SETMASK);
#endif
#ifdef SIG_BLOCK
        printf("const u32 SIG_BLOCK = %d; /* set of signals to block */\n", SIG_BLOCK);
#endif
#ifdef SIG_UNBLOCK
        printf("const u32 SIG_UNBLOCK = %d;   /* set of signals to, well, unblock */\n", SIG_UNBLOCK);
#endif
        printf("\n");
        //printf("const u32 SIGSET_SIZE = %zu;\n", sizeof(sigset_t));
        //printf("const u32 SIGINFO_SIZE = %zu;\n", sizeof(siginfo_t));
        //printf("const u32 SIGACTION_SIZE = %zu;\n", sizeof(struct sigaction));
        //printf("\n");
        preprocess_header("signal.h");
    }
    printf("}\n");
    printf("// libc/ctermios.c2i\n{\n"); {
#ifdef NCCS
        printf("const u32 NCCS = %d;\n", NCCS);
#endif
        printf("\n");
        printf("/* c_cc characters */\n");
#ifdef VEOF
        printf("const unsigned VEOF = %d;\n", VEOF);
#endif
#ifdef VEOL
        printf("const unsigned VEOL = %d;\n", VEOL);
#endif
#ifdef VEOL2
        printf("const unsigned VEOL2 = %d;\n", VEOL2);
#endif
#ifdef VERASE
        printf("const unsigned VERASE = %d;\n", VERASE);
#endif
#ifdef VWERASE
        printf("const unsigned VWERASE = %d;\n", VWERASE);
#endif
#ifdef VKILL
        printf("const unsigned VKILL = %d;\n", VKILL);
#endif
#ifdef VREPRINT
        printf("const unsigned VREPRINT = %d;\n", VREPRINT);
#endif
#ifdef VINTR
        printf("const unsigned VINTR = %d;\n", VINTR);
#endif
#ifdef VQUIT
        printf("const unsigned VQUIT = %d;\n", VQUIT);
#endif
#ifdef VSUSP
        printf("const unsigned VSUSP = %d;\n", VSUSP);
#endif
#ifdef VDSUSP
        printf("const unsigned VDSUSP = %d;\n", VDSUSP);
#endif
#ifdef VSTART
        printf("const unsigned VSTART = %d;\n", VSTART);
#endif
#ifdef VSTOP
        printf("const unsigned VSTOP = %d;\n", VSTOP);
#endif
#ifdef VLNEXT
        printf("const unsigned VLNEXT = %d;\n", VLNEXT);
#endif
#ifdef VDISCARD
        printf("const unsigned VDISCARD = %d;\n", VDISCARD);
#endif
#ifdef VMIN
        printf("const unsigned VMIN = %d;\n", VMIN);
#endif
#ifdef VSWTC
        printf("const unsigned VSWTC = %d;\n", VSWTC);
#endif
#ifdef VTIME
        printf("const unsigned VTIME = %d;\n", VTIME);
#endif
#ifdef VSTATUS
        printf("const unsigned VSTATUS = %d;\n", VSTATUS);
#endif
        printf("\n");

        printf("/* c_iflag bits */\n");
#ifdef IGNBRK
        printf("const unsigned IGNBRK  = 0%06o;\n", IGNBRK);
#endif
#ifdef BRKINT
        printf("const unsigned BRKINT  = 0%06o;\n", BRKINT);
#endif
#ifdef IGNPAR
        printf("const unsigned IGNPAR  = 0%06o;\n", IGNPAR);
#endif
#ifdef PARMRK
        printf("const unsigned PARMRK  = 0%06o;\n", PARMRK);
#endif
#ifdef INPCK
        printf("const unsigned INPCK   = 0%06o;\n", INPCK);
#endif
#ifdef ISTRIP
        printf("const unsigned ISTRIP  = 0%06o;\n", ISTRIP);
#endif
#ifdef INLCR
        printf("const unsigned INLCR   = 0%06o;\n", INLCR);
#endif
#ifdef IGNCR
        printf("const unsigned IGNCR   = 0%06o;\n", IGNCR);
#endif
#ifdef ICRNL
        printf("const unsigned ICRNL   = 0%06o;\n", ICRNL);
#endif
#ifdef IUCLC
        printf("const unsigned IUCLC   = 0%06o;\n", IUCLC);
#endif
#ifdef IXON
        printf("const unsigned IXON    = 0%06o;\n", IXON);
#endif
#ifdef IXANY
        printf("const unsigned IXANY   = 0%06o;\n", IXANY);
#endif
#ifdef IXOFF
        printf("const unsigned IXOFF   = 0%06o;\n", IXOFF);
#endif
#ifdef IMAXBEL
        printf("const unsigned IMAXBEL = 0%06o;\n", IMAXBEL);
#endif
#ifdef IUTF8
        printf("const unsigned IUTF8   = 0%06o;\n", IUTF8);
#endif
        printf("\n");

        printf("/* c_oflag bits */\n");
#ifdef OPOST
        printf("const unsigned OPOST   = 0%06o;\n", OPOST);
#endif
#ifdef OLCUC
        printf("const unsigned OLCUC   = 0%06o;\n", OLCUC);
#endif
#ifdef ONLCR
        printf("const unsigned ONLCR   = 0%06o;\n", ONLCR);
#endif
#ifdef OCRNL
        printf("const unsigned OCRNL   = 0%06o;\n", OCRNL);
#endif
#ifdef ONOCR
        printf("const unsigned ONOCR   = 0%06o;\n", ONOCR);
#endif
#ifdef ONLRET
        printf("const unsigned ONLRET  = 0%06o;\n", ONLRET);
#endif
#ifdef OFILL
        printf("const unsigned OFILL   = 0%06o;\n", OFILL);
#endif
#ifdef OFDEL
        printf("const unsigned OFDEL   = 0%06o;\n", OFDEL);
#endif
#ifdef NLDLY
        printf("const unsigned NLDLY   = 0%06o;\n", NLDLY);
#endif
#ifdef NL0
        printf("const unsigned NL0     = 0%06o;\n", NL0);
#endif
#ifdef NL1
        printf("const unsigned NL1     = 0%06o;\n", NL1);
#endif
#ifdef CRDLY
        printf("const unsigned CRDLY   = 0%06o;\n", CRDLY);
#endif
#ifdef CR0
        printf("const unsigned CR0     = 0%06o;\n", CR0);
#endif
#ifdef CR1
        printf("const unsigned CR1     = 0%06o;\n", CR1);
#endif
#ifdef CR2
        printf("const unsigned CR2     = 0%06o;\n", CR2);
#endif
#ifdef CR3
        printf("const unsigned CR3     = 0%06o;\n", CR3);
#endif
#ifdef TABDLY
        printf("const unsigned TABDLY  = 0%06o;\n", TABDLY);
#endif
#ifdef TAB0
        printf("const unsigned TAB0    = 0%06o;\n", TAB0);
#endif
#ifdef TAB1
        printf("const unsigned TAB1    = 0%06o;\n", TAB1);
#endif
#ifdef TAB2
        printf("const unsigned TAB2    = 0%06o;\n", TAB2);
#endif
#ifdef TAB3
        printf("const unsigned TAB3    = 0%06o;\n", TAB3);
#endif
#ifdef XTABS
        printf("const unsigned XTABS    = 0%06o;\n", XTABS);
#endif
#ifdef BSDLY
        printf("const unsigned BSDLY   = 0%06o;\n", BSDLY);
#endif
#ifdef BS0
        printf("const unsigned BS0     = 0%06o;\n", BS0);
#endif
#ifdef BS1
        printf("const unsigned BS1     = 0%06o;\n", BS1);
#endif
#ifdef VTDLY
        printf("const unsigned VTDLY   = 0%06o;\n", VTDLY);
#endif
#ifdef VT0
        printf("const unsigned VT0     = 0%06o;\n", VT0);
#endif
#ifdef VT1
        printf("const unsigned VT1     = 0%06o;\n", VT1);
#endif
#ifdef FFDLY
        printf("const unsigned FFDLY   = 0%06o;\n", FFDLY);
#endif
#ifdef FF0
        printf("const unsigned FF0     = 0%06o;\n", FF0);
#endif
#ifdef FF1
        printf("const unsigned FF1     = 0%06o;\n", FF1);
#endif
        printf("\n");
        printf("/* c_cflag bit meaning */\n");
#ifdef CBAUD
        printf("const unsigned CBAUD   = 0%06o;\n", CBAUD);
#endif
#ifdef B0
        printf("const unsigned B0      = %d;     /* hang up */\n", B0);
#endif
#ifdef B50
        printf("const unsigned B50     = %d;\n", B50);
#endif
#ifdef B75
        printf("const unsigned B75     = %d;\n", B75);
#endif
#ifdef B110
        printf("const unsigned B110    = %d;\n", B110);
#endif
#ifdef B134
        printf("const unsigned B134    = %d;\n", B134);
#endif
#ifdef B150
        printf("const unsigned B150    = %d;\n", B150);
#endif
#ifdef B200
        printf("const unsigned B200    = %d;\n", B200);
#endif
#ifdef B300
        printf("const unsigned B300    = %d;\n", B300);
#endif
#ifdef B600
        printf("const unsigned B600    = %d;\n", B600);
#endif
#ifdef B1200
        printf("const unsigned B1200   = %d;\n", B1200);
#endif
#ifdef B1800
        printf("const unsigned B1800   = %d;\n", B1800);
#endif
#ifdef B2400
        printf("const unsigned B2400   = %d;\n", B2400);
#endif
#ifdef B4800
        printf("const unsigned B4800   = %d;\n", B4800);
#endif
#ifdef B9600
        printf("const unsigned B9600   = %d;\n", B9600);
#endif
#ifdef B19200
        printf("const unsigned B19200  = %d;\n", B19200);
#endif
#ifdef B38400
        printf("const unsigned B38400  = %d;\n", B38400);
#endif
#ifdef EXTA
#if EXTA == B19200
        printf("const unsigned EXTA    = B19200;\n");
#else
        printf("const unsigned EXTA    = 0%06o;\n", EXTA);
#endif
#endif
#ifdef EXTB
#if EXTB == B38400
        printf("const unsigned EXTB    = B38400;\n");
#else
        printf("const unsigned EXTB    = 0%06o;\n", EXTB);
#endif
#endif

#ifdef CIGNORE
        printf("const unsigned CIGNORE = 0%06o;\n", CIGNORE);
#endif
#ifdef CSIZE
        printf("const unsigned CSIZE   = 0%06o;\n", CSIZE);
#endif
#ifdef CS5
        printf("const unsigned CS5     = 0%06o;\n", CS5);
#endif
#ifdef CS6
        printf("const unsigned CS6     = 0%06o;\n", CS6);
#endif
#ifdef CS7
        printf("const unsigned CS7     = 0%06o;\n", CS7);
#endif
#ifdef CS8
        printf("const unsigned CS8     = 0%06o;\n", CS8);
#endif
#ifdef CSTOPB
        printf("const unsigned CSTOPB  = 0%06o;\n", CSTOPB);
#endif
#ifdef CREAD
        printf("const unsigned CREAD   = 0%06o;\n", CREAD);
#endif
#ifdef PARENB
        printf("const unsigned PARENB  = 0%06o;\n", PARENB);
#endif
#ifdef PARODD
        printf("const unsigned PARODD  = 0%06o;\n", PARODD);
#endif
#ifdef HUPCL
        printf("const unsigned HUPCL   = 0%06o;\n", HUPCL);
#endif
#ifdef CLOCAL
        printf("const unsigned CLOCAL  = 0%06o;\n", CLOCAL);
#endif
#ifdef CBAUDEX
        printf("const unsigned CBAUDEX = 0%06o;\n", CBAUDEX);
#endif
#ifdef BOTHER
        printf("const unsigned BOTHER  = 0%06o;\n", BOTHER);
#endif
#ifdef B57600
#if B57600 == 57600
        printf("const unsigned B57600  = %d;\n", B57600);
#else
        printf("const unsigned B57600  = 0%06o;\n", B57600);
#endif
#endif
#ifdef B115200
#if B115200 == 115200
        printf("const unsigned B115200 = %d;\n", B115200);
#else
        printf("const unsigned B115200 = 0%06o;\n", B115200);
#endif
#endif
#ifdef B230400
#if B230400 == 230400
        printf("const unsigned B230400 = %d;\n", B230400);
#else
        printf("const unsigned B230400 = 0%06o;\n", B230400);
#endif
#endif
#ifdef B460800
#if B460800 == 460800
        printf("const unsigned B460800 = %d;\n", B460800);
#else
        printf("const unsigned B460800 = 0%06o;\n", B460800);
#endif
#endif
#ifdef B500000
#if B500000 == 500000
        printf("const unsigned B500000 = %d;\n", B500000);
#else
        printf("const unsigned B500000 = 0%06o;\n", B500000);
#endif
#endif
#ifdef B576000
#if B576000 == 576000
        printf("const unsigned B576000 = %d;\n", B576000);
#else
        printf("const unsigned B576000 = 0%06o;\n", B576000);
#endif
#endif
#ifdef B921600
#if B921600 == 921600
        printf("const unsigned B921600 = %d;\n", B921600);
#else
        printf("const unsigned B921600 = 0%06o;\n", B921600);
#endif
#endif
#ifdef B1000000
#if B1000000 == 1000000
        printf("const unsigned B1000000 = %d;\n", B1000000);
#else
        printf("const unsigned B1000000 = 0%06o;\n", B1000000);
#endif
#endif
#ifdef B1152000
#if B1152000 == 1152000
        printf("const unsigned B1152000 = %d;\n", B1152000);
#else
        printf("const unsigned B1152000 = 0%06o;\n", B1152000);
#endif
#endif
#ifdef B1500000
#if B1500000 == 1500000
        printf("const unsigned B1500000 = %d;\n", B1500000);
#else
        printf("const unsigned B1500000 = 0%06o;\n", B1500000);
#endif
#endif
#ifdef B2000000
#if B2000000 == 2000000
        printf("const unsigned B2000000 = %d;\n", B2000000);
#else
        printf("const unsigned B2000000 = 0%06o;\n", B2000000);
#endif
#endif
#ifdef B2500000
#if B2500000 == 2500000
        printf("const unsigned B2500000 = %d;\n", B2500000);
#else
        printf("const unsigned B2500000 = 0%06o;\n", B2500000);
#endif
#endif
#ifdef B3000000
#if B3000000 == 3000000
        printf("const unsigned B3000000 = %d;\n", B3000000);
#else
        printf("const unsigned B3000000 = 0%06o;\n", B3000000);
#endif
#endif
#ifdef B3500000
#if B3500000 == 3500000
        printf("const unsigned B3500000 = %d;\n", B3500000);
#else
        printf("const unsigned B3500000 = 0%06o;\n", B3500000);
#endif
#endif
#ifdef B4000000
#if B4000000 == 4000000
        printf("const unsigned B4000000 = %d;\n", B4000000);
#else
        printf("const unsigned B4000000 = 0%06o;\n", B4000000);
#endif
#endif
#ifdef CIBAUD
        printf("const unsigned CIBAUD = 0%11o;  /* input baud rate */\n", CIBAUD);
#endif
#ifdef CMSPAR
        printf("const unsigned CMSPAR = 0%11o;  /* mark or space (stick) parity */\n", CMSPAR);
#endif
#ifdef CRTSCTS
        printf("const unsigned CRTSCTS = 0%06o;  /* flow control */\n", CRTSCTS);
#endif
        printf("\n");
        printf("/* c_lflag bits */\n");
#ifdef ISIG
        printf("const unsigned ISIG    = 0%06o;\n", ISIG);
#endif
#ifdef ICANON
        printf("const unsigned ICANON  = 0%06o;\n", ICANON);
#endif
#ifdef XCASE
        printf("const unsigned XCASE   = 0%06o;\n", XCASE);
#endif
#ifdef ECHO
        printf("const unsigned ECHO    = 0%06o;\n", ECHO);
#endif
#ifdef ECHOE
        printf("const unsigned ECHOE   = 0%06o;\n", ECHOE);
#endif
#ifdef ECHOK
        printf("const unsigned ECHOK   = 0%06o;\n", ECHOK);
#endif
#ifdef ECHONL
        printf("const unsigned ECHONL  = 0%06o;\n", ECHONL);
#endif
#ifdef NOFLSH
        printf("const unsigned NOFLSH  = 0%06o;\n", NOFLSH);
#endif
#ifdef TOSTOP
        printf("const unsigned TOSTOP  = 0%06o;\n", TOSTOP);
#endif
#ifdef ECHOCTL
        printf("const unsigned ECHOCTL = 0%06o;\n", ECHOCTL);
#endif
#ifdef ECHOPRT
        printf("const unsigned ECHOPRT = 0%06o;\n", ECHOPRT);
#endif
#ifdef ECHOKE
        printf("const unsigned ECHOKE  = 0%06o;\n", ECHOKE);
#endif
#ifdef FLUSHO
        printf("const unsigned FLUSHO  = 0%06o;\n", FLUSHO);
#endif
#ifdef PENDIN
        printf("const unsigned PENDIN  = 0%06o;\n", PENDIN);
#endif
#ifdef IEXTEN
        printf("const unsigned IEXTEN  = 0%06o;\n", IEXTEN);
#endif
#ifdef EXTPROC
        printf("const unsigned EXTPROC = 0%06o;\n", EXTPROC);
#endif

        printf("\n");
#ifdef TCSANOW
        printf("const int TCSANOW   = %d;\n", TCSANOW);
#endif
#ifdef TCSADRAIN
        printf("const int TCSADRAIN = %d;\n", TCSADRAIN);
#endif
#ifdef TCSAFLUSH
        printf("const int TCSAFLUSH = %d;\n", TCSAFLUSH);
#endif
#ifdef TCSASOFT
        printf("const int TCSASOFT = %d;\n", TCSASOFT);
#endif
        printf("\n");
#ifdef TCIFLUSH
        printf("const int TCIFLUSH = %d;\n", TCIFLUSH);
#endif
#ifdef TCOFLUSH
        printf("const int TCOFLUSH = %d;\n", TCOFLUSH);
#endif
#ifdef TCIOFLUSH
        printf("const int TCIOFLUSH = %d;\n", TCIOFLUSH);
#endif
#ifdef TCOOFF
        printf("const int TCOOFF = %d;\n", TCOOFF);
#endif
#ifdef TCOON
        printf("const int TCOON = %d;\n", TCOON);
#endif
#ifdef TCIOFF
        printf("const int TCIOFF = %d;\n", TCIOFF);
#endif
#ifdef TCION
        printf("const int TCION = %d;\n", TCION);
#endif
        printf("\n");
        preprocess_header("termios.h");
    }
    printf("}\n");
    printf("// libc/ctype.c2i\n{\n"); {
        preprocess_header("ctype.h");
    }
    printf("}\n");
    printf("// libc/libgen.c2i\n{\n"); {
        preprocess_header("libgen.h");
    }
    printf("}\n");
    printf("// libc/libc_dirent.c2i\n{\n"); {
#ifdef DT_UNKNOWN
        printf("const unsigned DT_UNKNOWN = %d;\n", DT_UNKNOWN);
#endif
#ifdef DT_FIFO
        printf("const unsigned DT_FIFO = %d;\n", DT_FIFO);
#endif
#ifdef DT_CHR
        printf("const unsigned DT_CHR = %d;\n", DT_CHR);
#endif
#ifdef DT_DIR
        printf("const unsigned DT_DIR = %d;\n", DT_DIR);
#endif
#ifdef DT_BLK
        printf("const unsigned DT_BLK = %d;\n", DT_BLK);
#endif
#ifdef DT_REG
        printf("const unsigned DT_REG = %d;\n", DT_REG);
#endif
#ifdef DT_LNK
        printf("const unsigned DT_LNK = %d;\n", DT_LNK);
#endif
#ifdef DT_SOCK
        printf("const unsigned DT_SOCK = %d;\n", DT_SOCK);
#endif
#ifdef DT_WHT
        printf("const unsigned DT_WHT = %d;\n", DT_WHT);
#endif
        printf("const u32 DIRENT_SIZE = %zu;\n", sizeof(struct dirent));
        printf("\n");
        preprocess_header("dirent.h");
    }
    printf("}\n");
    printf("// libc/libc_fcntl.c2i\n{\n"); {
#ifdef O_RDONLY
        printf("const u32 O_RDONLY    = %#8o;\n",  O_RDONLY);
#endif
#ifdef O_WRONLY
        printf("const u32 O_WRONLY    = %#8o;\n",  O_WRONLY);
#endif
#ifdef O_RDWR
        printf("const u32 O_RDWR      = %#8o;\n",  O_RDWR);
#endif
#ifdef O_CREAT
        printf("const u32 O_CREAT     = %#8o;\n",  O_CREAT);
#endif
#ifdef O_EXCL
        printf("const u32 O_EXCL      = %#8o;\n",  O_EXCL);
#endif
#ifdef O_NOCTTY
        printf("const u32 O_NOCTTY    = %#8o;\n",  O_NOCTTY);
#endif
#ifdef O_TRUNC
        printf("const u32 O_TRUNC     = %#8o;\n",  O_TRUNC);
#endif
#ifdef O_APPEND
        printf("const u32 O_APPEND    = %#8o;\n",  O_APPEND);
#endif
#ifdef O_NONBLOCK
        printf("const u32 O_NONBLOCK  = %#8o;\n",  O_NONBLOCK);
#endif
#ifdef O_DIRECT
        printf("const u32 O_DIRECT    = %#8o;\n",  O_DIRECT);
#endif
#ifdef O_LARGEFILE
        printf("const u32 O_LARGEFILE = %#8o;\n",  O_LARGEFILE);
#endif
#ifdef O_DIRECTORY
        printf("const u32 O_DIRECTORY = %#8o;\n",  O_DIRECTORY);
#endif
#ifdef O_NOFOLLOW
        printf("const u32 O_NOFOLLOW  = %#8o;\n",  O_NOFOLLOW);
#endif
#ifdef O_SYNC
        printf("const u32 O_SYNC      = %#8o;\n",  O_SYNC);
#endif
#ifdef O_NOATIME
        printf("const u32 O_NOATIME   = %#8o;\n",  O_NOATIME);
#endif
#ifdef O_CLOEXEC
        printf("const u32 O_CLOEXEC   = %#8o;\n",  O_CLOEXEC);
#endif
#ifdef O_PATH
        printf("const u32 O_PATH      = %#8o;\n",  O_PATH);
#endif
#ifdef O_TMPFILE
        printf("const u32 O_TMPFILE   = %#8o;\n",  O_TMPFILE);
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
        preprocess_header("fcntl.h");
    }
    printf("}\n");
    printf("// libc/libc_poll.c2i\n{\n"); {
        preprocess_header("poll.h");
    }
    printf("}\n");
    printf("// libc/libc_time.c2i\n{\n"); {
        preprocess_header("time.h");
    }
    printf("}\n");
    printf("// libc/regex.c2i\n{\n"); {
        preprocess_header("regex.h");
    }
    printf("}\n");
    printf("// libc/stdarg.c2i\n{\n"); {
        preprocess_header("stdarg.h");
    }
    printf("}\n");
    printf("// libc/stdio.c2i\n{\n"); {
        preprocess_header("stdio.h");
    }
    printf("}\n");
    printf("// libc/stdlib.c2i\n{\n"); {
        preprocess_header("stdlib.h");
    }
    printf("}\n");
    printf("// libc/string.c2i\n{\n"); {
        preprocess_header("string.h");
    }
    printf("}\n");
    printf("// libc/strings.c2i\n{\n"); {
        preprocess_header("strings.h");
    }
    printf("}\n");
    printf("// libc/sys_ioctl.c2i\n{\n"); {
#ifdef TCGETS
        printf("const unsigned TCGETS = %d;\n", TCGETS);
#endif
#ifdef TCSETS
        printf("const unsigned TCSETS = %d;\n", TCSETS);
#endif
#ifdef TCSETSW
        printf("const unsigned TCSETSW = %d;\n", TCSETSW);
#endif
#ifdef TCSETSF
        printf("const unsigned TCSETSF = %d;\n", TCSETSF);
#endif
#ifdef TCGETA
        printf("const unsigned TCGETA = %d;\n", TCGETA);
#endif
#ifdef TCSETA
        printf("const unsigned TCSETA = %d;\n", TCSETA);
#endif
        printf("\n");
#ifdef RTLD_DEFAULT
        printf("const unsigned RTLD_DEFAULT = %s;\n", str(RTLD_DEFAULT));
#endif
#ifdef RTLD_NEXT
        printf("const unsigned RTLD_NEXT = %s;\n", str(RTLD_NEXT));
#endif
#ifdef RTLD_SELF
        printf("const unsigned RTLD_SELF = %s;\n", str(RTLD_SELF));
#endif
#ifdef RTLD_MAIN_ONLY
        printf("const unsigned RTLD_MAIN_ONLY = %s;\n", str(RTLD_MAIN_ONLY));
#endif
        printf("\n");
        preprocess_header("sys/ioctl.h");
    }
    printf("}\n");
    printf("// libc/sys_mman.c2i\n{\n"); {
#ifdef PROT_NONE
        printf("const u32 PROT_NONE  = 0x%x;\n", PROT_NONE);
#endif
#ifdef PROT_READ
        printf("const u32 PROT_READ  = 0x%x;\n", PROT_READ);
#endif
#ifdef PROT_WRITE
        printf("const u32 PROT_WRITE = 0x%x;\n", PROT_WRITE);
#endif
#ifdef PROT_EXEC
        printf("const u32 PROT_EXEC  = 0x%x;\n", PROT_EXEC);
#endif
        printf("\n");
#ifdef MAP_SHARED
        printf("const u32 MAP_SHARED    = 0x%02x;\n", MAP_SHARED);
#endif
#ifdef MAP_PRIVATE
        printf("const u32 MAP_PRIVATE   = 0x%02x;\n", MAP_PRIVATE);
#endif
#ifdef MAP_FIXED
        printf("const u32 MAP_FIXED     = 0x%02x;\n", MAP_FIXED);
#endif
#ifdef MAP_ANONYMOUS
        printf("const u32 MAP_ANONYMOUS = 0x%02x;\n", MAP_ANONYMOUS);
#endif
#ifdef MAP_POPULATE
        printf("const u32 MAP_POPULATE  = 0x%02x;\n", MAP_POPULATE);
#endif
        printf("\n");
#ifdef MAP_FAILED
        printf("const usize MAP_FAILED = (usize)(%lld);\n", (long long)MAP_FAILED);
#endif
        printf("\n");
        preprocess_header("sys/mman.h");
    }
    printf("}\n");
    printf("// libc/sys_socket.c2i\n{\n"); {
        preprocess_header("sys/socket.h");
    }
    printf("}\n");
    printf("// libc/sys_stat.c2i\n{\n"); {
        preprocess_header("sys/stat.h");
    }
    printf("}\n");
    printf("// libc/sys_time.c2i\n{\n"); {
        preprocess_header("sys/time.h");
    }
    printf("}\n");
    printf("// libc/sys_utsname.c2i\n{\n"); {
        preprocess_header("sys/utsname.h");
    }
    printf("}\n");
    printf("// libc/unistd.c2i\n{\n"); {
        printf("/* Standard file descriptors.  */\n");
#ifdef STDIN_FILENO
        printf("const int STDIN_FILENO  = %d;\n", STDIN_FILENO);
#endif
#ifdef STDOUT_FILENO
        printf("const int STDOUT_FILENO = %d;\n", STDOUT_FILENO);
#endif
#ifdef STDERR_FILENO
        printf("const int STDERR_FILENO = %d;\n", STDERR_FILENO);
#endif
        printf("\n");
        printf("const u32 STAT_SIZE = %zu;\n", sizeof(struct stat));
        printf("\n");
        printf("/* access flags.  */\n");
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
#ifdef F_ULOCK
        printf("const int F_ULOCK = %d;      /* unlock locked section */\n", F_ULOCK);
#endif
#ifdef F_LOCK
        printf("const int F_LOCK  = %d;      /* lock a section for exclusive use */\n", F_LOCK);
#endif
#ifdef F_TLOCK
        printf("const int F_TLOCK = %d;      /* test and lock a section for exclusive use */\n", F_TLOCK);
#endif
#ifdef F_TEST
        printf("const int F_TEST  = %d;      /* test a section for locks by other procs */\n", F_TEST);
#endif
        printf("\n");
        printf("/* sysconf names.  */\n");
#ifdef _SC_ARG_MAX
        printf("const u32 _SC_ARG_MAX = %d;\n", _SC_ARG_MAX);
#endif
#ifdef _SC_CHILD_MAX
        printf("const u32 _SC_CHILD_MAX = %d;\n", _SC_CHILD_MAX);
#endif
#ifdef _SC_CLK_TCK
        printf("const u32 _SC_CLK_TCK = %d;\n", _SC_CLK_TCK);
#endif
#ifdef _SC_NGROUPS_MAX
        printf("const u32 _SC_NGROUPS_MAX = %d;\n", _SC_NGROUPS_MAX);
#endif
#ifdef _SC_OPEN_MAX
        printf("const u32 _SC_OPEN_MAX = %d;\n", _SC_OPEN_MAX);
#endif
#ifdef _SC_JOB_CONTROL
        printf("const u32 _SC_JOB_CONTROL = %d;\n", _SC_JOB_CONTROL);
#endif
#ifdef _SC_SAVED_IDS
        printf("const u32 _SC_SAVED_IDS = %d;\n", _SC_SAVED_IDS);
#endif
#ifdef _SC_VERSION
        printf("const u32 _SC_VERSION = %d;\n", _SC_VERSION);
#endif
#ifdef _SC_PAGESIZE
        printf("const u32 _SC_PAGESIZE = %d;\n", _SC_PAGESIZE);
#endif
#ifdef _SC_PAGE_SIZE
        printf("const u32 _SC_PAGE_SIZE = %d;\n", _SC_PAGE_SIZE);
#endif
#ifdef _SC_NPROCESSORS_CONF
        printf("const u32 _SC_NPROCESSORS_CONF = %d;\n", _SC_NPROCESSORS_CONF);
#endif
#ifdef _SC_NPROCESSORS_ONLN
        printf("const u32 _SC_NPROCESSORS_ONLN = %d;\n", _SC_NPROCESSORS_ONLN);
#endif
#ifdef _SC_PHYS_PAGES
        printf("const u32 _SC_PHYS_PAGES = %d;\n", _SC_PHYS_PAGES);
#endif
#ifdef _SC_AVPHYS_PAGES
        printf("const u32 _SC_AVPHYS_PAGES = %d;\n", _SC_AVPHYS_PAGES);
#endif
#ifdef _SC_MQ_OPEN_MAX
        printf("const u32 _SC_MQ_OPEN_MAX = %d;\n", _SC_MQ_OPEN_MAX);
#endif
#ifdef _SC_MQ_PRIO_MAX
        printf("const u32 _SC_MQ_PRIO_MAX = %d;\n", _SC_MQ_PRIO_MAX);
#endif
#ifdef _SC_RTSIG_MAX
        printf("const u32 _SC_RTSIG_MAX = %d;\n", _SC_RTSIG_MAX);
#endif
#ifdef _SC_SEM_NSEMS_MAX
        printf("const u32 _SC_SEM_NSEMS_MAX = %d;\n", _SC_SEM_NSEMS_MAX);
#endif
#ifdef _SC_SEM_VALUE_MAX
        printf("const u32 _SC_SEM_VALUE_MAX = %d;\n", _SC_SEM_VALUE_MAX);
#endif
#ifdef _SC_SIGQUEUE_MAX
        printf("const u32 _SC_SIGQUEUE_MAX = %d;\n", _SC_SIGQUEUE_MAX);
#endif
#ifdef _SC_TIMER_MAX
        printf("const u32 _SC_TIMER_MAX = %d;\n", _SC_TIMER_MAX);
#endif
#ifdef _SC_TZNAME_MAX
        printf("const u32 _SC_TZNAME_MAX = %d;\n", _SC_TZNAME_MAX);
#endif
#ifdef _SC_ASYNCHRONOUS_IO
        printf("const u32 _SC_ASYNCHRONOUS_IO = %d;\n", _SC_ASYNCHRONOUS_IO);
#endif
#ifdef _SC_FSYNC
        printf("const u32 _SC_FSYNC = %d;\n", _SC_FSYNC);
#endif
#ifdef _SC_MAPPED_FILES
        printf("const u32 _SC_MAPPED_FILES = %d;\n", _SC_MAPPED_FILES);
#endif
#ifdef _SC_MEMLOCK
        printf("const u32 _SC_MEMLOCK = %d;\n", _SC_MEMLOCK);
#endif
#ifdef _SC_MEMLOCK_RANGE
        printf("const u32 _SC_MEMLOCK_RANGE = %d;\n", _SC_MEMLOCK_RANGE);
#endif
#ifdef _SC_MEMORY_PROTECTION
        printf("const u32 _SC_MEMORY_PROTECTION = %d;\n", _SC_MEMORY_PROTECTION);
#endif
#ifdef _SC_MESSAGE_PASSING
        printf("const u32 _SC_MESSAGE_PASSING = %d;\n", _SC_MESSAGE_PASSING);
#endif
#ifdef _SC_PRIORITIZED_IO
        printf("const u32 _SC_PRIORITIZED_IO = %d;\n", _SC_PRIORITIZED_IO);
#endif
#ifdef _SC_REALTIME_SIGNALS
        printf("const u32 _SC_REALTIME_SIGNALS = %d;\n", _SC_REALTIME_SIGNALS);
#endif
#ifdef _SC_SEMAPHORES
        printf("const u32 _SC_SEMAPHORES = %d;\n", _SC_SEMAPHORES);
#endif
#ifdef _SC_SHARED_MEMORY_OBJECTS
        printf("const u32 _SC_SHARED_MEMORY_OBJECTS = %d;\n", _SC_SHARED_MEMORY_OBJECTS);
#endif
#ifdef _SC_SYNCHRONIZED_IO
        printf("const u32 _SC_SYNCHRONIZED_IO = %d;\n", _SC_SYNCHRONIZED_IO);
#endif
#ifdef _SC_TIMERS
        printf("const u32 _SC_TIMERS = %d;\n", _SC_TIMERS);
#endif
#ifdef _SC_AIO_LISTIO_MAX
        printf("const u32 _SC_AIO_LISTIO_MAX = %d;\n", _SC_AIO_LISTIO_MAX);
#endif
#ifdef _SC_AIO_MAX
        printf("const u32 _SC_AIO_MAX = %d;\n", _SC_AIO_MAX);
#endif
#ifdef _SC_AIO_PRIO_DELTA_MAX
        printf("const u32 _SC_AIO_PRIO_DELTA_MAX = %d;\n", _SC_AIO_PRIO_DELTA_MAX);
#endif
#ifdef _SC_DELAYTIMER_MAX
        printf("const u32 _SC_DELAYTIMER_MAX = %d;\n", _SC_DELAYTIMER_MAX);
#endif
#ifdef _SC_THREAD_KEYS_MAX
        printf("const u32 _SC_THREAD_KEYS_MAX = %d;\n", _SC_THREAD_KEYS_MAX);
#endif
#ifdef _SC_THREAD_STACK_MIN
        printf("const u32 _SC_THREAD_STACK_MIN = %d;\n", _SC_THREAD_STACK_MIN);
#endif
#ifdef _SC_THREAD_THREADS_MAX
        printf("const u32 _SC_THREAD_THREADS_MAX = %d;\n", _SC_THREAD_THREADS_MAX);
#endif
#ifdef _SC_TTY_NAME_MAX
        printf("const u32 _SC_TTY_NAME_MAX = %d;\n", _SC_TTY_NAME_MAX);
#endif
#ifdef _SC_THREADS
        printf("const u32 _SC_THREADS = %d;\n", _SC_THREADS);
#endif
#ifdef _SC_THREAD_ATTR_STACKADDR
        printf("const u32 _SC_THREAD_ATTR_STACKADDR = %d;\n", _SC_THREAD_ATTR_STACKADDR);
#endif
#ifdef _SC_THREAD_ATTR_STACKSIZE
        printf("const u32 _SC_THREAD_ATTR_STACKSIZE = %d;\n", _SC_THREAD_ATTR_STACKSIZE);
#endif
#ifdef _SC_THREAD_PRIORITY_SCHEDULING
        printf("const u32 _SC_THREAD_PRIORITY_SCHEDULING = %d;\n", _SC_THREAD_PRIORITY_SCHEDULING);
#endif
#ifdef _SC_THREAD_PRIO_INHERIT
        printf("const u32 _SC_THREAD_PRIO_INHERIT = %d;\n", _SC_THREAD_PRIO_INHERIT);
#endif
#ifdef _SC_THREAD_PRIO_PROTECT
        printf("const u32 _SC_THREAD_PRIO_PROTECT = %d;\n", _SC_THREAD_PRIO_PROTECT);
#endif
#ifdef _SC_THREAD_PRIO_CEILING
        printf("const u32 _SC_THREAD_PRIO_CEILING = %d;\n", _SC_THREAD_PRIO_CEILING);
#endif
#ifdef _SC_THREAD_PROCESS_SHARED
        printf("const u32 _SC_THREAD_PROCESS_SHARED = %d;\n", _SC_THREAD_PROCESS_SHARED);
#endif
#ifdef _SC_THREAD_SAFE_FUNCTIONS
        printf("const u32 _SC_THREAD_SAFE_FUNCTIONS = %d;\n", _SC_THREAD_SAFE_FUNCTIONS);
#endif
#ifdef _SC_GETGR_R_SIZE_MAX
        printf("const u32 _SC_GETGR_R_SIZE_MAX = %d;\n", _SC_GETGR_R_SIZE_MAX);
#endif
#ifdef _SC_GETPW_R_SIZE_MAX
        printf("const u32 _SC_GETPW_R_SIZE_MAX = %d;\n", _SC_GETPW_R_SIZE_MAX);
#endif
#ifdef _SC_LOGIN_NAME_MAX
        printf("const u32 _SC_LOGIN_NAME_MAX = %d;\n", _SC_LOGIN_NAME_MAX);
#endif
#ifdef SC_THREAD_DESTRUCTOR_ITERATIONS
        printf("const u32 SC_THREAD_DESTRUCTOR_ITERATIONS = %d;\n", SC_THREAD_DESTRUCTOR_ITERATIONS);
#endif
#ifdef _SC_ADVISORY_INFO
        printf("const u32 _SC_ADVISORY_INFO = %d;\n", _SC_ADVISORY_INFO);
#endif
#ifdef _SC_ATEXIT_MAX
        printf("const u32 _SC_ATEXIT_MAX = %d;\n", _SC_ATEXIT_MAX);
#endif
#ifdef _SC_BARRIERS
        printf("const u32 _SC_BARRIERS = %d;\n", _SC_BARRIERS);
#endif
#ifdef _SC_BC_BASE_MAX
        printf("const u32 _SC_BC_BASE_MAX = %d;\n", _SC_BC_BASE_MAX);
#endif
#ifdef _SC_BC_DIM_MAX
        printf("const u32 _SC_BC_DIM_MAX = %d;\n", _SC_BC_DIM_MAX);
#endif
#ifdef _SC_BC_SCALE_MAX
        printf("const u32 _SC_BC_SCALE_MAX = %d;\n", _SC_BC_SCALE_MAX);
#endif
#ifdef _SC_BC_STRING_MAX
        printf("const u32 _SC_BC_STRING_MAX = %d;\n", _SC_BC_STRING_MAX);
#endif
#ifdef _SC_CLOCK_SELECTION
        printf("const u32 _SC_CLOCK_SELECTION = %d;\n", _SC_CLOCK_SELECTION);
#endif
#ifdef _SC_COLL_WEIGHTS_MAX
        printf("const u32 _SC_COLL_WEIGHTS_MAX = %d;\n", _SC_COLL_WEIGHTS_MAX);
#endif
#ifdef _SC_CPUTIME
        printf("const u32 _SC_CPUTIME = %d;\n", _SC_CPUTIME);
#endif
#ifdef _SC_EXPR_NEST_MAX
        printf("const u32 _SC_EXPR_NEST_MAX = %d;\n", _SC_EXPR_NEST_MAX);
#endif
#ifdef _SC_HOST_NAME_MAX
        printf("const u32 _SC_HOST_NAME_MAX = %d;\n", _SC_HOST_NAME_MAX);
#endif
#ifdef _SC_IOV_MAX
        printf("const u32 _SC_IOV_MAX = %d;\n", _SC_IOV_MAX);
#endif
#ifdef _SC_IPV6
        printf("const u32 _SC_IPV6 = %d;\n", _SC_IPV6);
#endif
#ifdef _SC_LINE_MAX
        printf("const u32 _SC_LINE_MAX = %d;\n", _SC_LINE_MAX);
#endif
#ifdef _SC_MONOTONIC_CLOCK
        printf("const u32 _SC_MONOTONIC_CLOCK = %d;\n", _SC_MONOTONIC_CLOCK);
#endif
#ifdef _SC_RAW_SOCKETS
        printf("const u32 _SC_RAW_SOCKETS = %d;\n", _SC_RAW_SOCKETS);
#endif
#ifdef _SC_READER_WRITER_LOCKS
        printf("const u32 _SC_READER_WRITER_LOCKS = %d;\n", _SC_READER_WRITER_LOCKS);
#endif
#ifdef _SC_REGEXP
        printf("const u32 _SC_REGEXP = %d;\n", _SC_REGEXP);
#endif
#ifdef _SC_RE_DUP_MAX
        printf("const u32 _SC_RE_DUP_MAX = %d;\n", _SC_RE_DUP_MAX);
#endif
#ifdef _SC_SHELL
        printf("const u32 _SC_SHELL = %d;\n", _SC_SHELL);
#endif
#ifdef _SC_SPAWN
        printf("const u32 _SC_SPAWN = %d;\n", _SC_SPAWN);
#endif
#ifdef _SC_SPIN_LOCKS
        printf("const u32 _SC_SPIN_LOCKS = %d;\n", _SC_SPIN_LOCKS);
#endif
#ifdef _SC_SPORADIC_SERVER
        printf("const u32 _SC_SPORADIC_SERVER = %d;\n", _SC_SPORADIC_SERVER);
#endif
#ifdef _SC_SS_REPL_MAX
        printf("const u32 _SC_SS_REPL_MAX = %d;\n", _SC_SS_REPL_MAX);
#endif
#ifdef _SC_SYMLOOP_MAX
        printf("const u32 _SC_SYMLOOP_MAX = %d;\n", _SC_SYMLOOP_MAX);
#endif
#ifdef _SC_THREAD_CPUTIME
        printf("const u32 _SC_THREAD_CPUTIME = %d;\n", _SC_THREAD_CPUTIME);
#endif
#ifdef _SC_THREAD_SPORADIC_SERVER
        printf("const u32 _SC_THREAD_SPORADIC_SERVER = %d;\n", _SC_THREAD_SPORADIC_SERVER);
#endif
#ifdef _SC_TIMEOUTS
        printf("const u32 _SC_TIMEOUTS = %d;\n", _SC_TIMEOUTS);
#endif
#ifdef _SC_TRACE
        printf("const u32 _SC_TRACE = %d;\n", _SC_TRACE);
#endif
#ifdef _SC_TRACE_EVENT_FILTER
        printf("const u32 _SC_TRACE_EVENT_FILTER = %d;\n", _SC_TRACE_EVENT_FILTER);
#endif
#ifdef _SC_TRACE_EVENT_NAME_MAX
        printf("const u32 _SC_TRACE_EVENT_NAME_MAX = %d;\n", _SC_TRACE_EVENT_NAME_MAX);
#endif
#ifdef _SC_TRACE_INHERIT
        printf("const u32 _SC_TRACE_INHERIT = %d;\n", _SC_TRACE_INHERIT);
#endif
#ifdef _SC_TRACE_LOG
        printf("const u32 _SC_TRACE_LOG = %d;\n", _SC_TRACE_LOG);
#endif
#ifdef _SC_TRACE_NAME_MAX
        printf("const u32 _SC_TRACE_NAME_MAX = %d;\n", _SC_TRACE_NAME_MAX);
#endif
#ifdef _SC_TRACE_SYS_MAX
        printf("const u32 _SC_TRACE_SYS_MAX = %d;\n", _SC_TRACE_SYS_MAX);
#endif
#ifdef _SC_TRACE_USER_EVENT_MAX
        printf("const u32 _SC_TRACE_USER_EVENT_MAX = %d;\n", _SC_TRACE_USER_EVENT_MAX);
#endif
#ifdef _SC_TYPED_MEMORY_OBJECTS
        printf("const u32 _SC_TYPED_MEMORY_OBJECTS = %d;\n", _SC_TYPED_MEMORY_OBJECTS);
#endif
#ifdef _SC_V7_ILP32_OFF32
        printf("const u32 _SC_V7_ILP32_OFF32 = %d;\n", _SC_V7_ILP32_OFF32);
#endif
        printf("\n");
        preprocess_header("unistd.h");
    }
    printf("}\n");
    printf("// dl/dlfcn.c2i\n{\n"); {
        preprocess_header("dlfcn.h");
    }
    printf("}\n");
    printf("// math/math.c2i\n{\n"); {
#ifdef HUGE_VAL
        printf("const double HUGE_VAL   = %s;\n", str(HUGE_VAL));
#endif
#ifdef HUGE_VALF
        printf("const float HUGE_VALF   = %s;\n", str(HUGE_VALF));
#endif
#ifdef HUGE_VALL
        printf("const long double HUGE_VALL = %s;\n", str(HUGE_VALL));
#endif
#ifdef NAN
        printf("const double NAN        = %s;\n", str(NAN));
#endif
#ifdef INFINITY
        printf("const double INFINITY   = %s;\n", str(INFINITY));
#endif
#ifdef FP_NAN
        printf("const int FP_NAN        = %s;\n", str(FP_NAN));
#endif
#ifdef FP_INFINITE
        printf("const int FP_INFINITE   = %s;\n", str(FP_INFINITE));
#endif
#ifdef FP_ZERO
        printf("const int FP_ZERO       = %s;\n", str(FP_ZERO));
#endif
#ifdef FP_SUBNORMAL
        printf("const int FP_SUBNORMAL  = %s;\n", str(FP_SUBNORMAL));
#endif
#ifdef FP_NORMAL
        printf("const int FP_NORMAL     = %s;\n", str(FP_NORMAL));
#endif
#ifdef M_E
        printf("const double M_E         = %s;   /* e */\n", str(M_E));
#endif
#ifdef M_LOG2E
        printf("const double M_LOG2E     = %s;   /* log2(e) */\n", str(M_LOG2E));
#endif
#ifdef M_LOG10E
        printf("const double M_LOG10E    = %s;  /* log10(e) */\n", str(M_LOG10E));
#endif
#ifdef M_LN2
        printf("const double M_LN2       = %s;  /* loge(2) */\n", str(M_LN2));
#endif
#ifdef M_LN10
        printf("const double M_LN10      = %s;   /* loge(10) */\n", str(M_LN10));
#endif
#ifdef M_PI
        printf("const double M_PI        = %s;   /* pi */\n", str(M_PI));
#endif
#ifdef M_PI_2
        printf("const double M_PI_2      = %s;   /* pi/2 */\n", str(M_PI_2));
#endif
#ifdef M_PI_4
        printf("const double M_PI_4      = %s;  /* pi/4 */\n", str(M_PI_4));
#endif
#ifdef M_1_PI
        printf("const double M_1_PI      = %s;  /* 1/pi */\n", str(M_1_PI));
#endif
#ifdef M_2_PI
        printf("const double M_2_PI      = %s;  /* 2/pi */\n", str(M_2_PI));
#endif
#ifdef M_2_SQRTPI
        printf("const double M_2_SQRTPI  = %s;   /* 2/sqrt(pi) */\n", str(M_2_SQRTPI));
#endif
#ifdef M_SQRT2
        printf("const double M_SQRT2     = %s;   /* sqrt(2) */\n", str(M_SQRT2));
#endif
#ifdef M_SQRT1_2
        printf("const double M_SQRT1_2   = %s;  /* 1/sqrt(2) */\n", str(M_SQRT1_2));
#endif
#ifdef MAXFLOAT
        printf("const float MAXFLOAT    = %s   // use FLT_MAX instead?\n", str(MAXFLOAT));
#endif
        preprocess_header("math.h");
    }
    printf("}\n");
    printf("// pthread/pthread.c2i\n{\n"); {
        printf("const u32 SIZEOF_ATTR_T = %zu;\n", sizeof(pthread_attr_t));
        printf("const u32 SIZEOF_MUTEX_T = %zu;\n", sizeof(pthread_mutex_t));
        printf("const u32 SIZEOF_MUTEXATTR_T = %zu;\n", sizeof(pthread_mutexattr_t));
        printf("const u32 SIZEOF_COND_T = %zu;\n", sizeof(pthread_cond_t));
        printf("const u32 SIZEOF_CONDATTR_T = %zu;\n", sizeof(pthread_condattr_t));
        printf("\n");
        preprocess_header("pthread.h");
    }
    printf("}\n");
    printf("// ----------------------------------------------------------------\n");
    printf("}\n");
    return errno;   // should still be zero
}
