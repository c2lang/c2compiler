#ifndef EXTERNAL_H
#define EXTERNAL_H

// --- internally added ---
#include <assert.h>

typedef char bool;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long int64_t;
typedef unsigned long uint64_t;
typedef long ssize_t;
typedef unsigned long size_t;
#define true 1
#define false 0
#define NULL ((void*)0)
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)
#define to_container(type, member, ptr) ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

extern int dprintf(int fd, const char *format, ...);
extern void abort(void);

static void c2_assert(bool condition, const char* file, uint32_t line, const char* func, const char* condstr) {
  if (condition) return;
  static const char me[] = "TODO";
  dprintf(2, "%s: %s:%u %s: Assertion '%s' failed\n", me, file, line, func, condstr);
  abort();
}


// --- module c2 ---


typedef char c2_c_char;

typedef uint8_t c2_c_uchar;

typedef int16_t c2_c_short;

typedef uint16_t c2_c_ushort;

typedef int32_t c2_c_int;

typedef uint32_t c2_c_uint;

typedef int64_t c2_c_long;

typedef uint64_t c2_c_ulong;

typedef uint64_t c2_c_size;

typedef int64_t c2_c_ssize;

typedef int64_t c2_c_longlong;

typedef uint64_t c2_c_ulonglong;

typedef float c2_c_float;

typedef double c2_c_double;


#define c2_min_i8 (-128l)

#define c2_max_i8 (127)

#define c2_min_u8 (0)

#define c2_max_u8 (255)

#define c2_min_i16 (-32768l)

#define c2_max_i16 (32767)

#define c2_min_u16 (0)

#define c2_max_u16 (65535)

#define c2_min_i32 (-2147483648l)

#define c2_max_i32 (2147483647)

#define c2_min_u32 (0)

#define c2_max_u32 (4294967295)

#define c2_min_i64 (-9223372036854775807l)

#define c2_max_i64 (9223372036854775807l)

#define c2_min_u64 (0)

#define c2_max_u64 (18446744073709551615lu)

#define c2_min_isize (-9223372036854775807l)

#define c2_max_isize (9223372036854775807l)

#define c2_min_usize (0)

#define c2_max_usize (18446744073709551615lu)


// --- module c_errno ---


int32_t* __errno_location(void);

#define ENOENT (2)

#define EEXIST (17)


// --- module csetjmp ---

typedef struct __jmp_buf_tag_ __jmp_buf_tag;

struct __jmp_buf_tag_ {
   char data[200];
};

typedef __jmp_buf_tag* jmp_buf;

int32_t setjmp(jmp_buf __env);
void longjmp(jmp_buf __env, int32_t __val);


// --- module ctype ---


int32_t isalpha(int32_t c);
int32_t isdigit(int32_t c);
int32_t islower(int32_t c);
int32_t isprint(int32_t c);
int32_t isspace(int32_t c);
int32_t isupper(int32_t c);
int32_t isxdigit(int32_t c);


// --- module libc_dirent ---

typedef struct DIR_ DIR;
typedef struct dirent_ dirent;

struct DIR_ {
};

typedef int32_t (*FilterFn)(const dirent*);

typedef int32_t (*DirentCompareFn)(const dirent**, const dirent**);

struct dirent_ {
   uint64_t d_ino;
   int64_t d_off;
   uint16_t d_reclen;
   uint8_t d_type;
   char d_name[256];
};

DIR* opendir(const char* name);
int32_t closedir(DIR* dirp);
dirent* readdir(DIR* dirp);

#define DT_DIR (4)


// --- module libc_fcntl ---


int32_t open(const char* __file, int32_t __oflag, ...);
int32_t openat(int32_t dirfd, const char* pathname, int32_t flags, ...);
int32_t fcntl(int32_t __fd, int32_t __cmd, ...);

#define O_RDONLY (0)

#define O_WRONLY (01)

#define O_CREAT (0100)

#define O_NOCTTY (0400)

#define O_TRUNC (01000)

#define O_NONBLOCK (04000)

#define O_DIRECTORY (0200000)

#define O_NOFOLLOW (0400000)

#define O_CLOEXEC (02000000)

#define F_SETFD (2)

#define AT_FDCWD (-100)

#define FD_CLOEXEC (1)


// --- module stdio ---

typedef struct _IO_marker_ _IO_marker;
typedef struct FILE_ FILE;

struct _IO_marker_ {
   _IO_marker* next;
   FILE* sbuf;
   int32_t _pos;
};

struct FILE_ {
};

typedef uint64_t off_t;

int32_t fflush(FILE* __stream);
int32_t fprintf(FILE* __stream, const char* __format, ...);
int32_t printf(const char* __format, ...);
int32_t sprintf(char* __s, const char* __format, ...);
int32_t fputs(const char* __s, FILE* __stream);
int32_t puts(const char* __s);
void perror(const char* __s);

extern FILE* stdout;

extern FILE* stderr;


// --- module stdlib ---

typedef struct div_t_ div_t;
typedef struct Ldiv_t_ Ldiv_t;
typedef struct random_data_ random_data;
typedef struct drand48_data_ drand48_data;

struct div_t_ {
   int32_t quot;
   int32_t rem;
};

struct Ldiv_t_ {
   int64_t quot;
   int64_t rem;
};

struct random_data_ {
};

struct drand48_data_ {
};

typedef void (*AtExitFn)();

typedef void (*OnExitFn)(int32_t, void*);

typedef int32_t (*__compar_fn_t)(const void*, const void*);

void* calloc(uint64_t count, uint64_t size);
void* malloc(uint64_t size);
void free(void* ptr);
double strtod(const char* nptr, char** endptr);
uint64_t strtoull(const char* nptr, char** endptr, int32_t base);
void abort(void);
void exit(int32_t __status);
void _exit(int32_t __status);
char* getenv(const char* __name);

#define EXIT_FAILURE (1)

#define EXIT_SUCCESS (0)


// --- module string ---


void* memcpy(void* dest, const void* src, uint64_t n);
int32_t memcmp(const void* s1, const void* s2, uint64_t n);
void* memset(void* b, int32_t c, uint64_t len);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
int32_t strcmp(const char* s1, const char* s2);
int32_t strncmp(const char* s1, const char* s2, uint64_t n);
char* strdup(const char* s);
char* strchr(const char* s, int32_t c);
char* strtok(char* s, const char* delim);
uint64_t strlen(const char* s);
char* strerror(int32_t errnum);


// --- module sys_mman ---


typedef uint64_t off_t;

void* mmap(void* addr, uint64_t length, int32_t prot, int32_t flags, int32_t fd, off_t offset);
int32_t munmap(void* addr, uint64_t length);

#define PROT_READ (0x1)

#define PROT_WRITE (0x2)

#define MAP_PRIVATE (0x2)

#define MAP_POPULATE (0x8000)

#define MAP_FAILED (-1)


// --- module sys_stat ---


struct stat {
   uint64_t st_dev;
   uint64_t st_ino;
   uint64_t st_nlink;
   uint32_t st_mode;
   uint32_t st_uid;
   uint32_t st_gid;
   uint64_t st_rdev;
   int64_t st_size;
   int64_t st_blksize;
   int64_t st_blocks;
   int64_t st_atime;
   uint64_t st_atime_nsec;
   uint64_t st_mtime;
   uint64_t st_mtime_nsec;
   uint64_t st_ctime;
   uint64_t st_ctime_nsec;
   uint32_t __unused4;
   uint32_t __unused5;
   int64_t reserved[2];
};

typedef uint32_t Mode;

int32_t fstat(int32_t fd, struct stat* buf);
int32_t stat(const char* pathname, struct stat* buf);
int32_t mkdir(const char* __file, uint32_t mode);

#define S_IFMT (0170000)

#define S_IFREG (0100000)


// --- module sys_time ---

typedef struct timeval_ timeval;
typedef struct timezone_ timezone;

typedef int64_t time_t;

typedef int64_t suseconds_t;

struct timeval_ {
   time_t tv_sec;
   suseconds_t tv_usec;
};

struct timezone_ {
   int32_t tz_minuteswest;
   int32_t tz_dsttime;
};

int32_t gettimeofday(timeval* tv, timezone* tz);


// --- module unistd ---


typedef int32_t pid_t;

char* getcwd(char* buf, uint64_t size);
int32_t chdir(const char* path);
int32_t fchdir(int32_t fd);
int32_t close(int32_t fd);
int64_t read(int32_t fd, void* buf, uint64_t count);
int32_t isatty(int32_t fd);
int64_t write(int32_t fd, const void* buf, uint64_t count);
int32_t pipe(int32_t* pipefd);
int32_t fsync(int32_t fd);
pid_t fork(void);
pid_t waitpid(pid_t pid, int32_t* wstatus, int32_t options);
int32_t dup(int32_t oldfd);
int32_t execv(const char* pathname, char** argv);

#define STDOUT_FILENO (1)

#define STDERR_FILENO (2)


// --- module stdarg ---

// Note: this module is a special case and is custom generated

#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_end __builtin_va_end

int32_t vdprintf(int32_t __fd, const char* __fmt, va_list __arg);
int32_t vsprintf(char* str, const char* format, va_list __ap);
int32_t vsnprintf(char* str, uint64_t size, const char* format, va_list __ap);

#endif
