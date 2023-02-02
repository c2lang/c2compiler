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


static const int8_t c2_min_i8 = -128l;

static const int8_t c2_max_i8 = 127;

static const uint8_t c2_min_u8 = 0;

static const uint8_t c2_max_u8 = 255;

static const int16_t c2_min_i16 = -32768l;

static const int16_t c2_max_i16 = 32767;

static const uint16_t c2_min_u16 = 0;

static const uint16_t c2_max_u16 = 65535;

static const int32_t c2_min_i32 = -2147483648l;

static const int32_t c2_max_i32 = 2147483647;

static const uint32_t c2_min_u32 = 0;

static const uint32_t c2_max_u32 = 4294967295;

static const int64_t c2_min_i64 = -9223372036854775807l;

static const int64_t c2_max_i64 = 9223372036854775807l;

static const uint64_t c2_min_u64 = 0;

static const uint64_t c2_max_u64 = 18446744073709551615lu;

static const int64_t c2_min_isize = -9223372036854775807l;

static const int64_t c2_max_isize = 9223372036854775807l;

static const uint64_t c2_min_usize = 0;

static const uint64_t c2_max_usize = 18446744073709551615lu;


// --- module c_errno ---


int32_t* __errno_location(void);

static const uint32_t ENOENT = 2;

static const uint32_t EEXIST = 17;


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


// --- module libc_fcntl ---


int32_t open(const char* __file, int32_t __oflag, ...);
int32_t openat(int32_t dirfd, const char* pathname, int32_t flags, ...);
int32_t fcntl(int32_t __fd, int32_t __cmd, ...);

static const uint32_t O_RDONLY = 0;

static const uint32_t O_WRONLY = 01;

static const uint32_t O_CREAT = 0100;

static const uint32_t O_NOCTTY = 0400;

static const uint32_t O_TRUNC = 01000;

static const uint32_t O_NONBLOCK = 04000;

static const uint32_t O_DIRECTORY = 0200000;

static const uint32_t O_NOFOLLOW = 0400000;

static const uint32_t O_CLOEXEC = 02000000;

static const uint32_t F_SETFD = 2;

static const int32_t AT_FDCWD = -100;

static const uint32_t FD_CLOEXEC = 1;


// --- module stdarg ---

// Note: this module is a special case and is custom generated

#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_end __builtin_va_end

int32_t vdprintf(int32_t __fd, const char* __fmt, va_list __arg);
int32_t vsprintf(char* str, const char* format, va_list __ap);
int32_t vsnprintf(char* str, uint64_t size, const char* format, va_list __ap);


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
uint64_t strtoull(const char* nptr, char** endptr, int32_t base);
void abort(void);
void exit(int32_t __status);
void _exit(int32_t __status);
char* getenv(const char* __name);

static const int8_t EXIT_FAILURE = 1;


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

static const uint32_t PROT_READ = 0x1;

static const uint32_t PROT_WRITE = 0x2;

static const uint32_t MAP_PRIVATE = 0x2;

static const uint32_t MAP_POPULATE = 0x8000;

static const size_t MAP_FAILED = -1;


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

static const uint32_t S_IFMT = 0170000;

static const uint32_t S_IFREG = 0100000;


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

static const int32_t STDOUT_FILENO = 1;

static const int32_t STDERR_FILENO = 2;


// --- module file_utils ---

typedef struct file_utils_Reader_ file_utils_Reader;


typedef struct file_utils_Writer_ file_utils_Writer;

struct file_utils_Reader_ {
   void* region;
   uint32_t size;
   int32_t errno;
};

struct file_utils_Writer_ {
   char msg[256];
};

static bool file_utils_Reader_open(file_utils_Reader* file, const char* filename);
static void file_utils_Reader_close(file_utils_Reader* file);
static bool file_utils_Reader_isOpen(const file_utils_Reader* file);
static const uint8_t* file_utils_Reader_data(file_utils_Reader* file);
static const char* file_utils_find_char(const char* full, char delim);
static int32_t file_utils_create_dir(const char* path, bool follow);
static int32_t file_utils_create_directory(const char* path);
static bool file_utils_Writer_write(file_utils_Writer* writer, const char* filename, const uint8_t* data, uint32_t len);

static uint8_t file_utils_empty;

static const int32_t file_utils_Err_not_a_file = 2001;

static bool file_utils_Reader_open(file_utils_Reader* file, const char* filename)
{
   file->region = NULL;
   file->size = 0;
   int32_t fd = open(filename, O_RDONLY);
   if (fd == -1) {
      file->errno = *__errno_location();
      return false;
   }
   struct stat statbuf;
   int32_t err = fstat(fd, &statbuf);
   if (err) {
      file->errno = *__errno_location();
      return false;
   }
   if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
      close(fd);
      file->errno = file_utils_Err_not_a_file;
      return false;
   }
   file->size = ((uint32_t)(statbuf.st_size));
   if (file->size == 0) {
      file->region = &file_utils_empty;
   } else {
      file->region = mmap(NULL, file->size, (PROT_READ | PROT_WRITE), (MAP_PRIVATE | MAP_POPULATE), fd, 0);
      if (file->region == ((void*)(MAP_FAILED))) {
         file->errno = *__errno_location();
         return false;
      }
   }
   close(fd);
   return true;
}

static void file_utils_Reader_close(file_utils_Reader* file)
{
   if (file->region) {
      munmap(file->region, file->size);
      file->region = NULL;
   }
}

static bool file_utils_Reader_isOpen(const file_utils_Reader* file)
{
   return file->region != NULL;
}

static const uint8_t* file_utils_Reader_data(file_utils_Reader* file)
{
   return ((uint8_t*)(file->region));
}

static const char* file_utils_find_char(const char* full, char delim)
{
   while (1) {
      char c = *full;
      if (c == delim) return full;

      if (c == 0) break;

      full++;
   }
   return NULL;
}

static int32_t file_utils_create_dir(const char* path, bool follow)
{
   int32_t err = mkdir(path, 0777);
   if ((err && *__errno_location() != EEXIST)) return -1;

   if (!follow) return 0;

   int32_t fd = openat(AT_FDCWD, path, ((((O_RDONLY | O_NOCTTY) | O_NONBLOCK) | O_NOFOLLOW) | O_DIRECTORY));
   if (fd == -1) return *__errno_location();

   err = fchdir(fd);
   if (err == -1) return -1;

   close(fd);
   return 0;
}

static int32_t file_utils_create_directory(const char* path)
{
   int32_t fd = openat(AT_FDCWD, ".", O_RDONLY);
   if (fd == -1) return *__errno_location();

   char tmp[128];
   const char* cp = path;
   int32_t err = 0;
   while (*cp) {
      const char* slash = file_utils_find_char(cp, '/');
      if (slash) {
         size_t len = ((size_t)(slash - cp));
         memcpy(tmp, cp, len);
         tmp[len] = 0;
         cp = slash + 1;
         err = file_utils_create_dir(tmp, true);
         if (err != 0) break;

      } else {
         err = file_utils_create_dir(cp, false);
         break;
      }
   }
   int32_t errno_ = 0;
   if (err) errno_ = *__errno_location();
   fchdir(fd);
   close(fd);
   return errno_;
}

static bool file_utils_Writer_write(file_utils_Writer* writer, const char* filename, const uint8_t* data, uint32_t len)
{
   writer->msg[0] = 0;
   int32_t fd = open(filename, ((O_CREAT | O_WRONLY) | O_TRUNC), 0660);
   if (fd == -1) {
      perror("open");
      return false;
   }
   int64_t written = write(fd, data, len);
   if (written != len) {
      perror("write");
      return false;
   }
   close(fd);
   return true;
}


// --- module constants ---



static const uint32_t constants_MaxIdentifierLen = 31;

static const uint32_t constants_MaxFeatureName = 31;

static const uint32_t constants_MaxFeatureDepth = 6;

static const uint32_t constants_MaxErrorMsgLen = 31;

static const uint32_t constants_Max_path = 512;

static const uint32_t constants_Max_open_files = 200;

static const char* constants_output_dir = "c2_output";

static const char* constants_recipe_name = "recipe.txt";

static const char* constants_manifest_name = "manifest.yaml";


// --- module yaml ---

typedef struct yaml_Node_ yaml_Node;
typedef struct yaml_StackLevel_ yaml_StackLevel;
typedef struct yaml_Data_ yaml_Data;


typedef struct yaml_Iter_ yaml_Iter;

typedef struct yaml_Location_ yaml_Location;
typedef struct yaml_Token_ yaml_Token;
typedef struct yaml_Tokenizer_ yaml_Tokenizer;

typedef struct yaml_Parser_ yaml_Parser;

typedef enum {
   yaml_NodeKind_Unknown,
   yaml_NodeKind_Scalar,
   yaml_NodeKind_Map,
   yaml_NodeKind_Sequence,
} __attribute__((packed)) yaml_NodeKind;

struct yaml_Node_ {
   yaml_NodeKind kind;
   uint32_t next_idx;
   uint32_t name_idx;
   union {
      uint32_t text_idx;
      uint32_t child_idx;
   };
};

struct yaml_StackLevel_ {
   int32_t indent;
   yaml_Node* node;
   yaml_Node* last_child;
};

struct yaml_Data_ {
   char* text;
   uint32_t text_size;
   char* text_cur;
   yaml_Node* nodes;
   uint32_t nodes_count;
   yaml_Node* nodes_cur;
   yaml_StackLevel* stack;
};

struct yaml_Iter_ {
   const void* data;
   const yaml_Node* node;
};

struct yaml_Location_ {
   uint32_t line;
   uint32_t column;
};

typedef enum {
   yaml_TokenKind_None,
   yaml_TokenKind_Plain_Scalar,
   yaml_TokenKind_Single_Quoted_Scalar,
   yaml_TokenKind_Double_Quoted_Scalar,
   yaml_TokenKind_Colon,
   yaml_TokenKind_Dash,
   yaml_TokenKind_Indent,
   yaml_TokenKind_Dedent,
   yaml_TokenKind_Doc_Start,
   yaml_TokenKind_Doc_End,
   yaml_TokenKind_Directive,
   yaml_TokenKind_Eof,
   yaml_TokenKind_Error,
} __attribute__((packed)) yaml_TokenKind;

struct yaml_Token_ {
   yaml_Location loc;
   yaml_TokenKind kind;
   bool same_line;
   union {
      const char* error_msg;
      uint32_t text_idx;
      int32_t indent;
   };
};

struct yaml_Tokenizer_ {
   const char* cur;
   yaml_Location loc;
   const char* input_start;
   char* error_msg;
   int32_t cur_indent;
   bool same_line;
   yaml_Data* data;
   yaml_Token next;
};

struct yaml_Parser_ {
   yaml_Token token;
   yaml_Tokenizer tokenizer;
   int32_t cur_indent;
   bool doc_started;
   bool in_document;
   yaml_StackLevel stack[8];
   uint32_t stack_size;
   yaml_Data data;
   __jmp_buf_tag jmp_err;
   char message[256];
};

static void yaml_Data_init(yaml_Data* d, uint32_t text_size, uint32_t nodes_count, yaml_StackLevel* stack);
static void yaml_Data_destroy(yaml_Data* d);
static void yaml_Data_resize_nodes(yaml_Data* d);
static void yaml_Data_resize_text(yaml_Data* d);
static yaml_Node* yaml_Data_add_node(yaml_Data* d, yaml_NodeKind kind, uint32_t name_idx);
static uint32_t yaml_Data_node2idx(const yaml_Data* d, const yaml_Node* n);
static uint32_t yaml_Data_add_text(yaml_Data* d, const char* text, uint32_t len);
static void yaml_Parser_dump(const yaml_Parser* p, bool verbose);
static void yaml_Data_dump(const yaml_Data* d, bool verbose);
static yaml_Node* yaml_Data_idx2node(const yaml_Data* d, uint32_t idx);
static void yaml_Data_dump_node(const yaml_Data* d, const yaml_Node* n, int32_t indent);
static bool yaml_Node_isMap(const yaml_Node* n);
static bool yaml_Node_isSequence(const yaml_Node* n);
static bool yaml_Node_isScalar(const yaml_Node* n);
static const yaml_Node* yaml_Parser_getRoot(const yaml_Parser* p);
static const yaml_Node* yaml_Parser_findNode(const yaml_Parser* p, const char* path);
static const yaml_Node* yaml_Data_findNode(const yaml_Data* d, const char* path);
static const yaml_Node* yaml_Data_findChildNode(const yaml_Data* d, const char* path, uint32_t next);
static yaml_Iter yaml_Parser_getNodeChildIter(const yaml_Parser* p, const yaml_Node* n);
static void yaml_Iter_next(yaml_Iter* iter);
static bool yaml_Iter_done(const yaml_Iter* iter);
static const char* yaml_Iter_getName(const yaml_Iter* iter);
static const char* yaml_Iter_getValue(const yaml_Iter* iter);
static yaml_Iter yaml_Iter_getChildIter(yaml_Iter* parent);
static const char* yaml_starts_with(const char* full, const char* start);
static const char* yaml_Location_str(const yaml_Location* loc);
static const char* yaml_Token_str(const yaml_Token* tok);
static void yaml_Tokenizer_init(yaml_Tokenizer* t, const char* input, yaml_Data* d, char* error_msg);
static void yaml_Tokenizer_lex(yaml_Tokenizer* t, yaml_Token* result);
static yaml_Token* yaml_Tokenizer_lex_next(yaml_Tokenizer* t);
static bool yaml_Tokenizer_lex_indent(yaml_Tokenizer* t, yaml_Token* result);
static void yaml_Tokenizer_lex_comment(yaml_Tokenizer* t);
static void yaml_Tokenizer_lex_directive(yaml_Tokenizer* t, yaml_Token* result);
static void yaml_Tokenizer_lex_quoted_string(yaml_Tokenizer* t, yaml_Token* result, char delim);
static bool yaml_is_string(char c);
static void yaml_Tokenizer_lex_string(yaml_Tokenizer* t, yaml_Token* result);
static void yaml_Tokenizer_error(yaml_Tokenizer* t, yaml_Token* result);
static yaml_Parser* yaml_Parser_create(void);
static void yaml_Parser_destroy(yaml_Parser* p);
static bool yaml_Parser_parse(yaml_Parser* p, const char* input);
static const char* yaml_Parser_getMessage(const yaml_Parser* p);
static void yaml_Parser_error(yaml_Parser* p, const char* format, ...);
static void yaml_Parser_consumeToken(yaml_Parser* p);
static void yaml_Parser_expectAndConsume(yaml_Parser* p, yaml_TokenKind kind);
static void yaml_Parser_parse_doc(yaml_Parser* p);
static void yaml_Parser_parse_node(yaml_Parser* p);
static void yaml_Parser_parse_value(yaml_Parser* p);
static void yaml_Parser_parse_node_or_value(yaml_Parser* p);
static void yaml_Parser_doc_start(yaml_Parser* p);
static void yaml_Parser_doc_end(yaml_Parser* p);
static void yaml_Parser_add_scalar_value(yaml_Parser* p, uint32_t value_idx);
static void yaml_Parser_pop(yaml_Parser* p);
static void yaml_Parser_push_root(yaml_Parser* p);
static void yaml_Parser_push_node(yaml_Parser* p, yaml_Node* n, yaml_NodeKind parent_kind, int32_t indent);

static const uint32_t yaml_MaxDepth = 8;

static const char* yaml_node_names[] = { "UNK", "SCA", "MAP", "SEQ" };

static const char* yaml_token_names[] = {
   "none",
   "scalar",
   "'scalar'",
   "\"scalar\"",
   ":",
   "-",
   "indent",
   "dedent",
   "---",
   "...",
   "%",
   "eof",
   "error"
};

static const uint32_t yaml_MaxDiag = 256;

static void yaml_Data_init(yaml_Data* d, uint32_t text_size, uint32_t nodes_count, yaml_StackLevel* stack)
{
   d->text = malloc(text_size);
   d->text_size = text_size;
   d->text_cur = d->text + 1;
   d->text[0] = 0;
   d->nodes = malloc(nodes_count * sizeof(yaml_Node));
   d->nodes_count = nodes_count;
   d->nodes_cur = &d->nodes[1];
   memset(&d->nodes[0], 0, sizeof(yaml_Node));
   d->stack = stack;
}

static void yaml_Data_destroy(yaml_Data* d)
{
   free(d->text);
   free(d->nodes);
}

static void yaml_Data_resize_nodes(yaml_Data* d)
{
   uint32_t idx = ((uint32_t)(d->nodes_cur - d->nodes));
   d->nodes_count *= 2;
   yaml_Node* nodes2 = malloc(d->nodes_count * sizeof(yaml_Node));
   memcpy(nodes2, d->nodes, idx * sizeof(yaml_Node));
   for (uint32_t i = 0; i < yaml_MaxDepth; i++) {
      yaml_StackLevel* sl = &d->stack[i];
      if (sl->node) {
         uint32_t node_idx = ((uint32_t)(sl->node - d->nodes));
         sl->node = &nodes2[node_idx];
      }
      if (sl->last_child) {
         uint32_t last_child_idx = ((uint32_t)(sl->last_child - d->nodes));
         sl->last_child = &nodes2[last_child_idx];
      }
   }
   free(d->nodes);
   d->nodes = nodes2;
   d->nodes_cur = &d->nodes[idx];
}

static void yaml_Data_resize_text(yaml_Data* d)
{
   uint32_t idx = ((uint32_t)(d->text_cur - d->text));
   d->text_size *= 2;
   char* text2 = malloc(d->text_size);
   memcpy(text2, d->text, idx + 1);
   free(d->text);
   d->text = text2;
   d->text_cur = &d->text[idx];
}

static yaml_Node* yaml_Data_add_node(yaml_Data* d, yaml_NodeKind kind, uint32_t name_idx)
{
   uint32_t idx = ((uint32_t)(d->nodes_cur - d->nodes));
   if (idx >= d->nodes_count - 1) yaml_Data_resize_nodes(d);
   yaml_Node* result = d->nodes_cur;
   d->nodes_cur++;
   result->kind = kind;
   result->next_idx = 0;
   result->name_idx = name_idx;
   result->child_idx = 0;
   return result;
}

static uint32_t yaml_Data_node2idx(const yaml_Data* d, const yaml_Node* n)
{
   return ((uint32_t)(n - d->nodes));
}

static uint32_t yaml_Data_add_text(yaml_Data* d, const char* text, uint32_t len)
{
   uint32_t idx = ((uint32_t)(d->text_cur - d->text));
   while (idx + len + 1 >= d->text_size) yaml_Data_resize_text(d);
   memcpy(d->text_cur, text, len);
   d->text_cur[len] = 0;
   d->text_cur += len + 1;
   return idx;
}

static void yaml_Parser_dump(const yaml_Parser* p, bool verbose)
{
   yaml_Data_dump(&p->data, verbose);
}

static void yaml_Data_dump(const yaml_Data* d, bool verbose)
{
   uint32_t node_count = ((uint32_t)(d->nodes_cur - d->nodes));
   if (verbose) {
      printf("Text %d/%u\n", ((uint32_t)(d->text_cur - d->text)), d->text_size);
      const char* cp = d->text + 1;
      while (cp < d->text_cur) {
         uint32_t len = ((uint32_t)(strlen(cp)));
         uint32_t offset = ((uint32_t)(cp - d->text));
         printf("  [%3u] %s\n", offset, cp);
         cp += len + 1;
      }
      printf("Nodes %u/%u\n", node_count, d->nodes_count);
      for (uint32_t i = 1; i < node_count; i++) {
         const yaml_Node* n = &d->nodes[i];
         printf("  [%2u] %s  next %3u  name %3u  value/child %3u\n", i, yaml_node_names[n->kind], n->next_idx, n->name_idx, n->text_idx);
      }
   }
   if (node_count > 1) yaml_Data_dump_node(d, &d->nodes[1], 0);
}

static yaml_Node* yaml_Data_idx2node(const yaml_Data* d, uint32_t idx)
{
   return &d->nodes[idx];
}

static void yaml_Data_dump_node(const yaml_Data* d, const yaml_Node* n, int32_t indent)
{
   for (int32_t i = 0; i < indent; i++) printf("   ");
   printf("[%2u] %s", yaml_Data_node2idx(d, n), yaml_node_names[n->kind]);
   printf("  name: ");
   if (n->name_idx) printf("%s", &d->text[n->name_idx]);
   else printf("-");
   printf("  value: ");
   switch (n->kind) {
   case yaml_NodeKind_Unknown:
      printf("-\n");
      break;
   case yaml_NodeKind_Scalar:
      if (n->text_idx) printf("%s", &d->text[n->text_idx]);
      printf("\n");
      break;
   case yaml_NodeKind_Map:
      // fallthrough
   case yaml_NodeKind_Sequence:
      printf("-\n");
      if (n->child_idx) yaml_Data_dump_node(d, yaml_Data_idx2node(d, n->child_idx), indent + 1);
      break;
   }
   if (n->next_idx) {
      yaml_Data_dump_node(d, yaml_Data_idx2node(d, n->next_idx), indent);
   }
}

static bool yaml_Node_isMap(const yaml_Node* n)
{
   return n->kind == yaml_NodeKind_Map;
}

static bool yaml_Node_isSequence(const yaml_Node* n)
{
   return n->kind == yaml_NodeKind_Sequence;
}

static bool yaml_Node_isScalar(const yaml_Node* n)
{
   return n->kind == yaml_NodeKind_Scalar;
}

static const yaml_Node* yaml_Parser_getRoot(const yaml_Parser* p)
{
   uint32_t node_count = ((uint32_t)(p->data.nodes_cur - p->data.nodes)) - 1;
   if (node_count == 0) return NULL;

   return &p->data.nodes[1];
}

static const yaml_Node* yaml_Parser_findNode(const yaml_Parser* p, const char* path)
{
   return yaml_Data_findNode(&p->data, path);
}

static const yaml_Node* yaml_Data_findNode(const yaml_Data* d, const char* path)
{
   uint32_t node_count = ((uint32_t)(d->nodes_cur - d->nodes)) - 1;
   if (node_count == 0) return NULL;

   const yaml_Node* root = &d->nodes[1];
   if (root->kind == yaml_NodeKind_Sequence) return NULL;

   return yaml_Data_findChildNode(d, path, root->child_idx);
}

static const yaml_Node* yaml_Data_findChildNode(const yaml_Data* d, const char* path, uint32_t next)
{
   while (next) {
      const yaml_Node* node = yaml_Data_idx2node(d, next);
      if (node->name_idx) {
         const char* name = &d->text[node->name_idx];
         const char* rest = yaml_starts_with(path, name);
         if (rest) {
            path = rest;
            if (path[0] == 0) return node;

            if (node->kind == yaml_NodeKind_Sequence) return NULL;

            next = node->child_idx;
            continue;
         }
      }
      next = node->next_idx;
   }
   return NULL;
}

static yaml_Iter yaml_Parser_getNodeChildIter(const yaml_Parser* p, const yaml_Node* n)
{
   yaml_Iter iter = { .data = &p->data, .node = NULL };
   if ((n->kind != yaml_NodeKind_Scalar && n->child_idx)) {
      iter.node = yaml_Data_idx2node(&p->data, n->child_idx);
   }
   return iter;
}

static void yaml_Iter_next(yaml_Iter* iter)
{
   const yaml_Data* d = ((yaml_Data*)(iter->data));
   if (iter->node) {
      if (iter->node->next_idx) iter->node = yaml_Data_idx2node(d, iter->node->next_idx);
      else iter->node = NULL;
   }
}

static bool yaml_Iter_done(const yaml_Iter* iter)
{
   return iter->node == NULL;
}

static const char* yaml_Iter_getName(const yaml_Iter* iter)
{
   const yaml_Data* d = ((yaml_Data*)(iter->data));
   if (iter->node) return &d->text[iter->node->name_idx];

   return NULL;
}

static const char* yaml_Iter_getValue(const yaml_Iter* iter)
{
   const yaml_Data* d = ((yaml_Data*)(iter->data));
   if ((iter->node && iter->node->kind == yaml_NodeKind_Scalar)) return &d->text[iter->node->text_idx];

   return NULL;
}

static yaml_Iter yaml_Iter_getChildIter(yaml_Iter* parent)
{
   yaml_Iter iter = { .data = parent->data, .node = NULL };
   if (parent->node == NULL) return iter;

   const yaml_Node* n = parent->node;
   if ((n->kind != yaml_NodeKind_Scalar && n->child_idx)) {
      const yaml_Data* d = ((yaml_Data*)(iter.data));
      iter.node = yaml_Data_idx2node(d, n->child_idx);
   }
   return iter;
}

static const char* yaml_starts_with(const char* full, const char* start)
{
   uint32_t len = ((uint32_t)(strlen(start)));
   if (strncmp(full, start, len) == 0) {
      full += len;
      if (full[0] == '.') return full + 1;

      if (full[0] == 0) return full;

   }
   return NULL;
}

static const char* yaml_Location_str(const yaml_Location* loc)
{
   static char msg[32];
   sprintf(msg, "at line %u:%u", loc->line, loc->column);
   return msg;
}

static const char* yaml_Token_str(const yaml_Token* tok)
{
   return yaml_token_names[tok->kind];
}

static void yaml_Tokenizer_init(yaml_Tokenizer* t, const char* input, yaml_Data* d, char* error_msg)
{
   memset(t, 0, sizeof(yaml_Tokenizer));
   t->cur = input;
   t->input_start = input;
   t->loc.line = 1;
   t->loc.column = 1;
   t->error_msg = error_msg;
   t->data = d;
   t->next.kind = yaml_TokenKind_None;
}

static void yaml_Tokenizer_lex(yaml_Tokenizer* t, yaml_Token* result)
{
   if (t->next.kind != yaml_TokenKind_None) {
      memcpy(result, &t->next, sizeof(yaml_Token));
      t->next.kind = yaml_TokenKind_None;
      return;
   }
   result->same_line = t->same_line;
   t->same_line = true;
   result->text_idx = 0;
   while (1) {
      if (((((t->loc.column == 1 && t->cur_indent) && *t->cur != ' ') && *t->cur != '\r') && *t->cur != '\n')) {
         result->loc = t->loc;
         result->kind = yaml_TokenKind_Dedent;
         result->indent = 0;
         t->cur_indent = 0;
         t->same_line = false;
         return;
      }
      switch (*t->cur) {
      case 0:
         result->loc = t->loc;
         result->kind = yaml_TokenKind_Eof;
         return;
      case '\t':
         sprintf(t->error_msg, "file contains TAB characters %s", yaml_Location_str(&t->loc));
         yaml_Tokenizer_error(t, result);
         return;
      case '\r':
         t->cur++;
         if (*t->cur != '\n') {
            sprintf(t->error_msg, "unexpected char 0x%02X %s", *t->cur, yaml_Location_str(&t->loc));
            yaml_Tokenizer_error(t, result);
            return;
         }
         // fallthrough
      case '\n':
         t->cur++;
         t->loc.line++;
         t->loc.column = 1;
         t->same_line = true;
         result->same_line = false;
         break;
      case ' ':
         if (t->loc.column == 1) {
            if (yaml_Tokenizer_lex_indent(t, result)) return;

            break;
         }
         t->cur++;
         t->loc.column++;
         break;
      case '"':
         yaml_Tokenizer_lex_quoted_string(t, result, '"');
         return;
      case '#':
         yaml_Tokenizer_lex_comment(t);
         break;
      case '%':
         yaml_Tokenizer_lex_directive(t, result);
         return;
      case '\'':
         yaml_Tokenizer_lex_quoted_string(t, result, '\'');
         return;
      case '-':
         if (((t->cur[1] == ' ' || t->cur[1] == '\r') || t->cur[1] == '\n')) {
            t->cur++;
            result->loc = t->loc;
            result->kind = yaml_TokenKind_Dash;
            t->loc.column++;
            return;
         }
         if (((t->loc.column == 1 && t->cur[1] == '-') && t->cur[2] == '-')) {
            t->cur += 3;
            result->loc = t->loc;
            result->kind = yaml_TokenKind_Doc_Start;
            t->loc.column += 3;
            return;
         }
         yaml_Tokenizer_lex_string(t, result);
         return;
      case '.':
         if (((t->loc.column == 1 && t->cur[1] == '.') && t->cur[2] == '.')) {
            result->loc = t->loc;
            result->kind = yaml_TokenKind_Doc_End;
            t->cur += 3;
            t->loc.column += 3;
            return;
         }
         yaml_Tokenizer_lex_string(t, result);
         return;
      case ':':
         t->cur++;
         result->loc = t->loc;
         result->kind = yaml_TokenKind_Colon;
         t->loc.column++;
         return;
      default:
         if (yaml_is_string(*t->cur)) {
            yaml_Tokenizer_lex_string(t, result);
            return;
         }
         sprintf(t->error_msg, "unhandled char 0x%02x (%c) %s", *t->cur, isprint(*t->cur) ? *t->cur : ' ', yaml_Location_str(&t->loc));
         yaml_Tokenizer_error(t, result);
         return;
      }
   }
}

static yaml_Token* yaml_Tokenizer_lex_next(yaml_Tokenizer* t)
{
   if (t->next.kind == yaml_TokenKind_None) yaml_Tokenizer_lex(t, &t->next);
   return &t->next;
}

static bool yaml_Tokenizer_lex_indent(yaml_Tokenizer* t, yaml_Token* result)
{
   const char* start = t->cur;
   while (*t->cur == ' ') t->cur++;
   int32_t indent = ((int32_t)(t->cur - start));
   result->loc = t->loc;
   t->loc.column += indent;
   if (t->cur_indent == indent) return false;

   if (t->cur_indent > indent) result->kind = yaml_TokenKind_Dedent;
   else result->kind = yaml_TokenKind_Indent;
   result->indent = indent;
   t->cur_indent = indent;
   return true;
}

static void yaml_Tokenizer_lex_comment(yaml_Tokenizer* t)
{
   const char* start = t->cur;
   t->cur++;
   while (1) {
      switch (*t->cur) {
      case 0:
         // fallthrough
      case '\r':
         // fallthrough
      case '\n':
         t->loc.column += (t->cur - start);
         return;
      default:
         t->cur++;
         break;
      }
   }
}

static void yaml_Tokenizer_lex_directive(yaml_Tokenizer* t, yaml_Token* result)
{
   t->cur++;
   const char* start = t->cur;
   uint32_t count;
   while (1) {
      switch (*t->cur) {
      case 0:
         // fallthrough
      case '\r':
         // fallthrough
      case '\n':
         goto out;
      default:
         t->cur++;
         break;
      }
   }
   out:
   count = ((uint32_t)(t->cur - start));
   t->error_msg[count] = 0;
   result->loc = t->loc;
   result->kind = yaml_TokenKind_Directive;
   result->text_idx = yaml_Data_add_text(t->data, start, count);
   t->loc.column += count + 1;
}

static void yaml_Tokenizer_lex_quoted_string(yaml_Tokenizer* t, yaml_Token* result, char delim)
{
   t->cur++;
   const char* start = t->cur;
   uint32_t count;
   while (1) {
      switch (*t->cur) {
      case 0:
         // fallthrough
      case '\r':
         // fallthrough
      case '\n':
         t->loc.column += (t->cur - start);
         sprintf(t->error_msg, "unterminated string %s", yaml_Location_str(&t->loc));
         yaml_Tokenizer_error(t, result);
         return;
      default:
         if (*t->cur == delim) goto out;

         t->cur++;
         break;
      }
   }
   out:
   count = ((uint32_t)(t->cur - start));
   t->cur++;
   result->loc = t->loc;
   result->kind = (delim == '"') ? yaml_TokenKind_Double_Quoted_Scalar : yaml_TokenKind_Single_Quoted_Scalar;
   result->text_idx = yaml_Data_add_text(t->data, start, count);
   t->loc.column += count + 2;
}

static bool yaml_is_string(char c)
{
   if (((((((isalpha(c) || isdigit(c)) || c == '_') || c == '-') || c == '.') || c == '/') || c == '~')) {
      return true;
   }
   return false;
}

static void yaml_Tokenizer_lex_string(yaml_Tokenizer* t, yaml_Token* result)
{
   const char* start = t->cur;
   t->cur++;
   while (1) {
      char c = *t->cur;
      if (yaml_is_string(c)) {
         t->cur++;
         continue;
      }
      if ((c == ' ' && yaml_is_string(t->cur[1]))) {
         t->cur += 2;
         continue;
      }
      break;
   }
   uint32_t count = ((uint32_t)(t->cur - start));
   result->loc = t->loc;
   result->kind = yaml_TokenKind_Plain_Scalar;
   result->text_idx = yaml_Data_add_text(t->data, start, count);
   t->loc.column += count;
}

static void yaml_Tokenizer_error(yaml_Tokenizer* t, yaml_Token* result)
{
   result->loc = t->loc;
   result->kind = yaml_TokenKind_Error;
   result->error_msg = t->error_msg;
}

static yaml_Parser* yaml_Parser_create(void)
{
   yaml_Parser* p = calloc(1, sizeof(yaml_Parser));
   yaml_Data_init(&p->data, 1024, 32, p->stack);
   return p;
}

static void yaml_Parser_destroy(yaml_Parser* p)
{
   yaml_Data_destroy(&p->data);
   free(p);
}

static bool yaml_Parser_parse(yaml_Parser* p, const char* input)
{
   yaml_Tokenizer_init(&p->tokenizer, input, &p->data, p->message);
   p->token.kind = yaml_TokenKind_None;
   int32_t res = setjmp(&p->jmp_err);
   if (res == 0) {
      yaml_Parser_consumeToken(p);
      while (p->token.kind != yaml_TokenKind_Eof) yaml_Parser_parse_doc(p);
   } else {
      return false;
   }
   return true;
}

static const char* yaml_Parser_getMessage(const yaml_Parser* p)
{
   return p->message;
}

static void yaml_Parser_error(yaml_Parser* p, const char* format, ...)
{
   va_list args;
   va_start(args, format);
   char* cp = p->message;
   cp += vsnprintf(cp, yaml_MaxDiag - 1, format, args);
   va_end(args);
   sprintf(cp, " %s", yaml_Location_str(&p->token.loc));
   longjmp(&p->jmp_err, 1);
}

static void yaml_Parser_consumeToken(yaml_Parser* p)
{
   yaml_Tokenizer_lex(&p->tokenizer, &p->token);
   if (p->token.kind == yaml_TokenKind_Error) longjmp(&p->jmp_err, 1);
}

static void yaml_Parser_expectAndConsume(yaml_Parser* p, yaml_TokenKind kind)
{
   if (p->token.kind != kind) {
      yaml_Parser_error(p, "expected '%s', got '%s'", yaml_token_names[kind], yaml_Token_str(&p->token));
   }
   yaml_Parser_consumeToken(p);
}

static void yaml_Parser_parse_doc(yaml_Parser* p)
{
   while (1) {
      switch (p->token.kind) {
      case yaml_TokenKind_Doc_Start:
         yaml_Parser_consumeToken(p);
         if (p->doc_started) yaml_Parser_doc_end(p);
         yaml_Parser_doc_start(p);
         break;
      case yaml_TokenKind_Doc_End:
         if ((!p->doc_started || !p->in_document)) {
            yaml_Parser_error(p, "END document without start");
         }
         yaml_Parser_consumeToken(p);
         yaml_Parser_doc_end(p);
         return;
      case yaml_TokenKind_Directive:
         yaml_Parser_consumeToken(p);
         break;
      case yaml_TokenKind_Eof:
         return;
      default:
         if (!p->doc_started) yaml_Parser_doc_start(p);
         yaml_Parser_parse_node(p);
         break;
      }
   }
}

static void yaml_Parser_parse_node(yaml_Parser* p)
{
   switch (p->token.kind) {
   case yaml_TokenKind_Plain_Scalar:
      // fallthrough
   case yaml_TokenKind_Single_Quoted_Scalar:
      // fallthrough
   case yaml_TokenKind_Double_Quoted_Scalar: {
      yaml_Node* n = yaml_Data_add_node(&p->data, yaml_NodeKind_Unknown, p->token.text_idx);
      yaml_Parser_push_node(p, n, yaml_NodeKind_Unknown, p->cur_indent);
      yaml_Parser_consumeToken(p);
      yaml_Parser_expectAndConsume(p, yaml_TokenKind_Colon);
      yaml_Parser_parse_value(p);
      break;
   }
   case yaml_TokenKind_Dash: {
      yaml_Parser_consumeToken(p);
      yaml_Node* n = yaml_Data_add_node(&p->data, yaml_NodeKind_Unknown, 0);
      yaml_Parser_push_node(p, n, yaml_NodeKind_Sequence, p->cur_indent + 1);
      yaml_Parser_parse_node_or_value(p);
      break;
   }
   case yaml_TokenKind_Indent:
      p->cur_indent = p->token.indent;
      yaml_Parser_consumeToken(p);
      break;
   case yaml_TokenKind_Dedent:
      p->cur_indent = p->token.indent;
      yaml_Parser_consumeToken(p);
      yaml_Parser_pop(p);
      break;
   case yaml_TokenKind_Doc_Start:
      // fallthrough
   case yaml_TokenKind_Doc_End:
      break;
   default:
      yaml_Parser_error(p, "%s() unhandled token '%s'", "parse_node", yaml_Token_str(&p->token));
      break;
   }
}

static void yaml_Parser_parse_value(yaml_Parser* p)
{
   switch (p->token.kind) {
   case yaml_TokenKind_Plain_Scalar:
      // fallthrough
   case yaml_TokenKind_Single_Quoted_Scalar:
      // fallthrough
   case yaml_TokenKind_Double_Quoted_Scalar:
      if (p->token.same_line) {
         yaml_Parser_add_scalar_value(p, p->token.text_idx);
         yaml_Parser_consumeToken(p);
      } else {
         assert(0);
      }
      return;
   case yaml_TokenKind_Dash: {
      yaml_Parser_consumeToken(p);
      yaml_Node* n = yaml_Data_add_node(&p->data, yaml_NodeKind_Unknown, 0);
      yaml_Parser_push_node(p, n, yaml_NodeKind_Sequence, p->cur_indent + 1);
      yaml_Parser_parse_node_or_value(p);
      return;
   }
   case yaml_TokenKind_Indent:
      p->cur_indent = p->token.indent;
      yaml_Parser_consumeToken(p);
      yaml_Parser_parse_node(p);
      return;
   case yaml_TokenKind_Dedent:
      p->cur_indent = p->token.indent;
      yaml_Parser_consumeToken(p);
      yaml_Parser_pop(p);
      return;
   case yaml_TokenKind_Doc_Start:
      // fallthrough
   case yaml_TokenKind_Doc_End:
      return;
   case yaml_TokenKind_Eof:
      yaml_Parser_add_scalar_value(p, 0);
      return;
   default:
      yaml_Parser_error(p, "%s() unhandled token '%s'", "parse_value", yaml_Token_str(&p->token));
      break;
   }
}

static void yaml_Parser_parse_node_or_value(yaml_Parser* p)
{
   switch (p->token.kind) {
   case yaml_TokenKind_Plain_Scalar:
      // fallthrough
   case yaml_TokenKind_Single_Quoted_Scalar:
      // fallthrough
   case yaml_TokenKind_Double_Quoted_Scalar: {
      yaml_Token* next = yaml_Tokenizer_lex_next(&p->tokenizer);
      if (next->kind == yaml_TokenKind_Colon) {
         p->cur_indent += 2;
         p->tokenizer.cur_indent += 2;
         yaml_Parser_parse_node(p);
         return;
      }
      break;
   }
   default:
      break;
   }
   yaml_Parser_parse_value(p);
}

static void yaml_Parser_doc_start(yaml_Parser* p)
{
   yaml_Parser_push_root(p);
   p->doc_started = true;
   p->in_document = true;
}

static void yaml_Parser_doc_end(yaml_Parser* p)
{
   p->cur_indent = -1;
   if ((p->stack_size == 1 && p->stack[0].node->kind == yaml_NodeKind_Unknown)) {
      p->stack[0].node->kind = yaml_NodeKind_Map;
   }
   yaml_Parser_pop(p);
   p->cur_indent = 0;
   p->in_document = false;
}

static void yaml_Parser_add_scalar_value(yaml_Parser* p, uint32_t value_idx)
{
   yaml_StackLevel* top = &p->stack[p->stack_size - 1];
   yaml_Node* n = top->node;
   if (n->kind != yaml_NodeKind_Unknown) {
      yaml_Parser_error(p, "%s() cannot add scalar to node", "add_scalar_value");
   }
   n->kind = yaml_NodeKind_Scalar;
   n->text_idx = value_idx;
}

static void yaml_Parser_pop(yaml_Parser* p)
{
   int32_t indent = p->cur_indent;
   while (1) {
      yaml_StackLevel* top = &p->stack[p->stack_size - 1];
      if (top->indent <= indent) break;

      if (p->stack_size >= 1) {
         yaml_StackLevel* prev = &p->stack[p->stack_size - 2];
         prev->last_child = top->node;
      }
      if (top->node->kind == yaml_NodeKind_Unknown) top->node->kind = yaml_NodeKind_Scalar;
      top->indent = 0;
      top->node = NULL;
      top->last_child = NULL;
      p->stack_size--;
   }
}

static void yaml_Parser_push_root(yaml_Parser* p)
{
   yaml_Node* root = yaml_Data_add_node(&p->data, yaml_NodeKind_Unknown, 0);
   yaml_StackLevel* top = &p->stack[0];
   if (p->stack_size) {
      top->node->next_idx = yaml_Data_node2idx(&p->data, root);
   }
   top->node = root;
   top->indent = -1;
   top->last_child = NULL;
   p->stack_size = 1;
}

static void yaml_Parser_push_node(yaml_Parser* p, yaml_Node* n, yaml_NodeKind parent_kind, int32_t indent)
{
   assert(p->stack_size);
   uint32_t n_idx = yaml_Data_node2idx(&p->data, n);
   yaml_StackLevel* top = &p->stack[p->stack_size - 1];
   if (indent < top->indent) {
      assert(indent + 1 == top->indent);
      yaml_Parser_pop(p);
      top = &p->stack[p->stack_size - 1];
   }
   if (top->indent == indent) {
      if (top->node) {
         if (top->node->kind == yaml_NodeKind_Unknown) top->node->kind = yaml_NodeKind_Scalar;
         top->node->next_idx = n_idx;
      }
      top->last_child = NULL;
   } else {
      assert(p->stack_size + 1 < yaml_MaxDepth);
      assert(indent > top->indent);
      yaml_Node* parent = top->node;
      if (parent->kind == yaml_NodeKind_Unknown) {
         if (parent_kind == yaml_NodeKind_Unknown) parent_kind = yaml_NodeKind_Map;
         parent->kind = parent_kind;
      }
      if (top->last_child) {
         top->last_child->next_idx = n_idx;
      } else {
         assert(parent->child_idx == 0);
         parent->child_idx = n_idx;
      }
      top->last_child = n;
      p->stack_size++;
      top = &p->stack[p->stack_size - 1];
   }
   top->indent = indent;
   top->node = n;
   yaml_StackLevel* prev = &p->stack[p->stack_size - 2];
   yaml_Node* parent = prev->node;
   if ((parent->kind != parent_kind && !((parent->kind == yaml_NodeKind_Map && parent_kind == yaml_NodeKind_Unknown)))) {
      if (parent->kind == yaml_NodeKind_Sequence) {
         yaml_Parser_error(p, "invalid scalar after sequence");
      } else {
         yaml_Parser_error(p, "invalid scalar after %s", yaml_node_names[parent->kind]);
      }
   }
}


// --- module color ---



static const char color_Black[] = "\033[0;30m";

static const char color_Red[] = "\033[0;31m";

static const char color_Green[] = "\033[0;32m";

static const char color_Yellow[] = "\033[0;33m";

static const char color_Blue[] = "\033[0;34m";

static const char color_Magenta[] = "\033[0;35m";

static const char color_Cyan[] = "\033[0;36m";

static const char color_Grey[] = "\033[0;37m";

static const char color_Darkgrey[] = "\033[01;30m";

static const char color_Bred[] = "\033[01;31m";

static const char color_Bgreen[] = "\033[01;32m";

static const char color_Byellow[] = "\033[01;33m";

static const char color_Bblue[] = "\033[01;34m";

static const char color_Bmagenta[] = "\033[01;35m";

static const char color_Bcyan[] = "\033[01;36m";

static const char color_White[] = "\033[01;37m";

static const char color_Normal[] = "\033[0m";


// --- module string_buffer ---

typedef struct string_buffer_Buf_ string_buffer_Buf;

struct string_buffer_Buf_ {
   uint32_t capacity;
   uint32_t size_;
   uint32_t indent_step;
   char* data_;
   bool colors;
   bool own;
};

static string_buffer_Buf* string_buffer_create(uint32_t capacity, bool colors, uint32_t indent_step);
static string_buffer_Buf* string_buffer_create_static(uint32_t capacity, bool colors, char* data);
static void string_buffer_Buf_free(string_buffer_Buf* buf);
static uint32_t string_buffer_Buf_size(const string_buffer_Buf* buf);
static const char* string_buffer_Buf_data(const string_buffer_Buf* buf);
static void string_buffer_Buf_clear(string_buffer_Buf* buf);
static void string_buffer_Buf_color(string_buffer_Buf* buf, const char* color);
static void string_buffer_Buf_add1(string_buffer_Buf* buf, char c);
static void string_buffer_Buf_add(string_buffer_Buf* buf, const char* text);
static void string_buffer_Buf_add2(string_buffer_Buf* buf, const char* text, uint32_t len);
static void string_buffer_Buf_add_line(string_buffer_Buf* buf, const char* text);
static void string_buffer_Buf_print(string_buffer_Buf* buf, const char* format, ...);
static void string_buffer_Buf_indent(string_buffer_Buf* buf, uint32_t indent);
static bool string_buffer_Buf_endsWith(const string_buffer_Buf* buf, char c);
static void string_buffer_Buf_resize(string_buffer_Buf* buf, uint32_t capacity);

static string_buffer_Buf* string_buffer_create(uint32_t capacity, bool colors, uint32_t indent_step)
{
   string_buffer_Buf* buf = malloc(sizeof(string_buffer_Buf));
   buf->capacity = capacity;
   buf->size_ = 0;
   buf->indent_step = indent_step;
   buf->data_ = malloc(capacity);
   buf->colors = colors;
   buf->own = true;
   return buf;
}

static string_buffer_Buf* string_buffer_create_static(uint32_t capacity, bool colors, char* data)
{
   string_buffer_Buf* buf = malloc(sizeof(string_buffer_Buf));
   buf->capacity = capacity;
   buf->size_ = 0;
   buf->data_ = data;
   buf->colors = colors;
   buf->own = false;
   return buf;
}

static void string_buffer_Buf_free(string_buffer_Buf* buf)
{
   if (buf->own) free(buf->data_);
   free(buf);
}

static uint32_t string_buffer_Buf_size(const string_buffer_Buf* buf)
{
   return buf->size_;
}

static const char* string_buffer_Buf_data(const string_buffer_Buf* buf)
{
   return buf->data_;
}

static void string_buffer_Buf_clear(string_buffer_Buf* buf)
{
   buf->size_ = 0;
}

static void string_buffer_Buf_color(string_buffer_Buf* buf, const char* color)
{
   if (!buf->colors) return;

   uint32_t len = ((uint32_t)(strlen(color)));
   string_buffer_Buf_add2(buf, color, len);
}

static void string_buffer_Buf_add1(string_buffer_Buf* buf, char c)
{
   if (buf->size_ + 2 > buf->capacity) {
      uint32_t new_cap = buf->capacity * 2;
      while (buf->size_ + 2 > new_cap) new_cap *= 2;
      string_buffer_Buf_resize(buf, new_cap);
   }
   buf->data_[buf->size_] = c;
   buf->size_ += 1;
   buf->data_[buf->size_] = 0;
}

static void string_buffer_Buf_add(string_buffer_Buf* buf, const char* text)
{
   uint32_t len = ((uint32_t)(strlen(text)));
   string_buffer_Buf_add2(buf, text, len);
}

static void string_buffer_Buf_add2(string_buffer_Buf* buf, const char* text, uint32_t len)
{
   if (buf->size_ + len + 1 > buf->capacity) {
      uint32_t new_cap = buf->capacity * 2;
      while (buf->size_ + len + 1 > new_cap) new_cap *= 2;
      string_buffer_Buf_resize(buf, new_cap);
   }
   memcpy(&buf->data_[buf->size_], text, len);
   buf->size_ += len;
   buf->data_[buf->size_] = 0;
}

static void string_buffer_Buf_add_line(string_buffer_Buf* buf, const char* text)
{
   const char* end = text;
   while (*end) {
      if ((*end == '\n' || *end == '\r')) break;

      end++;
      if ((end - text) >= 256) break;

   }
   uint32_t len = ((uint32_t)(end - text));
   string_buffer_Buf_add2(buf, text, len);
}

static void string_buffer_Buf_print(string_buffer_Buf* buf, const char* format, ...)
{
   char tmp[256];
   va_list args;
   va_start(args, format);
   int32_t len = vsprintf(tmp, format, args);
   string_buffer_Buf_add2(buf, tmp, ((uint32_t)(len)));
   va_end(args);
}

static void string_buffer_Buf_indent(string_buffer_Buf* buf, uint32_t indent)
{
   indent *= buf->indent_step;
   if (buf->size_ + indent + 1 > buf->capacity) {
      string_buffer_Buf_resize(buf, buf->capacity * 2);
   }
   char* cur = buf->data_ + buf->size_;
   memset(cur, ' ', indent);
   cur[indent] = 0;
   buf->size_ += indent;
}

static bool string_buffer_Buf_endsWith(const string_buffer_Buf* buf, char c)
{
   if ((buf->size_ && buf->data_[buf->size_ - 1] == c)) return true;

   return false;
}

static void string_buffer_Buf_resize(string_buffer_Buf* buf, uint32_t capacity)
{
   buf->capacity = capacity;
   char* data2 = malloc(buf->capacity);
   memcpy(data2, buf->data_, buf->size_);
   free(buf->data_);
   buf->data_ = data2;
}


// --- module string_pool ---

typedef struct string_pool_Node_ string_pool_Node;
typedef struct string_pool_Pool_ string_pool_Pool;

typedef enum {
   string_pool_Color_Black,
   string_pool_Color_Red,
} __attribute__((packed)) string_pool_Color;

typedef enum {
   string_pool_Dir_Left,
   string_pool_Dir_Right,
} __attribute__((packed)) string_pool_Dir;

struct string_pool_Node_ {
   uint16_t child[2];
   uint32_t word_idx;
   uint16_t parent;
   string_pool_Color color;
};

struct string_pool_Pool_ {
   uint32_t data_size;
   uint32_t data_capacity;
   char* data;
   string_pool_Node* nodes;
   uint16_t node_capacity;
   uint16_t node_count;
   string_pool_Node* root;
   uint32_t num_adds;
   uint32_t total_size;
};

static string_pool_Pool* string_pool_create(uint32_t data_capacity, uint16_t node_capacity);
static void string_pool_Pool_free(string_pool_Pool* p);
static const char* string_pool_Pool_getStart(const string_pool_Pool* pool);
static const char* string_pool_Pool_idx2str(const string_pool_Pool* pool, uint32_t idx);
static string_pool_Node* string_pool_Pool_getChild(string_pool_Pool* pool, string_pool_Node* x, string_pool_Dir dir);
static uint16_t string_pool_Pool_toIndex(string_pool_Pool* pool, string_pool_Node* x);
static void string_pool_Pool_rotate(string_pool_Pool* pool, string_pool_Node* p, string_pool_Dir dir);
static void string_pool_Pool_balance(string_pool_Pool* pool, string_pool_Node* n, string_pool_Node* p);
static uint32_t string_pool_compare(const char* left, const char* right, size_t rlen);
static uint32_t string_pool_Pool_add(string_pool_Pool* pool, const char* text, size_t len, bool filter);
static uint32_t string_pool_Pool_addStr(string_pool_Pool* pool, const char* text, bool filter);
static void string_pool_Pool_resize_nodes(string_pool_Pool* p, uint16_t capacity);
static void string_pool_Pool_report(const string_pool_Pool* pool);

static string_pool_Pool* string_pool_create(uint32_t data_capacity, uint16_t node_capacity)
{
   string_pool_Pool* p = calloc(1, sizeof(string_pool_Pool));
   p->data_capacity = data_capacity;
   p->data = malloc(p->data_capacity);
   p->data[0] = 0;
   p->data_size = 1;
   string_pool_Pool_resize_nodes(p, node_capacity);
   p->node_count = 1;
   return p;
}

static void string_pool_Pool_free(string_pool_Pool* p)
{
   free(p->data);
   free(p->nodes);
   free(p);
}

static const char* string_pool_Pool_getStart(const string_pool_Pool* pool)
{
   return pool->data;
}

static const char* string_pool_Pool_idx2str(const string_pool_Pool* pool, uint32_t idx)
{
   return pool->data + idx;
}

static string_pool_Node* string_pool_Pool_getChild(string_pool_Pool* pool, string_pool_Node* x, string_pool_Dir dir)
{
   assert(x);
   uint16_t idx = x->child[dir];
   if (idx) return pool->nodes + idx;

   return NULL;
}

static uint16_t string_pool_Pool_toIndex(string_pool_Pool* pool, string_pool_Node* x)
{
   return ((uint16_t)(x - pool->nodes));
}

static void string_pool_Pool_rotate(string_pool_Pool* pool, string_pool_Node* p, string_pool_Dir dir)
{
   string_pool_Dir rdir = ((string_pool_Dir)(1 - dir));
   uint16_t g = p->parent;
   uint16_t s = p->child[rdir];
   uint16_t c = pool->nodes[s].child[dir];
   p->child[rdir] = c;
   uint16_t p_idx = string_pool_Pool_toIndex(pool, p);
   if (c) pool->nodes[c].parent = p_idx;
   pool->nodes[s].child[dir] = p_idx;
   p->parent = s;
   pool->nodes[s].parent = g;
   if (g) {
      pool->nodes[g].child[p_idx == pool->nodes[g].child[string_pool_Dir_Right] ? string_pool_Dir_Right : string_pool_Dir_Left] = s;
   } else {
      pool->root = &pool->nodes[s];
   }
}

static void string_pool_Pool_balance(string_pool_Pool* pool, string_pool_Node* n, string_pool_Node* p)
{
   n->color = string_pool_Color_Red;
   if (p == NULL) {
      pool->root = n;
      return;
   }
   string_pool_Node* g;
   string_pool_Dir dir;
   string_pool_Dir rdir;
   do {
      if (p->color == string_pool_Color_Black) return;

      if (p->parent == 0) goto Case_I4;

      g = pool->nodes + p->parent;
      dir = (p == (pool->nodes + g->child[string_pool_Dir_Right])) ? string_pool_Dir_Right : string_pool_Dir_Left;
      rdir = ((string_pool_Dir)(1 - dir));
      uint16_t u_idx = g->child[rdir];
      if (u_idx == 0) goto Case_I56;

      string_pool_Node* u = pool->nodes + u_idx;
      if (u->color == string_pool_Color_Black) goto Case_I56;

      p->color = string_pool_Color_Black;
      u->color = string_pool_Color_Black;
      g->color = string_pool_Color_Red;
      n = g;
      if (n->parent == 0) break;

      p = pool->nodes + n->parent;
   } while (1);
   return;
   Case_I4:
   p->color = string_pool_Color_Black;
   return;
   Case_I56:
   if (n == string_pool_Pool_getChild(pool, p, rdir)) {
      string_pool_Pool_rotate(pool, p, dir);
      n = p;
      p = pool->nodes + g->child[dir];
   }
   string_pool_Pool_rotate(pool, g, rdir);
   p->color = string_pool_Color_Black;
   g->color = string_pool_Color_Red;
   return;
}

static uint32_t string_pool_compare(const char* left, const char* right, size_t rlen)
{
   uint32_t i = 0;
   while (i < rlen) {
      char l = left[i];
      char r = right[i];
      char c = l - r;
      if (c < 0) return 1;

      if (c > 0) return 0;

      i++;
   }
   if (left[rlen] == 0) return 2;

   return 0;
}

static uint32_t string_pool_Pool_add(string_pool_Pool* pool, const char* text, size_t len, bool filter)
{
   pool->num_adds++;
   pool->total_size += len;
   if (filter) {
      string_pool_Node* parent;
      string_pool_Node* n = pool->root;
      while (n) {
         const char* word = pool->data + n->word_idx;
         switch (string_pool_compare(word, text, len)) {
         case 0:
            if (n->child[string_pool_Dir_Left]) {
               n = pool->nodes + n->child[string_pool_Dir_Left];
               continue;
            } else {
               n->child[string_pool_Dir_Left] = pool->node_count;
               goto after_loop;
            }
            break;
         case 1:
            if (n->child[string_pool_Dir_Right]) {
               n = pool->nodes + n->child[string_pool_Dir_Right];
               continue;
            } else {
               n->child[string_pool_Dir_Right] = pool->node_count;
               goto after_loop;
            }
            break;
         case 2:
            return n->word_idx;
         }
      }
      after_loop:
      parent = n;
      uint16_t parent_idx = ((uint16_t)(n ? ((uint16_t)(n - pool->nodes)) : 0));
      if (pool->node_count == pool->node_capacity) {
         string_pool_Pool_resize_nodes(pool, pool->node_capacity * 2);
         parent = pool->nodes + parent_idx;
      }
      n = pool->nodes + pool->node_count;
      n->parent = parent_idx;
      pool->node_count++;
      n->word_idx = pool->data_size;
      n->child[string_pool_Dir_Left] = 0;
      n->child[string_pool_Dir_Right] = 0;
      string_pool_Pool_balance(pool, n, parent);
   }
   if (pool->data_size + len + 1 > pool->data_capacity) {
      fprintf(stderr, "pool: overflow (%u bytes)\n", pool->data_capacity);
      abort();
   }
   uint32_t idx = pool->data_size;
   char* dest = pool->data + pool->data_size;
   memcpy(dest, text, len);
   dest[len] = 0;
   pool->data_size += len + 1;
   assert(pool->data_size < pool->data_capacity);
   return idx;
}

static uint32_t string_pool_Pool_addStr(string_pool_Pool* pool, const char* text, bool filter)
{
   return string_pool_Pool_add(pool, text, strlen(text), filter);
}

static void string_pool_Pool_resize_nodes(string_pool_Pool* p, uint16_t capacity)
{
   p->node_capacity = capacity;
   string_pool_Node* nodes = malloc(p->node_capacity * sizeof(string_pool_Node));
   if (p->nodes) {
      assert(p->root);
      uint32_t root_idx = ((uint32_t)(p->root - p->nodes));
      memcpy(nodes, p->nodes, p->node_count * sizeof(string_pool_Node));
      free(p->nodes);
      p->root = &nodes[root_idx];
   }
   p->nodes = nodes;
}

static void string_pool_Pool_report(const string_pool_Pool* pool)
{
   uint32_t index_used = pool->node_count * sizeof(string_pool_Node);
   uint32_t index_size = pool->node_capacity * sizeof(string_pool_Node);
   index_used = (index_used + 1023) / 1024;
   index_size = (index_size + 1023) / 1024;
   printf("pool: %u(%u) entries, data %u(%u)/%u bytes, index %u/%u Kb\n", pool->node_count - 1, pool->num_adds, pool->data_size, pool->total_size, pool->data_capacity, index_used, index_size);
}


// --- module utils ---


static uint64_t utils_now(void);
static bool utils_findProjectDir(void);
static bool utils_useColor(void);

static uint64_t utils_now(void)
{
   timeval tv;
   gettimeofday(&tv, NULL);
   uint64_t now64 = ((uint64_t)(tv.tv_sec));
   now64 *= 1000000;
   now64 += tv.tv_usec;
   return now64;
}

static bool utils_findProjectDir(void)
{
   char base_path[512];
   char rel_path[512];
   char* path_prefix = NULL;
   char* path = getcwd(base_path, constants_Max_path);
   char buffer[512];
   while (1) {
      path = getcwd(buffer, constants_Max_path);
      if (path == NULL) {
         perror("getcwd");
         return false;
      }
      struct stat buf;
      int32_t error = stat(constants_recipe_name, &buf);
      if (error) {
         if ((buffer[0] == '/' && buffer[1] == 0)) return false;

         if (*__errno_location() != ENOENT) {
            perror("stat");
            return false;
         }
      } else {
         if ((buf.st_mode & S_IFMT) == S_IFREG) {
            path_prefix = base_path + strlen(buffer);
            if (*path_prefix == '/') path_prefix++;
            return true;
         }
      }
      error = chdir("..");
      if (error) {
         perror("chdir");
         return false;
      }
      strcat(rel_path, "../");
   }
   return true;
}

static bool utils_useColor(void)
{
   return isatty(1);
}


// --- module warning_flags ---

typedef struct warning_flags_Flags_ warning_flags_Flags;

struct warning_flags_Flags_ {
   bool no_unused;
   bool no_unused_variable;
   bool no_unused_function;
   bool no_unused_parameter;
   bool no_unused_type;
   bool no_unused_module;
   bool no_unused_import;
   bool no_unused_public;
   bool no_unused_label;
   bool no_unused_enum_constant;
   bool are_errors;
};



// --- module quicksort ---


typedef bool (*quicksort_CompareFn)(void*, const void*, const void*);

static void quicksort_swap(uint8_t* item, uint8_t* other, size_t size);
static void quicksort_sort(void* items, size_t count, size_t item_size, quicksort_CompareFn is_less, void* arg);

static void quicksort_swap(uint8_t* item, uint8_t* other, size_t size)
{
   const uint8_t* end = item + size;
   while (item < end) {
      uint8_t tmp = *other;
      *other = *item;
      *item = tmp;
      other++;
      item++;
   }
}

static void quicksort_sort(void* items, size_t count, size_t item_size, quicksort_CompareFn is_less, void* arg)
{
   if (count <= 1) return;

   uint8_t* begin = items;
   uint8_t* end = begin + count * item_size;
   uint8_t* left = begin;
   uint8_t* pivot = begin + (count / 2) * item_size;
   uint8_t* right = end - item_size;
   do {
      while (is_less(arg, left, pivot)) left += item_size;
      while (is_less(arg, pivot, right)) right -= item_size;
      if (left < right) {
         quicksort_swap(left, right, item_size);
         if (left == pivot) {
            pivot = right;
         } else if (right == pivot) {
            pivot = left;
         }

      }
      if (left <= right) {
         left += item_size;
         right -= item_size;
      }
   } while (left <= right);
   if (right > begin) {
      size_t part_items = (right - begin + item_size) / item_size;
      quicksort_sort(begin, part_items, item_size, is_less, arg);
   }
   if (left < end) {
      size_t part_items = (end - left) / item_size;
      quicksort_sort(left, part_items, item_size, is_less, arg);
   }
}


// --- module ast_context ---

typedef struct ast_context_Block_ ast_context_Block;
typedef struct ast_context_Context_ ast_context_Context;

struct ast_context_Block_ {
   uint8_t* data;
   uint32_t size;
   ast_context_Block* next;
};

struct ast_context_Context_ {
   ast_context_Block* blk_head;
   ast_context_Block* blk_tail;
   uint32_t blk_size;
   uint32_t num_allocs;
};

static ast_context_Block* ast_context_Block_create(uint32_t blk_size);
static ast_context_Block* ast_context_Block_free(ast_context_Block* b);
static uint32_t ast_context_Block_count(const ast_context_Block* b);
static uint32_t ast_context_Block_get_total(const ast_context_Block* b);
static ast_context_Context* ast_context_create(uint32_t blk_size);
static void ast_context_Context_freeBlocks(ast_context_Context* c);
static void ast_context_Context_free(ast_context_Context* c);
static void ast_context_Context_init(ast_context_Context* c);
static void* ast_context_Context_alloc(ast_context_Context* c, uint32_t len);
static void ast_context_Context_report(const ast_context_Context* c);

static ast_context_Block* ast_context_Block_create(uint32_t blk_size)
{
   ast_context_Block* b = malloc(sizeof(ast_context_Block));
   b->data = malloc(blk_size);
   b->size = 0;
   b->next = NULL;
   return b;
}

static ast_context_Block* ast_context_Block_free(ast_context_Block* b)
{
   ast_context_Block* next = b->next;
   free(b->data);
   free(b);
   return next;
}

static uint32_t ast_context_Block_count(const ast_context_Block* b)
{
   uint32_t total = 0;
   const ast_context_Block* blk = b;
   while (blk) {
      total++;
      blk = blk->next;
   }
   return total;
}

static uint32_t ast_context_Block_get_total(const ast_context_Block* b)
{
   uint32_t total = 0;
   const ast_context_Block* blk = b;
   while (blk) {
      total += blk->size;
      blk = blk->next;
   }
   return total;
}

static ast_context_Context* ast_context_create(uint32_t blk_size)
{
   ast_context_Context* c = calloc(1, sizeof(ast_context_Context));
   c->blk_size = blk_size;
   ast_context_Context_init(c);
   return c;
}

static void ast_context_Context_freeBlocks(ast_context_Context* c)
{
   ast_context_Block* blk = c->blk_head;
   while (blk) blk = ast_context_Block_free(blk);
}

static void ast_context_Context_free(ast_context_Context* c)
{
   ast_context_Context_freeBlocks(c);
   free(c);
}

static void ast_context_Context_init(ast_context_Context* c)
{
   c->blk_head = ast_context_Block_create(c->blk_size);
   c->blk_tail = c->blk_head;
}

static void* ast_context_Context_alloc(ast_context_Context* c, uint32_t len)
{
   len = ((len + 7) & ~0x7);
   c->num_allocs++;
   ast_context_Block* last = c->blk_tail;
   if (last->size + len >= c->blk_size) {
      c->blk_tail = ast_context_Block_create(c->blk_size);
      last->next = c->blk_tail;
      last = last->next;
   }
   void* cur = last->data + last->size;
   last->size += len;
   return cur;
}

static void ast_context_Context_report(const ast_context_Context* c)
{
   uint32_t num = ast_context_Block_count(c->blk_head);
   uint32_t total = ast_context_Block_get_total(c->blk_head);
   uint32_t avg = 0;
   if (c->num_allocs) avg = total / c->num_allocs;
   printf("context: %u allocs (avg %u bytes), %u blocks (%u), total %u Kb (%u)\n", c->num_allocs, avg, num, c->blk_size, (total / 1024) + 1, total);
}


// --- module linked_list ---

typedef struct linked_list_Element_ linked_list_Element;

struct linked_list_Element_ {
   linked_list_Element* prev;
   linked_list_Element* next;
};

static void linked_list_Element_init(linked_list_Element* src);
static void linked_list_Element_addTail(linked_list_Element* src, linked_list_Element* item);
static void linked_list_Element_addFront(linked_list_Element* src, linked_list_Element* item);
static linked_list_Element* linked_list_Element_popFront(linked_list_Element* item);
static void linked_list_Element_remove(linked_list_Element* item);
static uint64_t linked_list_Element_size(const linked_list_Element* src);
static bool linked_list_Element_isEmpty(const linked_list_Element* src);
static void linked_list_Element_move(linked_list_Element* src, linked_list_Element* dest);

static void linked_list_Element_init(linked_list_Element* src)
{
   src->prev = src;
   src->next = src;
}

static void linked_list_Element_addTail(linked_list_Element* src, linked_list_Element* item)
{
   linked_list_Element* old_tail = src->prev;
   src->prev = item;
   item->next = src;
   item->prev = old_tail;
   old_tail->next = item;
}

static void linked_list_Element_addFront(linked_list_Element* src, linked_list_Element* item)
{
   linked_list_Element* old_head = src->next;
   old_head->prev = item;
   item->next = old_head;
   item->prev = src;
   src->next = item;
}

static linked_list_Element* linked_list_Element_popFront(linked_list_Element* item)
{
   linked_list_Element* node = item->next;
   linked_list_Element_remove(node);
   return node;
}

static void linked_list_Element_remove(linked_list_Element* item)
{
   linked_list_Element* prev = item->prev;
   linked_list_Element* next = item->next;
   prev->next = next;
   next->prev = prev;
}

static uint64_t linked_list_Element_size(const linked_list_Element* src)
{
   uint64_t count = 0;
   linked_list_Element* node = src->next;
   while (node != src) {
      count++;
      node = node->next;
   }
   return count;
}

static bool linked_list_Element_isEmpty(const linked_list_Element* src)
{
   return src->next == src;
}

static void linked_list_Element_move(linked_list_Element* src, linked_list_Element* dest)
{
   linked_list_Element* node = src->next;
   while (node != src) {
      linked_list_Element* tmp = node;
      node = node->next;
      linked_list_Element_remove(tmp);
      linked_list_Element_addTail(dest, tmp);
   }
}


// --- module src_loc ---

typedef struct src_loc_SrcRange_ src_loc_SrcRange;

typedef uint32_t src_loc_SrcLoc;

struct src_loc_SrcRange_ {
   src_loc_SrcLoc start;
   src_loc_SrcLoc end;
};



// --- module process_utils ---


static bool process_utils_doWIFSIGNALED(int32_t state);
static bool process_utils_doWIFEXITED(int32_t state);
static char process_utils_getWEXITSTATUS(int32_t state);
static void process_utils_child_error(int32_t fd, const char* msg);
static int32_t process_utils_run(const char* path, const char* cmd, const char* logfile);
static void process_utils_parseArgs(const char* cmd, const char* args, char** argv, uint32_t _arg3);
static const char* process_utils_find_bin(const char* name);
static int32_t process_utils_run_args(const char* path, const char* cmd, const char* logfile, const char* args);

static const uint32_t process_utils_MAX_ARG_LEN = 512;

static const uint32_t process_utils_MAX_ARGS = 16;

static bool process_utils_doWIFSIGNALED(int32_t state)
{
   return ((((state & 0x7f)) > 0 && ((state & 0x7f)) < 0x7f));
}

static bool process_utils_doWIFEXITED(int32_t state)
{
   return (((state & 0xff)) == 0);
}

static char process_utils_getWEXITSTATUS(int32_t state)
{
   return ((char)(((state >> 8) & 0xff)));
}

static void process_utils_child_error(int32_t fd, const char* msg)
{
   size_t len = strlen(msg);
   ssize_t written = write(fd, msg, len);
   if (written != ((ssize_t)(len))) perror("write");
   fsync(fd);
   fprintf(stderr, "[make] %s\n", msg);
   fflush(stderr);
   _exit(-1);
}

static int32_t process_utils_run(const char* path, const char* cmd, const char* logfile)
{
   int32_t error_pipe[2];
   if (pipe(error_pipe)) {
      fprintf(stderr, "pipe() failed: %s\n", strerror(*__errno_location()));
      return -1;
   }
   if (fcntl(error_pipe[0], F_SETFD, FD_CLOEXEC) != 0) {
      fprintf(stderr, "fcncl(FD_CLOEXEC() failed: %s\n", strerror(*__errno_location()));
      return -1;
   }
   if (fcntl(error_pipe[1], F_SETFD, FD_CLOEXEC) != 0) {
      fprintf(stderr, "fcncl(FD_CLOEXEC) failed: %s\n", strerror(*__errno_location()));
      return -1;
   }
   pid_t child_pid = fork();
   if (child_pid == 0) {
      char errmsg[256];
      if (close(error_pipe[0])) {
         perror("close(errpipe)");
      }
      char output[512];
      sprintf(output, "%s%s", path, logfile);
      fflush(stdout);
      close(STDOUT_FILENO);
      int32_t fdout = open(output, ((O_TRUNC | O_CREAT) | O_WRONLY), 0644);
      if (fdout == -1) {
         sprintf(errmsg, "cannot open output '%s': %s", output, strerror(*__errno_location()));
         process_utils_child_error(error_pipe[1], errmsg);
      }
      close(STDERR_FILENO);
      if (dup(STDOUT_FILENO) == -1) {
         sprintf(errmsg, "dup(): %s", strerror(*__errno_location()));
         process_utils_child_error(error_pipe[1], errmsg);
      }
      printf("current dir: %s\n", getcwd(NULL, 0));
      if (chdir(path) != 0) {
         sprintf(errmsg, "cannot change to dir '%s': %s", path, strerror(*__errno_location()));
         process_utils_child_error(error_pipe[1], errmsg);
      }
      printf("changing to dir: %s\n", path);
      assert(strlen(cmd) < process_utils_MAX_ARG_LEN);
      char self[513];
      strcpy(self, cmd);
      char* argv[2] = { self, NULL };
      printf("running command: %s\n", cmd);
      execv(cmd, argv);
      int32_t lasterr = *__errno_location();
      fprintf(stderr, "failed to start %s: %s\n", cmd, strerror(lasterr));
      sprintf(errmsg, "error starting %s: %s", cmd, strerror(lasterr));
      process_utils_child_error(error_pipe[1], errmsg);
      _exit(-1);
   } else {
      if (close(error_pipe[1])) {
      }
      char error[256];
      memset(error, 0, sizeof(error));
      ssize_t numread = read(error_pipe[0], error, sizeof(error) - 1);
      if (numread < 0) {
         fprintf(stderr, "Error reading pipe\n");
         return -1;
      } else error[numread] = 0;
      close(error_pipe[0]);
      if (numread != 0) {
         return -1;
      }
      int32_t state = 0;
      pid_t pid = waitpid(child_pid, &state, 0);
      if (pid == -1) {
         fprintf(stderr, "Error waiting for pid: %s\n", strerror(*__errno_location()));
         return -1;
      }
      if (process_utils_doWIFSIGNALED(state)) {
         return -1;
      }
      if (process_utils_doWIFEXITED(state)) {
         char exitcode = process_utils_getWEXITSTATUS(state);
         if (exitcode != 0) return -1;

      } else {
         return -1;
      }
   }
   return 0;
}

static void process_utils_parseArgs(const char* cmd, const char* args, char** argv, uint32_t _arg3)
{
   static char tmp[512];
   uint32_t argc = 0;
   argv[argc++] = ((char*)(cmd));
   size_t len = strlen(args) + 1;
   assert(len < process_utils_MAX_ARG_LEN);
   memcpy(tmp, args, len);
   char* token = strtok(tmp, " ");
   while (token) {
      argv[argc] = token;
      argc++;
      token = strtok(NULL, " ");
   }
   argv[argc] = NULL;
}

static const char* process_utils_find_bin(const char* name)
{
   static char result[512];
   struct stat statbuf;
   char* path = strdup(getenv("PATH"));
   char* s = path;
   char* p = NULL;
   do {
      p = strchr(s, ':');
      if (p != NULL) p[0] = 0;
      sprintf(result, "%s/%s", s, name);
      if (stat(result, &statbuf) == 0) {
         free(path);
         return result;
      }
      s = p + 1;
   } while (p != NULL);
   free(path);
   return NULL;
}

static int32_t process_utils_run_args(const char* path, const char* cmd, const char* logfile, const char* args)
{
   int32_t error_pipe[2];
   if (pipe(error_pipe)) {
      fprintf(stderr, "pipe() failed: %s\n", strerror(*__errno_location()));
      return -1;
   }
   if (fcntl(error_pipe[0], F_SETFD, FD_CLOEXEC) != 0) {
      fprintf(stderr, "fcncl(FD_CLOEXEC() failed: %s\n", strerror(*__errno_location()));
      return -1;
   }
   if (fcntl(error_pipe[1], F_SETFD, FD_CLOEXEC) != 0) {
      fprintf(stderr, "fcncl(FD_CLOEXEC) failed: %s\n", strerror(*__errno_location()));
      return -1;
   }
   pid_t child_pid = fork();
   if (child_pid == 0) {
      char errmsg[256];
      if (close(error_pipe[0])) {
         perror("close(errpipe)");
      }
      char output[512];
      sprintf(output, "%s%s", path, logfile);
      fflush(stdout);
      close(STDOUT_FILENO);
      int32_t fdout = open(output, ((O_TRUNC | O_CREAT) | O_WRONLY), 0644);
      if (fdout == -1) {
         sprintf(errmsg, "cannot open output '%s': %s", output, strerror(*__errno_location()));
         process_utils_child_error(error_pipe[1], errmsg);
      }
      close(STDERR_FILENO);
      if (dup(STDOUT_FILENO) == -1) {
         sprintf(errmsg, "dup(): %s", strerror(*__errno_location()));
         process_utils_child_error(error_pipe[1], errmsg);
      }
      printf("current dir: %s\n", getcwd(NULL, 0));
      if (chdir(path) != 0) {
         sprintf(errmsg, "cannot change to dir '%s': %s", path, strerror(*__errno_location()));
         process_utils_child_error(error_pipe[1], errmsg);
      }
      printf("changing to dir: %s\n", path);
      printf("running command: %s %s\n", cmd, args);
      const char* self = process_utils_find_bin(cmd);
      if (!self) {
         printf("command not found\n");
         _exit(EXIT_FAILURE);
      }
      char* argv[16];
      process_utils_parseArgs(self, args, argv, process_utils_MAX_ARGS);
      execv(self, argv);
      int32_t lasterr = *__errno_location();
      fprintf(stderr, "failed to start %s: %s\n", cmd, strerror(lasterr));
      sprintf(errmsg, "error starting %s: %s", cmd, strerror(lasterr));
      process_utils_child_error(error_pipe[1], errmsg);
      _exit(EXIT_FAILURE);
   } else {
      if (close(error_pipe[1])) {
      }
      char error[256];
      memset(error, 0, sizeof(error));
      ssize_t numread = read(error_pipe[0], error, sizeof(error) - 1);
      if (numread < 0) {
         fprintf(stderr, "Error reading pipe\n");
         return -1;
      } else error[numread] = 0;
      close(error_pipe[0]);
      if (numread != 0) {
         return -1;
      }
      int32_t state = 0;
      pid_t pid = waitpid(child_pid, &state, 0);
      if (pid == -1) {
         fprintf(stderr, "Error waiting for pid: %s\n", strerror(*__errno_location()));
         return -1;
      }
      if (process_utils_doWIFSIGNALED(state)) {
         return -1;
      }
      if (process_utils_doWIFEXITED(state)) {
         char exitcode = process_utils_getWEXITSTATUS(state);
         if (exitcode != 0) return -1;

      } else {
         return -1;
      }
   }
   return 0;
}


// --- module refs ---

typedef struct refs_MapFile_ refs_MapFile;
typedef struct refs_TagRange_ refs_TagRange;
typedef struct refs_Files_ refs_Files;
typedef struct refs_Tag_ refs_Tag;
typedef struct refs_Tags_ refs_Tags;
typedef struct refs_Loc_ refs_Loc;
typedef struct refs_Locs_ refs_Locs;
typedef struct refs_Symbols_ refs_Symbols;
typedef union refs_U64Loc_ refs_U64Loc;
typedef struct refs_CacheLoc_ refs_CacheLoc;
typedef struct refs_LocsCache_ refs_LocsCache;
typedef struct refs_RefSrc_ refs_RefSrc;
typedef struct refs_Dest_ refs_Dest;
typedef struct refs_Refs_ refs_Refs;
typedef struct refs_UsesInfo_ refs_UsesInfo;

struct refs_MapFile_ {
   void* map;
   size_t size;
};

struct refs_TagRange_ {
   uint32_t start;
   uint32_t end;
};

struct refs_Files_ {
   uint32_t total_size;
   uint32_t idx_count;
   uint32_t idx_cap;
   uint32_t names_len;
   uint32_t names_cap;
   uint32_t name_indexes[0];
};

typedef void (*refs_FileTagsVisitor)(void*, uint32_t, uint32_t, uint32_t);

struct refs_RefSrc_ {
   uint32_t line;
   uint16_t col;
   uint16_t len;
};

struct refs_Tag_ {
   refs_RefSrc src;
   uint32_t loc_idx;
};

struct refs_Tags_ {
   uint32_t total_size;
   uint32_t count;
   uint32_t capacity;
   refs_Tag tags[0];
};

struct refs_Loc_ {
   uint32_t line;
   uint16_t file_id;
   uint16_t col;
};

struct refs_Locs_ {
   uint32_t total_size;
   uint32_t count;
   uint32_t capacity;
   refs_Loc locs[0];
};

struct refs_Symbols_ {
   uint32_t total_size;
   uint32_t idx_count;
   uint32_t idx_cap;
   uint32_t names_len;
   uint32_t names_cap;
   uint32_t name_indexes[0];
};

union refs_U64Loc_ {
   refs_Loc loc;
   uint64_t num;
};

struct refs_CacheLoc_ {
   uint64_t loc;
   uint32_t idx;
};

struct refs_LocsCache_ {
   uint32_t total_size;
   uint32_t count;
   uint32_t capacity;
   refs_CacheLoc locs[0];
};

struct refs_Dest_ {
   const char* filename;
   uint32_t line;
   uint16_t col;
};

struct refs_Refs_ {
   refs_Files* files;
   refs_Tags* tags;
   refs_Locs* locs;
   refs_Symbols* symbols;
   uint32_t cur_file_idx;
   refs_MapFile file;
   const char* dest_file_ptr;
   uint16_t dest_file_idx;
   refs_LocsCache* locs_cache;
};

typedef void (*refs_RefUsesFn)(void*, const refs_Dest*);

struct refs_UsesInfo_ {
   const refs_Refs* r;
   refs_RefUsesFn fn;
   void* arg;
   uint32_t loc_idx;
};

static uint32_t refs_round4(uint32_t x);
static refs_MapFile refs_open_file(const char* filename);
static void refs_close_file(refs_MapFile f);
static uint32_t refs_section_size(const void* section);
static void* refs_section_load(refs_MapFile* f, uint32_t minSize);
static bool refs_section_write(int32_t fd, void* section);
static void refs_section_free(void* t);
static void* refs_Files_tags(const refs_Files* f);
static void* refs_Files_names(const refs_Files* f);
static refs_Files* refs_Files_create(uint32_t max_idx, uint32_t max_data);
static const refs_TagRange* refs_Files_getTagRange(const refs_Files* f, uint32_t idx);
static refs_Files* refs_Files_resize(refs_Files* f, uint32_t max_idx, uint32_t max_data);
static void refs_Files_visitTags(const refs_Files* f, refs_FileTagsVisitor visitor, void* arg);
static const char* refs_Files_idx2name(const refs_Files* f, uint32_t file_id);
static uint32_t refs_Files_name2idx(const refs_Files* f, const char* filename);
static uint16_t refs_Files_add(refs_Files** f_ptr, const char* filename, uint32_t tag_start);
static void refs_Files_setEndTag(refs_Files* f, uint32_t file_idx, uint32_t end_tag);
static refs_Files* refs_Files_trim(refs_Files* f);
static void refs_Files_dump(const refs_Files* f, bool verbose);
static refs_Tags* refs_Tags_create(uint32_t capacity);
static uint32_t refs_Tags_getCount(const refs_Tags* t);
static refs_Tags* refs_Tags_resize(refs_Tags* t, uint32_t capacity);
static bool refs_tag_compare(void* _arg0, const void* left, const void* right);
static void refs_Tags_sort(void* arg, uint32_t _arg1, uint32_t start, uint32_t end);
static refs_Tags* refs_Tags_trim(refs_Tags* t);
static void refs_Tags_add(refs_Tags** t_ptr, const refs_RefSrc* src, uint32_t loc_idx);
static bool refs_Tags_find_line(const refs_Tag* tags, uint32_t* left, uint32_t* right, const uint32_t line);
static uint32_t refs_Tags_find(const refs_Tags* t, const refs_Dest* origin, uint32_t start, uint32_t last);
static void refs_Tags_dump(const refs_Tags* t, bool verbose);
static refs_Locs* refs_Locs_create(uint32_t capacity);
static refs_Locs* refs_Locs_resize(refs_Locs* t, uint32_t capacity);
static refs_Locs* refs_Locs_trim(refs_Locs* l);
static uint32_t refs_Locs_add(refs_Locs** t_ptr, uint16_t file_id, uint32_t line, uint16_t col);
static const refs_Loc* refs_Locs_idx2loc(const refs_Locs* t, uint32_t idx);
static void refs_Locs_dump(const refs_Locs* t, bool verbose);
static void* refs_Symbols_locs(const refs_Symbols* s);
static void* refs_Symbols_names(const refs_Symbols* s);
static refs_Symbols* refs_Symbols_create(uint32_t max_idx, uint32_t max_data);
static refs_Symbols* refs_Symbols_resize(refs_Symbols* f, uint32_t max_idx, uint32_t max_data);
static uint32_t refs_Symbols_name2idx(const refs_Symbols* f, const char* filename);
static void refs_Symbols_add(refs_Symbols** f_ptr, const char* symbol_name, uint32_t loc_idx);
static refs_Symbols* refs_Symbols_trim(refs_Symbols* f);
static void refs_Symbols_dump(const refs_Symbols* f, bool verbose);
static refs_LocsCache* refs_LocsCache_create(uint32_t capacity);
static refs_LocsCache* refs_LocsCache_resize(refs_LocsCache* t, uint32_t capacity);
static uint32_t refs_LocsCache_add(refs_LocsCache** t_ptr, uint16_t file_id, uint32_t line, uint16_t col);
static refs_Refs* refs_Refs_create(void);
static void refs_Refs_free(refs_Refs* r);
static refs_Refs* refs_Refs_load_internal(refs_MapFile f);
static refs_Refs* refs_Refs_load(const char* filename);
static bool refs_Refs_write(refs_Refs* r, const char* filename);
static void refs_Refs_trim(refs_Refs* r);
static void refs_Refs_add_file(refs_Refs* r, const char* filename);
static uint32_t refs_Refs_add_dest(refs_Refs* r, const refs_Dest* dest);
static void refs_Refs_add_tag(refs_Refs* r, const refs_RefSrc* src, const refs_Dest* dest);
static void refs_Refs_add_symbol(refs_Refs* r, const char* symbol_name, const refs_Dest* dest);
static uint32_t refs_Refs_ref2loc(const refs_Refs* r, const refs_Dest* origin);
static refs_Dest refs_Refs_loc2ref(const refs_Refs* r, uint32_t loc_idx);
static refs_Dest refs_Refs_findRef(const refs_Refs* r, const refs_Dest* origin);
static refs_Dest refs_Refs_findSymbol(const refs_Refs* r, const char* symbol_name);
static void refs_Refs_search_tags(void* arg, uint32_t file_idx, uint32_t start, uint32_t end);
static void refs_Refs_loc2uses(const refs_Refs* r, uint32_t loc_idx, refs_RefUsesFn fn, void* arg);
static void refs_Refs_findRefUses(const refs_Refs* r, const refs_Dest* origin, refs_RefUsesFn fn, void* arg);
static void refs_Refs_findSymbolUses(const refs_Refs* r, const char* symbol_name, refs_RefUsesFn fn, void* arg);
static void refs_Refs_dump(const refs_Refs* r, bool verbose);

static const uint32_t refs_NOT_FOUND = c2_max_u32;

static uint32_t refs_round4(uint32_t x)
{
   return ((x + 3) & ~0x3);
}

static refs_MapFile refs_open_file(const char* filename)
{
   refs_MapFile result = { NULL, 0 };
   int32_t fd = open(filename, (O_RDONLY | O_CLOEXEC));
   if (fd == -1) return result;

   struct stat statbuf;
   int32_t err = stat(filename, &statbuf);
   if (err) return result;

   int32_t size = ((int32_t)(statbuf.st_size));
   int32_t prot = PROT_READ;
   int32_t flags = MAP_PRIVATE;
   void* map = mmap(NULL, ((size_t)(size)), prot, flags, fd, 0);
   if (((size_t)(map)) == ((size_t)(-1))) {
      err = *__errno_location();
      close(fd);
      return result;
   }
   close(fd);
   result.map = map;
   result.size = ((size_t)(size));
   return result;
}

static void refs_close_file(refs_MapFile f)
{
   munmap(f.map, f.size);
}

static uint32_t refs_section_size(const void* section)
{
   const uint32_t* ptr = ((uint32_t*)(section));
   return *ptr;
}

static void* refs_section_load(refs_MapFile* f, uint32_t minSize)
{
   uint32_t size = ((uint32_t)(f->size));
   if (size < minSize) return NULL;

   void* section = f->map;
   uint32_t load_size = refs_section_size(section);
   if (load_size > size) return NULL;

   f->map = ((uint8_t*)(f->map)) + load_size;
   f->size -= load_size;
   return section;
}

static bool refs_section_write(int32_t fd, void* section)
{
   const uint32_t size = refs_section_size(section);
   ssize_t written = write(fd, section, size);
   if (size != written) {
      close(fd);
      return false;
   }
   return true;
}

static void refs_section_free(void* t)
{
   free(t);
}

static void* refs_Files_tags(const refs_Files* f)
{
   uint8_t* ptr = ((uint8_t*)(f->name_indexes));
   ptr += f->idx_cap * sizeof(uint32_t);
   return ptr;
}

static void* refs_Files_names(const refs_Files* f)
{
   uint8_t* ptr = ((uint8_t*)(f->name_indexes));
   ptr += f->idx_cap * (sizeof(uint32_t) + sizeof(refs_TagRange));
   return ptr;
}

static refs_Files* refs_Files_create(uint32_t max_idx, uint32_t max_data)
{
   uint32_t size = sizeof(refs_Files) + (max_idx * (sizeof(uint32_t) + sizeof(refs_TagRange)) + max_data);
   size = refs_round4(size);
   refs_Files* f = malloc(size);
   f->total_size = size;
   f->idx_count = 0;
   f->idx_cap = max_idx;
   f->names_len = 0;
   f->names_cap = max_data;
   uint32_t* last = ((uint32_t*)(f));
   last[(size / 4) - 1] = 0;
   return f;
}

static const refs_TagRange* refs_Files_getTagRange(const refs_Files* f, uint32_t idx)
{
   const refs_TagRange* tags = refs_Files_tags(f);
   return &tags[idx];
}

static refs_Files* refs_Files_resize(refs_Files* f, uint32_t max_idx, uint32_t max_data)
{
   refs_Files* f2 = refs_Files_create(max_idx, max_data);
   if (f->idx_count) {
      f2->idx_count = f->idx_count;
      memcpy(f2->name_indexes, f->name_indexes, f->idx_count * sizeof(uint32_t));
      memcpy(refs_Files_tags(f2), refs_Files_tags(f), f->idx_count * sizeof(refs_TagRange));
   }
   if (f->names_len) {
      f2->names_len = f->names_len;
      memcpy(refs_Files_names(f2), refs_Files_names(f), f->names_len);
   }
   free(f);
   return f2;
}

static void refs_Files_visitTags(const refs_Files* f, refs_FileTagsVisitor visitor, void* arg)
{
   const refs_TagRange* ranges = refs_Files_tags(f);
   for (uint32_t i = 0; i < f->idx_count; i++) {
      const refs_TagRange* range = &ranges[i];
      if (range->start != range->end) visitor(arg, i, range->start, range->end);
   }
}

static const char* refs_Files_idx2name(const refs_Files* f, uint32_t file_id)
{
   if (file_id >= f->idx_count) return NULL;

   const char* names = refs_Files_names(f);
   return names + f->name_indexes[file_id];
}

static uint32_t refs_Files_name2idx(const refs_Files* f, const char* filename)
{
   const char* names = refs_Files_names(f);
   for (uint32_t i = 0; i < f->idx_count; i++) {
      const char* name = names + f->name_indexes[i];
      if (strcmp(name, filename) == 0) return i;

   }
   return refs_NOT_FOUND;
}

static uint16_t refs_Files_add(refs_Files** f_ptr, const char* filename, uint32_t tag_start)
{
   refs_Files* f = *f_ptr;
   uint32_t idx = refs_Files_name2idx(f, filename);
   refs_TagRange* tags = refs_Files_tags(f);
   if (idx == refs_NOT_FOUND) {
      uint32_t len = ((uint32_t)(strlen(filename))) + 1;
      if ((f->idx_count == f->idx_cap || f->names_len + len > f->names_cap)) {
         f = refs_Files_resize(f, f->idx_count * 2, f->names_len * 2);
         *f_ptr = f;
         tags = refs_Files_tags(f);
      }
      idx = f->idx_count;
      f->name_indexes[idx] = f->names_len;
      f->idx_count++;
      tags[idx].start = 0;
      tags[idx].end = 0;
      char* names = refs_Files_names(f);
      memcpy(names + f->names_len, filename, len);
      f->names_len += len;
   }
   if (tag_start != refs_NOT_FOUND) {
      tags[idx].start = tag_start;
   }
   return ((uint16_t)(idx));
}

static void refs_Files_setEndTag(refs_Files* f, uint32_t file_idx, uint32_t end_tag)
{
   refs_TagRange* tags = refs_Files_tags(f);
   tags[file_idx].end = end_tag;
}

static refs_Files* refs_Files_trim(refs_Files* f)
{
   if ((f->idx_count == f->idx_cap && f->names_len == f->names_cap)) return f;

   return refs_Files_resize(f, f->idx_count, f->names_len);
}

static void refs_Files_dump(const refs_Files* f, bool verbose)
{
   printf("  files:   %5u bytes  %u/%u entries  %u/%u data\n", f->total_size, f->idx_count, f->idx_cap, f->names_len, f->names_cap);
   if (verbose) {
      const refs_TagRange* tags = refs_Files_tags(f);
      const char* names = refs_Files_names(f);
      for (uint32_t i = 0; i < f->idx_count; i++) {
         const uint32_t idx = f->name_indexes[i];
         printf("  [%4u] %5u %5u %s\n", i, tags[i].start, tags[i].end, names + idx);
      }
   }
}

static refs_Tags* refs_Tags_create(uint32_t capacity)
{
   uint32_t size = sizeof(refs_Tags) + capacity * sizeof(refs_Tag);
   size = refs_round4(size);
   refs_Tags* t = malloc(size);
   t->total_size = size;
   t->count = 0;
   t->capacity = capacity;
   return t;
}

static uint32_t refs_Tags_getCount(const refs_Tags* t)
{
   return t->count;
}

static refs_Tags* refs_Tags_resize(refs_Tags* t, uint32_t capacity)
{
   refs_Tags* t2 = refs_Tags_create(capacity);
   if (t->count) {
      t2->count = t->count;
      memcpy(t2->tags, t->tags, t->count * sizeof(refs_Tag));
   }
   free(t);
   return t2;
}

static bool refs_tag_compare(void* _arg0, const void* left, const void* right)
{
   const refs_Tag* l = left;
   const refs_Tag* r = right;
   const uint32_t line1 = l->src.line;
   const uint32_t line2 = r->src.line;
   if (line1 < line2) return true;

   if (line1 > line2) return false;

   return l->src.col < r->src.col;
}

static void refs_Tags_sort(void* arg, uint32_t _arg1, uint32_t start, uint32_t end)
{
   refs_Tags* t = arg;
   if (end - start <= 1) return;

   quicksort_sort(t->tags + start, end - start, sizeof(refs_Tag), refs_tag_compare, NULL);
}

static refs_Tags* refs_Tags_trim(refs_Tags* t)
{
   if (t->count == t->capacity) return t;

   return refs_Tags_resize(t, t->count);
}

static void refs_Tags_add(refs_Tags** t_ptr, const refs_RefSrc* src, uint32_t loc_idx)
{
   refs_Tags* t = *t_ptr;
   if (t->count == t->capacity) {
      t = refs_Tags_resize(t, t->count * 2);
      *t_ptr = t;
   }
   refs_Tag* cur = &t->tags[t->count];
   cur->src = *src;
   cur->loc_idx = loc_idx;
   t->count++;
}

static bool refs_Tags_find_line(const refs_Tag* tags, uint32_t* left, uint32_t* right, const uint32_t line)
{
   while (*left != *right) {
      const uint32_t middle = (*left + *right) / 2;
      const uint32_t mline = tags[middle].src.line;
      if (line < mline) {
         if (*right == middle) break;

         *right = middle;
      } else if (line > mline) {
         if (*left == middle) break;

         *left = middle;
      } else {
         uint32_t l = middle;
         uint32_t r = middle;
         while (((l - 1 >= *left && l >= 1) && tags[l - 1].src.line == line)) l--;
         while ((r + 1 < *right && tags[r + 1].src.line == line)) r++;
         *left = l;
         *right = r;
         return true;
      }

   }
   return false;
}

static uint32_t refs_Tags_find(const refs_Tags* t, const refs_Dest* origin, uint32_t start, uint32_t last)
{
   if (!refs_Tags_find_line(t->tags, &start, &last, origin->line)) return refs_NOT_FOUND;

   for (uint32_t i = start; i <= last; i++) {
      const refs_Tag* tag = &t->tags[i];
      const refs_RefSrc* src = &tag->src;
      if ((src->col <= origin->col && origin->col < (src->col + src->len))) {
         return tag->loc_idx;
      }
   }
   return refs_NOT_FOUND;
}

static void refs_Tags_dump(const refs_Tags* t, bool verbose)
{
   printf("  tags:   %5u bytes  %u/%u tags\n", t->total_size, t->count, t->capacity);
   if (verbose) {
      for (uint32_t i = 0; i < t->count; i++) {
         const refs_Tag* tt = &t->tags[i];
         printf("  [%5u] %u:%u:%u -> %u\n", i, tt->src.line, tt->src.col, tt->src.len, tt->loc_idx);
      }
   }
}

static refs_Locs* refs_Locs_create(uint32_t capacity)
{
   uint32_t size = sizeof(refs_Locs) + capacity * sizeof(refs_Loc);
   size = refs_round4(size);
   refs_Locs* t = malloc(size);
   t->total_size = size;
   t->count = 0;
   t->capacity = capacity;
   return t;
}

static refs_Locs* refs_Locs_resize(refs_Locs* t, uint32_t capacity)
{
   refs_Locs* t2 = refs_Locs_create(capacity);
   if (t->count) {
      t2->count = t->count;
      memcpy(t2->locs, t->locs, t->count * sizeof(refs_Loc));
   }
   free(t);
   return t2;
}

static refs_Locs* refs_Locs_trim(refs_Locs* l)
{
   if (l->count == l->capacity) return l;

   return refs_Locs_resize(l, l->count);
}

static uint32_t refs_Locs_add(refs_Locs** t_ptr, uint16_t file_id, uint32_t line, uint16_t col)
{
   refs_Locs* t = *t_ptr;
   if (t->count == t->capacity) {
      t = refs_Locs_resize(t, t->count * 2);
      *t_ptr = t;
   }
   uint32_t idx = t->count;
   refs_Loc* loc = &t->locs[idx];
   loc->line = line;
   loc->file_id = file_id;
   loc->col = col;
   t->count++;
   return idx;
}

static const refs_Loc* refs_Locs_idx2loc(const refs_Locs* t, uint32_t idx)
{
   return &t->locs[idx];
}

static void refs_Locs_dump(const refs_Locs* t, bool verbose)
{
   printf("  locs:    %5u bytes  %u/%u locs\n", t->total_size, t->count, t->capacity);
   if (verbose) {
      for (uint32_t i = 0; i < t->count; i++) {
         const refs_Loc* d = &t->locs[i];
         printf("  [%5u] %u:%u:%u\n", i, d->file_id, d->line, d->col);
      }
   }
}

static void* refs_Symbols_locs(const refs_Symbols* s)
{
   uint8_t* ptr = ((uint8_t*)(s->name_indexes));
   ptr += s->idx_cap * sizeof(uint32_t);
   return ptr;
}

static void* refs_Symbols_names(const refs_Symbols* s)
{
   uint8_t* ptr = ((uint8_t*)(s->name_indexes));
   ptr += s->idx_cap * (sizeof(uint32_t) + sizeof(uint32_t));
   return ptr;
}

static refs_Symbols* refs_Symbols_create(uint32_t max_idx, uint32_t max_data)
{
   uint32_t size = sizeof(refs_Symbols) + (max_idx * sizeof(uint32_t) * 2) + max_data;
   size = refs_round4(size);
   refs_Symbols* f = malloc(size);
   f->total_size = size;
   f->idx_count = 0;
   f->idx_cap = max_idx;
   f->names_len = 0;
   f->names_cap = max_data;
   uint32_t* last = ((uint32_t*)(f));
   last[(size / 4) - 1] = 0;
   return f;
}

static refs_Symbols* refs_Symbols_resize(refs_Symbols* f, uint32_t max_idx, uint32_t max_data)
{
   refs_Symbols* f2 = refs_Symbols_create(max_idx, max_data);
   if (f->idx_count) {
      f2->idx_count = f->idx_count;
      memcpy(f2->name_indexes, f->name_indexes, f->idx_count * sizeof(uint32_t));
      memcpy(refs_Symbols_locs(f2), refs_Symbols_locs(f), f->idx_count * sizeof(uint32_t));
   }
   if (f->names_len) {
      f2->names_len = f->names_len;
      memcpy(refs_Symbols_names(f2), refs_Symbols_names(f), f->names_len);
   }
   free(f);
   return f2;
}

static uint32_t refs_Symbols_name2idx(const refs_Symbols* f, const char* filename)
{
   const char* names = refs_Symbols_names(f);
   for (uint32_t i = 0; i < f->idx_count; i++) {
      const char* name = names + f->name_indexes[i];
      if (strcmp(name, filename) == 0) {
         const uint32_t* locs = refs_Symbols_locs(f);
         return locs[i];
      }
   }
   return refs_NOT_FOUND;
}

static void refs_Symbols_add(refs_Symbols** f_ptr, const char* symbol_name, uint32_t loc_idx)
{
   refs_Symbols* f = *f_ptr;
   uint32_t len = ((uint32_t)(strlen(symbol_name))) + 1;
   if ((f->idx_count == f->idx_cap || f->names_len + len > f->names_cap)) {
      f = refs_Symbols_resize(f, f->idx_count * 2, f->names_len * 2);
      *f_ptr = f;
   }
   f->name_indexes[f->idx_count] = f->names_len;
   uint32_t* locs = refs_Symbols_locs(f);
   locs[f->idx_count] = loc_idx;
   f->idx_count++;
   char* names = refs_Symbols_names(f);
   memcpy(names + f->names_len, symbol_name, len);
   f->names_len += len;
}

static refs_Symbols* refs_Symbols_trim(refs_Symbols* f)
{
   if ((f->idx_count == f->idx_cap && f->names_len == f->names_cap)) return f;

   return refs_Symbols_resize(f, f->idx_count, f->names_len);
}

static void refs_Symbols_dump(const refs_Symbols* f, bool verbose)
{
   printf("  symbols: %5u bytes  %u/%u entries  %u/%u data\n", f->total_size, f->idx_count, f->idx_cap, f->names_len, f->names_cap);
   if (verbose) {
      const uint32_t* locs = refs_Symbols_locs(f);
      const char* names = refs_Symbols_names(f);
      for (uint32_t i = 0; i < f->idx_count; i++) {
         uint32_t idx = f->name_indexes[i];
         printf("  [%5u] %5u %s\n", i, locs[i], names + idx);
      }
   }
}

static refs_LocsCache* refs_LocsCache_create(uint32_t capacity)
{
   uint32_t size = sizeof(refs_LocsCache) + capacity * sizeof(refs_CacheLoc);
   size = refs_round4(size);
   refs_LocsCache* t = malloc(size);
   t->total_size = size;
   t->count = 0;
   t->capacity = capacity;
   return t;
}

static refs_LocsCache* refs_LocsCache_resize(refs_LocsCache* t, uint32_t capacity)
{
   refs_LocsCache* t2 = refs_LocsCache_create(capacity);
   if (t->count) {
      t2->count = t->count;
      memcpy(t2->locs, t->locs, t->count * sizeof(refs_CacheLoc));
   }
   free(t);
   return t2;
}

static uint32_t refs_LocsCache_add(refs_LocsCache** t_ptr, uint16_t file_id, uint32_t line, uint16_t col)
{
   refs_LocsCache* t = *t_ptr;
   const refs_U64Loc l2 = { .loc = { line, file_id, col } };
   const uint64_t loc = l2.num;
   uint32_t left = 0;
   uint32_t right = t->count;
   if (t->count == 0) {
      refs_CacheLoc* c = &t->locs[0];
      c->loc = loc;
      c->idx = 0;
      t->count = 1;
      return refs_NOT_FOUND;
   }
   uint32_t middle = (left + right) / 2;
   while (left != right) {
      middle = (left + right) / 2;
      const refs_CacheLoc* cur = &t->locs[middle];
      if (loc > cur->loc) {
         if (left == middle) break;

         left = middle;
      } else if (loc < cur->loc) {
         if (right == middle) break;

         right = middle;
      } else {
         return cur->idx;
      }

   }
   if (t->count == t->capacity) {
      t = refs_LocsCache_resize(t, t->count * 2);
      *t_ptr = t;
   }
   if (loc > t->locs[middle].loc) middle++;
   for (uint32_t i = t->count; i > middle; i--) {
      t->locs[i] = t->locs[i - 1];
   }
   refs_CacheLoc* c = &t->locs[middle];
   c->loc = loc;
   c->idx = t->count;
   t->count++;
   return refs_NOT_FOUND;
}

static refs_Refs* refs_Refs_create(void)
{
   refs_Refs* r = calloc(1, sizeof(refs_Refs));
   r->files = refs_Files_create(32, 2048);
   r->tags = refs_Tags_create(128);
   r->locs = refs_Locs_create(64);
   r->symbols = refs_Symbols_create(64, 512);
   r->locs_cache = refs_LocsCache_create(64);
   return r;
}

static void refs_Refs_free(refs_Refs* r)
{
   if (r->file.map) {
      refs_close_file(r->file);
   } else {
      refs_section_free(r->files);
      refs_section_free(r->tags);
      refs_section_free(r->locs);
      refs_section_free(r->symbols);
      refs_section_free(r->locs_cache);
   }
   free(r);
}

static refs_Refs* refs_Refs_load_internal(refs_MapFile f)
{
   refs_Files* files = refs_section_load(&f, sizeof(refs_Files));
   if (!files) return NULL;

   refs_Tags* tags = refs_section_load(&f, sizeof(refs_Tags));
   if (!tags) return NULL;

   refs_Locs* locs = refs_section_load(&f, sizeof(refs_Locs));
   if (!locs) return NULL;

   refs_Symbols* symbols = refs_section_load(&f, sizeof(refs_Symbols));
   if (!symbols) return NULL;

   refs_Refs* r = calloc(1, sizeof(refs_Refs));
   r->files = files;
   r->tags = tags;
   r->locs = locs;
   r->symbols = symbols;
   return r;
}

static refs_Refs* refs_Refs_load(const char* filename)
{
   refs_MapFile f = refs_open_file(filename);
   if (!f.map) return NULL;

   refs_Refs* r = refs_Refs_load_internal(f);
   if (r) {
      r->file = f;
   } else {
      refs_close_file(f);
   }
   return r;
}

static bool refs_Refs_write(refs_Refs* r, const char* filename)
{
   refs_Refs_trim(r);
   int32_t fd = open(filename, (((O_CREAT | O_WRONLY) | O_CLOEXEC) | O_TRUNC), 0660);
   if (fd == -1) return false;

   if (!refs_section_write(fd, r->files)) return false;

   if (!refs_section_write(fd, r->tags)) return false;

   if (!refs_section_write(fd, r->locs)) return false;

   if (!refs_section_write(fd, r->symbols)) return false;

   close(fd);
   return true;
}

static void refs_Refs_trim(refs_Refs* r)
{
   if (r->cur_file_idx != refs_NOT_FOUND) refs_Files_setEndTag(r->files, r->cur_file_idx, refs_Tags_getCount(r->tags));
   r->cur_file_idx = refs_NOT_FOUND;
   r->files = refs_Files_trim(r->files);
   r->tags = refs_Tags_trim(r->tags);
   r->locs = refs_Locs_trim(r->locs);
   r->symbols = refs_Symbols_trim(r->symbols);
   refs_Files_visitTags(r->files, refs_Tags_sort, r->tags);
}

static void refs_Refs_add_file(refs_Refs* r, const char* filename)
{
   if (r->cur_file_idx != refs_NOT_FOUND) refs_Files_setEndTag(r->files, r->cur_file_idx, refs_Tags_getCount(r->tags));
   r->cur_file_idx = refs_Files_add(&r->files, filename, refs_Tags_getCount(r->tags));
}

static uint32_t refs_Refs_add_dest(refs_Refs* r, const refs_Dest* dest)
{
   uint16_t file_idx;
   if (r->dest_file_ptr == dest->filename) {
      file_idx = r->dest_file_idx;
   } else {
      file_idx = refs_Files_add(&r->files, dest->filename, refs_NOT_FOUND);
      r->dest_file_ptr = dest->filename;
      r->dest_file_idx = file_idx;
   }
   uint32_t loc_idx = refs_LocsCache_add(&r->locs_cache, file_idx, dest->line, dest->col);
   if (loc_idx == refs_NOT_FOUND) {
      loc_idx = refs_Locs_add(&r->locs, file_idx, dest->line, dest->col);
   }
   return loc_idx;
}

static void refs_Refs_add_tag(refs_Refs* r, const refs_RefSrc* src, const refs_Dest* dest)
{
   uint32_t loc_idx = refs_Refs_add_dest(r, dest);
   refs_Tags_add(&r->tags, src, loc_idx);
}

static void refs_Refs_add_symbol(refs_Refs* r, const char* symbol_name, const refs_Dest* dest)
{
   uint32_t loc_idx = refs_Refs_add_dest(r, dest);
   refs_Symbols_add(&r->symbols, symbol_name, loc_idx);
}

static uint32_t refs_Refs_ref2loc(const refs_Refs* r, const refs_Dest* origin)
{
   uint32_t file_id = refs_Files_name2idx(r->files, origin->filename);
   if (file_id == refs_NOT_FOUND) return refs_NOT_FOUND;

   const refs_TagRange* range = refs_Files_getTagRange(r->files, file_id);
   if (range->start == range->end) return refs_NOT_FOUND;

   uint32_t loc_idx = refs_Tags_find(r->tags, origin, range->start, range->end);
   return loc_idx;
}

static refs_Dest refs_Refs_loc2ref(const refs_Refs* r, uint32_t loc_idx)
{
   refs_Dest result = { NULL, 0, 0 };
   if (loc_idx == refs_NOT_FOUND) return result;

   const refs_Loc* loc = refs_Locs_idx2loc(r->locs, loc_idx);
   result.filename = refs_Files_idx2name(r->files, loc->file_id);
   result.line = loc->line;
   result.col = loc->col;
   return result;
}

static refs_Dest refs_Refs_findRef(const refs_Refs* r, const refs_Dest* origin)
{
   uint32_t loc_idx = refs_Refs_ref2loc(r, origin);
   return refs_Refs_loc2ref(r, loc_idx);
}

static refs_Dest refs_Refs_findSymbol(const refs_Refs* r, const char* symbol_name)
{
   uint32_t loc_idx = refs_Symbols_name2idx(r->symbols, symbol_name);
   return refs_Refs_loc2ref(r, loc_idx);
}

static void refs_Refs_search_tags(void* arg, uint32_t file_idx, uint32_t start, uint32_t end)
{
   const refs_UsesInfo* info = arg;
   const uint32_t loc_idx = info->loc_idx;
   const refs_Tag* tags = info->r->tags->tags;
   for (uint32_t i = start; i < end; i++) {
      const refs_Tag* tt = &tags[i];
      if (tt->loc_idx == loc_idx) {
         refs_Dest result = { refs_Files_idx2name(info->r->files, file_idx), tt->src.line, tt->src.col };
         info->fn(info->arg, &result);
      }
   }
}

static void refs_Refs_loc2uses(const refs_Refs* r, uint32_t loc_idx, refs_RefUsesFn fn, void* arg)
{
   if (loc_idx == refs_NOT_FOUND) return;

   refs_UsesInfo info = { r, fn, arg, loc_idx };
   refs_Files_visitTags(r->files, refs_Refs_search_tags, &info);
}

static void refs_Refs_findRefUses(const refs_Refs* r, const refs_Dest* origin, refs_RefUsesFn fn, void* arg)
{
   uint32_t loc_idx = refs_Refs_ref2loc(r, origin);
   refs_Refs_loc2uses(r, loc_idx, fn, arg);
}

static void refs_Refs_findSymbolUses(const refs_Refs* r, const char* symbol_name, refs_RefUsesFn fn, void* arg)
{
   uint32_t loc_idx = refs_Symbols_name2idx(r->symbols, symbol_name);
   refs_Refs_loc2uses(r, loc_idx, fn, arg);
}

static void refs_Refs_dump(const refs_Refs* r, bool verbose)
{
   printf("Refs:\n");
   refs_Files_dump(r->files, verbose);
   refs_Tags_dump(r->tags, verbose);
   refs_Locs_dump(r->locs, verbose);
   refs_Symbols_dump(r->symbols, verbose);
}


// --- module source_mgr ---

typedef struct source_mgr_File_ source_mgr_File;
typedef struct source_mgr_SourceMgr_ source_mgr_SourceMgr;
typedef struct source_mgr_Location_ source_mgr_Location;

struct source_mgr_Location_ {
   uint32_t line;
   uint32_t column;
   const char* filename;
   const char* line_start;
};

struct source_mgr_File_ {
   uint32_t filename;
   uint32_t offset;
   file_utils_Reader file;
   bool needed;
   uint32_t last_offset;
   source_mgr_Location last_loc;
};

struct source_mgr_SourceMgr_ {
   const string_pool_Pool* pool;
   source_mgr_File* files;
   uint32_t num_files;
   uint32_t max_files;
   uint32_t num_open;
   uint32_t max_open;
   uint32_t other_count;
   uint32_t other_size;
   uint32_t sources_count;
   uint32_t sources_size;
};

static void source_mgr_File_close(source_mgr_File* f);
static source_mgr_SourceMgr* source_mgr_create(const string_pool_Pool* pool, uint32_t max_open);
static void source_mgr_SourceMgr_free(source_mgr_SourceMgr* sm);
static void source_mgr_SourceMgr_clear(source_mgr_SourceMgr* sm, uint32_t handle);
static file_utils_Reader source_mgr_SourceMgr_openInternal(source_mgr_SourceMgr* sm, const char* filename, src_loc_SrcLoc loc);
static bool source_mgr_SourceMgr_close_oldest(source_mgr_SourceMgr* sm);
static int32_t source_mgr_SourceMgr_open(source_mgr_SourceMgr* sm, uint32_t filename, src_loc_SrcLoc loc, bool is_source);
static void source_mgr_SourceMgr_close(source_mgr_SourceMgr* sm, int32_t file_id);
static void source_mgr_SourceMgr_checkOpen(source_mgr_SourceMgr* sm, int32_t handle);
static const char* source_mgr_SourceMgr_get_content(source_mgr_SourceMgr* sm, int32_t handle);
static const char* source_mgr_SourceMgr_get_token_source(source_mgr_SourceMgr* sm, src_loc_SrcLoc loc);
static uint32_t source_mgr_SourceMgr_get_offset(source_mgr_SourceMgr* sm, int32_t handle);
static const char* source_mgr_SourceMgr_getFileName(source_mgr_SourceMgr* sm, int32_t handle);
static source_mgr_File* source_mgr_SourceMgr_find_file(source_mgr_SourceMgr* sm, src_loc_SrcLoc loc);
static void source_mgr_find_line_col(const char* data, uint32_t offset, source_mgr_Location* loc, uint32_t last_offset);
static source_mgr_Location source_mgr_SourceMgr_locate(source_mgr_SourceMgr* sm, src_loc_SrcLoc loc);
static source_mgr_Location source_mgr_SourceMgr_getLocation(source_mgr_SourceMgr* sm, src_loc_SrcLoc sloc);
static const char* source_mgr_SourceMgr_loc2str(source_mgr_SourceMgr* sm, src_loc_SrcLoc sloc);
static void source_mgr_SourceMgr_report(const source_mgr_SourceMgr* sm);

static const uint32_t source_mgr_InitialMaxFiles = 8;

static void source_mgr_File_close(source_mgr_File* f)
{
   file_utils_Reader_close(&f->file);
}

static source_mgr_SourceMgr* source_mgr_create(const string_pool_Pool* pool, uint32_t max_open)
{
   source_mgr_SourceMgr* sm = calloc(1, sizeof(source_mgr_SourceMgr));
   sm->pool = pool;
   sm->max_files = source_mgr_InitialMaxFiles;
   sm->files = malloc(sizeof(source_mgr_File) * sm->max_files);
   sm->max_open = max_open;
   return sm;
}

static void source_mgr_SourceMgr_free(source_mgr_SourceMgr* sm)
{
   for (uint32_t i = 0; i < sm->num_files; i++) {
      source_mgr_File_close(&sm->files[i]);
   }
   free(sm->files);
   free(sm);
}

static void source_mgr_SourceMgr_clear(source_mgr_SourceMgr* sm, uint32_t handle)
{
   handle++;
   for (uint32_t i = handle; i < sm->num_files; i++) {
      source_mgr_File* f = &sm->files[i];
      if (f->needed) {
         printf("WARN %s still not closed\n", string_pool_Pool_idx2str(sm->pool, f->filename));
      }
      file_utils_Reader_close(&f->file);
   }
   sm->num_files = handle;
   sm->num_open = 0;
   sm->other_count = handle;
   sm->other_size = 0;
   sm->sources_count = 0;
   sm->sources_size = 0;
   for (uint32_t i = 0; i < sm->num_files; i++) {
      const source_mgr_File* f = &sm->files[i];
      if (file_utils_Reader_isOpen(&f->file)) sm->num_open++;
      sm->other_size += f->file.size;
   }
}

static file_utils_Reader source_mgr_SourceMgr_openInternal(source_mgr_SourceMgr* sm, const char* filename, src_loc_SrcLoc loc)
{
   file_utils_Reader file;
   if (file_utils_Reader_open(&file, filename)) {
      sm->num_open++;
   } else {
      char error_msg[256];
      if (file.errno == file_utils_Err_not_a_file) {
         sprintf(error_msg, "cannot open %s: %s\n", filename, "not a regular file");
      } else {
         sprintf(error_msg, "cannot open %s: %s\n", filename, strerror(file.errno));
      }
      if (loc) {
         fprintf(stderr, "%s: %serror:%s %s\n", source_mgr_SourceMgr_loc2str(sm, loc), color_Red, color_Normal, error_msg);
      } else {
         fprintf(stderr, "%serror%s: %s\n", color_Red, color_Normal, error_msg);
      }
   }
   return file;
}

static bool source_mgr_SourceMgr_close_oldest(source_mgr_SourceMgr* sm)
{
   for (uint32_t i = 0; i < sm->num_files; i++) {
      source_mgr_File* f = &sm->files[i];
      if ((file_utils_Reader_isOpen(&f->file) && !f->needed)) {
         file_utils_Reader_close(&f->file);
         sm->num_open--;
         return true;
      }
   }
   return false;
}

static int32_t source_mgr_SourceMgr_open(source_mgr_SourceMgr* sm, uint32_t filename, src_loc_SrcLoc loc, bool is_source)
{
   if (sm->num_open == sm->max_open) {
      if (!source_mgr_SourceMgr_close_oldest(sm)) {
         fprintf(stderr, "%serror%s: too many files open\n", color_Red, color_Normal);
         return -1;
      }
   }
   file_utils_Reader file = source_mgr_SourceMgr_openInternal(sm, string_pool_Pool_idx2str(sm->pool, filename), loc);
   if (!file_utils_Reader_isOpen(&file)) return -1;

   if (sm->num_files == sm->max_files) {
      sm->max_files *= 2;
      source_mgr_File* files2 = malloc(sizeof(source_mgr_File) * sm->max_files);
      memcpy(files2, sm->files, sm->num_files * sizeof(source_mgr_File));
      free(sm->files);
      sm->files = files2;
   }
   int32_t file_id = ((int32_t)(sm->num_files));
   source_mgr_File* f = &sm->files[sm->num_files];
   memset(f, 0, sizeof(source_mgr_File));
   uint32_t offset = 1;
   if (sm->num_files) offset = sm->files[sm->num_files - 1].offset + sm->files[sm->num_files - 1].file.size;
   f->filename = filename;
   f->offset = offset;
   f->file = file;
   f->needed = true;
   if (is_source) {
      sm->sources_count++;
      sm->sources_size += file.size;
   } else {
      sm->other_count++;
      sm->other_size += file.size;
   }
   sm->num_files++;
   return file_id;
}

static void source_mgr_SourceMgr_close(source_mgr_SourceMgr* sm, int32_t file_id)
{
   sm->files[file_id].needed = false;
}

static void source_mgr_SourceMgr_checkOpen(source_mgr_SourceMgr* sm, int32_t handle)
{
   source_mgr_File* f = &sm->files[handle];
   if (file_utils_Reader_isOpen(&f->file)) return;

   if (sm->num_open == sm->max_open) {
      if (!source_mgr_SourceMgr_close_oldest(sm)) {
         fprintf(stderr, "%serror%s: too many files open\n", color_Red, color_Normal);
         exit(-1);
      }
   }
   f->file = source_mgr_SourceMgr_openInternal(sm, string_pool_Pool_idx2str(sm->pool, f->filename), 0);
   if (!file_utils_Reader_isOpen(&f->file)) exit(-1);
}

static const char* source_mgr_SourceMgr_get_content(source_mgr_SourceMgr* sm, int32_t handle)
{
   source_mgr_SourceMgr_checkOpen(sm, handle);
   return ((char*)(file_utils_Reader_data(&sm->files[handle].file)));
}

static const char* source_mgr_SourceMgr_get_token_source(source_mgr_SourceMgr* sm, src_loc_SrcLoc loc)
{
   source_mgr_File* f = source_mgr_SourceMgr_find_file(sm, loc);
   if (!f) return "";

   uint32_t offset = loc - f->offset;
   char* data = f->file.region;
   data += offset;
   return data;
}

static uint32_t source_mgr_SourceMgr_get_offset(source_mgr_SourceMgr* sm, int32_t handle)
{
   return sm->files[handle].offset;
}

static const char* source_mgr_SourceMgr_getFileName(source_mgr_SourceMgr* sm, int32_t handle)
{
   uint32_t idx = sm->files[handle].filename;
   return string_pool_Pool_idx2str(sm->pool, idx);
}

static source_mgr_File* source_mgr_SourceMgr_find_file(source_mgr_SourceMgr* sm, src_loc_SrcLoc loc)
{
   for (uint32_t i = 0; i < sm->num_files; i++) {
      source_mgr_File* f = &sm->files[i];
      if ((loc >= f->offset && loc < f->offset + f->file.size)) {
         source_mgr_SourceMgr_checkOpen(sm, ((int32_t)(i)));
         return f;
      }
   }
   return NULL;
}

static void source_mgr_find_line_col(const char* data, uint32_t offset, source_mgr_Location* loc, uint32_t last_offset)
{
   const char* line;
   uint32_t line_nr = loc->line;
   if (last_offset) {
      line = data + last_offset - loc->column + 1;
   } else {
      line = data;
   }
   for (uint32_t i = last_offset; i < offset; i++) {
      if (data[i] == '\n') {
         line_nr++;
         line = data + i + 1;
      }
   }
   loc->line = line_nr;
   loc->column = ((uint32_t)(&data[offset] - line)) + 1;
   loc->line_start = line;
}

static source_mgr_Location source_mgr_SourceMgr_locate(source_mgr_SourceMgr* sm, src_loc_SrcLoc loc)
{
   source_mgr_Location l = { 1, 1, NULL };
   source_mgr_File* f = source_mgr_SourceMgr_find_file(sm, loc);
   if (f) {
      l.filename = string_pool_Pool_idx2str(sm->pool, f->filename);
      uint32_t offset = loc - f->offset;
      uint32_t last_offset = 0;
      if ((f->last_offset != 0 && offset > f->last_offset)) {
         l = f->last_loc;
         last_offset = f->last_offset;
      }
      source_mgr_find_line_col(((char*)(file_utils_Reader_data(&f->file))), offset, &l, last_offset);
      f->last_offset = offset;
      f->last_loc = l;
   }
   return l;
}

static source_mgr_Location source_mgr_SourceMgr_getLocation(source_mgr_SourceMgr* sm, src_loc_SrcLoc sloc)
{
   source_mgr_Location loc = { };
   if (sloc == 0) {
      loc.filename = "-";
   } else {
      loc = source_mgr_SourceMgr_locate(sm, sloc);
   }
   return loc;
}

static const char* source_mgr_SourceMgr_loc2str(source_mgr_SourceMgr* sm, src_loc_SrcLoc sloc)
{
   static char tmp[256];
   if (sloc == 0) {
      strcpy(tmp, "-");
   } else {
      source_mgr_Location loc = source_mgr_SourceMgr_locate(sm, sloc);
      sprintf(tmp, "%s:%u:%u", loc.filename, loc.line, loc.column);
   }
   return tmp;
}

static void source_mgr_SourceMgr_report(const source_mgr_SourceMgr* sm)
{
   printf("source-mgr: %u files, %u sources (%u bytes), %u other (%u bytes)\n", sm->num_files, sm->sources_count, sm->sources_size, sm->other_count, sm->other_size);
}


// --- module string_list ---

typedef struct string_list_List_ string_list_List;

struct string_list_List_ {
   const string_pool_Pool* pool;
   uint32_t* indexes;
   uint32_t count;
   uint32_t capacity;
};

static void string_list_List_init(string_list_List* l, const string_pool_Pool* pool);
static void string_list_List_free(string_list_List* l);
static void string_list_List_clear(string_list_List* l);
static void string_list_List_resize(string_list_List* l, uint32_t capacity);
static void string_list_List_add(string_list_List* l, uint32_t name_idx);
static bool string_list_List_contains(const string_list_List* l, const char* name);
static uint32_t string_list_List_length(const string_list_List* l);
static const char* string_list_List_get(const string_list_List* l, uint32_t idx);

static void string_list_List_init(string_list_List* l, const string_pool_Pool* pool)
{
   memset(l, 0, sizeof(string_list_List));
   l->pool = pool;
   string_list_List_resize(l, 16);
}

static void string_list_List_free(string_list_List* l)
{
   free(l->indexes);
}

static void string_list_List_clear(string_list_List* l)
{
   l->count = 0;
}

static void string_list_List_resize(string_list_List* l, uint32_t capacity)
{
   l->capacity = capacity;
   uint32_t* indexes2 = malloc(capacity * sizeof(uint32_t));
   if (l->count) {
      memcpy(indexes2, l->indexes, l->count * sizeof(uint32_t));
      free(l->indexes);
   }
   l->indexes = indexes2;
}

static void string_list_List_add(string_list_List* l, uint32_t name_idx)
{
   if (l->count == l->capacity) string_list_List_resize(l, l->capacity * 2);
   l->indexes[l->count] = name_idx;
   l->count++;
}

static bool string_list_List_contains(const string_list_List* l, const char* name)
{
   for (uint32_t i = 0; i < l->count; i++) {
      if (strcmp(string_pool_Pool_idx2str(l->pool, l->indexes[i]), name) == 0) return true;

   }
   return false;
}

static uint32_t string_list_List_length(const string_list_List* l)
{
   return l->count;
}

static const char* string_list_List_get(const string_list_List* l, uint32_t idx)
{
   return string_pool_Pool_idx2str(l->pool, l->indexes[idx]);
}


// --- module token ---

typedef struct token_Token_ token_Token;

typedef enum {
   token_Kind_None,
   token_Kind_Identifier,
   token_Kind_NumberLiteral,
   token_Kind_CharLiteral,
   token_Kind_StringLiteral,
   token_Kind_LParen,
   token_Kind_RParen,
   token_Kind_LSquare,
   token_Kind_RSquare,
   token_Kind_LBrace,
   token_Kind_RBrace,
   token_Kind_Exclaim,
   token_Kind_ExclaimEqual,
   token_Kind_Star,
   token_Kind_StarEqual,
   token_Kind_Amp,
   token_Kind_AmpAmp,
   token_Kind_AmpEqual,
   token_Kind_Pipe,
   token_Kind_PipePipe,
   token_Kind_PipeEqual,
   token_Kind_Equal,
   token_Kind_EqualEqual,
   token_Kind_Semicolon,
   token_Kind_Colon,
   token_Kind_At,
   token_Kind_Caret,
   token_Kind_CaretEqual,
   token_Kind_Question,
   token_Kind_Dot,
   token_Kind_Ellipsis,
   token_Kind_Comma,
   token_Kind_Plus,
   token_Kind_PlusPlus,
   token_Kind_PlusEqual,
   token_Kind_Minus,
   token_Kind_MinusMinus,
   token_Kind_MinusEqual,
   token_Kind_Tilde,
   token_Kind_Slash,
   token_Kind_SlashEqual,
   token_Kind_Percent,
   token_Kind_PercentEqual,
   token_Kind_Less,
   token_Kind_LessLess,
   token_Kind_LessEqual,
   token_Kind_LessLessEqual,
   token_Kind_Greater,
   token_Kind_GreaterGreater,
   token_Kind_GreaterEqual,
   token_Kind_GreaterGreaterEqual,
   token_Kind_KW_bool,
   token_Kind_KW_char,
   token_Kind_KW_i8,
   token_Kind_KW_i16,
   token_Kind_KW_i32,
   token_Kind_KW_i64,
   token_Kind_KW_u8,
   token_Kind_KW_u16,
   token_Kind_KW_u32,
   token_Kind_KW_u64,
   token_Kind_KW_reg8,
   token_Kind_KW_reg16,
   token_Kind_KW_reg32,
   token_Kind_KW_reg64,
   token_Kind_KW_isize,
   token_Kind_KW_usize,
   token_Kind_KW_f32,
   token_Kind_KW_f64,
   token_Kind_KW_void,
   token_Kind_KW_as,
   token_Kind_KW_asm,
   token_Kind_KW_assert,
   token_Kind_KW_auto,
   token_Kind_KW_break,
   token_Kind_KW_case,
   token_Kind_KW_cast,
   token_Kind_KW_const,
   token_Kind_KW_continue,
   token_Kind_KW_default,
   token_Kind_KW_do,
   token_Kind_KW_elemsof,
   token_Kind_KW_else,
   token_Kind_KW_enum_max,
   token_Kind_KW_enum_min,
   token_Kind_KW_enum,
   token_Kind_KW_fallthrough,
   token_Kind_KW_false,
   token_Kind_KW_for,
   token_Kind_KW_func,
   token_Kind_KW_goto,
   token_Kind_KW_if,
   token_Kind_KW_import,
   token_Kind_KW_local,
   token_Kind_KW_module,
   token_Kind_KW_nil,
   token_Kind_KW_offsetof,
   token_Kind_KW_public,
   token_Kind_KW_return,
   token_Kind_KW_sizeof,
   token_Kind_KW_sswitch,
   token_Kind_KW_static_assert,
   token_Kind_KW_struct,
   token_Kind_KW_switch,
   token_Kind_KW_template,
   token_Kind_KW_to_container,
   token_Kind_KW_true,
   token_Kind_KW_type,
   token_Kind_KW_union,
   token_Kind_KW_volatile,
   token_Kind_KW_while,
   token_Kind_Eof,
   token_Kind_Warning,
   token_Kind_Error,
} __attribute__((packed)) token_Kind;

struct token_Token_ {
   src_loc_SrcLoc loc;
   token_Kind kind;
   bool more;
   uint8_t radix;
   union {
      const char* error_msg;
      struct {
         uint32_t text_idx;
         uint32_t text_len;
      };
      uint64_t number_value;
      uint8_t char_value;
   };
};

static const char* token_kind2str(token_Kind kind);
static void token_Token_init(token_Token* tok);

static const char* token_token_names[] = {
   "none",
   "identifier",
   "number",
   "char",
   "string",
   "(",
   ")",
   "[",
   "]",
   "{",
   "}",
   "!",
   "!=",
   "*",
   "*=",
   "&",
   "&&",
   "&=",
   "|",
   "||",
   "|=",
   "=",
   "==",
   ";",
   ":",
   "@",
   "^",
   "^=",
   "?",
   ".",
   "...",
   ",",
   "+",
   "++",
   "+=",
   "-",
   "--",
   "-=",
   "~",
   "/",
   "/=",
   "%",
   "%=",
   "<",
   "<<",
   "<=",
   "<<=",
   ">",
   ">>",
   ">=",
   ">>=",
   "bool",
   "char",
   "i8",
   "i16",
   "i32",
   "i64",
   "u8",
   "u16",
   "u32",
   "u64",
   "reg8",
   "reg16",
   "reg32",
   "reg64",
   "isize",
   "usize",
   "f32",
   "f64",
   "void",
   "as",
   "asm",
   "assert",
   "auto",
   "break",
   "case",
   "cast",
   "const",
   "continue",
   "default",
   "do",
   "elemsof",
   "else",
   "enum_max",
   "enum_min",
   "enum",
   "falltrhough",
   "false",
   "for",
   "func",
   "goto",
   "if",
   "import",
   "local",
   "module",
   "nil",
   "offsetof",
   "public",
   "return",
   "sizeof",
   "sswitch",
   "static_assert",
   "struct",
   "switch",
   "template",
   "to_container",
   "true",
   "type",
   "union",
   "volatile",
   "while",
   "eof",
   "warning",
   "error"
};

static const char* token_kind2str(token_Kind kind)
{
   return token_token_names[kind];
}

static void token_Token_init(token_Token* tok)
{
   memset(tok, 0, sizeof(token_Token));
   tok->more = true;
}


// --- module c2_tokenizer ---

typedef struct c2_tokenizer_Keyword_ c2_tokenizer_Keyword;
typedef struct c2_tokenizer_Feature_ c2_tokenizer_Feature;
typedef struct c2_tokenizer_Tokenizer_ c2_tokenizer_Tokenizer;

struct c2_tokenizer_Keyword_ {
   const char* name;
   token_Kind kind;
   uint8_t len;
};

typedef enum {
   c2_tokenizer_Action_INVALID = 0,
   c2_tokenizer_Action_TABSPACE,
   c2_tokenizer_Action_IDENT_OR_KEYWORD,
   c2_tokenizer_Action_IDENT,
   c2_tokenizer_Action_DIGIT,
   c2_tokenizer_Action_LPAREN,
   c2_tokenizer_Action_RPAREN,
   c2_tokenizer_Action_LSQUARE,
   c2_tokenizer_Action_RSQUARE,
   c2_tokenizer_Action_NEWLINE,
   c2_tokenizer_Action_EXCLAIM,
   c2_tokenizer_Action_DQUOTE,
   c2_tokenizer_Action_SQUOTE,
   c2_tokenizer_Action_POUND,
   c2_tokenizer_Action_STAR,
   c2_tokenizer_Action_PLUS,
   c2_tokenizer_Action_MINUS,
   c2_tokenizer_Action_COMMA,
   c2_tokenizer_Action_DOT,
   c2_tokenizer_Action_PERCENT,
   c2_tokenizer_Action_SLASH,
   c2_tokenizer_Action_COLON,
   c2_tokenizer_Action_SEMI_COLON,
   c2_tokenizer_Action_LESS,
   c2_tokenizer_Action_EQUAL,
   c2_tokenizer_Action_GREATER,
   c2_tokenizer_Action_QUESTION,
   c2_tokenizer_Action_AT,
   c2_tokenizer_Action_AMP,
   c2_tokenizer_Action_CARET,
   c2_tokenizer_Action_LBRACE,
   c2_tokenizer_Action_RBRACE,
   c2_tokenizer_Action_PIPE,
   c2_tokenizer_Action_TILDE,
   c2_tokenizer_Action_CR,
   c2_tokenizer_Action_EOF,
} __attribute__((packed)) c2_tokenizer_Action;

struct c2_tokenizer_Feature_ {
   bool is_if;
   bool enabled;
};

struct c2_tokenizer_Tokenizer_ {
   const char* cur;
   src_loc_SrcLoc loc_start;
   const char* input_start;
   token_Token next[16];
   uint32_t next_count;
   uint32_t next_head;
   const char* line_start;
   string_pool_Pool* pool;
   c2_tokenizer_Feature feature_stack[6];
   uint32_t feature_count;
   const string_list_List* features;
   char error_msg[256];
};

static const c2_tokenizer_Keyword* c2_tokenizer_check_keyword(const char* cp);
static void c2_tokenizer_Tokenizer_init(c2_tokenizer_Tokenizer* t, string_pool_Pool* pool, const char* input, src_loc_SrcLoc loc_start, const string_list_List* features);
static src_loc_SrcLoc c2_tokenizer_getTokenEnd(const char* input, src_loc_SrcLoc start);
static void c2_tokenizer_Tokenizer_lex(c2_tokenizer_Tokenizer* t, token_Token* result);
static void c2_tokenizer_Tokenizer_lex_internal(c2_tokenizer_Tokenizer* t, token_Token* result);
static token_Token c2_tokenizer_Tokenizer_lookahead(c2_tokenizer_Tokenizer* t, uint32_t n);
static void c2_tokenizer_Tokenizer_error(c2_tokenizer_Tokenizer* t, token_Token* result, const char* format, ...);
static void c2_tokenizer_Tokenizer_lex_identifier(c2_tokenizer_Tokenizer* t, token_Token* result);
static bool c2_tokenizer_is_octal(char c);
static bool c2_tokenizer_is_binary(char c);
static void c2_tokenizer_Tokenizer_lex_number(c2_tokenizer_Tokenizer* t, token_Token* result);
static void c2_tokenizer_Tokenizer_lex_char_literal(c2_tokenizer_Tokenizer* t, token_Token* result);
static void c2_tokenizer_Tokenizer_lex_string_literal(c2_tokenizer_Tokenizer* t, token_Token* result);
static void c2_tokenizer_Tokenizer_skip_line_comment(c2_tokenizer_Tokenizer* t);
static bool c2_tokenizer_Tokenizer_skip_block_comment(c2_tokenizer_Tokenizer* t, token_Token* result);
static bool c2_tokenizer_Tokenizer_skip_line(c2_tokenizer_Tokenizer* t, token_Token* result);
static bool c2_tokenizer_compare_word(const char* cur, const char* expect);
static bool c2_tokenizer_Tokenizer_lex_feature_cmd(c2_tokenizer_Tokenizer* t, token_Token* result);
static bool c2_tokenizer_Tokenizer_parse_error_warn(c2_tokenizer_Tokenizer* t, token_Token* result, bool is_error);
static bool c2_tokenizer_Tokenizer_is_enabled(const c2_tokenizer_Tokenizer* t);
static bool c2_tokenizer_Tokenizer_handle_if(c2_tokenizer_Tokenizer* t, token_Token* result);
static bool c2_tokenizer_Tokenizer_parse_feature(c2_tokenizer_Tokenizer* t, token_Token* result, bool* enabled);
static bool c2_tokenizer_Tokenizer_handle_else(c2_tokenizer_Tokenizer* t, token_Token* result);
static bool c2_tokenizer_Tokenizer_handle_endif(c2_tokenizer_Tokenizer* t, token_Token* result);
static bool c2_tokenizer_Tokenizer_skip_feature(c2_tokenizer_Tokenizer* t, token_Token* result);
static void c2_tokenizer_Tokenizer_skip_string_literal(c2_tokenizer_Tokenizer* t);
static void c2_tokenizer_Tokenizer_skip_char_literal(c2_tokenizer_Tokenizer* t);

static const uint32_t c2_tokenizer_MaxLookahead = 16;

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_a[] = {
   { "as", token_Kind_KW_as, 2 },
   { "asm", token_Kind_KW_asm, 3 },
   { "assert", token_Kind_KW_assert, 6 },
   { "auto", token_Kind_KW_auto, 4 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_b[] = {
   { "bool", token_Kind_KW_bool, 4 },
   { "break", token_Kind_KW_break, 5 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_c[] = {
   { "case", token_Kind_KW_case, 4 },
   { "cast", token_Kind_KW_cast, 4 },
   { "char", token_Kind_KW_char, 4 },
   { "const", token_Kind_KW_const, 5 },
   { "continue", token_Kind_KW_continue, 8 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_d[] = {
   { "default", token_Kind_KW_default, 7 },
   { "do", token_Kind_KW_do, 2 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_e[] = {
   { "elemsof", token_Kind_KW_elemsof, 7 },
   { "else", token_Kind_KW_else, 4 },
   { "enum", token_Kind_KW_enum, 4 },
   { "enum_max", token_Kind_KW_enum_max, 8 },
   { "enum_min", token_Kind_KW_enum_min, 8 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_f[] = {
   { "f32", token_Kind_KW_f32, 3 },
   { "f64", token_Kind_KW_f64, 3 },
   { "fallthrough", token_Kind_KW_fallthrough, 11 },
   { "false", token_Kind_KW_false, 5 },
   { "for", token_Kind_KW_for, 3 },
   { "func", token_Kind_KW_func, 4 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_g[] = {
   { "goto", token_Kind_KW_goto, 4 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_i[] = {
   { "i16", token_Kind_KW_i16, 3 },
   { "i32", token_Kind_KW_i32, 3 },
   { "i64", token_Kind_KW_i64, 3 },
   { "i8", token_Kind_KW_i8, 2 },
   { "if", token_Kind_KW_if, 2 },
   { "import", token_Kind_KW_import, 6 },
   { "isize", token_Kind_KW_isize, 5 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_l[] = {
   { "local", token_Kind_KW_local, 5 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_m[] = {
   { "module", token_Kind_KW_module, 6 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_n[] = {
   { "nil", token_Kind_KW_nil, 3 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_o[] = {
   { "offsetof", token_Kind_KW_offsetof, 8 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_p[] = {
   { "public", token_Kind_KW_public, 6 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_r[] = {
   { "reg16", token_Kind_KW_reg16, 5 },
   { "reg32", token_Kind_KW_reg32, 5 },
   { "reg64", token_Kind_KW_reg64, 5 },
   { "reg8", token_Kind_KW_reg8, 4 },
   { "return", token_Kind_KW_return, 6 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_s[] = {
   { "sizeof", token_Kind_KW_sizeof, 6 },
   { "sswitch", token_Kind_KW_sswitch, 7 },
   { "static_assert", token_Kind_KW_static_assert, 13 },
   { "struct", token_Kind_KW_struct, 6 },
   { "switch", token_Kind_KW_switch, 6 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_t[] = {
   { "template", token_Kind_KW_template, 8 },
   { "to_container", token_Kind_KW_to_container, 12 },
   { "true", token_Kind_KW_true, 4 },
   { "type", token_Kind_KW_type, 4 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_u[] = {
   { "u16", token_Kind_KW_u16, 3 },
   { "u32", token_Kind_KW_u32, 3 },
   { "u64", token_Kind_KW_u64, 3 },
   { "u8", token_Kind_KW_u8, 2 },
   { "union", token_Kind_KW_union, 5 },
   { "usize", token_Kind_KW_usize, 5 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_v[] = {
   { "void", token_Kind_KW_void, 4 },
   { "volatile", token_Kind_KW_volatile, 8 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword c2_tokenizer_Keywords_w[] = {
   { "while", token_Kind_KW_while, 5 },
   { NULL, token_Kind_None, 0 }
};

static const c2_tokenizer_Keyword* c2_tokenizer_keywords[] = {
   c2_tokenizer_Keywords_a,
   c2_tokenizer_Keywords_b,
   c2_tokenizer_Keywords_c,
   c2_tokenizer_Keywords_d,
   c2_tokenizer_Keywords_e,
   c2_tokenizer_Keywords_f,
   c2_tokenizer_Keywords_g,
   NULL,
   c2_tokenizer_Keywords_i,
   NULL,
   NULL,
   c2_tokenizer_Keywords_l,
   c2_tokenizer_Keywords_m,
   c2_tokenizer_Keywords_n,
   c2_tokenizer_Keywords_o,
   c2_tokenizer_Keywords_p,
   NULL,
   c2_tokenizer_Keywords_r,
   c2_tokenizer_Keywords_s,
   c2_tokenizer_Keywords_t,
   c2_tokenizer_Keywords_u,
   c2_tokenizer_Keywords_v,
   c2_tokenizer_Keywords_w,
   NULL,
   NULL,
   NULL
};

static const c2_tokenizer_Action c2_tokenizer_Char_lookup[128] = {
   [0] = c2_tokenizer_Action_EOF,
   ['\t'] = c2_tokenizer_Action_TABSPACE,
   ['\n'] = c2_tokenizer_Action_NEWLINE,
   ['\r'] = c2_tokenizer_Action_CR,
   [' '] = c2_tokenizer_Action_TABSPACE,
   ['!'] = c2_tokenizer_Action_EXCLAIM,
   ['"'] = c2_tokenizer_Action_DQUOTE,
   ['#'] = c2_tokenizer_Action_POUND,
   ['%'] = c2_tokenizer_Action_PERCENT,
   ['&'] = c2_tokenizer_Action_AMP,
   ['\''] = c2_tokenizer_Action_SQUOTE,
   ['('] = c2_tokenizer_Action_LPAREN,
   [')'] = c2_tokenizer_Action_RPAREN,
   ['*'] = c2_tokenizer_Action_STAR,
   ['+'] = c2_tokenizer_Action_PLUS,
   [','] = c2_tokenizer_Action_COMMA,
   ['-'] = c2_tokenizer_Action_MINUS,
   ['.'] = c2_tokenizer_Action_DOT,
   ['/'] = c2_tokenizer_Action_SLASH,
   ['0'] = c2_tokenizer_Action_DIGIT,
   ['1'] = c2_tokenizer_Action_DIGIT,
   ['2'] = c2_tokenizer_Action_DIGIT,
   ['3'] = c2_tokenizer_Action_DIGIT,
   ['4'] = c2_tokenizer_Action_DIGIT,
   ['5'] = c2_tokenizer_Action_DIGIT,
   ['6'] = c2_tokenizer_Action_DIGIT,
   ['7'] = c2_tokenizer_Action_DIGIT,
   ['8'] = c2_tokenizer_Action_DIGIT,
   ['9'] = c2_tokenizer_Action_DIGIT,
   [':'] = c2_tokenizer_Action_COLON,
   [';'] = c2_tokenizer_Action_SEMI_COLON,
   ['<'] = c2_tokenizer_Action_LESS,
   ['='] = c2_tokenizer_Action_EQUAL,
   ['>'] = c2_tokenizer_Action_GREATER,
   ['?'] = c2_tokenizer_Action_QUESTION,
   ['@'] = c2_tokenizer_Action_AT,
   ['A'] = c2_tokenizer_Action_IDENT,
   ['B'] = c2_tokenizer_Action_IDENT,
   ['C'] = c2_tokenizer_Action_IDENT,
   ['D'] = c2_tokenizer_Action_IDENT,
   ['E'] = c2_tokenizer_Action_IDENT,
   ['F'] = c2_tokenizer_Action_IDENT,
   ['G'] = c2_tokenizer_Action_IDENT,
   ['H'] = c2_tokenizer_Action_IDENT,
   ['I'] = c2_tokenizer_Action_IDENT,
   ['J'] = c2_tokenizer_Action_IDENT,
   ['K'] = c2_tokenizer_Action_IDENT,
   ['L'] = c2_tokenizer_Action_IDENT,
   ['M'] = c2_tokenizer_Action_IDENT,
   ['N'] = c2_tokenizer_Action_IDENT,
   ['O'] = c2_tokenizer_Action_IDENT,
   ['P'] = c2_tokenizer_Action_IDENT,
   ['Q'] = c2_tokenizer_Action_IDENT,
   ['R'] = c2_tokenizer_Action_IDENT,
   ['S'] = c2_tokenizer_Action_IDENT,
   ['T'] = c2_tokenizer_Action_IDENT,
   ['U'] = c2_tokenizer_Action_IDENT,
   ['V'] = c2_tokenizer_Action_IDENT,
   ['W'] = c2_tokenizer_Action_IDENT,
   ['X'] = c2_tokenizer_Action_IDENT,
   ['Y'] = c2_tokenizer_Action_IDENT,
   ['Z'] = c2_tokenizer_Action_IDENT,
   ['['] = c2_tokenizer_Action_LSQUARE,
   ['\\'] = c2_tokenizer_Action_TABSPACE,
   [']'] = c2_tokenizer_Action_RSQUARE,
   ['^'] = c2_tokenizer_Action_CARET,
   ['_'] = c2_tokenizer_Action_IDENT,
   ['a'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['b'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['c'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['d'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['e'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['f'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['g'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['h'] = c2_tokenizer_Action_IDENT,
   ['i'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['j'] = c2_tokenizer_Action_IDENT,
   ['k'] = c2_tokenizer_Action_IDENT,
   ['l'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['m'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['n'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['o'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['p'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['q'] = c2_tokenizer_Action_IDENT,
   ['r'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['s'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['t'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['u'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['v'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['w'] = c2_tokenizer_Action_IDENT_OR_KEYWORD,
   ['x'] = c2_tokenizer_Action_IDENT,
   ['y'] = c2_tokenizer_Action_IDENT,
   ['z'] = c2_tokenizer_Action_IDENT,
   ['{'] = c2_tokenizer_Action_LBRACE,
   ['|'] = c2_tokenizer_Action_PIPE,
   ['}'] = c2_tokenizer_Action_RBRACE,
   ['~'] = c2_tokenizer_Action_TILDE
};

static const uint8_t c2_tokenizer_Identifier_char[128] = {
   ['0'] = 1,
   ['1'] = 1,
   ['2'] = 1,
   ['3'] = 1,
   ['4'] = 1,
   ['5'] = 1,
   ['6'] = 1,
   ['7'] = 1,
   ['8'] = 1,
   ['9'] = 1,
   ['A'] = 1,
   ['B'] = 1,
   ['C'] = 1,
   ['D'] = 1,
   ['E'] = 1,
   ['F'] = 1,
   ['G'] = 1,
   ['H'] = 1,
   ['I'] = 1,
   ['J'] = 1,
   ['K'] = 1,
   ['L'] = 1,
   ['M'] = 1,
   ['N'] = 1,
   ['O'] = 1,
   ['P'] = 1,
   ['Q'] = 1,
   ['R'] = 1,
   ['S'] = 1,
   ['T'] = 1,
   ['U'] = 1,
   ['V'] = 1,
   ['W'] = 1,
   ['X'] = 1,
   ['Y'] = 1,
   ['Z'] = 1,
   ['_'] = 1,
   ['a'] = 1,
   ['b'] = 1,
   ['c'] = 1,
   ['d'] = 1,
   ['e'] = 1,
   ['f'] = 1,
   ['g'] = 1,
   ['h'] = 1,
   ['i'] = 1,
   ['j'] = 1,
   ['k'] = 1,
   ['l'] = 1,
   ['m'] = 1,
   ['n'] = 1,
   ['o'] = 1,
   ['p'] = 1,
   ['q'] = 1,
   ['r'] = 1,
   ['s'] = 1,
   ['t'] = 1,
   ['u'] = 1,
   ['v'] = 1,
   ['w'] = 1,
   ['x'] = 1,
   ['y'] = 1,
   ['z'] = 1
};

static const c2_tokenizer_Keyword* c2_tokenizer_check_keyword(const char* cp)
{
   const c2_tokenizer_Keyword* table = c2_tokenizer_keywords[*cp - 'a'];
   uint32_t i = 0;
   while (table[i].name) {
      const char* word = cp;
      const char* kw = table[i].name;
      uint32_t idx = 0;
      while (1) {
         char a = kw[idx];
         char b = word[idx];
         if (a == 0) {
            if (!c2_tokenizer_Identifier_char[b]) return &table[i];

            break;
         }
         if (a != b) {
            if (b < a) return NULL;

            break;
         }
         idx++;
      }
      i++;
   }
   return NULL;
}

static void c2_tokenizer_Tokenizer_init(c2_tokenizer_Tokenizer* t, string_pool_Pool* pool, const char* input, src_loc_SrcLoc loc_start, const string_list_List* features)
{
   memset(t, 0, sizeof(c2_tokenizer_Tokenizer));
   t->cur = input;
   t->input_start = input;
   t->loc_start = loc_start;
   t->line_start = input;
   t->pool = pool;
   t->features = features;
   for (uint32_t i = 0; i < c2_tokenizer_MaxLookahead; i++) {
      token_Token_init(&t->next[i]);
   }
}

static src_loc_SrcLoc c2_tokenizer_getTokenEnd(const char* input, src_loc_SrcLoc start)
{
   c2_tokenizer_Tokenizer t;
   string_pool_Pool* pool = string_pool_create(128, 20);
   string_list_List features;
   string_list_List_init(&features, pool);
   c2_tokenizer_Tokenizer_init(&t, pool, input, start, &features);
   token_Token result;
   token_Token_init(&result);
   c2_tokenizer_Tokenizer_lex(&t, &result);
   string_list_List_free(&features);
   string_pool_Pool_free(pool);
   return start + ((src_loc_SrcLoc)(t.cur - t.input_start)) - 1;
}

static void c2_tokenizer_Tokenizer_lex(c2_tokenizer_Tokenizer* t, token_Token* result)
{
   if (t->next_count) {
      memcpy(result, &t->next[t->next_head], sizeof(token_Token));
      t->next_head = (t->next_head + 1) % c2_tokenizer_MaxLookahead;
      t->next_count--;
      return;
   }
   c2_tokenizer_Tokenizer_lex_internal(t, result);
}

static void c2_tokenizer_Tokenizer_lex_internal(c2_tokenizer_Tokenizer* t, token_Token* result)
{
   while (1) {
      c2_tokenizer_Action act = c2_tokenizer_Char_lookup[*t->cur];
      switch (act) {
      case c2_tokenizer_Action_INVALID:
         c2_tokenizer_Tokenizer_error(t, result, "invalid char '%c'", *t->cur);
         return;
      case c2_tokenizer_Action_TABSPACE:
         t->cur++;
         continue;
      case c2_tokenizer_Action_IDENT_OR_KEYWORD: {
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         const c2_tokenizer_Keyword* kw = c2_tokenizer_check_keyword(t->cur);
         if (kw) {
            result->kind = kw->kind;
            t->cur += kw->len;
         } else {
            c2_tokenizer_Tokenizer_lex_identifier(t, result);
         }
         return;
      }
      case c2_tokenizer_Action_IDENT:
         c2_tokenizer_Tokenizer_lex_identifier(t, result);
         return;
      case c2_tokenizer_Action_DIGIT:
         c2_tokenizer_Tokenizer_lex_number(t, result);
         return;
      case c2_tokenizer_Action_LPAREN:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         result->kind = token_Kind_LParen;
         t->cur++;
         return;
      case c2_tokenizer_Action_RPAREN:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         result->kind = token_Kind_RParen;
         t->cur++;
         return;
      case c2_tokenizer_Action_LSQUARE:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         result->kind = token_Kind_LSquare;
         t->cur++;
         return;
      case c2_tokenizer_Action_RSQUARE:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         result->kind = token_Kind_RSquare;
         t->cur++;
         return;
      case c2_tokenizer_Action_NEWLINE:
         t->cur++;
         t->line_start = t->cur;
         continue;
      case c2_tokenizer_Action_EXCLAIM:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         t->cur++;
         if (*t->cur == '=') {
            result->kind = token_Kind_ExclaimEqual;
            t->cur++;
         } else {
            result->kind = token_Kind_Exclaim;
         }
         return;
      case c2_tokenizer_Action_DQUOTE:
         c2_tokenizer_Tokenizer_lex_string_literal(t, result);
         return;
      case c2_tokenizer_Action_SQUOTE:
         c2_tokenizer_Tokenizer_lex_char_literal(t, result);
         return;
      case c2_tokenizer_Action_POUND:
         if (c2_tokenizer_Tokenizer_lex_feature_cmd(t, result)) return;

         if (!c2_tokenizer_Tokenizer_is_enabled(t)) {
            if (c2_tokenizer_Tokenizer_skip_feature(t, result)) return;

         }
         continue;
      case c2_tokenizer_Action_STAR:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         t->cur++;
         if (*t->cur == '=') {
            result->kind = token_Kind_StarEqual;
            t->cur++;
         } else {
            result->kind = token_Kind_Star;
         }
         return;
      case c2_tokenizer_Action_PLUS:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         t->cur++;
         if (*t->cur == '+') {
            t->cur++;
            result->kind = token_Kind_PlusPlus;
            return;
         }
         if (*t->cur == '=') {
            t->cur++;
            result->kind = token_Kind_PlusEqual;
            return;
         }
         result->kind = token_Kind_Plus;
         return;
      case c2_tokenizer_Action_MINUS:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         t->cur++;
         if (*t->cur == '-') {
            t->cur++;
            result->kind = token_Kind_MinusMinus;
            return;
         }
         if (*t->cur == '=') {
            t->cur++;
            result->kind = token_Kind_MinusEqual;
            return;
         }
         if (*t->cur == '>') {
            t->cur--;
            c2_tokenizer_Tokenizer_error(t, result, "use the dot operators instead of '->'");
            return;
         }
         result->kind = token_Kind_Minus;
         return;
      case c2_tokenizer_Action_COMMA:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         result->kind = token_Kind_Comma;
         t->cur++;
         return;
      case c2_tokenizer_Action_DOT:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         t->cur++;
         if ((t->cur[0] == '.' && t->cur[1] == '.')) {
            t->cur += 2;
            result->kind = token_Kind_Ellipsis;
         } else {
            result->kind = token_Kind_Dot;
         }
         return;
      case c2_tokenizer_Action_PERCENT:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         t->cur++;
         if (*t->cur == '=') {
            result->kind = token_Kind_PercentEqual;
            t->cur++;
            return;
         }
         result->kind = token_Kind_Percent;
         return;
      case c2_tokenizer_Action_SLASH:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         t->cur++;
         if (*t->cur == '=') {
            result->kind = token_Kind_SlashEqual;
            t->cur++;
            return;
         }
         if (*t->cur == '/') {
            c2_tokenizer_Tokenizer_skip_line_comment(t);
            continue;
         }
         if (*t->cur == '*') {
            if (c2_tokenizer_Tokenizer_skip_block_comment(t, result)) return;

            continue;
         }
         result->kind = token_Kind_Slash;
         return;
      case c2_tokenizer_Action_COLON:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         result->kind = token_Kind_Colon;
         t->cur++;
         return;
      case c2_tokenizer_Action_SEMI_COLON:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         result->kind = token_Kind_Semicolon;
         t->cur++;
         return;
      case c2_tokenizer_Action_LESS:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         t->cur++;
         if (*t->cur == '=') {
            t->cur++;
            result->kind = token_Kind_LessEqual;
            return;
         }
         if (*t->cur == '<') {
            t->cur++;
            if (*t->cur == '=') {
               t->cur++;
               result->kind = token_Kind_LessLessEqual;
            }
            result->kind = token_Kind_LessLess;
            return;
         }
         result->kind = token_Kind_Less;
         return;
      case c2_tokenizer_Action_EQUAL:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         t->cur++;
         if (*t->cur == '=') {
            result->kind = token_Kind_EqualEqual;
            t->cur++;
         } else {
            result->kind = token_Kind_Equal;
         }
         return;
      case c2_tokenizer_Action_GREATER:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         t->cur++;
         if (*t->cur == '=') {
            t->cur++;
            result->kind = token_Kind_GreaterEqual;
            return;
         }
         if (*t->cur == '>') {
            t->cur++;
            if (*t->cur == '=') {
               t->cur++;
               result->kind = token_Kind_GreaterGreaterEqual;
            }
            result->kind = token_Kind_GreaterGreater;
            return;
         }
         result->kind = token_Kind_Greater;
         return;
      case c2_tokenizer_Action_QUESTION:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         result->kind = token_Kind_Question;
         t->cur++;
         return;
      case c2_tokenizer_Action_AT:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         result->kind = token_Kind_At;
         t->cur++;
         return;
      case c2_tokenizer_Action_AMP:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         t->cur++;
         if (*t->cur == '&') {
            result->kind = token_Kind_AmpAmp;
            t->cur++;
            return;
         }
         if (*t->cur == '=') {
            result->kind = token_Kind_AmpEqual;
            t->cur++;
            return;
         }
         result->kind = token_Kind_Amp;
         return;
      case c2_tokenizer_Action_CARET:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         t->cur++;
         if (*t->cur == '=') {
            t->cur++;
            result->kind = token_Kind_CaretEqual;
            return;
         }
         result->kind = token_Kind_Caret;
         return;
      case c2_tokenizer_Action_LBRACE:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         result->kind = token_Kind_LBrace;
         t->cur++;
         return;
      case c2_tokenizer_Action_RBRACE:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         result->kind = token_Kind_RBrace;
         t->cur++;
         return;
      case c2_tokenizer_Action_PIPE:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         t->cur++;
         if (*t->cur == '|') {
            result->kind = token_Kind_PipePipe;
            t->cur++;
            return;
         }
         if (*t->cur == '=') {
            result->kind = token_Kind_PipeEqual;
            t->cur++;
            return;
         }
         result->kind = token_Kind_Pipe;
         return;
      case c2_tokenizer_Action_TILDE:
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
         result->kind = token_Kind_Tilde;
         t->cur++;
         return;
      case c2_tokenizer_Action_CR:
         t->cur++;
         if (*t->cur != '\n') {
            c2_tokenizer_Tokenizer_error(t, result, "unexpected char 0x%02X", *t->cur);
            return;
         }
         t->cur++;
         return;
      case c2_tokenizer_Action_EOF:
         result->loc = 0;
         result->kind = token_Kind_Eof;
         result->more = false;
         return;
      }
   }
}

static token_Token c2_tokenizer_Tokenizer_lookahead(c2_tokenizer_Tokenizer* t, uint32_t n)
{
   assert(n > 0);
   assert(n <= c2_tokenizer_MaxLookahead);
   while (t->next_count < n) {
      const uint32_t slot = (t->next_head + t->next_count) % c2_tokenizer_MaxLookahead;
      c2_tokenizer_Tokenizer_lex_internal(t, &t->next[slot]);
      t->next_count++;
   }
   uint32_t slot = (t->next_head + n - 1) % c2_tokenizer_MaxLookahead;
   return t->next[slot];
}

static void c2_tokenizer_Tokenizer_error(c2_tokenizer_Tokenizer* t, token_Token* result, const char* format, ...)
{
   va_list args;
   va_start(args, format);
   vsnprintf(t->error_msg, sizeof(t->error_msg) - 1, format, args);
   va_end(args);
   result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
   result->kind = token_Kind_Error;
   result->error_msg = t->error_msg;
   result->more = false;
}

static void c2_tokenizer_Tokenizer_lex_identifier(c2_tokenizer_Tokenizer* t, token_Token* result)
{
   result->kind = token_Kind_Identifier;
   result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
   const char* start = t->cur;
   const char* end = t->cur + 1;
   while (c2_tokenizer_Identifier_char[*end]) end++;
   size_t len = ((size_t)(end - start));
   if (len > constants_MaxIdentifierLen) {
      c2_tokenizer_Tokenizer_error(t, result, "identifier too long (max %u)", constants_MaxIdentifierLen);
      return;
   }
   t->cur += len;
   result->text_idx = string_pool_Pool_add(t->pool, start, len, true);
}

static bool c2_tokenizer_is_octal(char c)
{
   return ((c >= '0' && c <= '7'));
}

static bool c2_tokenizer_is_binary(char c)
{
   return ((c >= '0' && c <= '1'));
}

static void c2_tokenizer_Tokenizer_lex_number(c2_tokenizer_Tokenizer* t, token_Token* result)
{
   result->kind = token_Kind_NumberLiteral;
   result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
   const char* start;
   if (t->cur[0] == '0') {
      if (t->cur[1] == 'x') {
         result->radix = 16;
         t->cur += 2;
         start = t->cur;
         while (isxdigit(*t->cur)) t->cur++;
         if (isalpha(*t->cur)) {
            c2_tokenizer_Tokenizer_error(t, result, "invalid letter '%c' in hexadecimal constant", *t->cur);
            return;
         }
         result->number_value = strtoull(start, NULL, 16);
         return;
      }
      if (c2_tokenizer_is_octal(t->cur[1])) {
         result->radix = 8;
         t->cur++;
         start = t->cur;
         while (c2_tokenizer_is_octal(*t->cur)) t->cur++;
         if (isxdigit(*t->cur)) {
            c2_tokenizer_Tokenizer_error(t, result, "invalid digit '%c' in octal constant", *t->cur);
            return;
         }
         result->number_value = strtoull(start, NULL, 8);
         return;
      }
      if (t->cur[1] == 'b') {
         result->radix = 2;
         t->cur += 2;
         start = t->cur;
         while (c2_tokenizer_is_binary(*t->cur)) t->cur++;
         if (isdigit(*t->cur)) {
            c2_tokenizer_Tokenizer_error(t, result, "invalid digit '%c' in binary constant", *t->cur);
            return;
         }
         result->number_value = strtoull(start, NULL, 2);
         return;
      }
      if (isdigit(t->cur[1])) {
         c2_tokenizer_Tokenizer_error(t, result, "decimal numbers may not start with a 0");
         return;
      }
      t->cur++;
      result->radix = 10;
      result->number_value = 0;
      return;
   }
   result->radix = 10;
   start = t->cur;
   while (isdigit(*t->cur)) t->cur++;
   result->number_value = strtoull(start, NULL, 10);
}

static void c2_tokenizer_Tokenizer_lex_char_literal(c2_tokenizer_Tokenizer* t, token_Token* result)
{
   result->kind = token_Kind_CharLiteral;
   result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
   if (t->cur[1] == '\\') {
      t->cur++;
      switch (t->cur[1]) {
      case 'a':
         result->char_value = '\a';
         break;
      case 'b':
         result->char_value = '\b';
         break;
      case 'f':
         result->char_value = '\f';
         break;
      case 'n':
         result->char_value = '\n';
         break;
      case 'r':
         result->char_value = '\r';
         break;
      case 't':
         result->char_value = '\t';
         break;
      case 'v':
         result->char_value = '\v';
         break;
      case '\\':
         result->char_value = '\\';
         break;
      case '\'':
         result->char_value = '\'';
         break;
      case '"':
         result->char_value = '"';
         break;
      default:
         c2_tokenizer_Tokenizer_error(t, result, "unknown escape sequence '\\%c'", t->cur[1]);
         return;
      }
   } else {
      result->char_value = ((uint8_t)(t->cur[1]));
   }
   if (t->cur[2] != '\'') {
      if ((t->cur[2] != 0 && t->cur[3] == '\'')) {
         c2_tokenizer_Tokenizer_error(t, result, "multi-character character constant");
      } else {
         c2_tokenizer_Tokenizer_error(t, result, "missing terminating ' character");
      }
      return;
   }
   t->cur += 3;
}

static void c2_tokenizer_Tokenizer_lex_string_literal(c2_tokenizer_Tokenizer* t, token_Token* result)
{
   result->kind = token_Kind_StringLiteral;
   result->loc = t->loc_start + ((src_loc_SrcLoc)(t->cur - t->input_start));
   t->cur++;
   const char* start = t->cur;
   size_t len;
   uint32_t num_escapes = 0;
   while (1) {
      switch (*t->cur) {
      case 0:
         // fallthrough
      case '\r':
         // fallthrough
      case '\n':
         t->cur--;
         c2_tokenizer_Tokenizer_error(t, result, "unterminated string");
         return;
      case '\\':
         t->cur++;
         switch (*t->cur) {
         case '0':
            break;
         case 'a':
            break;
         case 'b':
            break;
         case 'f':
            break;
         case 'n':
            break;
         case 'r':
            break;
         case 't':
            break;
         case 'v':
            break;
         case '"':
            break;
         case '\\':
            break;
         case '\'':
            break;
         default:
            t->cur--;
            c2_tokenizer_Tokenizer_error(t, result, "unknown escape sequence '\\%c'", *t->cur);
            return;
         }
         num_escapes++;
         t->cur++;
         break;
      case '"':
         goto out;
         break;
      default:
         t->cur++;
         break;
      }
   }
   out:
   len = ((uint32_t)(t->cur - start));
   t->cur++;
   result->text_len = ((uint32_t)(len)) + 1 - num_escapes;
   result->text_idx = string_pool_Pool_add(t->pool, start, len, false);
}

static void c2_tokenizer_Tokenizer_skip_line_comment(c2_tokenizer_Tokenizer* t)
{
   t->cur += 2;
   const char* start = t->cur;
   const char* end = start;
   while (*end) {
      if ((*end == '\r' || *end == '\n')) break;

      end++;
   }
   size_t len = ((size_t)(end - start));
   t->cur += len;
}

static bool c2_tokenizer_Tokenizer_skip_block_comment(c2_tokenizer_Tokenizer* t, token_Token* result)
{
   t->cur += 2;
   while (1) {
      switch (*t->cur) {
      case 0:
         t->cur--;
         c2_tokenizer_Tokenizer_error(t, result, "un-terminated block comment");
         return true;
      case '/':
         if (t->cur[1] == '*') {
            c2_tokenizer_Tokenizer_error(t, result, "'/*' within block comment");
            return true;
         }
         break;
      case '*':
         if (t->cur[1] == '/') {
            t->cur += 2;
            return false;
         }
         break;
      default:
         break;
      }
      t->cur++;
   }
   return false;
}

static bool c2_tokenizer_Tokenizer_skip_line(c2_tokenizer_Tokenizer* t, token_Token* result)
{
   while (*t->cur != '\n') {
      if (!isspace(*t->cur)) {
         c2_tokenizer_Tokenizer_error(t, result, "unexpected character '%c'", *t->cur);
         return true;
      }
      t->cur++;
   }
   return false;
}

static bool c2_tokenizer_compare_word(const char* cur, const char* expect)
{
   while (*expect) {
      if (*cur != *expect) return false;

      cur++;
      expect++;
   }
   return !c2_tokenizer_Identifier_char[*cur];
}

static bool c2_tokenizer_Tokenizer_lex_feature_cmd(c2_tokenizer_Tokenizer* t, token_Token* result)
{
   if (t->cur != t->line_start) {
      c2_tokenizer_Tokenizer_error(t, result, "#if/#else/#endif must be at start of line");
      return true;
   }
   t->cur++;
   if (c2_tokenizer_compare_word(t->cur, "if")) {
      t->cur += 2;
      if (c2_tokenizer_Tokenizer_handle_if(t, result)) return true;

   } else if (c2_tokenizer_compare_word(t->cur, "else")) {
      t->cur += 4;
      if (c2_tokenizer_Tokenizer_skip_line(t, result)) return true;

      if (c2_tokenizer_Tokenizer_handle_else(t, result)) return true;

   } else if (c2_tokenizer_compare_word(t->cur, "endif")) {
      t->cur += 5;
      if (c2_tokenizer_Tokenizer_skip_line(t, result)) return true;

      if (c2_tokenizer_Tokenizer_handle_endif(t, result)) return true;

   } else if (c2_tokenizer_compare_word(t->cur, "error")) {
      t->cur += 5;
      return c2_tokenizer_Tokenizer_parse_error_warn(t, result, true);
   } else if (c2_tokenizer_compare_word(t->cur, "warn")) {
      t->cur += 4;
      return c2_tokenizer_Tokenizer_parse_error_warn(t, result, false);
   } else {
      c2_tokenizer_Tokenizer_error(t, result, "unknown feature-selection command");
      return true;
   }




   return false;
}

static bool c2_tokenizer_Tokenizer_parse_error_warn(c2_tokenizer_Tokenizer* t, token_Token* result, bool is_error)
{
   t->cur++;
   if (*t->cur != '"') {
      c2_tokenizer_Tokenizer_error(t, result, "expect '\"'");
      return true;
   }
   t->cur++;
   const char* start = t->cur;
   while (*t->cur != '"') {
      switch (*t->cur) {
      case 0:
         // fallthrough
      case '\r':
         // fallthrough
      case '\n':
         c2_tokenizer_Tokenizer_error(t, result, "unterminated string");
         return true;
      case '"':
         break;
      default:
         t->cur++;
         break;
      }
   }
   size_t len = ((size_t)(t->cur - start));
   t->cur++;
   if (len > constants_MaxIdentifierLen) {
      c2_tokenizer_Tokenizer_error(t, result, "error msg too long (max %u bytes)", constants_MaxErrorMsgLen);
      return true;
   }
   char msg[32];
   memcpy(msg, start, len);
   msg[len] = 0;
   if (c2_tokenizer_Tokenizer_is_enabled(t)) {
      if (is_error) {
         t->cur = t->line_start;
         c2_tokenizer_Tokenizer_error(t, result, "%s", msg);
      } else {
         strcpy(t->error_msg, msg);
         result->loc = t->loc_start + ((src_loc_SrcLoc)(t->line_start - t->input_start));
         result->kind = token_Kind_Warning;
         result->error_msg = t->error_msg;
      }
      return true;
   }
   return false;
}

static bool c2_tokenizer_Tokenizer_is_enabled(const c2_tokenizer_Tokenizer* t)
{
   for (uint32_t i = 0; i < t->feature_count; i++) {
      if (!t->feature_stack[i].enabled) return false;

   }
   return true;
}

static bool c2_tokenizer_Tokenizer_handle_if(c2_tokenizer_Tokenizer* t, token_Token* result)
{
   if (t->feature_count >= constants_MaxFeatureDepth) {
      c2_tokenizer_Tokenizer_error(t, result, "feature nesting too much");
      return true;
   }
   t->cur++;
   bool enabled = false;
   c2_tokenizer_Action act = c2_tokenizer_Char_lookup[*t->cur];
   switch (act) {
   case c2_tokenizer_Action_INVALID:
      c2_tokenizer_Tokenizer_error(t, result, "invalid char '%c'", *t->cur);
      return true;
   case c2_tokenizer_Action_IDENT_OR_KEYWORD:
      // fallthrough
   case c2_tokenizer_Action_IDENT:
      if (c2_tokenizer_Tokenizer_parse_feature(t, result, &enabled)) return true;

      break;
   case c2_tokenizer_Action_DIGIT:
      if (*t->cur != '0') enabled = true;
      t->cur++;
      break;
   case c2_tokenizer_Action_EOF:
      t->cur--;
      c2_tokenizer_Tokenizer_error(t, result, "expected feature");
      return true;
   default:
      c2_tokenizer_Tokenizer_error(t, result, "invalid feature value");
      return true;
   }
   c2_tokenizer_Feature* next = &t->feature_stack[t->feature_count];
   next->is_if = true;
   next->enabled = enabled;
   t->feature_count++;
   return false;
}

static bool c2_tokenizer_Tokenizer_parse_feature(c2_tokenizer_Tokenizer* t, token_Token* result, bool* enabled)
{
   const char* start = t->cur;
   while (c2_tokenizer_Identifier_char[*t->cur]) t->cur++;
   size_t len = ((size_t)(t->cur - start));
   if (len > constants_MaxFeatureName) {
      c2_tokenizer_Tokenizer_error(t, result, "feature name too long (max %u)", constants_MaxFeatureName);
      return true;
   }
   char name[32];
   memcpy(name, start, len);
   name[len] = 0;
   if (string_list_List_contains(t->features, name)) *enabled = true;
   return false;
}

static bool c2_tokenizer_Tokenizer_handle_else(c2_tokenizer_Tokenizer* t, token_Token* result)
{
   if (t->feature_count == 0) {
      c2_tokenizer_Tokenizer_error(t, result, "#else without #if");
      return true;
   }
   c2_tokenizer_Feature* top = &t->feature_stack[t->feature_count - 1];
   if (!top->is_if) {
      c2_tokenizer_Tokenizer_error(t, result, "#else in #else");
      return true;
   }
   top->is_if = false;
   top->enabled = !top->enabled;
   return false;
}

static bool c2_tokenizer_Tokenizer_handle_endif(c2_tokenizer_Tokenizer* t, token_Token* result)
{
   if (t->feature_count == 0) {
      c2_tokenizer_Tokenizer_error(t, result, "#endif without #if/#else");
      return true;
   }
   t->feature_count--;
   return false;
}

static bool c2_tokenizer_Tokenizer_skip_feature(c2_tokenizer_Tokenizer* t, token_Token* result)
{
   while (1) {
      c2_tokenizer_Action act = c2_tokenizer_Char_lookup[*t->cur];
      switch (act) {
      case c2_tokenizer_Action_INVALID:
         t->cur++;
         break;
      case c2_tokenizer_Action_NEWLINE:
         t->cur++;
         t->line_start = t->cur;
         break;
      case c2_tokenizer_Action_DQUOTE:
         c2_tokenizer_Tokenizer_skip_string_literal(t);
         break;
      case c2_tokenizer_Action_POUND:
         if (c2_tokenizer_Tokenizer_lex_feature_cmd(t, result)) return true;

         if (c2_tokenizer_Tokenizer_is_enabled(t)) return false;

         break;
      case c2_tokenizer_Action_EOF: {
         t->cur--;
         c2_tokenizer_Feature* top = &t->feature_stack[t->feature_count - 1];
         c2_tokenizer_Tokenizer_error(t, result, "un-terminated #%s", top->is_if ? "if" : "else");
         return true;
      }
      default:
         t->cur++;
         break;
      }
   }
   return false;
}

static void c2_tokenizer_Tokenizer_skip_string_literal(c2_tokenizer_Tokenizer* t)
{
   t->cur++;
   while (1) {
      switch (*t->cur) {
      case 0:
         return;
      case '\r':
         t->cur++;
         return;
      case '\n':
         return;
      case '"':
         t->cur++;
         return;
      default:
         t->cur++;
         break;
      }
   }
}

static void c2_tokenizer_Tokenizer_skip_char_literal(c2_tokenizer_Tokenizer* t)
{
   t->cur++;
   while (1) {
      switch (*t->cur) {
      case 0:
         return;
      case '\'':
         t->cur++;
         return;
      default:
         t->cur++;
         break;
      }
   }
}


// --- module attr ---

typedef struct attr_Value_ attr_Value;

typedef enum {
   attr_AttrKind_Unknown,
   attr_AttrKind_Export,
   attr_AttrKind_Packed,
   attr_AttrKind_Unused,
   attr_AttrKind_UnusedParams,
   attr_AttrKind_Section,
   attr_AttrKind_NoReturn,
   attr_AttrKind_Inline,
   attr_AttrKind_Aligned,
   attr_AttrKind_Weak,
   attr_AttrKind_Opaque,
   attr_AttrKind_CName,
   attr_AttrKind_NoTypeDef,
} __attribute__((packed)) attr_AttrKind;

struct attr_Value_ {
   union {
      uint32_t text_idx;
      uint32_t number;
   };
   src_loc_SrcLoc loc;
   bool is_number;
};

typedef enum {
   attr_AttrReq_NoArg = 0,
   attr_AttrReq_Arg,
   attr_AttrReq_Number,
   attr_AttrReq_String,
   attr_AttrReq_Power2,
   attr_AttrReq_Ok,
} __attribute__((packed)) attr_AttrReq;

static const char* attr_kind2name(attr_AttrKind k);
static void attr_init(string_pool_Pool* pool);
static attr_AttrKind attr_find(uint32_t name_idx);
static bool attr_isPowerOf2(uint32_t val);
static attr_AttrReq attr_check(attr_AttrKind kind, const attr_Value* value);

static const char* attr_attrKind_names[] = {
   "",
   "export",
   "packed",
   "unused",
   "unused_params",
   "section",
   "noreturn",
   "inline",
   "aligned",
   "weak",
   "opaque",
   "cname",
   "no_typedef"
};

static uint32_t attr_name_indexes[13];

static const attr_AttrReq attr_Required_arg[13] = { [attr_AttrKind_Section] = attr_AttrReq_String, [attr_AttrKind_Aligned] = attr_AttrReq_Number, [attr_AttrKind_CName] = attr_AttrReq_String };

static const char* attr_kind2name(attr_AttrKind k)
{
   return attr_attrKind_names[k];
}

static void attr_init(string_pool_Pool* pool)
{
   for (uint32_t i = 1; i < ARRAY_SIZE(attr_attrKind_names); i++) {
      attr_name_indexes[i] = string_pool_Pool_addStr(pool, attr_attrKind_names[i], true);
   }
}

static attr_AttrKind attr_find(uint32_t name_idx)
{
   for (uint32_t i = 1; i < ARRAY_SIZE(attr_name_indexes); i++) {
      if (name_idx == attr_name_indexes[i]) return ((attr_AttrKind)(i));

   }
   return attr_AttrKind_Unknown;
}

static bool attr_isPowerOf2(uint32_t val)
{
   return (val && !((val & (val - 1))));
}

static attr_AttrReq attr_check(attr_AttrKind kind, const attr_Value* value)
{
   switch (attr_Required_arg[kind]) {
   case attr_AttrReq_NoArg:
      if (value) return attr_AttrReq_NoArg;

      break;
   case attr_AttrReq_Number:
      if (!value) return attr_AttrReq_Arg;

      if (!value->is_number) return attr_AttrReq_Number;

      assert(kind == attr_AttrKind_Aligned);
      if (!attr_isPowerOf2(value->number)) return attr_AttrReq_Power2;

      break;
   case attr_AttrReq_String:
      if (!value) return attr_AttrReq_Arg;

      if (value->is_number) return attr_AttrReq_String;

      break;
   default:
      break;
   }
   return attr_AttrReq_Ok;
}


// --- module attr_table ---

typedef struct attr_table_Attr_ attr_table_Attr;
typedef struct attr_table_Table_ attr_table_Table;

struct attr_table_Attr_ {
   void* decl;
   attr_Value value;
   attr_AttrKind kind;
};

struct attr_table_Table_ {
   uint32_t count;
   uint32_t capacity;
   attr_table_Attr* attrs;
};

static attr_table_Table* attr_table_create(void);
static void attr_table_Table_free(attr_table_Table* t);
static void attr_table_Table_resize(attr_table_Table* t, uint32_t capacity);
static void attr_table_Table_add(attr_table_Table* t, void* decl, attr_AttrKind kind, const attr_Value* value);
static const attr_Value* attr_table_Table_find(const attr_table_Table* t, const void* decl, attr_AttrKind kind);

static attr_table_Table* attr_table_create(void)
{
   attr_table_Table* t = calloc(1, sizeof(attr_table_Table));
   attr_table_Table_resize(t, 2);
   return t;
}

static void attr_table_Table_free(attr_table_Table* t)
{
   free(t->attrs);
   free(t);
}

static void attr_table_Table_resize(attr_table_Table* t, uint32_t capacity)
{
   t->capacity = capacity;
   attr_table_Attr* attrs2 = malloc(capacity * sizeof(attr_table_Attr));
   if (t->count) {
      memcpy(attrs2, t->attrs, t->count * sizeof(attr_table_Attr));
      free(t->attrs);
   }
   t->attrs = attrs2;
}

static void attr_table_Table_add(attr_table_Table* t, void* decl, attr_AttrKind kind, const attr_Value* value)
{
   if (t->count == t->capacity) attr_table_Table_resize(t, t->capacity * 2);
   attr_table_Attr* a = &t->attrs[t->count];
   a->decl = decl;
   a->value = *value;
   a->kind = kind;
   t->count++;
}

static const attr_Value* attr_table_Table_find(const attr_table_Table* t, const void* decl, attr_AttrKind kind)
{
   for (uint32_t i = 0; i < t->count; i++) {
      const attr_table_Attr* a = &t->attrs[i];
      if ((a->decl == decl && a->kind == kind)) return &a->value;

   }
   return NULL;
}


// --- module c2recipe ---

typedef struct c2recipe_File_ c2recipe_File;
typedef struct c2recipe_Target_ c2recipe_Target;
typedef struct c2recipe_Recipe_ c2recipe_Recipe;

typedef struct c2recipe_Token_ c2recipe_Token;
typedef struct c2recipe_Parser_ c2recipe_Parser;


struct c2recipe_File_ {
   uint32_t name;
   src_loc_SrcLoc loc;
};

struct c2recipe_Target_ {
   c2recipe_Recipe* recipe;
   uint32_t name_idx;
   src_loc_SrcLoc loc;
   warning_flags_Flags warnings;
   string_list_List features;
   string_list_List libs;
   c2recipe_File* files;
   uint32_t num_files;
   uint32_t max_files;
};

struct c2recipe_Recipe_ {
   string_pool_Pool* pool;
   source_mgr_SourceMgr* sm;
   c2recipe_Target** targets;
   uint32_t num_targets;
   uint32_t max_targets;
};

typedef enum {
   c2recipe_Kind_Plugin,
   c2recipe_Kind_PluginOptions,
   c2recipe_Kind_Text,
   c2recipe_Kind_Executable,
   c2recipe_Kind_Lib,
   c2recipe_Kind_File,
   c2recipe_Kind_End,
   c2recipe_Kind_Warnings,
   c2recipe_Kind_GenerateC,
   c2recipe_Kind_GenerateIR,
   c2recipe_Kind_EnableAssert,
   c2recipe_Kind_NoLibc,
   c2recipe_Kind_Config,
   c2recipe_Kind_Export,
   c2recipe_Kind_Use,
   c2recipe_Kind_Eof,
} __attribute__((packed)) c2recipe_Kind;

struct c2recipe_Token_ {
   c2recipe_Kind kind;
   src_loc_SrcLoc loc;
   bool more;
   uint32_t value;
};

struct c2recipe_Parser_ {
   c2recipe_Recipe* recipe;
   string_pool_Pool* pool;
   source_mgr_SourceMgr* sm;
   const char* input_start;
   const char* cur;
   src_loc_SrcLoc loc_start;
   __jmp_buf_tag jmpbuf;
   c2recipe_Token token;
   bool new_line;
   c2recipe_Target* target;
};

static c2recipe_Target* c2recipe_Target_create(c2recipe_Recipe* recipe, uint32_t name_idx, src_loc_SrcLoc loc, string_pool_Pool* pool);
static void c2recipe_Target_free(c2recipe_Target* t);
static uint32_t c2recipe_Target_getNameIdx(const c2recipe_Target* t);
static uint32_t c2recipe_Target_numFiles(const c2recipe_Target* t);
static const string_list_List* c2recipe_Target_getFeatures(const c2recipe_Target* t);
static const string_list_List* c2recipe_Target_getLibs(const c2recipe_Target* t);
static const warning_flags_Flags* c2recipe_Target_getWarnings(const c2recipe_Target* t);
static void c2recipe_Target_addFile(c2recipe_Target* t, uint32_t filename, src_loc_SrcLoc loc);
static int32_t c2recipe_Target_openFile(c2recipe_Target* t, uint32_t idx);
static void c2recipe_Target_closeFile(c2recipe_Target* t, int32_t file_id);
static c2recipe_Recipe* c2recipe_create(source_mgr_SourceMgr* sm, string_pool_Pool* pool);
static void c2recipe_Recipe_free(c2recipe_Recipe* r);
static c2recipe_Target* c2recipe_Recipe_addTarget(c2recipe_Recipe* r, uint32_t name, src_loc_SrcLoc loc);
static void c2recipe_Recipe_addDummyTarget(c2recipe_Recipe* r, const char* filename);
static bool c2recipe_Recipe_parse(c2recipe_Recipe* r, int32_t file_id);
static uint32_t c2recipe_Recipe_numTargets(const c2recipe_Recipe* r);
static c2recipe_Target* c2recipe_Recipe_getTarget(const c2recipe_Recipe* r, uint32_t idx);
static int32_t c2recipe_Recipe_openFile(c2recipe_Recipe* r, c2recipe_File* f);
static void c2recipe_Recipe_closeFile(c2recipe_Recipe* r, int32_t file_id);
static void c2recipe_Token_init(c2recipe_Token* t);
static bool c2recipe_Parser_parse(c2recipe_Recipe* recipe, string_pool_Pool* pool, source_mgr_SourceMgr* sm, int32_t file_id);
static void c2recipe_Parser_error(c2recipe_Parser* p, const char* format, ...);
static void c2recipe_Parser_consumeToken(c2recipe_Parser* p);
static void c2recipe_Parser_expect(c2recipe_Parser* p, c2recipe_Kind kind, const char* msg);
static bool c2recipe_Parser_is(const c2recipe_Parser* p, c2recipe_Kind kind);
static void c2recipe_Parser_lex(c2recipe_Parser* p, c2recipe_Token* result);
static void c2recipe_Parser_lex_plugin_options(c2recipe_Parser* p, c2recipe_Token* result);
static void c2recipe_Parser_lex_option(c2recipe_Parser* p, c2recipe_Token* result);
static void c2recipe_Parser_skip_comments(c2recipe_Parser* p);
static void c2recipe_Parser_parseTop(c2recipe_Parser* p);
static void c2recipe_Parser_parsePlugin(c2recipe_Parser* p);
static void c2recipe_Parser_parseWarnings(c2recipe_Parser* p);
static void c2recipe_Parser_parseExecutable(c2recipe_Parser* p);
static void c2recipe_Parser_parseLibrary(c2recipe_Parser* p);
static void c2recipe_Parser_parseTarget(c2recipe_Parser* p);
static const char* c2recipe_get_prefix(const char* input, char* output, uint32_t maxlen);
static bool c2recipe_Recipe_getYamlInfo(c2recipe_Recipe* _arg0, const yaml_Parser* parser);
static bool c2recipe_Recipe_parseYaml(c2recipe_Recipe* r, int32_t file_id);

static const char* c2recipe_kind_names[] = {
   "plugin",
   "[plugin_options]",
   "text",
   "executable",
   "lib",
   "file",
   "end",
   "$warnings",
   "$generate-c",
   "$generate-ir",
   "$enable-assert",
   "$nolibc",
   "$config",
   "$export",
   "$use",
   "eof"
};

static c2recipe_Target* c2recipe_Target_create(c2recipe_Recipe* recipe, uint32_t name_idx, src_loc_SrcLoc loc, string_pool_Pool* pool)
{
   c2recipe_Target* t = calloc(1, sizeof(c2recipe_Target));
   t->recipe = recipe;
   t->name_idx = name_idx;
   t->loc = loc;
   t->max_files = 8;
   string_list_List_init(&t->features, pool);
   string_list_List_init(&t->libs, pool);
   t->files = malloc(t->max_files * sizeof(c2recipe_File));
   return t;
}

static void c2recipe_Target_free(c2recipe_Target* t)
{
   string_list_List_free(&t->features);
   string_list_List_free(&t->libs);
   free(t->files);
   free(t);
}

static uint32_t c2recipe_Target_getNameIdx(const c2recipe_Target* t)
{
   return t->name_idx;
}

static uint32_t c2recipe_Target_numFiles(const c2recipe_Target* t)
{
   return t->num_files;
}

static const string_list_List* c2recipe_Target_getFeatures(const c2recipe_Target* t)
{
   return &t->features;
}

static const string_list_List* c2recipe_Target_getLibs(const c2recipe_Target* t)
{
   return &t->libs;
}

static const warning_flags_Flags* c2recipe_Target_getWarnings(const c2recipe_Target* t)
{
   return &t->warnings;
}

static void c2recipe_Target_addFile(c2recipe_Target* t, uint32_t filename, src_loc_SrcLoc loc)
{
   if (t->num_files == t->max_files) {
      t->max_files *= 2;
      c2recipe_File* files2 = malloc(t->max_files * sizeof(c2recipe_File));
      memcpy(files2, t->files, t->num_files * sizeof(c2recipe_File));
      free(t->files);
      t->files = files2;
   }
   t->files[t->num_files].name = filename;
   t->files[t->num_files].loc = loc;
   t->num_files++;
}

static int32_t c2recipe_Target_openFile(c2recipe_Target* t, uint32_t idx)
{
   return c2recipe_Recipe_openFile(t->recipe, &t->files[idx]);
}

static void c2recipe_Target_closeFile(c2recipe_Target* t, int32_t file_id)
{
   c2recipe_Recipe_closeFile(t->recipe, file_id);
}

static c2recipe_Recipe* c2recipe_create(source_mgr_SourceMgr* sm, string_pool_Pool* pool)
{
   c2recipe_Recipe* r = calloc(1, sizeof(c2recipe_Recipe));
   r->sm = sm;
   r->pool = pool;
   r->max_targets = 4;
   r->targets = calloc(r->max_targets, sizeof(c2recipe_Target*));
   return r;
}

static void c2recipe_Recipe_free(c2recipe_Recipe* r)
{
   for (uint32_t i = 0; i < r->num_targets; i++) {
      c2recipe_Target_free(r->targets[i]);
   }
   free(((void*)(r->targets)));
   free(r);
}

static c2recipe_Target* c2recipe_Recipe_addTarget(c2recipe_Recipe* r, uint32_t name, src_loc_SrcLoc loc)
{
   if (r->num_targets == r->max_targets) {
      r->max_targets *= 2;
      c2recipe_Target** targets2 = malloc(r->max_targets * sizeof(c2recipe_Target*));
      memcpy(((void*)(targets2)), ((void*)(r->targets)), r->num_targets * sizeof(c2recipe_Target*));
      free(((void*)(r->targets)));
      r->targets = targets2;
   }
   c2recipe_Target* t = c2recipe_Target_create(r, name, loc, r->pool);
   r->targets[r->num_targets] = t;
   r->num_targets++;
   return t;
}

static void c2recipe_Recipe_addDummyTarget(c2recipe_Recipe* r, const char* filename)
{
   uint32_t target_name = string_pool_Pool_addStr(r->pool, "dummy", true);
   uint32_t file_idx = string_pool_Pool_addStr(r->pool, filename, false);
   c2recipe_Target* t = c2recipe_Recipe_addTarget(r, target_name, 0);
   c2recipe_Target_addFile(t, file_idx, 0);
   t->warnings.no_unused = true;
   t->warnings.no_unused_variable = true;
   t->warnings.no_unused_function = true;
   t->warnings.no_unused_parameter = true;
   t->warnings.no_unused_type = true;
   t->warnings.no_unused_module = true;
   t->warnings.no_unused_import = true;
   t->warnings.no_unused_public = true;
   t->warnings.no_unused_label = true;
   t->warnings.no_unused_enum_constant = true;
}

static bool c2recipe_Recipe_parse(c2recipe_Recipe* r, int32_t file_id)
{
   return c2recipe_Parser_parse(r, r->pool, r->sm, file_id);
}

static uint32_t c2recipe_Recipe_numTargets(const c2recipe_Recipe* r)
{
   return r->num_targets;
}

static c2recipe_Target* c2recipe_Recipe_getTarget(const c2recipe_Recipe* r, uint32_t idx)
{
   return r->targets[idx];
}

static int32_t c2recipe_Recipe_openFile(c2recipe_Recipe* r, c2recipe_File* f)
{
   return source_mgr_SourceMgr_open(r->sm, f->name, f->loc, true);
}

static void c2recipe_Recipe_closeFile(c2recipe_Recipe* r, int32_t file_id)
{
   source_mgr_SourceMgr_close(r->sm, file_id);
}

static void c2recipe_Token_init(c2recipe_Token* t)
{
   memset(t, 0, sizeof(c2recipe_Token));
   t->more = true;
}

static bool c2recipe_Parser_parse(c2recipe_Recipe* recipe, string_pool_Pool* pool, source_mgr_SourceMgr* sm, int32_t file_id)
{
   const char* data = source_mgr_SourceMgr_get_content(sm, file_id);
   c2recipe_Parser p = { };
   p.recipe = recipe;
   p.pool = pool;
   p.sm = sm;
   p.input_start = data;
   p.cur = data;
   p.loc_start = source_mgr_SourceMgr_get_offset(sm, file_id);
   p.new_line = true;
   c2recipe_Token_init(&p.token);
   int32_t res = setjmp(&p.jmpbuf);
   if (res == 0) {
      c2recipe_Parser_consumeToken(&p);
      c2recipe_Parser_parseTop(&p);
   } else {
      return false;
   }
   return true;
}

static void c2recipe_Parser_error(c2recipe_Parser* p, const char* format, ...)
{
   char msg[128];
   va_list args;
   va_start(args, format);
   vsnprintf(msg, sizeof(msg) - 1, format, args);
   va_end(args);
   if (utils_useColor()) {
      fprintf(stderr, "%s: %serror:%s %s\n", source_mgr_SourceMgr_loc2str(p->sm, p->token.loc), color_Red, color_Normal, msg);
   } else {
      fprintf(stderr, "%s: error: %s\n", source_mgr_SourceMgr_loc2str(p->sm, p->token.loc), msg);
   }
   longjmp(&p->jmpbuf, 1);
}

static void c2recipe_Parser_consumeToken(c2recipe_Parser* p)
{
   c2recipe_Parser_lex(p, &p->token);
}

static void c2recipe_Parser_expect(c2recipe_Parser* p, c2recipe_Kind kind, const char* msg)
{
   if (p->token.kind != kind) c2recipe_Parser_error(p, msg);
}

static bool c2recipe_Parser_is(const c2recipe_Parser* p, c2recipe_Kind kind)
{
   return p->token.kind == kind;
}

static void c2recipe_Parser_lex(c2recipe_Parser* p, c2recipe_Token* result)
{
   while (1) {
      switch (*p->cur) {
      case 0:
         p->cur--;
         result->loc = p->loc_start + ((src_loc_SrcLoc)(p->cur - p->input_start));
         result->kind = c2recipe_Kind_Eof;
         result->more = false;
         return;
      case ' ':
         // fallthrough
      case '\t':
         // fallthrough
      case '\r':
         p->cur++;
         break;
      case '\n':
         p->cur++;
         p->new_line = true;
         break;
      case '#':
         c2recipe_Parser_skip_comments(p);
         break;
      case '[':
         c2recipe_Parser_lex_plugin_options(p, result);
         p->new_line = false;
         return;
      case '$':
         c2recipe_Parser_lex_option(p, result);
         p->new_line = false;
         return;
      case '/': {
         const char* start = p->cur;
         while ((*p->cur && !isspace(*p->cur))) p->cur++;
         result->kind = p->new_line ? c2recipe_Kind_File : c2recipe_Kind_Text;
         p->new_line = false;
         uint32_t len = ((uint32_t)(p->cur - start));
         result->value = string_pool_Pool_add(p->pool, start, len, false);
         return;
      }
      default:
         if (isalpha(*p->cur)) {
            result->loc = p->loc_start + ((src_loc_SrcLoc)(p->cur - p->input_start));
            if (memcmp(p->cur, "plugin ", 7) == 0) {
               result->kind = c2recipe_Kind_Plugin;
               p->cur += 7;
               p->new_line = false;
               return;
            }
            if (memcmp(p->cur, "executable ", 11) == 0) {
               result->kind = c2recipe_Kind_Executable;
               p->cur += 11;
               p->new_line = false;
               return;
            }
            if (memcmp(p->cur, "lib ", 4) == 0) {
               result->kind = c2recipe_Kind_Lib;
               p->cur += 4;
               p->new_line = false;
               return;
            }
            if (memcmp(p->cur, "end", 3) == 0) {
               result->kind = c2recipe_Kind_End;
               p->cur += 3;
               p->new_line = false;
               return;
            }
            const char* start = p->cur;
            while ((*p->cur && !isspace(*p->cur))) p->cur++;
            result->kind = p->new_line ? c2recipe_Kind_File : c2recipe_Kind_Text;
            p->new_line = false;
            uint32_t len = ((uint32_t)(p->cur - start));
            result->value = string_pool_Pool_add(p->pool, start, len, false);
            return;
         }
         result->loc = p->loc_start + ((src_loc_SrcLoc)(p->cur - p->input_start));
         c2recipe_Parser_error(p, "unexpected input '%c'", *p->cur);
         return;
      }
   }
}

static void c2recipe_Parser_lex_plugin_options(c2recipe_Parser* p, c2recipe_Token* result)
{
   p->cur++;
   const char* start = p->cur;
   while (1) {
      if (*p->cur == 0) {
         return;
      }
      if (*p->cur == ']') {
         uint32_t len = ((uint32_t)(p->cur - start));
         result->loc = p->loc_start + ((src_loc_SrcLoc)(start - p->input_start));
         result->kind = c2recipe_Kind_PluginOptions;
         result->value = string_pool_Pool_add(p->pool, start, len, false);
         p->cur++;
         return;
      }
      p->cur++;
   }
}

static void c2recipe_Parser_lex_option(c2recipe_Parser* p, c2recipe_Token* result)
{
   p->cur++;
   result->loc = p->loc_start + ((src_loc_SrcLoc)(p->cur - p->input_start));
   const char* end = p->cur;
   while ((*end && !isspace(*end))) end++;
   uint32_t len = ((uint32_t)(end - p->cur));
   if (len >= 20) c2recipe_Parser_error(p, "unknown option");
   char option[24];
   memcpy(option, p->cur, len);
   option[len] = 0;
   if (strcmp(option, "warnings") == 0) {
      result->kind = c2recipe_Kind_Warnings;
      p->cur += 8;
      return;
   }
   if (strcmp(option, "generate-c") == 0) {
      result->kind = c2recipe_Kind_GenerateC;
      p->cur += 10;
      return;
   }
   if (strcmp(option, "enable-assert") == 0) {
      result->kind = c2recipe_Kind_EnableAssert;
      p->cur += 13;
      return;
   }
   if (strcmp(option, "nolibc") == 0) {
      result->kind = c2recipe_Kind_NoLibc;
      p->cur += 6;
      return;
   }
   if (strcmp(option, "config") == 0) {
      result->kind = c2recipe_Kind_Config;
      p->cur += 6;
      return;
   }
   if (strcmp(option, "export") == 0) {
      result->kind = c2recipe_Kind_Export;
      p->cur += 6;
      return;
   }
   if (strcmp(option, "plugin") == 0) {
      result->kind = c2recipe_Kind_Plugin;
      p->cur += 6;
      return;
   }
   if (strcmp(option, "generate-ir") == 0) {
      result->kind = c2recipe_Kind_GenerateIR;
      p->cur += 6;
      return;
   }
   if (strcmp(option, "use") == 0) {
      result->kind = c2recipe_Kind_Use;
      p->cur += 3;
      return;
   }
   c2recipe_Parser_error(p, "unknown option '%s'", option);
}

static void c2recipe_Parser_skip_comments(c2recipe_Parser* p)
{
   while (*p->cur) {
      if (*p->cur == '\n') return;

      p->cur++;
   }
}

static void c2recipe_Parser_parseTop(c2recipe_Parser* p)
{
   while (1) {
      switch (p->token.kind) {
      case c2recipe_Kind_Plugin:
         c2recipe_Parser_parsePlugin(p);
         break;
      case c2recipe_Kind_PluginOptions:
         break;
      case c2recipe_Kind_Text:
         break;
      case c2recipe_Kind_Executable:
         c2recipe_Parser_parseExecutable(p);
         break;
      case c2recipe_Kind_Lib:
         c2recipe_Parser_parseLibrary(p);
         break;
      case c2recipe_Kind_File:
         break;
      case c2recipe_Kind_End:
         break;
      case c2recipe_Kind_Warnings:
         // fallthrough
      case c2recipe_Kind_GenerateC:
         // fallthrough
      case c2recipe_Kind_GenerateIR:
         // fallthrough
      case c2recipe_Kind_EnableAssert:
         // fallthrough
      case c2recipe_Kind_NoLibc:
         // fallthrough
      case c2recipe_Kind_Config:
         // fallthrough
      case c2recipe_Kind_Export:
         // fallthrough
      case c2recipe_Kind_Use:
         c2recipe_Parser_error(p, "must be inside target");
         break;
      case c2recipe_Kind_Eof:
         return;
      }
   }
}

static void c2recipe_Parser_parsePlugin(c2recipe_Parser* p)
{
   c2recipe_Parser_consumeToken(p);
   c2recipe_Parser_expect(p, c2recipe_Kind_Text, "expect plugin name");
   c2recipe_Parser_consumeToken(p);
   if (p->token.kind == c2recipe_Kind_PluginOptions) {
      c2recipe_Parser_consumeToken(p);
   }
}

static void c2recipe_Parser_parseWarnings(c2recipe_Parser* p)
{
   c2recipe_Parser_consumeToken(p);
   c2recipe_Parser_expect(p, c2recipe_Kind_Text, "expect options");
   warning_flags_Flags* warnings = &p->target->warnings;
   while (c2recipe_Parser_is(p, c2recipe_Kind_Text)) {
      const char* option = string_pool_Pool_idx2str(p->pool, p->token.value);
      if (strcmp(option, "no-unused") == 0) {
         warnings->no_unused = true;
         warnings->no_unused_variable = true;
         warnings->no_unused_function = true;
         warnings->no_unused_parameter = true;
         warnings->no_unused_type = true;
         warnings->no_unused_module = true;
         warnings->no_unused_import = true;
         warnings->no_unused_public = true;
         warnings->no_unused_label = true;
         warnings->no_unused_enum_constant = true;
      } else if (strcmp(option, "no-unused-variable") == 0) {
         warnings->no_unused_variable = true;
      } else if (strcmp(option, "no-unused-function") == 0) {
         warnings->no_unused_function = true;
      } else if (strcmp(option, "no-unused-parameter") == 0) {
         warnings->no_unused_parameter = true;
      } else if (strcmp(option, "no-unused-type") == 0) {
         warnings->no_unused_type = true;
      } else if (strcmp(option, "no-unused-module") == 0) {
         warnings->no_unused_module = true;
      } else if (strcmp(option, "no-unused-import") == 0) {
         warnings->no_unused_import = true;
      } else if (strcmp(option, "no-unused-public") == 0) {
         warnings->no_unused_public = true;
      } else if (strcmp(option, "no-unused-label") == 0) {
         warnings->no_unused_label = true;
      } else if (strcmp(option, "no-unused-enum-constant") == 0) {
         warnings->no_unused_enum_constant = true;
      } else if (strcmp(option, "promote-to-error") == 0) {
         warnings->are_errors = true;
      } else {
         c2recipe_Parser_error(p, "unknown warning '%s'", option);
      }










      c2recipe_Parser_consumeToken(p);
   }
}

static void c2recipe_Parser_parseExecutable(c2recipe_Parser* p)
{
   c2recipe_Parser_consumeToken(p);
   c2recipe_Parser_expect(p, c2recipe_Kind_Text, "expect target name");
   p->target = c2recipe_Recipe_addTarget(p->recipe, p->token.value, p->token.loc);
   c2recipe_Parser_consumeToken(p);
   c2recipe_Parser_parseTarget(p);
}

static void c2recipe_Parser_parseLibrary(c2recipe_Parser* p)
{
   c2recipe_Parser_consumeToken(p);
   c2recipe_Parser_expect(p, c2recipe_Kind_Text, "expect target name");
   uint32_t name = p->token.value;
   src_loc_SrcLoc loc = p->token.loc;
   c2recipe_Parser_consumeToken(p);
   c2recipe_Parser_expect(p, c2recipe_Kind_Text, "expect lib type");
   c2recipe_Parser_consumeToken(p);
   p->target = c2recipe_Recipe_addTarget(p->recipe, name, loc);
   c2recipe_Parser_parseTarget(p);
}

static void c2recipe_Parser_parseTarget(c2recipe_Parser* p)
{
   bool files_started = false;
   while (1) {
      switch (p->token.kind) {
      case c2recipe_Kind_Plugin:
         c2recipe_Parser_parsePlugin(p);
         break;
      case c2recipe_Kind_PluginOptions:
         // fallthrough
      case c2recipe_Kind_Text:
         // fallthrough
      case c2recipe_Kind_Executable:
         // fallthrough
      case c2recipe_Kind_Lib:
         c2recipe_Parser_error(p, "syntax error");
         break;
      case c2recipe_Kind_File:
         files_started = true;
         c2recipe_Target_addFile(p->target, p->token.value, p->token.loc);
         c2recipe_Parser_consumeToken(p);
         break;
      case c2recipe_Kind_End:
         c2recipe_Parser_consumeToken(p);
         p->target = NULL;
         return;
      case c2recipe_Kind_Warnings:
         if (files_started) c2recipe_Parser_error(p, "$options must come before files");
         c2recipe_Parser_parseWarnings(p);
         break;
      case c2recipe_Kind_GenerateC:
         if (files_started) c2recipe_Parser_error(p, "$options must come before files");
         c2recipe_Parser_consumeToken(p);
         while (p->token.kind == c2recipe_Kind_Text) {
            c2recipe_Parser_consumeToken(p);
         }
         break;
      case c2recipe_Kind_GenerateIR:
         if (files_started) c2recipe_Parser_error(p, "$options must come before files");
         c2recipe_Parser_consumeToken(p);
         while (p->token.kind == c2recipe_Kind_Text) c2recipe_Parser_consumeToken(p);
         break;
      case c2recipe_Kind_EnableAssert:
         if (files_started) c2recipe_Parser_error(p, "$options must come before files");
         c2recipe_Parser_consumeToken(p);
         break;
      case c2recipe_Kind_NoLibc:
         if (files_started) c2recipe_Parser_error(p, "$options must come before files");
         c2recipe_Parser_consumeToken(p);
         while (p->token.kind == c2recipe_Kind_Text) c2recipe_Parser_consumeToken(p);
         break;
      case c2recipe_Kind_Config:
         if (files_started) c2recipe_Parser_error(p, "$options must come before files");
         c2recipe_Parser_consumeToken(p);
         c2recipe_Parser_expect(p, c2recipe_Kind_Text, "expect config");
         string_list_List_add(&p->target->features, p->token.value);
         c2recipe_Parser_consumeToken(p);
         break;
      case c2recipe_Kind_Export:
         if (files_started) c2recipe_Parser_error(p, "$options must come before files");
         c2recipe_Parser_consumeToken(p);
         while (p->token.kind == c2recipe_Kind_Text) c2recipe_Parser_consumeToken(p);
         break;
      case c2recipe_Kind_Use:
         if (files_started) c2recipe_Parser_error(p, "$options must come before files");
         c2recipe_Parser_consumeToken(p);
         c2recipe_Parser_expect(p, c2recipe_Kind_Text, "expect library name");
         string_list_List_add(&p->target->libs, p->token.value);
         c2recipe_Parser_consumeToken(p);
         c2recipe_Parser_expect(p, c2recipe_Kind_Text, "expect library type");
         c2recipe_Parser_consumeToken(p);
         while (p->token.kind == c2recipe_Kind_Text) c2recipe_Parser_consumeToken(p);
         break;
      case c2recipe_Kind_Eof:
         c2recipe_Parser_error(p, "un-terminated target");
         return;
      }
   }
}

static const char* c2recipe_get_prefix(const char* input, char* output, uint32_t maxlen)
{
   maxlen--;
   while ((*input && maxlen)) {
      if (*input == '.') break;

      *output++ = *input++;
      maxlen--;
   }
   *output = 0;
   if (maxlen == 0) return NULL;

   return input + 1;
}

static bool c2recipe_Recipe_getYamlInfo(c2recipe_Recipe* _arg0, const yaml_Parser* parser)
{
   const yaml_Node* root = yaml_Parser_getRoot(parser);
   if ((!root || !yaml_Node_isMap(root))) {
      fprintf(stderr, "empty recipe?\n");
      return false;
   }
   yaml_Iter iter = yaml_Parser_getNodeChildIter(parser, root);
   while (!yaml_Iter_done(&iter)) {
      const char* name = yaml_Iter_getName(&iter);
      char prefix[32];
      const char* after = c2recipe_get_prefix(name, prefix, sizeof(prefix));
      if (!after) {
         printf("invalid item %s\n", name);
         return false;
      }
      char* p = prefix;
      do {
         const char* _tmp = p;
         if (strcmp(_tmp, "plugin") == 0) {
         } else if (strcmp(_tmp, "executable") == 0) {
         } else {
            printf("unknown item %s\n", prefix);
            return false;
         }
      } while (0);
      yaml_Iter_next(&iter);
   }
   return true;
}

static bool c2recipe_Recipe_parseYaml(c2recipe_Recipe* r, int32_t file_id)
{
   const char* data = source_mgr_SourceMgr_get_content(r->sm, file_id);
   uint32_t loc_start = source_mgr_SourceMgr_get_offset(r->sm, file_id);
   yaml_Parser* parser = yaml_Parser_create();
   bool ok = yaml_Parser_parse(parser, ((char*)(data)));
   if (ok) {
      if (!c2recipe_Recipe_getYamlInfo(r, parser)) return false;

   } else {
      fprintf(stderr, "Error: %s\n", yaml_Parser_getMessage(parser));
   }
   yaml_Parser_destroy(parser);
   return ok;
}


// --- module diagnostics ---

typedef struct diagnostics_Diags_ diagnostics_Diags;

struct diagnostics_Diags_ {
   source_mgr_SourceMgr* sm;
   string_buffer_Buf* out;
   uint32_t num_errors;
   uint32_t num_warnings;
   bool promote_warnings;
};

typedef enum {
   diagnostics_Category_Info,
   diagnostics_Category_Warning,
   diagnostics_Category_Error,
} __attribute__((packed)) diagnostics_Category;

static diagnostics_Diags* diagnostics_create(source_mgr_SourceMgr* sm, bool use_color);
static void diagnostics_Diags_free(diagnostics_Diags* diags);
static void diagnostics_Diags_clear(diagnostics_Diags* diags);
static void diagnostics_Diags_setWarningAsError(diagnostics_Diags* diags, bool are_errors);
static void diagnostics_Diags_error(diagnostics_Diags* diags, src_loc_SrcLoc loc, const char* format, ...);
static void diagnostics_Diags_error2(diagnostics_Diags* diags, src_loc_SrcLoc loc, const char* format, va_list args);
static void diagnostics_Diags_note(diagnostics_Diags* diags, src_loc_SrcLoc loc, const char* format, ...);
static void diagnostics_Diags_note2(diagnostics_Diags* diags, src_loc_SrcLoc loc, const char* format, va_list args);
static void diagnostics_Diags_warn(diagnostics_Diags* diags, src_loc_SrcLoc loc, const char* format, ...);
static void diagnostics_Diags_warn2(diagnostics_Diags* diags, src_loc_SrcLoc loc, const char* format, va_list args);
static void diagnostics_Diags_reportRange(diagnostics_Diags* diags, src_loc_SrcLoc loc, src_loc_SrcRange range, const char* format, ...);
static void diagnostics_Diags_errorRange2(diagnostics_Diags* diags, src_loc_SrcLoc loc, src_loc_SrcRange range, const char* format, va_list args);
static void diagnostics_Diags_internal(diagnostics_Diags* diags, diagnostics_Category category, src_loc_SrcLoc sloc, src_loc_SrcRange range, const char* format, va_list args);
static bool diagnostics_Diags_isOk(const diagnostics_Diags* diags);
static bool diagnostics_Diags_hasErrors(const diagnostics_Diags* diags);
static uint32_t diagnostics_Diags_getNumErrors(const diagnostics_Diags* diags);
static uint32_t diagnostics_Diags_getNumWarnings(const diagnostics_Diags* diags);
static void diagnostics_Diags_printStatus(const diagnostics_Diags* diags);

static const char* diagnostics_category_names[] = { "info", "warning", "error" };

static const char* diagnostics_category_colors[] = { color_Grey, color_Magenta, color_Bred };

static diagnostics_Diags* diagnostics_create(source_mgr_SourceMgr* sm, bool use_color)
{
   diagnostics_Diags* diags = calloc(1, sizeof(diagnostics_Diags));
   diags->sm = sm;
   diags->out = string_buffer_create(512, use_color, 1);
   return diags;
}

static void diagnostics_Diags_free(diagnostics_Diags* diags)
{
   string_buffer_Buf_free(diags->out);
   free(diags);
}

static void diagnostics_Diags_clear(diagnostics_Diags* diags)
{
   diags->num_errors = 0;
   diags->num_warnings = 0;
}

static void diagnostics_Diags_setWarningAsError(diagnostics_Diags* diags, bool are_errors)
{
   diags->promote_warnings = are_errors;
}

static void diagnostics_Diags_error(diagnostics_Diags* diags, src_loc_SrcLoc loc, const char* format, ...)
{
   diagnostics_Category category = diagnostics_Category_Error;
   va_list args;
   va_start(args, format);
   src_loc_SrcRange range = { 0, 0 };
   diagnostics_Diags_internal(diags, category, loc, range, format, args);
   va_end(args);
}

static void diagnostics_Diags_error2(diagnostics_Diags* diags, src_loc_SrcLoc loc, const char* format, va_list args)
{
   src_loc_SrcRange range = { 0, 0 };
   diagnostics_Diags_internal(diags, diagnostics_Category_Error, loc, range, format, args);
}

static void diagnostics_Diags_note(diagnostics_Diags* diags, src_loc_SrcLoc loc, const char* format, ...)
{
   va_list args;
   va_start(args, format);
   src_loc_SrcRange range = { 0, 0 };
   diagnostics_Diags_internal(diags, diagnostics_Category_Info, loc, range, format, args);
   va_end(args);
}

static void diagnostics_Diags_note2(diagnostics_Diags* diags, src_loc_SrcLoc loc, const char* format, va_list args)
{
   src_loc_SrcRange range = { 0, 0 };
   diagnostics_Diags_internal(diags, diagnostics_Category_Info, loc, range, format, args);
}

static void diagnostics_Diags_warn(diagnostics_Diags* diags, src_loc_SrcLoc loc, const char* format, ...)
{
   va_list args;
   va_start(args, format);
   src_loc_SrcRange range = { 0, 0 };
   diagnostics_Category category = diagnostics_Category_Warning;
   if (diags->promote_warnings) category = diagnostics_Category_Error;
   diagnostics_Diags_internal(diags, category, loc, range, format, args);
   va_end(args);
}

static void diagnostics_Diags_warn2(diagnostics_Diags* diags, src_loc_SrcLoc loc, const char* format, va_list args)
{
   src_loc_SrcRange range = { 0, 0 };
   diagnostics_Category category = diagnostics_Category_Warning;
   if (diags->promote_warnings) category = diagnostics_Category_Error;
   diagnostics_Diags_internal(diags, category, loc, range, format, args);
}

static void diagnostics_Diags_reportRange(diagnostics_Diags* diags, src_loc_SrcLoc loc, src_loc_SrcRange range, const char* format, ...)
{
   diagnostics_Category category = diagnostics_Category_Error;
   va_list args;
   va_start(args, format);
   diagnostics_Diags_internal(diags, category, loc, range, format, args);
   va_end(args);
}

static void diagnostics_Diags_errorRange2(diagnostics_Diags* diags, src_loc_SrcLoc loc, src_loc_SrcRange range, const char* format, va_list args)
{
   diagnostics_Diags_internal(diags, diagnostics_Category_Error, loc, range, format, args);
}

static void diagnostics_Diags_internal(diagnostics_Diags* diags, diagnostics_Category category, src_loc_SrcLoc sloc, src_loc_SrcRange range, const char* format, va_list args)
{
   if (category == diagnostics_Category_Error) {
      diags->num_errors++;
   } else {
      diags->num_warnings++;
   }
   string_buffer_Buf* out = diags->out;
   string_buffer_Buf_clear(out);
   source_mgr_Location startLoc = source_mgr_SourceMgr_getLocation(diags->sm, range.start);
   source_mgr_Location loc = source_mgr_SourceMgr_getLocation(diags->sm, sloc);
   if (range.end) {
      const char* src = source_mgr_SourceMgr_get_token_source(diags->sm, range.end);
      range.end = c2_tokenizer_getTokenEnd(src, range.end);
   }
   source_mgr_Location endLoc = source_mgr_SourceMgr_getLocation(diags->sm, range.end);
   if (sloc) {
      string_buffer_Buf_print(out, "%s:%u:%u: ", loc.filename, loc.line, loc.column);
   }
   string_buffer_Buf_color(out, diagnostics_category_colors[category]);
   string_buffer_Buf_add(out, diagnostics_category_names[category]);
   string_buffer_Buf_add(out, ": ");
   string_buffer_Buf_color(out, color_Normal);
   char tmp[256];
   vsprintf(tmp, format, args);
   string_buffer_Buf_add(out, tmp);
   string_buffer_Buf_add(out, "\n");
   if (sloc) {
      string_buffer_Buf_add_line(out, loc.line_start);
      string_buffer_Buf_add(out, "\n");
      if ((range.start && range.end)) {
         assert(endLoc.column >= startLoc.column);
         string_buffer_Buf_indent(out, startLoc.column - 1);
         string_buffer_Buf_color(out, color_Bgreen);
         for (uint32_t i = startLoc.column; i <= endLoc.column; i++) {
            if (i == loc.column) string_buffer_Buf_add(out, "^");
            else string_buffer_Buf_add(out, "~");
         }
         string_buffer_Buf_color(out, color_Normal);
      } else {
         string_buffer_Buf_indent(out, loc.column - 1);
         string_buffer_Buf_color(out, color_Bgreen);
         string_buffer_Buf_add(out, "^");
         string_buffer_Buf_color(out, color_Normal);
      }
   }
   FILE* stream = (category == diagnostics_Category_Info) ? stdout : stderr;
   fprintf(stream, "%s\n", string_buffer_Buf_data(out));
}

static bool diagnostics_Diags_isOk(const diagnostics_Diags* diags)
{
   return diags->num_errors == 0;
}

static bool diagnostics_Diags_hasErrors(const diagnostics_Diags* diags)
{
   return diags->num_errors != 0;
}

static uint32_t diagnostics_Diags_getNumErrors(const diagnostics_Diags* diags)
{
   return diags->num_errors;
}

static uint32_t diagnostics_Diags_getNumWarnings(const diagnostics_Diags* diags)
{
   return diags->num_warnings;
}

static void diagnostics_Diags_printStatus(const diagnostics_Diags* diags)
{
   string_buffer_Buf* out = diags->out;
   string_buffer_Buf_clear(out);
   if (diags->num_warnings) {
      string_buffer_Buf_print(out, "%u warning%s", diags->num_warnings, diags->num_warnings > 1 ? "s" : "");
   }
   if (diags->num_errors) {
      if (diags->num_warnings) string_buffer_Buf_add(out, " and ");
      string_buffer_Buf_print(out, "%u error%s", diags->num_errors, diags->num_errors > 1 ? "s" : "");
   }
   if (string_buffer_Buf_size(out)) {
      string_buffer_Buf_add(out, " generated.\n");
      fputs(string_buffer_Buf_data(out), stderr);
   }
}


// --- module ast ---

typedef struct ast_DeclBits_ ast_DeclBits;
typedef struct ast_Decl_ ast_Decl;

typedef struct ast_FunctionDeclBits_ ast_FunctionDeclBits;
typedef struct ast_FunctionDecl_ ast_FunctionDecl;

typedef struct ast_ImportDeclBits_ ast_ImportDeclBits;
typedef struct ast_ImportDecl_ ast_ImportDecl;

typedef struct ast_StructTypeDeclBits_ ast_StructTypeDeclBits;
typedef struct ast_StructLayout_ ast_StructLayout;
typedef struct ast_StructTypeDecl_ ast_StructTypeDecl;

typedef struct ast_VarDeclBits_ ast_VarDeclBits;
typedef struct ast_VarDecl_ ast_VarDecl;

typedef struct ast_EnumTypeDeclBits_ ast_EnumTypeDeclBits;
typedef struct ast_EnumTypeDecl_ ast_EnumTypeDecl;

typedef struct ast_EnumConstantDeclBits_ ast_EnumConstantDeclBits;
typedef struct ast_EnumConstantDecl_ ast_EnumConstantDecl;

typedef struct ast_DeclStmt_ ast_DeclStmt;

typedef struct ast_AliasTypeDecl_ ast_AliasTypeDecl;

typedef struct ast_StaticAssertDecl_ ast_StaticAssertDecl;

typedef struct ast_FunctionTypeDecl_ ast_FunctionTypeDecl;

typedef struct ast_StmtBits_ ast_StmtBits;
typedef struct ast_Stmt_ ast_Stmt;

typedef struct ast_AssertStmt_ ast_AssertStmt;

typedef struct ast_CompoundStmtBits_ ast_CompoundStmtBits;
typedef struct ast_CompoundStmt_ ast_CompoundStmt;

typedef struct ast_DoStmt_ ast_DoStmt;

typedef struct ast_ForStmt_ ast_ForStmt;

typedef struct ast_IfStmtBits_ ast_IfStmtBits;
typedef struct ast_IfStmt_ ast_IfStmt;

typedef struct ast_ReturnStmtBits_ ast_ReturnStmtBits;
typedef struct ast_ReturnStmt_ ast_ReturnStmt;

typedef struct ast_SwitchStmtBits_ ast_SwitchStmtBits;
typedef struct ast_SwitchStmt_ ast_SwitchStmt;

typedef struct ast_CaseStmtBits_ ast_CaseStmtBits;
typedef struct ast_CaseStmt_ ast_CaseStmt;

typedef struct ast_DefaultStmtBits_ ast_DefaultStmtBits;
typedef struct ast_DefaultStmt_ ast_DefaultStmt;

typedef struct ast_BreakStmt_ ast_BreakStmt;

typedef struct ast_ContinueStmt_ ast_ContinueStmt;

typedef struct ast_FallthroughStmt_ ast_FallthroughStmt;

typedef struct ast_WhileStmt_ ast_WhileStmt;

typedef struct ast_LabelStmt_ ast_LabelStmt;

typedef struct ast_GotoStmt_ ast_GotoStmt;

typedef struct ast_ExprBits_ ast_ExprBits;
typedef struct ast_Expr_ ast_Expr;

typedef struct ast_BuiltinExprBits_ ast_BuiltinExprBits;
typedef struct ast_ToContainerData_ ast_ToContainerData;
typedef struct ast_OffsetOfData_ ast_OffsetOfData;
typedef struct ast_BuiltinExpr_ ast_BuiltinExpr;

typedef struct ast_BooleanLiteralBits_ ast_BooleanLiteralBits;
typedef struct ast_BooleanLiteral_ ast_BooleanLiteral;

typedef struct ast_CharLiteralBits_ ast_CharLiteralBits;
typedef struct ast_CharLiteral_ ast_CharLiteral;

typedef struct ast_StringLiteral_ ast_StringLiteral;

typedef struct ast_IntegerLiteralBits_ ast_IntegerLiteralBits;
typedef struct ast_IntegerLiteral_ ast_IntegerLiteral;

typedef struct ast_NilExpr_ ast_NilExpr;

typedef struct ast_IdentifierExprBits_ ast_IdentifierExprBits;
typedef struct ast_IdentifierExpr_ ast_IdentifierExpr;

typedef struct ast_CallExprBits_ ast_CallExprBits;
typedef struct ast_CallExpr_ ast_CallExpr;

typedef struct ast_InitListExprBits_ ast_InitListExprBits;
typedef struct ast_InitListExpr_ ast_InitListExpr;

typedef struct ast_FieldDesignatedInitExpr_ ast_FieldDesignatedInitExpr;

typedef struct ast_ArrayDesignatedInitExpr_ ast_ArrayDesignatedInitExpr;

typedef struct ast_ParenExpr_ ast_ParenExpr;

typedef struct ast_UnaryOperatorBits_ ast_UnaryOperatorBits;
typedef struct ast_UnaryOperator_ ast_UnaryOperator;

typedef struct ast_ConditionalOperator_ ast_ConditionalOperator;

typedef struct ast_BinaryOperatorBits_ ast_BinaryOperatorBits;
typedef struct ast_BinaryOperator_ ast_BinaryOperator;

typedef struct ast_TypeExpr_ ast_TypeExpr;

typedef struct ast_BitOffsetExpr_ ast_BitOffsetExpr;

typedef struct ast_ArraySubscriptExpr_ ast_ArraySubscriptExpr;

typedef struct ast_MemberExprBits_ ast_MemberExprBits;
typedef union ast_MemberRef_ ast_MemberRef;
typedef struct ast_MemberExpr_ ast_MemberExpr;

typedef struct ast_ExplicitCastExpr_ ast_ExplicitCastExpr;

typedef struct ast_ImplicitCastBits_ ast_ImplicitCastBits;
typedef struct ast_ImplicitCastExpr_ ast_ImplicitCastExpr;


typedef struct ast_Stat_ ast_Stat;
typedef struct ast_Stats_ ast_Stats;

typedef struct ast_TypeBits_ ast_TypeBits;
typedef struct ast_Type_ ast_Type;

typedef struct ast_QualType_ ast_QualType;

typedef struct ast_BuiltinTypeBits_ ast_BuiltinTypeBits;
typedef struct ast_BuiltinType_ ast_BuiltinType;

typedef struct ast_PointerType_ ast_PointerType;

typedef struct ast_ArrayTypeBits_ ast_ArrayTypeBits;
typedef struct ast_ArrayType_ ast_ArrayType;

typedef struct ast_StructType_ ast_StructType;

typedef struct ast_EnumType_ ast_EnumType;

typedef struct ast_FunctionType_ ast_FunctionType;

typedef struct ast_AliasType_ ast_AliasType;

typedef struct ast_ModuleType_ ast_ModuleType;

typedef struct ast_TypeRefBits_ ast_TypeRefBits;
typedef struct ast_Ref_ ast_Ref;
typedef struct ast_TypeRef_ ast_TypeRef;
typedef struct ast_TypeRefHolder_ ast_TypeRefHolder;

typedef struct ast_AST_ ast_AST;

typedef struct ast_Module_ ast_Module;

typedef struct ast_DeclList_ ast_DeclList;

typedef struct ast_ExprList_ ast_ExprList;

typedef struct ast_FunctionDeclList_ ast_FunctionDeclList;

typedef struct ast_ImportDeclList_ ast_ImportDeclList;

typedef struct ast_SymbolTable_ ast_SymbolTable;

typedef struct ast_TemplateInstance_ ast_TemplateInstance;
typedef struct ast_TemplateFunction_ ast_TemplateFunction;
typedef struct ast_InstanceTable_ ast_InstanceTable;

typedef struct ast_PointerPoolSlot_ ast_PointerPoolSlot;
typedef struct ast_PointerPool_ ast_PointerPool;

typedef struct ast_StringTypeSlot_ ast_StringTypeSlot;
typedef struct ast_StringTypePool_ ast_StringTypePool;

typedef struct ast_Instantiator_ ast_Instantiator;

typedef enum {
   ast_DeclKind_Function,
   ast_DeclKind_Import,
   ast_DeclKind_StructType,
   ast_DeclKind_EnumType,
   ast_DeclKind_EnumConstant,
   ast_DeclKind_FunctionType,
   ast_DeclKind_AliasType,
   ast_DeclKind_Var,
   ast_DeclKind_StaticAssert,
} __attribute__((packed)) ast_DeclKind;

typedef enum {
   ast_DeclCheckState_Unchecked,
   ast_DeclCheckState_InProgress,
   ast_DeclCheckState_Checked,
} __attribute__((packed)) ast_DeclCheckState;

struct ast_DeclBits_ {
   uint32_t kind : 8;
   uint32_t check_state : 2;
   uint32_t is_public : 1;
   uint32_t is_used : 1;
   uint32_t is_used_public : 1;
   uint32_t has_attr : 1;
   uint32_t attr_export : 1;
   uint32_t attr_unused : 1;
   uint32_t is_external : 1;
   uint32_t is_generated : 1;
};

struct ast_ImportDeclBits_ {
   uint32_t  : 18;
   uint32_t is_local : 1;
};

struct ast_FunctionDeclBits_ {
   uint32_t  : 18;
   uint32_t is_variadic : 1;
   uint32_t has_prefix : 1;
   uint32_t call_kind : 2;
   uint32_t has_return : 1;
   uint32_t attr_unused_params : 1;
   uint32_t attr_noreturn : 1;
   uint32_t attr_inline : 1;
   uint32_t attr_weak : 1;
   uint32_t is_template : 1;
};

struct ast_StructTypeDeclBits_ {
   uint32_t  : 18;
   uint32_t is_struct : 1;
   uint32_t is_global : 1;
   uint32_t attr_packed : 1;
   uint32_t attr_opaque : 1;
   uint32_t attr_notypedef : 1;
};

struct ast_EnumTypeDeclBits_ {
   uint32_t  : 18;
   uint32_t is_incr : 1;
   uint32_t num_constants : 12;
};

struct ast_EnumConstantDeclBits_ {
   uint32_t  : 18;
   uint32_t has_init : 1;
};

struct ast_VarDeclBits_ {
   uint32_t  : 18;
   uint32_t kind : 3;
   uint32_t has_init_or_bitfield : 1;
   uint32_t has_local : 1;
   uint32_t attr_weak : 1;
};

struct ast_QualType_ {
   size_t ptr;
};

struct ast_Decl_ {
   union {
      ast_DeclBits declBits;
      ast_ImportDeclBits importDeclBits;
      ast_FunctionDeclBits functionDeclBits;
      ast_StructTypeDeclBits structTypeDeclBits;
      ast_EnumTypeDeclBits enumTypeDeclBits;
      ast_EnumConstantDeclBits enumConstantDeclBits;
      ast_VarDeclBits varDeclBits;
      uint32_t bits;
   };
   src_loc_SrcLoc loc;
   uint32_t name_idx;
   uint32_t ast_idx;
   ast_QualType qt;
};

typedef enum {
   ast_CallKind_Invalid,
   ast_CallKind_Normal,
   ast_CallKind_StructFunc,
   ast_CallKind_StaticStructFunc,
} __attribute__((packed)) ast_CallKind;

struct ast_TypeRefBits_ {
   uint32_t is_const : 1;
   uint32_t is_volatile : 1;
   uint32_t num_ptrs : 2;
   uint32_t num_arrays : 2;
   uint32_t incr_array : 1;
   uint32_t is_user : 1;
   uint32_t has_prefix : 1;
   uint32_t builtin_kind : 4;
};

struct ast_Ref_ {
   src_loc_SrcLoc loc;
   uint32_t name_idx;
   ast_Decl* decl;
};

struct ast_TypeRef_ {
   union {
      ast_TypeRefBits flags;
      uint32_t flagBits;
   };
   uint32_t dest;
   ast_Ref refs[0];
};

struct ast_FunctionDecl_ {
   ast_Decl parent;
   ast_CompoundStmt* body;
   ast_QualType rt;
   uint8_t num_params;
   uint16_t instance_idx;
   uint32_t template_name;
   src_loc_SrcLoc template_loc;
   ast_TypeRef rtype;
};

struct ast_ImportDecl_ {
   ast_Decl parent;
   uint32_t alias_idx;
   src_loc_SrcLoc alias_loc;
   ast_Module* dest;
};

struct ast_StructLayout_ {
   uint32_t size;
   uint32_t alignment;
   uint32_t attr_alignment;
   uint32_t offset;
};

struct ast_StructTypeDecl_ {
   ast_Decl parent;
   ast_StructLayout layout;
   uint32_t num_members;
   uint32_t num_struct_functions;
   ast_FunctionDecl** struct_functions;
   ast_Decl* members[0];
};

typedef enum {
   ast_VarDeclKind_GlobalVar,
   ast_VarDeclKind_LocalVar,
   ast_VarDeclKind_FunctionParam,
   ast_VarDeclKind_StructMember,
} __attribute__((packed)) ast_VarDeclKind;

struct ast_VarDecl_ {
   ast_Decl parent;
   ast_TypeRef typeRef;
};

struct ast_EnumTypeDecl_ {
   ast_Decl parent;
   ast_QualType implType;
   ast_EnumConstantDecl* constants[0];
};

struct ast_EnumConstantDecl_ {
   ast_Decl parent;
   uint32_t value;
   ast_Expr* init[0];
};

struct ast_StmtBits_ {
   uint32_t kind : 6;
};

struct ast_ReturnStmtBits_ {
   uint32_t  : 6;
   uint32_t has_value : 1;
};

struct ast_SwitchStmtBits_ {
   uint32_t  : 6;
   uint32_t is_sswitch : 1;
   uint32_t num_cases : 25;
};

struct ast_CaseStmtBits_ {
   uint32_t  : 6;
   uint32_t num_stmts : 15;
   uint32_t has_decls : 1;
};

struct ast_DefaultStmtBits_ {
   uint32_t  : 6;
   uint32_t num_stmts : 15;
   uint32_t has_decls : 1;
};

struct ast_CompoundStmtBits_ {
   uint32_t  : 6;
   uint32_t count : 26;
};

struct ast_ExprBits_ {
   uint32_t  : 6;
   uint32_t kind : 8;
   uint32_t is_ctv : 1;
   uint32_t is_ctc : 1;
   uint32_t valtype : 2;
   uint32_t has_effect : 1;
};

struct ast_IfStmtBits_ {
   uint32_t  : 6;
   uint32_t has_else : 1;
};

struct ast_BuiltinExprBits_ {
   uint32_t  : 19;
   uint32_t kind : 3;
};

struct ast_BooleanLiteralBits_ {
   uint32_t  : 19;
   uint32_t value : 1;
};

struct ast_CharLiteralBits_ {
   uint32_t  : 19;
   uint32_t value : 8;
};

struct ast_IdentifierExprBits_ {
   uint32_t  : 19;
   uint32_t has_decl : 1;
   uint32_t kind : 4;
};

struct ast_MemberExprBits_ {
   uint32_t  : 19;
   uint32_t kind : 4;
   uint32_t num_refs : 3;
   uint32_t num_decls : 3;
   uint32_t has_expr : 1;
   uint32_t is_struct_func : 1;
   uint32_t is_static_sf : 1;
};

struct ast_IntegerLiteralBits_ {
   uint32_t  : 19;
   uint32_t radix : 5;
   uint32_t is_signed : 1;
};

struct ast_UnaryOperatorBits_ {
   uint32_t  : 19;
   uint32_t kind : 4;
};

struct ast_BinaryOperatorBits_ {
   uint32_t  : 19;
   uint32_t kind : 5;
};

struct ast_CallExprBits_ {
   uint32_t  : 19;
   uint32_t calls_struct_func : 1;
   uint32_t calls_static_sf : 1;
   uint32_t is_template_call : 1;
};

struct ast_InitListExprBits_ {
   uint32_t  : 19;
   uint32_t num_values : 13;
};

struct ast_ImplicitCastBits_ {
   uint32_t  : 19;
   uint32_t kind : 2;
};

struct ast_Stmt_ {
   union {
      ast_StmtBits stmtBits;
      ast_ReturnStmtBits returnStmtBits;
      ast_SwitchStmtBits switchStmtBits;
      ast_CaseStmtBits caseStmtBits;
      ast_DefaultStmtBits defaultStmtBits;
      ast_CompoundStmtBits compoundStmtBits;
      ast_ExprBits exprBits;
      ast_IfStmtBits ifStmtBits;
      ast_BuiltinExprBits builtinExprBits;
      ast_BooleanLiteralBits booleanLiteralBits;
      ast_CharLiteralBits charLiteralBits;
      ast_IdentifierExprBits identifierExprBits;
      ast_MemberExprBits memberExprBits;
      ast_IntegerLiteralBits integerLiteralBits;
      ast_UnaryOperatorBits unaryOperatorBits;
      ast_BinaryOperatorBits binaryOperatorBits;
      ast_CallExprBits callExprBits;
      ast_InitListExprBits initListExprBits;
      ast_ImplicitCastBits implicitCastBits;
      uint32_t bits;
   };
};

struct ast_DeclStmt_ {
   ast_Stmt parent;
   ast_VarDecl* decl;
};

struct ast_AliasTypeDecl_ {
   ast_Decl parent;
   ast_TypeRef typeRef;
};

struct ast_StaticAssertDecl_ {
   ast_Decl parent;
   ast_Expr* lhs;
   ast_Expr* rhs;
};

struct ast_FunctionTypeDecl_ {
   ast_Decl parent;
   ast_FunctionDecl* fn;
};

typedef enum {
   ast_StmtKind_Return,
   ast_StmtKind_Expr,
   ast_StmtKind_If,
   ast_StmtKind_While,
   ast_StmtKind_Do,
   ast_StmtKind_For,
   ast_StmtKind_Switch,
   ast_StmtKind_Case,
   ast_StmtKind_Default,
   ast_StmtKind_Break,
   ast_StmtKind_Continue,
   ast_StmtKind_Fallthrough,
   ast_StmtKind_Label,
   ast_StmtKind_Goto,
   ast_StmtKind_Compound,
   ast_StmtKind_Decl,
   ast_StmtKind_Assert,
} __attribute__((packed)) ast_StmtKind;

struct ast_AssertStmt_ {
   ast_Stmt parent;
   ast_Expr* inner;
};

struct ast_CompoundStmt_ {
   ast_Stmt parent;
   src_loc_SrcLoc endLoc;
   ast_Stmt* stmts[0];
};

struct ast_DoStmt_ {
   ast_Stmt parent;
   ast_Stmt* cond;
   ast_Stmt* body;
};

struct ast_ForStmt_ {
   ast_Stmt parent;
   ast_Stmt* init;
   ast_Expr* cond;
   ast_Expr* incr;
   ast_Stmt* body;
};

struct ast_IfStmt_ {
   ast_Stmt parent;
   ast_Stmt* cond;
   ast_Stmt* then;
   ast_Stmt* else_stmt[0];
};

struct ast_ReturnStmt_ {
   ast_Stmt parent;
   src_loc_SrcLoc loc;
   ast_Expr* value[0];
};

struct ast_SwitchStmt_ {
   ast_Stmt parent;
   ast_Stmt* cond;
   ast_Stmt* cases[0];
};

struct ast_CaseStmt_ {
   ast_Stmt parent;
   src_loc_SrcLoc loc;
   ast_Expr* cond;
   ast_Stmt* stmts[0];
};

struct ast_DefaultStmt_ {
   ast_Stmt parent;
   src_loc_SrcLoc loc;
   ast_Stmt* stmts[0];
};

struct ast_BreakStmt_ {
   ast_Stmt parent;
   src_loc_SrcLoc loc;
};

struct ast_ContinueStmt_ {
   ast_Stmt parent;
   src_loc_SrcLoc loc;
};

struct ast_FallthroughStmt_ {
   ast_Stmt parent;
   src_loc_SrcLoc loc;
};

struct ast_WhileStmt_ {
   ast_Stmt parent;
   ast_Stmt* cond;
   ast_Stmt* body;
};

struct ast_LabelStmt_ {
   ast_Stmt parent;
   src_loc_SrcLoc loc;
   uint32_t name;
};

struct ast_GotoStmt_ {
   ast_Stmt parent;
   src_loc_SrcLoc loc;
   uint32_t name;
};

typedef enum {
   ast_ExprKind_IntegerLiteral,
   ast_ExprKind_BooleanLiteral,
   ast_ExprKind_CharLiteral,
   ast_ExprKind_StringLiteral,
   ast_ExprKind_Nil,
   ast_ExprKind_Identifier,
   ast_ExprKind_Type,
   ast_ExprKind_Call,
   ast_ExprKind_InitList,
   ast_ExprKind_FieldDesignatedInit,
   ast_ExprKind_ArrayDesignatedInit,
   ast_ExprKind_BinaryOperator,
   ast_ExprKind_UnaryOperator,
   ast_ExprKind_ConditionalOperator,
   ast_ExprKind_Builtin,
   ast_ExprKind_ArraySubscript,
   ast_ExprKind_Member,
   ast_ExprKind_Paren,
   ast_ExprKind_BitOffset,
   ast_ExprKind_ExplicitCast,
   ast_ExprKind_ImplicitCast,
} __attribute__((packed)) ast_ExprKind;

typedef enum {
   ast_ValType_NValue,
   ast_ValType_RValue,
   ast_ValType_LValue,
} __attribute__((packed)) ast_ValType;

struct ast_Expr_ {
   ast_Stmt parent;
   src_loc_SrcLoc loc;
   ast_QualType qt;
};

typedef enum {
   ast_BuiltinExprKind_Sizeof,
   ast_BuiltinExprKind_Elemsof,
   ast_BuiltinExprKind_EnumMin,
   ast_BuiltinExprKind_EnumMax,
   ast_BuiltinExprKind_OffsetOf,
   ast_BuiltinExprKind_ToContainer,
} __attribute__((packed)) ast_BuiltinExprKind;

struct ast_ToContainerData_ {
   ast_Expr* member;
   ast_Expr* pointer;
};

struct ast_OffsetOfData_ {
   ast_Expr* member;
};

struct ast_BuiltinExpr_ {
   ast_Expr parent;
   ast_Expr* inner;
   size_t value;
   ast_OffsetOfData offset[0];
   ast_ToContainerData container[0];
};

struct ast_BooleanLiteral_ {
   ast_Expr parent;
};

struct ast_CharLiteral_ {
   ast_Expr parent;
};

struct ast_StringLiteral_ {
   ast_Expr parent;
   uint32_t value;
};

struct ast_IntegerLiteral_ {
   ast_Expr parent;
   uint64_t val;
};

struct ast_NilExpr_ {
   ast_Expr parent;
};

typedef enum {
   ast_IdentifierKind_Unresolved,
   ast_IdentifierKind_Module,
   ast_IdentifierKind_Function,
   ast_IdentifierKind_Type,
   ast_IdentifierKind_Var,
   ast_IdentifierKind_EnumConstant,
   ast_IdentifierKind_StructMember,
   ast_IdentifierKind_Label,
} __attribute__((packed)) ast_IdentifierKind;

struct ast_IdentifierExpr_ {
   ast_Expr parent;
   union {
      uint32_t name_idx;
      ast_Decl* decl;
   };
};

struct ast_CallExpr_ {
   ast_Expr parent;
   src_loc_SrcLoc endLoc;
   uint16_t template_idx;
   uint8_t num_args;
   ast_Expr* fn;
   ast_Expr* args[0];
};

struct ast_InitListExpr_ {
   ast_Expr parent;
   src_loc_SrcLoc right;
   ast_Expr* values[0];
};

struct ast_FieldDesignatedInitExpr_ {
   ast_Expr parent;
   uint32_t field;
   ast_Expr* initValue;
};

struct ast_ArrayDesignatedInitExpr_ {
   ast_Expr parent;
   ast_Expr* designator;
   ast_Expr* initValue;
};

struct ast_ParenExpr_ {
   ast_Expr parent;
   ast_Expr* inner;
};

typedef enum {
   ast_UnaryOpcode_PostInc,
   ast_UnaryOpcode_PostDec,
   ast_UnaryOpcode_PreInc,
   ast_UnaryOpcode_PreDec,
   ast_UnaryOpcode_AddrOf,
   ast_UnaryOpcode_Deref,
   ast_UnaryOpcode_Minus,
   ast_UnaryOpcode_Not,
   ast_UnaryOpcode_LNot,
} __attribute__((packed)) ast_UnaryOpcode;

struct ast_UnaryOperator_ {
   ast_Expr parent;
   ast_Expr* inner;
};

struct ast_ConditionalOperator_ {
   ast_Expr parent;
   src_loc_SrcLoc colonLoc;
   ast_Expr* cond;
   ast_Expr* lhs;
   ast_Expr* rhs;
};

typedef enum {
   ast_BinaryOpcode_Multiply,
   ast_BinaryOpcode_Divide,
   ast_BinaryOpcode_Reminder,
   ast_BinaryOpcode_Add,
   ast_BinaryOpcode_Subtract,
   ast_BinaryOpcode_ShiftLeft,
   ast_BinaryOpcode_ShiftRight,
   ast_BinaryOpcode_LessThan,
   ast_BinaryOpcode_GreaterThan,
   ast_BinaryOpcode_LessEqual,
   ast_BinaryOpcode_GreaterEqual,
   ast_BinaryOpcode_Equal,
   ast_BinaryOpcode_NotEqual,
   ast_BinaryOpcode_And,
   ast_BinaryOpcode_Xor,
   ast_BinaryOpcode_Or,
   ast_BinaryOpcode_LAnd,
   ast_BinaryOpcode_LOr,
   ast_BinaryOpcode_Assign,
   ast_BinaryOpcode_MulAssign,
   ast_BinaryOpcode_DivAssign,
   ast_BinaryOpcode_RemAssign,
   ast_BinaryOpcode_AddAssign,
   ast_BinaryOpcode_SubAssign,
   ast_BinaryOpcode_ShlAssign,
   ast_BinaryOpcode_ShrASsign,
   ast_BinaryOpcode_AndAssign,
   ast_BinaryOpcode_XorAssign,
   ast_BinaryOpcode_OrAssign,
} __attribute__((packed)) ast_BinaryOpcode;

struct ast_BinaryOperator_ {
   ast_Expr parent;
   ast_Expr* lhs;
   ast_Expr* rhs;
};

struct ast_TypeExpr_ {
   ast_Expr parent;
   ast_TypeRef typeRef;
};

struct ast_BitOffsetExpr_ {
   ast_Expr parent;
   ast_Expr* lhs;
   ast_Expr* rhs;
};

struct ast_ArraySubscriptExpr_ {
   ast_Expr parent;
   ast_Expr* base;
   ast_Expr* idx;
};

union ast_MemberRef_ {
   uint32_t name_idx;
   ast_Decl* decl;
   ast_Expr* expr;
};

struct ast_MemberExpr_ {
   ast_Expr parent;
   ast_MemberRef refs[0];
};

struct ast_ExplicitCastExpr_ {
   ast_Expr parent;
   ast_Expr* inner;
   ast_TypeRef dest;
};

typedef enum {
   ast_ImplicitCastKind_ArrayToPointerDecay,
   ast_ImplicitCastKind_FunctionToPointerDecay,
   ast_ImplicitCastKind_LValueToRValue,
   ast_ImplicitCastKind_PointerToBoolean,
   ast_ImplicitCastKind_PointerToInteger,
} __attribute__((packed)) ast_ImplicitCastKind;

struct ast_ImplicitCastExpr_ {
   ast_Expr parent;
   ast_Expr* inner;
};

struct ast_Stat_ {
   uint32_t count;
   uint32_t size;
};

struct ast_Stats_ {
   ast_Stat types[8];
   ast_Stat exprs[21];
   ast_Stat stmts[17];
   ast_Stat decls[9];
};

typedef enum {
   ast_TypeKind_Builtin,
   ast_TypeKind_Pointer,
   ast_TypeKind_Array,
   ast_TypeKind_Struct,
   ast_TypeKind_Enum,
   ast_TypeKind_Function,
   ast_TypeKind_Alias,
   ast_TypeKind_Module,
} __attribute__((packed)) ast_TypeKind;

struct ast_TypeBits_ {
   uint32_t kind : 8;
};

struct ast_BuiltinTypeBits_ {
   uint32_t  : 8;
   uint32_t kind : 4;
};

struct ast_ArrayTypeBits_ {
   uint32_t  : 8;
   uint32_t has_size : 1;
};

struct ast_Type_ {
   union {
      ast_TypeBits typeBits;
      ast_BuiltinTypeBits builtinTypeBits;
      ast_ArrayTypeBits arrayTypeBits;
      uint32_t bits;
   };
   uint32_t ptr_pool_idx;
   ast_QualType canonicalType;
};

typedef enum {
   ast_BuiltinKind_Char,
   ast_BuiltinKind_Int8,
   ast_BuiltinKind_Int16,
   ast_BuiltinKind_Int32,
   ast_BuiltinKind_Int64,
   ast_BuiltinKind_UInt8,
   ast_BuiltinKind_UInt16,
   ast_BuiltinKind_UInt32,
   ast_BuiltinKind_UInt64,
   ast_BuiltinKind_Float32,
   ast_BuiltinKind_Float64,
   ast_BuiltinKind_ISize,
   ast_BuiltinKind_USize,
   ast_BuiltinKind_Bool,
   ast_BuiltinKind_Void,
} __attribute__((packed)) ast_BuiltinKind;

struct ast_BuiltinType_ {
   ast_Type parent;
};

struct ast_PointerType_ {
   ast_Type parent;
   ast_QualType inner;
};

struct ast_ArrayType_ {
   ast_Type parent;
   ast_QualType elem;
   uint32_t size;
};

struct ast_StructType_ {
   ast_Type parent;
   ast_StructTypeDecl* decl;
};

struct ast_EnumType_ {
   ast_Type parent;
   ast_EnumTypeDecl* decl;
};

struct ast_FunctionType_ {
   ast_Type parent;
   ast_FunctionDecl* decl;
};

struct ast_AliasType_ {
   ast_Type parent;
   ast_AliasTypeDecl* decl;
};

struct ast_ModuleType_ {
   ast_Type parent;
   ast_Module* mod;
};

struct ast_TypeRefHolder_ {
   uint64_t ref;
   ast_Ref user;
   ast_Ref prefix;
   ast_Expr* arrays[3];
};

struct ast_ImportDeclList_ {
   uint32_t count;
   uint32_t capacity;
   ast_ImportDecl** decls;
};

struct ast_DeclList_ {
   uint32_t count;
   uint32_t capacity;
   ast_Decl** decls;
};

struct ast_FunctionDeclList_ {
   uint32_t count;
   uint32_t capacity;
   ast_FunctionDecl** decls;
};

struct ast_AST_ {
   const char* filename;
   ast_Module* mod;
   void* ptr;
   uint32_t idx;
   ast_ImportDeclList imports;
   ast_DeclList types;
   ast_DeclList variables;
   ast_FunctionDeclList functions;
   ast_DeclList static_asserts;
   attr_table_Table* attrs;
};

typedef void (*ast_ImportVisitor)(void*, ast_ImportDecl*);

typedef void (*ast_FunctionVisitor)(void*, ast_FunctionDecl*);

typedef void (*ast_TypeDeclVisitor)(void*, ast_Decl*);

typedef void (*ast_VarDeclVisitor)(void*, ast_VarDecl*);

typedef void (*ast_DeclVisitor)(void*, ast_Decl*);

struct ast_SymbolTable_ {
   uint32_t num_symbols;
   uint32_t max_symbols;
   uint32_t* symbols;
   ast_DeclList decls;
};

struct ast_InstanceTable_ {
   uint32_t count;
   uint32_t capacity;
   ast_TemplateFunction* funcs;
};

struct ast_Module_ {
   uint32_t name_idx;
   bool used;
   ast_ModuleType* mt;
   ast_AST** files;
   uint32_t num_files;
   uint32_t max_files;
   ast_SymbolTable symbols;
   ast_InstanceTable instances;
};

typedef void (*ast_ASTVisitor)(void*, ast_AST*);

typedef void (*ast_TemplateVisitor)(void*, ast_FunctionDecl*, uint32_t);

struct ast_ExprList_ {
   uint32_t count;
   uint32_t capacity;
   ast_Expr** exprs;
};

struct ast_TemplateInstance_ {
   ast_QualType qt;
   ast_FunctionDecl* instance;
};

struct ast_TemplateFunction_ {
   ast_FunctionDecl* fd;
   uint16_t count;
   uint16_t capacity;
   ast_TemplateInstance* instances;
};

struct ast_PointerPoolSlot_ {
   ast_Type* ptrs[4];
};

struct ast_PointerPool_ {
   ast_context_Context* context;
   uint32_t count;
   uint32_t capacity;
   ast_PointerPoolSlot* slots;
};

struct ast_StringTypeSlot_ {
   uint32_t len;
   ast_Type* type_;
};

struct ast_StringTypePool_ {
   uint32_t count;
   uint32_t capacity;
   ast_StringTypeSlot* slots;
   ast_context_Context* context;
};

struct ast_Instantiator_ {
   ast_context_Context* c;
   const ast_TypeRef* ref;
   uint32_t template_name;
};

static void ast_Decl_init(ast_Decl* d, ast_DeclKind k, uint32_t name_idx, src_loc_SrcLoc loc, bool is_public, ast_QualType qt, uint32_t ast_idx);
static ast_DeclKind ast_Decl_getKind(const ast_Decl* d);
static ast_DeclCheckState ast_Decl_getCheckState(const ast_Decl* d);
static void ast_Decl_setCheckState(ast_Decl* d, ast_DeclCheckState s);
static bool ast_Decl_isChecked(const ast_Decl* d);
static void ast_Decl_setChecked(ast_Decl* d);
static void ast_Decl_setHasAttr(ast_Decl* d);
static bool ast_Decl_hasAttr(const ast_Decl* d);
static void ast_Decl_setAttrExport(ast_Decl* d);
static void ast_Decl_setAttrUnused(ast_Decl* d);
static bool ast_Decl_isExported(const ast_Decl* d);
static const char* ast_Decl_getName(const ast_Decl* d);
static uint32_t ast_Decl_getNameIdx(const ast_Decl* d);
static const char* ast_Decl_getModuleName(const ast_Decl* d);
static src_loc_SrcLoc ast_Decl_getLoc(const ast_Decl* d);
static ast_QualType ast_Decl_getType(const ast_Decl* d);
static void ast_Decl_setType(ast_Decl* d, ast_QualType qt);
static ast_AST* ast_Decl_getAST(const ast_Decl* d);
static uint32_t ast_Decl_getASTIdx(const ast_Decl* d);
static ast_Module* ast_Decl_getModule(const ast_Decl* d);
static bool ast_Decl_isPublic(const ast_Decl* d);
static bool ast_Decl_isUsed(const ast_Decl* d);
static bool ast_Decl_isUsedPublic(const ast_Decl* d);
static void ast_Decl_setUsed(ast_Decl* d);
static void ast_Decl_setUsedPublic(ast_Decl* d);
static bool ast_Decl_isExternal(const ast_Decl* d);
static void ast_Decl_setExternal(ast_Decl* d);
static bool ast_Decl_isGenerated(const ast_Decl* d);
static void ast_Decl_setGenerated(ast_Decl* d);
static void ast_Decl_dump(const ast_Decl* d);
static bool ast_Decl_isTypeDecl(const ast_Decl* d);
static const char* ast_Decl_getKindName(const ast_Decl* d);
static const char* ast_Decl_getCName(const ast_Decl* d);
static const char* ast_Decl_getFullName(const ast_Decl* d);
static void ast_Decl_print(const ast_Decl* d, string_buffer_Buf* out, uint32_t indent);
static void ast_Decl_printKind(const ast_Decl* d, string_buffer_Buf* out, uint32_t indent, bool print_type);
static void ast_Decl_printName(const ast_Decl* d, string_buffer_Buf* out);
static void ast_Decl_printBits(const ast_Decl* d, string_buffer_Buf* out);
static void ast_Decl_printAttrs(const ast_Decl* d, string_buffer_Buf* out);
static void ast_Decl_printUsed(const ast_Decl* d, string_buffer_Buf* out);
static ast_FunctionDecl* ast_FunctionDecl_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, bool is_public, uint32_t ast_idx, const ast_TypeRefHolder* rtype, const ast_Ref* prefix, ast_VarDecl** params, uint32_t num_params, bool is_variadic);
static ast_FunctionDecl* ast_FunctionDecl_createTemplate(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, bool is_public, uint32_t ast_idx, const ast_TypeRefHolder* rtype, uint32_t template_name, src_loc_SrcLoc template_loc, ast_VarDecl** params, uint32_t num_params, bool is_variadic);
static ast_FunctionDecl* ast_FunctionDecl_instantiate(const ast_FunctionDecl* fd, ast_context_Context* c, const ast_TypeRef* ref);
static void ast_FunctionDecl_setBody(ast_FunctionDecl* d, ast_CompoundStmt* body);
static ast_CompoundStmt* ast_FunctionDecl_getBody(const ast_FunctionDecl* d);
static void ast_FunctionDecl_setRType(ast_FunctionDecl* d, ast_QualType rt);
static ast_QualType ast_FunctionDecl_getRType(const ast_FunctionDecl* d);
static bool ast_FunctionDecl_hasReturn(const ast_FunctionDecl* d);
static ast_Decl* ast_FunctionDecl_asDecl(ast_FunctionDecl* d);
static ast_TypeRef* ast_FunctionDecl_getTypeRef(ast_FunctionDecl* d);
static bool ast_FunctionDecl_hasPrefix(const ast_FunctionDecl* d);
static bool ast_FunctionDecl_isTemplate(const ast_FunctionDecl* d);
static uint32_t ast_FunctionDecl_getTemplateNameIdx(const ast_FunctionDecl* d);
static src_loc_SrcLoc ast_FunctionDecl_getTemplateLoc(const ast_FunctionDecl* d);
static void ast_FunctionDecl_setTemplateInstanceIdx(ast_FunctionDecl* d, uint16_t idx);
static uint16_t ast_FunctionDecl_getTemplateInstanceIdx(const ast_FunctionDecl* d);
static void ast_FunctionDecl_setInstanceName(ast_FunctionDecl* d, uint32_t name_idx);
static ast_Ref* ast_FunctionDecl_getPrefix(const ast_FunctionDecl* d);
static const char* ast_FunctionDecl_getPrefixName(const ast_FunctionDecl* d);
static void ast_FunctionDecl_setCallKind(ast_FunctionDecl* d, ast_CallKind kind);
static ast_CallKind ast_FunctionDecl_getCallKind(const ast_FunctionDecl* d);
static bool ast_FunctionDecl_isVariadic(const ast_FunctionDecl* d);
static uint32_t ast_FunctionDecl_getNumParams(const ast_FunctionDecl* d);
static ast_VarDecl** ast_FunctionDecl_getParams(ast_FunctionDecl* d);
static void ast_FunctionDecl_setAttrUnusedParams(ast_FunctionDecl* d);
static void ast_FunctionDecl_setAttrNoReturn(ast_FunctionDecl* d);
static void ast_FunctionDecl_setAttrInline(ast_FunctionDecl* d);
static void ast_FunctionDecl_setAttrWeak(ast_FunctionDecl* d);
static void ast_FunctionDecl_print(const ast_FunctionDecl* d, string_buffer_Buf* out, uint32_t indent);
static void ast_FunctionDecl_printType(const ast_FunctionDecl* d, string_buffer_Buf* out);
static ast_ImportDecl* ast_ImportDecl_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, uint32_t alias_name, src_loc_SrcLoc alias_loc, uint32_t ast_idx, bool is_local);
static ast_Decl* ast_ImportDecl_asDecl(ast_ImportDecl* d);
static const char* ast_ImportDecl_getAliasName(const ast_ImportDecl* d);
static uint32_t ast_ImportDecl_getImportNameIdx(const ast_ImportDecl* d);
static void ast_ImportDecl_setDest(ast_ImportDecl* d, ast_Module* mod);
static ast_Module* ast_ImportDecl_getDest(const ast_ImportDecl* d);
static bool ast_ImportDecl_isLocal(const ast_ImportDecl* d);
static void ast_ImportDecl_print(const ast_ImportDecl* d, string_buffer_Buf* out, uint32_t indent);
static ast_StructTypeDecl* ast_StructTypeDecl_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, bool is_public, uint32_t ast_idx, bool is_struct, bool is_global, ast_VarDecl** members, uint32_t num_members);
static ast_Decl* ast_StructTypeDecl_asDecl(ast_StructTypeDecl* d);
static uint32_t ast_StructTypeDecl_getNumMembers(const ast_StructTypeDecl* d);
static ast_Decl** ast_StructTypeDecl_getMembers(ast_StructTypeDecl* d);
static bool ast_StructTypeDecl_isStruct(const ast_StructTypeDecl* d);
static bool ast_StructTypeDecl_isUnion(const ast_StructTypeDecl* d);
static uint32_t ast_StructTypeDecl_getOffset(const ast_StructTypeDecl* d);
static void ast_StructTypeDecl_setOffset(ast_StructTypeDecl* d, uint32_t offset);
static uint32_t ast_StructTypeDecl_getSize(const ast_StructTypeDecl* d);
static void ast_StructTypeDecl_setSize(ast_StructTypeDecl* d, uint32_t size);
static uint32_t ast_StructTypeDecl_getAlignment(const ast_StructTypeDecl* d);
static void ast_StructTypeDecl_setAlignment(ast_StructTypeDecl* d, uint32_t alignment);
static uint32_t ast_StructTypeDecl_getAttrAlignment(const ast_StructTypeDecl* d);
static void ast_StructTypeDecl_setAttrAlignment(ast_StructTypeDecl* d, uint32_t alignment);
static void ast_StructTypeDecl_setPacked(ast_StructTypeDecl* d);
static bool ast_StructTypeDecl_isPacked(const ast_StructTypeDecl* d);
static void ast_StructTypeDecl_setOpaque(ast_StructTypeDecl* d);
static bool ast_StructTypeDecl_isOpaque(const ast_StructTypeDecl* d);
static bool ast_StructTypeDecl_isGlobal(const ast_StructTypeDecl* d);
static void ast_StructTypeDecl_setAttrNoTypeDef(ast_StructTypeDecl* d);
static bool ast_StructTypeDecl_hasAttrNoTypeDef(const ast_StructTypeDecl* d);
static void ast_StructTypeDecl_setStructFunctions(ast_StructTypeDecl* d, ast_context_Context* c, ast_FunctionDecl** funcs, uint32_t count);
static ast_Decl* ast_StructTypeDecl_findAny(const ast_StructTypeDecl* s, uint32_t name_idx);
static ast_Decl* ast_StructTypeDecl_findMember(const ast_StructTypeDecl* s, uint32_t name_idx, uint32_t* offset);
static void ast_StructTypeDecl_print(const ast_StructTypeDecl* d, string_buffer_Buf* out, uint32_t indent);
static ast_VarDecl* ast_VarDecl_create(ast_context_Context* c, ast_VarDeclKind kind, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* ref, uint32_t ast_idx, ast_Expr* initValue);
static ast_VarDecl* ast_VarDecl_createStructMember(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* ref, uint32_t ast_idx, ast_Expr* bitfield);
static ast_VarDecl* ast_VarDecl_instantiate(const ast_VarDecl* vd, ast_Instantiator* inst);
static ast_Decl* ast_VarDecl_asDecl(ast_VarDecl* d);
static ast_VarDeclKind ast_VarDecl_getKind(const ast_VarDecl* d);
static bool ast_VarDecl_isGlobal(const ast_VarDecl* d);
static bool ast_VarDecl_isLocal(const ast_VarDecl* d);
static ast_TypeRef* ast_VarDecl_getTypeRef(ast_VarDecl* d);
static bool ast_VarDecl_hasInit(const ast_VarDecl* d);
static ast_Expr* ast_VarDecl_getInit(const ast_VarDecl* d);
static ast_Expr** ast_VarDecl_getInit2(ast_VarDecl* d);
static ast_Expr* ast_VarDecl_getBitfield(const ast_VarDecl* d);
static bool ast_VarDecl_hasLocalQualifier(const ast_VarDecl* d);
static void ast_VarDecl_setLocal(ast_VarDecl* d, bool has_local);
static void ast_VarDecl_setAttrWeak(ast_VarDecl* d);
static void ast_VarDecl_setOffset(ast_VarDecl* d, uint32_t offset);
static uint32_t ast_VarDecl_getOffset(const ast_VarDecl* d);
static uint32_t* ast_VarDecl_getOffsetPtr(const ast_VarDecl* d);
static void ast_VarDecl_print(const ast_VarDecl* d, string_buffer_Buf* out, uint32_t indent);
static void ast_VarDecl_printType(const ast_VarDecl* d, string_buffer_Buf* out);
static ast_EnumTypeDecl* ast_EnumTypeDecl_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, bool is_public, uint32_t ast_idx, ast_QualType implType, bool is_incr, ast_EnumConstantDecl** constants, uint32_t num_constants);
static void ast_EnumTypeDecl_setIncrMembers(ast_EnumTypeDecl* d, ast_Decl** constants, uint32_t num_constants);
static ast_QualType ast_EnumTypeDecl_getImplType(const ast_EnumTypeDecl* d);
static ast_Decl* ast_EnumTypeDecl_asDecl(ast_EnumTypeDecl* d);
static uint32_t ast_EnumTypeDecl_getNumConstants(const ast_EnumTypeDecl* d);
static ast_EnumConstantDecl** ast_EnumTypeDecl_getConstants(ast_EnumTypeDecl* d);
static ast_EnumConstantDecl* ast_EnumTypeDecl_findConstant(const ast_EnumTypeDecl* d, uint32_t name_idx);
static void ast_EnumTypeDecl_print(const ast_EnumTypeDecl* d, string_buffer_Buf* out, uint32_t indent);
static ast_EnumConstantDecl* ast_EnumConstantDecl_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, bool is_public, uint32_t ast_idx, ast_Expr* initValue);
static ast_Decl* ast_EnumConstantDecl_asDecl(ast_EnumConstantDecl* d);
static uint32_t ast_EnumConstantDecl_getValue(const ast_EnumConstantDecl* d);
static void ast_EnumConstantDecl_setValue(ast_EnumConstantDecl* d, uint32_t value);
static ast_Expr* ast_EnumConstantDecl_getInit(const ast_EnumConstantDecl* d);
static ast_Expr** ast_EnumConstantDecl_getInit2(ast_EnumConstantDecl* d);
static void ast_EnumConstantDecl_print(const ast_EnumConstantDecl* d, string_buffer_Buf* out, uint32_t indent);
static ast_DeclStmt* ast_DeclStmt_create(ast_context_Context* c, ast_VarDecl* decl);
static ast_Stmt* ast_DeclStmt_instantiate(ast_DeclStmt* s, ast_Instantiator* inst);
static ast_VarDecl* ast_DeclStmt_getDecl(const ast_DeclStmt* d);
static void ast_DeclStmt_print(const ast_DeclStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_AliasTypeDecl* ast_AliasTypeDecl_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, bool is_public, uint32_t ast_idx, const ast_TypeRefHolder* ref);
static ast_Decl* ast_AliasTypeDecl_asDecl(ast_AliasTypeDecl* d);
static ast_TypeRef* ast_AliasTypeDecl_getTypeRef(ast_AliasTypeDecl* d);
static void ast_AliasTypeDecl_print(const ast_AliasTypeDecl* d, string_buffer_Buf* out, uint32_t indent);
static ast_StaticAssertDecl* ast_StaticAssertDecl_create(ast_context_Context* c, uint32_t ast_idx, src_loc_SrcLoc loc, ast_Expr* lhs, ast_Expr* rhs);
static ast_Decl* ast_StaticAssertDecl_asDecl(ast_StaticAssertDecl* d);
static ast_Expr* ast_StaticAssertDecl_getLhs(const ast_StaticAssertDecl* d);
static ast_Expr* ast_StaticAssertDecl_getRhs(const ast_StaticAssertDecl* d);
static void ast_StaticAssertDecl_print(const ast_StaticAssertDecl* d, string_buffer_Buf* out, uint32_t indent);
static ast_FunctionTypeDecl* ast_FunctionTypeDecl_create(ast_context_Context* c, ast_FunctionDecl* fn);
static ast_Decl* ast_FunctionTypeDecl_asDecl(ast_FunctionTypeDecl* t);
static ast_FunctionDecl* ast_FunctionTypeDecl_getDecl(const ast_FunctionTypeDecl* d);
static void ast_FunctionTypeDecl_print(const ast_FunctionTypeDecl* d, string_buffer_Buf* out, uint32_t indent);
static void ast_Stmt_init(ast_Stmt* s, ast_StmtKind k);
static ast_Stmt* ast_Stmt_instantiate(ast_Stmt* s, ast_Instantiator* inst);
static ast_StmtKind ast_Stmt_getKind(const ast_Stmt* s);
static void ast_Stmt_dump(const ast_Stmt* s);
static void ast_Stmt_print(const ast_Stmt* s, string_buffer_Buf* out, uint32_t indent);
static void ast_Stmt_printKind(const ast_Stmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_AssertStmt* ast_AssertStmt_create(ast_context_Context* c, ast_Expr* inner);
static ast_Stmt* ast_AssertStmt_instantiate(ast_AssertStmt* s, ast_Instantiator* inst);
static void ast_AssertStmt_print(const ast_AssertStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_Expr* ast_AssertStmt_getInner(const ast_AssertStmt* s);
static ast_Expr** ast_AssertStmt_getInner2(ast_AssertStmt* s);
static ast_CompoundStmt* ast_CompoundStmt_create(ast_context_Context* c, src_loc_SrcLoc end, ast_Stmt** stmts, uint32_t count);
static ast_CompoundStmt* ast_CompoundStmt_instantiate(const ast_CompoundStmt* s, ast_Instantiator* inst);
static src_loc_SrcLoc ast_CompoundStmt_getEndLoc(const ast_CompoundStmt* s);
static uint32_t ast_CompoundStmt_getCount(const ast_CompoundStmt* s);
static ast_Stmt** ast_CompoundStmt_getStmts(ast_CompoundStmt* s);
static ast_Stmt* ast_CompoundStmt_getLastStmt(const ast_CompoundStmt* s);
static void ast_CompoundStmt_print(const ast_CompoundStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_DoStmt* ast_DoStmt_create(ast_context_Context* c, ast_Stmt* cond, ast_Stmt* body);
static ast_Stmt* ast_DoStmt_instantiate(ast_DoStmt* s, ast_Instantiator* inst);
static void ast_DoStmt_print(const ast_DoStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_Stmt* ast_DoStmt_getCond(const ast_DoStmt* s);
static ast_Stmt* ast_DoStmt_getBody(const ast_DoStmt* s);
static ast_ForStmt* ast_ForStmt_create(ast_context_Context* c, ast_Stmt* init_, ast_Expr* cond, ast_Expr* incr, ast_Stmt* body);
static ast_Stmt* ast_ForStmt_instantiate(ast_ForStmt* s, ast_Instantiator* inst);
static ast_Stmt* ast_ForStmt_getInit(const ast_ForStmt* s);
static ast_Expr* ast_ForStmt_getCond(const ast_ForStmt* s);
static ast_Expr* ast_ForStmt_getIncr(const ast_ForStmt* s);
static ast_Stmt* ast_ForStmt_getBody(const ast_ForStmt* s);
static ast_Stmt** ast_ForStmt_getInit2(ast_ForStmt* s);
static ast_Expr** ast_ForStmt_getCond2(ast_ForStmt* s);
static ast_Expr** ast_ForStmt_getIncr2(ast_ForStmt* s);
static ast_Stmt** ast_ForStmt_getBody2(ast_ForStmt* s);
static void ast_ForStmt_print(const ast_ForStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_IfStmt* ast_IfStmt_create(ast_context_Context* c, ast_Stmt* cond, ast_Stmt* then, ast_Stmt* else_stmt);
static ast_Stmt* ast_IfStmt_instantiate(ast_IfStmt* s, ast_Instantiator* inst);
static ast_Stmt* ast_IfStmt_getCond(const ast_IfStmt* s);
static ast_Stmt** ast_IfStmt_getCond2(ast_IfStmt* s);
static ast_Stmt* ast_IfStmt_getThen(const ast_IfStmt* s);
static ast_Stmt* ast_IfStmt_getElse(const ast_IfStmt* s);
static void ast_IfStmt_print(const ast_IfStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_ReturnStmt* ast_ReturnStmt_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* value);
static ast_Stmt* ast_ReturnStmt_instantiate(ast_ReturnStmt* s, ast_Instantiator* inst);
static src_loc_SrcLoc ast_ReturnStmt_getLoc(const ast_ReturnStmt* s);
static ast_Expr* ast_ReturnStmt_getValue(const ast_ReturnStmt* s);
static ast_Expr** ast_ReturnStmt_getValue2(ast_ReturnStmt* s);
static void ast_ReturnStmt_print(const ast_ReturnStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_SwitchStmt* ast_SwitchStmt_create(ast_context_Context* c, ast_Stmt* cond, ast_Stmt** cases, uint32_t numCases, bool is_sswitch);
static ast_Stmt* ast_SwitchStmt_instantiate(ast_SwitchStmt* s, ast_Instantiator* inst);
static ast_Stmt* ast_SwitchStmt_getCond(const ast_SwitchStmt* s);
static ast_Stmt** ast_SwitchStmt_getCond2(ast_SwitchStmt* s);
static bool ast_SwitchStmt_isSSwitch(const ast_SwitchStmt* s);
static uint32_t ast_SwitchStmt_getNumCases(const ast_SwitchStmt* s);
static ast_Stmt** ast_SwitchStmt_getCases(ast_SwitchStmt* s);
static void ast_SwitchStmt_print(const ast_SwitchStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_CaseStmt* ast_CaseStmt_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* cond, ast_Stmt** stmts, uint32_t numStmts);
static ast_Stmt* ast_CaseStmt_instantiate(ast_CaseStmt* s, ast_Instantiator* inst);
static uint32_t ast_CaseStmt_getNumStmts(const ast_CaseStmt* s);
static ast_Stmt** ast_CaseStmt_getStmts(ast_CaseStmt* s);
static src_loc_SrcLoc ast_CaseStmt_getLoc(const ast_CaseStmt* s);
static ast_Expr* ast_CaseStmt_getCond(const ast_CaseStmt* s);
static ast_Expr** ast_CaseStmt_getCond2(ast_CaseStmt* s);
static void ast_CaseStmt_setHasDecls(ast_CaseStmt* s);
static bool ast_CaseStmt_hasDecls(const ast_CaseStmt* s);
static void ast_CaseStmt_print(const ast_CaseStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_DefaultStmt* ast_DefaultStmt_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Stmt** stmts, uint32_t numStmts);
static ast_Stmt* ast_DefaultStmt_instantiate(ast_DefaultStmt* s, ast_Instantiator* inst);
static src_loc_SrcLoc ast_DefaultStmt_getLoc(const ast_DefaultStmt* s);
static uint32_t ast_DefaultStmt_getNumStmts(const ast_DefaultStmt* s);
static ast_Stmt** ast_DefaultStmt_getStmts(ast_DefaultStmt* s);
static void ast_DefaultStmt_setHasDecls(ast_DefaultStmt* s);
static bool ast_DefaultStmt_hasDecls(const ast_DefaultStmt* s);
static void ast_DefaultStmt_print(const ast_DefaultStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_BreakStmt* ast_BreakStmt_create(ast_context_Context* c, src_loc_SrcLoc loc);
static src_loc_SrcLoc ast_BreakStmt_getLoc(const ast_BreakStmt* s);
static void ast_BreakStmt_print(const ast_BreakStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_ContinueStmt* ast_ContinueStmt_create(ast_context_Context* c, src_loc_SrcLoc loc);
static src_loc_SrcLoc ast_ContinueStmt_getLoc(const ast_ContinueStmt* s);
static void ast_ContinueStmt_print(const ast_ContinueStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_FallthroughStmt* ast_FallthroughStmt_create(ast_context_Context* c, src_loc_SrcLoc loc);
static src_loc_SrcLoc ast_FallthroughStmt_getLoc(const ast_FallthroughStmt* s);
static void ast_FallthroughStmt_print(const ast_FallthroughStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_WhileStmt* ast_WhileStmt_create(ast_context_Context* c, ast_Stmt* cond, ast_Stmt* body);
static ast_Stmt* ast_WhileStmt_instantiate(ast_WhileStmt* s, ast_Instantiator* inst);
static void ast_WhileStmt_print(const ast_WhileStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_Stmt* ast_WhileStmt_getCond(const ast_WhileStmt* s);
static ast_Stmt** ast_WhileStmt_getCond2(ast_WhileStmt* s);
static ast_Stmt* ast_WhileStmt_getBody(const ast_WhileStmt* s);
static ast_LabelStmt* ast_LabelStmt_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc);
static const char* ast_LabelStmt_getName(const ast_LabelStmt* s);
static uint32_t ast_LabelStmt_getNameIdx(const ast_LabelStmt* s);
static src_loc_SrcLoc ast_LabelStmt_getLoc(ast_LabelStmt* s);
static void ast_LabelStmt_print(const ast_LabelStmt* s, string_buffer_Buf* out, uint32_t indent);
static ast_GotoStmt* ast_GotoStmt_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc);
static const char* ast_GotoStmt_getName(const ast_GotoStmt* g);
static uint32_t ast_GotoStmt_getNameIdx(const ast_GotoStmt* g);
static src_loc_SrcLoc ast_GotoStmt_getLoc(ast_GotoStmt* g);
static void ast_GotoStmt_print(const ast_GotoStmt* s, string_buffer_Buf* out, uint32_t indent);
static void ast_Expr_init(ast_Expr* e, ast_ExprKind k, src_loc_SrcLoc loc, bool ctv, bool ctc, bool has_effect, ast_ValType valtype);
static ast_Expr* ast_Expr_instantiate(ast_Expr* e, ast_Instantiator* inst);
static ast_Stmt* ast_Expr_asStmt(ast_Expr* e);
static ast_ExprKind ast_Expr_getKind(const ast_Expr* e);
static bool ast_Expr_isCtv(const ast_Expr* e);
static bool ast_Expr_isCtc(const ast_Expr* e);
static void ast_Expr_setCtv(ast_Expr* e);
static void ast_Expr_setCtc(ast_Expr* e);
static void ast_Expr_copyConstantFlags(ast_Expr* e, const ast_Expr* other);
static void ast_Expr_combineConstantFlags(ast_Expr* e, const ast_Expr* lhs, const ast_Expr* rhs);
static bool ast_Expr_hasEffect(const ast_Expr* e);
static ast_ValType ast_Expr_getValType(const ast_Expr* e);
static bool ast_Expr_isNValue(const ast_Expr* e);
static bool ast_Expr_isRValue(const ast_Expr* e);
static bool ast_Expr_isLValue(const ast_Expr* e);
static void ast_Expr_setLValue(ast_Expr* e);
static void ast_Expr_setRValue(ast_Expr* e);
static void ast_Expr_setValType(ast_Expr* e, ast_ValType valtype);
static void ast_Expr_copyValType(ast_Expr* e, const ast_Expr* other);
static src_loc_SrcLoc ast_Expr_getLoc(const ast_Expr* e);
static src_loc_SrcLoc ast_Expr_getStartLoc(const ast_Expr* e);
static src_loc_SrcLoc ast_Expr_getEndLoc(const ast_Expr* e);
static src_loc_SrcRange ast_Expr_getRange(const ast_Expr* e);
static void ast_Expr_setType(ast_Expr* e, ast_QualType qt_);
static ast_QualType ast_Expr_getType(const ast_Expr* e);
static void ast_Expr_dump(const ast_Expr* e);
static void ast_Expr_print(const ast_Expr* e, string_buffer_Buf* out, uint32_t indent);
static void ast_Expr_printLiteral(const ast_Expr* e, string_buffer_Buf* out);
static void ast_Expr_printKind(const ast_Expr* e, string_buffer_Buf* out, uint32_t indent);
static void ast_Expr_printTypeBits(const ast_Expr* e, string_buffer_Buf* out);
static ast_BuiltinExpr* ast_BuiltinExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* inner, ast_BuiltinExprKind kind);
static ast_BuiltinExpr* ast_BuiltinExpr_createOffsetOf(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* typeExpr, ast_Expr* member);
static ast_BuiltinExpr* ast_BuiltinExpr_createToContainer(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* typeExpr, ast_Expr* member, ast_Expr* pointer);
static ast_Expr* ast_BuiltinExpr_instantiate(ast_BuiltinExpr* e, ast_Instantiator* inst);
static ast_BuiltinExprKind ast_BuiltinExpr_getKind(const ast_BuiltinExpr* e);
static size_t ast_BuiltinExpr_getValue(const ast_BuiltinExpr* e);
static void ast_BuiltinExpr_setValue(ast_BuiltinExpr* e, size_t value);
static ast_Expr* ast_BuiltinExpr_getInner(const ast_BuiltinExpr* e);
static src_loc_SrcLoc ast_BuiltinExpr_getEndLoc(const ast_BuiltinExpr* e);
static ast_Expr* ast_BuiltinExpr_getOffsetOfMember(const ast_BuiltinExpr* b);
static ast_Expr* ast_BuiltinExpr_getToContainerMember(const ast_BuiltinExpr* b);
static ast_Expr* ast_BuiltinExpr_getToContainerPointer(const ast_BuiltinExpr* b);
static ast_Expr** ast_BuiltinExpr_getToContainerPointer2(ast_BuiltinExpr* b);
static void ast_BuiltinExpr_print(const ast_BuiltinExpr* e, string_buffer_Buf* out, uint32_t indent);
static void ast_BuiltinExpr_printLiteral(const ast_BuiltinExpr* e, string_buffer_Buf* out);
static ast_BooleanLiteral* ast_BooleanLiteral_create(ast_context_Context* c, src_loc_SrcLoc loc, bool val);
static bool ast_BooleanLiteral_getValue(const ast_BooleanLiteral* e);
static void ast_BooleanLiteral_print(const ast_BooleanLiteral* e, string_buffer_Buf* out, uint32_t indent);
static void ast_BooleanLiteral_printLiteral(const ast_BooleanLiteral* e, string_buffer_Buf* out);
static ast_CharLiteral* ast_CharLiteral_create(ast_context_Context* c, src_loc_SrcLoc loc, uint8_t val);
static uint8_t ast_CharLiteral_getValue(const ast_CharLiteral* e);
static void ast_CharLiteral_print(const ast_CharLiteral* e, string_buffer_Buf* out, uint32_t indent);
static void ast_CharLiteral_printLiteral(const ast_CharLiteral* e, string_buffer_Buf* out);
static ast_StringLiteral* ast_StringLiteral_create(ast_context_Context* c, src_loc_SrcLoc loc, uint32_t value, uint32_t len);
static const char* ast_StringLiteral_getText(const ast_StringLiteral* e);
static void ast_StringLiteral_printLiteral(const ast_StringLiteral* e, string_buffer_Buf* out);
static void ast_StringLiteral_print(const ast_StringLiteral* e, string_buffer_Buf* out, uint32_t indent);
static ast_IntegerLiteral* ast_IntegerLiteral_create(ast_context_Context* c, src_loc_SrcLoc loc, uint8_t radix, uint64_t val);
static ast_IntegerLiteral* ast_IntegerLiteral_createUnsignedConstant(ast_context_Context* c, src_loc_SrcLoc loc, uint64_t val, ast_QualType qt);
static ast_IntegerLiteral* ast_IntegerLiteral_createSignedConstant(ast_context_Context* c, src_loc_SrcLoc loc, int64_t val, ast_QualType qt);
static uint64_t ast_IntegerLiteral_getValue(const ast_IntegerLiteral* e);
static bool ast_IntegerLiteral_isDecimal(const ast_IntegerLiteral* e);
static bool ast_IntegerLiteral_isSigned(const ast_IntegerLiteral* e);
static void ast_printBinary(string_buffer_Buf* out, uint64_t value);
static void ast_printOctal(string_buffer_Buf* out, uint64_t value);
static void ast_IntegerLiteral_print(const ast_IntegerLiteral* e, string_buffer_Buf* out, uint32_t indent);
static void ast_IntegerLiteral_printLiteral(const ast_IntegerLiteral* e, string_buffer_Buf* out);
static void ast_IntegerLiteral_printDecimal(const ast_IntegerLiteral* e, string_buffer_Buf* out);
static ast_NilExpr* ast_NilExpr_create(ast_context_Context* c, src_loc_SrcLoc loc);
static void ast_NilExpr_print(const ast_NilExpr* e, string_buffer_Buf* out, uint32_t indent);
static void ast_NilExpr_printLiteral(const ast_NilExpr* _arg0, string_buffer_Buf* out);
static ast_IdentifierExpr* ast_IdentifierExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, uint32_t name);
static ast_Expr* ast_IdentifierExpr_instantiate(ast_IdentifierExpr* e, ast_Instantiator* inst);
static ast_Expr* ast_IdentifierExpr_asExpr(ast_IdentifierExpr* e);
static ast_Decl* ast_IdentifierExpr_getDecl(const ast_IdentifierExpr* e);
static ast_Ref ast_IdentifierExpr_getRef(const ast_IdentifierExpr* e);
static void ast_IdentifierExpr_setKind(ast_IdentifierExpr* e, ast_IdentifierKind kind);
static ast_IdentifierKind ast_IdentifierExpr_getKind(const ast_IdentifierExpr* e);
static void ast_IdentifierExpr_setDecl(ast_IdentifierExpr* e, ast_Decl* decl);
static const char* ast_IdentifierExpr_getName(const ast_IdentifierExpr* e);
static uint32_t ast_IdentifierExpr_getNameIdx(const ast_IdentifierExpr* e);
static void ast_IdentifierExpr_print(const ast_IdentifierExpr* e, string_buffer_Buf* out, uint32_t indent);
static void ast_IdentifierExpr_printLiteral(const ast_IdentifierExpr* e, string_buffer_Buf* out);
static ast_CallExpr* ast_CallExpr_create(ast_context_Context* c, src_loc_SrcLoc endLoc, ast_Expr* fn, ast_Expr** args, uint32_t num_args);
static ast_CallExpr* ast_CallExpr_createTemplate(ast_context_Context* c, src_loc_SrcLoc endLoc, ast_Expr* fn, ast_Expr** args, uint32_t num_args, const ast_TypeRefHolder* ref);
static ast_Expr* ast_CallExpr_instantiate(ast_CallExpr* e, ast_Instantiator* inst);
static void ast_CallExpr_setCallsStructFunc(ast_CallExpr* e);
static bool ast_CallExpr_isStructFunc(const ast_CallExpr* e);
static void ast_CallExpr_setCallsStaticStructFunc(ast_CallExpr* e);
static bool ast_CallExpr_isStaticStructFunc(const ast_CallExpr* e);
static bool ast_CallExpr_isTemplateCall(const ast_CallExpr* e);
static ast_TypeRef* ast_CallExpr_getTemplateArg(const ast_CallExpr* e);
static void ast_CallExpr_setTemplateIdx(ast_CallExpr* e, uint32_t idx);
static uint32_t ast_CallExpr_getTemplateIdx(const ast_CallExpr* e);
static src_loc_SrcLoc ast_CallExpr_getEndLoc(const ast_CallExpr* e);
static ast_Expr* ast_CallExpr_getFunc(const ast_CallExpr* e);
static ast_Expr** ast_CallExpr_getFunc2(ast_CallExpr* e);
static uint32_t ast_CallExpr_getNumArgs(const ast_CallExpr* e);
static ast_Expr** ast_CallExpr_getArgs(ast_CallExpr* e);
static void ast_CallExpr_print(const ast_CallExpr* e, string_buffer_Buf* out, uint32_t indent);
static ast_InitListExpr* ast_InitListExpr_create(ast_context_Context* c, src_loc_SrcLoc left, src_loc_SrcLoc right, ast_Expr** values, uint32_t num_values);
static ast_Expr* ast_InitListExpr_instantiate(ast_InitListExpr* e, ast_Instantiator* inst);
static uint32_t ast_InitListExpr_getNumValues(const ast_InitListExpr* e);
static ast_Expr** ast_InitListExpr_getValues(ast_InitListExpr* e);
static void ast_InitListExpr_print(const ast_InitListExpr* e, string_buffer_Buf* out, uint32_t indent);
static ast_FieldDesignatedInitExpr* ast_FieldDesignatedInitExpr_create(ast_context_Context* c, uint32_t field, src_loc_SrcLoc loc, ast_Expr* initValue);
static ast_Expr* ast_FieldDesignatedInitExpr_instantiate(ast_FieldDesignatedInitExpr* e, ast_Instantiator* inst);
static uint32_t ast_FieldDesignatedInitExpr_getField(const ast_FieldDesignatedInitExpr* e);
static const char* ast_FieldDesignatedInitExpr_getFieldName(const ast_FieldDesignatedInitExpr* e);
static ast_Expr* ast_FieldDesignatedInitExpr_getInit(ast_FieldDesignatedInitExpr* e);
static ast_Expr** ast_FieldDesignatedInitExpr_getInit2(ast_FieldDesignatedInitExpr* e);
static void ast_FieldDesignatedInitExpr_print(const ast_FieldDesignatedInitExpr* e, string_buffer_Buf* out, uint32_t indent);
static ast_ArrayDesignatedInitExpr* ast_ArrayDesignatedInitExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* designator, ast_Expr* initValue);
static ast_Expr* ast_ArrayDesignatedInitExpr_instantiate(ast_ArrayDesignatedInitExpr* e, ast_Instantiator* inst);
static ast_Expr* ast_ArrayDesignatedInitExpr_getDesignator(const ast_ArrayDesignatedInitExpr* e);
static ast_Expr** ast_ArrayDesignatedInitExpr_getDesignator2(ast_ArrayDesignatedInitExpr* e);
static ast_Expr* ast_ArrayDesignatedInitExpr_getInit(const ast_ArrayDesignatedInitExpr* e);
static ast_Expr** ast_ArrayDesignatedInitExpr_getInit2(ast_ArrayDesignatedInitExpr* e);
static void ast_ArrayDesignatedInitExpr_print(const ast_ArrayDesignatedInitExpr* e, string_buffer_Buf* out, uint32_t indent);
static ast_ParenExpr* ast_ParenExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* inner);
static ast_Expr* ast_ParenExpr_instantiate(ast_ParenExpr* e, ast_Instantiator* inst);
static ast_Expr* ast_ParenExpr_getInner(const ast_ParenExpr* e);
static ast_Expr** ast_ParenExpr_getInner2(ast_ParenExpr* e);
static void ast_ParenExpr_print(const ast_ParenExpr* e, string_buffer_Buf* out, uint32_t indent);
static void ast_ParenExpr_printLiteral(const ast_ParenExpr* e, string_buffer_Buf* out);
static ast_UnaryOperator* ast_UnaryOperator_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_UnaryOpcode kind, ast_Expr* inner);
static ast_Expr* ast_UnaryOperator_instantiate(ast_UnaryOperator* e, ast_Instantiator* inst);
static ast_UnaryOpcode ast_UnaryOperator_getOpcode(const ast_UnaryOperator* e);
static ast_Expr* ast_UnaryOperator_getInner(const ast_UnaryOperator* e);
static ast_Expr** ast_UnaryOperator_getInner2(ast_UnaryOperator* e);
static ast_Expr* ast_UnaryOperator_asExpr(ast_UnaryOperator* e);
static bool ast_UnaryOperator_isBefore(const ast_UnaryOperator* e);
static src_loc_SrcLoc ast_UnaryOperator_getStartLoc(const ast_UnaryOperator* e);
static src_loc_SrcLoc ast_UnaryOperator_getEndLoc(const ast_UnaryOperator* e);
static const char* ast_UnaryOperator_getOpcodeStr(const ast_UnaryOperator* e);
static void ast_UnaryOperator_print(const ast_UnaryOperator* e, string_buffer_Buf* out, uint32_t indent);
static void ast_UnaryOperator_printLiteral(const ast_UnaryOperator* e, string_buffer_Buf* out);
static ast_ConditionalOperator* ast_ConditionalOperator_create(ast_context_Context* c, src_loc_SrcLoc questionLoc, src_loc_SrcLoc colonLoc, ast_Expr* cond, ast_Expr* lhs, ast_Expr* rhs);
static ast_Expr* ast_ConditionalOperator_instantiate(ast_ConditionalOperator* e, ast_Instantiator* inst);
static ast_Expr* ast_ConditionalOperator_getCond(const ast_ConditionalOperator* e);
static ast_Expr** ast_ConditionalOperator_getCond2(ast_ConditionalOperator* e);
static ast_Expr* ast_ConditionalOperator_getLHS(const ast_ConditionalOperator* e);
static ast_Expr** ast_ConditionalOperator_getLHS2(ast_ConditionalOperator* e);
static ast_Expr* ast_ConditionalOperator_getRHS(const ast_ConditionalOperator* e);
static ast_Expr** ast_ConditionalOperator_getRHS2(ast_ConditionalOperator* e);
static void ast_ConditionalOperator_print(const ast_ConditionalOperator* e, string_buffer_Buf* out, uint32_t indent);
static ast_BinaryOperator* ast_BinaryOperator_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_BinaryOpcode kind, ast_Expr* lhs, ast_Expr* rhs);
static ast_Expr* ast_BinaryOperator_instantiate(ast_BinaryOperator* e, ast_Instantiator* inst);
static ast_BinaryOpcode ast_BinaryOperator_getOpcode(const ast_BinaryOperator* e);
static ast_Expr* ast_BinaryOperator_getLHS(const ast_BinaryOperator* e);
static ast_Expr** ast_BinaryOperator_getLHS2(ast_BinaryOperator* e);
static ast_Expr* ast_BinaryOperator_getRHS(const ast_BinaryOperator* e);
static ast_Expr** ast_BinaryOperator_getRHS2(ast_BinaryOperator* e);
static const char* ast_BinaryOperator_getOpcodeStr(const ast_BinaryOperator* e);
static void ast_BinaryOperator_print(const ast_BinaryOperator* e, string_buffer_Buf* out, uint32_t indent);
static void ast_BinaryOperator_printLiteral(const ast_BinaryOperator* e, string_buffer_Buf* out);
static ast_TypeExpr* ast_TypeExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, const ast_TypeRefHolder* ref);
static ast_Expr* ast_TypeExpr_instantiate(ast_TypeExpr* e, ast_Instantiator* inst);
static ast_TypeRef* ast_TypeExpr_getTypeRef(ast_TypeExpr* e);
static void ast_TypeExpr_print(const ast_TypeExpr* e, string_buffer_Buf* out, uint32_t indent);
static ast_BitOffsetExpr* ast_BitOffsetExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* lhs, ast_Expr* rhs);
static ast_Expr* ast_BitOffsetExpr_instantiate(ast_BitOffsetExpr* e, ast_Instantiator* inst);
static ast_Expr* ast_BitOffsetExpr_getLHS(ast_BitOffsetExpr* e);
static ast_Expr** ast_BitOffsetExpr_getLHS2(ast_BitOffsetExpr* e);
static ast_Expr* ast_BitOffsetExpr_getRHS(ast_BitOffsetExpr* e);
static ast_Expr** ast_BitOffsetExpr_getRHS2(ast_BitOffsetExpr* e);
static void ast_BitOffsetExpr_print(const ast_BitOffsetExpr* e, string_buffer_Buf* out, uint32_t indent);
static ast_ArraySubscriptExpr* ast_ArraySubscriptExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* base, ast_Expr* idx);
static ast_Expr* ast_ArraySubscriptExpr_instantiate(ast_ArraySubscriptExpr* e, ast_Instantiator* inst);
static ast_Expr* ast_ArraySubscriptExpr_getBase(const ast_ArraySubscriptExpr* e);
static ast_Expr** ast_ArraySubscriptExpr_getBase2(ast_ArraySubscriptExpr* e);
static ast_Expr* ast_ArraySubscriptExpr_getIndex(const ast_ArraySubscriptExpr* e);
static ast_Expr** ast_ArraySubscriptExpr_getIndex2(ast_ArraySubscriptExpr* e);
static void ast_ArraySubscriptExpr_print(const ast_ArraySubscriptExpr* e, string_buffer_Buf* out, uint32_t indent);
static ast_MemberExpr* ast_MemberExpr_create(ast_context_Context* c, ast_Expr* base, const ast_Ref* refs, uint32_t refcount);
static ast_Expr* ast_MemberExpr_instantiate(ast_MemberExpr* e, ast_Instantiator* inst);
static bool ast_MemberExpr_hasExpr(const ast_MemberExpr* e);
static ast_Expr* ast_MemberExpr_getExprBase(const ast_MemberExpr* e);
static const char* ast_MemberExpr_getName(const ast_MemberExpr* e, uint32_t ref_idx);
static uint32_t ast_MemberExpr_getNumRefs(const ast_MemberExpr* e);
static uint32_t ast_MemberExpr_getNameIdx(const ast_MemberExpr* e, uint32_t ref_idx);
static src_loc_SrcLoc ast_MemberExpr_getLoc(const ast_MemberExpr* e, uint32_t ref_idx);
static ast_IdentifierKind ast_MemberExpr_getKind(const ast_MemberExpr* e);
static void ast_MemberExpr_setKind(ast_MemberExpr* e, ast_IdentifierKind kind);
static void ast_MemberExpr_setIsStructFunc(ast_MemberExpr* e);
static bool ast_MemberExpr_isStructFunc(const ast_MemberExpr* e);
static void ast_MemberExpr_setIsStaticStructFunc(ast_MemberExpr* e);
static bool ast_MemberExpr_isStaticStructFunc(const ast_MemberExpr* e);
static ast_Decl* ast_MemberExpr_getFullDecl(const ast_MemberExpr* e);
static ast_Decl* ast_MemberExpr_getDecl(const ast_MemberExpr* e, uint32_t ref_idx);
static void ast_MemberExpr_setDecl(ast_MemberExpr* e, ast_Decl* d, uint32_t ref_idx);
static src_loc_SrcLoc ast_MemberExpr_getStartLoc(const ast_MemberExpr* e);
static src_loc_SrcLoc ast_MemberExpr_getEndLoc(const ast_MemberExpr* e);
static ast_QualType ast_MemberExpr_getBaseType(const ast_MemberExpr* m);
static void ast_MemberExpr_print(const ast_MemberExpr* e, string_buffer_Buf* out, uint32_t indent);
static void ast_MemberExpr_printLiteral(const ast_MemberExpr* e, string_buffer_Buf* out);
static void ast_MemberExpr_dump(const ast_MemberExpr* m);
static ast_ExplicitCastExpr* ast_ExplicitCastExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, const ast_TypeRefHolder* ref, ast_Expr* inner);
static ast_Expr* ast_ExplicitCastExpr_instantiate(ast_ExplicitCastExpr* e, ast_Instantiator* inst);
static ast_Expr* ast_ExplicitCastExpr_getInner(const ast_ExplicitCastExpr* e);
static ast_Expr** ast_ExplicitCastExpr_getInner2(ast_ExplicitCastExpr* e);
static ast_TypeRef* ast_ExplicitCastExpr_getTypeRef(ast_ExplicitCastExpr* e);
static void ast_ExplicitCastExpr_print(const ast_ExplicitCastExpr* e, string_buffer_Buf* out, uint32_t indent);
static ast_ImplicitCastExpr* ast_ImplicitCastExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_ImplicitCastKind kind, ast_Expr* inner);
static ast_ImplicitCastKind ast_ImplicitCastExpr_getKind(const ast_ImplicitCastExpr* e);
static ast_Expr* ast_ImplicitCastExpr_getInner(const ast_ImplicitCastExpr* e);
static void ast_ImplicitCastExpr_print(const ast_ImplicitCastExpr* e, string_buffer_Buf* out, uint32_t indent);
static void ast_init(ast_context_Context* c, const char* names_start, uint32_t wordsize);
static void ast_deinit(bool print_stats);
static const char* ast_idx2name(uint32_t idx);
static ast_Type* ast_getPointerType(ast_QualType inner);
static uint32_t ast_addAST(ast_AST* ast_);
static uint32_t ast_ast2idx(const ast_AST* ast_);
static ast_AST* ast_idx2ast(uint32_t idx);
static void ast_Stats_reset(ast_Stats* s);
static void ast_Stats_addType(ast_TypeKind kind, uint32_t size);
static void ast_Stats_addExpr(ast_ExprKind kind, uint32_t size);
static void ast_Stats_addStmt(ast_StmtKind kind, uint32_t size);
static void ast_Stats_addDecl(ast_DeclKind kind, uint32_t size);
static void ast_Stats_dump(const ast_Stats* s);
static void ast_Type_init(ast_Type* t, ast_TypeKind k);
static ast_TypeKind ast_Type_getKind(const ast_Type* t);
static bool ast_Type_hasCanonicalType(const ast_Type* t);
static ast_QualType ast_Type_getCanonicalType(const ast_Type* t);
static void ast_Type_setCanonicalType(ast_Type* t, ast_QualType canon);
static uint32_t ast_Type_getIndex(const ast_Type* t);
static ast_ArrayType* ast_Type_asArray(ast_Type* t);
static bool ast_Type_isBuiltinType(const ast_Type* t);
static bool ast_Type_isArrayType(const ast_Type* t);
static bool ast_Type_isStructType(const ast_Type* t);
static bool ast_Type_isPointerType(const ast_Type* t);
static bool ast_Type_isVoidType(const ast_Type* t);
static void ast_Type_dump(const ast_Type* t);
static uint32_t ast_Type_getAlignment(const ast_Type* t);
static uint32_t ast_Type_getSize(const ast_Type* t);
static void ast_Type_print(const ast_Type* t, string_buffer_Buf* out);
static void ast_Type_fullPrint(const ast_Type* t, string_buffer_Buf* out, uint32_t indent);
static ast_QualType ast_QualType_init(ast_Type* t);
static void ast_QualType_set(ast_QualType* qt, ast_Type* t);
static void ast_QualType_setConst(ast_QualType* qt);
static bool ast_QualType_isConst(ast_QualType* qt);
static bool ast_QualType_isVolatile(ast_QualType* qt);
static void ast_QualType_setVolatile(ast_QualType* qt);
static uint32_t ast_QualType_getQuals(const ast_QualType* qt);
static void ast_QualType_copyQuals(ast_QualType* qt, ast_QualType other);
static void ast_QualType_clearQuals(ast_QualType* qt);
static bool ast_QualType_isConstant(const ast_QualType* qt);
static bool ast_QualType_isValid(const ast_QualType* qt);
static bool ast_QualType_isInvalid(const ast_QualType* qt);
static ast_Type* ast_QualType_getType(const ast_QualType* qt);
static ast_Type* ast_QualType_getTypeOrNil(const ast_QualType* qt);
static bool ast_QualType_hasCanonicalType(const ast_QualType* qt);
static ast_QualType ast_QualType_getCanonicalType(const ast_QualType* qt);
static void ast_QualType_setCanonicalType(ast_QualType* qt, ast_QualType canon);
static ast_TypeKind ast_QualType_getKind(ast_QualType* qt);
static uint32_t ast_QualType_getIndex(ast_QualType* qt);
static uint32_t ast_QualType_getAlignment(ast_QualType* qt);
static uint32_t ast_QualType_getSize(ast_QualType* qt);
static bool ast_QualType_isBuiltinType(const ast_QualType* qt);
static bool ast_QualType_isArrayType(const ast_QualType* qt);
static bool ast_QualType_isStructType(const ast_QualType* qt);
static bool ast_QualType_isPointerType(const ast_QualType* qt);
static bool ast_QualType_isVoidType(const ast_QualType* qt);
static ast_BuiltinType* ast_QualType_getBuiltinType(const ast_QualType* qt);
static ast_BuiltinType* ast_QualType_getBuiltinTypeOrNil(const ast_QualType* qt);
static ast_StructType* ast_QualType_getStructType(const ast_QualType* qt);
static ast_PointerType* ast_QualType_getPointerType(const ast_QualType* qt);
static ast_ArrayType* ast_QualType_getArrayType(const ast_QualType* qt);
static ast_FunctionType* ast_QualType_getFunctionTypeOrNil(const ast_QualType* qt);
static ast_StructType* ast_QualType_getStructTypeOrNil(const ast_QualType* qt);
static ast_PointerType* ast_QualType_getPointerTypeOrNil(const ast_QualType* qt);
static ast_ArrayType* ast_QualType_getArrayTypeOrNil(const ast_QualType* qt);
static ast_EnumType* ast_QualType_getEnumTypeOrNil(const ast_QualType* qt);
static bool ast_QualType_needsCtvInit(const ast_QualType* qt);
static const char* ast_QualType_diagName(const ast_QualType* qt);
static void ast_QualType_dump(const ast_QualType* qt);
static void ast_QualType_dump_full(const ast_QualType* qt);
static void ast_QualType_printQuoted(const ast_QualType* qt, string_buffer_Buf* out);
static void ast_QualType_print(const ast_QualType* qt, string_buffer_Buf* out);
static void ast_QualType_printInner(const ast_QualType* qt, string_buffer_Buf* out, bool printCanon);
static void ast_QualType_fullPrint(const ast_QualType* qt, string_buffer_Buf* out, uint32_t indent);
static ast_BuiltinType* ast_BuiltinType_create(ast_context_Context* c, ast_BuiltinKind kind);
static ast_BuiltinKind ast_BuiltinType_getKind(const ast_BuiltinType* b);
static const char* ast_BuiltinType_kind2str(const ast_BuiltinType* b);
static bool ast_BuiltinType_isPromotableIntegerType(const ast_BuiltinType* b);
static uint32_t ast_BuiltinType_getAlignment(const ast_BuiltinType* b);
static uint32_t ast_BuiltinType_getWidth(const ast_BuiltinType* b);
static void ast_BuiltinType_print(const ast_BuiltinType* b, string_buffer_Buf* out);
static void ast_BuiltinType_fullPrint(const ast_BuiltinType* t, string_buffer_Buf* out, uint32_t indent);
static ast_PointerType* ast_PointerType_create(ast_context_Context* c, ast_QualType inner);
static ast_Type* ast_PointerType_asType(ast_PointerType* t);
static ast_QualType ast_PointerType_getInner(const ast_PointerType* t);
static void ast_PointerType_print(const ast_PointerType* t, string_buffer_Buf* out);
static void ast_PointerType_fullPrint(const ast_PointerType* t, string_buffer_Buf* out, uint32_t indent);
static ast_ArrayType* ast_ArrayType_create(ast_context_Context* c, ast_QualType elem, bool has_size, uint32_t size);
static ast_Type* ast_ArrayType_asType(ast_ArrayType* t);
static ast_QualType ast_ArrayType_getElemType(const ast_ArrayType* t);
static uint32_t ast_ArrayType_hasSize(const ast_ArrayType* t);
static uint32_t ast_ArrayType_getSize(const ast_ArrayType* t);
static void ast_ArrayType_setSize(ast_ArrayType* t, uint32_t size);
static void ast_ArrayType_print(const ast_ArrayType* t, string_buffer_Buf* out);
static void ast_ArrayType_fullPrint(const ast_ArrayType* t, string_buffer_Buf* out, uint32_t indent);
static ast_StructType* ast_StructType_create(ast_context_Context* c, ast_StructTypeDecl* decl);
static ast_StructTypeDecl* ast_StructType_getDecl(const ast_StructType* t);
static ast_Type* ast_StructType_asType(ast_StructType* t);
static void ast_StructType_print(const ast_StructType* t, string_buffer_Buf* out);
static void ast_StructType_fullPrint(const ast_StructType* t, string_buffer_Buf* out, uint32_t indent);
static ast_EnumType* ast_EnumType_create(ast_context_Context* c, ast_EnumTypeDecl* decl);
static ast_EnumTypeDecl* ast_EnumType_getDecl(const ast_EnumType* t);
static ast_Type* ast_EnumType_asType(ast_EnumType* t);
static const char* ast_EnumType_getName(const ast_EnumType* t);
static void ast_EnumType_print(const ast_EnumType* t, string_buffer_Buf* out);
static void ast_EnumType_fullPrint(const ast_EnumType* t, string_buffer_Buf* out, uint32_t indent);
static ast_FunctionType* ast_FunctionType_create(ast_context_Context* c, ast_FunctionDecl* decl);
static ast_FunctionDecl* ast_FunctionType_getDecl(const ast_FunctionType* t);
static ast_Type* ast_FunctionType_asType(ast_FunctionType* t);
static void ast_FunctionType_print(const ast_FunctionType* t, string_buffer_Buf* out);
static void ast_FunctionType_fullPrint(const ast_FunctionType* t, string_buffer_Buf* out, uint32_t indent);
static ast_AliasType* ast_AliasType_create(ast_context_Context* c, ast_AliasTypeDecl* decl);
static ast_AliasTypeDecl* ast_AliasType_getDecl(const ast_AliasType* t);
static void ast_AliasType_print(const ast_AliasType* t, string_buffer_Buf* out);
static void ast_AliasType_fullPrint(const ast_AliasType* t, string_buffer_Buf* out, uint32_t indent);
static ast_ModuleType* ast_ModuleType_create(ast_context_Context* c, ast_Module* mod);
static ast_Type* ast_ModuleType_asType(ast_ModuleType* t);
static ast_Module* ast_ModuleType_getModule(const ast_ModuleType* t);
static void ast_ModuleType_print(const ast_ModuleType* t, string_buffer_Buf* out);
static void ast_ModuleType_fullPrint(const ast_ModuleType* t, string_buffer_Buf* out, uint32_t indent);
static const char* ast_Ref_getName(const ast_Ref* r);
static void ast_TypeRefHolder_init(ast_TypeRefHolder* h);
static uint32_t ast_TypeRefHolder_getExtraSize(const ast_TypeRefHolder* h);
static void ast_TypeRefHolder_setQualifiers(ast_TypeRefHolder* h, uint32_t qualifiers);
static void ast_TypeRefHolder_addPointer(ast_TypeRefHolder* h);
static void ast_TypeRefHolder_setIncrArray(ast_TypeRefHolder* h);
static uint32_t ast_TypeRefHolder_getNumArrays(const ast_TypeRefHolder* h);
static void ast_TypeRefHolder_addArray(ast_TypeRefHolder* h, ast_Expr* array);
static void ast_TypeRefHolder_setBuiltin(ast_TypeRefHolder* h, ast_BuiltinKind kind);
static void ast_TypeRefHolder_setUser(ast_TypeRefHolder* h, src_loc_SrcLoc loc, uint32_t name_idx);
static void ast_TypeRefHolder_setPrefix(ast_TypeRefHolder* h, src_loc_SrcLoc loc, uint32_t name_idx);
static void ast_TypeRefHolder_fill(const ast_TypeRefHolder* h, ast_TypeRef* dest);
static void ast_TypeRefHolder_dump(const ast_TypeRefHolder* h);
static bool ast_TypeRef_matchesTemplate(const ast_TypeRef* r, uint32_t template_arg);
static void ast_TypeRef_initRef0(ast_TypeRef* r, const ast_TypeRef* r2);
static void ast_TypeRef_instantiate(ast_TypeRef* r, const ast_TypeRef* r1, const ast_TypeRef* r2, uint32_t template_arg);
static void ast_TypeRef_setDest(ast_TypeRef* r, uint32_t dest);
static uint32_t ast_TypeRef_getDest(const ast_TypeRef* r);
static uint32_t ast_TypeRef_getExtraSize(const ast_TypeRef* r);
static uint32_t ast_TypeRef_getMaxSizeNoArray(void);
static void* ast_TypeRef_getPointerAfter(const ast_TypeRef* r);
static bool ast_TypeRef_isConst(const ast_TypeRef* r);
static bool ast_TypeRef_isVolatile(const ast_TypeRef* r);
static bool ast_TypeRef_isUser(const ast_TypeRef* r);
static bool ast_TypeRef_hasPrefix(const ast_TypeRef* r);
static bool ast_TypeRef_isIncrArray(const ast_TypeRef* r);
static bool ast_TypeRef_isPointerTo(const ast_TypeRef* r, uint32_t ptr_idx);
static ast_BuiltinKind ast_TypeRef_getBuiltinKind(const ast_TypeRef* r);
static uint32_t ast_TypeRef_getNumPointers(const ast_TypeRef* r);
static const ast_Ref* ast_TypeRef_getUser(const ast_TypeRef* r);
static const ast_Ref* ast_TypeRef_getPrefix(const ast_TypeRef* r);
static void ast_TypeRef_setPrefix(ast_TypeRef* r, ast_Decl* d);
static void ast_TypeRef_setUser(ast_TypeRef* r, ast_Decl* d);
static uint32_t ast_TypeRef_getNumArrays(const ast_TypeRef* r);
static ast_Expr* ast_TypeRef_getArray(const ast_TypeRef* r, uint32_t idx);
static ast_Expr** ast_TypeRef_getArray2(ast_TypeRef* r, uint32_t idx);
static void ast_TypeRef_print(const ast_TypeRef* r, string_buffer_Buf* out, bool filled);
static void ast_TypeRef_dump(const ast_TypeRef* r);
static ast_AST* ast_AST_create(const char* filename, ast_Module* mod);
static void ast_AST_free(ast_AST* a);
static const char* ast_AST_getFilename(const ast_AST* a);
static uint32_t ast_AST_getIdx(const ast_AST* a);
static uint32_t ast_AST_getNameIdx(const ast_AST* a);
static void ast_AST_setPtr(ast_AST* a, void* ptr);
static void* ast_AST_getPtr(const ast_AST* a);
static ast_Module* ast_AST_getMod(const ast_AST* a);
static void ast_AST_addImport(ast_AST* a, ast_ImportDecl* d);
static ast_ImportDecl* ast_AST_findImport(const ast_AST* a, uint32_t name);
static void ast_AST_addFunc(ast_AST* a, ast_FunctionDecl* d);
static void ast_AST_addTypeDecl(ast_AST* a, ast_Decl* d);
static void ast_AST_addVarDecl(ast_AST* a, ast_Decl* d);
static void ast_AST_addStaticAssert(ast_AST* a, ast_StaticAssertDecl* d);
static void ast_AST_visitImports(const ast_AST* a, ast_ImportVisitor visitor, void* arg);
static const ast_ImportDeclList* ast_AST_getImports(const ast_AST* a);
static void ast_AST_visitStructFunctions(const ast_AST* a, ast_FunctionVisitor visitor, void* arg);
static void ast_AST_visitFunctions(const ast_AST* a, ast_FunctionVisitor visitor, void* arg);
static void ast_AST_visitTypeDecls(const ast_AST* a, ast_TypeDeclVisitor visitor, void* arg);
static void ast_AST_visitVarDecls(const ast_AST* a, ast_VarDeclVisitor visitor, void* arg);
static void ast_AST_visitStaticAsserts(const ast_AST* a, ast_DeclVisitor visitor, void* arg);
static void ast_AST_visitDecls(const ast_AST* a, ast_DeclVisitor visitor, void* arg);
static ast_Decl* ast_AST_findType(const ast_AST* a, uint32_t name_idx);
static void ast_AST_storeAttr(ast_AST* a, ast_Decl* d, attr_AttrKind kind, const attr_Value* value);
static const attr_Value* ast_AST_getAttr(const ast_AST* a, const ast_Decl* d, attr_AttrKind kind);
static void ast_AST_info(const ast_AST* a, string_buffer_Buf* out);
static void ast_AST_print(const ast_AST* a, string_buffer_Buf* out, bool show_funcs);
static ast_Module* ast_Module_create(ast_context_Context* c, uint32_t name_idx);
static void ast_Module_free(ast_Module* m);
static void ast_Module_setUsed(ast_Module* m);
static bool ast_Module_isUsed(const ast_Module* m);
static const ast_SymbolTable* ast_Module_getSymbols(const ast_Module* m);
static ast_ModuleType* ast_Module_getType(const ast_Module* m);
static void ast_Module_visitASTs(const ast_Module* m, ast_ASTVisitor visitor, void* arg);
static void ast_Module_visitImports(const ast_Module* m, ast_ImportVisitor visitor, void* arg);
static void ast_Module_visitStructFunctions(const ast_Module* m, ast_FunctionVisitor visitor, void* arg);
static void ast_Module_visitFunctions(const ast_Module* m, ast_FunctionVisitor visitor, void* arg);
static void ast_Module_visitTypeDecls(const ast_Module* m, ast_TypeDeclVisitor visitor, void* arg);
static void ast_Module_visitVarDecls(const ast_Module* m, ast_VarDeclVisitor visitor, void* arg);
static void ast_Module_visitStaticAsserts(const ast_Module* m, ast_DeclVisitor visitor, void* arg);
static void ast_Module_visitDecls(const ast_Module* m, ast_DeclVisitor visitor, void* arg);
static ast_Decl* ast_Module_findType(const ast_Module* m, uint32_t name_idx);
static const char* ast_Module_getName(const ast_Module* m);
static uint32_t ast_Module_getNameIdx(const ast_Module* m);
static void ast_Module_resizeFiles(ast_Module* m, uint32_t cap);
static ast_AST* ast_Module_add(ast_Module* m, const char* filename);
static void ast_Module_addSymbol(ast_Module* m, uint32_t name_idx, ast_Decl* d);
static ast_Decl* ast_Module_findSymbol(const ast_Module* m, uint32_t name_idx);
static ast_FunctionDecl* ast_Module_findInstance(const ast_Module* m, ast_FunctionDecl* fd, ast_QualType qt);
static uint16_t ast_Module_addInstance(ast_Module* m, ast_FunctionDecl* fd, ast_QualType qt, ast_FunctionDecl* instance);
static ast_FunctionDecl* ast_Module_getInstance(const ast_Module* m, ast_FunctionDecl* fd, uint32_t idx);
static void ast_Module_visitInstances(const ast_Module* m, ast_FunctionDecl* fd, ast_TemplateVisitor visitor, void* arg);
static void ast_Module_info(const ast_Module* m, string_buffer_Buf* out);
static void ast_Module_print(const ast_Module* m, string_buffer_Buf* out, bool show_funcs);
static void ast_DeclList_init(ast_DeclList* l, uint32_t initial_size);
static void ast_DeclList_free(ast_DeclList* l);
static void ast_DeclList_add(ast_DeclList* l, ast_Decl* d);
static uint32_t ast_DeclList_size(const ast_DeclList* l);
static ast_Decl* ast_DeclList_get(const ast_DeclList* l, uint32_t idx);
static ast_Decl** ast_DeclList_getDecls(const ast_DeclList* l);
static void ast_DeclList_setSize(ast_DeclList* l, uint32_t size);
static void ast_ExprList_init(ast_ExprList* l, uint32_t initial_size);
static void ast_ExprList_free(ast_ExprList* l);
static void ast_ExprList_add(ast_ExprList* l, ast_Expr* d);
static uint32_t ast_ExprList_size(const ast_ExprList* l);
static ast_Expr** ast_ExprList_getExprs(const ast_ExprList* l);
static void ast_FunctionDeclList_init(ast_FunctionDeclList* l);
static void ast_FunctionDeclList_free(ast_FunctionDeclList* l);
static void ast_FunctionDeclList_add(ast_FunctionDeclList* l, ast_FunctionDecl* d);
static uint32_t ast_FunctionDeclList_size(const ast_FunctionDeclList* l);
static ast_FunctionDecl** ast_FunctionDeclList_getDecls(const ast_FunctionDeclList* l);
static void ast_ImportDeclList_init(ast_ImportDeclList* l);
static void ast_ImportDeclList_free(ast_ImportDeclList* l);
static void ast_ImportDeclList_add(ast_ImportDeclList* l, ast_ImportDecl* d);
static uint32_t ast_ImportDeclList_size(const ast_ImportDeclList* l);
static ast_ImportDecl** ast_ImportDeclList_getDecls(const ast_ImportDeclList* l);
static ast_ImportDecl* ast_ImportDeclList_find(const ast_ImportDeclList* l, uint32_t name_idx);
static ast_ImportDecl* ast_ImportDeclList_findAny(const ast_ImportDeclList* l, uint32_t name_idx);
static void ast_SymbolTable_init(ast_SymbolTable* t, uint32_t initial);
static void ast_SymbolTable_free(ast_SymbolTable* t);
static uint32_t ast_SymbolTable_size(const ast_SymbolTable* t);
static ast_Decl** ast_SymbolTable_getDecls(const ast_SymbolTable* l);
static void ast_SymbolTable_crop(ast_SymbolTable* t, uint32_t size);
static void ast_SymbolTable_resize(ast_SymbolTable* t, uint32_t capacity);
static void ast_SymbolTable_add(ast_SymbolTable* t, uint32_t name_idx, ast_Decl* d);
static ast_Decl* ast_SymbolTable_find(const ast_SymbolTable* t, uint32_t name_idx);
static void ast_SymbolTable_print(const ast_SymbolTable* t, string_buffer_Buf* out);
static void ast_TemplateFunction_init(ast_TemplateFunction* f, ast_FunctionDecl* fd);
static void ast_TemplateFunction_resize(ast_TemplateFunction* f, uint16_t capacity);
static uint16_t ast_TemplateFunction_add(ast_TemplateFunction* f, ast_QualType qt, ast_FunctionDecl* instance);
static ast_FunctionDecl* ast_TemplateFunction_find(const ast_TemplateFunction* f, ast_QualType qt);
static ast_FunctionDecl* ast_TemplateFunction_get(const ast_TemplateFunction* f, uint32_t idx);
static void ast_InstanceTable_init(ast_InstanceTable* t);
static void ast_InstanceTable_free(ast_InstanceTable* t);
static void ast_InstanceTable_resize(ast_InstanceTable* t, uint32_t capacity);
static ast_TemplateFunction* ast_InstanceTable_findFunc(const ast_InstanceTable* t, ast_FunctionDecl* fd);
static ast_FunctionDecl* ast_InstanceTable_find(const ast_InstanceTable* t, ast_FunctionDecl* fd, ast_QualType qt);
static ast_FunctionDecl* ast_InstanceTable_get(const ast_InstanceTable* t, ast_FunctionDecl* fd, uint32_t idx);
static uint16_t ast_InstanceTable_add(ast_InstanceTable* t, ast_FunctionDecl* fd, ast_QualType qt, ast_FunctionDecl* instance);
static void ast_InstanceTable_visit(const ast_InstanceTable* t, ast_FunctionDecl* fd, ast_TemplateVisitor visitor, void* arg);
static void ast_PointerPool_init(ast_PointerPool* p, ast_context_Context* c);
static void ast_PointerPool_clear(ast_PointerPool* p);
static void ast_PointerPool_resize(ast_PointerPool* p, uint32_t cap);
static ast_Type* ast_PointerPool_getPointer(ast_PointerPool* p, ast_QualType qt);
static void ast_StringTypePool_init(ast_StringTypePool* p, ast_context_Context* c);
static void ast_StringTypePool_clear(ast_StringTypePool* p);
static void ast_StringTypePool_resize(ast_StringTypePool* p, uint32_t cap);
static ast_QualType ast_StringTypePool_get(ast_StringTypePool* p, uint32_t len);

static const char* ast_declCheckState_names[] = { "unchecked", "in-progress", "checked" };

static const char* ast_declKind_names[] = {
   "FunctionDecl",
   "ImportDecl",
   "StructTypeDecl",
   "EnumTypeDecl",
   "EnumConstantDecl",
   "FunctionType",
   "AliasTypeDecl",
   "VarDecl",
   "StaticAssert"
};

static const uint32_t ast_NumDeclBits = 18;

static const char* ast_callKind_names[] = { "Invalid", "Normal", "SF", "SSF" };

static const char* ast_varDeclNames[] = { " global", " local", " parameter", " member" };

static const char* ast_stmtKind_names[] = {
   "ReturnStmt",
   "ExprStmt",
   "IfStmt",
   "WhileStmt",
   "DoStmt",
   "ForStmt",
   "SwitchStmt",
   "CaseStmt",
   "DefaultStmt",
   "BreakStmt",
   "ContinueStmt",
   "FallthroughStmt",
   "LabelStmt",
   "GotoStmt",
   "CompoundStmt",
   "DeclStmt",
   "AssertStmt"
};

static const uint32_t ast_NumStmtBits = 6;

static const char* ast_exprKind_names[] = {
   "IntegerLiteral",
   "BooleanLiteral",
   "CharLiteral",
   "StringLiteral",
   "Nil",
   "Identifier",
   "TypeExpr",
   "Call",
   "InitList",
   "FieldDesignatedInit",
   "ArrayDesignatedInit",
   "BinaryOperator",
   "UnaryOperator",
   "ConditionalOp",
   "Builtin",
   "ArraySubscript",
   "Member",
   "Paren",
   "BitOffset",
   "ExplicitCast",
   "ImplicitCast"
};

static const char* ast_valType_names[] = { "nvalue", "rvalue", "lvalue" };

static const uint32_t ast_NumExprBits = ast_NumStmtBits + 13;

static const char* ast_builtin_names[] = { "sizeof", "elemsof", "enum_min", "enum_max", "offsetof", "to_container" };

static const char* ast_identifierKind_names[] = {
   "Unresolved",
   "Module",
   "Function",
   "Type",
   "Var",
   "EnumConstant",
   "StructMember",
   "Label"
};

static const char* ast_unaryOpcode_names[] = {
   "++",
   "--",
   "++",
   "--",
   "&",
   "*",
   "-",
   "~",
   "!"
};

static const char* ast_binaryOpcode_names[] = {
   "*",
   "/",
   "%",
   "+",
   "-",
   "<<",
   ">>",
   "<",
   ">",
   "<=",
   ">=",
   "==",
   "!=",
   "&",
   "^",
   "|",
   "&&",
   "||",
   "=",
   "*=",
   "/=",
   "%=",
   "+=",
   "-=",
   "<<=",
   ">>=",
   "&=",
   "^=",
   "|="
};

static const uint32_t ast_MemberExprMaxDepth = 7;

static const char* ast_implicitCastKind_names[] = { "ArrayToPointerDecay", "FunctionToPointerDecay", "LValueToRValue", "PointerToBoolean", "PointerToInteger" };

static const ast_QualType ast_QualType_Invalid = { };

static ast_QualType ast_g_char;

static ast_QualType ast_g_u8;

static ast_QualType ast_g_u16;

static ast_QualType ast_g_u32;

static ast_QualType ast_g_u64;

static ast_QualType ast_g_i8;

static ast_QualType ast_g_i16;

static ast_QualType ast_g_i32;

static ast_QualType ast_g_i64;

static ast_QualType ast_g_f32;

static ast_QualType ast_g_f64;

static ast_QualType ast_g_isize;

static ast_QualType ast_g_usize;

static ast_QualType ast_g_void;

static ast_QualType ast_g_bool;

static ast_QualType ast_g_void_ptr;

static const char* ast_g_names_start;

static ast_PointerPool ast_g_pointers;

static ast_StringTypePool ast_g_string_types;

static uint32_t ast_g_wordsize;

static uint32_t ast_ast_count;

static uint32_t ast_ast_capacity;

static ast_AST** ast_g_ast_list;

static const char* ast_col_Stmt = color_Bmagenta;

static const char* ast_col_Decl = color_Bgreen;

static const char* ast_col_Expr = color_Bmagenta;

static const char* ast_col_Attr = color_Blue;

static const char* ast_col_Template = color_Green;

static const char* ast_col_Type = color_Green;

static const char* ast_col_Value = color_Bcyan;

static const char* ast_col_Error = color_Red;

static const char* ast_col_Calc = color_Yellow;

static const char* ast_col_Normal = color_Normal;

static ast_Stats ast_stats;

static const char* ast_typeKind_names[] = {
   "Builtin",
   "Pointer",
   "Array",
   "Struct",
   "Enum",
   "Function",
   "Alias",
   "Module"
};

static const uint32_t ast_NumTypeBits = 8;

static const size_t ast_QualType_Const = 0x1;

static const size_t ast_QualType_Volatile = 0x2;

static const size_t ast_QualType_Mask = 0x3;

static const char* ast_builtinType_names[] = {
   "char",
   "i8",
   "i16",
   "i32",
   "i64",
   "u8",
   "u16",
   "u32",
   "u64",
   "f32",
   "f64",
   "isize",
   "usize",
   "bool",
   "void"
};

static const bool ast_BuiltinType_promotable[] = {
   true,
   true,
   true,
   false,
   false,
   true,
   true,
   false,
   false,
   false,
   false,
   false,
   false,
   true,
   false
};

static uint32_t ast_builtinType_sizes[] = {
   1,
   1,
   2,
   4,
   8,
   1,
   2,
   4,
   8,
   4,
   8,
   8,
   8,
   1,
   0
};

static uint32_t ast_builtinType_width[] = {
   7,
   7,
   15,
   31,
   63,
   8,
   16,
   32,
   64,
   0,
   0,
   63,
   64,
   1,
   0
};

static void ast_Decl_init(ast_Decl* d, ast_DeclKind k, uint32_t name_idx, src_loc_SrcLoc loc, bool is_public, ast_QualType qt, uint32_t ast_idx)
{
   d->bits = 0;
   d->declBits.kind = k;
   d->declBits.is_public = is_public;
   d->loc = loc;
   d->name_idx = name_idx;
   d->ast_idx = ast_idx;
   d->qt = qt;
}

static ast_DeclKind ast_Decl_getKind(const ast_Decl* d)
{
   return ((ast_DeclKind)(d->declBits.kind));
}

static ast_DeclCheckState ast_Decl_getCheckState(const ast_Decl* d)
{
   return ((ast_DeclCheckState)(d->declBits.check_state));
}

static void ast_Decl_setCheckState(ast_Decl* d, ast_DeclCheckState s)
{
   d->declBits.check_state = s;
}

static bool ast_Decl_isChecked(const ast_Decl* d)
{
   return d->declBits.check_state == ast_DeclCheckState_Checked;
}

static void ast_Decl_setChecked(ast_Decl* d)
{
   d->declBits.check_state = ast_DeclCheckState_Checked;
}

static void ast_Decl_setHasAttr(ast_Decl* d)
{
   d->declBits.has_attr = 1;
}

static bool ast_Decl_hasAttr(const ast_Decl* d)
{
   return d->declBits.has_attr;
}

static void ast_Decl_setAttrExport(ast_Decl* d)
{
   d->declBits.attr_export = 1;
}

static void ast_Decl_setAttrUnused(ast_Decl* d)
{
   d->declBits.attr_unused = 1;
}

static bool ast_Decl_isExported(const ast_Decl* d)
{
   return d->declBits.attr_export;
}

static const char* ast_Decl_getName(const ast_Decl* d)
{
   if (d->name_idx) return &ast_g_names_start[d->name_idx];

   return NULL;
}

static uint32_t ast_Decl_getNameIdx(const ast_Decl* d)
{
   return d->name_idx;
}

static const char* ast_Decl_getModuleName(const ast_Decl* d)
{
   if (d->ast_idx == 0) return NULL;

   const ast_AST* a = ast_Decl_getAST(d);
   const ast_Module* mod = ast_AST_getMod(a);
   return ast_Module_getName(mod);
}

static src_loc_SrcLoc ast_Decl_getLoc(const ast_Decl* d)
{
   return d->loc;
}

static ast_QualType ast_Decl_getType(const ast_Decl* d)
{
   return d->qt;
}

static void ast_Decl_setType(ast_Decl* d, ast_QualType qt)
{
   d->qt = qt;
}

static ast_AST* ast_Decl_getAST(const ast_Decl* d)
{
   return ast_idx2ast(d->ast_idx);
}

static uint32_t ast_Decl_getASTIdx(const ast_Decl* d)
{
   return d->ast_idx;
}

static ast_Module* ast_Decl_getModule(const ast_Decl* d)
{
   return ast_idx2ast(d->ast_idx)->mod;
}

static bool ast_Decl_isPublic(const ast_Decl* d)
{
   return d->declBits.is_public;
}

static bool ast_Decl_isUsed(const ast_Decl* d)
{
   return d->declBits.is_used;
}

static bool ast_Decl_isUsedPublic(const ast_Decl* d)
{
   return d->declBits.is_used_public;
}

static void ast_Decl_setUsed(ast_Decl* d)
{
   d->declBits.is_used = true;
}

static void ast_Decl_setUsedPublic(ast_Decl* d)
{
   d->declBits.is_used_public = true;
}

static bool ast_Decl_isExternal(const ast_Decl* d)
{
   return d->declBits.is_external;
}

static void ast_Decl_setExternal(ast_Decl* d)
{
   d->declBits.is_external = 1;
}

static bool ast_Decl_isGenerated(const ast_Decl* d)
{
   return d->declBits.is_generated;
}

static void ast_Decl_setGenerated(ast_Decl* d)
{
   d->declBits.is_generated = 1;
}

static void ast_Decl_dump(const ast_Decl* d)
{
   string_buffer_Buf* out = string_buffer_create(10 * 4096, utils_useColor(), 2);
   ast_Decl_print(d, out, 0);
   string_buffer_Buf_color(out, ast_col_Normal);
   puts(string_buffer_Buf_data(out));
   string_buffer_Buf_free(out);
}

static bool ast_Decl_isTypeDecl(const ast_Decl* d)
{
   switch (ast_Decl_getKind(d)) {
   case ast_DeclKind_Function:
      break;
   case ast_DeclKind_Import:
      break;
   case ast_DeclKind_StructType:
      return true;
   case ast_DeclKind_EnumType:
      return true;
   case ast_DeclKind_EnumConstant:
      break;
   case ast_DeclKind_FunctionType:
      return true;
   case ast_DeclKind_AliasType:
      return true;
   case ast_DeclKind_Var:
      break;
   case ast_DeclKind_StaticAssert:
      break;
   }
   return false;
}

static const char* ast_Decl_getKindName(const ast_Decl* d)
{
   switch (ast_Decl_getKind(d)) {
   case ast_DeclKind_Function:
      return "function";
   case ast_DeclKind_Import:
      return "import";
   case ast_DeclKind_StructType:
      return "type";
   case ast_DeclKind_EnumType:
      return "type";
   case ast_DeclKind_EnumConstant:
      return "enum constant";
   case ast_DeclKind_FunctionType:
      return "type";
   case ast_DeclKind_AliasType:
      return "type";
   case ast_DeclKind_Var:
      return "variable";
   case ast_DeclKind_StaticAssert:
      return "static-assert";
   }
   return "";
}

static const char* ast_Decl_getCName(const ast_Decl* d)
{
   if (!ast_Decl_hasAttr(d)) return NULL;

   const ast_AST* a = ast_Decl_getAST(d);
   const attr_Value* cname = ast_AST_getAttr(a, d, attr_AttrKind_CName);
   if (!cname) return NULL;

   return ast_idx2name(cname->text_idx);
}

static const char* ast_Decl_getFullName(const ast_Decl* d)
{
   static char tmp[128];
   if (ast_Decl_getKind(d) != ast_DeclKind_Function) return ast_Decl_getName(d);

   const char* modname = ast_Decl_getModuleName(d);
   const ast_FunctionDecl* fd = ((ast_FunctionDecl*)(d));
   if (ast_FunctionDecl_hasPrefix(fd)) {
      sprintf(tmp, "%s.%s.%s", modname, ast_FunctionDecl_getPrefixName(fd), ast_Decl_getName(d));
   } else {
      sprintf(tmp, "%s.%s", modname, ast_Decl_getName(d));
   }
   return tmp;
}

static void ast_Decl_print(const ast_Decl* d, string_buffer_Buf* out, uint32_t indent)
{
   switch (ast_Decl_getKind(d)) {
   case ast_DeclKind_Function:
      ast_FunctionDecl_print(((ast_FunctionDecl*)(d)), out, indent);
      break;
   case ast_DeclKind_Import:
      ast_ImportDecl_print(((ast_ImportDecl*)(d)), out, indent);
      break;
   case ast_DeclKind_StructType:
      ast_StructTypeDecl_print(((ast_StructTypeDecl*)(d)), out, indent);
      break;
   case ast_DeclKind_EnumType:
      ast_EnumTypeDecl_print(((ast_EnumTypeDecl*)(d)), out, indent);
      break;
   case ast_DeclKind_EnumConstant:
      ast_EnumConstantDecl_print(((ast_EnumConstantDecl*)(d)), out, indent);
      break;
   case ast_DeclKind_FunctionType:
      ast_FunctionTypeDecl_print(((ast_FunctionTypeDecl*)(d)), out, indent);
      break;
   case ast_DeclKind_AliasType:
      ast_AliasTypeDecl_print(((ast_AliasTypeDecl*)(d)), out, indent);
      break;
   case ast_DeclKind_Var:
      ast_VarDecl_print(((ast_VarDecl*)(d)), out, indent);
      break;
   case ast_DeclKind_StaticAssert:
      ast_StaticAssertDecl_print(((ast_StaticAssertDecl*)(d)), out, indent);
      break;
   }
}

static void ast_Decl_printKind(const ast_Decl* d, string_buffer_Buf* out, uint32_t indent, bool print_type)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_color(out, ast_col_Decl);
   string_buffer_Buf_add(out, ast_declKind_names[ast_Decl_getKind(d)]);
   if (print_type) {
      string_buffer_Buf_add1(out, ' ');
      ast_QualType_printQuoted(&d->qt, out);
   }
}

static void ast_Decl_printName(const ast_Decl* d, string_buffer_Buf* out)
{
   string_buffer_Buf_add1(out, ' ');
   string_buffer_Buf_color(out, ast_col_Value);
   if (d->name_idx) {
      string_buffer_Buf_add(out, ast_Decl_getName(d));
   } else string_buffer_Buf_add(out, "(nil)");
}

static void ast_Decl_printBits(const ast_Decl* d, string_buffer_Buf* out)
{
   string_buffer_Buf_color(out, ast_col_Attr);
   if (ast_Decl_isPublic(d)) string_buffer_Buf_add(out, " public");
   ast_DeclCheckState cs = ast_Decl_getCheckState(d);
   if (cs != ast_DeclCheckState_Checked) {
      string_buffer_Buf_add1(out, ' ');
      string_buffer_Buf_add(out, ast_declCheckState_names[cs]);
   }
   if (!ast_Decl_isUsed(d)) {
      string_buffer_Buf_add(out, " unused");
   }
   if (d->declBits.has_attr) string_buffer_Buf_add(out, " attr");
   if (d->declBits.attr_export) string_buffer_Buf_add(out, " export");
}

static void ast_Decl_printAttrs(const ast_Decl* d, string_buffer_Buf* out)
{
   if (!ast_Decl_hasAttr(d)) return;

   const ast_AST* a = ast_Decl_getAST(d);
   string_buffer_Buf_color(out, ast_col_Expr);
   const attr_Value* cname = ast_AST_getAttr(a, d, attr_AttrKind_CName);
   if (cname) {
      string_buffer_Buf_print(out, " cname=%s", ast_idx2name(cname->text_idx));
   }
   const attr_Value* section = ast_AST_getAttr(a, d, attr_AttrKind_Section);
   if (section) {
      string_buffer_Buf_print(out, " section=%s", ast_idx2name(section->text_idx));
   }
}

static void ast_Decl_printUsed(const ast_Decl* d, string_buffer_Buf* out)
{
   string_buffer_Buf_color(out, ast_col_Attr);
   string_buffer_Buf_print(out, " used=%d/%d", ast_Decl_isUsed(d), ast_Decl_isUsedPublic(d));
}

static ast_FunctionDecl* ast_FunctionDecl_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, bool is_public, uint32_t ast_idx, const ast_TypeRefHolder* rtype, const ast_Ref* prefix, ast_VarDecl** params, uint32_t num_params, bool is_variadic)
{
   uint32_t size = sizeof(ast_FunctionDecl) + num_params * sizeof(ast_VarDecl*) + ast_TypeRefHolder_getExtraSize(rtype);
   if (prefix) size += sizeof(ast_Ref);
   ast_FunctionDecl* d = ast_context_Context_alloc(c, size);
   ast_FunctionType* ftype = ast_FunctionType_create(c, d);
   ast_QualType qt = ast_QualType_init(ast_FunctionType_asType(ftype));
   ast_Decl_init(&d->parent, ast_DeclKind_Function, name, loc, is_public, qt, ast_idx);
   d->parent.functionDeclBits.is_variadic = is_variadic;
   d->parent.functionDeclBits.call_kind = prefix ? ast_CallKind_StaticStructFunc : ast_CallKind_Normal;
   d->num_params = ((uint8_t)(num_params));
   d->instance_idx = 0;
   d->template_name = 0;
   d->template_loc = 0;
   d->rt = ast_QualType_Invalid;
   ast_TypeRefHolder_fill(rtype, &d->rtype);
   d->body = NULL;
   uint8_t* tail = ast_TypeRef_getPointerAfter(&d->rtype);
   if (prefix) {
      d->parent.functionDeclBits.has_prefix = 1;
      memcpy(tail, prefix, sizeof(ast_Ref));
      tail += sizeof(ast_Ref);
   }
   if (num_params) {
      memcpy(tail, ((void*)(params)), num_params * sizeof(ast_VarDecl*));
   }
   ast_Stats_addDecl(ast_DeclKind_Function, size);
   return d;
}

static ast_FunctionDecl* ast_FunctionDecl_createTemplate(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, bool is_public, uint32_t ast_idx, const ast_TypeRefHolder* rtype, uint32_t template_name, src_loc_SrcLoc template_loc, ast_VarDecl** params, uint32_t num_params, bool is_variadic)
{
   uint32_t size = sizeof(ast_FunctionDecl) + num_params * sizeof(ast_VarDecl*) + ast_TypeRefHolder_getExtraSize(rtype);
   ast_FunctionDecl* d = ast_context_Context_alloc(c, size);
   ast_FunctionType* ftype = ast_FunctionType_create(c, d);
   ast_QualType qt = ast_QualType_init(ast_FunctionType_asType(ftype));
   ast_Decl_init(&d->parent, ast_DeclKind_Function, name, loc, is_public, qt, ast_idx);
   d->parent.functionDeclBits.is_variadic = is_variadic;
   d->parent.functionDeclBits.call_kind = ast_CallKind_Normal;
   d->parent.functionDeclBits.is_template = 1;
   d->num_params = ((uint8_t)(num_params));
   d->instance_idx = 0;
   d->template_name = template_name;
   d->template_loc = template_loc;
   d->rt = ast_QualType_Invalid;
   ast_TypeRefHolder_fill(rtype, &d->rtype);
   d->body = NULL;
   uint8_t* tail = ast_TypeRef_getPointerAfter(&d->rtype);
   if (num_params) {
      memcpy(tail, ((void*)(params)), num_params * sizeof(ast_VarDecl*));
   }
   ast_Stats_addDecl(ast_DeclKind_Function, size);
   return d;
}

static ast_FunctionDecl* ast_FunctionDecl_instantiate(const ast_FunctionDecl* fd, ast_context_Context* c, const ast_TypeRef* ref)
{
   bool rtype_matches = ast_TypeRef_matchesTemplate(&fd->rtype, fd->template_name);
   uint32_t extra = rtype_matches ? ast_TypeRef_getExtraSize(ref) : ast_TypeRef_getExtraSize(&fd->rtype);
   uint32_t size = sizeof(ast_FunctionDecl) + fd->num_params * sizeof(ast_VarDecl*) + extra;
   ast_FunctionDecl* fd2 = ast_context_Context_alloc(c, size);
   memcpy(&fd2->parent, &fd->parent, sizeof(ast_Decl));
   fd2->parent.functionDeclBits.is_template = 0;
   ast_FunctionType* ftype = ast_FunctionType_create(c, fd2);
   fd2->parent.qt = ast_QualType_init(ast_FunctionType_asType(ftype));
   fd2->body = fd->body;
   ast_Instantiator inst = { c, ref, fd->template_name };
   fd2->body = ast_CompoundStmt_instantiate(fd->body, &inst);
   fd2->rt = ast_QualType_Invalid;
   fd2->num_params = fd->num_params;
   fd2->instance_idx = 0;
   fd2->template_name = 0;
   ast_TypeRef_instantiate(&fd2->rtype, &fd->rtype, ref, fd->template_name);
   ast_VarDecl** src = ((ast_VarDecl**)(ast_TypeRef_getPointerAfter(&fd->rtype)));
   ast_VarDecl** dst = ((ast_VarDecl**)(ast_TypeRef_getPointerAfter(&fd2->rtype)));
   for (uint32_t i = 0; i < fd2->num_params; i++) {
      dst[i] = ast_VarDecl_instantiate(src[i], &inst);
   }
   ast_Stats_addDecl(ast_DeclKind_Function, size);
   return fd2;
}

static void ast_FunctionDecl_setBody(ast_FunctionDecl* d, ast_CompoundStmt* body)
{
   d->body = body;
}

static ast_CompoundStmt* ast_FunctionDecl_getBody(const ast_FunctionDecl* d)
{
   return d->body;
}

static void ast_FunctionDecl_setRType(ast_FunctionDecl* d, ast_QualType rt)
{
   if (!ast_QualType_isVoidType(&rt)) d->parent.functionDeclBits.has_return = 1;
   d->rt = rt;
}

static ast_QualType ast_FunctionDecl_getRType(const ast_FunctionDecl* d)
{
   return d->rt;
}

static bool ast_FunctionDecl_hasReturn(const ast_FunctionDecl* d)
{
   return d->parent.functionDeclBits.has_return;
}

static ast_Decl* ast_FunctionDecl_asDecl(ast_FunctionDecl* d)
{
   return &d->parent;
}

static ast_TypeRef* ast_FunctionDecl_getTypeRef(ast_FunctionDecl* d)
{
   return &d->rtype;
}

static bool ast_FunctionDecl_hasPrefix(const ast_FunctionDecl* d)
{
   return d->parent.functionDeclBits.has_prefix;
}

static bool ast_FunctionDecl_isTemplate(const ast_FunctionDecl* d)
{
   return d->parent.functionDeclBits.is_template;
}

static uint32_t ast_FunctionDecl_getTemplateNameIdx(const ast_FunctionDecl* d)
{
   return d->template_name;
}

static src_loc_SrcLoc ast_FunctionDecl_getTemplateLoc(const ast_FunctionDecl* d)
{
   return d->template_loc;
}

static void ast_FunctionDecl_setTemplateInstanceIdx(ast_FunctionDecl* d, uint16_t idx)
{
   d->instance_idx = idx;
}

static uint16_t ast_FunctionDecl_getTemplateInstanceIdx(const ast_FunctionDecl* d)
{
   return d->instance_idx;
}

static void ast_FunctionDecl_setInstanceName(ast_FunctionDecl* d, uint32_t name_idx)
{
   d->parent.name_idx = name_idx;
}

static ast_Ref* ast_FunctionDecl_getPrefix(const ast_FunctionDecl* d)
{
   if (ast_FunctionDecl_hasPrefix(d)) return ast_TypeRef_getPointerAfter(&d->rtype);

   return NULL;
}

static const char* ast_FunctionDecl_getPrefixName(const ast_FunctionDecl* d)
{
   if (!ast_FunctionDecl_hasPrefix(d)) return NULL;

   const ast_Ref* ref = ast_TypeRef_getPointerAfter(&d->rtype);
   return ast_Ref_getName(ref);
}

static void ast_FunctionDecl_setCallKind(ast_FunctionDecl* d, ast_CallKind kind)
{
   d->parent.functionDeclBits.call_kind = kind;
}

static ast_CallKind ast_FunctionDecl_getCallKind(const ast_FunctionDecl* d)
{
   return ((ast_CallKind)(d->parent.functionDeclBits.call_kind));
}

static bool ast_FunctionDecl_isVariadic(const ast_FunctionDecl* d)
{
   return d->parent.functionDeclBits.is_variadic;
}

static uint32_t ast_FunctionDecl_getNumParams(const ast_FunctionDecl* d)
{
   return d->num_params;
}

static ast_VarDecl** ast_FunctionDecl_getParams(ast_FunctionDecl* d)
{
   uint8_t* tail = ast_TypeRef_getPointerAfter(&d->rtype);
   if (ast_FunctionDecl_hasPrefix(d)) tail += sizeof(ast_Ref);
   ast_VarDecl** params = ((ast_VarDecl**)(tail));
   return params;
}

static void ast_FunctionDecl_setAttrUnusedParams(ast_FunctionDecl* d)
{
   d->parent.functionDeclBits.attr_unused_params = 1;
}

static void ast_FunctionDecl_setAttrNoReturn(ast_FunctionDecl* d)
{
   d->parent.functionDeclBits.attr_noreturn = 1;
}

static void ast_FunctionDecl_setAttrInline(ast_FunctionDecl* d)
{
   d->parent.functionDeclBits.attr_inline = 1;
}

static void ast_FunctionDecl_setAttrWeak(ast_FunctionDecl* d)
{
   d->parent.functionDeclBits.attr_weak = 1;
}

static void ast_FunctionDecl_print(const ast_FunctionDecl* d, string_buffer_Buf* out, uint32_t indent)
{
   bool valid_type = ast_QualType_isValid(&d->parent.qt);
   ast_Decl_printKind(&d->parent, out, indent, valid_type);
   if (!valid_type) {
      string_buffer_Buf_add(out, " ");
      ast_TypeRef_print(&d->rtype, out, true);
   }
   ast_Decl_printBits(&d->parent, out);
   string_buffer_Buf_color(out, ast_col_Attr);
   if (d->parent.functionDeclBits.attr_noreturn) string_buffer_Buf_add(out, " noreturn");
   if (d->parent.functionDeclBits.attr_inline) string_buffer_Buf_add(out, " inline");
   if (d->parent.functionDeclBits.attr_weak) string_buffer_Buf_add(out, " weak");
   string_buffer_Buf_add1(out, ' ');
   string_buffer_Buf_add(out, ast_callKind_names[ast_FunctionDecl_getCallKind(d)]);
   ast_Decl_printAttrs(&d->parent, out);
   string_buffer_Buf_add1(out, ' ');
   string_buffer_Buf_color(out, ast_col_Value);
   const uint8_t* tail = ast_TypeRef_getPointerAfter(&d->rtype);
   if (ast_FunctionDecl_hasPrefix(d)) {
      const ast_Ref* prefix = ((ast_Ref*)(tail));
      string_buffer_Buf_add(out, ast_Ref_getName(prefix));
      string_buffer_Buf_add1(out, '.');
      tail += sizeof(ast_Ref);
   }
   string_buffer_Buf_add(out, ast_Decl_getName(&d->parent));
   string_buffer_Buf_add1(out, '\n');
   if (d->parent.functionDeclBits.is_template) {
      string_buffer_Buf_indent(out, indent + 1);
      string_buffer_Buf_color(out, ast_col_Template);
      string_buffer_Buf_print(out, "template %s\n", ast_idx2name(d->template_name));
   }
   ast_VarDecl** params = ((ast_VarDecl**)(tail));
   for (uint32_t i = 0; i < d->num_params; i++) {
      ast_VarDecl_print(params[i], out, indent + 1);
   }
   if (d->body) {
      ast_CompoundStmt_print(d->body, out, indent + 1);
   }
}

static void ast_FunctionDecl_printType(const ast_FunctionDecl* d, string_buffer_Buf* out)
{
   if (ast_QualType_isValid(&d->rt)) {
      ast_QualType_print(&d->rt, out);
   } else {
      ast_TypeRef_print(&d->rtype, out, true);
   }
   string_buffer_Buf_add(out, " (");
   const uint8_t* tail = ast_TypeRef_getPointerAfter(&d->rtype);
   if (ast_FunctionDecl_hasPrefix(d)) tail += sizeof(ast_Ref);
   ast_VarDecl** params = ((ast_VarDecl**)(tail));
   for (uint32_t i = 0; i < d->num_params; i++) {
      if (i != 0) string_buffer_Buf_add(out, ", ");
      ast_VarDecl_printType(params[i], out);
   }
   if (d->parent.functionDeclBits.is_variadic) string_buffer_Buf_add(out, ", ...");
   string_buffer_Buf_add1(out, ')');
}

static ast_ImportDecl* ast_ImportDecl_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, uint32_t alias_name, src_loc_SrcLoc alias_loc, uint32_t ast_idx, bool is_local)
{
   ast_ImportDecl* d = ast_context_Context_alloc(c, sizeof(ast_ImportDecl));
   ast_Decl_init(&d->parent, ast_DeclKind_Import, name, loc, false, ast_QualType_Invalid, ast_idx);
   d->parent.importDeclBits.is_local = is_local;
   d->alias_idx = alias_name;
   d->alias_loc = alias_loc;
   d->dest = NULL;
   ast_Stats_addDecl(ast_DeclKind_Import, sizeof(ast_ImportDecl));
   return d;
}

static ast_Decl* ast_ImportDecl_asDecl(ast_ImportDecl* d)
{
   return &d->parent;
}

static const char* ast_ImportDecl_getAliasName(const ast_ImportDecl* d)
{
   return ast_idx2name(d->alias_idx);
}

static uint32_t ast_ImportDecl_getImportNameIdx(const ast_ImportDecl* d)
{
   if (d->alias_idx) return d->alias_idx;

   return d->parent.name_idx;
}

static void ast_ImportDecl_setDest(ast_ImportDecl* d, ast_Module* mod)
{
   d->dest = mod;
}

static ast_Module* ast_ImportDecl_getDest(const ast_ImportDecl* d)
{
   return d->dest;
}

static bool ast_ImportDecl_isLocal(const ast_ImportDecl* d)
{
   return d->parent.importDeclBits.is_local;
}

static void ast_ImportDecl_print(const ast_ImportDecl* d, string_buffer_Buf* out, uint32_t indent)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_color(out, ast_col_Decl);
   string_buffer_Buf_add(out, "ImportDecl");
   ast_Decl_printUsed(&d->parent, out);
   string_buffer_Buf_add(out, " module=");
   if (d->dest) {
      string_buffer_Buf_add(out, ast_Module_getName(d->dest));
   } else {
      string_buffer_Buf_add(out, "<nil>");
   }
   if (d->parent.importDeclBits.is_local) {
      string_buffer_Buf_color(out, ast_col_Attr);
      string_buffer_Buf_add(out, " local");
   }
   ast_Decl_printName(&d->parent, out);
   if (d->alias_idx) {
      string_buffer_Buf_color(out, ast_col_Attr);
      string_buffer_Buf_add(out, " as ");
      string_buffer_Buf_color(out, ast_col_Value);
      string_buffer_Buf_print(out, "%s", ast_idx2name(d->alias_idx));
   }
   string_buffer_Buf_add1(out, '\n');
}

static ast_StructTypeDecl* ast_StructTypeDecl_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, bool is_public, uint32_t ast_idx, bool is_struct, bool is_global, ast_VarDecl** members, uint32_t num_members)
{
   uint32_t size = sizeof(ast_StructTypeDecl) + num_members * sizeof(ast_VarDecl*);
   ast_StructTypeDecl* d = ast_context_Context_alloc(c, size);
   ast_StructType* stype = ast_StructType_create(c, d);
   ast_QualType qt = ast_QualType_init(ast_StructType_asType(stype));
   ast_Type_setCanonicalType(ast_StructType_asType(stype), qt);
   ast_Decl_init(&d->parent, ast_DeclKind_StructType, name, loc, is_public, qt, ast_idx);
   d->parent.structTypeDeclBits.is_struct = is_struct;
   d->parent.structTypeDeclBits.is_global = is_global;
   if (!is_global) ast_Decl_setUsed(&d->parent);
   d->layout.size = 0;
   d->layout.alignment = 0;
   d->layout.attr_alignment = 1;
   d->layout.offset = 0;
   d->num_members = num_members;
   d->num_struct_functions = 0;
   d->struct_functions = NULL;
   if (num_members) {
      memcpy(((void*)(d->members)), ((void*)(members)), num_members * sizeof(ast_VarDecl*));
   }
   ast_Stats_addDecl(ast_DeclKind_StructType, size);
   return d;
}

static ast_Decl* ast_StructTypeDecl_asDecl(ast_StructTypeDecl* d)
{
   return &d->parent;
}

static uint32_t ast_StructTypeDecl_getNumMembers(const ast_StructTypeDecl* d)
{
   return d->num_members;
}

static ast_Decl** ast_StructTypeDecl_getMembers(ast_StructTypeDecl* d)
{
   return d->members;
}

static bool ast_StructTypeDecl_isStruct(const ast_StructTypeDecl* d)
{
   return d->parent.structTypeDeclBits.is_struct;
}

static bool ast_StructTypeDecl_isUnion(const ast_StructTypeDecl* d)
{
   return !d->parent.structTypeDeclBits.is_struct;
}

static uint32_t ast_StructTypeDecl_getOffset(const ast_StructTypeDecl* d)
{
   return d->layout.offset;
}

static void ast_StructTypeDecl_setOffset(ast_StructTypeDecl* d, uint32_t offset)
{
   d->layout.offset = offset;
}

static uint32_t ast_StructTypeDecl_getSize(const ast_StructTypeDecl* d)
{
   return d->layout.size;
}

static void ast_StructTypeDecl_setSize(ast_StructTypeDecl* d, uint32_t size)
{
   d->layout.size = size;
}

static uint32_t ast_StructTypeDecl_getAlignment(const ast_StructTypeDecl* d)
{
   return d->layout.alignment;
}

static void ast_StructTypeDecl_setAlignment(ast_StructTypeDecl* d, uint32_t alignment)
{
   d->layout.alignment = alignment;
}

static uint32_t ast_StructTypeDecl_getAttrAlignment(const ast_StructTypeDecl* d)
{
   return d->layout.attr_alignment;
}

static void ast_StructTypeDecl_setAttrAlignment(ast_StructTypeDecl* d, uint32_t alignment)
{
   d->layout.attr_alignment = alignment;
}

static void ast_StructTypeDecl_setPacked(ast_StructTypeDecl* d)
{
   d->parent.structTypeDeclBits.attr_packed = 1;
}

static bool ast_StructTypeDecl_isPacked(const ast_StructTypeDecl* d)
{
   return d->parent.structTypeDeclBits.attr_packed;
}

static void ast_StructTypeDecl_setOpaque(ast_StructTypeDecl* d)
{
   d->parent.structTypeDeclBits.attr_opaque = 1;
}

static bool ast_StructTypeDecl_isOpaque(const ast_StructTypeDecl* d)
{
   return d->parent.structTypeDeclBits.attr_opaque;
}

static bool ast_StructTypeDecl_isGlobal(const ast_StructTypeDecl* d)
{
   return d->parent.structTypeDeclBits.is_global;
}

static void ast_StructTypeDecl_setAttrNoTypeDef(ast_StructTypeDecl* d)
{
   d->parent.structTypeDeclBits.attr_notypedef = 1;
}

static bool ast_StructTypeDecl_hasAttrNoTypeDef(const ast_StructTypeDecl* d)
{
   return d->parent.structTypeDeclBits.attr_notypedef;
}

static void ast_StructTypeDecl_setStructFunctions(ast_StructTypeDecl* d, ast_context_Context* c, ast_FunctionDecl** funcs, uint32_t count)
{
   const uint32_t size = count * sizeof(ast_FunctionDecl*);
   void* dest = ast_context_Context_alloc(c, size);
   memcpy(dest, ((void*)(funcs)), size);
   d->struct_functions = dest;
   d->num_struct_functions = count;
}

static ast_Decl* ast_StructTypeDecl_findAny(const ast_StructTypeDecl* s, uint32_t name_idx)
{
   for (uint32_t i = 0; i < ast_StructTypeDecl_getNumMembers(s); i++) {
      ast_Decl* d = s->members[i];
      uint32_t member_name = ast_Decl_getNameIdx(d);
      if (member_name == name_idx) return d;

      if ((member_name == 0 && ast_Decl_getKind(d) == ast_DeclKind_StructType)) {
         ast_StructTypeDecl* sub = ((ast_StructTypeDecl*)(d));
         d = ast_StructTypeDecl_findAny(sub, name_idx);
         if (d) return d;

      }
   }
   if (s->parent.structTypeDeclBits.is_global) {
      for (uint32_t i = 0; i < s->num_struct_functions; i++) {
         ast_Decl* sf = ((ast_Decl*)(s->struct_functions[i]));
         if (ast_Decl_getNameIdx(sf) == name_idx) return sf;

      }
   }
   return NULL;
}

static ast_Decl* ast_StructTypeDecl_findMember(const ast_StructTypeDecl* s, uint32_t name_idx, uint32_t* offset)
{
   for (uint32_t i = 0; i < ast_StructTypeDecl_getNumMembers(s); i++) {
      ast_Decl* d = s->members[i];
      uint32_t member_name = ast_Decl_getNameIdx(d);
      if (member_name == name_idx) return d;

      if ((member_name == 0 && ast_Decl_getKind(d) == ast_DeclKind_StructType)) {
         ast_StructTypeDecl* sub = ((ast_StructTypeDecl*)(d));
         d = ast_StructTypeDecl_findMember(sub, name_idx, offset);
         if (d) {
            *offset += ast_StructTypeDecl_getOffset(sub);
            return d;
         }
      }
   }
   return NULL;
}

static void ast_StructTypeDecl_print(const ast_StructTypeDecl* d, string_buffer_Buf* out, uint32_t indent)
{
   ast_Decl_printKind(&d->parent, out, indent, true);
   ast_Decl_printBits(&d->parent, out);
   bool is_global = d->parent.structTypeDeclBits.is_global;
   if (is_global) string_buffer_Buf_add(out, " global");
   if (d->parent.structTypeDeclBits.is_struct) string_buffer_Buf_add(out, " struct");
   else string_buffer_Buf_add(out, " union");
   if (ast_StructTypeDecl_isPacked(d)) string_buffer_Buf_add(out, " packed");
   if (ast_StructTypeDecl_isOpaque(d)) string_buffer_Buf_add(out, " opaque");
   if (ast_StructTypeDecl_hasAttrNoTypeDef(d)) string_buffer_Buf_add(out, " notypedef");
   if (is_global) ast_Decl_printAttrs(&d->parent, out);
   string_buffer_Buf_color(out, ast_col_Calc);
   if (!is_global) string_buffer_Buf_print(out, " offset=%u", d->layout.offset);
   string_buffer_Buf_print(out, " size=%u align=%u", d->layout.size, d->layout.alignment);
   ast_Decl_printName(&d->parent, out);
   string_buffer_Buf_add1(out, '\n');
   for (uint32_t i = 0; i < d->num_members; i++) {
      ast_Decl_print(d->members[i], out, indent + 1);
   }
}

static ast_VarDecl* ast_VarDecl_create(ast_context_Context* c, ast_VarDeclKind kind, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* ref, uint32_t ast_idx, ast_Expr* initValue)
{
   uint32_t size = sizeof(ast_VarDecl) + ast_TypeRefHolder_getExtraSize(ref);
   if (initValue) size += sizeof(ast_Expr*);
   assert(kind != ast_VarDeclKind_StructMember);
   ast_VarDecl* d = ast_context_Context_alloc(c, size);
   ast_Decl_init(&d->parent, ast_DeclKind_Var, name, loc, is_public, ast_QualType_Invalid, ast_idx);
   d->parent.varDeclBits.kind = kind;
   ast_TypeRefHolder_fill(ref, &d->typeRef);
   if (initValue) {
      d->parent.varDeclBits.has_init_or_bitfield = 1;
      ast_Expr** i = ast_VarDecl_getInit2(d);
      *i = initValue;
   }
   ast_Stats_addDecl(ast_DeclKind_Var, size);
   return d;
}

static ast_VarDecl* ast_VarDecl_createStructMember(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* ref, uint32_t ast_idx, ast_Expr* bitfield)
{
   uint32_t size = sizeof(ast_VarDecl) + ast_TypeRefHolder_getExtraSize(ref);
   if (bitfield) size += sizeof(ast_Expr*);
   size += sizeof(uint32_t);
   size = ((size + 7) & ~0x7);
   ast_VarDecl* d = ast_context_Context_alloc(c, size);
   ast_Decl_init(&d->parent, ast_DeclKind_Var, name, loc, is_public, ast_QualType_Invalid, ast_idx);
   d->parent.varDeclBits.kind = ast_VarDeclKind_StructMember;
   if (name == 0) ast_Decl_setUsed(&d->parent);
   ast_TypeRefHolder_fill(ref, &d->typeRef);
   if (bitfield) {
      d->parent.varDeclBits.has_init_or_bitfield = 1;
      ast_Expr** i = ast_VarDecl_getInit2(d);
      *i = bitfield;
   }
   uint32_t* ptr = ast_VarDecl_getOffsetPtr(d);
   *ptr = 0;
   ast_Stats_addDecl(ast_DeclKind_Var, size);
   return d;
}

static ast_VarDecl* ast_VarDecl_instantiate(const ast_VarDecl* vd, ast_Instantiator* inst)
{
   bool matches = ast_TypeRef_matchesTemplate(&vd->typeRef, inst->template_name);
   uint32_t extra = matches ? ast_TypeRef_getExtraSize(inst->ref) : ast_TypeRef_getExtraSize(&vd->typeRef);
   uint32_t size = sizeof(ast_VarDecl) + extra;
   ast_VarDecl* vd2 = ast_context_Context_alloc(inst->c, size);
   vd2->parent = vd->parent;
   ast_TypeRef_instantiate(&vd2->typeRef, &vd->typeRef, inst->ref, inst->template_name);
   ast_Expr* ie = ast_VarDecl_getInit(vd);
   if (ie) {
      ast_Expr** ie2 = ast_VarDecl_getInit2(vd2);
      *ie2 = ast_Expr_instantiate(ie, inst);
   }
   ast_Stats_addDecl(ast_DeclKind_Var, size);
   return vd2;
}

static ast_Decl* ast_VarDecl_asDecl(ast_VarDecl* d)
{
   return &d->parent;
}

static ast_VarDeclKind ast_VarDecl_getKind(const ast_VarDecl* d)
{
   return ((ast_VarDeclKind)(d->parent.varDeclBits.kind));
}

static bool ast_VarDecl_isGlobal(const ast_VarDecl* d)
{
   return ast_VarDecl_getKind(d) == ast_VarDeclKind_GlobalVar;
}

static bool ast_VarDecl_isLocal(const ast_VarDecl* d)
{
   return ast_VarDecl_getKind(d) == ast_VarDeclKind_LocalVar;
}

static ast_TypeRef* ast_VarDecl_getTypeRef(ast_VarDecl* d)
{
   return &d->typeRef;
}

static bool ast_VarDecl_hasInit(const ast_VarDecl* d)
{
   return d->parent.varDeclBits.has_init_or_bitfield;
}

static ast_Expr* ast_VarDecl_getInit(const ast_VarDecl* d)
{
   if (d->parent.varDeclBits.has_init_or_bitfield) {
      ast_Expr** init_ = ast_TypeRef_getPointerAfter(&d->typeRef);
      return init_[0];
   }
   return NULL;
}

static ast_Expr** ast_VarDecl_getInit2(ast_VarDecl* d)
{
   if (d->parent.varDeclBits.has_init_or_bitfield) {
      return ast_TypeRef_getPointerAfter(&d->typeRef);
   }
   return NULL;
}

static ast_Expr* ast_VarDecl_getBitfield(const ast_VarDecl* d)
{
   if (ast_VarDecl_getKind(d) == ast_VarDeclKind_StructMember) return ast_VarDecl_getInit(d);

   return NULL;
}

static bool ast_VarDecl_hasLocalQualifier(const ast_VarDecl* d)
{
   return d->parent.varDeclBits.has_local;
}

static void ast_VarDecl_setLocal(ast_VarDecl* d, bool has_local)
{
   d->parent.varDeclBits.has_local = has_local;
}

static void ast_VarDecl_setAttrWeak(ast_VarDecl* d)
{
   d->parent.varDeclBits.attr_weak = 1;
}

static void ast_VarDecl_setOffset(ast_VarDecl* d, uint32_t offset)
{
   uint32_t* ptr = ast_VarDecl_getOffsetPtr(d);
   *ptr = offset;
}

static uint32_t ast_VarDecl_getOffset(const ast_VarDecl* d)
{
   return *ast_VarDecl_getOffsetPtr(d);
}

static uint32_t* ast_VarDecl_getOffsetPtr(const ast_VarDecl* d)
{
   uint8_t* tail = ast_TypeRef_getPointerAfter(&d->typeRef);
   if (d->parent.varDeclBits.has_init_or_bitfield) tail += sizeof(ast_Expr*);
   return ((uint32_t*)(tail));
}

static void ast_VarDecl_print(const ast_VarDecl* d, string_buffer_Buf* out, uint32_t indent)
{
   bool valid_type = ast_QualType_isValid(&d->parent.qt);
   ast_Decl_printKind(&d->parent, out, indent, valid_type);
   if (!valid_type) {
      string_buffer_Buf_add1(out, ' ');
      ast_TypeRef_print(&d->typeRef, out, true);
   }
   string_buffer_Buf_color(out, ast_col_Attr);
   ast_VarDeclKind k = ast_VarDecl_getKind(d);
   string_buffer_Buf_add(out, ast_varDeclNames[k]);
   bool has_init_or_bitfield = d->parent.varDeclBits.has_init_or_bitfield;
   if ((k == ast_VarDeclKind_StructMember && has_init_or_bitfield)) string_buffer_Buf_add(out, " bitfield");
   if (d->parent.varDeclBits.attr_weak) string_buffer_Buf_add(out, " weak");
   ast_Decl_printBits(&d->parent, out);
   ast_Decl_printAttrs(&d->parent, out);
   if (k == ast_VarDeclKind_StructMember) {
      string_buffer_Buf_color(out, ast_col_Calc);
      string_buffer_Buf_print(out, " offset=%u", *ast_VarDecl_getOffsetPtr(d));
   }
   string_buffer_Buf_color(out, ast_col_Value);
   ast_Decl_printName(&d->parent, out);
   string_buffer_Buf_add1(out, '\n');
   if (has_init_or_bitfield) {
      ast_Expr* i = ast_VarDecl_getInit(d);
      ast_Expr_print(i, out, indent + 1);
   }
}

static void ast_VarDecl_printType(const ast_VarDecl* d, string_buffer_Buf* out)
{
   if (ast_QualType_isValid(&d->parent.qt)) {
      ast_QualType_printQuoted(&d->parent.qt, out);
   } else {
      ast_TypeRef_print(&d->typeRef, out, true);
   }
}

static ast_EnumTypeDecl* ast_EnumTypeDecl_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, bool is_public, uint32_t ast_idx, ast_QualType implType, bool is_incr, ast_EnumConstantDecl** constants, uint32_t num_constants)
{
   uint32_t size = sizeof(ast_EnumTypeDecl) + num_constants * sizeof(ast_EnumConstantDecl*);
   ast_EnumTypeDecl* d = ast_context_Context_alloc(c, size);
   ast_EnumType* etype = ast_EnumType_create(c, d);
   ast_QualType qt = ast_QualType_init(((ast_Type*)(etype)));
   ast_Decl_init(&d->parent, ast_DeclKind_EnumType, name, loc, is_public, qt, ast_idx);
   d->parent.enumTypeDeclBits.is_incr = is_incr;
   d->parent.enumTypeDeclBits.num_constants = num_constants;
   d->implType = implType;
   if (is_incr) {
   } else {
      memcpy(((void*)(d->constants)), ((void*)(constants)), num_constants * sizeof(ast_EnumConstantDecl*));
      for (uint32_t i = 0; i < num_constants; i++) {
         ast_Decl_setType(ast_EnumConstantDecl_asDecl(constants[i]), qt);
      }
   }
   ast_Stats_addDecl(ast_DeclKind_EnumType, size);
   return d;
}

static void ast_EnumTypeDecl_setIncrMembers(ast_EnumTypeDecl* d, ast_Decl** constants, uint32_t num_constants)
{
   d->parent.enumTypeDeclBits.num_constants = num_constants;
   memcpy(((void*)(d->constants)), ((void*)(constants)), num_constants * sizeof(ast_EnumConstantDecl*));
}

static ast_QualType ast_EnumTypeDecl_getImplType(const ast_EnumTypeDecl* d)
{
   return d->implType;
}

static ast_Decl* ast_EnumTypeDecl_asDecl(ast_EnumTypeDecl* d)
{
   return &d->parent;
}

static uint32_t ast_EnumTypeDecl_getNumConstants(const ast_EnumTypeDecl* d)
{
   return d->parent.enumTypeDeclBits.num_constants;
}

static ast_EnumConstantDecl** ast_EnumTypeDecl_getConstants(ast_EnumTypeDecl* d)
{
   assert(ast_EnumTypeDecl_getNumConstants(d));
   return d->constants;
}

static ast_EnumConstantDecl* ast_EnumTypeDecl_findConstant(const ast_EnumTypeDecl* d, uint32_t name_idx)
{
   for (uint32_t i = 0; i < d->parent.enumTypeDeclBits.num_constants; i++) {
      ast_EnumConstantDecl* ecd = d->constants[i];
      ast_Decl* ed = ((ast_Decl*)(ecd));
      if (ast_Decl_getNameIdx(ed) == name_idx) return ecd;

   }
   return NULL;
}

static void ast_EnumTypeDecl_print(const ast_EnumTypeDecl* d, string_buffer_Buf* out, uint32_t indent)
{
   ast_Decl_printKind(&d->parent, out, indent, true);
   ast_Decl_printBits(&d->parent, out);
   if (d->parent.enumTypeDeclBits.is_incr) string_buffer_Buf_add(out, " incremental");
   ast_Decl_printAttrs(&d->parent, out);
   ast_Decl_printName(&d->parent, out);
   string_buffer_Buf_add1(out, ' ');
   ast_QualType_print(&d->implType, out);
   string_buffer_Buf_add1(out, '\n');
   for (uint32_t i = 0; i < d->parent.enumTypeDeclBits.num_constants; i++) {
      ast_EnumConstantDecl_print(d->constants[i], out, indent + 1);
   }
}

static ast_EnumConstantDecl* ast_EnumConstantDecl_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, bool is_public, uint32_t ast_idx, ast_Expr* initValue)
{
   uint32_t size = sizeof(ast_EnumConstantDecl);
   if (initValue) size += sizeof(ast_Expr*);
   ast_EnumConstantDecl* d = ast_context_Context_alloc(c, size);
   ast_Decl_init(&d->parent, ast_DeclKind_EnumConstant, name, loc, is_public, ast_QualType_Invalid, ast_idx);
   d->value = 0;
   if (initValue) {
      d->parent.enumConstantDeclBits.has_init = 1;
      d->init[0] = initValue;
   }
   ast_Stats_addDecl(ast_DeclKind_EnumConstant, size);
   return d;
}

static ast_Decl* ast_EnumConstantDecl_asDecl(ast_EnumConstantDecl* d)
{
   return &d->parent;
}

static uint32_t ast_EnumConstantDecl_getValue(const ast_EnumConstantDecl* d)
{
   return d->value;
}

static void ast_EnumConstantDecl_setValue(ast_EnumConstantDecl* d, uint32_t value)
{
   d->value = value;
}

static ast_Expr* ast_EnumConstantDecl_getInit(const ast_EnumConstantDecl* d)
{
   if (d->parent.enumConstantDeclBits.has_init) return d->init[0];

   return NULL;
}

static ast_Expr** ast_EnumConstantDecl_getInit2(ast_EnumConstantDecl* d)
{
   if (d->parent.enumConstantDeclBits.has_init) return &d->init[0];

   return NULL;
}

static void ast_EnumConstantDecl_print(const ast_EnumConstantDecl* d, string_buffer_Buf* out, uint32_t indent)
{
   ast_Decl_printKind(&d->parent, out, indent, true);
   ast_Decl_printBits(&d->parent, out);
   ast_Decl_printName(&d->parent, out);
   string_buffer_Buf_color(out, ast_col_Calc);
   string_buffer_Buf_print(out, " %u", d->value);
   string_buffer_Buf_add1(out, '\n');
   if (d->parent.enumConstantDeclBits.has_init) ast_Expr_print(d->init[0], out, indent + 1);
}

static ast_DeclStmt* ast_DeclStmt_create(ast_context_Context* c, ast_VarDecl* decl)
{
   ast_DeclStmt* s = ast_context_Context_alloc(c, sizeof(ast_DeclStmt));
   ast_Stmt_init(&s->parent, ast_StmtKind_Decl);
   s->decl = decl;
   ast_Stats_addStmt(ast_StmtKind_Decl, sizeof(ast_DeclStmt));
   return s;
}

static ast_Stmt* ast_DeclStmt_instantiate(ast_DeclStmt* s, ast_Instantiator* inst)
{
   ast_VarDecl* decl2 = ast_VarDecl_instantiate(s->decl, inst);
   return ((ast_Stmt*)(ast_DeclStmt_create(inst->c, decl2)));
}

static ast_VarDecl* ast_DeclStmt_getDecl(const ast_DeclStmt* d)
{
   return d->decl;
}

static void ast_DeclStmt_print(const ast_DeclStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   string_buffer_Buf_add1(out, '\n');
   ast_VarDecl_print(s->decl, out, indent + 1);
}

static ast_AliasTypeDecl* ast_AliasTypeDecl_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc, bool is_public, uint32_t ast_idx, const ast_TypeRefHolder* ref)
{
   uint32_t size = sizeof(ast_AliasTypeDecl) + ast_TypeRefHolder_getExtraSize(ref);
   ast_AliasTypeDecl* d = ast_context_Context_alloc(c, size);
   ast_AliasType* at = ast_AliasType_create(c, d);
   ast_Decl_init(&d->parent, ast_DeclKind_AliasType, name, loc, is_public, ast_QualType_init(((ast_Type*)(at))), ast_idx);
   ast_TypeRefHolder_fill(ref, &d->typeRef);
   ast_Stats_addDecl(ast_DeclKind_AliasType, size);
   return d;
}

static ast_Decl* ast_AliasTypeDecl_asDecl(ast_AliasTypeDecl* d)
{
   return &d->parent;
}

static ast_TypeRef* ast_AliasTypeDecl_getTypeRef(ast_AliasTypeDecl* d)
{
   return &d->typeRef;
}

static void ast_AliasTypeDecl_print(const ast_AliasTypeDecl* d, string_buffer_Buf* out, uint32_t indent)
{
   ast_Decl_printKind(&d->parent, out, indent, true);
   ast_Decl_printBits(&d->parent, out);
   ast_Decl_printAttrs(&d->parent, out);
   ast_Decl_printName(&d->parent, out);
   if (ast_QualType_isInvalid(&d->parent.qt)) {
      string_buffer_Buf_add1(out, ' ');
      ast_TypeRef_print(&d->typeRef, out, true);
      string_buffer_Buf_add1(out, '\n');
   }
}

static ast_StaticAssertDecl* ast_StaticAssertDecl_create(ast_context_Context* c, uint32_t ast_idx, src_loc_SrcLoc loc, ast_Expr* lhs, ast_Expr* rhs)
{
   ast_StaticAssertDecl* d = ast_context_Context_alloc(c, sizeof(ast_StaticAssertDecl));
   ast_Decl_init(&d->parent, ast_DeclKind_StaticAssert, 0, loc, false, ast_QualType_Invalid, ast_idx);
   d->lhs = lhs;
   d->rhs = rhs;
   ast_Stats_addDecl(ast_DeclKind_StaticAssert, sizeof(ast_StaticAssertDecl));
   return d;
}

static ast_Decl* ast_StaticAssertDecl_asDecl(ast_StaticAssertDecl* d)
{
   return &d->parent;
}

static ast_Expr* ast_StaticAssertDecl_getLhs(const ast_StaticAssertDecl* d)
{
   return d->lhs;
}

static ast_Expr* ast_StaticAssertDecl_getRhs(const ast_StaticAssertDecl* d)
{
   return d->rhs;
}

static void ast_StaticAssertDecl_print(const ast_StaticAssertDecl* d, string_buffer_Buf* out, uint32_t indent)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_print(out, "StaticAssert\n");
   ast_Expr_print(d->lhs, out, indent + 1);
   ast_Expr_print(d->rhs, out, indent + 1);
}

static ast_FunctionTypeDecl* ast_FunctionTypeDecl_create(ast_context_Context* c, ast_FunctionDecl* fn)
{
   ast_FunctionTypeDecl* ftd = ast_context_Context_alloc(c, sizeof(ast_FunctionTypeDecl));
   ast_Decl* d = ast_FunctionDecl_asDecl(fn);
   ast_Decl_init(&ftd->parent, ast_DeclKind_FunctionType, ast_Decl_getNameIdx(d), ast_Decl_getLoc(d), ast_Decl_isPublic(d), ast_Decl_getType(d), ast_Decl_getASTIdx(d));
   ftd->fn = fn;
   ast_Stats_addDecl(ast_DeclKind_FunctionType, sizeof(ast_FunctionTypeDecl));
   return ftd;
}

static ast_Decl* ast_FunctionTypeDecl_asDecl(ast_FunctionTypeDecl* t)
{
   return &t->parent;
}

static ast_FunctionDecl* ast_FunctionTypeDecl_getDecl(const ast_FunctionTypeDecl* d)
{
   return d->fn;
}

static void ast_FunctionTypeDecl_print(const ast_FunctionTypeDecl* d, string_buffer_Buf* out, uint32_t indent)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_color(out, ast_col_Decl);
   string_buffer_Buf_add(out, "FunctionTypeDecl");
   ast_Decl_printAttrs(&d->parent, out);
   string_buffer_Buf_add1(out, '\n');
   ast_FunctionDecl_print(d->fn, out, indent + 1);
}

static void ast_Stmt_init(ast_Stmt* s, ast_StmtKind k)
{
   s->bits = 0;
   s->stmtBits.kind = k;
}

static ast_Stmt* ast_Stmt_instantiate(ast_Stmt* s, ast_Instantiator* inst)
{
   switch (ast_Stmt_getKind(s)) {
   case ast_StmtKind_Return:
      return ast_ReturnStmt_instantiate(((ast_ReturnStmt*)(s)), inst);
   case ast_StmtKind_Expr:
      return ((ast_Stmt*)(ast_Expr_instantiate(((ast_Expr*)(s)), inst)));
   case ast_StmtKind_If:
      return ast_IfStmt_instantiate(((ast_IfStmt*)(s)), inst);
   case ast_StmtKind_While:
      return ast_WhileStmt_instantiate(((ast_WhileStmt*)(s)), inst);
   case ast_StmtKind_Do:
      return ast_DoStmt_instantiate(((ast_DoStmt*)(s)), inst);
   case ast_StmtKind_For:
      return ast_ForStmt_instantiate(((ast_ForStmt*)(s)), inst);
   case ast_StmtKind_Switch:
      return ast_SwitchStmt_instantiate(((ast_SwitchStmt*)(s)), inst);
   case ast_StmtKind_Case:
      return ast_CaseStmt_instantiate(((ast_CaseStmt*)(s)), inst);
   case ast_StmtKind_Default:
      return ast_DefaultStmt_instantiate(((ast_DefaultStmt*)(s)), inst);
   case ast_StmtKind_Break:
      return s;
   case ast_StmtKind_Continue:
      return s;
   case ast_StmtKind_Fallthrough:
      return s;
   case ast_StmtKind_Label:
      break;
   case ast_StmtKind_Goto:
      break;
   case ast_StmtKind_Compound:
      return ((ast_Stmt*)(ast_CompoundStmt_instantiate(((ast_CompoundStmt*)(s)), inst)));
   case ast_StmtKind_Decl:
      return ast_DeclStmt_instantiate(((ast_DeclStmt*)(s)), inst);
   case ast_StmtKind_Assert:
      return ast_AssertStmt_instantiate(((ast_AssertStmt*)(s)), inst);
   }
   ast_Stmt_dump(s);
   assert(0);
   return NULL;
}

static ast_StmtKind ast_Stmt_getKind(const ast_Stmt* s)
{
   return ((ast_StmtKind)(s->stmtBits.kind));
}

static void ast_Stmt_dump(const ast_Stmt* s)
{
   string_buffer_Buf* out = string_buffer_create(10 * 4096, utils_useColor(), 2);
   ast_Stmt_print(s, out, 0);
   string_buffer_Buf_color(out, ast_col_Normal);
   puts(string_buffer_Buf_data(out));
   string_buffer_Buf_free(out);
}

static void ast_Stmt_print(const ast_Stmt* s, string_buffer_Buf* out, uint32_t indent)
{
   switch (ast_Stmt_getKind(s)) {
   case ast_StmtKind_Return:
      ast_ReturnStmt_print(((ast_ReturnStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_Expr:
      ast_Expr_print(((ast_Expr*)(s)), out, indent);
      break;
   case ast_StmtKind_If:
      ast_IfStmt_print(((ast_IfStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_While:
      ast_WhileStmt_print(((ast_WhileStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_Do:
      ast_DoStmt_print(((ast_DoStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_For:
      ast_ForStmt_print(((ast_ForStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_Switch:
      ast_SwitchStmt_print(((ast_SwitchStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_Case:
      ast_CaseStmt_print(((ast_CaseStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_Default:
      ast_DefaultStmt_print(((ast_DefaultStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_Break:
      ast_BreakStmt_print(((ast_BreakStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_Continue:
      ast_ContinueStmt_print(((ast_ContinueStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_Fallthrough:
      ast_FallthroughStmt_print(((ast_FallthroughStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_Label:
      ast_LabelStmt_print(((ast_LabelStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_Goto:
      ast_GotoStmt_print(((ast_GotoStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_Compound:
      ast_CompoundStmt_print(((ast_CompoundStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_Decl:
      ast_DeclStmt_print(((ast_DeclStmt*)(s)), out, indent);
      break;
   case ast_StmtKind_Assert:
      ast_AssertStmt_print(((ast_AssertStmt*)(s)), out, indent);
      break;
   }
}

static void ast_Stmt_printKind(const ast_Stmt* s, string_buffer_Buf* out, uint32_t indent)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_color(out, ast_col_Stmt);
   string_buffer_Buf_add(out, ast_stmtKind_names[ast_Stmt_getKind(s)]);
}

static ast_AssertStmt* ast_AssertStmt_create(ast_context_Context* c, ast_Expr* inner)
{
   ast_AssertStmt* s = ast_context_Context_alloc(c, sizeof(ast_AssertStmt));
   ast_Stmt_init(&s->parent, ast_StmtKind_Assert);
   s->inner = inner;
   ast_Stats_addStmt(ast_StmtKind_Assert, sizeof(ast_AssertStmt));
   return s;
}

static ast_Stmt* ast_AssertStmt_instantiate(ast_AssertStmt* s, ast_Instantiator* inst)
{
   ast_AssertStmt* s2 = ast_AssertStmt_create(inst->c, ast_Expr_instantiate(s->inner, inst));
   return ((ast_Stmt*)(s2));
}

static void ast_AssertStmt_print(const ast_AssertStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   string_buffer_Buf_add1(out, '\n');
   ast_Expr_print(s->inner, out, indent + 1);
}

static ast_Expr* ast_AssertStmt_getInner(const ast_AssertStmt* s)
{
   return s->inner;
}

static ast_Expr** ast_AssertStmt_getInner2(ast_AssertStmt* s)
{
   return &s->inner;
}

static ast_CompoundStmt* ast_CompoundStmt_create(ast_context_Context* c, src_loc_SrcLoc end, ast_Stmt** stmts, uint32_t count)
{
   assert(count < 65556);
   uint32_t size = sizeof(ast_CompoundStmt) + count * sizeof(ast_Stmt*);
   ast_CompoundStmt* s = ast_context_Context_alloc(c, size);
   ast_Stmt_init(&s->parent, ast_StmtKind_Compound);
   s->parent.compoundStmtBits.count = count;
   s->endLoc = end;
   if (count) {
      memcpy(((void*)(s->stmts)), ((void*)(stmts)), count * sizeof(ast_Stmt*));
   }
   ast_Stats_addStmt(ast_StmtKind_Compound, size);
   return s;
}

static ast_CompoundStmt* ast_CompoundStmt_instantiate(const ast_CompoundStmt* s, ast_Instantiator* inst)
{
   const uint32_t count = s->parent.compoundStmtBits.count;
   uint32_t size = sizeof(ast_CompoundStmt) + count * sizeof(ast_Stmt*);
   ast_CompoundStmt* s2 = ast_context_Context_alloc(inst->c, size);
   s2->parent = s->parent;
   s2->endLoc = s->endLoc;
   for (uint32_t i = 0; i < count; i++) {
      s2->stmts[i] = ast_Stmt_instantiate(s->stmts[i], inst);
   }
   return s2;
}

static src_loc_SrcLoc ast_CompoundStmt_getEndLoc(const ast_CompoundStmt* s)
{
   return s->endLoc;
}

static uint32_t ast_CompoundStmt_getCount(const ast_CompoundStmt* s)
{
   return s->parent.compoundStmtBits.count;
}

static ast_Stmt** ast_CompoundStmt_getStmts(ast_CompoundStmt* s)
{
   if (ast_CompoundStmt_getCount(s)) return s->stmts;

   return NULL;
}

static ast_Stmt* ast_CompoundStmt_getLastStmt(const ast_CompoundStmt* s)
{
   uint32_t count = ast_CompoundStmt_getCount(s);
   if (count) return s->stmts[count - 1];

   return NULL;
}

static void ast_CompoundStmt_print(const ast_CompoundStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   string_buffer_Buf_add1(out, '\n');
   const uint32_t count = s->parent.compoundStmtBits.count;
   for (uint32_t i = 0; i < count; i++) {
      ast_Stmt_print(s->stmts[i], out, indent + 1);
   }
}

static ast_DoStmt* ast_DoStmt_create(ast_context_Context* c, ast_Stmt* cond, ast_Stmt* body)
{
   ast_DoStmt* s = ast_context_Context_alloc(c, sizeof(ast_DoStmt));
   ast_Stmt_init(&s->parent, ast_StmtKind_Do);
   s->cond = cond;
   s->body = body;
   ast_Stats_addStmt(ast_StmtKind_Do, sizeof(ast_DoStmt));
   return s;
}

static ast_Stmt* ast_DoStmt_instantiate(ast_DoStmt* s, ast_Instantiator* inst)
{
   ast_Stmt* cond2 = ast_Stmt_instantiate(s->cond, inst);
   ast_Stmt* body2 = ast_Stmt_instantiate(s->body, inst);
   return ((ast_Stmt*)(ast_DoStmt_create(inst->c, cond2, body2)));
}

static void ast_DoStmt_print(const ast_DoStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   string_buffer_Buf_add1(out, '\n');
   ast_Stmt_print(s->cond, out, indent + 1);
   ast_Stmt_print(s->body, out, indent + 1);
}

static ast_Stmt* ast_DoStmt_getCond(const ast_DoStmt* s)
{
   return s->cond;
}

static ast_Stmt* ast_DoStmt_getBody(const ast_DoStmt* s)
{
   return s->body;
}

static ast_ForStmt* ast_ForStmt_create(ast_context_Context* c, ast_Stmt* init_, ast_Expr* cond, ast_Expr* incr, ast_Stmt* body)
{
   ast_ForStmt* s = ast_context_Context_alloc(c, sizeof(ast_ForStmt));
   ast_Stmt_init(&s->parent, ast_StmtKind_For);
   s->init = init_;
   s->cond = cond;
   s->incr = incr;
   s->body = body;
   ast_Stats_addStmt(ast_StmtKind_For, sizeof(ast_ForStmt));
   return s;
}

static ast_Stmt* ast_ForStmt_instantiate(ast_ForStmt* s, ast_Instantiator* inst)
{
   ast_Stmt* init2 = s->init ? ast_Stmt_instantiate(s->init, inst) : NULL;
   ast_Expr* cond2 = s->cond ? ast_Expr_instantiate(s->cond, inst) : NULL;
   ast_Expr* incr2 = s->incr ? ast_Expr_instantiate(s->incr, inst) : NULL;
   ast_Stmt* body2 = s->body ? ast_Stmt_instantiate(s->body, inst) : NULL;
   return ((ast_Stmt*)(ast_ForStmt_create(inst->c, init2, cond2, incr2, body2)));
}

static ast_Stmt* ast_ForStmt_getInit(const ast_ForStmt* s)
{
   return s->init;
}

static ast_Expr* ast_ForStmt_getCond(const ast_ForStmt* s)
{
   return s->cond;
}

static ast_Expr* ast_ForStmt_getIncr(const ast_ForStmt* s)
{
   return s->incr;
}

static ast_Stmt* ast_ForStmt_getBody(const ast_ForStmt* s)
{
   return s->body;
}

static ast_Stmt** ast_ForStmt_getInit2(ast_ForStmt* s)
{
   return s->init ? &s->init : NULL;
}

static ast_Expr** ast_ForStmt_getCond2(ast_ForStmt* s)
{
   return s->cond ? &s->cond : NULL;
}

static ast_Expr** ast_ForStmt_getIncr2(ast_ForStmt* s)
{
   return s->incr ? &s->incr : NULL;
}

static ast_Stmt** ast_ForStmt_getBody2(ast_ForStmt* s)
{
   return s->body ? &s->body : NULL;
}

static void ast_ForStmt_print(const ast_ForStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   string_buffer_Buf_add1(out, '\n');
   if (s->init) ast_Stmt_print(s->init, out, indent + 1);
   if (s->cond) ast_Expr_print(s->cond, out, indent + 1);
   if (s->incr) ast_Expr_print(s->incr, out, indent + 1);
   if (s->body) ast_Stmt_print(s->body, out, indent + 1);
}

static ast_IfStmt* ast_IfStmt_create(ast_context_Context* c, ast_Stmt* cond, ast_Stmt* then, ast_Stmt* else_stmt)
{
   uint32_t size = sizeof(ast_IfStmt);
   if (else_stmt) size += sizeof(ast_Stmt*);
   ast_IfStmt* s = ast_context_Context_alloc(c, size);
   ast_Stmt_init(&s->parent, ast_StmtKind_If);
   s->cond = cond;
   s->then = then;
   if (else_stmt) {
      s->parent.ifStmtBits.has_else = 1;
      s->else_stmt[0] = else_stmt;
   }
   ast_Stats_addStmt(ast_StmtKind_If, size);
   return s;
}

static ast_Stmt* ast_IfStmt_instantiate(ast_IfStmt* s, ast_Instantiator* inst)
{
   ast_Stmt* cond2 = ast_Stmt_instantiate(s->cond, inst);
   ast_Stmt* then2 = ast_Stmt_instantiate(s->then, inst);
   ast_Stmt* else2 = NULL;
   if (s->parent.ifStmtBits.has_else) else2 = ast_Stmt_instantiate(s->else_stmt[0], inst);
   return ((ast_Stmt*)(ast_IfStmt_create(inst->c, cond2, then2, else2)));
}

static ast_Stmt* ast_IfStmt_getCond(const ast_IfStmt* s)
{
   return s->cond;
}

static ast_Stmt** ast_IfStmt_getCond2(ast_IfStmt* s)
{
   return &s->cond;
}

static ast_Stmt* ast_IfStmt_getThen(const ast_IfStmt* s)
{
   return s->then;
}

static ast_Stmt* ast_IfStmt_getElse(const ast_IfStmt* s)
{
   if (s->parent.ifStmtBits.has_else) return s->else_stmt[0];

   return NULL;
}

static void ast_IfStmt_print(const ast_IfStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   string_buffer_Buf_add1(out, '\n');
   ast_Stmt_print(s->cond, out, indent + 1);
   if (s->then) ast_Stmt_print(s->then, out, indent + 1);
   if (s->parent.ifStmtBits.has_else) ast_Stmt_print(s->else_stmt[0], out, indent + 1);
}

static ast_ReturnStmt* ast_ReturnStmt_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* value)
{
   uint32_t size = sizeof(ast_ReturnStmt);
   if (value) size += sizeof(ast_Expr*);
   ast_ReturnStmt* s = ast_context_Context_alloc(c, size);
   ast_Stmt_init(&s->parent, ast_StmtKind_Return);
   s->loc = loc;
   if (value) {
      s->parent.returnStmtBits.has_value = 1;
      s->value[0] = value;
   }
   ast_Stats_addStmt(ast_StmtKind_Return, size);
   return s;
}

static ast_Stmt* ast_ReturnStmt_instantiate(ast_ReturnStmt* s, ast_Instantiator* inst)
{
   if (!s->parent.returnStmtBits.has_value) return ((ast_Stmt*)(s));

   return ((ast_Stmt*)(ast_ReturnStmt_create(inst->c, s->loc, ast_Expr_instantiate(s->value[0], inst))));
}

static src_loc_SrcLoc ast_ReturnStmt_getLoc(const ast_ReturnStmt* s)
{
   return s->loc;
}

static ast_Expr* ast_ReturnStmt_getValue(const ast_ReturnStmt* s)
{
   if (s->parent.returnStmtBits.has_value) return s->value[0];

   return NULL;
}

static ast_Expr** ast_ReturnStmt_getValue2(ast_ReturnStmt* s)
{
   if (s->parent.returnStmtBits.has_value) return &s->value[0];

   return NULL;
}

static void ast_ReturnStmt_print(const ast_ReturnStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   string_buffer_Buf_add1(out, '\n');
   if (s->parent.returnStmtBits.has_value) {
      ast_Expr_print(s->value[0], out, indent + 1);
   }
}

static ast_SwitchStmt* ast_SwitchStmt_create(ast_context_Context* c, ast_Stmt* cond, ast_Stmt** cases, uint32_t numCases, bool is_sswitch)
{
   uint32_t size = sizeof(ast_SwitchStmt) + numCases * sizeof(ast_Stmt*);
   ast_SwitchStmt* s = ast_context_Context_alloc(c, size);
   ast_Stmt_init(&s->parent, ast_StmtKind_Switch);
   s->parent.switchStmtBits.is_sswitch = is_sswitch;
   s->parent.switchStmtBits.num_cases = numCases;
   s->cond = cond;
   memcpy(((void*)(s->cases)), ((void*)(cases)), numCases * sizeof(ast_Stmt*));
   ast_Stats_addStmt(ast_StmtKind_Switch, size);
   return s;
}

static ast_Stmt* ast_SwitchStmt_instantiate(ast_SwitchStmt* s, ast_Instantiator* inst)
{
   uint32_t numCases = ast_SwitchStmt_getNumCases(s);
   uint32_t size = sizeof(ast_SwitchStmt) + numCases * sizeof(ast_Stmt*);
   ast_SwitchStmt* s2 = ast_context_Context_alloc(inst->c, size);
   s2->parent = s->parent;
   s2->cond = ast_Stmt_instantiate(s->cond, inst);
   for (uint32_t i = 0; i < numCases; i++) {
      s2->cases[i] = ast_Stmt_instantiate(s->cases[i], inst);
   }
   ast_Stats_addStmt(ast_StmtKind_Switch, size);
   return ((ast_Stmt*)(s2));
}

static ast_Stmt* ast_SwitchStmt_getCond(const ast_SwitchStmt* s)
{
   return s->cond;
}

static ast_Stmt** ast_SwitchStmt_getCond2(ast_SwitchStmt* s)
{
   return s->cond ? &s->cond : NULL;
}

static bool ast_SwitchStmt_isSSwitch(const ast_SwitchStmt* s)
{
   return s->parent.switchStmtBits.is_sswitch;
}

static uint32_t ast_SwitchStmt_getNumCases(const ast_SwitchStmt* s)
{
   return s->parent.switchStmtBits.num_cases;
}

static ast_Stmt** ast_SwitchStmt_getCases(ast_SwitchStmt* s)
{
   return s->cases;
}

static void ast_SwitchStmt_print(const ast_SwitchStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   if (ast_SwitchStmt_isSSwitch(s)) {
      string_buffer_Buf_color(out, ast_col_Attr);
      string_buffer_Buf_add(out, " string");
   }
   string_buffer_Buf_add1(out, '\n');
   ast_Stmt_print(s->cond, out, indent + 1);
   for (uint32_t i = 0; i < s->parent.switchStmtBits.num_cases; i++) {
      ast_Stmt_print(s->cases[i], out, indent + 1);
   }
}

static ast_CaseStmt* ast_CaseStmt_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* cond, ast_Stmt** stmts, uint32_t numStmts)
{
   uint32_t size = sizeof(ast_CaseStmt) + numStmts * sizeof(ast_Stmt*);
   ast_CaseStmt* s = ast_context_Context_alloc(c, size);
   ast_Stmt_init(&s->parent, ast_StmtKind_Case);
   s->parent.caseStmtBits.num_stmts = numStmts;
   s->loc = loc;
   s->cond = cond;
   memcpy(((void*)(s->stmts)), ((void*)(stmts)), numStmts * sizeof(ast_Stmt*));
   ast_Stats_addStmt(ast_StmtKind_Case, size);
   return s;
}

static ast_Stmt* ast_CaseStmt_instantiate(ast_CaseStmt* s, ast_Instantiator* inst)
{
   uint32_t numStmts = ast_CaseStmt_getNumStmts(s);
   uint32_t size = sizeof(ast_CaseStmt) + numStmts * sizeof(ast_Stmt*);
   ast_CaseStmt* s2 = ast_context_Context_alloc(inst->c, size);
   s2->parent = s->parent;
   s2->loc = s->loc;
   s2->cond = ast_Expr_instantiate(s->cond, inst);
   for (uint32_t i = 0; i < numStmts; i++) {
      s2->stmts[i] = ast_Stmt_instantiate(s->stmts[i], inst);
   }
   ast_Stats_addStmt(ast_StmtKind_Case, size);
   return ((ast_Stmt*)(s));
}

static uint32_t ast_CaseStmt_getNumStmts(const ast_CaseStmt* s)
{
   return s->parent.caseStmtBits.num_stmts;
}

static ast_Stmt** ast_CaseStmt_getStmts(ast_CaseStmt* s)
{
   return s->stmts;
}

static src_loc_SrcLoc ast_CaseStmt_getLoc(const ast_CaseStmt* s)
{
   return s->loc;
}

static ast_Expr* ast_CaseStmt_getCond(const ast_CaseStmt* s)
{
   return s->cond;
}

static ast_Expr** ast_CaseStmt_getCond2(ast_CaseStmt* s)
{
   return &s->cond;
}

static void ast_CaseStmt_setHasDecls(ast_CaseStmt* s)
{
   s->parent.caseStmtBits.has_decls = 1;
}

static bool ast_CaseStmt_hasDecls(const ast_CaseStmt* s)
{
   return s->parent.caseStmtBits.has_decls;
}

static void ast_CaseStmt_print(const ast_CaseStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   if (s->parent.caseStmtBits.has_decls) {
      string_buffer_Buf_color(out, ast_col_Attr);
      string_buffer_Buf_add(out, " decls");
   }
   string_buffer_Buf_add1(out, '\n');
   ast_Expr_print(s->cond, out, indent + 1);
   for (uint32_t i = 0; i < s->parent.caseStmtBits.num_stmts; i++) {
      ast_Stmt_print(s->stmts[i], out, indent + 1);
   }
}

static ast_DefaultStmt* ast_DefaultStmt_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Stmt** stmts, uint32_t numStmts)
{
   uint32_t size = sizeof(ast_DefaultStmt) + numStmts * sizeof(ast_Stmt*);
   ast_DefaultStmt* s = ast_context_Context_alloc(c, size);
   ast_Stmt_init(&s->parent, ast_StmtKind_Default);
   s->parent.defaultStmtBits.num_stmts = numStmts;
   s->loc = loc;
   memcpy(((void*)(s->stmts)), ((void*)(stmts)), numStmts * sizeof(ast_Stmt*));
   ast_Stats_addStmt(ast_StmtKind_Default, size);
   return s;
}

static ast_Stmt* ast_DefaultStmt_instantiate(ast_DefaultStmt* s, ast_Instantiator* inst)
{
   uint32_t numStmts = ast_DefaultStmt_getNumStmts(s);
   uint32_t size = sizeof(ast_DefaultStmt) + numStmts * sizeof(ast_Stmt*);
   ast_DefaultStmt* s2 = ast_context_Context_alloc(inst->c, size);
   s2->parent = s->parent;
   s2->loc = s->loc;
   for (uint32_t i = 0; i < numStmts; i++) {
   }
   ast_Stats_addStmt(ast_StmtKind_Default, size);
   return ((ast_Stmt*)(s2));
}

static src_loc_SrcLoc ast_DefaultStmt_getLoc(const ast_DefaultStmt* s)
{
   return s->loc;
}

static uint32_t ast_DefaultStmt_getNumStmts(const ast_DefaultStmt* s)
{
   return s->parent.defaultStmtBits.num_stmts;
}

static ast_Stmt** ast_DefaultStmt_getStmts(ast_DefaultStmt* s)
{
   return s->stmts;
}

static void ast_DefaultStmt_setHasDecls(ast_DefaultStmt* s)
{
   s->parent.defaultStmtBits.has_decls = 1;
}

static bool ast_DefaultStmt_hasDecls(const ast_DefaultStmt* s)
{
   return s->parent.defaultStmtBits.has_decls;
}

static void ast_DefaultStmt_print(const ast_DefaultStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   if (s->parent.defaultStmtBits.has_decls) {
      string_buffer_Buf_color(out, ast_col_Attr);
      string_buffer_Buf_add(out, " decls");
   }
   string_buffer_Buf_add1(out, '\n');
   for (uint32_t i = 0; i < s->parent.defaultStmtBits.num_stmts; i++) {
      ast_Stmt_print(s->stmts[i], out, indent + 1);
   }
}

static ast_BreakStmt* ast_BreakStmt_create(ast_context_Context* c, src_loc_SrcLoc loc)
{
   ast_BreakStmt* s = ast_context_Context_alloc(c, sizeof(ast_BreakStmt));
   ast_Stmt_init(&s->parent, ast_StmtKind_Break);
   s->loc = loc;
   ast_Stats_addStmt(ast_StmtKind_Break, sizeof(ast_BreakStmt));
   return s;
}

static src_loc_SrcLoc ast_BreakStmt_getLoc(const ast_BreakStmt* s)
{
   return s->loc;
}

static void ast_BreakStmt_print(const ast_BreakStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   string_buffer_Buf_add1(out, '\n');
}

static ast_ContinueStmt* ast_ContinueStmt_create(ast_context_Context* c, src_loc_SrcLoc loc)
{
   ast_ContinueStmt* s = ast_context_Context_alloc(c, sizeof(ast_ContinueStmt));
   ast_Stmt_init(&s->parent, ast_StmtKind_Continue);
   s->loc = loc;
   ast_Stats_addStmt(ast_StmtKind_Continue, sizeof(ast_ContinueStmt));
   return s;
}

static src_loc_SrcLoc ast_ContinueStmt_getLoc(const ast_ContinueStmt* s)
{
   return s->loc;
}

static void ast_ContinueStmt_print(const ast_ContinueStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   string_buffer_Buf_add1(out, '\n');
}

static ast_FallthroughStmt* ast_FallthroughStmt_create(ast_context_Context* c, src_loc_SrcLoc loc)
{
   ast_FallthroughStmt* s = ast_context_Context_alloc(c, sizeof(ast_FallthroughStmt));
   ast_Stmt_init(&s->parent, ast_StmtKind_Fallthrough);
   s->loc = loc;
   ast_Stats_addStmt(ast_StmtKind_Fallthrough, sizeof(ast_FallthroughStmt));
   return s;
}

static src_loc_SrcLoc ast_FallthroughStmt_getLoc(const ast_FallthroughStmt* s)
{
   return s->loc;
}

static void ast_FallthroughStmt_print(const ast_FallthroughStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   string_buffer_Buf_add1(out, '\n');
}

static ast_WhileStmt* ast_WhileStmt_create(ast_context_Context* c, ast_Stmt* cond, ast_Stmt* body)
{
   ast_WhileStmt* s = ast_context_Context_alloc(c, sizeof(ast_WhileStmt));
   ast_Stmt_init(&s->parent, ast_StmtKind_While);
   s->cond = cond;
   s->body = body;
   ast_Stats_addStmt(ast_StmtKind_While, sizeof(ast_WhileStmt));
   return s;
}

static ast_Stmt* ast_WhileStmt_instantiate(ast_WhileStmt* s, ast_Instantiator* inst)
{
   ast_Stmt* cond2 = ast_Stmt_instantiate(s->cond, inst);
   ast_Stmt* body2 = ast_Stmt_instantiate(s->body, inst);
   return ((ast_Stmt*)(ast_WhileStmt_create(inst->c, cond2, body2)));
}

static void ast_WhileStmt_print(const ast_WhileStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   string_buffer_Buf_add1(out, '\n');
   ast_Stmt_print(s->cond, out, indent + 1);
   ast_Stmt_print(s->body, out, indent + 1);
}

static ast_Stmt* ast_WhileStmt_getCond(const ast_WhileStmt* s)
{
   return s->cond;
}

static ast_Stmt** ast_WhileStmt_getCond2(ast_WhileStmt* s)
{
   return &s->cond;
}

static ast_Stmt* ast_WhileStmt_getBody(const ast_WhileStmt* s)
{
   return s->body;
}

static ast_LabelStmt* ast_LabelStmt_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc)
{
   ast_LabelStmt* s = ast_context_Context_alloc(c, sizeof(ast_LabelStmt));
   ast_Stmt_init(&s->parent, ast_StmtKind_Label);
   s->loc = loc;
   s->name = name;
   ast_Stats_addStmt(ast_StmtKind_Label, sizeof(ast_LabelStmt));
   return s;
}

static const char* ast_LabelStmt_getName(const ast_LabelStmt* s)
{
   return ast_idx2name(s->name);
}

static uint32_t ast_LabelStmt_getNameIdx(const ast_LabelStmt* s)
{
   return s->name;
}

static src_loc_SrcLoc ast_LabelStmt_getLoc(ast_LabelStmt* s)
{
   return s->loc;
}

static void ast_LabelStmt_print(const ast_LabelStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   string_buffer_Buf_color(out, ast_col_Value);
   string_buffer_Buf_print(out, " %s\n", ast_idx2name(s->name));
}

static ast_GotoStmt* ast_GotoStmt_create(ast_context_Context* c, uint32_t name, src_loc_SrcLoc loc)
{
   ast_GotoStmt* s = ast_context_Context_alloc(c, sizeof(ast_GotoStmt));
   ast_Stmt_init(&s->parent, ast_StmtKind_Goto);
   s->loc = loc;
   s->name = name;
   ast_Stats_addStmt(ast_StmtKind_Goto, sizeof(ast_GotoStmt));
   return s;
}

static const char* ast_GotoStmt_getName(const ast_GotoStmt* g)
{
   return ast_idx2name(g->name);
}

static uint32_t ast_GotoStmt_getNameIdx(const ast_GotoStmt* g)
{
   return g->name;
}

static src_loc_SrcLoc ast_GotoStmt_getLoc(ast_GotoStmt* g)
{
   return g->loc;
}

static void ast_GotoStmt_print(const ast_GotoStmt* s, string_buffer_Buf* out, uint32_t indent)
{
   ast_Stmt_printKind(&s->parent, out, indent);
   string_buffer_Buf_color(out, ast_col_Value);
   string_buffer_Buf_print(out, " %s\n", ast_idx2name(s->name));
}

static void ast_Expr_init(ast_Expr* e, ast_ExprKind k, src_loc_SrcLoc loc, bool ctv, bool ctc, bool has_effect, ast_ValType valtype)
{
   ast_Stmt_init(&e->parent, ast_StmtKind_Expr);
   e->parent.exprBits.kind = k;
   e->parent.exprBits.is_ctv = ctv;
   e->parent.exprBits.is_ctc = ctc;
   e->parent.exprBits.has_effect = has_effect;
   e->parent.exprBits.valtype = valtype;
   e->loc = loc;
   e->qt.ptr = 0;
}

static ast_Expr* ast_Expr_instantiate(ast_Expr* e, ast_Instantiator* inst)
{
   switch (ast_Expr_getKind(e)) {
   case ast_ExprKind_IntegerLiteral:
      return e;
   case ast_ExprKind_BooleanLiteral:
      return e;
   case ast_ExprKind_CharLiteral:
      return e;
   case ast_ExprKind_StringLiteral:
      return e;
   case ast_ExprKind_Nil:
      return e;
   case ast_ExprKind_Identifier:
      return ast_IdentifierExpr_instantiate(((ast_IdentifierExpr*)(e)), inst);
   case ast_ExprKind_Type:
      return ast_TypeExpr_instantiate(((ast_TypeExpr*)(e)), inst);
   case ast_ExprKind_Call:
      return ast_CallExpr_instantiate(((ast_CallExpr*)(e)), inst);
   case ast_ExprKind_InitList:
      return ast_InitListExpr_instantiate(((ast_InitListExpr*)(e)), inst);
   case ast_ExprKind_FieldDesignatedInit:
      return ast_FieldDesignatedInitExpr_instantiate(((ast_FieldDesignatedInitExpr*)(e)), inst);
   case ast_ExprKind_ArrayDesignatedInit:
      return ast_ArrayDesignatedInitExpr_instantiate(((ast_ArrayDesignatedInitExpr*)(e)), inst);
   case ast_ExprKind_BinaryOperator:
      return ast_BinaryOperator_instantiate(((ast_BinaryOperator*)(e)), inst);
   case ast_ExprKind_UnaryOperator:
      return ast_UnaryOperator_instantiate(((ast_UnaryOperator*)(e)), inst);
   case ast_ExprKind_ConditionalOperator:
      return ast_ConditionalOperator_instantiate(((ast_ConditionalOperator*)(e)), inst);
   case ast_ExprKind_Builtin:
      return ast_BuiltinExpr_instantiate(((ast_BuiltinExpr*)(e)), inst);
   case ast_ExprKind_ArraySubscript:
      return ast_ArraySubscriptExpr_instantiate(((ast_ArraySubscriptExpr*)(e)), inst);
   case ast_ExprKind_Member:
      return ast_MemberExpr_instantiate(((ast_MemberExpr*)(e)), inst);
   case ast_ExprKind_Paren:
      return ast_ParenExpr_instantiate(((ast_ParenExpr*)(e)), inst);
   case ast_ExprKind_BitOffset:
      return ast_BitOffsetExpr_instantiate(((ast_BitOffsetExpr*)(e)), inst);
   case ast_ExprKind_ExplicitCast:
      return ast_ExplicitCastExpr_instantiate(((ast_ExplicitCastExpr*)(e)), inst);
   case ast_ExprKind_ImplicitCast:
      break;
   }
   ast_Expr_dump(e);
   assert(0);
   return NULL;
}

static ast_Stmt* ast_Expr_asStmt(ast_Expr* e)
{
   return &e->parent;
}

static ast_ExprKind ast_Expr_getKind(const ast_Expr* e)
{
   return ((ast_ExprKind)(e->parent.exprBits.kind));
}

static bool ast_Expr_isCtv(const ast_Expr* e)
{
   return e->parent.exprBits.is_ctv;
}

static bool ast_Expr_isCtc(const ast_Expr* e)
{
   return e->parent.exprBits.is_ctc;
}

static void ast_Expr_setCtv(ast_Expr* e)
{
   e->parent.exprBits.is_ctv = true;
}

static void ast_Expr_setCtc(ast_Expr* e)
{
   e->parent.exprBits.is_ctc = true;
}

static void ast_Expr_copyConstantFlags(ast_Expr* e, const ast_Expr* other)
{
   e->parent.exprBits.is_ctc = other->parent.exprBits.is_ctc;
   e->parent.exprBits.is_ctv = other->parent.exprBits.is_ctv;
}

static void ast_Expr_combineConstantFlags(ast_Expr* e, const ast_Expr* lhs, const ast_Expr* rhs)
{
   e->parent.exprBits.is_ctc = (lhs->parent.exprBits.is_ctc && rhs->parent.exprBits.is_ctc);
   e->parent.exprBits.is_ctv = (lhs->parent.exprBits.is_ctv && rhs->parent.exprBits.is_ctv);
}

static bool ast_Expr_hasEffect(const ast_Expr* e)
{
   return e->parent.exprBits.has_effect;
}

static ast_ValType ast_Expr_getValType(const ast_Expr* e)
{
   return ((ast_ValType)(e->parent.exprBits.valtype));
}

static bool ast_Expr_isNValue(const ast_Expr* e)
{
   return ast_Expr_getValType(e) == ast_ValType_NValue;
}

static bool ast_Expr_isRValue(const ast_Expr* e)
{
   return ast_Expr_getValType(e) == ast_ValType_RValue;
}

static bool ast_Expr_isLValue(const ast_Expr* e)
{
   return ast_Expr_getValType(e) == ast_ValType_LValue;
}

static void ast_Expr_setLValue(ast_Expr* e)
{
   e->parent.exprBits.valtype = ast_ValType_LValue;
}

static void ast_Expr_setRValue(ast_Expr* e)
{
   e->parent.exprBits.valtype = ast_ValType_RValue;
}

static void ast_Expr_setValType(ast_Expr* e, ast_ValType valtype)
{
   e->parent.exprBits.valtype = valtype;
}

static void ast_Expr_copyValType(ast_Expr* e, const ast_Expr* other)
{
   e->parent.exprBits.valtype = other->parent.exprBits.valtype;
}

static src_loc_SrcLoc ast_Expr_getLoc(const ast_Expr* e)
{
   return e->loc;
}

static src_loc_SrcLoc ast_Expr_getStartLoc(const ast_Expr* e)
{
   switch (ast_Expr_getKind(e)) {
   case ast_ExprKind_IntegerLiteral:
      // fallthrough
   case ast_ExprKind_BooleanLiteral:
      // fallthrough
   case ast_ExprKind_CharLiteral:
      // fallthrough
   case ast_ExprKind_StringLiteral:
      // fallthrough
   case ast_ExprKind_Nil:
      // fallthrough
   case ast_ExprKind_Identifier:
      break;
   case ast_ExprKind_Type:
      break;
   case ast_ExprKind_Call:
      break;
   case ast_ExprKind_InitList:
      break;
   case ast_ExprKind_FieldDesignatedInit:
      break;
   case ast_ExprKind_ArrayDesignatedInit:
      break;
   case ast_ExprKind_BinaryOperator: {
      const ast_BinaryOperator* b = ((ast_BinaryOperator*)(e));
      return ast_Expr_getStartLoc(ast_BinaryOperator_getLHS(b));
   }
   case ast_ExprKind_UnaryOperator: {
      const ast_UnaryOperator* u = ((ast_UnaryOperator*)(e));
      return ast_UnaryOperator_getStartLoc(u);
   }
   case ast_ExprKind_ConditionalOperator: {
      const ast_ConditionalOperator* c = ((ast_ConditionalOperator*)(e));
      return ast_Expr_getStartLoc(ast_ConditionalOperator_getCond(c));
   }
   case ast_ExprKind_Builtin:
      break;
   case ast_ExprKind_ArraySubscript: {
      const ast_ArraySubscriptExpr* a = ((ast_ArraySubscriptExpr*)(e));
      return ast_Expr_getStartLoc(ast_ArraySubscriptExpr_getBase(a));
   }
   case ast_ExprKind_Member: {
      const ast_MemberExpr* m = ((ast_MemberExpr*)(e));
      return ast_MemberExpr_getStartLoc(m);
   }
   case ast_ExprKind_Paren:
      break;
   case ast_ExprKind_BitOffset:
      break;
   case ast_ExprKind_ExplicitCast:
      break;
   case ast_ExprKind_ImplicitCast: {
      const ast_ImplicitCastExpr* c = ((ast_ImplicitCastExpr*)(e));
      return ast_Expr_getStartLoc(ast_ImplicitCastExpr_getInner(c));
   }
   }
   return e->loc;
}

static src_loc_SrcLoc ast_Expr_getEndLoc(const ast_Expr* e)
{
   switch (ast_Expr_getKind(e)) {
   case ast_ExprKind_IntegerLiteral:
      // fallthrough
   case ast_ExprKind_BooleanLiteral:
      // fallthrough
   case ast_ExprKind_CharLiteral:
      // fallthrough
   case ast_ExprKind_StringLiteral:
      // fallthrough
   case ast_ExprKind_Nil:
      // fallthrough
   case ast_ExprKind_Identifier:
      break;
   case ast_ExprKind_Type:
      break;
   case ast_ExprKind_Call:
      break;
   case ast_ExprKind_InitList:
      break;
   case ast_ExprKind_FieldDesignatedInit:
      break;
   case ast_ExprKind_ArrayDesignatedInit:
      break;
   case ast_ExprKind_BinaryOperator: {
      const ast_BinaryOperator* b = ((ast_BinaryOperator*)(e));
      return ast_Expr_getEndLoc(ast_BinaryOperator_getRHS(b));
   }
   case ast_ExprKind_UnaryOperator: {
      const ast_UnaryOperator* u = ((ast_UnaryOperator*)(e));
      return ast_UnaryOperator_getEndLoc(u);
   }
   case ast_ExprKind_ConditionalOperator: {
      const ast_ConditionalOperator* c = ((ast_ConditionalOperator*)(e));
      return ast_Expr_getEndLoc(ast_ConditionalOperator_getRHS(c));
   }
   case ast_ExprKind_Builtin: {
      const ast_BuiltinExpr* bi = ((ast_BuiltinExpr*)(e));
      return ast_BuiltinExpr_getEndLoc(bi);
   }
   case ast_ExprKind_ArraySubscript:
      break;
   case ast_ExprKind_Member: {
      const ast_MemberExpr* m = ((ast_MemberExpr*)(e));
      return ast_MemberExpr_getEndLoc(m);
   }
   case ast_ExprKind_Paren: {
      const ast_ParenExpr* p = ((ast_ParenExpr*)(e));
      return ast_Expr_getEndLoc(ast_ParenExpr_getInner(p)) + 1;
   }
   case ast_ExprKind_BitOffset:
      break;
   case ast_ExprKind_ExplicitCast: {
      const ast_ExplicitCastExpr* c = ((ast_ExplicitCastExpr*)(e));
      return ast_Expr_getEndLoc(ast_ExplicitCastExpr_getInner(c)) + 1;
   }
   case ast_ExprKind_ImplicitCast: {
      const ast_ImplicitCastExpr* c = ((ast_ImplicitCastExpr*)(e));
      return ast_Expr_getEndLoc(ast_ImplicitCastExpr_getInner(c));
   }
   }
   return e->loc;
}

static src_loc_SrcRange ast_Expr_getRange(const ast_Expr* e)
{
   src_loc_SrcRange range = { ast_Expr_getStartLoc(e), ast_Expr_getEndLoc(e) };
   return range;
}

static void ast_Expr_setType(ast_Expr* e, ast_QualType qt_)
{
   e->qt = qt_;
}

static ast_QualType ast_Expr_getType(const ast_Expr* e)
{
   return e->qt;
}

static void ast_Expr_dump(const ast_Expr* e)
{
   string_buffer_Buf* out = string_buffer_create(10 * 4096, utils_useColor(), 2);
   ast_Expr_print(e, out, 0);
   string_buffer_Buf_color(out, ast_col_Normal);
   puts(string_buffer_Buf_data(out));
   string_buffer_Buf_free(out);
}

static void ast_Expr_print(const ast_Expr* e, string_buffer_Buf* out, uint32_t indent)
{
   switch (ast_Expr_getKind(e)) {
   case ast_ExprKind_IntegerLiteral:
      ast_IntegerLiteral_print(((ast_IntegerLiteral*)(e)), out, indent);
      break;
   case ast_ExprKind_BooleanLiteral:
      ast_BooleanLiteral_print(((ast_BooleanLiteral*)(e)), out, indent);
      break;
   case ast_ExprKind_CharLiteral:
      ast_CharLiteral_print(((ast_CharLiteral*)(e)), out, indent);
      break;
   case ast_ExprKind_StringLiteral:
      ast_StringLiteral_print(((ast_StringLiteral*)(e)), out, indent);
      break;
   case ast_ExprKind_Nil:
      ast_NilExpr_print(((ast_NilExpr*)(e)), out, indent);
      break;
   case ast_ExprKind_Identifier:
      ast_IdentifierExpr_print(((ast_IdentifierExpr*)(e)), out, indent);
      break;
   case ast_ExprKind_Type:
      ast_TypeExpr_print(((ast_TypeExpr*)(e)), out, indent);
      break;
   case ast_ExprKind_Call:
      ast_CallExpr_print(((ast_CallExpr*)(e)), out, indent);
      break;
   case ast_ExprKind_InitList:
      ast_InitListExpr_print(((ast_InitListExpr*)(e)), out, indent);
      break;
   case ast_ExprKind_FieldDesignatedInit:
      ast_FieldDesignatedInitExpr_print(((ast_FieldDesignatedInitExpr*)(e)), out, indent);
      break;
   case ast_ExprKind_ArrayDesignatedInit:
      ast_ArrayDesignatedInitExpr_print(((ast_ArrayDesignatedInitExpr*)(e)), out, indent);
      break;
   case ast_ExprKind_BinaryOperator:
      ast_BinaryOperator_print(((ast_BinaryOperator*)(e)), out, indent);
      break;
   case ast_ExprKind_UnaryOperator:
      ast_UnaryOperator_print(((ast_UnaryOperator*)(e)), out, indent);
      break;
   case ast_ExprKind_ConditionalOperator:
      ast_ConditionalOperator_print(((ast_ConditionalOperator*)(e)), out, indent);
      break;
   case ast_ExprKind_Builtin:
      ast_BuiltinExpr_print(((ast_BuiltinExpr*)(e)), out, indent);
      break;
   case ast_ExprKind_ArraySubscript:
      ast_ArraySubscriptExpr_print(((ast_ArraySubscriptExpr*)(e)), out, indent);
      break;
   case ast_ExprKind_Member:
      ast_MemberExpr_print(((ast_MemberExpr*)(e)), out, indent);
      break;
   case ast_ExprKind_Paren:
      ast_ParenExpr_print(((ast_ParenExpr*)(e)), out, indent);
      break;
   case ast_ExprKind_BitOffset:
      ast_BitOffsetExpr_print(((ast_BitOffsetExpr*)(e)), out, indent);
      break;
   case ast_ExprKind_ExplicitCast:
      ast_ExplicitCastExpr_print(((ast_ExplicitCastExpr*)(e)), out, indent);
      break;
   case ast_ExprKind_ImplicitCast:
      ast_ImplicitCastExpr_print(((ast_ImplicitCastExpr*)(e)), out, indent);
      break;
   }
}

static void ast_Expr_printLiteral(const ast_Expr* e, string_buffer_Buf* out)
{
   switch (ast_Expr_getKind(e)) {
   case ast_ExprKind_IntegerLiteral:
      ast_IntegerLiteral_printLiteral(((ast_IntegerLiteral*)(e)), out);
      return;
   case ast_ExprKind_BooleanLiteral:
      ast_BooleanLiteral_printLiteral(((ast_BooleanLiteral*)(e)), out);
      return;
   case ast_ExprKind_CharLiteral:
      ast_CharLiteral_printLiteral(((ast_CharLiteral*)(e)), out);
      return;
   case ast_ExprKind_StringLiteral:
      break;
   case ast_ExprKind_Nil:
      ast_NilExpr_printLiteral(((ast_NilExpr*)(e)), out);
      return;
   case ast_ExprKind_Identifier:
      ast_IdentifierExpr_printLiteral(((ast_IdentifierExpr*)(e)), out);
      return;
   case ast_ExprKind_Type:
      break;
   case ast_ExprKind_Call:
      break;
   case ast_ExprKind_InitList:
      break;
   case ast_ExprKind_FieldDesignatedInit:
      break;
   case ast_ExprKind_ArrayDesignatedInit:
      break;
   case ast_ExprKind_BinaryOperator:
      ast_BinaryOperator_printLiteral(((ast_BinaryOperator*)(e)), out);
      return;
   case ast_ExprKind_UnaryOperator:
      ast_UnaryOperator_printLiteral(((ast_UnaryOperator*)(e)), out);
      return;
   case ast_ExprKind_ConditionalOperator:
      break;
   case ast_ExprKind_Builtin:
      ast_BuiltinExpr_printLiteral(((ast_BuiltinExpr*)(e)), out);
      return;
   case ast_ExprKind_ArraySubscript:
      break;
   case ast_ExprKind_Member:
      ast_MemberExpr_printLiteral(((ast_MemberExpr*)(e)), out);
      return;
   case ast_ExprKind_Paren:
      ast_ParenExpr_printLiteral(((ast_ParenExpr*)(e)), out);
      return;
   case ast_ExprKind_BitOffset:
      break;
   case ast_ExprKind_ExplicitCast:
      break;
   case ast_ExprKind_ImplicitCast:
      break;
   }
   string_buffer_Buf_print(out, "?? kind=%u", ast_Expr_getKind(e));
}

static void ast_Expr_printKind(const ast_Expr* e, string_buffer_Buf* out, uint32_t indent)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_color(out, ast_col_Expr);
   string_buffer_Buf_add(out, ast_exprKind_names[ast_Expr_getKind(e)]);
}

static void ast_Expr_printTypeBits(const ast_Expr* e, string_buffer_Buf* out)
{
   string_buffer_Buf_add1(out, ' ');
   ast_QualType_printQuoted(&e->qt, out);
   string_buffer_Buf_color(out, ast_col_Attr);
   if (e->parent.exprBits.is_ctc) string_buffer_Buf_add(out, " CTC");
   if (e->parent.exprBits.is_ctv) string_buffer_Buf_add(out, " CTV");
   string_buffer_Buf_add1(out, ' ');
   string_buffer_Buf_add(out, ast_valType_names[ast_Expr_getValType(e)]);
}

static ast_BuiltinExpr* ast_BuiltinExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* inner, ast_BuiltinExprKind kind)
{
   const uint32_t size = sizeof(ast_BuiltinExpr);
   ast_BuiltinExpr* e = ast_context_Context_alloc(c, size);
   ast_Expr_init(&e->parent, ast_ExprKind_Builtin, loc, 1, 1, 0, ast_ValType_RValue);
   e->parent.parent.builtinExprBits.kind = kind;
   e->inner = inner;
   e->value = 0;
   ast_Stats_addExpr(ast_ExprKind_Builtin, size);
   return e;
}

static ast_BuiltinExpr* ast_BuiltinExpr_createOffsetOf(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* typeExpr, ast_Expr* member)
{
   const uint32_t size = sizeof(ast_BuiltinExpr) + sizeof(ast_OffsetOfData);
   ast_BuiltinExpr* e = ast_context_Context_alloc(c, size);
   ast_Expr_init(&e->parent, ast_ExprKind_Builtin, loc, 1, 1, 0, ast_ValType_RValue);
   e->parent.parent.builtinExprBits.kind = ast_BuiltinExprKind_OffsetOf;
   e->inner = typeExpr;
   e->offset[0].member = member;
   ast_Stats_addExpr(ast_ExprKind_Builtin, size);
   return e;
}

static ast_BuiltinExpr* ast_BuiltinExpr_createToContainer(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* typeExpr, ast_Expr* member, ast_Expr* pointer)
{
   const uint32_t size = sizeof(ast_BuiltinExpr) + sizeof(ast_ToContainerData);
   ast_BuiltinExpr* e = ast_context_Context_alloc(c, size);
   ast_Expr_init(&e->parent, ast_ExprKind_Builtin, loc, 1, 1, 0, ast_ValType_RValue);
   e->parent.parent.builtinExprBits.kind = ast_BuiltinExprKind_ToContainer;
   e->inner = typeExpr;
   e->container[0].member = member;
   e->container[0].pointer = pointer;
   ast_Stats_addExpr(ast_ExprKind_Builtin, size);
   return e;
}

static ast_Expr* ast_BuiltinExpr_instantiate(ast_BuiltinExpr* e, ast_Instantiator* inst)
{
   ast_BuiltinExpr* bi = NULL;
   switch (ast_BuiltinExpr_getKind(e)) {
   case ast_BuiltinExprKind_Sizeof:
      // fallthrough
   case ast_BuiltinExprKind_Elemsof:
      // fallthrough
   case ast_BuiltinExprKind_EnumMin:
      // fallthrough
   case ast_BuiltinExprKind_EnumMax:
      bi = ast_BuiltinExpr_create(inst->c, e->parent.loc, ast_Expr_instantiate(e->inner, inst), ast_BuiltinExpr_getKind(e));
      break;
   case ast_BuiltinExprKind_OffsetOf:
      bi = ast_BuiltinExpr_createOffsetOf(inst->c, e->parent.loc, ast_Expr_instantiate(e->inner, inst), ast_Expr_instantiate(e->offset[0].member, inst));
      break;
   case ast_BuiltinExprKind_ToContainer:
      bi = ast_BuiltinExpr_createToContainer(inst->c, e->parent.loc, ast_Expr_instantiate(e->inner, inst), ast_Expr_instantiate(e->container[0].member, inst), ast_Expr_instantiate(e->container[0].pointer, inst));
      break;
   }
   return ((ast_Expr*)(bi));
}

static ast_BuiltinExprKind ast_BuiltinExpr_getKind(const ast_BuiltinExpr* e)
{
   return ((ast_BuiltinExprKind)(e->parent.parent.builtinExprBits.kind));
}

static size_t ast_BuiltinExpr_getValue(const ast_BuiltinExpr* e)
{
   return e->value;
}

static void ast_BuiltinExpr_setValue(ast_BuiltinExpr* e, size_t value)
{
   e->value = value;
}

static ast_Expr* ast_BuiltinExpr_getInner(const ast_BuiltinExpr* e)
{
   return e->inner;
}

static src_loc_SrcLoc ast_BuiltinExpr_getEndLoc(const ast_BuiltinExpr* e)
{
   switch (ast_BuiltinExpr_getKind(e)) {
   case ast_BuiltinExprKind_Sizeof:
      // fallthrough
   case ast_BuiltinExprKind_Elemsof:
      // fallthrough
   case ast_BuiltinExprKind_EnumMin:
      // fallthrough
   case ast_BuiltinExprKind_EnumMax:
      break;
   case ast_BuiltinExprKind_OffsetOf:
      return ast_Expr_getEndLoc(e->offset[0].member) + 1;
   case ast_BuiltinExprKind_ToContainer:
      return ast_Expr_getEndLoc(e->container[0].pointer) + 1;
   }
   return ast_Expr_getEndLoc(e->inner);
}

static ast_Expr* ast_BuiltinExpr_getOffsetOfMember(const ast_BuiltinExpr* b)
{
   assert(ast_BuiltinExpr_getKind(b) == ast_BuiltinExprKind_OffsetOf);
   return b->offset[0].member;
}

static ast_Expr* ast_BuiltinExpr_getToContainerMember(const ast_BuiltinExpr* b)
{
   assert(ast_BuiltinExpr_getKind(b) == ast_BuiltinExprKind_ToContainer);
   return b->container[0].member;
}

static ast_Expr* ast_BuiltinExpr_getToContainerPointer(const ast_BuiltinExpr* b)
{
   assert(ast_BuiltinExpr_getKind(b) == ast_BuiltinExprKind_ToContainer);
   return b->container[0].pointer;
}

static ast_Expr** ast_BuiltinExpr_getToContainerPointer2(ast_BuiltinExpr* b)
{
   assert(ast_BuiltinExpr_getKind(b) == ast_BuiltinExprKind_ToContainer);
   return &b->container[0].pointer;
}

static void ast_BuiltinExpr_print(const ast_BuiltinExpr* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_color(out, ast_col_Attr);
   string_buffer_Buf_print(out, " %s", ast_builtin_names[ast_BuiltinExpr_getKind(e)]);
   string_buffer_Buf_color(out, ast_col_Calc);
   string_buffer_Buf_print(out, " %u", e->value);
   string_buffer_Buf_add1(out, '\n');
   ast_Expr_print(e->inner, out, indent + 1);
   switch (ast_BuiltinExpr_getKind(e)) {
   case ast_BuiltinExprKind_Sizeof:
      break;
   case ast_BuiltinExprKind_Elemsof:
      break;
   case ast_BuiltinExprKind_EnumMin:
      break;
   case ast_BuiltinExprKind_EnumMax:
      break;
   case ast_BuiltinExprKind_OffsetOf:
      ast_Expr_print(e->offset[0].member, out, indent + 1);
      break;
   case ast_BuiltinExprKind_ToContainer:
      ast_Expr_print(e->container[0].member, out, indent + 1);
      ast_Expr_print(e->container[0].pointer, out, indent + 1);
      break;
   }
}

static void ast_BuiltinExpr_printLiteral(const ast_BuiltinExpr* e, string_buffer_Buf* out)
{
   string_buffer_Buf_add(out, ast_builtin_names[ast_BuiltinExpr_getKind(e)]);
   string_buffer_Buf_add1(out, '(');
   string_buffer_Buf_add1(out, ')');
   switch (ast_BuiltinExpr_getKind(e)) {
   case ast_BuiltinExprKind_Sizeof:
      break;
   case ast_BuiltinExprKind_Elemsof:
      break;
   case ast_BuiltinExprKind_EnumMin:
      break;
   case ast_BuiltinExprKind_EnumMax:
      break;
   case ast_BuiltinExprKind_OffsetOf:
      break;
   case ast_BuiltinExprKind_ToContainer:
      break;
   }
}

static ast_BooleanLiteral* ast_BooleanLiteral_create(ast_context_Context* c, src_loc_SrcLoc loc, bool val)
{
   ast_BooleanLiteral* e = ast_context_Context_alloc(c, sizeof(ast_BooleanLiteral));
   ast_Expr_init(&e->parent, ast_ExprKind_BooleanLiteral, loc, 1, 1, 0, ast_ValType_RValue);
   e->parent.parent.booleanLiteralBits.value = val;
   ast_Expr_setType(&e->parent, ast_g_bool);
   ast_Stats_addExpr(ast_ExprKind_BooleanLiteral, sizeof(ast_BooleanLiteral));
   return e;
}

static bool ast_BooleanLiteral_getValue(const ast_BooleanLiteral* e)
{
   return e->parent.parent.booleanLiteralBits.value;
}

static void ast_BooleanLiteral_print(const ast_BooleanLiteral* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, ' ');
   string_buffer_Buf_color(out, ast_col_Value);
   string_buffer_Buf_add(out, e->parent.parent.booleanLiteralBits.value ? "true" : "false");
   string_buffer_Buf_add1(out, '\n');
}

static void ast_BooleanLiteral_printLiteral(const ast_BooleanLiteral* e, string_buffer_Buf* out)
{
   string_buffer_Buf_add(out, e->parent.parent.booleanLiteralBits.value ? "true" : "false");
}

static ast_CharLiteral* ast_CharLiteral_create(ast_context_Context* c, src_loc_SrcLoc loc, uint8_t val)
{
   ast_CharLiteral* e = ast_context_Context_alloc(c, sizeof(ast_CharLiteral));
   ast_Expr_init(&e->parent, ast_ExprKind_CharLiteral, loc, 1, 1, 0, ast_ValType_RValue);
   e->parent.parent.charLiteralBits.value = val;
   ast_Stats_addExpr(ast_ExprKind_CharLiteral, sizeof(ast_CharLiteral));
   ast_Expr_setType(&e->parent, ast_g_char);
   return e;
}

static uint8_t ast_CharLiteral_getValue(const ast_CharLiteral* e)
{
   return ((uint8_t)(e->parent.parent.charLiteralBits.value));
}

static void ast_CharLiteral_print(const ast_CharLiteral* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, ' ');
   string_buffer_Buf_color(out, ast_col_Value);
   ast_CharLiteral_printLiteral(e, out);
   string_buffer_Buf_add1(out, '\n');
}

static void ast_CharLiteral_printLiteral(const ast_CharLiteral* e, string_buffer_Buf* out)
{
   char c = ((char)(e->parent.parent.charLiteralBits.value));
   switch (c) {
   case '\a':
      string_buffer_Buf_add(out, "'\\a'");
      break;
   case '\b':
      string_buffer_Buf_add(out, "'\\b'");
      break;
   case '\f':
      string_buffer_Buf_add(out, "'\\f'");
      break;
   case '\n':
      string_buffer_Buf_add(out, "'\\n'");
      break;
   case '\r':
      string_buffer_Buf_add(out, "'\\r'");
      break;
   case '\t':
      string_buffer_Buf_add(out, "'\\t'");
      break;
   case '\v':
      string_buffer_Buf_add(out, "'\\v'");
      break;
   case '\'':
      string_buffer_Buf_add(out, "'\\''");
      break;
   case '\\':
      string_buffer_Buf_add(out, "'\\\\'");
      break;
   default:
      string_buffer_Buf_print(out, "'%c'", c);
      break;
   }
}

static ast_StringLiteral* ast_StringLiteral_create(ast_context_Context* c, src_loc_SrcLoc loc, uint32_t value, uint32_t len)
{
   ast_StringLiteral* e = ast_context_Context_alloc(c, sizeof(ast_StringLiteral));
   ast_Expr_init(&e->parent, ast_ExprKind_StringLiteral, loc, 1, 1, 0, ast_ValType_LValue);
   e->value = value;
   ast_Stats_addExpr(ast_ExprKind_StringLiteral, sizeof(ast_StringLiteral));
   ast_Expr_setType(&e->parent, ast_StringTypePool_get(&ast_g_string_types, len));
   return e;
}

static const char* ast_StringLiteral_getText(const ast_StringLiteral* e)
{
   return ast_idx2name(e->value);
}

static void ast_StringLiteral_printLiteral(const ast_StringLiteral* e, string_buffer_Buf* out)
{
   string_buffer_Buf_print(out, "\"%s\"", ast_idx2name(e->value));
}

static void ast_StringLiteral_print(const ast_StringLiteral* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, ' ');
   string_buffer_Buf_color(out, ast_col_Value);
   ast_StringLiteral_printLiteral(e, out);
   string_buffer_Buf_add1(out, '\n');
}

static ast_IntegerLiteral* ast_IntegerLiteral_create(ast_context_Context* c, src_loc_SrcLoc loc, uint8_t radix, uint64_t val)
{
   ast_IntegerLiteral* i = ast_context_Context_alloc(c, sizeof(ast_IntegerLiteral));
   ast_Expr_init(&i->parent, ast_ExprKind_IntegerLiteral, loc, 1, 1, 0, ast_ValType_RValue);
   i->parent.parent.integerLiteralBits.radix = radix;
   i->val = val;
   ast_Stats_addExpr(ast_ExprKind_IntegerLiteral, sizeof(ast_IntegerLiteral));
   ast_Expr_setType(&i->parent, ast_g_i32);
   return i;
}

static ast_IntegerLiteral* ast_IntegerLiteral_createUnsignedConstant(ast_context_Context* c, src_loc_SrcLoc loc, uint64_t val, ast_QualType qt)
{
   ast_IntegerLiteral* i = ast_IntegerLiteral_create(c, loc, 10, val);
   ast_Expr_setCtv(&i->parent);
   ast_Expr_setCtc(&i->parent);
   ast_Expr_setType(&i->parent, qt);
   return i;
}

static ast_IntegerLiteral* ast_IntegerLiteral_createSignedConstant(ast_context_Context* c, src_loc_SrcLoc loc, int64_t val, ast_QualType qt)
{
   ast_IntegerLiteral* i = ast_IntegerLiteral_create(c, loc, 10, ((uint64_t)(val)));
   i->parent.parent.integerLiteralBits.is_signed = 1;
   ast_Expr_setCtv(&i->parent);
   ast_Expr_setCtc(&i->parent);
   ast_Expr_setType(&i->parent, qt);
   return i;
}

static uint64_t ast_IntegerLiteral_getValue(const ast_IntegerLiteral* e)
{
   return e->val;
}

static bool ast_IntegerLiteral_isDecimal(const ast_IntegerLiteral* e)
{
   return e->parent.parent.integerLiteralBits.radix == 10;
}

static bool ast_IntegerLiteral_isSigned(const ast_IntegerLiteral* e)
{
   return e->parent.parent.integerLiteralBits.is_signed;
}

static void ast_printBinary(string_buffer_Buf* out, uint64_t value)
{
   char tmp[64];
   tmp[63] = 0;
   char* cp = &tmp[62];
   while (value) {
      *cp = '0' + ((value & 0x1));
      cp--;
      value /= 2;
   }
   *cp-- = 'b';
   *cp = '0';
   string_buffer_Buf_add(out, cp);
}

static void ast_printOctal(string_buffer_Buf* out, uint64_t value)
{
   char tmp[32];
   tmp[31] = 0;
   char* cp = &tmp[30];
   while (value) {
      *cp = '0' + ((value & 0x7));
      cp--;
      value /= 8;
   }
   *cp = '0';
   string_buffer_Buf_add(out, cp);
}

static void ast_IntegerLiteral_print(const ast_IntegerLiteral* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_color(out, ast_col_Value);
   string_buffer_Buf_add1(out, ' ');
   ast_IntegerLiteral_printLiteral(e, out);
   string_buffer_Buf_add1(out, '\n');
}

static void ast_IntegerLiteral_printLiteral(const ast_IntegerLiteral* e, string_buffer_Buf* out)
{
   switch (e->parent.parent.integerLiteralBits.radix) {
   case 2:
      ast_printBinary(out, e->val);
      break;
   case 8:
      ast_printOctal(out, e->val);
      break;
   case 10:
      if (e->parent.parent.integerLiteralBits.is_signed) {
         string_buffer_Buf_print(out, "%ld", ((int64_t)(e->val)));
      } else {
         string_buffer_Buf_print(out, "%lu", e->val);
      }
      break;
   case 16:
      string_buffer_Buf_print(out, "0x%lx", e->val);
      break;
   }
}

static void ast_IntegerLiteral_printDecimal(const ast_IntegerLiteral* e, string_buffer_Buf* out)
{
   if (e->parent.parent.integerLiteralBits.is_signed) {
      string_buffer_Buf_print(out, "%ld", ((int64_t)(e->val)));
   } else {
      string_buffer_Buf_print(out, "%lu", e->val);
   }
}

static ast_NilExpr* ast_NilExpr_create(ast_context_Context* c, src_loc_SrcLoc loc)
{
   ast_NilExpr* e = ast_context_Context_alloc(c, sizeof(ast_NilExpr));
   ast_Expr_init(&e->parent, ast_ExprKind_Nil, loc, 1, 1, 0, ast_ValType_RValue);
   ast_Expr_setType(&e->parent, ast_g_void_ptr);
   ast_Stats_addExpr(ast_ExprKind_Nil, sizeof(ast_NilExpr));
   return e;
}

static void ast_NilExpr_print(const ast_NilExpr* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, '\n');
}

static void ast_NilExpr_printLiteral(const ast_NilExpr* _arg0, string_buffer_Buf* out)
{
   string_buffer_Buf_add(out, "nil");
}

static ast_IdentifierExpr* ast_IdentifierExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, uint32_t name)
{
   ast_IdentifierExpr* e = ast_context_Context_alloc(c, sizeof(ast_IdentifierExpr));
   ast_Expr_init(&e->parent, ast_ExprKind_Identifier, loc, 0, 0, 0, ast_ValType_NValue);
   e->name_idx = name;
   ast_Stats_addExpr(ast_ExprKind_Identifier, sizeof(ast_IdentifierExpr));
   return e;
}

static ast_Expr* ast_IdentifierExpr_instantiate(ast_IdentifierExpr* e, ast_Instantiator* inst)
{
   return ((ast_Expr*)(ast_IdentifierExpr_create(inst->c, e->parent.loc, e->name_idx)));
}

static ast_Expr* ast_IdentifierExpr_asExpr(ast_IdentifierExpr* e)
{
   return &e->parent;
}

static ast_Decl* ast_IdentifierExpr_getDecl(const ast_IdentifierExpr* e)
{
   if (!e->parent.parent.identifierExprBits.has_decl) return NULL;

   return e->decl;
}

static ast_Ref ast_IdentifierExpr_getRef(const ast_IdentifierExpr* e)
{
   ast_Ref ref = { e->parent.loc, ast_IdentifierExpr_getNameIdx(e), ast_IdentifierExpr_getDecl(e) };
   return ref;
}

static void ast_IdentifierExpr_setKind(ast_IdentifierExpr* e, ast_IdentifierKind kind)
{
   e->parent.parent.identifierExprBits.kind = kind;
}

static ast_IdentifierKind ast_IdentifierExpr_getKind(const ast_IdentifierExpr* e)
{
   return ((ast_IdentifierKind)(e->parent.parent.identifierExprBits.kind));
}

static void ast_IdentifierExpr_setDecl(ast_IdentifierExpr* e, ast_Decl* decl)
{
   e->decl = decl;
   e->parent.parent.identifierExprBits.has_decl = true;
}

static const char* ast_IdentifierExpr_getName(const ast_IdentifierExpr* e)
{
   if (e->parent.parent.identifierExprBits.has_decl) return ast_Decl_getName(e->decl);

   return ast_idx2name(e->name_idx);
}

static uint32_t ast_IdentifierExpr_getNameIdx(const ast_IdentifierExpr* e)
{
   if (e->parent.parent.identifierExprBits.has_decl) return ast_Decl_getNameIdx(e->decl);

   return e->name_idx;
}

static void ast_IdentifierExpr_print(const ast_IdentifierExpr* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, ' ');
   ast_IdentifierKind kind = ast_IdentifierExpr_getKind(e);
   if (kind == ast_IdentifierKind_Unresolved) string_buffer_Buf_color(out, ast_col_Error);
   else string_buffer_Buf_color(out, ast_col_Attr);
   string_buffer_Buf_add(out, ast_identifierKind_names[kind]);
   string_buffer_Buf_add1(out, ' ');
   if (e->parent.parent.identifierExprBits.has_decl) {
      string_buffer_Buf_color(out, ast_col_Value);
      string_buffer_Buf_add(out, ast_Decl_getName(e->decl));
   } else {
      string_buffer_Buf_color(out, ast_col_Value);
      string_buffer_Buf_add(out, ast_idx2name(e->name_idx));
   }
   string_buffer_Buf_add1(out, '\n');
}

static void ast_IdentifierExpr_printLiteral(const ast_IdentifierExpr* e, string_buffer_Buf* out)
{
   string_buffer_Buf_add(out, ast_IdentifierExpr_getName(e));
}

static ast_CallExpr* ast_CallExpr_create(ast_context_Context* c, src_loc_SrcLoc endLoc, ast_Expr* fn, ast_Expr** args, uint32_t num_args)
{
   uint32_t size = sizeof(ast_CallExpr) + num_args * sizeof(ast_Expr*);
   ast_CallExpr* e = ast_context_Context_alloc(c, size);
   ast_Expr_init(&e->parent, ast_ExprKind_Call, ast_Expr_getLoc(fn), 0, 0, 1, ast_ValType_RValue);
   e->endLoc = endLoc;
   e->template_idx = 0;
   e->num_args = ((uint8_t)(num_args));
   e->fn = fn;
   memcpy(((void*)(e->args)), ((void*)(args)), num_args * sizeof(ast_Expr*));
   ast_Stats_addExpr(ast_ExprKind_Call, size);
   return e;
}

static ast_CallExpr* ast_CallExpr_createTemplate(ast_context_Context* c, src_loc_SrcLoc endLoc, ast_Expr* fn, ast_Expr** args, uint32_t num_args, const ast_TypeRefHolder* ref)
{
   uint32_t size = sizeof(ast_CallExpr) + num_args * sizeof(ast_Expr*);
   size += sizeof(ast_TypeRef) + ast_TypeRefHolder_getExtraSize(ref);
   ast_CallExpr* e = ast_context_Context_alloc(c, size);
   ast_Expr_init(&e->parent, ast_ExprKind_Call, ast_Expr_getLoc(fn), 0, 0, 1, ast_ValType_RValue);
   e->parent.parent.callExprBits.is_template_call = 1;
   e->endLoc = endLoc;
   e->template_idx = 0;
   e->num_args = ((uint8_t)(num_args));
   e->fn = fn;
   memcpy(((void*)(e->args)), ((void*)(args)), num_args * sizeof(ast_Expr*));
   ast_TypeRef* destRef = ((ast_TypeRef*)(&e->args[num_args]));
   ast_TypeRefHolder_fill(ref, destRef);
   ast_Stats_addExpr(ast_ExprKind_Call, size);
   return e;
}

static ast_Expr* ast_CallExpr_instantiate(ast_CallExpr* e, ast_Instantiator* inst)
{
   assert(!ast_CallExpr_isTemplateCall(e));
   uint32_t size = sizeof(ast_CallExpr) + e->num_args * sizeof(ast_Expr*);
   ast_CallExpr* e2 = ast_context_Context_alloc(inst->c, size);
   e2->parent = e->parent;
   e2->endLoc = e->endLoc;
   e2->num_args = e->num_args;
   e2->fn = ast_Expr_instantiate(e->fn, inst);
   for (uint32_t i = 0; i < e->num_args; i++) {
      e2->args[i] = ast_Expr_instantiate(e->args[i], inst);
   }
   ast_Stats_addExpr(ast_ExprKind_Call, size);
   return ((ast_Expr*)(e2));
}

static void ast_CallExpr_setCallsStructFunc(ast_CallExpr* e)
{
   e->parent.parent.callExprBits.calls_struct_func = 1;
}

static bool ast_CallExpr_isStructFunc(const ast_CallExpr* e)
{
   return e->parent.parent.callExprBits.calls_struct_func;
}

static void ast_CallExpr_setCallsStaticStructFunc(ast_CallExpr* e)
{
   e->parent.parent.callExprBits.calls_static_sf = 1;
}

static bool ast_CallExpr_isStaticStructFunc(const ast_CallExpr* e)
{
   return e->parent.parent.callExprBits.calls_static_sf;
}

static bool ast_CallExpr_isTemplateCall(const ast_CallExpr* e)
{
   return e->parent.parent.callExprBits.is_template_call;
}

static ast_TypeRef* ast_CallExpr_getTemplateArg(const ast_CallExpr* e)
{
   if (ast_CallExpr_isTemplateCall(e)) return ((ast_TypeRef*)(&e->args[e->num_args]));

   return NULL;
}

static void ast_CallExpr_setTemplateIdx(ast_CallExpr* e, uint32_t idx)
{
   e->template_idx = ((uint16_t)(idx));
}

static uint32_t ast_CallExpr_getTemplateIdx(const ast_CallExpr* e)
{
   return e->template_idx;
}

static src_loc_SrcLoc ast_CallExpr_getEndLoc(const ast_CallExpr* e)
{
   return e->endLoc;
}

static ast_Expr* ast_CallExpr_getFunc(const ast_CallExpr* e)
{
   return e->fn;
}

static ast_Expr** ast_CallExpr_getFunc2(ast_CallExpr* e)
{
   return &e->fn;
}

static uint32_t ast_CallExpr_getNumArgs(const ast_CallExpr* e)
{
   return e->num_args;
}

static ast_Expr** ast_CallExpr_getArgs(ast_CallExpr* e)
{
   return e->args;
}

static void ast_CallExpr_print(const ast_CallExpr* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   if (e->parent.parent.callExprBits.calls_struct_func) string_buffer_Buf_add(out, " SF");
   if (e->parent.parent.callExprBits.calls_static_sf) string_buffer_Buf_add(out, " SSF");
   string_buffer_Buf_add1(out, '\n');
   if (e->parent.parent.callExprBits.is_template_call) {
      string_buffer_Buf_indent(out, indent + 1);
      string_buffer_Buf_color(out, ast_col_Template);
      string_buffer_Buf_add(out, "template ");
      ast_TypeRef* ref = ((ast_TypeRef*)(&e->args[e->num_args]));
      ast_TypeRef_print(ref, out, true);
      string_buffer_Buf_add1(out, '\n');
   }
   ast_Expr_print(e->fn, out, indent + 1);
   for (uint32_t i = 0; i < ast_CallExpr_getNumArgs(e); i++) {
      ast_Expr_print(e->args[i], out, indent + 1);
   }
}

static ast_InitListExpr* ast_InitListExpr_create(ast_context_Context* c, src_loc_SrcLoc left, src_loc_SrcLoc right, ast_Expr** values, uint32_t num_values)
{
   uint32_t size = sizeof(ast_InitListExpr) + num_values * sizeof(ast_Expr*);
   ast_InitListExpr* e = ast_context_Context_alloc(c, size);
   ast_Expr_init(&e->parent, ast_ExprKind_InitList, left, 0, 0, 0, ast_ValType_RValue);
   e->parent.parent.initListExprBits.num_values = num_values;
   e->right = right;
   if (num_values) {
      memcpy(((void*)(e->values)), ((void*)(values)), num_values * sizeof(ast_Expr*));
   }
   ast_Stats_addExpr(ast_ExprKind_InitList, size);
   return e;
}

static ast_Expr* ast_InitListExpr_instantiate(ast_InitListExpr* e, ast_Instantiator* inst)
{
   uint32_t num_values = ast_InitListExpr_getNumValues(e);
   uint32_t size = sizeof(ast_InitListExpr) + num_values * sizeof(ast_Expr*);
   ast_InitListExpr* e2 = ast_context_Context_alloc(inst->c, size);
   e2->parent = e->parent;
   e2->right = e->right;
   for (uint32_t i = 0; i < num_values; i++) {
      e2->values[i] = ast_Expr_instantiate(e->values[i], inst);
   }
   ast_Stats_addExpr(ast_ExprKind_InitList, size);
   return ((ast_Expr*)(e2));
}

static uint32_t ast_InitListExpr_getNumValues(const ast_InitListExpr* e)
{
   return e->parent.parent.initListExprBits.num_values;
}

static ast_Expr** ast_InitListExpr_getValues(ast_InitListExpr* e)
{
   return e->values;
}

static void ast_InitListExpr_print(const ast_InitListExpr* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, '\n');
   for (uint32_t i = 0; i < ast_InitListExpr_getNumValues(e); i++) {
      ast_Expr_print(e->values[i], out, indent + 1);
   }
}

static ast_FieldDesignatedInitExpr* ast_FieldDesignatedInitExpr_create(ast_context_Context* c, uint32_t field, src_loc_SrcLoc loc, ast_Expr* initValue)
{
   ast_FieldDesignatedInitExpr* e = ast_context_Context_alloc(c, sizeof(ast_FieldDesignatedInitExpr));
   ast_Expr_init(&e->parent, ast_ExprKind_FieldDesignatedInit, loc, 0, 0, 0, ast_ValType_RValue);
   e->field = field;
   e->initValue = initValue;
   ast_Stats_addExpr(ast_ExprKind_FieldDesignatedInit, sizeof(ast_FieldDesignatedInitExpr));
   return e;
}

static ast_Expr* ast_FieldDesignatedInitExpr_instantiate(ast_FieldDesignatedInitExpr* e, ast_Instantiator* inst)
{
   return ((ast_Expr*)(ast_FieldDesignatedInitExpr_create(inst->c, e->field, e->parent.loc, ast_Expr_instantiate(e->initValue, inst))));
}

static uint32_t ast_FieldDesignatedInitExpr_getField(const ast_FieldDesignatedInitExpr* e)
{
   return e->field;
}

static const char* ast_FieldDesignatedInitExpr_getFieldName(const ast_FieldDesignatedInitExpr* e)
{
   return ast_idx2name(e->field);
}

static ast_Expr* ast_FieldDesignatedInitExpr_getInit(ast_FieldDesignatedInitExpr* e)
{
   return e->initValue;
}

static ast_Expr** ast_FieldDesignatedInitExpr_getInit2(ast_FieldDesignatedInitExpr* e)
{
   return &e->initValue;
}

static void ast_FieldDesignatedInitExpr_print(const ast_FieldDesignatedInitExpr* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_color(out, ast_col_Value);
   string_buffer_Buf_print(out, " %s\n", ast_idx2name(e->field));
   ast_Expr_print(e->initValue, out, indent + 1);
}

static ast_ArrayDesignatedInitExpr* ast_ArrayDesignatedInitExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* designator, ast_Expr* initValue)
{
   ast_ArrayDesignatedInitExpr* e = ast_context_Context_alloc(c, sizeof(ast_ArrayDesignatedInitExpr));
   ast_Expr_init(&e->parent, ast_ExprKind_ArrayDesignatedInit, loc, 0, 0, 0, ast_ValType_RValue);
   e->designator = designator;
   e->initValue = initValue;
   ast_Stats_addExpr(ast_ExprKind_ArrayDesignatedInit, sizeof(ast_ArrayDesignatedInitExpr));
   return e;
}

static ast_Expr* ast_ArrayDesignatedInitExpr_instantiate(ast_ArrayDesignatedInitExpr* e, ast_Instantiator* inst)
{
   ast_ArrayDesignatedInitExpr* f = ast_ArrayDesignatedInitExpr_create(inst->c, e->parent.loc, ast_Expr_instantiate(e->designator, inst), ast_Expr_instantiate(e->initValue, inst));
   return ((ast_Expr*)(f));
}

static ast_Expr* ast_ArrayDesignatedInitExpr_getDesignator(const ast_ArrayDesignatedInitExpr* e)
{
   return e->designator;
}

static ast_Expr** ast_ArrayDesignatedInitExpr_getDesignator2(ast_ArrayDesignatedInitExpr* e)
{
   return &e->designator;
}

static ast_Expr* ast_ArrayDesignatedInitExpr_getInit(const ast_ArrayDesignatedInitExpr* e)
{
   return e->initValue;
}

static ast_Expr** ast_ArrayDesignatedInitExpr_getInit2(ast_ArrayDesignatedInitExpr* e)
{
   return &e->initValue;
}

static void ast_ArrayDesignatedInitExpr_print(const ast_ArrayDesignatedInitExpr* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, '\n');
   ast_Expr_print(e->designator, out, indent + 1);
   ast_Expr_print(e->initValue, out, indent + 1);
}

static ast_ParenExpr* ast_ParenExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* inner)
{
   ast_ParenExpr* e = ast_context_Context_alloc(c, sizeof(ast_ParenExpr));
   ast_Expr_init(&e->parent, ast_ExprKind_Paren, loc, 0, 0, 0, ast_ValType_NValue);
   e->inner = inner;
   ast_Stats_addExpr(ast_ExprKind_Paren, sizeof(ast_ParenExpr));
   return e;
}

static ast_Expr* ast_ParenExpr_instantiate(ast_ParenExpr* e, ast_Instantiator* inst)
{
   return ((ast_Expr*)(ast_ParenExpr_create(inst->c, e->parent.loc, ast_Expr_instantiate(e->inner, inst))));
}

static ast_Expr* ast_ParenExpr_getInner(const ast_ParenExpr* e)
{
   return e->inner;
}

static ast_Expr** ast_ParenExpr_getInner2(ast_ParenExpr* e)
{
   return &e->inner;
}

static void ast_ParenExpr_print(const ast_ParenExpr* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, '\n');
   ast_Expr_print(e->inner, out, indent + 1);
}

static void ast_ParenExpr_printLiteral(const ast_ParenExpr* e, string_buffer_Buf* out)
{
   string_buffer_Buf_add1(out, '(');
   ast_Expr_printLiteral(e->inner, out);
   string_buffer_Buf_add1(out, '(');
}

static ast_UnaryOperator* ast_UnaryOperator_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_UnaryOpcode kind, ast_Expr* inner)
{
   ast_UnaryOperator* e = ast_context_Context_alloc(c, sizeof(ast_UnaryOperator));
   ast_Expr_init(&e->parent, ast_ExprKind_UnaryOperator, loc, 0, 0, kind <= ast_UnaryOpcode_PreDec, ast_ValType_RValue);
   e->parent.parent.unaryOperatorBits.kind = kind;
   e->inner = inner;
   ast_Stats_addExpr(ast_ExprKind_UnaryOperator, sizeof(ast_UnaryOperator));
   return e;
}

static ast_Expr* ast_UnaryOperator_instantiate(ast_UnaryOperator* e, ast_Instantiator* inst)
{
   return ((ast_Expr*)(ast_UnaryOperator_create(inst->c, e->parent.loc, ast_UnaryOperator_getOpcode(e), ast_Expr_instantiate(e->inner, inst))));
}

static ast_UnaryOpcode ast_UnaryOperator_getOpcode(const ast_UnaryOperator* e)
{
   return ((ast_UnaryOpcode)(e->parent.parent.unaryOperatorBits.kind));
}

static ast_Expr* ast_UnaryOperator_getInner(const ast_UnaryOperator* e)
{
   return e->inner;
}

static ast_Expr** ast_UnaryOperator_getInner2(ast_UnaryOperator* e)
{
   return &e->inner;
}

static ast_Expr* ast_UnaryOperator_asExpr(ast_UnaryOperator* e)
{
   return &e->parent;
}

static bool ast_UnaryOperator_isBefore(const ast_UnaryOperator* e)
{
   switch (ast_UnaryOperator_getOpcode(e)) {
   case ast_UnaryOpcode_PostInc:
      return false;
   case ast_UnaryOpcode_PostDec:
      return false;
   case ast_UnaryOpcode_PreInc:
      return true;
   case ast_UnaryOpcode_PreDec:
      return true;
   case ast_UnaryOpcode_AddrOf:
      return true;
   case ast_UnaryOpcode_Deref:
      return true;
   case ast_UnaryOpcode_Minus:
      return true;
   case ast_UnaryOpcode_Not:
      return true;
   case ast_UnaryOpcode_LNot:
      return true;
   }
   return true;
}

static src_loc_SrcLoc ast_UnaryOperator_getStartLoc(const ast_UnaryOperator* e)
{
   if (ast_UnaryOperator_isBefore(e)) return ast_Expr_getLoc(&e->parent);

   return ast_Expr_getStartLoc(e->inner);
}

static src_loc_SrcLoc ast_UnaryOperator_getEndLoc(const ast_UnaryOperator* e)
{
   if (ast_UnaryOperator_isBefore(e)) return ast_Expr_getStartLoc(e->inner);

   return ast_Expr_getLoc(&e->parent);
}

static const char* ast_UnaryOperator_getOpcodeStr(const ast_UnaryOperator* e)
{
   return ast_unaryOpcode_names[ast_UnaryOperator_getOpcode(e)];
}

static void ast_UnaryOperator_print(const ast_UnaryOperator* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, ' ');
   string_buffer_Buf_color(out, ast_col_Value);
   string_buffer_Buf_add(out, ast_unaryOpcode_names[ast_UnaryOperator_getOpcode(e)]);
   string_buffer_Buf_add1(out, '\n');
   ast_Expr_print(e->inner, out, indent + 1);
}

static void ast_UnaryOperator_printLiteral(const ast_UnaryOperator* e, string_buffer_Buf* out)
{
   const char* opcode = ast_unaryOpcode_names[ast_UnaryOperator_getOpcode(e)];
   if (ast_UnaryOperator_isBefore(e)) {
      string_buffer_Buf_add(out, opcode);
      ast_Expr_printLiteral(e->inner, out);
   } else {
      ast_Expr_printLiteral(e->inner, out);
      string_buffer_Buf_add(out, opcode);
   }
}

static ast_ConditionalOperator* ast_ConditionalOperator_create(ast_context_Context* c, src_loc_SrcLoc questionLoc, src_loc_SrcLoc colonLoc, ast_Expr* cond, ast_Expr* lhs, ast_Expr* rhs)
{
   ast_ConditionalOperator* e = ast_context_Context_alloc(c, sizeof(ast_ConditionalOperator));
   ast_Expr_init(&e->parent, ast_ExprKind_ConditionalOperator, questionLoc, 0, 1, 1, ast_ValType_RValue);
   e->colonLoc = colonLoc;
   e->cond = cond;
   e->lhs = lhs;
   e->rhs = rhs;
   ast_Stats_addExpr(ast_ExprKind_ConditionalOperator, sizeof(ast_ConditionalOperator));
   return e;
}

static ast_Expr* ast_ConditionalOperator_instantiate(ast_ConditionalOperator* e, ast_Instantiator* inst)
{
   ast_ConditionalOperator* o = ast_ConditionalOperator_create(inst->c, e->parent.loc, e->colonLoc, ast_Expr_instantiate(e->cond, inst), ast_Expr_instantiate(e->lhs, inst), ast_Expr_instantiate(e->rhs, inst));
   return ((ast_Expr*)(o));
}

static ast_Expr* ast_ConditionalOperator_getCond(const ast_ConditionalOperator* e)
{
   return e->cond;
}

static ast_Expr** ast_ConditionalOperator_getCond2(ast_ConditionalOperator* e)
{
   return &e->cond;
}

static ast_Expr* ast_ConditionalOperator_getLHS(const ast_ConditionalOperator* e)
{
   return e->lhs;
}

static ast_Expr** ast_ConditionalOperator_getLHS2(ast_ConditionalOperator* e)
{
   return &e->lhs;
}

static ast_Expr* ast_ConditionalOperator_getRHS(const ast_ConditionalOperator* e)
{
   return e->rhs;
}

static ast_Expr** ast_ConditionalOperator_getRHS2(ast_ConditionalOperator* e)
{
   return &e->rhs;
}

static void ast_ConditionalOperator_print(const ast_ConditionalOperator* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, '\n');
   ast_Expr_print(e->cond, out, indent + 1);
   ast_Expr_print(e->lhs, out, indent + 1);
   ast_Expr_print(e->rhs, out, indent + 1);
}

static ast_BinaryOperator* ast_BinaryOperator_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_BinaryOpcode kind, ast_Expr* lhs, ast_Expr* rhs)
{
   ast_BinaryOperator* e = ast_context_Context_alloc(c, sizeof(ast_BinaryOperator));
   ast_Expr_init(&e->parent, ast_ExprKind_BinaryOperator, loc, 0, 0, kind >= ast_BinaryOpcode_Assign, ast_ValType_RValue);
   e->parent.parent.binaryOperatorBits.kind = kind;
   e->lhs = lhs;
   e->rhs = rhs;
   ast_Stats_addExpr(ast_ExprKind_BinaryOperator, sizeof(ast_BinaryOperator));
   return e;
}

static ast_Expr* ast_BinaryOperator_instantiate(ast_BinaryOperator* e, ast_Instantiator* inst)
{
   return ((ast_Expr*)(ast_BinaryOperator_create(inst->c, e->parent.loc, ast_BinaryOperator_getOpcode(e), ast_Expr_instantiate(e->lhs, inst), ast_Expr_instantiate(e->rhs, inst))));
}

static ast_BinaryOpcode ast_BinaryOperator_getOpcode(const ast_BinaryOperator* e)
{
   return ((ast_BinaryOpcode)(e->parent.parent.binaryOperatorBits.kind));
}

static ast_Expr* ast_BinaryOperator_getLHS(const ast_BinaryOperator* e)
{
   return e->lhs;
}

static ast_Expr** ast_BinaryOperator_getLHS2(ast_BinaryOperator* e)
{
   return &e->lhs;
}

static ast_Expr* ast_BinaryOperator_getRHS(const ast_BinaryOperator* e)
{
   return e->rhs;
}

static ast_Expr** ast_BinaryOperator_getRHS2(ast_BinaryOperator* e)
{
   return &e->rhs;
}

static const char* ast_BinaryOperator_getOpcodeStr(const ast_BinaryOperator* e)
{
   return ast_binaryOpcode_names[ast_BinaryOperator_getOpcode(e)];
}

static void ast_BinaryOperator_print(const ast_BinaryOperator* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_color(out, ast_col_Value);
   string_buffer_Buf_add1(out, ' ');
   string_buffer_Buf_add(out, ast_binaryOpcode_names[ast_BinaryOperator_getOpcode(e)]);
   string_buffer_Buf_add1(out, '\n');
   string_buffer_Buf_indent(out, indent + 1);
   string_buffer_Buf_color(out, ast_col_Attr);
   string_buffer_Buf_add(out, "LHS=\n");
   ast_Expr_print(e->lhs, out, indent + 1);
   string_buffer_Buf_indent(out, indent + 1);
   string_buffer_Buf_color(out, ast_col_Attr);
   string_buffer_Buf_add(out, "RHS=\n");
   ast_Expr_print(e->rhs, out, indent + 1);
}

static void ast_BinaryOperator_printLiteral(const ast_BinaryOperator* e, string_buffer_Buf* out)
{
   ast_Expr_printLiteral(e->lhs, out);
   string_buffer_Buf_add(out, ast_binaryOpcode_names[ast_BinaryOperator_getOpcode(e)]);
   ast_Expr_printLiteral(e->rhs, out);
}

static ast_TypeExpr* ast_TypeExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, const ast_TypeRefHolder* ref)
{
   uint32_t size = sizeof(ast_TypeExpr) + ast_TypeRefHolder_getExtraSize(ref);
   ast_TypeExpr* e = ast_context_Context_alloc(c, size);
   ast_Expr_init(&e->parent, ast_ExprKind_Type, loc, 0, 0, 0, ast_ValType_NValue);
   ast_TypeRefHolder_fill(ref, &e->typeRef);
   ast_Stats_addExpr(ast_ExprKind_Type, size);
   return e;
}

static ast_Expr* ast_TypeExpr_instantiate(ast_TypeExpr* e, ast_Instantiator* inst)
{
   bool matches = ast_TypeRef_matchesTemplate(&e->typeRef, inst->template_name);
   uint32_t extra = matches ? ast_TypeRef_getExtraSize(inst->ref) : ast_TypeRef_getExtraSize(&e->typeRef);
   uint32_t size = sizeof(ast_TypeExpr) + extra;
   ast_TypeExpr* e2 = ast_context_Context_alloc(inst->c, size);
   e2->parent = e->parent;
   ast_TypeRef_instantiate(&e2->typeRef, &e->typeRef, inst->ref, inst->template_name);
   ast_Stats_addExpr(ast_ExprKind_Type, size);
   return ((ast_Expr*)(e2));
}

static ast_TypeRef* ast_TypeExpr_getTypeRef(ast_TypeExpr* e)
{
   return &e->typeRef;
}

static void ast_TypeExpr_print(const ast_TypeExpr* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, ' ');
   ast_TypeRef_print(&e->typeRef, out, true);
   string_buffer_Buf_add1(out, '\n');
}

static ast_BitOffsetExpr* ast_BitOffsetExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* lhs, ast_Expr* rhs)
{
   ast_BitOffsetExpr* e = ast_context_Context_alloc(c, sizeof(ast_BitOffsetExpr));
   ast_Expr_init(&e->parent, ast_ExprKind_BitOffset, loc, 0, 0, 0, ast_ValType_RValue);
   e->lhs = lhs;
   e->rhs = rhs;
   ast_Stats_addExpr(ast_ExprKind_BitOffset, sizeof(ast_BitOffsetExpr));
   return e;
}

static ast_Expr* ast_BitOffsetExpr_instantiate(ast_BitOffsetExpr* e, ast_Instantiator* inst)
{
   ast_BitOffsetExpr* b = ast_BitOffsetExpr_create(inst->c, e->parent.loc, ast_Expr_instantiate(e->lhs, inst), ast_Expr_instantiate(e->rhs, inst));
   return ((ast_Expr*)(b));
}

static ast_Expr* ast_BitOffsetExpr_getLHS(ast_BitOffsetExpr* e)
{
   return e->lhs;
}

static ast_Expr** ast_BitOffsetExpr_getLHS2(ast_BitOffsetExpr* e)
{
   return &e->lhs;
}

static ast_Expr* ast_BitOffsetExpr_getRHS(ast_BitOffsetExpr* e)
{
   return e->rhs;
}

static ast_Expr** ast_BitOffsetExpr_getRHS2(ast_BitOffsetExpr* e)
{
   return &e->rhs;
}

static void ast_BitOffsetExpr_print(const ast_BitOffsetExpr* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, '\n');
   ast_Expr_print(e->lhs, out, indent + 1);
   ast_Expr_print(e->rhs, out, indent + 1);
}

static ast_ArraySubscriptExpr* ast_ArraySubscriptExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_Expr* base, ast_Expr* idx)
{
   ast_ArraySubscriptExpr* e = ast_context_Context_alloc(c, sizeof(ast_ArraySubscriptExpr));
   ast_Expr_init(&e->parent, ast_ExprKind_ArraySubscript, loc, 0, 0, 0, ast_ValType_LValue);
   e->base = base;
   e->idx = idx;
   ast_Stats_addExpr(ast_ExprKind_ArraySubscript, sizeof(ast_ArraySubscriptExpr));
   return e;
}

static ast_Expr* ast_ArraySubscriptExpr_instantiate(ast_ArraySubscriptExpr* e, ast_Instantiator* inst)
{
   ast_ArraySubscriptExpr* a = ast_ArraySubscriptExpr_create(inst->c, e->parent.loc, ast_Expr_instantiate(e->base, inst), ast_Expr_instantiate(e->idx, inst));
   return ((ast_Expr*)(a));
}

static ast_Expr* ast_ArraySubscriptExpr_getBase(const ast_ArraySubscriptExpr* e)
{
   return e->base;
}

static ast_Expr** ast_ArraySubscriptExpr_getBase2(ast_ArraySubscriptExpr* e)
{
   return &e->base;
}

static ast_Expr* ast_ArraySubscriptExpr_getIndex(const ast_ArraySubscriptExpr* e)
{
   return e->idx;
}

static ast_Expr** ast_ArraySubscriptExpr_getIndex2(ast_ArraySubscriptExpr* e)
{
   return &e->idx;
}

static void ast_ArraySubscriptExpr_print(const ast_ArraySubscriptExpr* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, '\n');
   ast_Expr_print(e->base, out, indent + 1);
   ast_Expr_print(e->idx, out, indent + 1);
}

static ast_MemberExpr* ast_MemberExpr_create(ast_context_Context* c, ast_Expr* base, const ast_Ref* refs, uint32_t refcount)
{
   uint32_t size = sizeof(ast_MemberExpr) + refcount * sizeof(ast_MemberRef) + (refcount - 1) * sizeof(uint32_t);
   if (base) size += sizeof(ast_MemberRef);
   size = ((size + 7) & ~0x7);
   ast_MemberExpr* e = ast_context_Context_alloc(c, size);
   ast_Expr_init(&e->parent, ast_ExprKind_Member, refs[0].loc, 0, 0, 0, ast_ValType_NValue);
   e->parent.parent.memberExprBits.num_refs = refcount;
   uint32_t offset = 0;
   if (base) {
      offset = 1;
      e->refs[0].expr = base;
      e->parent.parent.memberExprBits.has_expr = 1;
   }
   for (uint32_t i = 0; i < refcount; i++) {
      e->refs[i + offset].name_idx = refs[i].name_idx;
   }
   src_loc_SrcLoc* locs = ((src_loc_SrcLoc*)(&e->refs[refcount + offset]));
   for (uint32_t i = 0; i < refcount - 1; i++) {
      locs[i] = refs[i + 1].loc;
   }
   ast_Stats_addExpr(ast_ExprKind_Member, size);
   return e;
}

static ast_Expr* ast_MemberExpr_instantiate(ast_MemberExpr* e, ast_Instantiator* inst)
{
   uint32_t refcount = e->parent.parent.memberExprBits.num_refs;
   ast_Expr* base = ast_MemberExpr_getExprBase(e);
   uint32_t size = sizeof(ast_MemberExpr) + refcount * sizeof(ast_MemberRef) + (refcount - 1) * sizeof(uint32_t);
   if (base) size += sizeof(ast_MemberRef);
   size = ((size + 7) & ~0x7);
   ast_MemberExpr* e2 = ast_context_Context_alloc(inst->c, size);
   memcpy(e2, e, size);
   if (base) e2->refs[0].expr = ast_Expr_instantiate(base, inst);
   return ((ast_Expr*)(e2));
}

static bool ast_MemberExpr_hasExpr(const ast_MemberExpr* e)
{
   return e->parent.parent.memberExprBits.has_expr;
}

static ast_Expr* ast_MemberExpr_getExprBase(const ast_MemberExpr* e)
{
   if (ast_MemberExpr_hasExpr(e)) return e->refs[0].expr;

   return NULL;
}

static const char* ast_MemberExpr_getName(const ast_MemberExpr* e, uint32_t ref_idx)
{
   const ast_MemberRef* ref = &e->refs[ref_idx + ast_MemberExpr_hasExpr(e)];
   if (e->parent.parent.memberExprBits.num_decls > ref_idx) {
      return ast_Decl_getName(ref->decl);
   }
   return ast_idx2name(ref->name_idx);
}

static uint32_t ast_MemberExpr_getNumRefs(const ast_MemberExpr* e)
{
   return e->parent.parent.memberExprBits.num_refs;
}

static uint32_t ast_MemberExpr_getNameIdx(const ast_MemberExpr* e, uint32_t ref_idx)
{
   const ast_MemberRef* ref = &e->refs[ref_idx + ast_MemberExpr_hasExpr(e)];
   if (e->parent.parent.memberExprBits.num_decls > ref_idx) {
      assert(0);
   }
   return ref->name_idx;
}

static src_loc_SrcLoc ast_MemberExpr_getLoc(const ast_MemberExpr* e, uint32_t ref_idx)
{
   if (ref_idx == 0) return ast_Expr_getLoc(&e->parent);

   src_loc_SrcLoc* locs = ((src_loc_SrcLoc*)(&e->refs[ast_MemberExpr_getNumRefs(e) + ast_MemberExpr_hasExpr(e)]));
   return locs[ref_idx - 1];
}

static ast_IdentifierKind ast_MemberExpr_getKind(const ast_MemberExpr* e)
{
   return ((ast_IdentifierKind)(e->parent.parent.memberExprBits.kind));
}

static void ast_MemberExpr_setKind(ast_MemberExpr* e, ast_IdentifierKind kind)
{
   e->parent.parent.memberExprBits.kind = kind;
}

static void ast_MemberExpr_setIsStructFunc(ast_MemberExpr* e)
{
   e->parent.parent.memberExprBits.is_struct_func = 1;
}

static bool ast_MemberExpr_isStructFunc(const ast_MemberExpr* e)
{
   return e->parent.parent.memberExprBits.is_struct_func;
}

static void ast_MemberExpr_setIsStaticStructFunc(ast_MemberExpr* e)
{
   e->parent.parent.memberExprBits.is_static_sf = 1;
}

static bool ast_MemberExpr_isStaticStructFunc(const ast_MemberExpr* e)
{
   return e->parent.parent.memberExprBits.is_static_sf;
}

static ast_Decl* ast_MemberExpr_getFullDecl(const ast_MemberExpr* e)
{
   uint32_t num = ast_MemberExpr_getNumRefs(e);
   if (e->parent.parent.memberExprBits.num_decls < num) return NULL;

   num += ast_MemberExpr_hasExpr(e);
   return e->refs[num - 1].decl;
}

static ast_Decl* ast_MemberExpr_getDecl(const ast_MemberExpr* e, uint32_t ref_idx)
{
   if (e->parent.parent.memberExprBits.num_decls <= ref_idx) return NULL;

   return e->refs[ref_idx + ast_MemberExpr_hasExpr(e)].decl;
}

static void ast_MemberExpr_setDecl(ast_MemberExpr* e, ast_Decl* d, uint32_t ref_idx)
{
   e->parent.parent.memberExprBits.num_decls = ref_idx + 1;
   e->refs[ref_idx + ast_MemberExpr_hasExpr(e)].decl = d;
}

static src_loc_SrcLoc ast_MemberExpr_getStartLoc(const ast_MemberExpr* e)
{
   if (ast_MemberExpr_hasExpr(e)) return ast_Expr_getStartLoc(e->refs[0].expr);

   return ast_Expr_getLoc(&e->parent);
}

static src_loc_SrcLoc ast_MemberExpr_getEndLoc(const ast_MemberExpr* e)
{
   return ast_MemberExpr_getLoc(e, ast_MemberExpr_getNumRefs(e) - 1);
}

static ast_QualType ast_MemberExpr_getBaseType(const ast_MemberExpr* m)
{
   uint32_t numRefs = ast_MemberExpr_getNumRefs(m);
   if ((ast_MemberExpr_hasExpr(m) && numRefs == 1)) {
      return ast_Expr_getType(m->refs[0].expr);
   }
   return ast_Decl_getType(m->refs[ast_MemberExpr_hasExpr(m) + numRefs - 2].decl);
}

static void ast_MemberExpr_print(const ast_MemberExpr* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, ' ');
   ast_IdentifierKind kind = ast_MemberExpr_getKind(e);
   if (kind == ast_IdentifierKind_Unresolved) string_buffer_Buf_color(out, ast_col_Error);
   else string_buffer_Buf_color(out, ast_col_Attr);
   string_buffer_Buf_add(out, ast_identifierKind_names[kind]);
   string_buffer_Buf_color(out, ast_col_Attr);
   if (ast_MemberExpr_isStructFunc(e)) string_buffer_Buf_add(out, " SF");
   if (ast_MemberExpr_isStaticStructFunc(e)) string_buffer_Buf_add(out, " SSF");
   string_buffer_Buf_print(out, " refs=%u/%u ", e->parent.parent.memberExprBits.num_decls, ast_MemberExpr_getNumRefs(e));
   string_buffer_Buf_color(out, ast_col_Value);
   ast_MemberExpr_printLiteral(e, out);
   string_buffer_Buf_add1(out, '\n');
   if (ast_MemberExpr_hasExpr(e)) ast_Expr_print(e->refs[0].expr, out, indent + 1);
}

static void ast_MemberExpr_printLiteral(const ast_MemberExpr* e, string_buffer_Buf* out)
{
   if (ast_MemberExpr_hasExpr(e)) {
      string_buffer_Buf_add(out, "<expr>.");
   }
   for (uint32_t i = 0; i < ast_MemberExpr_getNumRefs(e); i++) {
      if (i != 0) string_buffer_Buf_add(out, ".");
      string_buffer_Buf_add(out, ast_MemberExpr_getName(e, i));
   }
}

static void ast_MemberExpr_dump(const ast_MemberExpr* m)
{
   string_buffer_Buf* out = string_buffer_create(10 * 4096, utils_useColor(), 2);
   string_buffer_Buf_color(out, ast_col_Expr);
   string_buffer_Buf_print(out, "MemberExpr expr %d ref %u/%u\n", ast_MemberExpr_hasExpr(m), m->parent.parent.memberExprBits.num_decls, ast_MemberExpr_getNumRefs(m));
   if (ast_MemberExpr_hasExpr(m)) {
      string_buffer_Buf_indent(out, 1);
      string_buffer_Buf_color(out, ast_col_Value);
      string_buffer_Buf_print(out, "<expr>\n");
      ast_Expr* e = ast_MemberExpr_getExprBase(m);
      ast_Expr_print(e, out, 1);
   }
   for (uint32_t i = 0; i < ast_MemberExpr_getNumRefs(m); i++) {
      const ast_MemberRef* ref = &m->refs[i + ast_MemberExpr_hasExpr(m)];
      string_buffer_Buf_indent(out, 1);
      if (m->parent.parent.memberExprBits.num_decls > i) {
         string_buffer_Buf_print(out, "[%u]\n", i);
         ast_Decl_print(ref->decl, out, 1);
      } else {
         string_buffer_Buf_print(out, "[%u] %s\n", i, ast_idx2name(ref->name_idx));
      }
   }
   string_buffer_Buf_color(out, ast_col_Normal);
   puts(string_buffer_Buf_data(out));
   string_buffer_Buf_free(out);
}

static ast_ExplicitCastExpr* ast_ExplicitCastExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, const ast_TypeRefHolder* ref, ast_Expr* inner)
{
   uint32_t size = sizeof(ast_ExplicitCastExpr) + ast_TypeRefHolder_getExtraSize(ref);
   ast_ExplicitCastExpr* e = ast_context_Context_alloc(c, size);
   ast_Expr_init(&e->parent, ast_ExprKind_ExplicitCast, loc, 0, 0, 0, ast_ValType_NValue);
   e->inner = inner;
   ast_TypeRefHolder_fill(ref, &e->dest);
   ast_Stats_addExpr(ast_ExprKind_ExplicitCast, size);
   return e;
}

static ast_Expr* ast_ExplicitCastExpr_instantiate(ast_ExplicitCastExpr* e, ast_Instantiator* inst)
{
   bool matches = ast_TypeRef_matchesTemplate(&e->dest, inst->template_name);
   uint32_t extra = matches ? ast_TypeRef_getExtraSize(inst->ref) : ast_TypeRef_getExtraSize(&e->dest);
   uint32_t size = sizeof(ast_ExplicitCastExpr) + extra;
   ast_ExplicitCastExpr* e2 = ast_context_Context_alloc(inst->c, size);
   e2->parent = e->parent;
   e2->inner = ast_Expr_instantiate(e->inner, inst);
   ast_TypeRef_instantiate(&e2->dest, &e->dest, inst->ref, inst->template_name);
   return ((ast_Expr*)(e2));
}

static ast_Expr* ast_ExplicitCastExpr_getInner(const ast_ExplicitCastExpr* e)
{
   return e->inner;
}

static ast_Expr** ast_ExplicitCastExpr_getInner2(ast_ExplicitCastExpr* e)
{
   return &e->inner;
}

static ast_TypeRef* ast_ExplicitCastExpr_getTypeRef(ast_ExplicitCastExpr* e)
{
   return &e->dest;
}

static void ast_ExplicitCastExpr_print(const ast_ExplicitCastExpr* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add(out, " -> ");
   ast_TypeRef_print(&e->dest, out, true);
   string_buffer_Buf_add1(out, '\n');
   ast_Expr_print(e->inner, out, indent + 1);
}

static ast_ImplicitCastExpr* ast_ImplicitCastExpr_create(ast_context_Context* c, src_loc_SrcLoc loc, ast_ImplicitCastKind kind, ast_Expr* inner)
{
   ast_ImplicitCastExpr* e = ast_context_Context_alloc(c, sizeof(ast_ImplicitCastExpr));
   ast_Expr_init(&e->parent, ast_ExprKind_ImplicitCast, loc, 0, 0, 0, ast_ValType_RValue);
   e->parent.parent.implicitCastBits.kind = kind;
   e->inner = inner;
   ast_Expr_copyConstantFlags(&e->parent, inner);
   switch (kind) {
   case ast_ImplicitCastKind_ArrayToPointerDecay:
      ast_Expr_copyValType(&e->parent, inner);
      e->parent.parent.exprBits.is_ctv = false;
      break;
   case ast_ImplicitCastKind_FunctionToPointerDecay:
      ast_Expr_copyValType(&e->parent, inner);
      break;
   case ast_ImplicitCastKind_LValueToRValue:
      e->parent.parent.exprBits.is_ctc = false;
      break;
   case ast_ImplicitCastKind_PointerToBoolean:
      ast_Expr_copyValType(&e->parent, inner);
      break;
   case ast_ImplicitCastKind_PointerToInteger:
      ast_Expr_copyValType(&e->parent, inner);
      break;
   }
   ast_Stats_addExpr(ast_ExprKind_ImplicitCast, sizeof(ast_ImplicitCastExpr));
   return e;
}

static ast_ImplicitCastKind ast_ImplicitCastExpr_getKind(const ast_ImplicitCastExpr* e)
{
   return ((ast_ImplicitCastKind)(e->parent.parent.implicitCastBits.kind));
}

static ast_Expr* ast_ImplicitCastExpr_getInner(const ast_ImplicitCastExpr* e)
{
   return e->inner;
}

static void ast_ImplicitCastExpr_print(const ast_ImplicitCastExpr* e, string_buffer_Buf* out, uint32_t indent)
{
   ast_Expr_printKind(&e->parent, out, indent);
   ast_Expr_printTypeBits(&e->parent, out);
   string_buffer_Buf_add1(out, ' ');
   string_buffer_Buf_color(out, ast_col_Calc);
   string_buffer_Buf_add(out, ast_implicitCastKind_names[ast_ImplicitCastExpr_getKind(e)]);
   string_buffer_Buf_add1(out, '\n');
   ast_Expr_print(e->inner, out, indent + 1);
}

static void ast_init(ast_context_Context* c, const char* names_start, uint32_t wordsize)
{
   ast_Stats_reset(&ast_stats);
   ast_PointerPool_init(&ast_g_pointers, c);
   ast_StringTypePool_init(&ast_g_string_types, c);
   ast_g_wordsize = wordsize;
   ast_builtinType_sizes[ast_BuiltinKind_ISize] = wordsize;
   ast_builtinType_sizes[ast_BuiltinKind_USize] = wordsize;
   ast_QualType_set(&ast_g_char, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_Char))));
   ast_QualType_set(&ast_g_u8, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_UInt8))));
   ast_QualType_set(&ast_g_u16, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_UInt16))));
   ast_QualType_set(&ast_g_u32, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_UInt32))));
   ast_QualType_set(&ast_g_u64, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_UInt64))));
   ast_QualType_set(&ast_g_i8, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_Int8))));
   ast_QualType_set(&ast_g_i16, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_Int16))));
   ast_QualType_set(&ast_g_i32, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_Int32))));
   ast_QualType_set(&ast_g_i64, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_Int64))));
   ast_QualType_set(&ast_g_f32, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_Float32))));
   ast_QualType_set(&ast_g_f64, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_Float64))));
   ast_QualType_set(&ast_g_isize, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_ISize))));
   ast_QualType_set(&ast_g_usize, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_USize))));
   ast_QualType_set(&ast_g_void, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_Void))));
   ast_QualType_set(&ast_g_bool, ((ast_Type*)(ast_BuiltinType_create(c, ast_BuiltinKind_Bool))));
   ast_Type* void_ptr = ast_getPointerType(ast_g_void);
   ast_QualType_set(&ast_g_void_ptr, void_ptr);
   ast_Type_setCanonicalType(void_ptr, ast_g_void_ptr);
   ast_g_names_start = names_start;
   ast_ast_count = 1;
   ast_ast_capacity = 0;
   ast_g_ast_list = NULL;
}

static void ast_deinit(bool print_stats)
{
   if (print_stats) ast_Stats_dump(&ast_stats);
   ast_g_names_start = NULL;
   ast_ast_count = 0;
   ast_ast_capacity = 0;
   free(((void*)(ast_g_ast_list)));
   ast_PointerPool_clear(&ast_g_pointers);
   ast_StringTypePool_clear(&ast_g_string_types);
}

static const char* ast_idx2name(uint32_t idx)
{
   if (idx) return ast_g_names_start + idx;

   return NULL;
}

static ast_Type* ast_getPointerType(ast_QualType inner)
{
   return ast_PointerPool_getPointer(&ast_g_pointers, inner);
}

static uint32_t ast_addAST(ast_AST* ast_)
{
   if (ast_ast_count >= ast_ast_capacity) {
      if (ast_ast_capacity == 0) ast_ast_capacity = 16;
      else ast_ast_capacity *= 2;
      void* buf = malloc(ast_ast_capacity * sizeof(ast_AST*));
      if (ast_g_ast_list) {
         void* old = ((void*)(ast_g_ast_list));
         memcpy(buf, old, ast_ast_count * sizeof(ast_AST*));
         free(old);
      }
      ast_g_ast_list = buf;
   }
   uint32_t idx = ast_ast_count;
   ast_g_ast_list[idx] = ast_;
   ast_ast_count++;
   return idx;
}

static uint32_t ast_ast2idx(const ast_AST* ast_)
{
   if (ast_) return ast_->idx;

   return 0;
}

static ast_AST* ast_idx2ast(uint32_t idx)
{
   if (idx == 0) return NULL;

   return ast_g_ast_list[idx];
}

static void ast_Stats_reset(ast_Stats* s)
{
   memset(s, 0, sizeof(ast_Stats));
}

static void ast_Stats_addType(ast_TypeKind kind, uint32_t size)
{
   ast_stats.types[kind].count++;
   ast_stats.types[kind].size += size;
}

static void ast_Stats_addExpr(ast_ExprKind kind, uint32_t size)
{
   ast_stats.exprs[kind].count++;
   ast_stats.exprs[kind].size += size;
}

static void ast_Stats_addStmt(ast_StmtKind kind, uint32_t size)
{
   ast_stats.stmts[kind].count++;
   ast_stats.stmts[kind].size += size;
}

static void ast_Stats_addDecl(ast_DeclKind kind, uint32_t size)
{
   ast_stats.decls[kind].count++;
   ast_stats.decls[kind].size += size;
}

static void ast_Stats_dump(const ast_Stats* s)
{
   printf("----------------------------\n");
   printf("--- Types ---\n");
   uint32_t typesTotal = 0;
   uint32_t typesCount = 0;
   for (uint32_t i = 0; i <= 7; i++) {
      const ast_Stat* ss = &s->types[i];
      printf("  %20s  %6u  %7u\n", ast_typeKind_names[i], ss->count, ss->size);
      typesCount += ss->count;
      typesTotal += ss->size;
   }
   printf("  %20s  %6u  %7u\n", "total", typesCount, typesTotal);
   printf("--- Expressions ---\n");
   uint32_t exprTotal = 0;
   uint32_t exprCount = 0;
   for (uint32_t i = 0; i <= 20; i++) {
      const ast_Stat* ss = &s->exprs[i];
      printf("  %20s  %6u  %7u\n", ast_exprKind_names[i], ss->count, ss->size);
      exprCount += ss->count;
      exprTotal += ss->size;
   }
   printf("  %20s  %6u  %7u\n", "total", exprCount, exprTotal);
   printf("--- Statements ---\n");
   uint32_t stmtTotal = 0;
   uint32_t stmtCount = 0;
   for (uint32_t i = 0; i <= 16; i++) {
      const ast_Stat* ss = &s->stmts[i];
      printf("  %20s  %6u  %7u\n", ast_stmtKind_names[i], ss->count, ss->size);
      stmtCount += ss->count;
      stmtTotal += ss->size;
   }
   printf("  %20s  %6u  %7u\n", "total", stmtCount, stmtTotal);
   printf("--- Decls ---\n");
   uint32_t declTotal = 0;
   uint32_t declCount = 0;
   for (uint32_t i = 0; i <= 8; i++) {
      const ast_Stat* ss = &s->decls[i];
      printf("  %20s  %6u  %7u\n", ast_declKind_names[i], ss->count, ss->size);
      declCount += ss->count;
      declTotal += ss->size;
   }
   printf("  %20s  %6u  %7u\n", "total", declCount, declTotal);
   printf("--- Total ---\n");
   uint32_t totalCount = typesCount + exprCount + stmtCount + declCount;
   uint32_t totalSize = typesTotal + exprTotal + stmtTotal + declTotal;
   printf("  %20s  %6u  %7u\n", "objects", totalCount, totalSize);
   printf("----------------------------\n");
}

static void ast_Type_init(ast_Type* t, ast_TypeKind k)
{
   t->bits = 0;
   t->typeBits.kind = k;
   t->ptr_pool_idx = 0;
   t->canonicalType.ptr = 0;
}

static ast_TypeKind ast_Type_getKind(const ast_Type* t)
{
   return ((ast_TypeKind)(t->typeBits.kind));
}

static bool ast_Type_hasCanonicalType(const ast_Type* t)
{
   return t->canonicalType.ptr != 0;
}

static ast_QualType ast_Type_getCanonicalType(const ast_Type* t)
{
   return t->canonicalType;
}

static void ast_Type_setCanonicalType(ast_Type* t, ast_QualType canon)
{
   t->canonicalType = canon;
}

static uint32_t ast_Type_getIndex(const ast_Type* t)
{
   return t->ptr_pool_idx;
}

static ast_ArrayType* ast_Type_asArray(ast_Type* t)
{
   if (ast_Type_getKind(t) == ast_TypeKind_Array) return ((ast_ArrayType*)(t));

   return NULL;
}

static bool ast_Type_isBuiltinType(const ast_Type* t)
{
   return (ast_Type_getKind(t) == ast_TypeKind_Builtin);
}

static bool ast_Type_isArrayType(const ast_Type* t)
{
   return (ast_Type_getKind(t) == ast_TypeKind_Array);
}

static bool ast_Type_isStructType(const ast_Type* t)
{
   return (ast_Type_getKind(t) == ast_TypeKind_Struct);
}

static bool ast_Type_isPointerType(const ast_Type* t)
{
   return (ast_Type_getKind(t) == ast_TypeKind_Pointer);
}

static bool ast_Type_isVoidType(const ast_Type* t)
{
   return t == ast_QualType_getTypeOrNil(&ast_g_void);
}

static void ast_Type_dump(const ast_Type* t)
{
   string_buffer_Buf* out = string_buffer_create(256, utils_useColor(), 2);
   ast_Type_print(t, out);
   string_buffer_Buf_color(out, color_Normal);
   printf("%s\n", string_buffer_Buf_data(out));
   string_buffer_Buf_free(out);
}

static uint32_t ast_Type_getAlignment(const ast_Type* t)
{
   switch (ast_Type_getKind(t)) {
   case ast_TypeKind_Builtin: {
      const ast_BuiltinType* bi = ((ast_BuiltinType*)(t));
      return ast_BuiltinType_getAlignment(bi);
   }
   case ast_TypeKind_Pointer:
      break;
   case ast_TypeKind_Array: {
      const ast_ArrayType* at = ((ast_ArrayType*)(t));
      ast_QualType elem = ast_ArrayType_getElemType(at);
      return ast_QualType_getAlignment(&elem);
   }
   case ast_TypeKind_Struct: {
      const ast_StructType* s = ((ast_StructType*)(t));
      const ast_StructTypeDecl* std = ast_StructType_getDecl(s);
      return ast_StructTypeDecl_getAlignment(std);
   }
   case ast_TypeKind_Enum: {
      const ast_EnumType* e = ((ast_EnumType*)(t));
      const ast_EnumTypeDecl* etd = ast_EnumType_getDecl(e);
      ast_QualType it = ast_EnumTypeDecl_getImplType(etd);
      return ast_Type_getAlignment(ast_QualType_getTypeOrNil(&it));
   }
   case ast_TypeKind_Function:
      break;
   case ast_TypeKind_Alias: {
      ast_QualType canon = ast_Type_getCanonicalType(t);
      return ast_QualType_getAlignment(&canon);
   }
   case ast_TypeKind_Module:
      return 0;
   }
   return ast_g_wordsize;
}

static uint32_t ast_Type_getSize(const ast_Type* t)
{
   switch (ast_Type_getKind(t)) {
   case ast_TypeKind_Builtin: {
      const ast_BuiltinType* bi = ((ast_BuiltinType*)(t));
      return ast_BuiltinType_getAlignment(bi);
   }
   case ast_TypeKind_Pointer:
      break;
   case ast_TypeKind_Array: {
      const ast_ArrayType* at = ((ast_ArrayType*)(t));
      ast_QualType elem = ast_ArrayType_getElemType(at);
      return ast_ArrayType_getSize(at) * ast_QualType_getSize(&elem);
   }
   case ast_TypeKind_Struct: {
      const ast_StructType* s = ((ast_StructType*)(t));
      const ast_StructTypeDecl* std = ast_StructType_getDecl(s);
      return ast_StructTypeDecl_getSize(std);
   }
   case ast_TypeKind_Enum: {
      const ast_EnumType* e = ((ast_EnumType*)(t));
      const ast_EnumTypeDecl* etd = ast_EnumType_getDecl(e);
      ast_QualType it = ast_EnumTypeDecl_getImplType(etd);
      return ast_Type_getSize(ast_QualType_getTypeOrNil(&it));
   }
   case ast_TypeKind_Function:
      break;
   case ast_TypeKind_Alias: {
      ast_QualType canon = ast_Type_getCanonicalType(t);
      return ast_QualType_getAlignment(&canon);
   }
   case ast_TypeKind_Module:
      return 0;
   }
   return ast_g_wordsize;
}

static void ast_Type_print(const ast_Type* t, string_buffer_Buf* out)
{
   string_buffer_Buf_color(out, ast_col_Type);
   switch (ast_Type_getKind(t)) {
   case ast_TypeKind_Builtin:
      ast_BuiltinType_print(((ast_BuiltinType*)(t)), out);
      break;
   case ast_TypeKind_Pointer:
      ast_PointerType_print(((ast_PointerType*)(t)), out);
      break;
   case ast_TypeKind_Array:
      ast_ArrayType_print(((ast_ArrayType*)(t)), out);
      break;
   case ast_TypeKind_Struct:
      ast_StructType_print(((ast_StructType*)(t)), out);
      break;
   case ast_TypeKind_Enum:
      ast_EnumType_print(((ast_EnumType*)(t)), out);
      break;
   case ast_TypeKind_Function:
      ast_FunctionType_print(((ast_FunctionType*)(t)), out);
      break;
   case ast_TypeKind_Alias:
      ast_AliasType_print(((ast_AliasType*)(t)), out);
      break;
   case ast_TypeKind_Module:
      ast_ModuleType_print(((ast_ModuleType*)(t)), out);
      break;
   }
}

static void ast_Type_fullPrint(const ast_Type* t, string_buffer_Buf* out, uint32_t indent)
{
   switch (ast_Type_getKind(t)) {
   case ast_TypeKind_Builtin:
      ast_BuiltinType_fullPrint(((ast_BuiltinType*)(t)), out, indent);
      break;
   case ast_TypeKind_Pointer:
      ast_PointerType_fullPrint(((ast_PointerType*)(t)), out, indent);
      break;
   case ast_TypeKind_Array:
      ast_ArrayType_fullPrint(((ast_ArrayType*)(t)), out, indent);
      break;
   case ast_TypeKind_Struct:
      ast_StructType_fullPrint(((ast_StructType*)(t)), out, indent);
      break;
   case ast_TypeKind_Enum:
      ast_EnumType_fullPrint(((ast_EnumType*)(t)), out, indent);
      break;
   case ast_TypeKind_Function:
      ast_FunctionType_fullPrint(((ast_FunctionType*)(t)), out, indent);
      break;
   case ast_TypeKind_Alias:
      ast_AliasType_fullPrint(((ast_AliasType*)(t)), out, indent);
      break;
   case ast_TypeKind_Module:
      ast_ModuleType_fullPrint(((ast_ModuleType*)(t)), out, indent);
      break;
   }
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_add(out, "canonical=");
   ast_Type* canon = ast_QualType_getTypeOrNil(&t->canonicalType);
   if (canon) {
      if (canon == t) {
         string_buffer_Buf_add(out, "this\n");
      } else {
         ast_Type_fullPrint(canon, out, indent + 1);
      }
   } else {
      string_buffer_Buf_add(out, "NIL\n");
   }
}

static ast_QualType ast_QualType_init(ast_Type* t)
{
   ast_QualType qt = { ((size_t)(t)) };
   return qt;
}

static void ast_QualType_set(ast_QualType* qt, ast_Type* t)
{
   qt->ptr = ((size_t)(t));
}

static void ast_QualType_setConst(ast_QualType* qt)
{
   qt->ptr |= ast_QualType_Const;
}

static bool ast_QualType_isConst(ast_QualType* qt)
{
   return ((qt->ptr & ast_QualType_Const)) != 0;
}

static bool ast_QualType_isVolatile(ast_QualType* qt)
{
   return ((qt->ptr & ast_QualType_Volatile)) != 0;
}

static void ast_QualType_setVolatile(ast_QualType* qt)
{
   qt->ptr |= ast_QualType_Volatile;
}

static uint32_t ast_QualType_getQuals(const ast_QualType* qt)
{
   return (qt->ptr & ast_QualType_Mask);
}

static void ast_QualType_copyQuals(ast_QualType* qt, ast_QualType other)
{
   qt->ptr &= ~ast_QualType_Mask;
   qt->ptr |= ((other.ptr & ast_QualType_Mask));
}

static void ast_QualType_clearQuals(ast_QualType* qt)
{
   qt->ptr &= ~ast_QualType_Mask;
}

static bool ast_QualType_isConstant(const ast_QualType* qt)
{
   ast_QualType canon = ast_QualType_getCanonicalType(qt);
   const ast_Type* t = ast_QualType_getTypeOrNil(&canon);
   if (ast_QualType_isConst(&canon)) return true;

   if (ast_Type_getKind(t) == ast_TypeKind_Array) {
      const ast_ArrayType* at = ((ast_ArrayType*)(t));
      canon = ast_ArrayType_getElemType(at);
      return ast_QualType_isConstant(&canon);
   }
   return false;
}

static bool ast_QualType_isValid(const ast_QualType* qt)
{
   return qt->ptr != 0;
}

static bool ast_QualType_isInvalid(const ast_QualType* qt)
{
   return qt->ptr == 0;
}

static ast_Type* ast_QualType_getType(const ast_QualType* qt)
{
   size_t t = (qt->ptr & ~ast_QualType_Mask);
   assert(t);
   return ((ast_Type*)(t));
}

static ast_Type* ast_QualType_getTypeOrNil(const ast_QualType* qt)
{
   size_t temp = (qt->ptr & ~ast_QualType_Mask);
   return ((ast_Type*)(temp));
}

static bool ast_QualType_hasCanonicalType(const ast_QualType* qt)
{
   const ast_Type* t = ast_QualType_getType(qt);
   return t->canonicalType.ptr != 0;
}

static ast_QualType ast_QualType_getCanonicalType(const ast_QualType* qt)
{
   const ast_Type* t = ast_QualType_getType(qt);
   ast_QualType canon = t->canonicalType;
   ast_QualType_copyQuals(&canon, *qt);
   return canon;
}

static void ast_QualType_setCanonicalType(ast_QualType* qt, ast_QualType canon)
{
   ast_Type* t = ast_QualType_getType(qt);
   ast_Type_setCanonicalType(t, canon);
}

static ast_TypeKind ast_QualType_getKind(ast_QualType* qt)
{
   ast_Type* t = ast_QualType_getType(qt);
   return ast_Type_getKind(t);
}

static uint32_t ast_QualType_getIndex(ast_QualType* qt)
{
   const ast_Type* t = ast_QualType_getType(qt);
   return ast_Type_getIndex(t);
}

static uint32_t ast_QualType_getAlignment(ast_QualType* qt)
{
   const ast_Type* t = ast_QualType_getType(qt);
   return ast_Type_getAlignment(t);
}

static uint32_t ast_QualType_getSize(ast_QualType* qt)
{
   const ast_Type* t = ast_QualType_getType(qt);
   return ast_Type_getSize(t);
}

static bool ast_QualType_isBuiltinType(const ast_QualType* qt)
{
   return ast_Type_isBuiltinType(ast_QualType_getTypeOrNil(qt));
}

static bool ast_QualType_isArrayType(const ast_QualType* qt)
{
   return ast_Type_isArrayType(ast_QualType_getTypeOrNil(qt));
}

static bool ast_QualType_isStructType(const ast_QualType* qt)
{
   return ast_Type_isStructType(ast_QualType_getTypeOrNil(qt));
}

static bool ast_QualType_isPointerType(const ast_QualType* qt)
{
   return ast_Type_isPointerType(ast_QualType_getTypeOrNil(qt));
}

static bool ast_QualType_isVoidType(const ast_QualType* qt)
{
   return ast_Type_isVoidType(ast_QualType_getTypeOrNil(qt));
}

static ast_BuiltinType* ast_QualType_getBuiltinType(const ast_QualType* qt)
{
   return ((ast_BuiltinType*)(ast_QualType_getTypeOrNil(qt)));
}

static ast_BuiltinType* ast_QualType_getBuiltinTypeOrNil(const ast_QualType* qt)
{
   const ast_Type* t = ast_QualType_getTypeOrNil(qt);
   if ((t && ast_Type_getKind(t) == ast_TypeKind_Builtin)) return ((ast_BuiltinType*)(t));

   return NULL;
}

static ast_StructType* ast_QualType_getStructType(const ast_QualType* qt)
{
   return ((ast_StructType*)(ast_QualType_getTypeOrNil(qt)));
}

static ast_PointerType* ast_QualType_getPointerType(const ast_QualType* qt)
{
   return ((ast_PointerType*)(ast_QualType_getTypeOrNil(qt)));
}

static ast_ArrayType* ast_QualType_getArrayType(const ast_QualType* qt)
{
   return ((ast_ArrayType*)(ast_QualType_getTypeOrNil(qt)));
}

static ast_FunctionType* ast_QualType_getFunctionTypeOrNil(const ast_QualType* qt)
{
   const ast_Type* t = ast_QualType_getTypeOrNil(qt);
   if ((t && ast_Type_getKind(t) == ast_TypeKind_Function)) return ((ast_FunctionType*)(t));

   return NULL;
}

static ast_StructType* ast_QualType_getStructTypeOrNil(const ast_QualType* qt)
{
   const ast_Type* t = ast_QualType_getTypeOrNil(qt);
   if ((t && ast_Type_getKind(t) == ast_TypeKind_Struct)) return ((ast_StructType*)(t));

   return NULL;
}

static ast_PointerType* ast_QualType_getPointerTypeOrNil(const ast_QualType* qt)
{
   const ast_Type* t = ast_QualType_getTypeOrNil(qt);
   if ((t && ast_Type_getKind(t) == ast_TypeKind_Pointer)) return ((ast_PointerType*)(t));

   return NULL;
}

static ast_ArrayType* ast_QualType_getArrayTypeOrNil(const ast_QualType* qt)
{
   const ast_Type* t = ast_QualType_getTypeOrNil(qt);
   if ((t && ast_Type_getKind(t) == ast_TypeKind_Array)) return ((ast_ArrayType*)(t));

   return NULL;
}

static ast_EnumType* ast_QualType_getEnumTypeOrNil(const ast_QualType* qt)
{
   const ast_Type* t = ast_QualType_getTypeOrNil(qt);
   if (ast_Type_getKind(t) == ast_TypeKind_Enum) return ((ast_EnumType*)(t));

   return NULL;
}

static bool ast_QualType_needsCtvInit(const ast_QualType* qt)
{
   ast_QualType canon = ast_QualType_getCanonicalType(qt);
   const ast_Type* t = ast_QualType_getTypeOrNil(&canon);
   if (!t) {
      printf("MISSING CANONICAL\n");
      ast_QualType_dump_full(qt);
      return false;
   }
   assert(t);
   switch (ast_Type_getKind(t)) {
   case ast_TypeKind_Builtin:
      return true;
   case ast_TypeKind_Pointer:
      return false;
   case ast_TypeKind_Array:
      return true;
   case ast_TypeKind_Struct:
      return true;
   case ast_TypeKind_Enum:
      return true;
   case ast_TypeKind_Function:
      return false;
   case ast_TypeKind_Alias:
      return false;
   case ast_TypeKind_Module:
      return false;
   }
   return false;
}

static const char* ast_QualType_diagName(const ast_QualType* qt)
{
   static char msgs[128][4];
   static uint32_t msg_id = 0;
   char* msg = msgs[msg_id];
   msg_id = (msg_id + 1) % ARRAY_SIZE(msgs);
   string_buffer_Buf* buf = string_buffer_create_static(128, false, msg);
   ast_QualType_print(qt, buf);
   string_buffer_Buf_free(buf);
   return msg;
}

static void ast_QualType_dump(const ast_QualType* qt)
{
   string_buffer_Buf* out = string_buffer_create(512, utils_useColor(), 2);
   ast_Type* t = ast_QualType_getTypeOrNil(qt);
   if (t) {
      uint32_t quals = ast_QualType_getQuals(qt);
      if (quals) string_buffer_Buf_color(out, ast_col_Type);
      if ((quals & ast_QualType_Const)) string_buffer_Buf_add(out, "const ");
      if ((quals & ast_QualType_Volatile)) string_buffer_Buf_add(out, "volatile ");
      ast_Type_print(t, out);
   } else {
      string_buffer_Buf_add(out, "QualType(nil)\n");
   }
   string_buffer_Buf_color(out, color_Normal);
   puts(string_buffer_Buf_data(out));
   string_buffer_Buf_free(out);
}

static void ast_QualType_dump_full(const ast_QualType* qt)
{
   string_buffer_Buf* out = string_buffer_create(512, utils_useColor(), 1);
   ast_QualType_fullPrint(qt, out, 0);
   puts(string_buffer_Buf_data(out));
   string_buffer_Buf_free(out);
}

static void ast_QualType_printQuoted(const ast_QualType* qt, string_buffer_Buf* out)
{
   string_buffer_Buf_color(out, ast_col_Type);
   ast_QualType_print(qt, out);
   string_buffer_Buf_color(out, ast_col_Type);
}

static void ast_QualType_print(const ast_QualType* qt, string_buffer_Buf* out)
{
   ast_QualType_printInner(qt, out, true);
}

static void ast_QualType_printInner(const ast_QualType* qt, string_buffer_Buf* out, bool printCanon)
{
   const ast_Type* t = ast_QualType_getTypeOrNil(qt);
   if (t) {
      uint32_t quals = ast_QualType_getQuals(qt);
      if (quals) string_buffer_Buf_color(out, ast_col_Type);
      if ((quals & ast_QualType_Const)) string_buffer_Buf_add(out, "const ");
      if ((quals & ast_QualType_Volatile)) string_buffer_Buf_add(out, "volatile ");
      ast_Type_print(t, out);
      if (printCanon) {
         ast_QualType qt2 = ast_QualType_getCanonicalType(qt);
         const ast_Type* canon = ast_QualType_getTypeOrNil(&qt2);
         if ((canon && canon != t)) {
            string_buffer_Buf_add(out, " => ");
            ast_QualType_printInner(&qt2, out, false);
         }
      }
   } else {
      string_buffer_Buf_color(out, ast_col_Error);
      string_buffer_Buf_add(out, "??");
   }
}

static void ast_QualType_fullPrint(const ast_QualType* qt, string_buffer_Buf* out, uint32_t indent)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_add(out, "QualType");
   uint32_t quals = ast_QualType_getQuals(qt);
   if ((quals & ast_QualType_Const)) string_buffer_Buf_add(out, " const");
   if ((quals & ast_QualType_Volatile)) string_buffer_Buf_add(out, " volatile");
   const ast_Type* t = ast_QualType_getTypeOrNil(qt);
   if (t) {
      string_buffer_Buf_add1(out, '\n');
      ast_Type_fullPrint(t, out, indent + 1);
   } else string_buffer_Buf_add(out, " type=nil\n");
}

static ast_BuiltinType* ast_BuiltinType_create(ast_context_Context* c, ast_BuiltinKind kind)
{
   ast_BuiltinType* b = ast_context_Context_alloc(c, sizeof(ast_BuiltinType));
   ast_Type_init(&b->parent, ast_TypeKind_Builtin);
   b->parent.builtinTypeBits.kind = kind;
   ast_Type_setCanonicalType(&b->parent, ast_QualType_init(&b->parent));
   ast_Stats_addType(ast_TypeKind_Builtin, sizeof(ast_BuiltinType));
   return b;
}

static ast_BuiltinKind ast_BuiltinType_getKind(const ast_BuiltinType* b)
{
   return ((ast_BuiltinKind)(b->parent.builtinTypeBits.kind));
}

static const char* ast_BuiltinType_kind2str(const ast_BuiltinType* b)
{
   return ast_builtinType_names[ast_BuiltinType_getKind(b)];
}

static bool ast_BuiltinType_isPromotableIntegerType(const ast_BuiltinType* b)
{
   return ast_BuiltinType_promotable[ast_BuiltinType_getKind(b)];
}

static uint32_t ast_BuiltinType_getAlignment(const ast_BuiltinType* b)
{
   return ast_builtinType_sizes[ast_BuiltinType_getKind(b)];
}

static uint32_t ast_BuiltinType_getWidth(const ast_BuiltinType* b)
{
   return ast_builtinType_width[ast_BuiltinType_getKind(b)];
}

static void ast_BuiltinType_print(const ast_BuiltinType* b, string_buffer_Buf* out)
{
   string_buffer_Buf_add(out, ast_builtinType_names[ast_BuiltinType_getKind(b)]);
}

static void ast_BuiltinType_fullPrint(const ast_BuiltinType* t, string_buffer_Buf* out, uint32_t indent)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_print(out, "BuiltinType [%p] %s\n", t, ast_BuiltinType_kind2str(t));
}

static ast_PointerType* ast_PointerType_create(ast_context_Context* c, ast_QualType inner)
{
   ast_PointerType* t = ast_context_Context_alloc(c, sizeof(ast_PointerType));
   ast_Type_init(&t->parent, ast_TypeKind_Pointer);
   t->inner = inner;
   ast_Stats_addType(ast_TypeKind_Pointer, sizeof(ast_PointerType));
   return t;
}

static ast_Type* ast_PointerType_asType(ast_PointerType* t)
{
   return &t->parent;
}

static ast_QualType ast_PointerType_getInner(const ast_PointerType* t)
{
   return t->inner;
}

static void ast_PointerType_print(const ast_PointerType* t, string_buffer_Buf* out)
{
   ast_QualType_printInner(&t->inner, out, false);
   string_buffer_Buf_add1(out, '*');
}

static void ast_PointerType_fullPrint(const ast_PointerType* t, string_buffer_Buf* out, uint32_t indent)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_print(out, "PointerType [%p]\n", t);
   ast_QualType_fullPrint(&t->inner, out, indent + 1);
}

static ast_ArrayType* ast_ArrayType_create(ast_context_Context* c, ast_QualType elem, bool has_size, uint32_t size)
{
   ast_ArrayType* t = ast_context_Context_alloc(c, sizeof(ast_ArrayType));
   ast_Type_init(&t->parent, ast_TypeKind_Array);
   t->parent.arrayTypeBits.has_size = has_size;
   t->elem = elem;
   t->size = size;
   ast_Stats_addType(ast_TypeKind_Array, sizeof(ast_ArrayType));
   return t;
}

static ast_Type* ast_ArrayType_asType(ast_ArrayType* t)
{
   return &t->parent;
}

static ast_QualType ast_ArrayType_getElemType(const ast_ArrayType* t)
{
   return t->elem;
}

static uint32_t ast_ArrayType_hasSize(const ast_ArrayType* t)
{
   return t->parent.arrayTypeBits.has_size;
}

static uint32_t ast_ArrayType_getSize(const ast_ArrayType* t)
{
   return t->size;
}

static void ast_ArrayType_setSize(ast_ArrayType* t, uint32_t size)
{
   t->size = size;
}

static void ast_ArrayType_print(const ast_ArrayType* t, string_buffer_Buf* out)
{
   ast_QualType_printInner(&t->elem, out, false);
   string_buffer_Buf_add1(out, '[');
   string_buffer_Buf_print(out, "%u", t->size);
   string_buffer_Buf_add1(out, ']');
}

static void ast_ArrayType_fullPrint(const ast_ArrayType* t, string_buffer_Buf* out, uint32_t indent)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_print(out, "ArrayType [%p]", t);
   string_buffer_Buf_print(out, " size=%u", t->size);
   string_buffer_Buf_add1(out, '\n');
   ast_QualType_fullPrint(&t->elem, out, indent + 1);
}

static ast_StructType* ast_StructType_create(ast_context_Context* c, ast_StructTypeDecl* decl)
{
   ast_StructType* t = ast_context_Context_alloc(c, sizeof(ast_StructType));
   ast_Type_init(&t->parent, ast_TypeKind_Struct);
   t->decl = decl;
   ast_Stats_addType(ast_TypeKind_Struct, sizeof(ast_StructType));
   return t;
}

static ast_StructTypeDecl* ast_StructType_getDecl(const ast_StructType* t)
{
   return t->decl;
}

static ast_Type* ast_StructType_asType(ast_StructType* t)
{
   return &t->parent;
}

static void ast_StructType_print(const ast_StructType* t, string_buffer_Buf* out)
{
   string_buffer_Buf_add(out, "(struct)");
   if (ast_StructTypeDecl_isGlobal(t->decl)) {
      string_buffer_Buf_add(out, ast_Decl_getModuleName(&t->decl->parent));
      string_buffer_Buf_add1(out, '.');
   }
   const char* name = ast_Decl_getName(&t->decl->parent);
   if (name) string_buffer_Buf_add(out, name);
   else string_buffer_Buf_add(out, "<anonymous>");
}

static void ast_StructType_fullPrint(const ast_StructType* t, string_buffer_Buf* out, uint32_t indent)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_print(out, "StructType [%p] %s\n", t, ast_Decl_getName(ast_StructTypeDecl_asDecl(t->decl)));
}

static ast_EnumType* ast_EnumType_create(ast_context_Context* c, ast_EnumTypeDecl* decl)
{
   ast_EnumType* t = ast_context_Context_alloc(c, sizeof(ast_EnumType));
   ast_Type_init(&t->parent, ast_TypeKind_Enum);
   t->decl = decl;
   ast_Stats_addType(ast_TypeKind_Enum, sizeof(ast_EnumType));
   ast_Type_setCanonicalType(&t->parent, ast_QualType_init(((ast_Type*)(t))));
   return t;
}

static ast_EnumTypeDecl* ast_EnumType_getDecl(const ast_EnumType* t)
{
   return t->decl;
}

static ast_Type* ast_EnumType_asType(ast_EnumType* t)
{
   return &t->parent;
}

static const char* ast_EnumType_getName(const ast_EnumType* t)
{
   return ast_Decl_getName(&t->decl->parent);
}

static void ast_EnumType_print(const ast_EnumType* t, string_buffer_Buf* out)
{
   string_buffer_Buf_add(out, "(enum)");
   string_buffer_Buf_add(out, ast_Decl_getModuleName(&t->decl->parent));
   string_buffer_Buf_add1(out, '.');
   string_buffer_Buf_add(out, ast_Decl_getName(&t->decl->parent));
}

static void ast_EnumType_fullPrint(const ast_EnumType* t, string_buffer_Buf* out, uint32_t indent)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_print(out, "EnumType [%p] %s\n", t, ast_Decl_getName(ast_EnumTypeDecl_asDecl(t->decl)));
}

static ast_FunctionType* ast_FunctionType_create(ast_context_Context* c, ast_FunctionDecl* decl)
{
   ast_FunctionType* t = ast_context_Context_alloc(c, sizeof(ast_FunctionType));
   ast_Type_init(&t->parent, ast_TypeKind_Function);
   t->decl = NULL;
   t->decl = decl;
   ast_Type_setCanonicalType(&t->parent, ast_QualType_init(&t->parent));
   ast_Stats_addType(ast_TypeKind_Function, sizeof(ast_FunctionType));
   return t;
}

static ast_FunctionDecl* ast_FunctionType_getDecl(const ast_FunctionType* t)
{
   return t->decl;
}

static ast_Type* ast_FunctionType_asType(ast_FunctionType* t)
{
   return &t->parent;
}

static void ast_FunctionType_print(const ast_FunctionType* t, string_buffer_Buf* out)
{
   ast_FunctionDecl_printType(t->decl, out);
}

static void ast_FunctionType_fullPrint(const ast_FunctionType* t, string_buffer_Buf* out, uint32_t indent)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_print(out, "FunctionType [%p] %s\n", t, ast_Decl_getName(ast_FunctionDecl_asDecl(t->decl)));
}

static ast_AliasType* ast_AliasType_create(ast_context_Context* c, ast_AliasTypeDecl* decl)
{
   ast_AliasType* t = ast_context_Context_alloc(c, sizeof(ast_AliasType));
   ast_Type_init(&t->parent, ast_TypeKind_Alias);
   t->decl = decl;
   ast_Stats_addType(ast_TypeKind_Alias, sizeof(ast_AliasType));
   return t;
}

static ast_AliasTypeDecl* ast_AliasType_getDecl(const ast_AliasType* t)
{
   return t->decl;
}

static void ast_AliasType_print(const ast_AliasType* t, string_buffer_Buf* out)
{
   string_buffer_Buf_add(out, "(alias");
   string_buffer_Buf_add(out, ast_Decl_getModuleName(&t->decl->parent));
   string_buffer_Buf_add1(out, '.');
   string_buffer_Buf_add(out, ast_Decl_getName(&t->decl->parent));
}

static void ast_AliasType_fullPrint(const ast_AliasType* t, string_buffer_Buf* out, uint32_t indent)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_print(out, "AliasType [%p] %s\n", t, ast_Decl_getName(ast_AliasTypeDecl_asDecl(t->decl)));
}

static ast_ModuleType* ast_ModuleType_create(ast_context_Context* c, ast_Module* mod)
{
   ast_ModuleType* t = ast_context_Context_alloc(c, sizeof(ast_ModuleType));
   ast_Type_init(&t->parent, ast_TypeKind_Module);
   t->mod = mod;
   ast_Type_setCanonicalType(&t->parent, ast_QualType_init(&t->parent));
   ast_Stats_addType(ast_TypeKind_Module, sizeof(ast_ModuleType));
   return t;
}

static ast_Type* ast_ModuleType_asType(ast_ModuleType* t)
{
   return &t->parent;
}

static ast_Module* ast_ModuleType_getModule(const ast_ModuleType* t)
{
   return t->mod;
}

static void ast_ModuleType_print(const ast_ModuleType* t, string_buffer_Buf* out)
{
   string_buffer_Buf_print(out, "Module %s", ast_Module_getName(t->mod));
}

static void ast_ModuleType_fullPrint(const ast_ModuleType* t, string_buffer_Buf* out, uint32_t indent)
{
   string_buffer_Buf_indent(out, indent);
   string_buffer_Buf_print(out, "ModuleType %s\n", ast_Module_getName(t->mod));
}

static const char* ast_Ref_getName(const ast_Ref* r)
{
   return ast_idx2name(r->name_idx);
}

static void ast_TypeRefHolder_init(ast_TypeRefHolder* h)
{
   ast_TypeRef* r = ((ast_TypeRef*)(&h->ref));
   r->flagBits = 0;
   r->dest = 0;
}

static uint32_t ast_TypeRefHolder_getExtraSize(const ast_TypeRefHolder* h)
{
   ast_TypeRef* r = ((ast_TypeRef*)(&h->ref));
   return ast_TypeRef_getExtraSize(r);
}

static void ast_TypeRefHolder_setQualifiers(ast_TypeRefHolder* h, uint32_t qualifiers)
{
   ast_TypeRef* r = ((ast_TypeRef*)(&h->ref));
   if ((qualifiers & ast_QualType_Volatile)) r->flags.is_volatile = 1;
   if ((qualifiers & ast_QualType_Const)) r->flags.is_const = 1;
}

static void ast_TypeRefHolder_addPointer(ast_TypeRefHolder* h)
{
   ast_TypeRef* r = ((ast_TypeRef*)(&h->ref));
   assert(r->flags.num_ptrs != 3);
   r->flags.num_ptrs++;
}

static void ast_TypeRefHolder_setIncrArray(ast_TypeRefHolder* h)
{
   ast_TypeRef* r = ((ast_TypeRef*)(&h->ref));
   r->flags.incr_array = 1;
}

static uint32_t ast_TypeRefHolder_getNumArrays(const ast_TypeRefHolder* h)
{
   ast_TypeRef* r = ((ast_TypeRef*)(&h->ref));
   return ast_TypeRef_getNumArrays(r);
}

static void ast_TypeRefHolder_addArray(ast_TypeRefHolder* h, ast_Expr* array)
{
   ast_TypeRef* r = ((ast_TypeRef*)(&h->ref));
   assert(r->flags.num_arrays != 3);
   h->arrays[r->flags.num_arrays] = array;
   r->flags.num_arrays++;
}

static void ast_TypeRefHolder_setBuiltin(ast_TypeRefHolder* h, ast_BuiltinKind kind)
{
   ast_TypeRef* r = ((ast_TypeRef*)(&h->ref));
   r->flags.builtin_kind = kind;
}

static void ast_TypeRefHolder_setUser(ast_TypeRefHolder* h, src_loc_SrcLoc loc, uint32_t name_idx)
{
   ast_TypeRef* r = ((ast_TypeRef*)(&h->ref));
   r->flags.is_user = 1;
   h->user.loc = loc;
   h->user.name_idx = name_idx;
   h->user.decl = NULL;
}

static void ast_TypeRefHolder_setPrefix(ast_TypeRefHolder* h, src_loc_SrcLoc loc, uint32_t name_idx)
{
   ast_TypeRef* r = ((ast_TypeRef*)(&h->ref));
   h->prefix = h->user;
   r->flags.has_prefix = 1;
   h->user.loc = loc;
   h->user.name_idx = name_idx;
   h->user.decl = NULL;
}

static void ast_TypeRefHolder_fill(const ast_TypeRefHolder* h, ast_TypeRef* dest)
{
   const ast_TypeRef* r = ((ast_TypeRef*)(&h->ref));
   *dest = *r;
   if (ast_TypeRef_isUser(r)) {
      dest->refs[0] = r->refs[0];
      if (ast_TypeRef_hasPrefix(r)) {
         dest->refs[1] = r->refs[1];
      }
   }
   for (uint32_t i = 0; i < r->flags.num_arrays; i++) {
      ast_Expr** a = ast_TypeRef_getArray2(dest, i);
      *a = h->arrays[i];
   }
}

static void ast_TypeRefHolder_dump(const ast_TypeRefHolder* h)
{
   const ast_TypeRef* r = ((ast_TypeRef*)(&h->ref));
   string_buffer_Buf* out = string_buffer_create(128, utils_useColor(), 2);
   ast_TypeRef_print(r, out, false);
   for (uint32_t i = 0; i < ast_TypeRef_getNumArrays(r); i++) {
      string_buffer_Buf_add(out, "[");
      ast_Expr_printLiteral(h->arrays[i], out);
      string_buffer_Buf_add(out, "]");
   }
   string_buffer_Buf_color(out, ast_col_Normal);
   puts(string_buffer_Buf_data(out));
   string_buffer_Buf_free(out);
}

static bool ast_TypeRef_matchesTemplate(const ast_TypeRef* r, uint32_t template_arg)
{
   if ((!r->flags.is_user || r->flags.has_prefix)) return false;

   return r->refs[0].name_idx == template_arg;
}

static void ast_TypeRef_initRef0(ast_TypeRef* r, const ast_TypeRef* r2)
{
   r->flagBits = r2->flagBits;
   r->refs[0] = r2->refs[0];
}

static void ast_TypeRef_instantiate(ast_TypeRef* r, const ast_TypeRef* r1, const ast_TypeRef* r2, uint32_t template_arg)
{
   if (ast_TypeRef_matchesTemplate(r1, template_arg)) {
      r->flagBits = r2->flagBits;
      if (r2->flags.is_user) {
         r->refs[0].name_idx = r2->refs[0].name_idx;
         if (r2->flags.has_prefix) {
            r->refs[1].name_idx = r2->refs[1].name_idx;
         }
      }
   } else {
      memcpy(r, r1, sizeof(ast_TypeRef) + ast_TypeRef_getExtraSize(r1));
   }
}

static void ast_TypeRef_setDest(ast_TypeRef* r, uint32_t dest)
{
   r->dest = dest;
}

static uint32_t ast_TypeRef_getDest(const ast_TypeRef* r)
{
   return r->dest;
}

static uint32_t ast_TypeRef_getExtraSize(const ast_TypeRef* r)
{
   uint32_t numrefs = r->flags.is_user + r->flags.has_prefix;
   uint32_t extra = numrefs * sizeof(ast_Ref);
   extra += r->flags.num_arrays * sizeof(ast_Expr*);
   return extra;
}

static uint32_t ast_TypeRef_getMaxSizeNoArray(void)
{
   return sizeof(ast_TypeRef) + 2 * sizeof(ast_Ref);
}

static void* ast_TypeRef_getPointerAfter(const ast_TypeRef* r)
{
   uint8_t* ptr = ((uint8_t*)(r)) + sizeof(ast_TypeRef) + ast_TypeRef_getExtraSize(r);
   return ptr;
}

static bool ast_TypeRef_isConst(const ast_TypeRef* r)
{
   return r->flags.is_const;
}

static bool ast_TypeRef_isVolatile(const ast_TypeRef* r)
{
   return r->flags.is_volatile;
}

static bool ast_TypeRef_isUser(const ast_TypeRef* r)
{
   return r->flags.is_user;
}

static bool ast_TypeRef_hasPrefix(const ast_TypeRef* r)
{
   return r->flags.has_prefix;
}

static bool ast_TypeRef_isIncrArray(const ast_TypeRef* r)
{
   return r->flags.incr_array;
}

static bool ast_TypeRef_isPointerTo(const ast_TypeRef* r, uint32_t ptr_idx)
{
   if ((r->dest != ptr_idx || ptr_idx == 0)) return false;

   return (((r->flags.num_ptrs == 1 && r->flags.num_arrays == 0) && r->flags.is_user));
}

static ast_BuiltinKind ast_TypeRef_getBuiltinKind(const ast_TypeRef* r)
{
   return ((ast_BuiltinKind)(r->flags.builtin_kind));
}

static uint32_t ast_TypeRef_getNumPointers(const ast_TypeRef* r)
{
   return r->flags.num_ptrs;
}

static const ast_Ref* ast_TypeRef_getUser(const ast_TypeRef* r)
{
   if (r->flags.is_user) return &r->refs[0];

   return NULL;
}

static const ast_Ref* ast_TypeRef_getPrefix(const ast_TypeRef* r)
{
   if (r->flags.has_prefix) return &r->refs[1];

   return NULL;
}

static void ast_TypeRef_setPrefix(ast_TypeRef* r, ast_Decl* d)
{
   r->refs[1].decl = d;
}

static void ast_TypeRef_setUser(ast_TypeRef* r, ast_Decl* d)
{
   r->refs[0].decl = d;
}

static uint32_t ast_TypeRef_getNumArrays(const ast_TypeRef* r)
{
   return r->flags.num_arrays;
}

static ast_Expr* ast_TypeRef_getArray(const ast_TypeRef* r, uint32_t idx)
{
   const uint32_t numrefs = r->flags.is_user + r->flags.has_prefix;
   const uint8_t* ptr = ((uint8_t*)(r->refs)) + numrefs * sizeof(ast_Ref);
   ast_Expr** arrays = ((ast_Expr**)(ptr));
   return arrays[idx];
}

static ast_Expr** ast_TypeRef_getArray2(ast_TypeRef* r, uint32_t idx)
{
   const uint32_t numrefs = r->flags.is_user + r->flags.has_prefix;
   const uint8_t* ptr = ((uint8_t*)(r->refs)) + numrefs * sizeof(ast_Ref);
   ast_Expr** arrays = ((ast_Expr**)(ptr));
   return &arrays[idx];
}

static void ast_TypeRef_print(const ast_TypeRef* r, string_buffer_Buf* out, bool filled)
{
   string_buffer_Buf_color(out, ast_col_Error);
   if (ast_TypeRef_isConst(r)) string_buffer_Buf_add(out, "const ");
   if (ast_TypeRef_isVolatile(r)) string_buffer_Buf_add(out, "volatile ");
   if (r->flags.is_user) {
      if (r->flags.has_prefix) {
         string_buffer_Buf_add(out, ast_idx2name(r->refs[1].name_idx));
         string_buffer_Buf_add1(out, '.');
      }
      string_buffer_Buf_add(out, ast_idx2name(r->refs[0].name_idx));
   } else {
      string_buffer_Buf_add(out, ast_builtinType_names[r->flags.builtin_kind]);
   }
   for (uint32_t i = 0; i < r->flags.num_ptrs; i++) string_buffer_Buf_add1(out, '*');
   if (r->flags.incr_array) {
      string_buffer_Buf_add(out, "[+]");
   }
   if (filled) {
      for (uint32_t i = 0; i < r->flags.num_arrays; i++) {
         string_buffer_Buf_add1(out, '[');
         const ast_Expr* a = ast_TypeRef_getArray(r, i);
         if (a) ast_Expr_printLiteral(a, out);
         string_buffer_Buf_add1(out, ']');
      }
   }
}

static void ast_TypeRef_dump(const ast_TypeRef* r)
{
   string_buffer_Buf* out = string_buffer_create(128, utils_useColor(), 2);
   ast_TypeRef_print(r, out, true);
   string_buffer_Buf_color(out, ast_col_Normal);
   puts(string_buffer_Buf_data(out));
   string_buffer_Buf_free(out);
}

static ast_AST* ast_AST_create(const char* filename, ast_Module* mod)
{
   ast_AST* a = calloc(1, sizeof(ast_AST));
   a->filename = filename;
   a->mod = mod;
   a->idx = ast_addAST(a);
   ast_ImportDeclList_init(&a->imports);
   ast_DeclList_init(&a->types, 0);
   ast_DeclList_init(&a->variables, 4);
   ast_FunctionDeclList_init(&a->functions);
   ast_DeclList_init(&a->static_asserts, 0);
   return a;
}

static void ast_AST_free(ast_AST* a)
{
   ast_ImportDeclList_free(&a->imports);
   ast_DeclList_free(&a->types);
   ast_DeclList_free(&a->variables);
   ast_FunctionDeclList_free(&a->functions);
   ast_DeclList_free(&a->static_asserts);
   if (a->attrs) attr_table_Table_free(a->attrs);
   free(a);
}

static const char* ast_AST_getFilename(const ast_AST* a)
{
   return a->filename;
}

static uint32_t ast_AST_getIdx(const ast_AST* a)
{
   return a->idx;
}

static uint32_t ast_AST_getNameIdx(const ast_AST* a)
{
   return ast_Module_getNameIdx(a->mod);
}

static void ast_AST_setPtr(ast_AST* a, void* ptr)
{
   a->ptr = ptr;
}

static void* ast_AST_getPtr(const ast_AST* a)
{
   return a->ptr;
}

static ast_Module* ast_AST_getMod(const ast_AST* a)
{
   return a->mod;
}

static void ast_AST_addImport(ast_AST* a, ast_ImportDecl* d)
{
   ast_ImportDeclList_add(&a->imports, d);
}

static ast_ImportDecl* ast_AST_findImport(const ast_AST* a, uint32_t name)
{
   ast_ImportDecl** imports = ast_ImportDeclList_getDecls(&a->imports);
   for (uint32_t i = 1; i < ast_ImportDeclList_size(&a->imports); i++) {
      ast_ImportDecl* d = imports[i];
      if (ast_Decl_getNameIdx(ast_ImportDecl_asDecl(d)) == name) return d;

   }
   return NULL;
}

static void ast_AST_addFunc(ast_AST* a, ast_FunctionDecl* d)
{
   ast_FunctionDeclList_add(&a->functions, d);
}

static void ast_AST_addTypeDecl(ast_AST* a, ast_Decl* d)
{
   ast_DeclList_add(&a->types, d);
}

static void ast_AST_addVarDecl(ast_AST* a, ast_Decl* d)
{
   ast_DeclList_add(&a->variables, d);
}

static void ast_AST_addStaticAssert(ast_AST* a, ast_StaticAssertDecl* d)
{
   ast_DeclList_add(&a->static_asserts, ast_StaticAssertDecl_asDecl(d));
}

static void ast_AST_visitImports(const ast_AST* a, ast_ImportVisitor visitor, void* arg)
{
   ast_ImportDecl** imports = ast_ImportDeclList_getDecls(&a->imports);
   for (uint32_t i = 1; i < ast_ImportDeclList_size(&a->imports); i++) {
      visitor(arg, imports[i]);
   }
}

static const ast_ImportDeclList* ast_AST_getImports(const ast_AST* a)
{
   return &a->imports;
}

static void ast_AST_visitStructFunctions(const ast_AST* a, ast_FunctionVisitor visitor, void* arg)
{
   ast_FunctionDecl** functions = ast_FunctionDeclList_getDecls(&a->functions);
   for (uint32_t i = 0; i < ast_FunctionDeclList_size(&a->functions); i++) {
      ast_FunctionDecl* d = functions[i];
      if (ast_FunctionDecl_hasPrefix(d)) visitor(arg, d);
   }
}

static void ast_AST_visitFunctions(const ast_AST* a, ast_FunctionVisitor visitor, void* arg)
{
   ast_FunctionDecl** functions = ast_FunctionDeclList_getDecls(&a->functions);
   for (uint32_t i = 0; i < ast_FunctionDeclList_size(&a->functions); i++) {
      ast_FunctionDecl* d = functions[i];
      visitor(arg, d);
   }
}

static void ast_AST_visitTypeDecls(const ast_AST* a, ast_TypeDeclVisitor visitor, void* arg)
{
   ast_Decl** types = ast_DeclList_getDecls(&a->types);
   for (uint32_t i = 0; i < ast_DeclList_size(&a->types); i++) {
      visitor(arg, types[i]);
   }
}

static void ast_AST_visitVarDecls(const ast_AST* a, ast_VarDeclVisitor visitor, void* arg)
{
   ast_Decl** variables = ast_DeclList_getDecls(&a->variables);
   for (uint32_t i = 0; i < ast_DeclList_size(&a->variables); i++) {
      visitor(arg, ((ast_VarDecl*)(variables[i])));
   }
}

static void ast_AST_visitStaticAsserts(const ast_AST* a, ast_DeclVisitor visitor, void* arg)
{
   ast_Decl** asserts = ast_DeclList_getDecls(&a->static_asserts);
   for (uint32_t i = 0; i < ast_DeclList_size(&a->static_asserts); i++) {
      visitor(arg, asserts[i]);
   }
}

static void ast_AST_visitDecls(const ast_AST* a, ast_DeclVisitor visitor, void* arg)
{
   ast_ImportDecl** imports = ast_ImportDeclList_getDecls(&a->imports);
   for (uint32_t i = 0; i < ast_ImportDeclList_size(&a->imports); i++) {
      visitor(arg, ((ast_Decl*)(imports[i])));
   }
   ast_Decl** types = ast_DeclList_getDecls(&a->types);
   for (uint32_t i = 0; i < ast_DeclList_size(&a->types); i++) {
      visitor(arg, types[i]);
   }
   ast_Decl** variables = ast_DeclList_getDecls(&a->variables);
   for (uint32_t i = 0; i < ast_DeclList_size(&a->variables); i++) {
      visitor(arg, variables[i]);
   }
   ast_FunctionDecl** functions = ast_FunctionDeclList_getDecls(&a->functions);
   for (uint32_t i = 0; i < ast_FunctionDeclList_size(&a->functions); i++) {
      ast_FunctionDecl* d = functions[i];
      visitor(arg, ((ast_Decl*)(d)));
   }
}

static ast_Decl* ast_AST_findType(const ast_AST* a, uint32_t name_idx)
{
   ast_Decl** types = ast_DeclList_getDecls(&a->types);
   for (uint32_t i = 0; i < ast_DeclList_size(&a->types); i++) {
      ast_Decl* d = types[i];
      if (ast_Decl_getNameIdx(d) == name_idx) return d;

   }
   return NULL;
}

static void ast_AST_storeAttr(ast_AST* a, ast_Decl* d, attr_AttrKind kind, const attr_Value* value)
{
   if (!a->attrs) a->attrs = attr_table_create();
   attr_table_Table_add(a->attrs, d, kind, value);
}

static const attr_Value* ast_AST_getAttr(const ast_AST* a, const ast_Decl* d, attr_AttrKind kind)
{
   if (a->attrs) return attr_table_Table_find(a->attrs, d, kind);

   return NULL;
}

static void ast_AST_info(const ast_AST* a, string_buffer_Buf* out)
{
   string_buffer_Buf_print(out, "    %s\n", a->filename);
}

static void ast_AST_print(const ast_AST* a, string_buffer_Buf* out, bool show_funcs)
{
   string_buffer_Buf_print(out, "---- AST %s ----\n", a->filename);
   ast_ImportDecl** imports = ast_ImportDeclList_getDecls(&a->imports);
   for (uint32_t i = 1; i < ast_ImportDeclList_size(&a->imports); i++) {
      ast_ImportDecl_print(imports[i], out, 0);
   }
   if (ast_ImportDeclList_size(&a->imports) > 1) string_buffer_Buf_add1(out, '\n');
   ast_Decl** types = ast_DeclList_getDecls(&a->types);
   for (uint32_t i = 0; i < ast_DeclList_size(&a->types); i++) {
      ast_Decl_print(types[i], out, 0);
      string_buffer_Buf_add1(out, '\n');
   }
   ast_Decl** variables = ast_DeclList_getDecls(&a->variables);
   for (uint32_t i = 0; i < ast_DeclList_size(&a->variables); i++) {
      ast_Decl_print(variables[i], out, 0);
      string_buffer_Buf_add1(out, '\n');
   }
   if (show_funcs) {
      ast_FunctionDecl** functions = ast_FunctionDeclList_getDecls(&a->functions);
      for (uint32_t i = 0; i < ast_FunctionDeclList_size(&a->functions); i++) {
         ast_FunctionDecl_print(functions[i], out, 0);
         string_buffer_Buf_add1(out, '\n');
      }
   }
   ast_Decl** asserts = ast_DeclList_getDecls(&a->static_asserts);
   for (uint32_t i = 0; i < ast_DeclList_size(&a->static_asserts); i++) {
      ast_Decl_print(asserts[i], out, 0);
      string_buffer_Buf_add1(out, '\n');
   }
}

static ast_Module* ast_Module_create(ast_context_Context* c, uint32_t name_idx)
{
   ast_Module* m = calloc(1, sizeof(ast_Module));
   m->mt = ast_ModuleType_create(c, m);
   m->name_idx = name_idx;
   ast_Module_resizeFiles(m, 1);
   ast_SymbolTable_init(&m->symbols, 16);
   ast_InstanceTable_init(&m->instances);
   return m;
}

static void ast_Module_free(ast_Module* m)
{
   for (uint32_t i = 0; i < m->num_files; i++) {
      ast_AST_free(m->files[i]);
   }
   free(((void*)(m->files)));
   ast_SymbolTable_free(&m->symbols);
   ast_InstanceTable_free(&m->instances);
   free(m);
}

static void ast_Module_setUsed(ast_Module* m)
{
   m->used = true;
}

static bool ast_Module_isUsed(const ast_Module* m)
{
   return m->used;
}

static const ast_SymbolTable* ast_Module_getSymbols(const ast_Module* m)
{
   return &m->symbols;
}

static ast_ModuleType* ast_Module_getType(const ast_Module* m)
{
   return m->mt;
}

static void ast_Module_visitASTs(const ast_Module* m, ast_ASTVisitor visitor, void* arg)
{
   for (uint32_t i = 0; i < m->num_files; i++) {
      visitor(arg, m->files[i]);
   }
}

static void ast_Module_visitImports(const ast_Module* m, ast_ImportVisitor visitor, void* arg)
{
   for (uint32_t i = 0; i < m->num_files; i++) {
      ast_AST_visitImports(m->files[i], visitor, arg);
   }
}

static void ast_Module_visitStructFunctions(const ast_Module* m, ast_FunctionVisitor visitor, void* arg)
{
   for (uint32_t i = 0; i < m->num_files; i++) {
      ast_AST_visitStructFunctions(m->files[i], visitor, arg);
   }
}

static void ast_Module_visitFunctions(const ast_Module* m, ast_FunctionVisitor visitor, void* arg)
{
   for (uint32_t i = 0; i < m->num_files; i++) {
      ast_AST_visitFunctions(m->files[i], visitor, arg);
   }
}

static void ast_Module_visitTypeDecls(const ast_Module* m, ast_TypeDeclVisitor visitor, void* arg)
{
   for (uint32_t i = 0; i < m->num_files; i++) {
      ast_AST_visitTypeDecls(m->files[i], visitor, arg);
   }
}

static void ast_Module_visitVarDecls(const ast_Module* m, ast_VarDeclVisitor visitor, void* arg)
{
   for (uint32_t i = 0; i < m->num_files; i++) {
      ast_AST_visitVarDecls(m->files[i], visitor, arg);
   }
}

static void ast_Module_visitStaticAsserts(const ast_Module* m, ast_DeclVisitor visitor, void* arg)
{
   for (uint32_t i = 0; i < m->num_files; i++) {
      ast_AST_visitStaticAsserts(m->files[i], visitor, arg);
   }
}

static void ast_Module_visitDecls(const ast_Module* m, ast_DeclVisitor visitor, void* arg)
{
   for (uint32_t i = 0; i < m->num_files; i++) {
      ast_AST_visitDecls(m->files[i], visitor, arg);
   }
}

static ast_Decl* ast_Module_findType(const ast_Module* m, uint32_t name_idx)
{
   ast_Decl* result = NULL;
   for (uint32_t i = 0; i < m->num_files; i++) {
      result = ast_AST_findType(m->files[i], name_idx);
      if (result) break;

   }
   return result;
}

static const char* ast_Module_getName(const ast_Module* m)
{
   return ast_idx2name(m->name_idx);
}

static uint32_t ast_Module_getNameIdx(const ast_Module* m)
{
   return m->name_idx;
}

static void ast_Module_resizeFiles(ast_Module* m, uint32_t cap)
{
   m->max_files = cap;
   void* buf = malloc(m->max_files * sizeof(ast_AST*));
   if (m->files) {
      void* old = ((void*)(m->files));
      memcpy(buf, old, m->num_files * sizeof(ast_AST*));
      free(old);
   }
   m->files = buf;
}

static ast_AST* ast_Module_add(ast_Module* m, const char* filename)
{
   ast_AST* a = ast_AST_create(filename, m);
   if (m->num_files == m->max_files) ast_Module_resizeFiles(m, m->max_files * 2);
   m->files[m->num_files] = a;
   m->num_files++;
   return a;
}

static void ast_Module_addSymbol(ast_Module* m, uint32_t name_idx, ast_Decl* d)
{
   ast_SymbolTable_add(&m->symbols, name_idx, d);
}

static ast_Decl* ast_Module_findSymbol(const ast_Module* m, uint32_t name_idx)
{
   return ast_SymbolTable_find(&m->symbols, name_idx);
}

static ast_FunctionDecl* ast_Module_findInstance(const ast_Module* m, ast_FunctionDecl* fd, ast_QualType qt)
{
   return ast_InstanceTable_find(&m->instances, fd, qt);
}

static uint16_t ast_Module_addInstance(ast_Module* m, ast_FunctionDecl* fd, ast_QualType qt, ast_FunctionDecl* instance)
{
   return ast_InstanceTable_add(&m->instances, fd, qt, instance);
}

static ast_FunctionDecl* ast_Module_getInstance(const ast_Module* m, ast_FunctionDecl* fd, uint32_t idx)
{
   return ast_InstanceTable_get(&m->instances, fd, idx);
}

static void ast_Module_visitInstances(const ast_Module* m, ast_FunctionDecl* fd, ast_TemplateVisitor visitor, void* arg)
{
   ast_InstanceTable_visit(&m->instances, fd, visitor, arg);
}

static void ast_Module_info(const ast_Module* m, string_buffer_Buf* out)
{
   string_buffer_Buf_print(out, "  module %s\n", ast_idx2name(m->name_idx));
   for (uint32_t i = 0; i < m->num_files; i++) {
      ast_AST_info(m->files[i], out);
   }
}

static void ast_Module_print(const ast_Module* m, string_buffer_Buf* out, bool show_funcs)
{
   string_buffer_Buf_print(out, "------ module %s (used %d) ------\n", ast_idx2name(m->name_idx), m->used);
   for (uint32_t i = 0; i < m->num_files; i++) {
      ast_AST_print(m->files[i], out, show_funcs);
   }
}

static void ast_DeclList_init(ast_DeclList* l, uint32_t initial_size)
{
   memset(l, 0, sizeof(ast_DeclList));
   if (initial_size) {
      l->capacity = initial_size;
      l->decls = malloc(l->capacity * sizeof(ast_Decl*));
   }
}

static void ast_DeclList_free(ast_DeclList* l)
{
   if (l->decls) free(((void*)(l->decls)));
}

static void ast_DeclList_add(ast_DeclList* l, ast_Decl* d)
{
   if (l->count >= l->capacity) {
      l->capacity += 4;
      void* decls2 = malloc(l->capacity * sizeof(ast_Decl*));
      void* old = ((void*)(l->decls));
      if (old) {
         memcpy(decls2, old, l->count * sizeof(ast_Decl*));
         free(old);
      }
      l->decls = decls2;
   }
   l->decls[l->count] = d;
   l->count++;
}

static uint32_t ast_DeclList_size(const ast_DeclList* l)
{
   return l->count;
}

static ast_Decl* ast_DeclList_get(const ast_DeclList* l, uint32_t idx)
{
   return l->decls[idx];
}

static ast_Decl** ast_DeclList_getDecls(const ast_DeclList* l)
{
   return l->decls;
}

static void ast_DeclList_setSize(ast_DeclList* l, uint32_t size)
{
   l->count = size;
}

static void ast_ExprList_init(ast_ExprList* l, uint32_t initial_size)
{
   memset(l, 0, sizeof(ast_ExprList));
   if (initial_size) {
      l->capacity = initial_size;
      l->exprs = malloc(l->capacity * sizeof(ast_Expr*));
   }
}

static void ast_ExprList_free(ast_ExprList* l)
{
   if (l->exprs) free(((void*)(l->exprs)));
}

static void ast_ExprList_add(ast_ExprList* l, ast_Expr* d)
{
   if (l->count >= l->capacity) {
      l->capacity += 4;
      void* exprs2 = malloc(l->capacity * sizeof(ast_Expr*));
      void* old = ((void*)(l->exprs));
      if (old) {
         memcpy(exprs2, old, l->count * sizeof(ast_Expr*));
         free(old);
      }
      l->exprs = exprs2;
   }
   l->exprs[l->count] = d;
   l->count++;
}

static uint32_t ast_ExprList_size(const ast_ExprList* l)
{
   return l->count;
}

static ast_Expr** ast_ExprList_getExprs(const ast_ExprList* l)
{
   return l->exprs;
}

static void ast_FunctionDeclList_init(ast_FunctionDeclList* l)
{
   memset(l, 0, sizeof(ast_FunctionDeclList));
}

static void ast_FunctionDeclList_free(ast_FunctionDeclList* l)
{
   if (l->decls) free(((void*)(l->decls)));
}

static void ast_FunctionDeclList_add(ast_FunctionDeclList* l, ast_FunctionDecl* d)
{
   if (l->count >= l->capacity) {
      l->capacity = (l->capacity == 0) ? 4 : l->capacity * 2;
      void* decls2 = malloc(l->capacity * sizeof(ast_FunctionDecl*));
      void* old = ((void*)(l->decls));
      if (old) {
         memcpy(decls2, old, l->count * sizeof(ast_FunctionDecl*));
         free(old);
      }
      l->decls = decls2;
   }
   l->decls[l->count] = d;
   l->count++;
}

static uint32_t ast_FunctionDeclList_size(const ast_FunctionDeclList* l)
{
   return l->count;
}

static ast_FunctionDecl** ast_FunctionDeclList_getDecls(const ast_FunctionDeclList* l)
{
   return l->decls;
}

static void ast_ImportDeclList_init(ast_ImportDeclList* l)
{
   memset(l, 0, sizeof(ast_ImportDeclList));
}

static void ast_ImportDeclList_free(ast_ImportDeclList* l)
{
   if (l->decls) free(((void*)(l->decls)));
}

static void ast_ImportDeclList_add(ast_ImportDeclList* l, ast_ImportDecl* d)
{
   if (l->count >= l->capacity) {
      l->capacity += 4;
      void* decls2 = malloc(l->capacity * sizeof(ast_ImportDecl*));
      void* old = ((void*)(l->decls));
      if (old) {
         memcpy(decls2, old, l->count * sizeof(ast_ImportDecl*));
         free(old);
      }
      l->decls = decls2;
   }
   l->decls[l->count] = d;
   l->count++;
}

static uint32_t ast_ImportDeclList_size(const ast_ImportDeclList* l)
{
   return l->count;
}

static ast_ImportDecl** ast_ImportDeclList_getDecls(const ast_ImportDeclList* l)
{
   return l->decls;
}

static ast_ImportDecl* ast_ImportDeclList_find(const ast_ImportDeclList* l, uint32_t name_idx)
{
   for (uint32_t i = 0; i < l->count; i++) {
      ast_ImportDecl* d = l->decls[i];
      if (ast_ImportDecl_getImportNameIdx(d) == name_idx) return d;

   }
   return NULL;
}

static ast_ImportDecl* ast_ImportDeclList_findAny(const ast_ImportDeclList* l, uint32_t name_idx)
{
   for (uint32_t i = 0; i < l->count; i++) {
      ast_ImportDecl* d = l->decls[i];
      if (ast_Decl_getNameIdx(ast_ImportDecl_asDecl(d)) == name_idx) return d;

   }
   return NULL;
}

static void ast_SymbolTable_init(ast_SymbolTable* t, uint32_t initial)
{
   t->num_symbols = 0;
   t->max_symbols = 0;
   ast_SymbolTable_resize(t, initial);
   ast_DeclList_init(&t->decls, initial);
}

static void ast_SymbolTable_free(ast_SymbolTable* t)
{
   free(t->symbols);
   ast_DeclList_free(&t->decls);
}

static uint32_t ast_SymbolTable_size(const ast_SymbolTable* t)
{
   return t->num_symbols;
}

static ast_Decl** ast_SymbolTable_getDecls(const ast_SymbolTable* l)
{
   return ast_DeclList_getDecls(&l->decls);
}

static void ast_SymbolTable_crop(ast_SymbolTable* t, uint32_t size)
{
   t->num_symbols = size;
   ast_DeclList_setSize(&t->decls, size);
}

static void ast_SymbolTable_resize(ast_SymbolTable* t, uint32_t capacity)
{
   uint32_t* symbols = malloc(capacity * sizeof(uint32_t));
   t->max_symbols = capacity;
   if (t->symbols) {
      memcpy(symbols, t->symbols, t->num_symbols * sizeof(uint32_t));
      free(t->symbols);
   }
   t->symbols = symbols;
}

static void ast_SymbolTable_add(ast_SymbolTable* t, uint32_t name_idx, ast_Decl* d)
{
   if (t->num_symbols == t->max_symbols) ast_SymbolTable_resize(t, t->max_symbols * 2);
   t->symbols[t->num_symbols] = name_idx;
   t->num_symbols++;
   ast_DeclList_add(&t->decls, d);
}

static ast_Decl* ast_SymbolTable_find(const ast_SymbolTable* t, uint32_t name_idx)
{
   for (uint32_t i = 0; i < t->num_symbols; i++) {
      if (t->symbols[i] == name_idx) return ast_DeclList_get(&t->decls, i);

   }
   return NULL;
}

static void ast_SymbolTable_print(const ast_SymbolTable* t, string_buffer_Buf* out)
{
   for (uint32_t i = 0; i < t->num_symbols; i++) {
      const ast_Decl* d = ast_DeclList_get(&t->decls, i);
      const char* col = ast_Decl_isUsed(d) ? color_Normal : color_Grey;
      string_buffer_Buf_color(out, col);
      string_buffer_Buf_print(out, "     %s", ast_idx2name(t->symbols[i]));
      if (ast_Decl_isPublic(d)) {
         string_buffer_Buf_color(out, color_Yellow);
         string_buffer_Buf_add(out, " public");
      }
      string_buffer_Buf_add1(out, '\n');
   }
}

static void ast_TemplateFunction_init(ast_TemplateFunction* f, ast_FunctionDecl* fd)
{
   f->fd = fd;
   f->count = 0;
   f->capacity = 0;
   f->instances = NULL;
   ast_TemplateFunction_resize(f, 2);
}

static void ast_TemplateFunction_resize(ast_TemplateFunction* f, uint16_t capacity)
{
   f->capacity = capacity;
   ast_TemplateInstance* inst2 = malloc(capacity * sizeof(ast_TemplateInstance));
   if (f->count) {
      memcpy(inst2, f->instances, f->count * sizeof(ast_TemplateInstance));
      free(f->instances);
   }
   f->instances = inst2;
}

static uint16_t ast_TemplateFunction_add(ast_TemplateFunction* f, ast_QualType qt, ast_FunctionDecl* instance)
{
   if (f->count == f->capacity) ast_TemplateFunction_resize(f, f->capacity * 2);
   uint16_t idx = f->count;
   ast_TemplateInstance* ti = &f->instances[idx];
   f->count++;
   ti->qt = qt;
   ti->instance = instance;
   return idx;
}

static ast_FunctionDecl* ast_TemplateFunction_find(const ast_TemplateFunction* f, ast_QualType qt)
{
   for (uint32_t i = 0; i < f->count; i++) {
      const ast_TemplateInstance* ti = &f->instances[i];
      if (ti->qt.ptr == qt.ptr) return ti->instance;

   }
   return NULL;
}

static ast_FunctionDecl* ast_TemplateFunction_get(const ast_TemplateFunction* f, uint32_t idx)
{
   return f->instances[idx].instance;
}

static void ast_InstanceTable_init(ast_InstanceTable* t)
{
   t->count = 0;
   t->capacity = 0;
   t->funcs = NULL;
}

static void ast_InstanceTable_free(ast_InstanceTable* t)
{
   for (uint32_t i = 0; i < t->count; i++) {
      free(t->funcs[i].instances);
   }
   free(t->funcs);
}

static void ast_InstanceTable_resize(ast_InstanceTable* t, uint32_t capacity)
{
   t->capacity = capacity;
   ast_TemplateFunction* funcs2 = malloc(capacity * sizeof(ast_TemplateFunction));
   if (t->count) {
      memcpy(funcs2, t->funcs, t->count * sizeof(ast_TemplateFunction));
      free(t->funcs);
   }
   t->funcs = funcs2;
}

static ast_TemplateFunction* ast_InstanceTable_findFunc(const ast_InstanceTable* t, ast_FunctionDecl* fd)
{
   for (uint32_t i = 0; i < t->count; i++) {
      ast_TemplateFunction* fi = &t->funcs[i];
      if (fi->fd == fd) return fi;

   }
   return NULL;
}

static ast_FunctionDecl* ast_InstanceTable_find(const ast_InstanceTable* t, ast_FunctionDecl* fd, ast_QualType qt)
{
   const ast_TemplateFunction* fi = ast_InstanceTable_findFunc(t, fd);
   if (fi) return ast_TemplateFunction_find(fi, qt);

   return NULL;
}

static ast_FunctionDecl* ast_InstanceTable_get(const ast_InstanceTable* t, ast_FunctionDecl* fd, uint32_t idx)
{
   const ast_TemplateFunction* fi = ast_InstanceTable_findFunc(t, fd);
   assert(fi);
   return ast_TemplateFunction_get(fi, idx);
}

static uint16_t ast_InstanceTable_add(ast_InstanceTable* t, ast_FunctionDecl* fd, ast_QualType qt, ast_FunctionDecl* instance)
{
   ast_TemplateFunction* fi = ast_InstanceTable_findFunc(t, fd);
   if (!fi) {
      if (t->count == t->capacity) {
         if (t->capacity == 0) t->capacity = 2;
         ast_InstanceTable_resize(t, t->capacity * 2);
      }
      fi = &t->funcs[t->count];
      t->count++;
      ast_TemplateFunction_init(fi, fd);
   }
   return ast_TemplateFunction_add(fi, qt, instance);
}

static void ast_InstanceTable_visit(const ast_InstanceTable* t, ast_FunctionDecl* fd, ast_TemplateVisitor visitor, void* arg)
{
   ast_TemplateFunction* fi = ast_InstanceTable_findFunc(t, fd);
   if (!fi) return;

   for (uint32_t i = 0; i < fi->count; i++) {
      visitor(arg, fi->instances[i].instance, i + 1);
   }
}

static void ast_PointerPool_init(ast_PointerPool* p, ast_context_Context* c)
{
   p->count = 1;
   p->capacity = 0;
   p->slots = NULL;
   p->context = c;
   ast_PointerPool_resize(p, 64);
}

static void ast_PointerPool_clear(ast_PointerPool* p)
{
   free(p->slots);
   p->count = 1;
   p->capacity = 0;
   p->slots = NULL;
}

static void ast_PointerPool_resize(ast_PointerPool* p, uint32_t cap)
{
   p->capacity = cap;
   ast_PointerPoolSlot* slots2 = malloc(p->capacity * sizeof(ast_PointerPoolSlot));
   if (p->count > 1) {
      memcpy(slots2, p->slots, p->count * sizeof(ast_PointerPoolSlot));
      free(p->slots);
   }
   p->slots = slots2;
}

static ast_Type* ast_PointerPool_getPointer(ast_PointerPool* p, ast_QualType qt)
{
   ast_Type* t = ast_QualType_getTypeOrNil(&qt);
   assert(t);
   const uint32_t ptr_pool_idx = t->ptr_pool_idx;
   ast_PointerPoolSlot* slot = &p->slots[ptr_pool_idx];
   if (ptr_pool_idx == 0) {
      uint32_t slot_idx = p->count;
      if (slot_idx == p->capacity) ast_PointerPool_resize(p, p->capacity * 2);
      slot = &p->slots[slot_idx];
      memset(slot, 0, sizeof(ast_PointerPoolSlot));
      t->ptr_pool_idx = slot_idx;
      p->count++;
   }
   uint32_t quals = ast_QualType_getQuals(&qt);
   ast_Type* ptr = slot->ptrs[quals];
   if (ptr) return ptr;

   ptr = ((ast_Type*)(ast_PointerType_create(p->context, qt)));
   slot->ptrs[quals] = ptr;
   return ptr;
}

static void ast_StringTypePool_init(ast_StringTypePool* p, ast_context_Context* c)
{
   p->count = 0;
   p->capacity = 0;
   p->slots = NULL;
   p->context = c;
   ast_StringTypePool_resize(p, 8);
}

static void ast_StringTypePool_clear(ast_StringTypePool* p)
{
   free(p->slots);
   p->count = 0;
   p->capacity = 0;
   p->slots = NULL;
}

static void ast_StringTypePool_resize(ast_StringTypePool* p, uint32_t cap)
{
   p->capacity = cap;
   ast_StringTypeSlot* slots2 = malloc(p->capacity * sizeof(ast_StringTypeSlot));
   if (p->count) {
      memcpy(slots2, p->slots, p->count * sizeof(ast_StringTypeSlot));
      free(p->slots);
   }
   p->slots = slots2;
}

static ast_QualType ast_StringTypePool_get(ast_StringTypePool* p, uint32_t len)
{
   for (uint32_t i = 0; i < p->count; i++) {
      ast_StringTypeSlot* s = &p->slots[i];
      if (s->len == len) return ast_QualType_init(s->type_);

   }
   if (p->count == p->capacity) ast_StringTypePool_resize(p, p->capacity * 2);
   ast_Type* t = ((ast_Type*)(ast_ArrayType_create(p->context, ast_g_char, true, len)));
   uint32_t idx = p->count;
   p->slots[idx].len = len;
   p->slots[idx].type_ = t;
   p->count++;
   return ast_QualType_init(t);
}


// --- module ctv_analyser ---

typedef struct ctv_analyser_Limit_ ctv_analyser_Limit;
typedef struct ctv_analyser_Value_ ctv_analyser_Value;

struct ctv_analyser_Limit_ {
   int64_t min_val;
   uint64_t max_val;
   const char* min_str;
   const char* max_str;
};

struct ctv_analyser_Value_ {
   bool is_signed;
   union {
      uint64_t uvalue;
      int64_t svalue;
   };
};

static const ctv_analyser_Limit* ctv_analyser_getLimit(uint32_t width);
static ctv_analyser_Value ctv_analyser_get_value(const ast_Expr* e);
static ctv_analyser_Value ctv_analyser_get_decl_value(const ast_Decl* d);
static ctv_analyser_Value ctv_analyser_get_unaryop_value(const ast_UnaryOperator* e);
static ctv_analyser_Value ctv_analyser_get_binaryop_value(const ast_BinaryOperator* e);
static bool ctv_analyser_Value_isNegative(const ctv_analyser_Value* v);
static bool ctv_analyser_Value_equals(const ctv_analyser_Value* v1, const ctv_analyser_Value* v2);
static const char* ctv_analyser_Value_str(const ctv_analyser_Value* v);

static const ctv_analyser_Limit ctv_analyser_Limits[] = {
   { 0, 1, "0", "1" },
   { -128, 127, "-128", "127" },
   { 0, 255, "0", "255" },
   { -32768, 32767, "-32768", "32767" },
   { -32768, 32767, "-32768", "32767" },
   { -2147483648, 2147483647, "-2147483648", "2147483647" },
   { 0, 4294967295, "0", "4294967295" },
   { -9223372036854775807lu, 9223372036854775807lu, "-9223372036854775808", "9223372036854775807" },
   { 0, 18446744073709551615lu, "0", "18446744073709551615" }
};

static const ctv_analyser_Limit* ctv_analyser_getLimit(uint32_t width)
{
   switch (width) {
   case 1:
      return &ctv_analyser_Limits[0];
   case 7:
      return &ctv_analyser_Limits[1];
   case 8:
      return &ctv_analyser_Limits[2];
   case 15:
      return &ctv_analyser_Limits[3];
   case 16:
      return &ctv_analyser_Limits[4];
   case 31:
      return &ctv_analyser_Limits[5];
   case 32:
      return &ctv_analyser_Limits[6];
   case 63:
      return &ctv_analyser_Limits[7];
   case 64:
      return &ctv_analyser_Limits[8];
   }
   assert(0);
   return NULL;
}

static ctv_analyser_Value ctv_analyser_get_value(const ast_Expr* e)
{
   ctv_analyser_Value result = { };
   if (!ast_Expr_isCtv(e)) ast_Expr_dump(e);
   assert(ast_Expr_isCtv(e));
   switch (ast_Expr_getKind(e)) {
   case ast_ExprKind_IntegerLiteral: {
      const ast_IntegerLiteral* i = ((ast_IntegerLiteral*)(e));
      result.uvalue = ast_IntegerLiteral_getValue(i);
      break;
   }
   case ast_ExprKind_BooleanLiteral: {
      const ast_BooleanLiteral* b = ((ast_BooleanLiteral*)(e));
      result.uvalue = ast_BooleanLiteral_getValue(b);
      break;
   }
   case ast_ExprKind_CharLiteral: {
      const ast_CharLiteral* c = ((ast_CharLiteral*)(e));
      result.uvalue = ast_CharLiteral_getValue(c);
      break;
   }
   case ast_ExprKind_StringLiteral:
      assert(0);
      break;
   case ast_ExprKind_Nil:
      break;
   case ast_ExprKind_Identifier: {
      const ast_IdentifierExpr* i = ((ast_IdentifierExpr*)(e));
      return ctv_analyser_get_decl_value(ast_IdentifierExpr_getDecl(i));
   }
   case ast_ExprKind_Type:
      // fallthrough
   case ast_ExprKind_Call:
      // fallthrough
   case ast_ExprKind_InitList:
      // fallthrough
   case ast_ExprKind_FieldDesignatedInit:
      // fallthrough
   case ast_ExprKind_ArrayDesignatedInit:
      break;
   case ast_ExprKind_BinaryOperator:
      return ctv_analyser_get_binaryop_value(((ast_BinaryOperator*)(e)));
   case ast_ExprKind_UnaryOperator:
      return ctv_analyser_get_unaryop_value(((ast_UnaryOperator*)(e)));
   case ast_ExprKind_ConditionalOperator:
      break;
   case ast_ExprKind_Builtin: {
      const ast_BuiltinExpr* bi = ((ast_BuiltinExpr*)(e));
      result.uvalue = ast_BuiltinExpr_getValue(bi);
      break;
   }
   case ast_ExprKind_ArraySubscript:
      break;
   case ast_ExprKind_Member: {
      const ast_MemberExpr* m = ((ast_MemberExpr*)(e));
      return ctv_analyser_get_decl_value(ast_MemberExpr_getFullDecl(m));
   }
   case ast_ExprKind_Paren: {
      const ast_ParenExpr* p = ((ast_ParenExpr*)(e));
      return ctv_analyser_get_value(ast_ParenExpr_getInner(p));
   }
   case ast_ExprKind_BitOffset:
      break;
   case ast_ExprKind_ExplicitCast: {
      const ast_ImplicitCastExpr* i = ((ast_ImplicitCastExpr*)(e));
      return ctv_analyser_get_value(ast_ImplicitCastExpr_getInner(i));
   }
   case ast_ExprKind_ImplicitCast: {
      const ast_ImplicitCastExpr* i = ((ast_ImplicitCastExpr*)(e));
      return ctv_analyser_get_value(ast_ImplicitCastExpr_getInner(i));
   }
   }
   return result;
}

static ctv_analyser_Value ctv_analyser_get_decl_value(const ast_Decl* d)
{
   assert(d);
   ctv_analyser_Value result = { };
   switch (ast_Decl_getKind(d)) {
   case ast_DeclKind_EnumConstant: {
      const ast_EnumConstantDecl* ecd = ((ast_EnumConstantDecl*)(d));
      result.uvalue = ast_EnumConstantDecl_getValue(ecd);
      break;
   }
   case ast_DeclKind_Var: {
      const ast_VarDecl* vd = ((ast_VarDecl*)(d));
      const ast_Expr* initval = ast_VarDecl_getInit(vd);
      assert(initval);
      return ctv_analyser_get_value(initval);
   }
   default:
      assert(0);
      break;
   }
   return result;
}

static ctv_analyser_Value ctv_analyser_get_unaryop_value(const ast_UnaryOperator* e)
{
   ctv_analyser_Value result = { };
   const ast_Expr* inner = ast_UnaryOperator_getInner(e);
   ctv_analyser_Value res2 = ctv_analyser_get_value(inner);
   switch (ast_UnaryOperator_getOpcode(e)) {
   case ast_UnaryOpcode_PostInc:
      // fallthrough
   case ast_UnaryOpcode_PostDec:
      // fallthrough
   case ast_UnaryOpcode_PreInc:
      // fallthrough
   case ast_UnaryOpcode_PreDec:
      break;
   case ast_UnaryOpcode_AddrOf:
      // fallthrough
   case ast_UnaryOpcode_Deref:
      break;
   case ast_UnaryOpcode_Minus:
      result.is_signed = true;
      result.svalue = res2.is_signed ? -res2.svalue : ((int64_t)(-res2.uvalue));
      break;
   case ast_UnaryOpcode_Not:
      result.svalue = (result.svalue == 0) ? 1 : 0;
      break;
   case ast_UnaryOpcode_LNot:
      result.uvalue = res2.is_signed ? ((uint64_t)(~res2.svalue)) : ~res2.uvalue;
      break;
   }
   return result;
}

static ctv_analyser_Value ctv_analyser_get_binaryop_value(const ast_BinaryOperator* e)
{
   ctv_analyser_Value result = { };
   ctv_analyser_Value left = ctv_analyser_get_value(ast_BinaryOperator_getLHS(e));
   ctv_analyser_Value right = ctv_analyser_get_value(ast_BinaryOperator_getRHS(e));
   result.is_signed = left.is_signed;
   switch (ast_BinaryOperator_getOpcode(e)) {
   case ast_BinaryOpcode_Multiply:
      if ((left.is_signed || right.is_signed)) {
         int64_t lval = (left.is_signed) ? left.svalue : ((int64_t)(left.uvalue));
         int64_t rval = (right.is_signed) ? right.svalue : ((int64_t)(right.uvalue));
         result.svalue = lval * rval;
         result.is_signed = true;
      } else {
         result.uvalue = left.uvalue * right.uvalue;
      }
      break;
   case ast_BinaryOpcode_Divide:
      assert(right.svalue != 0);
      if ((left.is_signed || right.is_signed)) {
         int64_t lval = (left.is_signed) ? left.svalue : ((int64_t)(left.uvalue));
         int64_t rval = (right.is_signed) ? right.svalue : ((int64_t)(right.uvalue));
         result.svalue = lval / rval;
         result.is_signed = true;
      } else {
         result.uvalue = left.uvalue / right.uvalue;
      }
      break;
   case ast_BinaryOpcode_Reminder:
      assert(right.svalue != 0);
      if (left.is_signed) result.svalue = left.svalue % right.svalue;
      else result.uvalue = left.uvalue % right.uvalue;
      break;
   case ast_BinaryOpcode_Add:
      if ((left.is_signed || right.is_signed)) {
         int64_t lval = (left.is_signed) ? left.svalue : ((int64_t)(left.uvalue));
         int64_t rval = (right.is_signed) ? right.svalue : ((int64_t)(right.uvalue));
         result.svalue = lval + rval;
         result.is_signed = true;
      } else {
         result.uvalue = left.uvalue + right.uvalue;
      }
      break;
   case ast_BinaryOpcode_Subtract: {
      int64_t lval = (left.is_signed) ? left.svalue : ((int64_t)(left.uvalue));
      int64_t rval = (right.is_signed) ? right.svalue : ((int64_t)(right.uvalue));
      result.svalue = lval - rval;
      result.is_signed = true;
      break;
   }
   case ast_BinaryOpcode_ShiftLeft:
      result.uvalue = left.uvalue << right.uvalue;
      break;
   case ast_BinaryOpcode_ShiftRight:
      result.uvalue = left.uvalue >> right.uvalue;
      break;
   case ast_BinaryOpcode_LessThan:
      result.is_signed = false;
      if (left.is_signed) result.svalue = left.svalue < right.svalue;
      else result.uvalue = left.uvalue < right.uvalue;
      break;
   case ast_BinaryOpcode_GreaterThan:
      result.is_signed = false;
      if (left.is_signed) result.svalue = left.svalue > right.svalue;
      else result.uvalue = left.uvalue > right.uvalue;
      break;
   case ast_BinaryOpcode_LessEqual:
      result.is_signed = false;
      if (left.is_signed) result.svalue = left.svalue <= right.svalue;
      else result.uvalue = left.uvalue <= right.uvalue;
      break;
   case ast_BinaryOpcode_GreaterEqual:
      result.is_signed = false;
      if (left.is_signed) result.svalue = left.svalue >= right.svalue;
      else result.uvalue = left.uvalue >= right.uvalue;
      break;
   case ast_BinaryOpcode_Equal:
      result.is_signed = false;
      if (left.is_signed) result.svalue = left.svalue == right.svalue;
      else result.uvalue = left.uvalue == right.uvalue;
      break;
   case ast_BinaryOpcode_NotEqual:
      result.is_signed = false;
      if (left.is_signed) result.svalue = left.svalue != right.svalue;
      else result.uvalue = left.uvalue != right.uvalue;
      break;
   case ast_BinaryOpcode_And:
      result.is_signed = false;
      if (left.is_signed) result.svalue = (left.svalue & right.svalue);
      else result.uvalue = (left.uvalue & right.uvalue);
      break;
   case ast_BinaryOpcode_Xor:
      result.is_signed = false;
      if (left.is_signed) result.svalue = (left.svalue ^ right.svalue);
      else result.uvalue = (left.uvalue ^ right.uvalue);
      break;
   case ast_BinaryOpcode_Or:
      result.is_signed = false;
      if (left.is_signed) result.svalue = (left.svalue | right.svalue);
      else result.uvalue = (left.uvalue | right.uvalue);
      break;
   case ast_BinaryOpcode_LAnd:
      result.is_signed = false;
      if (left.is_signed) result.svalue = (left.svalue && right.svalue);
      else result.uvalue = (left.uvalue && right.uvalue);
      break;
   case ast_BinaryOpcode_LOr:
      result.is_signed = false;
      if (left.is_signed) result.svalue = (left.svalue || right.svalue);
      else result.uvalue = (left.uvalue || right.uvalue);
      break;
   case ast_BinaryOpcode_Assign:
      // fallthrough
   case ast_BinaryOpcode_MulAssign:
      // fallthrough
   case ast_BinaryOpcode_DivAssign:
      // fallthrough
   case ast_BinaryOpcode_RemAssign:
      // fallthrough
   case ast_BinaryOpcode_AddAssign:
      // fallthrough
   case ast_BinaryOpcode_SubAssign:
      // fallthrough
   case ast_BinaryOpcode_ShlAssign:
      // fallthrough
   case ast_BinaryOpcode_ShrASsign:
      // fallthrough
   case ast_BinaryOpcode_AndAssign:
      // fallthrough
   case ast_BinaryOpcode_XorAssign:
      // fallthrough
   case ast_BinaryOpcode_OrAssign:
      assert(0);
      break;
   }
   return result;
}

static bool ctv_analyser_Value_isNegative(const ctv_analyser_Value* v)
{
   if ((v->is_signed && v->svalue < 0)) return true;

   return false;
}

static bool ctv_analyser_Value_equals(const ctv_analyser_Value* v1, const ctv_analyser_Value* v2)
{
   if (v1->is_signed == v2->is_signed) {
      return v1->uvalue == v2->uvalue;
   }
   if (v1->is_signed) {
      if (v1->svalue >= 0) return v1->uvalue == v2->uvalue;

   } else {
      if (v2->svalue >= 0) return v1->uvalue == v2->uvalue;

   }
   return false;
}

static const char* ctv_analyser_Value_str(const ctv_analyser_Value* v)
{
   static char text[32][4];
   static uint8_t index = 0;
   char* out = text[index];
   index = (index + 1) % 4;
   if (v->is_signed) {
      sprintf(out, "%ld", v->svalue);
   } else {
      sprintf(out, "%lu", v->uvalue);
   }
   return out;
}


// --- module size_analyser ---

typedef struct size_analyser_TypeSize_ size_analyser_TypeSize;

struct size_analyser_TypeSize_ {
   uint32_t size;
   uint32_t align;
   uint32_t bitfield_size;
   uint32_t bitfield_width;
};

static size_analyser_TypeSize size_analyser_sizeOfUnion(ast_StructTypeDecl* s);
static size_analyser_TypeSize size_analyser_sizeOfStruct(ast_StructTypeDecl* s);
static size_analyser_TypeSize size_analyser_sizeOfType(ast_QualType qt);

static const uint32_t size_analyser_PointerSize = 8;

static size_analyser_TypeSize size_analyser_sizeOfUnion(ast_StructTypeDecl* s)
{
   size_analyser_TypeSize result = { 0, 1, 0, 0 };
   result.align = ast_StructTypeDecl_getAttrAlignment(s);
   uint32_t num_members = ast_StructTypeDecl_getNumMembers(s);
   ast_Decl** members = ast_StructTypeDecl_getMembers(s);
   for (uint32_t i = 0; i < num_members; i++) {
      ast_Decl* d = members[i];
      size_analyser_TypeSize m_size = size_analyser_sizeOfType(ast_Decl_getType(d));
      if (m_size.size > result.size) result.size = m_size.size;
      if (m_size.align > result.align) result.align = m_size.align;
   }
   return result;
}

static size_analyser_TypeSize size_analyser_sizeOfStruct(ast_StructTypeDecl* s)
{
   if (ast_StructTypeDecl_isUnion(s)) return size_analyser_sizeOfUnion(s);

   size_analyser_TypeSize result = { 0, 1, 0, 0 };
   bool packed = ast_StructTypeDecl_isPacked(s);
   result.align = ast_StructTypeDecl_getAttrAlignment(s);
   uint32_t num_members = ast_StructTypeDecl_getNumMembers(s);
   ast_Decl** members = ast_StructTypeDecl_getMembers(s);
   if (packed) {
      for (uint32_t i = 0; i < num_members; i++) {
         ast_Decl* d = members[i];
         size_analyser_TypeSize member = size_analyser_sizeOfType(ast_Decl_getType(d));
         result.size += member.size;
      }
   } else {
      for (uint32_t i = 0; i < num_members; i++) {
         ast_Decl* d = members[i];
         size_analyser_TypeSize member = size_analyser_sizeOfType(ast_Decl_getType(d));
         ast_VarDecl* vd = (ast_Decl_getKind(d) == ast_DeclKind_Var) ? ((ast_VarDecl*)(d)) : NULL;
         if (vd) {
            const ast_Expr* bitfield = ast_VarDecl_getBitfield(vd);
            if (bitfield) {
               ctv_analyser_Value value = ctv_analyser_get_value(bitfield);
               member.bitfield_size = ((uint32_t)(value.uvalue));
               member.bitfield_width = member.size * 8;
               member.size = 0;
               member.align = 0;
            }
         }
         if ((result.bitfield_width && member.align != 0)) {
            uint32_t bytesize = (result.bitfield_size + 7) / 8;
            result.size += bytesize;
            if (bytesize > result.align) result.align = bytesize;
            result.bitfield_width = 0;
            result.bitfield_size = 0;
         }
         if (member.align > 1) {
            if (member.align > result.align) result.align = member.align;
            uint32_t rest = result.size % member.align;
            if (rest != 0) {
               uint32_t pad = member.align - rest;
               result.size += pad;
            }
         }
         if (vd) {
            if (result.size != 0) ast_VarDecl_setOffset(vd, result.size);
         } else {
            assert(ast_Decl_getKind(d) == ast_DeclKind_StructType);
            ast_StructTypeDecl* std = ((ast_StructTypeDecl*)(d));
            ast_StructTypeDecl_setOffset(std, result.size);
         }
         if (member.bitfield_width) {
            uint32_t total_bitsize = result.bitfield_size + member.bitfield_size;
            if (total_bitsize > member.bitfield_width) {
               uint32_t bytesize = (result.bitfield_size + 7) / 8;
               member.align = bytesize;
               if (bytesize > 1) {
                  uint32_t rest = result.size % bytesize;
                  if (rest != 0) {
                     uint32_t pad = member.align - rest;
                     result.size += pad;
                  }
               }
               result.size += bytesize;
               result.bitfield_size = member.bitfield_size;
               result.bitfield_width = member.bitfield_width;
            } else {
               result.bitfield_size = total_bitsize;
               result.bitfield_width = member.bitfield_width;
            }
         } else {
            result.size += member.size;
         }
      }
      if (result.bitfield_width) {
         uint32_t bytesize = (result.bitfield_size + 7) / 8;
         result.size += bytesize;
      }
      uint32_t rest = result.size % result.align;
      if (rest != 0) {
         uint32_t pad = result.align - rest;
         result.size += pad;
      }
   }
   return result;
}

static size_analyser_TypeSize size_analyser_sizeOfType(ast_QualType qt)
{
   size_analyser_TypeSize result = { 0, 1, 0, 0 };
   if (ast_QualType_isInvalid(&qt)) return result;

   qt = ast_QualType_getCanonicalType(&qt);
   ast_Type* t = ast_QualType_getType(&qt);
   switch (ast_Type_getKind(t)) {
   case ast_TypeKind_Builtin: {
      const ast_BuiltinType* bi = ((ast_BuiltinType*)(t));
      result.size = ast_BuiltinType_getAlignment(bi);
      result.align = result.size;
      break;
   }
   case ast_TypeKind_Pointer:
      result.size = size_analyser_PointerSize;
      result.align = result.size;
      break;
   case ast_TypeKind_Array: {
      ast_ArrayType* arrayType = ((ast_ArrayType*)(t));
      result = size_analyser_sizeOfType(ast_ArrayType_getElemType(arrayType));
      result.size *= ast_ArrayType_getSize(arrayType);
      break;
   }
   case ast_TypeKind_Struct: {
      ast_StructType* st = ((ast_StructType*)(t));
      ast_StructTypeDecl* d = ast_StructType_getDecl(st);
      assert(ast_Decl_isChecked(ast_StructTypeDecl_asDecl(d)));
      result.size = ast_StructTypeDecl_getSize(d);
      result.align = ast_StructTypeDecl_getAlignment(d);
      break;
   }
   case ast_TypeKind_Enum: {
      ast_EnumType* et = ((ast_EnumType*)(t));
      ast_EnumTypeDecl* etd = ast_EnumType_getDecl(et);
      return size_analyser_sizeOfType(ast_EnumTypeDecl_getImplType(etd));
   }
   case ast_TypeKind_Function:
      result.size = size_analyser_PointerSize;
      result.align = size_analyser_PointerSize;
      break;
   case ast_TypeKind_Alias:
      assert(0);
      break;
   case ast_TypeKind_Module:
      assert(0);
      break;
   }
   return result;
}


// --- module unused_checker ---

typedef struct unused_checker_Checker_ unused_checker_Checker;

struct unused_checker_Checker_ {
   diagnostics_Diags* diags;
   const warning_flags_Flags* warnings;
};

static void unused_checker_check(diagnostics_Diags* diags, const warning_flags_Flags* warnings, ast_Module* mod);
static void unused_checker_Checker_check(void* arg, ast_Decl* d);
static void unused_checker_Checker_checkEnum(unused_checker_Checker* c, ast_EnumTypeDecl* d);
static void unused_checker_Checker_checkStructMembers(unused_checker_Checker* c, ast_Decl* d);

static void unused_checker_check(diagnostics_Diags* diags, const warning_flags_Flags* warnings, ast_Module* mod)
{
   unused_checker_Checker c = { .diags = diags, .warnings = warnings };
   ast_Module_visitDecls(mod, unused_checker_Checker_check, &c);
}

static void unused_checker_Checker_check(void* arg, ast_Decl* d)
{
   unused_checker_Checker* c = arg;
   bool used = ast_Decl_isUsed(d);
   if (((ast_Decl_isPublic(d) && !ast_Decl_isUsedPublic(d)) && !c->warnings->no_unused_public)) {
      diagnostics_Diags_warn(c->diags, ast_Decl_getLoc(d), "unused public %s: %s", ast_Decl_getKindName(d), ast_Decl_getFullName(d));
   }
   switch (ast_Decl_getKind(d)) {
   case ast_DeclKind_Function:
      if (c->warnings->no_unused_function) return;

      break;
   case ast_DeclKind_Import:
      if (c->warnings->no_unused_import) return;

      break;
   case ast_DeclKind_StructType:
      if (used) {
         unused_checker_Checker_checkStructMembers(c, d);
      }
      if (c->warnings->no_unused_type) return;

      break;
   case ast_DeclKind_EnumType:
      if ((used && !c->warnings->no_unused_enum_constant)) {
         unused_checker_Checker_checkEnum(c, ((ast_EnumTypeDecl*)(d)));
      }
      break;
   case ast_DeclKind_EnumConstant:
      break;
   case ast_DeclKind_FunctionType:
      if (c->warnings->no_unused_type) return;

      break;
   case ast_DeclKind_AliasType:
      if (c->warnings->no_unused_type) return;

      break;
   case ast_DeclKind_Var:
      if (c->warnings->no_unused_variable) return;

      break;
   case ast_DeclKind_StaticAssert:
      break;
   }
   if (!used) {
      diagnostics_Diags_warn(c->diags, ast_Decl_getLoc(d), "unused %s: %s", ast_Decl_getKindName(d), ast_Decl_getFullName(d));
      return;
   }
}

static void unused_checker_Checker_checkEnum(unused_checker_Checker* c, ast_EnumTypeDecl* d)
{
   uint32_t num_consts = ast_EnumTypeDecl_getNumConstants(d);
   ast_EnumConstantDecl** constants = ast_EnumTypeDecl_getConstants(d);
   for (uint32_t i = 0; i < num_consts; i++) {
      ast_EnumConstantDecl* ecd = constants[i];
      ast_Decl* dd = ((ast_Decl*)(ecd));
      if (!ast_Decl_isUsed(dd)) {
         diagnostics_Diags_warn(c->diags, ast_Decl_getLoc(dd), "unused %s: %s", ast_Decl_getKindName(dd), ast_Decl_getName(dd));
      }
   }
}

static void unused_checker_Checker_checkStructMembers(unused_checker_Checker* c, ast_Decl* d)
{
   ast_StructTypeDecl* std = ((ast_StructTypeDecl*)(d));
   uint32_t num_members = ast_StructTypeDecl_getNumMembers(std);
   ast_Decl** members = ast_StructTypeDecl_getMembers(std);
   for (uint32_t i = 0; i < num_members; i++) {
      ast_Decl* member = members[i];
      if (ast_Decl_getKind(member) == ast_DeclKind_StructType) {
         unused_checker_Checker_checkStructMembers(c, member);
      } else {
         if (!ast_Decl_isUsed(member)) {
            diagnostics_Diags_warn(c->diags, ast_Decl_getLoc(member), "unused struct member: %s", ast_Decl_getName(member));
         }
      }
   }
}


// --- module ast_utils ---

typedef struct ast_utils_StmtList_ ast_utils_StmtList;

struct ast_utils_StmtList_ {
   uint32_t count;
   uint32_t capacity;
   ast_Stmt* stack[4];
   ast_Stmt** heap;
};

static void ast_utils_StmtList_init(ast_utils_StmtList* l);
static void ast_utils_StmtList_add(ast_utils_StmtList* l, ast_Stmt* s);
static void ast_utils_StmtList_free(ast_utils_StmtList* l);
static uint32_t ast_utils_StmtList_size(const ast_utils_StmtList* l);
static ast_Stmt** ast_utils_StmtList_getData(ast_utils_StmtList* l);

static const uint32_t ast_utils_InitialHeapSize = 8;

static const uint32_t ast_utils_StackSize = 4;

static void ast_utils_StmtList_init(ast_utils_StmtList* l)
{
   memset(l, 0, sizeof(ast_utils_StmtList));
}

static void ast_utils_StmtList_add(ast_utils_StmtList* l, ast_Stmt* s)
{
   if (l->count < ast_utils_StackSize) {
      l->stack[l->count] = s;
   } else if (l->count > ast_utils_StackSize) {
      if (l->count == l->capacity) {
         l->capacity *= 2;
         ast_Stmt** heap2 = malloc(l->capacity * sizeof(ast_Stmt*));
         memcpy(((void*)(heap2)), ((void*)(l->heap)), l->count * sizeof(ast_Stmt*));
         free(((void*)(l->heap)));
         l->heap = heap2;
      }
      l->heap[l->count] = s;
   } else {
      l->capacity = ast_utils_InitialHeapSize;
      l->heap = malloc(l->capacity * sizeof(ast_Stmt*));
      memcpy(((void*)(l->heap)), ((void*)(l->stack)), ast_utils_StackSize * sizeof(ast_Stmt*));
      l->heap[l->count] = s;
   }

   l->count++;
}

static void ast_utils_StmtList_free(ast_utils_StmtList* l)
{
   if (l->heap) free(((void*)(l->heap)));
}

static uint32_t ast_utils_StmtList_size(const ast_utils_StmtList* l)
{
   return l->count;
}

static ast_Stmt** ast_utils_StmtList_getData(ast_utils_StmtList* l)
{
   if (l->heap) return l->heap;

   return l->stack;
}


// --- module struct_func_list ---

typedef struct struct_func_list_Info_ struct_func_list_Info;
typedef struct struct_func_list_List_ struct_func_list_List;

struct struct_func_list_Info_ {
   ast_Decl* decl;
   ast_FunctionDeclList functions;
};

struct struct_func_list_List_ {
   struct_func_list_Info* data;
   uint32_t count;
   uint32_t capacity;
};

static void struct_func_list_List_free(struct_func_list_List* v);
static void struct_func_list_List_resize(struct_func_list_List* v);
static void struct_func_list_List_addDecl(struct_func_list_List* v, ast_Decl* decl);
static ast_Decl* struct_func_list_List_getDecl(struct_func_list_List* v, uint32_t index);
static void struct_func_list_List_addFunc(struct_func_list_List* v, uint32_t index, ast_FunctionDecl* fn);

static void struct_func_list_List_free(struct_func_list_List* v)
{
   for (uint32_t i = 0; i < v->count; i++) {
      ast_FunctionDeclList_free(&v->data[i].functions);
   }
   free(v->data);
   v->count = 0;
   v->capacity = 0;
   v->data = NULL;
}

static void struct_func_list_List_resize(struct_func_list_List* v)
{
   v->capacity = v->capacity == 0 ? 4 : v->capacity * 2;
   struct_func_list_Info* data2 = malloc(v->capacity * sizeof(struct_func_list_Info));
   if (v->data) {
      memcpy(data2, v->data, v->count * sizeof(struct_func_list_Info));
      free(v->data);
   }
   v->data = data2;
}

static void struct_func_list_List_addDecl(struct_func_list_List* v, ast_Decl* decl)
{
   if (v->count == v->capacity) struct_func_list_List_resize(v);
   struct_func_list_Info* info = &v->data[v->count];
   info->decl = decl;
   ast_FunctionDeclList_init(&info->functions);
   v->count++;
}

static ast_Decl* struct_func_list_List_getDecl(struct_func_list_List* v, uint32_t index)
{
   return v->data[index].decl;
}

static void struct_func_list_List_addFunc(struct_func_list_List* v, uint32_t index, ast_FunctionDecl* fn)
{
   assert(index < v->count);
   struct_func_list_Info* info = &v->data[index];
   ast_FunctionDeclList_add(&info->functions, fn);
}


// --- module c2module_loader ---

typedef struct c2module_loader_CType_ c2module_loader_CType;

struct c2module_loader_CType_ {
   const char* name;
   ast_BuiltinKind kind;
};

static void c2module_loader_create_signed(ast_context_Context* context, ast_AST* a, string_pool_Pool* pool, const char* name, int64_t value, ast_QualType qt, ast_BuiltinKind kind);
static void c2module_loader_create_unsigned(ast_context_Context* context, ast_AST* a, string_pool_Pool* pool, const char* name, uint64_t value, ast_QualType qt, ast_BuiltinKind kind);
static ast_Module* c2module_loader_load(ast_context_Context* context, string_pool_Pool* pool);

static const c2module_loader_CType c2module_loader_CTypes[] = {
   { "c_char", ast_BuiltinKind_Char },
   { "c_uchar", ast_BuiltinKind_UInt8 },
   { "c_short", ast_BuiltinKind_Int16 },
   { "c_ushort", ast_BuiltinKind_UInt16 },
   { "c_int", ast_BuiltinKind_Int32 },
   { "c_uint", ast_BuiltinKind_UInt32 },
   { "c_long", ast_BuiltinKind_Int64 },
   { "c_ulong", ast_BuiltinKind_UInt64 },
   { "c_size", ast_BuiltinKind_UInt64 },
   { "c_ssize", ast_BuiltinKind_Int64 },
   { "c_longlong", ast_BuiltinKind_Int64 },
   { "c_ulonglong", ast_BuiltinKind_UInt64 },
   { "c_float", ast_BuiltinKind_Float32 },
   { "c_double", ast_BuiltinKind_Float64 }
};

static void c2module_loader_create_signed(ast_context_Context* context, ast_AST* a, string_pool_Pool* pool, const char* name, int64_t value, ast_QualType qt, ast_BuiltinKind kind)
{
   ast_Expr* ie = ((ast_Expr*)(ast_IntegerLiteral_createSignedConstant(context, 0, value, qt)));
   uint32_t name2 = string_pool_Pool_addStr(pool, name, true);
   ast_TypeRefHolder ref;
   ast_TypeRefHolder_init(&ref);
   ast_TypeRefHolder_setBuiltin(&ref, kind);
   ast_VarDecl* var = ast_VarDecl_create(context, ast_VarDeclKind_GlobalVar, name2, 0, true, &ref, ast_AST_getIdx(a), ie);
   ast_Decl* d = ((ast_Decl*)(var));
   ast_QualType_setConst(&qt);
   ast_Decl_setType(d, qt);
   ast_Decl_setChecked(d);
   ast_AST_addVarDecl(a, d);
   ast_Module_addSymbol(ast_AST_getMod(a), name2, d);
}

static void c2module_loader_create_unsigned(ast_context_Context* context, ast_AST* a, string_pool_Pool* pool, const char* name, uint64_t value, ast_QualType qt, ast_BuiltinKind kind)
{
   ast_Expr* ie = ((ast_Expr*)(ast_IntegerLiteral_createUnsignedConstant(context, 0, value, qt)));
   uint32_t name2 = string_pool_Pool_addStr(pool, name, true);
   ast_TypeRefHolder ref;
   ast_TypeRefHolder_init(&ref);
   ast_TypeRefHolder_setBuiltin(&ref, kind);
   ast_VarDecl* var = ast_VarDecl_create(context, ast_VarDeclKind_GlobalVar, name2, 0, true, &ref, ast_AST_getIdx(a), ie);
   ast_Decl* d = ((ast_Decl*)(var));
   ast_QualType_setConst(&qt);
   ast_Decl_setType(d, qt);
   ast_Decl_setChecked(d);
   ast_AST_addVarDecl(a, d);
   ast_Module_addSymbol(ast_AST_getMod(a), name2, d);
}

static ast_Module* c2module_loader_load(ast_context_Context* context, string_pool_Pool* pool)
{
   uint32_t name = string_pool_Pool_add(pool, "c2", 2, true);
   ast_Module* m = ast_Module_create(context, name);
   ast_Module_setUsed(m);
   ast_AST* a = ast_Module_add(m, "<generated>");
   for (uint32_t i = 0; i < ARRAY_SIZE(c2module_loader_CTypes); i++) {
      uint32_t type_name = string_pool_Pool_addStr(pool, c2module_loader_CTypes[i].name, true);
      ast_QualType qt = { };
      switch (c2module_loader_CTypes[i].kind) {
      case ast_BuiltinKind_Char:
         qt = ast_g_char;
         break;
      case ast_BuiltinKind_Int8:
         qt = ast_g_i8;
         break;
      case ast_BuiltinKind_Int16:
         qt = ast_g_i16;
         break;
      case ast_BuiltinKind_Int32:
         qt = ast_g_i32;
         break;
      case ast_BuiltinKind_Int64:
         qt = ast_g_i64;
         break;
      case ast_BuiltinKind_UInt8:
         qt = ast_g_u8;
         break;
      case ast_BuiltinKind_UInt16:
         qt = ast_g_u16;
         break;
      case ast_BuiltinKind_UInt32:
         qt = ast_g_u32;
         break;
      case ast_BuiltinKind_UInt64:
         qt = ast_g_u64;
         break;
      case ast_BuiltinKind_Float32:
         qt = ast_g_f32;
         break;
      case ast_BuiltinKind_Float64:
         qt = ast_g_f64;
         break;
      default:
         assert(0);
         break;
      }
      ast_TypeRefHolder ref;
      ast_TypeRefHolder_init(&ref);
      ast_TypeRefHolder_setBuiltin(&ref, c2module_loader_CTypes[i].kind);
      ast_AliasTypeDecl* t = ast_AliasTypeDecl_create(context, type_name, 0, true, ast_AST_getIdx(a), &ref);
      ast_Decl* d = ast_AliasTypeDecl_asDecl(t);
      ast_Decl_setType(d, qt);
      ast_QualType qt2 = ast_Decl_getType(d);
      ast_QualType_setCanonicalType(&qt2, qt);
      ast_Decl_setChecked(d);
      ast_AST_addTypeDecl(a, d);
      ast_Module_addSymbol(m, type_name, d);
   }
   c2module_loader_create_signed(context, a, pool, "min_i8", -128, ast_g_i8, ast_BuiltinKind_Int8);
   c2module_loader_create_signed(context, a, pool, "max_i8", 127, ast_g_i8, ast_BuiltinKind_Int8);
   c2module_loader_create_unsigned(context, a, pool, "min_u8", 0, ast_g_u8, ast_BuiltinKind_UInt8);
   c2module_loader_create_unsigned(context, a, pool, "max_u8", 255, ast_g_u8, ast_BuiltinKind_UInt8);
   c2module_loader_create_signed(context, a, pool, "min_i16", -32768, ast_g_i16, ast_BuiltinKind_Int16);
   c2module_loader_create_signed(context, a, pool, "max_i16", 32767, ast_g_i16, ast_BuiltinKind_Int16);
   c2module_loader_create_unsigned(context, a, pool, "min_u16", 0, ast_g_u16, ast_BuiltinKind_UInt16);
   c2module_loader_create_unsigned(context, a, pool, "max_u16", 65535, ast_g_u16, ast_BuiltinKind_UInt16);
   c2module_loader_create_signed(context, a, pool, "min_i32", -2147483648, ast_g_i32, ast_BuiltinKind_Int32);
   c2module_loader_create_signed(context, a, pool, "max_i32", 2147483647, ast_g_i32, ast_BuiltinKind_Int32);
   c2module_loader_create_unsigned(context, a, pool, "min_u32", 0, ast_g_u32, ast_BuiltinKind_UInt32);
   c2module_loader_create_unsigned(context, a, pool, "max_u32", 4294967295, ast_g_u32, ast_BuiltinKind_UInt32);
   c2module_loader_create_signed(context, a, pool, "min_i64", -9223372036854775807lu, ast_g_i64, ast_BuiltinKind_Int64);
   c2module_loader_create_signed(context, a, pool, "max_i64", 9223372036854775807lu, ast_g_i64, ast_BuiltinKind_Int64);
   c2module_loader_create_unsigned(context, a, pool, "min_u64", 0, ast_g_u64, ast_BuiltinKind_UInt64);
   c2module_loader_create_unsigned(context, a, pool, "max_u64", 18446744073709551615lu, ast_g_u64, ast_BuiltinKind_UInt64);
   c2module_loader_create_signed(context, a, pool, "min_isize", -9223372036854775807lu, ast_g_i64, ast_BuiltinKind_Int64);
   c2module_loader_create_signed(context, a, pool, "max_isize", 9223372036854775807lu, ast_g_i64, ast_BuiltinKind_Int64);
   c2module_loader_create_unsigned(context, a, pool, "min_usize", 0, ast_g_u64, ast_BuiltinKind_UInt64);
   c2module_loader_create_unsigned(context, a, pool, "max_usize", 18446744073709551615lu, ast_g_u64, ast_BuiltinKind_UInt64);
   return m;
}


// --- module module_list ---

typedef struct module_list_ModList_ module_list_ModList;

struct module_list_ModList_ {
   ast_Module** mods;
   uint32_t num_mods;
   uint32_t max_mods;
   bool owns_modules;
};

static module_list_ModList* module_list_create(bool owns_modules);
static void module_list_ModList_free(module_list_ModList* l);
static ast_Module* module_list_ModList_find(const module_list_ModList* l, uint32_t modname_idx);
static void module_list_ModList_add(module_list_ModList* list, ast_Module* m);
static void module_list_ModList_resize(module_list_ModList* l, uint32_t cap);

static module_list_ModList* module_list_create(bool owns_modules)
{
   module_list_ModList* l = calloc(1, sizeof(module_list_ModList));
   l->owns_modules = owns_modules;
   module_list_ModList_resize(l, 4);
   return l;
}

static void module_list_ModList_free(module_list_ModList* l)
{
   if (l->owns_modules) {
      for (uint32_t i = 0; i < l->num_mods; i++) {
         ast_Module_free(l->mods[i]);
      }
   }
   free(((void*)(l->mods)));
   free(l);
}

static ast_Module* module_list_ModList_find(const module_list_ModList* l, uint32_t modname_idx)
{
   for (uint32_t i = 0; i < l->num_mods; i++) {
      if (ast_Module_getNameIdx(l->mods[i]) == modname_idx) return l->mods[i];

   }
   return NULL;
}

static void module_list_ModList_add(module_list_ModList* list, ast_Module* m)
{
   if (list->num_mods == list->max_mods) module_list_ModList_resize(list, list->max_mods * 2);
   list->mods[list->num_mods] = m;
   list->num_mods++;
}

static void module_list_ModList_resize(module_list_ModList* l, uint32_t cap)
{
   l->max_mods = cap;
   void* buf = malloc(l->max_mods * sizeof(ast_Module*));
   if (l->mods) {
      void* old = ((void*)(l->mods));
      memcpy(buf, old, l->num_mods * sizeof(ast_Module*));
      free(old);
   }
   l->mods = buf;
}


// --- module scope ---

typedef struct scope_Level_ scope_Level;
typedef struct scope_Scope_ scope_Scope;

struct scope_Level_ {
   uint32_t flags;
   uint32_t first_index;
};

struct scope_Scope_ {
   const module_list_ModList* allmodules;
   diagnostics_Diags* diags;
   const ast_ImportDeclList* imports;
   const ast_Module* mod;
   const ast_SymbolTable* symbols;
   bool warn_on_unused;
   ast_SymbolTable local_scope;
   scope_Level levels[32];
   uint32_t lvl;
};

static scope_Scope* scope_create(module_list_ModList* allmodules, diagnostics_Diags* diags, const ast_ImportDeclList* imports, ast_Module* mod, const ast_SymbolTable* symbols, bool warn_on_unused);
static void scope_Scope_free(scope_Scope* s);
static void scope_Scope_reset(scope_Scope* s);
static void scope_Scope_enter(scope_Scope* s, uint32_t flags);
static void scope_Scope_exit(scope_Scope* s);
static bool scope_Scope_allowBreak(const scope_Scope* s);
static bool scope_Scope_allowContinue(const scope_Scope* s);
static bool scope_Scope_allowFallthrough(const scope_Scope* s);
static bool scope_Scope_add(scope_Scope* s, ast_Decl* d);
static ast_Decl* scope_Scope_find(scope_Scope* s, uint32_t name_idx, src_loc_SrcLoc loc);
static bool scope_Scope_checkGlobalSymbol(scope_Scope* s, uint32_t name_idx, src_loc_SrcLoc loc);
static ast_ImportDecl* scope_Scope_findModule(scope_Scope* s, uint32_t name_idx, src_loc_SrcLoc loc);
static ast_Decl* scope_Scope_findGlobalSymbolInModule(scope_Scope* s, ast_Module* mod, uint32_t name_idx, src_loc_SrcLoc loc);
static ast_Decl* scope_Scope_findType(scope_Scope* s, uint32_t name_idx, src_loc_SrcLoc loc);
static ast_Decl* scope_Scope_findGlobalSymbol(scope_Scope* s, uint32_t name_idx, src_loc_SrcLoc loc);
static void scope_Scope_dump(const scope_Scope* s);

static const uint32_t scope_MaxLevels = 32;

static const uint32_t scope_Function = 0x1;

static const uint32_t scope_Break = 0x2;

static const uint32_t scope_Continue = 0x4;

static const uint32_t scope_Decl = 0x8;

static const uint32_t scope_Control = 0x10;

static const uint32_t scope_Block = 0x20;

static const uint32_t scope_Fallthrough = 0x40;

static scope_Scope* scope_create(module_list_ModList* allmodules, diagnostics_Diags* diags, const ast_ImportDeclList* imports, ast_Module* mod, const ast_SymbolTable* symbols, bool warn_on_unused)
{
   scope_Scope* s = calloc(1, sizeof(scope_Scope));
   s->allmodules = allmodules;
   s->diags = diags;
   s->imports = imports;
   s->mod = mod;
   s->symbols = symbols;
   s->warn_on_unused = warn_on_unused;
   ast_SymbolTable_init(&s->local_scope, 64);
   return s;
}

static void scope_Scope_free(scope_Scope* s)
{
   ast_SymbolTable_free(&s->local_scope);
   free(s);
}

static void scope_Scope_reset(scope_Scope* s)
{
   s->lvl = 0;
   ast_SymbolTable_crop(&s->local_scope, 0);
}

static void scope_Scope_enter(scope_Scope* s, uint32_t flags)
{
   if (s->lvl == scope_MaxLevels) {
      diagnostics_Diags_error(s->diags, 0, "max scope depth reached");
      assert(0);
      return;
   }
   scope_Level* top = &s->levels[s->lvl];
   if (s->lvl) {
      const scope_Level* parent = &s->levels[s->lvl - 1];
      flags |= ((parent->flags & ((scope_Break | scope_Continue))));
   }
   top->flags = flags;
   top->first_index = ast_SymbolTable_size(&s->local_scope);
   s->lvl++;
}

static void scope_Scope_exit(scope_Scope* s)
{
   assert(s->lvl != 0);
   s->lvl--;
   uint32_t first = s->levels[s->lvl].first_index;
   if (s->warn_on_unused) {
      uint32_t last = ast_SymbolTable_size(&s->local_scope);
      ast_Decl** decls = ast_SymbolTable_getDecls(&s->local_scope);
      for (uint32_t i = first; i < last; i++) {
         ast_Decl* d = decls[i];
         if (!ast_Decl_isUsed(d)) {
            ast_VarDecl* vd = ((ast_VarDecl*)(d));
            if (ast_VarDecl_isLocal(vd)) diagnostics_Diags_warn(s->diags, ast_Decl_getLoc(d), "unused variable %s", ast_Decl_getName(d));
         }
      }
   }
   ast_SymbolTable_crop(&s->local_scope, first);
}

static bool scope_Scope_allowBreak(const scope_Scope* s)
{
   assert(s->lvl);
   const scope_Level* top = &s->levels[s->lvl - 1];
   return ((top->flags & scope_Break));
}

static bool scope_Scope_allowContinue(const scope_Scope* s)
{
   assert(s->lvl);
   const scope_Level* top = &s->levels[s->lvl - 1];
   return ((top->flags & scope_Continue));
}

static bool scope_Scope_allowFallthrough(const scope_Scope* s)
{
   assert(s->lvl);
   const scope_Level* top = &s->levels[s->lvl - 1];
   return ((top->flags & scope_Fallthrough));
}

static bool scope_Scope_add(scope_Scope* s, ast_Decl* d)
{
   assert(s->lvl);
   const uint32_t name_idx = ast_Decl_getNameIdx(d);
   ast_Decl* decl = ast_SymbolTable_find(&s->local_scope, name_idx);
   if (decl) return true;

   decl = scope_Scope_findGlobalSymbol(s, name_idx, ast_Decl_getLoc(d));
   if (decl) return true;

   ast_SymbolTable_add(&s->local_scope, name_idx, d);
   return false;
}

static ast_Decl* scope_Scope_find(scope_Scope* s, uint32_t name_idx, src_loc_SrcLoc loc)
{
   ast_Decl* decl = ast_SymbolTable_find(&s->local_scope, name_idx);
   if (decl) return decl;

   decl = scope_Scope_findGlobalSymbol(s, name_idx, loc);
   if (!decl) {
      const char* name = ast_idx2name(name_idx);
      const char* kind = "variable";
      if (isupper(name[0])) kind = "type/constant";
      diagnostics_Diags_error(s->diags, loc, "unknown %s: %s", kind, name);
   }
   return decl;
}

static bool scope_Scope_checkGlobalSymbol(scope_Scope* s, uint32_t name_idx, src_loc_SrcLoc loc)
{
   uint32_t num_imports = ast_ImportDeclList_size(s->imports);
   ast_ImportDecl** imports = ast_ImportDeclList_getDecls(s->imports);
   ast_Decl* decl = NULL;
   for (uint32_t i = 0; i < num_imports; i++) {
      ast_ImportDecl* id = imports[i];
      if (name_idx == ast_ImportDecl_getImportNameIdx(id)) {
         decl = ((ast_Decl*)(id));
         break;
      }
      if (ast_ImportDecl_isLocal(id)) {
         ast_Module* dest = ast_ImportDecl_getDest(id);
         const ast_SymbolTable* symbols = ast_Module_getSymbols(dest);
         decl = ast_SymbolTable_find(symbols, name_idx);
         if (decl) break;

      }
   }
   if (decl) {
      diagnostics_Diags_error(s->diags, loc, "redefinition of '%s'", ast_idx2name(name_idx));
      diagnostics_Diags_note(s->diags, ast_Decl_getLoc(decl), "previous definition is here");
      return false;
   }
   return true;
}

static ast_ImportDecl* scope_Scope_findModule(scope_Scope* s, uint32_t name_idx, src_loc_SrcLoc loc)
{
   assert(s);
   ast_ImportDecl* d = ast_ImportDeclList_find(s->imports, name_idx);
   ast_Decl_setUsed(ast_ImportDecl_asDecl(d));
   if (d) return d;

   d = ast_ImportDeclList_findAny(s->imports, name_idx);
   if (d) {
      diagnostics_Diags_error(s->diags, loc, "module %s imported with alias %s", ast_Decl_getName(ast_ImportDecl_asDecl(d)), ast_ImportDecl_getAliasName(d));
      return NULL;
   }
   ast_Module* mod = module_list_ModList_find(s->allmodules, name_idx);
   if (mod) {
      diagnostics_Diags_error(s->diags, loc, "module %s not imported", ast_idx2name(name_idx));
   } else {
      diagnostics_Diags_error(s->diags, loc, "unknown module '%s'", ast_idx2name(name_idx));
   }
   return NULL;
}

static ast_Decl* scope_Scope_findGlobalSymbolInModule(scope_Scope* s, ast_Module* mod, uint32_t name_idx, src_loc_SrcLoc loc)
{
   assert(s);
   ast_Decl* d = ast_SymbolTable_find(ast_Module_getSymbols(mod), name_idx);
   if (!d) {
      diagnostics_Diags_error(s->diags, loc, "no symbol '%s' in module %s", ast_idx2name(name_idx), ast_Module_getName(mod));
      return NULL;
   }
   if (mod != s->mod) {
      if (!ast_Decl_isPublic(d)) {
         diagnostics_Diags_error(s->diags, loc, "symbol '%s' is not public", ast_idx2name(name_idx));
         return NULL;
      }
   }
   return d;
}

static ast_Decl* scope_Scope_findType(scope_Scope* s, uint32_t name_idx, src_loc_SrcLoc loc)
{
   assert(s);
   ast_Decl* decl = scope_Scope_findGlobalSymbol(s, name_idx, loc);
   if (!decl) {
      diagnostics_Diags_error(s->diags, loc, "unknown type: %s", ast_idx2name(name_idx));
   }
   return decl;
}

static ast_Decl* scope_Scope_findGlobalSymbol(scope_Scope* s, uint32_t name_idx, src_loc_SrcLoc loc)
{
   ast_Decl* decl = NULL;
   ast_ImportDecl* used_import = NULL;
   bool ambiguous = false;
   bool visible_match = false;
   uint32_t num_imports = ast_ImportDeclList_size(s->imports);
   ast_ImportDecl** imports = ast_ImportDeclList_getDecls(s->imports);
   for (uint32_t i = 0; i < num_imports; i++) {
      ast_ImportDecl* id = imports[i];
      if (name_idx == ast_ImportDecl_getImportNameIdx(id)) {
         decl = ((ast_Decl*)(id));
         used_import = id;
         visible_match = true;
         continue;
      }
      if (ast_ImportDecl_isLocal(id)) {
         ast_Module* dest = ast_ImportDecl_getDest(id);
         const ast_SymbolTable* symbols = ast_Module_getSymbols(dest);
         ast_Decl* d = ast_SymbolTable_find(symbols, name_idx);
         if (!d) continue;

         bool visible = !(((s->mod != dest) && !ast_Decl_isPublic(d)));
         if (decl) {
            if (visible_match == visible) {
               const char* name = ast_idx2name(name_idx);
               const char* mod2_name = ast_idx2name(ast_ImportDecl_getImportNameIdx(id));
               if (!ambiguous) {
                  diagnostics_Diags_error(s->diags, loc, "symbol %s is ambiguous", name);
                  const char* mod1_name = ast_idx2name(ast_ImportDecl_getImportNameIdx(used_import));
                  diagnostics_Diags_note(s->diags, loc, "did you mean %s.%s or %s.%s?", mod1_name, name, mod2_name, name);
                  ambiguous = true;
               } else {
                  diagnostics_Diags_error(s->diags, loc, "did you mean %s.%s?", mod2_name, name);
               }
               continue;
            }
            if (!visible_match) {
               decl = d;
               used_import = id;
               visible_match = visible;
            }
         } else {
            decl = d;
            used_import = id;
            visible_match = visible;
         }
      }
   }
   if (ambiguous) return NULL;

   if (decl) {
      bool external = (ast_ImportDecl_getDest(used_import) != s->mod);
      if (external) ast_Decl_setUsed(ast_ImportDecl_asDecl(used_import));
      if (!visible_match) {
         diagnostics_Diags_error(s->diags, loc, "symbol %s is not public", ast_idx2name(name_idx));
         return NULL;
      }
      if (external) ast_Decl_setUsedPublic(decl);
   }
   return decl;
}

static void scope_Scope_dump(const scope_Scope* s)
{
   printf("Scope (lvl %u) %u\n", s->lvl, ast_SymbolTable_size(&s->local_scope));
   for (uint32_t i = 0; i < s->lvl; i++) {
      const scope_Level* l = &s->levels[i];
      printf("  [%u]  start %2u  flags 0x%02x\n", i, l->first_index, l->flags);
   }
}


// --- module component ---

typedef struct component_Component_ component_Component;

struct component_Component_ {
   const char* name;
   bool is_external;
   ast_context_Context* context;
   ast_Module** mods;
   uint32_t num_mods;
   uint32_t max_mods;
   module_list_ModList* globalList;
};

typedef void (*component_ModuleVisitor)(void*, ast_Module*);

static component_Component* component_create(ast_context_Context* context, module_list_ModList* globalList, const char* name, bool is_external);
static void component_Component_free(component_Component* c);
static const char* component_Component_getName(const component_Component* c);
static bool component_Component_isExternal(const component_Component* c);
static void component_Component_visitModules(const component_Component* c, component_ModuleVisitor visitor, void* arg);
static ast_Module* component_Component_getModule(const component_Component* c, uint32_t idx);
static ast_Module** component_Component_getModules(component_Component* c);
static uint32_t component_Component_getNumModules(const component_Component* c);
static void component_Component_resize(component_Component* c, uint32_t cap);
static ast_Module* component_Component_getOrAddModule(component_Component* c, uint32_t name_idx);
static bool component_Component_hasModule(const component_Component* c, const ast_Module* mod);
static ast_Module* component_Component_getTopModule(component_Component* c);
static void component_Component_info(const component_Component* c);
static void component_Component_print(const component_Component* c, bool show_funcs);
static void component_Component_printModules(const component_Component* c);
static void component_Component_printSymbols(const component_Component* c);

static component_Component* component_create(ast_context_Context* context, module_list_ModList* globalList, const char* name, bool is_external)
{
   component_Component* c = calloc(1, sizeof(component_Component));
   c->name = name;
   c->is_external = is_external;
   c->context = context;
   component_Component_resize(c, 4);
   c->globalList = globalList;
   return c;
}

static void component_Component_free(component_Component* c)
{
   for (uint32_t i = 0; i < c->num_mods; i++) {
      ast_Module_free(c->mods[i]);
   }
   free(((void*)(c->mods)));
   free(c);
}

static const char* component_Component_getName(const component_Component* c)
{
   return c->name;
}

static bool component_Component_isExternal(const component_Component* c)
{
   return c->is_external;
}

static void component_Component_visitModules(const component_Component* c, component_ModuleVisitor visitor, void* arg)
{
   for (uint32_t i = 0; i < c->num_mods; i++) {
      visitor(arg, c->mods[i]);
   }
}

static ast_Module* component_Component_getModule(const component_Component* c, uint32_t idx)
{
   return c->mods[idx];
}

static ast_Module** component_Component_getModules(component_Component* c)
{
   return c->mods;
}

static uint32_t component_Component_getNumModules(const component_Component* c)
{
   return c->num_mods;
}

static void component_Component_resize(component_Component* c, uint32_t cap)
{
   c->max_mods = cap;
   void* buf = malloc(c->max_mods * sizeof(ast_Module*));
   if (c->mods) {
      void* old = ((void*)(c->mods));
      memcpy(buf, old, c->num_mods * sizeof(ast_Module*));
      free(old);
   }
   c->mods = buf;
}

static ast_Module* component_Component_getOrAddModule(component_Component* c, uint32_t name_idx)
{
   for (uint32_t i = 0; i < c->num_mods; i++) {
      ast_Module* m = c->mods[i];
      if (ast_Module_getNameIdx(m) == name_idx) return m;

   }
   ast_Module* m = module_list_ModList_find(c->globalList, name_idx);
   if (m) return NULL;

   m = ast_Module_create(c->context, name_idx);
   if (c->num_mods == c->max_mods) component_Component_resize(c, c->max_mods * 2);
   module_list_ModList_add(c->globalList, m);
   c->mods[c->num_mods] = m;
   c->num_mods++;
   return m;
}

static bool component_Component_hasModule(const component_Component* c, const ast_Module* mod)
{
   for (uint32_t i = 0; i < c->num_mods; i++) {
      if (c->mods[i] == mod) return true;

   }
   return false;
}

static ast_Module* component_Component_getTopModule(component_Component* c)
{
   if (c->num_mods == 0) return NULL;

   return c->mods[c->num_mods - 1];
}

static void component_Component_info(const component_Component* c)
{
   string_buffer_Buf* out = string_buffer_create(64 * 1024, utils_useColor(), 1);
   string_buffer_Buf_print(out, "component %s\n", c->name);
   for (uint32_t i = 0; i < c->num_mods; i++) {
      ast_Module_info(c->mods[i], out);
   }
   printf("%s\n", string_buffer_Buf_data(out));
   string_buffer_Buf_free(out);
}

static void component_Component_print(const component_Component* c, bool show_funcs)
{
   string_buffer_Buf* out = string_buffer_create(128 * 1024, utils_useColor(), 1);
   for (uint32_t i = 0; i < c->num_mods; i++) {
      ast_Module_print(c->mods[i], out, show_funcs);
   }
   string_buffer_Buf_color(out, color_Normal);
   puts(string_buffer_Buf_data(out));
   string_buffer_Buf_free(out);
}

static void component_Component_printModules(const component_Component* c)
{
   string_buffer_Buf* out = string_buffer_create(4096, utils_useColor(), 1);
   string_buffer_Buf_print(out, "--- %s ---\n", c->name);
   for (uint32_t i = 0; i < c->num_mods; i++) {
      const ast_Module* m = c->mods[i];
      const char* col = ast_Module_isUsed(m) ? color_Normal : color_Grey;
      string_buffer_Buf_color(out, col);
      string_buffer_Buf_print(out, "   %s\n", ast_Module_getName(m));
   }
   string_buffer_Buf_color(out, color_Normal);
   puts(string_buffer_Buf_data(out));
   string_buffer_Buf_free(out);
}

static void component_Component_printSymbols(const component_Component* c)
{
   string_buffer_Buf* out = string_buffer_create(4096, utils_useColor(), 1);
   string_buffer_Buf_print(out, "--- %s ---\n", c->name);
   for (uint32_t i = 0; i < c->num_mods; i++) {
      string_buffer_Buf_color(out, color_Cyan);
      string_buffer_Buf_print(out, "  %s\n", ast_Module_getName(c->mods[i]));
      const ast_SymbolTable* table = ast_Module_getSymbols(c->mods[i]);
      ast_SymbolTable_print(table, out);
   }
   string_buffer_Buf_color(out, color_Normal);
   puts(string_buffer_Buf_data(out));
   string_buffer_Buf_free(out);
}


// --- module manifest ---


static const yaml_Node* manifest_get_checked(yaml_Parser* parser, const char* path);
static bool manifest_getYamlInfoInfo(yaml_Parser* parser, string_pool_Pool* astPool, component_Component* comp);
static void manifest_parse(source_mgr_SourceMgr* sm, int32_t file_id, string_pool_Pool* astPool, component_Component* comp);

static const yaml_Node* manifest_get_checked(yaml_Parser* parser, const char* path)
{
   const yaml_Node* node = yaml_Parser_findNode(parser, path);
   if (!node) {
      fprintf(stderr, "missing node %s\n", path);
      exit(-1);
   }
   return node;
}

static bool manifest_getYamlInfoInfo(yaml_Parser* parser, string_pool_Pool* astPool, component_Component* comp)
{
   const yaml_Node* lib_lang = manifest_get_checked(parser, "info.language");
   const yaml_Node* lib_type = manifest_get_checked(parser, "info.type");
   const yaml_Node* lib_kinds = manifest_get_checked(parser, "info.kinds");
   const yaml_Node* modulesNode = manifest_get_checked(parser, "modules");
   yaml_Iter iter = yaml_Parser_getNodeChildIter(parser, modulesNode);
   while (!yaml_Iter_done(&iter)) {
      const char* value = yaml_Iter_getValue(&iter);
      assert(value);
      size_t len = strlen(value);
      uint32_t modname = string_pool_Pool_add(astPool, value, len, 1);
      component_Component_getOrAddModule(comp, modname);
      yaml_Iter_next(&iter);
   }
   return true;
}

static void manifest_parse(source_mgr_SourceMgr* sm, int32_t file_id, string_pool_Pool* astPool, component_Component* comp)
{
   const char* data = source_mgr_SourceMgr_get_content(sm, file_id);
   uint32_t loc_start = source_mgr_SourceMgr_get_offset(sm, file_id);
   yaml_Parser* parser = yaml_Parser_create();
   bool ok = yaml_Parser_parse(parser, ((char*)(data)));
   if (ok) {
      manifest_getYamlInfoInfo(parser, astPool, comp);
   } else {
      fprintf(stderr, "Error: %s\n", yaml_Parser_getMessage(parser));
   }
   yaml_Parser_destroy(parser);
}


// --- module refs_generator ---

typedef struct refs_generator_Generator_ refs_generator_Generator;

struct refs_generator_Generator_ {
   source_mgr_SourceMgr* sm;
   refs_Refs* refs;
   const char* curfile;
};

static void refs_generator_Generator_handleExpr(refs_generator_Generator* gen, const ast_Expr* e);
static void refs_generator_Generator_on_decl(void* arg, ast_Decl* d);
static void refs_generator_Generator_handleTypeRef(refs_generator_Generator* gen, const ast_TypeRef* ref);
static void refs_generator_Generator_handleRef(refs_generator_Generator* gen, const ast_Ref* ref);
static void refs_generator_Generator_on_ast(void* arg, ast_AST* a);
static void refs_generator_Generator_on_module(void* arg, ast_Module* m);
static void refs_generator_generate(source_mgr_SourceMgr* sm, const char* output_dir, component_Component** c, uint32_t count);

static void refs_generator_Generator_handleExpr(refs_generator_Generator* gen, const ast_Expr* e)
{
   switch (ast_Expr_getKind(e)) {
   case ast_ExprKind_Identifier: {
      const ast_IdentifierExpr* i = ((ast_IdentifierExpr*)(e));
      ast_Ref ref = ast_IdentifierExpr_getRef(i);
      refs_generator_Generator_handleRef(gen, &ref);
      break;
   }
   default:
      break;
   }
}

static void refs_generator_Generator_on_decl(void* arg, ast_Decl* d)
{
   refs_generator_Generator* gen = arg;
   switch (ast_Decl_getKind(d)) {
   case ast_DeclKind_Function: {
      ast_FunctionDecl* fd = ((ast_FunctionDecl*)(d));
      refs_generator_Generator_handleTypeRef(gen, ast_FunctionDecl_getTypeRef(fd));
      uint32_t num_params = ast_FunctionDecl_getNumParams(fd);
      ast_VarDecl** params = ast_FunctionDecl_getParams(fd);
      for (uint32_t i = 0; i < num_params; i++) {
         refs_generator_Generator_on_decl(gen, ((ast_Decl*)(params[i])));
      }
      break;
   }
   case ast_DeclKind_Import:
      return;
   case ast_DeclKind_StructType: {
      ast_StructTypeDecl* s = ((ast_StructTypeDecl*)(d));
      uint32_t num_members = ast_StructTypeDecl_getNumMembers(s);
      ast_Decl** members = ast_StructTypeDecl_getMembers(s);
      for (uint32_t i = 0; i < num_members; i++) {
         refs_generator_Generator_on_decl(arg, members[i]);
      }
      break;
   }
   case ast_DeclKind_EnumType: {
      ast_EnumTypeDecl* etd = ((ast_EnumTypeDecl*)(d));
      ast_EnumConstantDecl** constants = ast_EnumTypeDecl_getConstants(etd);
      for (uint32_t i = 0; i < ast_EnumTypeDecl_getNumConstants(etd); i++) {
         refs_generator_Generator_on_decl(arg, ((ast_Decl*)(constants[i])));
      }
      break;
   }
   case ast_DeclKind_EnumConstant:
      break;
   case ast_DeclKind_FunctionType:
      break;
   case ast_DeclKind_AliasType:
      break;
   case ast_DeclKind_Var: {
      ast_VarDecl* v = ((ast_VarDecl*)(d));
      refs_generator_Generator_handleTypeRef(gen, ast_VarDecl_getTypeRef(v));
      const ast_Expr* i = ast_VarDecl_getInit(v);
      if (i) refs_generator_Generator_handleExpr(gen, i);
      break;
   }
   case ast_DeclKind_StaticAssert:
      return;
   }
   const char* name = ast_Decl_getName(d);
   if (!name) return;

   source_mgr_Location loc = source_mgr_SourceMgr_getLocation(gen->sm, ast_Decl_getLoc(d));
   refs_Dest dest = { gen->curfile, loc.line, ((uint16_t)(loc.column)) };
   refs_Refs_add_symbol(gen->refs, ast_Decl_getName(d), &dest);
}

static void refs_generator_Generator_handleTypeRef(refs_generator_Generator* gen, const ast_TypeRef* ref)
{
   const ast_Ref* prefix = ast_TypeRef_getPrefix(ref);
   if (prefix) refs_generator_Generator_handleRef(gen, prefix);
   const ast_Ref* user = ast_TypeRef_getUser(ref);
   if (user) refs_generator_Generator_handleRef(gen, user);
}

static void refs_generator_Generator_handleRef(refs_generator_Generator* gen, const ast_Ref* ref)
{
   if (!ref->decl) {
      printf("refs: Decl not set!\n");
      return;
   }
   source_mgr_Location src_loc = source_mgr_SourceMgr_getLocation(gen->sm, ref->loc);
   source_mgr_Location dst_loc = source_mgr_SourceMgr_getLocation(gen->sm, ast_Decl_getLoc(ref->decl));
   uint16_t len = ((uint16_t)(strlen(ast_idx2name(ref->name_idx))));
   refs_RefSrc src = { src_loc.line, ((uint16_t)(src_loc.column)), len };
   refs_Dest dest = { dst_loc.filename, dst_loc.line, ((uint16_t)(dst_loc.column)) };
   refs_Refs_add_tag(gen->refs, &src, &dest);
}

static void refs_generator_Generator_on_ast(void* arg, ast_AST* a)
{
   refs_generator_Generator* gen = arg;
   gen->curfile = ast_AST_getFilename(a);
   refs_Refs_add_file(gen->refs, ast_AST_getFilename(a));
   ast_AST_visitDecls(a, refs_generator_Generator_on_decl, arg);
}

static void refs_generator_Generator_on_module(void* arg, ast_Module* m)
{
   ast_Module_visitASTs(m, refs_generator_Generator_on_ast, arg);
}

static void refs_generator_generate(source_mgr_SourceMgr* sm, const char* output_dir, component_Component** c, uint32_t count)
{
   refs_generator_Generator gen = { sm, refs_Refs_create(), NULL };
   for (uint32_t i = 0; i < count; i++) {
      component_Component_visitModules(c[i], refs_generator_Generator_on_module, &gen);
   }
   char outfile[128];
   sprintf(outfile, "%s/%s", output_dir, "refs");
   refs_Refs_write(gen.refs, outfile);
   refs_Refs_free(gen.refs);
}


// --- module c_generator ---

typedef struct c_generator_Fragment_ c_generator_Fragment;
typedef struct c_generator_Generator_ c_generator_Generator;



struct c_generator_Fragment_ {
   string_buffer_Buf* buf;
   linked_list_Element list;
};

struct c_generator_Generator_ {
   string_buffer_Buf* out;
   const char* target;
   const char* output_dir;
   bool cur_external;
   ast_Decl* mainFunc;
   uint32_t stdargName;
   const char* mod_name;
   ast_Module* mod;
   linked_list_Element free_list;
   linked_list_Element used_list;
};

static c_generator_Fragment* c_generator_Fragment_create(void);
static void c_generator_Fragment_clear(c_generator_Fragment* f);
static void c_generator_Fragment_free(c_generator_Fragment* f);
static c_generator_Fragment* c_generator_Generator_getFragment(c_generator_Generator* gen);
static void c_generator_Generator_addFragment(c_generator_Generator* gen, c_generator_Fragment* f);
static void c_generator_Generator_freeFragment(c_generator_Generator* gen, c_generator_Fragment* f);
static void c_generator_Generator_emitCtv(c_generator_Generator* _arg0, string_buffer_Buf* out, const ast_Expr* e);
static void c_generator_Generator_emitCName(c_generator_Generator* gen, string_buffer_Buf* out, const ast_Decl* d);
static void c_generator_Generator_emitCNameMod(c_generator_Generator* _arg0, string_buffer_Buf* out, const ast_Decl* d, ast_Module* mod);
static void c_generator_Generator_emitDeclName(c_generator_Generator* gen, string_buffer_Buf* out, const ast_Decl* d);
static void c_generator_Generator_emitEnum(c_generator_Generator* gen, string_buffer_Buf* out, ast_Decl* d);
static void c_generator_Generator_emitTypePre(c_generator_Generator* gen, string_buffer_Buf* out, ast_QualType qt);
static void c_generator_Generator_emitTypePost(c_generator_Generator* gen, string_buffer_Buf* out, ast_QualType qt);
static void c_generator_Generator_genDeclIfNeeded(c_generator_Generator* gen, ast_Decl* d);
static void c_generator_Generator_genTypeIfNeeded(c_generator_Generator* gen, ast_QualType qt, bool full);
static void c_generator_Generator_emitStructMember(c_generator_Generator* gen, string_buffer_Buf* out, ast_Decl* d, uint32_t indent);
static void c_generator_Generator_emitStruct(c_generator_Generator* gen, string_buffer_Buf* out, ast_Decl* d, uint32_t indent);
static void c_generator_Generator_emitFunctionType(c_generator_Generator* gen, string_buffer_Buf* out, ast_Decl* d);
static void c_generator_Generator_emitAliasType(c_generator_Generator* gen, string_buffer_Buf* out, ast_Decl* d);
static void c_generator_Generator_emitGlobalVarDecl(c_generator_Generator* gen, string_buffer_Buf* out, ast_Decl* d);
static void c_generator_Generator_on_forward_structs(void* arg, ast_Decl* d);
static void c_generator_Generator_emitGlobalDecl(c_generator_Generator* gen, ast_Decl* d);
static void c_generator_Generator_flattenFragments(c_generator_Generator* gen);
static void c_generator_Generator_on_decl(void* arg, ast_Decl* d);
static void c_generator_Generator_on_vardecl(void* arg, ast_VarDecl* vd);
static void c_generator_Generator_on_ast_types(void* arg, ast_AST* a);
static void c_generator_Generator_gen_func_proto(c_generator_Generator* gen, ast_FunctionDecl* fd, string_buffer_Buf* out);
static void c_generator_Generator_gen_func_forward_decl(void* arg, ast_FunctionDecl* fd);
static void c_generator_Generator_gen_func_protos(void* arg, ast_AST* a);
static void c_generator_Generator_emitFunction(c_generator_Generator* gen, ast_FunctionDecl* fd);
static void c_generator_Generator_gen_full_func(void* arg, ast_FunctionDecl* fd);
static void c_generator_Generator_gen_full_funcs(void* arg, ast_AST* a);
static void c_generator_Generator_on_ast_vars(void* arg, ast_AST* a);
static void c_generator_Generator_on_ast_structs(void* arg, ast_AST* a);
static void c_generator_Generator_decl_mark_generated(void* _arg0, ast_Decl* d);
static void c_generator_Generator_ast_mark_generated(void* arg, ast_AST* a);
static void c_generator_Generator_on_module(void* arg, ast_Module* m);
static void c_generator_Generator_init(c_generator_Generator* gen, const char* target, const char* output_dir, ast_Decl* mainFunc, uint32_t stdargName);
static void c_generator_Generator_free(c_generator_Generator* gen);
static void c_generator_Generator_write(c_generator_Generator* gen, const char* output_dir, const char* filename);
static void c_generator_Generator_createMakefile(c_generator_Generator* gen, const char* output_dir);
static void c_generator_generate(const char* target, const char* output_dir, ast_Module* c2mod, component_Component** comps, uint32_t count, ast_Decl* mainFunc, uint32_t stdargName, bool print_code);
static void c_generator_build(const char* output_dir);
static void c_generator_Generator_emitExpr(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e);
static void c_generator_Generator_emitBinaryOperator(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e);
static void c_generator_Generator_emitUnaryOperator(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e);
static void c_generator_emitDotOrArrow(string_buffer_Buf* out, ast_QualType qt);
static void c_generator_Generator_emitMemberExpr(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e);
static void c_generator_Generator_emitMemberExprBase(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e);
static void c_generator_Generator_emitFieldDesigExpr(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e);
static void c_generator_Generator_emitArrayDesigExpr(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e);
static void c_generator_Generator_emitCall(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e);
static void c_generator_Generator_emitBuiltinExpr(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e);
static void c_generator_Generator_emitStmt(c_generator_Generator* gen, ast_Stmt* s, uint32_t indent, bool newline);
static void c_generator_Generator_emitSwitchStmt(c_generator_Generator* gen, ast_Stmt* s, uint32_t indent);

static const char c_generator_Dir[] = "cgen";

static const char c_generator_LogFile[] = "build.log";

static const char c_generator_Filename[] = "main.c";

static const char* c_generator_builtinType_cnames[] = {
   "char",
   "int8_t",
   "int16_t",
   "int32_t",
   "int64_t",
   "uint8_t",
   "uint16_t",
   "uint32_t",
   "uint64_t",
   "float",
   "double",
   "ssize_t",
   "size_t",
   "bool",
   "void"
};

static c_generator_Fragment* c_generator_Fragment_create(void)
{
   c_generator_Fragment* f = malloc(sizeof(c_generator_Fragment));
   f->buf = string_buffer_create(128, false, 3);
   return f;
}

static void c_generator_Fragment_clear(c_generator_Fragment* f)
{
   string_buffer_Buf_clear(f->buf);
}

static void c_generator_Fragment_free(c_generator_Fragment* f)
{
   string_buffer_Buf_free(f->buf);
   free(f);
}

static c_generator_Fragment* c_generator_Generator_getFragment(c_generator_Generator* gen)
{
   if (linked_list_Element_isEmpty(&gen->free_list)) {
      return c_generator_Fragment_create();
   }
   linked_list_Element* e = linked_list_Element_popFront(&gen->free_list);
   c_generator_Fragment* f = to_container(c_generator_Fragment, list, e);
   c_generator_Fragment_clear(f);
   return f;
}

static void c_generator_Generator_addFragment(c_generator_Generator* gen, c_generator_Fragment* f)
{
   linked_list_Element_addTail(&gen->used_list, &f->list);
}

static void c_generator_Generator_freeFragment(c_generator_Generator* gen, c_generator_Fragment* f)
{
   linked_list_Element_addTail(&gen->free_list, &f->list);
}

static void c_generator_Generator_emitCtv(c_generator_Generator* _arg0, string_buffer_Buf* out, const ast_Expr* e)
{
   ctv_analyser_Value val = ctv_analyser_get_value(e);
   if (val.is_signed) string_buffer_Buf_print(out, "%ld", val.svalue);
   else string_buffer_Buf_print(out, "%lu", val.uvalue);
}

static void c_generator_Generator_emitCName(c_generator_Generator* gen, string_buffer_Buf* out, const ast_Decl* d)
{
   c_generator_Generator_emitCNameMod(gen, out, d, gen->mod);
}

static void c_generator_Generator_emitCNameMod(c_generator_Generator* _arg0, string_buffer_Buf* out, const ast_Decl* d, ast_Module* mod)
{
   if (ast_Decl_isExternal(d)) {
      const char* cname = ast_Decl_getCName(d);
      if (cname) {
         string_buffer_Buf_add(out, cname);
      } else {
         string_buffer_Buf_add(out, ast_Decl_getName(d));
      }
      return;
   }
   string_buffer_Buf_add(out, ast_Module_getName(mod));
   string_buffer_Buf_add1(out, '_');
   if (ast_Decl_getKind(d) == ast_DeclKind_Function) {
      ast_FunctionDecl* fd = ((ast_FunctionDecl*)(d));
      ast_Ref* prefix = ast_FunctionDecl_getPrefix(fd);
      if (prefix) {
         string_buffer_Buf_add(out, ast_idx2name(prefix->name_idx));
         string_buffer_Buf_add1(out, '_');
      }
   }
   if (ast_Decl_getKind(d) == ast_DeclKind_EnumConstant) {
      ast_QualType qt = ast_Decl_getType(d);
      ast_EnumType* et = ((ast_EnumType*)(ast_QualType_getType(&qt)));
      string_buffer_Buf_add(out, ast_EnumType_getName(et));
      string_buffer_Buf_add1(out, '_');
   }
   string_buffer_Buf_add(out, ast_Decl_getName(d));
}

static void c_generator_Generator_emitDeclName(c_generator_Generator* gen, string_buffer_Buf* out, const ast_Decl* d)
{
   if (ast_Decl_getKind(d) == ast_DeclKind_Var) {
      ast_VarDecl* vd = ((ast_VarDecl*)(d));
      if (!ast_VarDecl_isGlobal(vd)) {
         string_buffer_Buf_add(out, ast_Decl_getName(d));
         return;
      }
   }
   c_generator_Generator_emitCNameMod(gen, out, d, ast_Decl_getModule(d));
}

static void c_generator_Generator_emitEnum(c_generator_Generator* gen, string_buffer_Buf* out, ast_Decl* d)
{
   ast_EnumTypeDecl* etd = ((ast_EnumTypeDecl*)(d));
   string_buffer_Buf_add(out, "typedef enum {\n");
   uint32_t num_constants = ast_EnumTypeDecl_getNumConstants(etd);
   ast_EnumConstantDecl** constants = ast_EnumTypeDecl_getConstants(etd);
   for (uint32_t i = 0; i < num_constants; i++) {
      ast_EnumConstantDecl* ecd = constants[i];
      string_buffer_Buf_indent(out, 1);
      string_buffer_Buf_add(out, gen->mod_name);
      string_buffer_Buf_add1(out, '_');
      string_buffer_Buf_add(out, ast_Decl_getName(d));
      string_buffer_Buf_add1(out, '_');
      string_buffer_Buf_add(out, ast_Decl_getName(ast_EnumConstantDecl_asDecl(ecd)));
      ast_Expr* ie = ast_EnumConstantDecl_getInit(ecd);
      ast_Decl_setGenerated(ast_EnumConstantDecl_asDecl(ecd));
      if (ie) {
         string_buffer_Buf_add(out, " = ");
         c_generator_Generator_emitExpr(gen, out, ie);
      }
      string_buffer_Buf_add(out, ",\n");
   }
   string_buffer_Buf_add(out, "} __attribute__((packed)) ");
   c_generator_Generator_emitCName(gen, out, d);
   string_buffer_Buf_add(out, ";\n\n");
}

static void c_generator_Generator_emitTypePre(c_generator_Generator* gen, string_buffer_Buf* out, ast_QualType qt)
{
   ast_Decl* decl = NULL;
   if (ast_QualType_isConst(&qt)) string_buffer_Buf_add(out, "const ");
   switch (ast_QualType_getKind(&qt)) {
   case ast_TypeKind_Builtin: {
      ast_BuiltinType* bt = ((ast_BuiltinType*)(ast_QualType_getType(&qt)));
      string_buffer_Buf_add(out, c_generator_builtinType_cnames[ast_BuiltinType_getKind(bt)]);
      return;
   }
   case ast_TypeKind_Pointer: {
      ast_PointerType* pt = ((ast_PointerType*)(ast_QualType_getType(&qt)));
      c_generator_Generator_emitTypePre(gen, out, ast_PointerType_getInner(pt));
      string_buffer_Buf_add1(out, '*');
      return;
   }
   case ast_TypeKind_Array: {
      ast_ArrayType* at = ((ast_ArrayType*)(ast_QualType_getType(&qt)));
      c_generator_Generator_emitTypePre(gen, out, ast_ArrayType_getElemType(at));
      return;
   }
   case ast_TypeKind_Struct: {
      ast_StructType* st = ((ast_StructType*)(ast_QualType_getType(&qt)));
      ast_StructTypeDecl* std = ast_StructType_getDecl(st);
      if (ast_StructTypeDecl_hasAttrNoTypeDef(std)) {
         string_buffer_Buf_add(out, ast_StructTypeDecl_isStruct(std) ? "struct " : "union ");
      }
      decl = ((ast_Decl*)(ast_StructType_getDecl(st)));
      break;
   }
   case ast_TypeKind_Enum: {
      ast_EnumType* et = ((ast_EnumType*)(ast_QualType_getType(&qt)));
      decl = ((ast_Decl*)(ast_EnumType_getDecl(et)));
      break;
   }
   case ast_TypeKind_Function: {
      ast_FunctionType* ft = ((ast_FunctionType*)(ast_QualType_getType(&qt)));
      decl = ((ast_Decl*)(ast_FunctionType_getDecl(ft)));
      break;
   }
   case ast_TypeKind_Alias: {
      ast_AliasType* at = ((ast_AliasType*)(ast_QualType_getType(&qt)));
      decl = ((ast_Decl*)(ast_AliasType_getDecl(at)));
      break;
   }
   case ast_TypeKind_Module:
      assert(0);
      return;
   }
   c_generator_Generator_emitCNameMod(gen, out, decl, ast_Decl_getModule(decl));
}

static void c_generator_Generator_emitTypePost(c_generator_Generator* gen, string_buffer_Buf* out, ast_QualType qt)
{
   if (ast_QualType_getKind(&qt) != ast_TypeKind_Array) return;

   ast_ArrayType* at = ((ast_ArrayType*)(ast_QualType_getType(&qt)));
   string_buffer_Buf_add1(out, '[');
   if (ast_ArrayType_hasSize(at)) string_buffer_Buf_print(out, "%u", ast_ArrayType_getSize(at));
   string_buffer_Buf_add1(out, ']');
   c_generator_Generator_emitTypePost(gen, out, ast_ArrayType_getElemType(at));
}

static void c_generator_Generator_genDeclIfNeeded(c_generator_Generator* gen, ast_Decl* d)
{
   if (ast_Decl_isGenerated(d)) return;

   if (ast_Decl_getKind(d) == ast_DeclKind_Var) {
      ast_VarDecl* vd = ((ast_VarDecl*)(d));
      if (!ast_VarDecl_isGlobal(vd)) return;

   }
   c_generator_Generator_emitGlobalDecl(gen, d);
}

static void c_generator_Generator_genTypeIfNeeded(c_generator_Generator* gen, ast_QualType qt, bool full)
{
   ast_Decl* d = NULL;
   switch (ast_QualType_getKind(&qt)) {
   case ast_TypeKind_Builtin:
      return;
   case ast_TypeKind_Pointer: {
      ast_PointerType* pt = ((ast_PointerType*)(ast_QualType_getType(&qt)));
      c_generator_Generator_genTypeIfNeeded(gen, ast_PointerType_getInner(pt), false);
      return;
   }
   case ast_TypeKind_Array: {
      ast_ArrayType* at = ((ast_ArrayType*)(ast_QualType_getType(&qt)));
      c_generator_Generator_genTypeIfNeeded(gen, ast_ArrayType_getElemType(at), true);
      return;
   }
   case ast_TypeKind_Struct: {
      ast_StructType* st = ((ast_StructType*)(ast_QualType_getType(&qt)));
      if (!full) return;

      d = ((ast_Decl*)(ast_StructType_getDecl(st)));
      break;
   }
   case ast_TypeKind_Enum: {
      ast_EnumType* et = ((ast_EnumType*)(ast_QualType_getType(&qt)));
      d = ((ast_Decl*)(ast_EnumType_getDecl(et)));
      break;
   }
   case ast_TypeKind_Function: {
      ast_FunctionType* et = ((ast_FunctionType*)(ast_QualType_getType(&qt)));
      d = ((ast_Decl*)(ast_FunctionType_getDecl(et)));
      break;
   }
   case ast_TypeKind_Alias: {
      ast_AliasType* at = ((ast_AliasType*)(ast_QualType_getType(&qt)));
      d = ((ast_Decl*)(ast_AliasType_getDecl(at)));
      break;
   }
   case ast_TypeKind_Module:
      assert(0);
      return;
   }
   if (!ast_Decl_isGenerated(d)) c_generator_Generator_emitGlobalDecl(gen, d);
}

static void c_generator_Generator_emitStructMember(c_generator_Generator* gen, string_buffer_Buf* out, ast_Decl* d, uint32_t indent)
{
   if (ast_Decl_getKind(d) == ast_DeclKind_Var) {
      c_generator_Generator_genTypeIfNeeded(gen, ast_Decl_getType(d), true);
      string_buffer_Buf_indent(out, indent);
      c_generator_Generator_emitTypePre(gen, out, ast_Decl_getType(d));
      string_buffer_Buf_add1(out, ' ');
      if (ast_Decl_getNameIdx(d)) string_buffer_Buf_add(out, ast_Decl_getName(d));
      c_generator_Generator_emitTypePost(gen, out, ast_Decl_getType(d));
      ast_VarDecl* vd = ((ast_VarDecl*)(d));
      ast_Expr* bitfield = ast_VarDecl_getBitfield(vd);
      if (bitfield) {
         string_buffer_Buf_add(out, " : ");
         c_generator_Generator_emitCtv(gen, out, bitfield);
      }
      string_buffer_Buf_add(out, ";\n");
   } else {
      assert(ast_Decl_getKind(d) == ast_DeclKind_StructType);
      c_generator_Generator_emitStruct(gen, out, d, indent);
   }
}

static void c_generator_Generator_emitStruct(c_generator_Generator* gen, string_buffer_Buf* out, ast_Decl* d, uint32_t indent)
{
   ast_StructTypeDecl* std = ((ast_StructTypeDecl*)(d));
   if (ast_StructTypeDecl_isGlobal(std)) {
      if (ast_StructTypeDecl_isStruct(std)) string_buffer_Buf_add(out, "struct ");
      else string_buffer_Buf_add(out, "union ");
      c_generator_Generator_emitCName(gen, out, d);
      if (!ast_StructTypeDecl_hasAttrNoTypeDef(std)) string_buffer_Buf_add1(out, '_');
      string_buffer_Buf_add(out, " {\n");
   } else {
      string_buffer_Buf_indent(out, indent);
      if (ast_StructTypeDecl_isStruct(std)) string_buffer_Buf_add(out, "struct");
      else string_buffer_Buf_add(out, "union");
      string_buffer_Buf_add(out, " {\n");
   }
   uint32_t num_members = ast_StructTypeDecl_getNumMembers(std);
   ast_Decl** members = ast_StructTypeDecl_getMembers(std);
   for (uint32_t i = 0; i < num_members; i++) {
      c_generator_Generator_emitStructMember(gen, out, members[i], indent + 1);
   }
   if (ast_StructTypeDecl_isGlobal(std)) {
      string_buffer_Buf_add(out, "};\n\n");
   } else {
      string_buffer_Buf_indent(out, indent);
      string_buffer_Buf_add1(out, '}');
      if (ast_Decl_getNameIdx(d)) {
         string_buffer_Buf_add1(out, ' ');
         string_buffer_Buf_add(out, ast_Decl_getName(d));
      }
      string_buffer_Buf_add(out, ";\n");
   }
}

static void c_generator_Generator_emitFunctionType(c_generator_Generator* gen, string_buffer_Buf* out, ast_Decl* d)
{
   ast_FunctionTypeDecl* ftd = ((ast_FunctionTypeDecl*)(d));
   ast_FunctionDecl* fd = ast_FunctionTypeDecl_getDecl(ftd);
   ast_Decl_setGenerated(ast_FunctionDecl_asDecl(fd));
   string_buffer_Buf_add(out, "typedef ");
   c_generator_Generator_emitTypePre(gen, out, ast_FunctionDecl_getRType(fd));
   string_buffer_Buf_add(out, " (*");
   c_generator_Generator_emitCName(gen, out, d);
   string_buffer_Buf_add(out, ")(");
   uint32_t num_params = ast_FunctionDecl_getNumParams(fd);
   ast_VarDecl** params = ast_FunctionDecl_getParams(fd);
   for (uint32_t i = 0; i < num_params; i++) {
      ast_Decl* arg = ((ast_Decl*)(params[i]));
      if (i != 0) string_buffer_Buf_add(out, ", ");
      c_generator_Generator_emitTypePre(gen, out, ast_Decl_getType(arg));
   }
   if (ast_FunctionDecl_isVariadic(fd)) {
      if (num_params) string_buffer_Buf_add(out, ", ");
      string_buffer_Buf_add(out, "...");
   }
   string_buffer_Buf_add(out, ");\n\n");
}

static void c_generator_Generator_emitAliasType(c_generator_Generator* gen, string_buffer_Buf* out, ast_Decl* d)
{
   ast_QualType qt = ast_Decl_getType(d);
   qt = ast_QualType_getCanonicalType(&qt);
   string_buffer_Buf_add(out, "typedef ");
   c_generator_Generator_emitTypePre(gen, out, qt);
   c_generator_Generator_emitTypePost(gen, out, qt);
   string_buffer_Buf_add1(out, ' ');
   c_generator_Generator_emitCName(gen, out, d);
   string_buffer_Buf_add(out, ";\n\n");
}

static void c_generator_Generator_emitGlobalVarDecl(c_generator_Generator* gen, string_buffer_Buf* out, ast_Decl* d)
{
   ast_VarDecl* vd = ((ast_VarDecl*)(d));
   ast_QualType qt = ast_Decl_getType(d);
   if ((gen->cur_external && !ast_QualType_isConstant(&qt))) {
      string_buffer_Buf_add(out, "extern ");
   } else {
      string_buffer_Buf_add(out, "static ");
   }
   c_generator_Generator_emitTypePre(gen, out, qt);
   string_buffer_Buf_add1(out, ' ');
   c_generator_Generator_emitCName(gen, out, d);
   c_generator_Generator_emitTypePost(gen, out, qt);
   ast_Expr* ie = ast_VarDecl_getInit(vd);
   if (ie) {
      string_buffer_Buf_add(out, " = ");
      c_generator_Generator_emitExpr(gen, out, ie);
   }
   string_buffer_Buf_add(out, ";\n\n");
}

static void c_generator_Generator_on_forward_structs(void* arg, ast_Decl* d)
{
   c_generator_Generator* gen = arg;
   string_buffer_Buf* out = gen->out;
   if (ast_Decl_getKind(d) != ast_DeclKind_StructType) return;

   if ((gen->cur_external && !ast_Decl_isUsed(d))) return;

   ast_StructTypeDecl* std = ((ast_StructTypeDecl*)(d));
   if (ast_StructTypeDecl_hasAttrNoTypeDef(std)) return;

   string_buffer_Buf_add(out, "typedef ");
   if (ast_StructTypeDecl_isStruct(std)) string_buffer_Buf_add(out, "struct");
   else string_buffer_Buf_add(out, "union");
   string_buffer_Buf_add1(out, ' ');
   c_generator_Generator_emitCName(gen, out, d);
   string_buffer_Buf_add(out, "_ ");
   c_generator_Generator_emitCName(gen, out, d);
   string_buffer_Buf_add(out, ";\n");
}

static void c_generator_Generator_emitGlobalDecl(c_generator_Generator* gen, ast_Decl* d)
{
   if (ast_Decl_isGenerated(d)) return;

   if ((gen->cur_external && !ast_Decl_isUsed(d))) return;

   switch (ast_Decl_getKind(d)) {
   case ast_DeclKind_Function: {
      ast_FunctionDecl* fd = ((ast_FunctionDecl*)(d));
      if (ast_FunctionDecl_isTemplate(fd)) break;

      c_generator_Generator_gen_func_proto(gen, fd, gen->out);
      string_buffer_Buf_add(gen->out, ";\n");
      break;
   }
   case ast_DeclKind_Import:
      assert(0);
      return;
   case ast_DeclKind_StructType: {
      c_generator_Fragment* f = c_generator_Generator_getFragment(gen);
      c_generator_Generator_emitStruct(gen, f->buf, d, 0);
      c_generator_Generator_addFragment(gen, f);
      break;
   }
   case ast_DeclKind_EnumType: {
      c_generator_Fragment* f = c_generator_Generator_getFragment(gen);
      c_generator_Generator_emitEnum(gen, f->buf, d);
      c_generator_Generator_addFragment(gen, f);
      break;
   }
   case ast_DeclKind_EnumConstant:
      printf("TODO gen enum %d\n", ast_Decl_isGenerated(d));
      break;
   case ast_DeclKind_FunctionType: {
      c_generator_Fragment* f = c_generator_Generator_getFragment(gen);
      c_generator_Generator_emitFunctionType(gen, f->buf, d);
      c_generator_Generator_addFragment(gen, f);
      break;
   }
   case ast_DeclKind_AliasType: {
      c_generator_Fragment* f = c_generator_Generator_getFragment(gen);
      c_generator_Generator_emitAliasType(gen, f->buf, d);
      c_generator_Generator_addFragment(gen, f);
      break;
   }
   case ast_DeclKind_Var: {
      c_generator_Fragment* f = c_generator_Generator_getFragment(gen);
      c_generator_Generator_emitGlobalVarDecl(gen, f->buf, d);
      c_generator_Generator_addFragment(gen, f);
      break;
   }
   case ast_DeclKind_StaticAssert:
      assert(0);
      break;
   }
   ast_Decl_setGenerated(d);
   c_generator_Generator_flattenFragments(gen);
}

static void c_generator_Generator_flattenFragments(c_generator_Generator* gen)
{
   while (!linked_list_Element_isEmpty(&gen->used_list)) {
      linked_list_Element* e = linked_list_Element_popFront(&gen->used_list);
      c_generator_Fragment* f = to_container(c_generator_Fragment, list, e);
      string_buffer_Buf_add2(gen->out, string_buffer_Buf_data(f->buf), string_buffer_Buf_size(f->buf));
      c_generator_Generator_freeFragment(gen, f);
   }
}

static void c_generator_Generator_on_decl(void* arg, ast_Decl* d)
{
   c_generator_Generator* gen = arg;
   c_generator_Generator_emitGlobalDecl(gen, d);
}

static void c_generator_Generator_on_vardecl(void* arg, ast_VarDecl* vd)
{
   c_generator_Generator* gen = arg;
   c_generator_Generator_emitGlobalDecl(gen, ((ast_Decl*)(vd)));
}

static void c_generator_Generator_on_ast_types(void* arg, ast_AST* a)
{
   ast_AST_visitTypeDecls(a, c_generator_Generator_on_decl, arg);
}

static void c_generator_Generator_gen_func_proto(c_generator_Generator* gen, ast_FunctionDecl* fd, string_buffer_Buf* out)
{
   ast_Decl* d = ((ast_Decl*)(fd));
   if (ast_FunctionDecl_isTemplate(fd)) return;

   if (d == gen->mainFunc) {
      string_buffer_Buf_add(out, "int32_t main");
   } else {
      if ((!gen->cur_external && !ast_Decl_isExported(d))) string_buffer_Buf_add(out, "static ");
      c_generator_Generator_emitTypePre(gen, out, ast_FunctionDecl_getRType(fd));
      string_buffer_Buf_add1(out, ' ');
      c_generator_Generator_emitCName(gen, out, d);
   }
   string_buffer_Buf_add1(out, '(');
   uint32_t num_params = ast_FunctionDecl_getNumParams(fd);
   ast_VarDecl** params = ast_FunctionDecl_getParams(fd);
   for (uint32_t i = 0; i < num_params; i++) {
      ast_Decl* argx = ((ast_Decl*)(params[i]));
      if (i != 0) string_buffer_Buf_add(out, ", ");
      c_generator_Generator_emitTypePre(gen, out, ast_Decl_getType(argx));
      string_buffer_Buf_add1(out, ' ');
      const char* name = ast_Decl_getName(argx);
      if (name) string_buffer_Buf_add(out, name);
      else string_buffer_Buf_print(out, "_arg%u", i);
   }
   if (ast_FunctionDecl_isVariadic(fd)) {
      if (num_params) string_buffer_Buf_add(out, ", ");
      string_buffer_Buf_add(out, "...");
   } else {
      if (num_params == 0) string_buffer_Buf_add(out, "void");
   }
   string_buffer_Buf_add1(out, ')');
}

static void c_generator_Generator_gen_func_forward_decl(void* arg, ast_FunctionDecl* fd)
{
   c_generator_Generator* gen = arg;
   c_generator_Generator_emitGlobalDecl(gen, ((ast_Decl*)(fd)));
}

static void c_generator_Generator_gen_func_protos(void* arg, ast_AST* a)
{
   ast_AST_visitFunctions(a, c_generator_Generator_gen_func_forward_decl, arg);
}

static void c_generator_Generator_emitFunction(c_generator_Generator* gen, ast_FunctionDecl* fd)
{
   c_generator_Fragment* f = c_generator_Generator_getFragment(gen);
   string_buffer_Buf* out = f->buf;
   c_generator_Generator_gen_func_proto(gen, fd, out);
   string_buffer_Buf_add1(out, '\n');
   string_buffer_Buf* saved = gen->out;
   gen->out = out;
   c_generator_Generator_emitStmt(gen, ((ast_Stmt*)(ast_FunctionDecl_getBody(fd))), 0, true);
   string_buffer_Buf_add1(out, '\n');
   gen->out = saved;
   c_generator_Generator_addFragment(gen, f);
}

static void c_generator_Generator_gen_full_func(void* arg, ast_FunctionDecl* fd)
{
   c_generator_Generator* gen = arg;
   if (ast_FunctionDecl_isTemplate(fd)) return;

   c_generator_Generator_emitFunction(gen, fd);
   c_generator_Generator_flattenFragments(gen);
}

static void c_generator_Generator_gen_full_funcs(void* arg, ast_AST* a)
{
   ast_AST_visitFunctions(a, c_generator_Generator_gen_full_func, arg);
}

static void c_generator_Generator_on_ast_vars(void* arg, ast_AST* a)
{
   ast_AST_visitVarDecls(a, c_generator_Generator_on_vardecl, arg);
}

static void c_generator_Generator_on_ast_structs(void* arg, ast_AST* a)
{
   c_generator_Generator* gen = arg;
   ast_AST_visitTypeDecls(a, c_generator_Generator_on_forward_structs, arg);
   string_buffer_Buf_add1(gen->out, '\n');
}

static void c_generator_Generator_decl_mark_generated(void* _arg0, ast_Decl* d)
{
   ast_Decl_setGenerated(d);
}

static void c_generator_Generator_ast_mark_generated(void* arg, ast_AST* a)
{
   ast_AST_visitDecls(a, c_generator_Generator_decl_mark_generated, arg);
}

static void c_generator_Generator_on_module(void* arg, ast_Module* m)
{
   if (!ast_Module_isUsed(m)) return;

   c_generator_Generator* gen = arg;
   string_buffer_Buf* out = gen->out;
   string_buffer_Buf_print(out, "\n// --- module %s ---\n\n", ast_Module_getName(m));
   gen->mod_name = ast_Module_getName(m);
   gen->mod = m;
   if (ast_Module_getNameIdx(m) == gen->stdargName) {
      string_buffer_Buf_add(out, "// Note: this module is a special case and is custom generated\n\n");
      string_buffer_Buf_add(out, "#define va_list __builtin_va_list\n");
      string_buffer_Buf_add(out, "#define va_start __builtin_va_start\n");
      string_buffer_Buf_add(out, "#define va_end __builtin_va_end\n");
      string_buffer_Buf_add1(out, '\n');
      string_buffer_Buf_add(out, "int32_t vdprintf(int32_t __fd, const char* __fmt, va_list __arg);\n");
      string_buffer_Buf_add(out, "int32_t vsprintf(char* str, const char* format, va_list __ap);\n");
      string_buffer_Buf_add(out, "int32_t vsnprintf(char* str, uint64_t size, const char* format, va_list __ap);\n");
      string_buffer_Buf_add1(out, '\n');
      ast_Module_visitASTs(m, c_generator_Generator_ast_mark_generated, arg);
      return;
   }
   ast_Module_visitASTs(m, c_generator_Generator_on_ast_structs, arg);
   ast_Module_visitASTs(m, c_generator_Generator_on_ast_types, arg);
   ast_Module_visitASTs(m, c_generator_Generator_gen_func_protos, arg);
   string_buffer_Buf_add1(out, '\n');
   ast_Module_visitASTs(m, c_generator_Generator_on_ast_vars, arg);
   if (!gen->cur_external) {
      ast_Module_visitASTs(m, c_generator_Generator_gen_full_funcs, arg);
   }
}

static void c_generator_Generator_init(c_generator_Generator* gen, const char* target, const char* output_dir, ast_Decl* mainFunc, uint32_t stdargName)
{
   memset(gen, 0, sizeof(c_generator_Generator));
   gen->out = string_buffer_create(256 * 1024, false, 3);
   gen->target = target;
   gen->output_dir = output_dir;
   gen->mainFunc = mainFunc;
   gen->stdargName = stdargName;
   linked_list_Element_init(&gen->free_list);
   linked_list_Element_init(&gen->used_list);
}

static void c_generator_Generator_free(c_generator_Generator* gen)
{
   while (!linked_list_Element_isEmpty(&gen->free_list)) {
      linked_list_Element* e = linked_list_Element_popFront(&gen->free_list);
      c_generator_Fragment* f = to_container(c_generator_Fragment, list, e);
      c_generator_Fragment_free(f);
   }
   string_buffer_Buf_free(gen->out);
}

static void c_generator_Generator_write(c_generator_Generator* gen, const char* output_dir, const char* filename)
{
   char fullname[256];
   sprintf(fullname, "%s/%s", output_dir, filename);
   file_utils_Writer writer;
   bool ok = file_utils_Writer_write(&writer, fullname, ((uint8_t*)(string_buffer_Buf_data(gen->out))), string_buffer_Buf_size(gen->out));
   if (!ok) {
      fprintf(stderr, "error writing %s: %s\n", fullname, strerror(*__errno_location()));
   }
}

static void c_generator_Generator_createMakefile(c_generator_Generator* gen, const char* output_dir)
{
   string_buffer_Buf* out = gen->out;
   string_buffer_Buf_clear(out);
   string_buffer_Buf_add(out, "# This makefile is auto-generated, any modifications will be lost\n\n");
   string_buffer_Buf_add(out, "CC=gcc\n");
   string_buffer_Buf_add(out, "CFLAGS=-Wall -Wextra -Wno-unused -Wno-switch -Wno-char-subscripts -Wno-zero-length-bounds -Wno-format-overflow -Wno-stringop-overflow\n");
   string_buffer_Buf_add(out, "CFLAGS+=-pipe -O2 -std=c99 -g -Wno-missing-field-initializers\n");
   string_buffer_Buf_add1(out, '\n');
   string_buffer_Buf_print(out, "../%s: main.c\n", gen->target);
   string_buffer_Buf_print(out, "\t\t$(CC) $(CFLAGS) main.c -o ../%s\n\n", gen->target);
   string_buffer_Buf_add(out, "symbols:\n");
   string_buffer_Buf_print(out, "\t\tnm -g -D -C --defined-only ../%s\n\n", gen->target);
   string_buffer_Buf_add(out, "clean:\n");
   string_buffer_Buf_print(out, "\t\trm -f ../%s\n\n", gen->target);
   c_generator_Generator_write(gen, output_dir, "Makefile");
}

static void c_generator_generate(const char* target, const char* output_dir, ast_Module* c2mod, component_Component** comps, uint32_t count, ast_Decl* mainFunc, uint32_t stdargName, bool print_code)
{
   char dir[256];
   sprintf(dir, "%s/%s", output_dir, c_generator_Dir);
   int32_t err = file_utils_create_directory(dir);
   if (err) {
      fprintf(stderr, "Error creating directory %s: %s\n", dir, strerror(err));
      return;
   }
   c_generator_Generator gen;
   c_generator_Generator_init(&gen, target, dir, mainFunc, stdargName);
   string_buffer_Buf* out = gen.out;
   string_buffer_Buf_add(out, "// --- internally added ---\n");
   string_buffer_Buf_add(out, "#include <assert.h>\n\n");
   string_buffer_Buf_add(out, "typedef char bool;\n");
   string_buffer_Buf_add(out, "typedef signed char int8_t;\n");
   string_buffer_Buf_add(out, "typedef unsigned char uint8_t;\n");
   string_buffer_Buf_add(out, "typedef signed short int16_t;\n");
   string_buffer_Buf_add(out, "typedef unsigned short uint16_t;\n");
   string_buffer_Buf_add(out, "typedef signed int int32_t;\n");
   string_buffer_Buf_add(out, "typedef unsigned int uint32_t;\n");
   string_buffer_Buf_add(out, "typedef signed long int64_t;\n");
   string_buffer_Buf_add(out, "typedef unsigned long uint64_t;\n");
   string_buffer_Buf_add(out, "typedef long ssize_t;\n");
   string_buffer_Buf_add(out, "typedef unsigned long size_t;\n");
   string_buffer_Buf_add(out, "#define true 1\n");
   string_buffer_Buf_add(out, "#define false 0\n");
   string_buffer_Buf_add(out, "#define NULL ((void*)0)\n");
   string_buffer_Buf_add(out, "#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))\n");
   string_buffer_Buf_add(out, "#define offsetof(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)\n");
   string_buffer_Buf_add(out, "#define to_container(type, member, ptr) ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))\n");
   gen.cur_external = false;
   c_generator_Generator_on_module(&gen, c2mod);
   for (uint32_t i = 0; i < count; i++) {
      component_Component* c = comps[i];
      gen.cur_external = component_Component_isExternal(c);
      component_Component_visitModules(c, c_generator_Generator_on_module, &gen);
   }
   if (print_code) puts(string_buffer_Buf_data(gen.out));
   c_generator_Generator_write(&gen, dir, c_generator_Filename);
   c_generator_Generator_createMakefile(&gen, dir);
   c_generator_Generator_free(&gen);
}

static void c_generator_build(const char* output_dir)
{
   char dir[256];
   sprintf(dir, "%s/%s/", output_dir, c_generator_Dir);
   int32_t retval = process_utils_run(dir, "/usr/bin/make", c_generator_LogFile);
   if (retval != 0) {
      fprintf(stderr, "Error during external C compilation\n");
      fprintf(stderr, "see %s%s for defails\n", dir, c_generator_LogFile);
   }
}

static void c_generator_Generator_emitExpr(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e)
{
   switch (ast_Expr_getKind(e)) {
   case ast_ExprKind_IntegerLiteral: {
      ast_IntegerLiteral* i = ((ast_IntegerLiteral*)(e));
      ast_IntegerLiteral_printLiteral(i, out);
      if ((ast_IntegerLiteral_isDecimal(i) && ast_IntegerLiteral_getValue(i) > c2_max_u32)) {
         if (ast_IntegerLiteral_isSigned(i)) string_buffer_Buf_add(out, "l");
         else string_buffer_Buf_add(out, "lu");
      }
      break;
   }
   case ast_ExprKind_BooleanLiteral: {
      ast_BooleanLiteral* b = ((ast_BooleanLiteral*)(e));
      if (ast_BooleanLiteral_getValue(b)) string_buffer_Buf_add(out, "true");
      else string_buffer_Buf_add(out, "false");
      break;
   }
   case ast_ExprKind_CharLiteral: {
      ast_CharLiteral* c = ((ast_CharLiteral*)(e));
      ast_CharLiteral_printLiteral(c, out);
      break;
   }
   case ast_ExprKind_StringLiteral: {
      ast_StringLiteral* s = ((ast_StringLiteral*)(e));
      ast_StringLiteral_printLiteral(s, out);
      break;
   }
   case ast_ExprKind_Nil:
      string_buffer_Buf_add(out, "NULL");
      break;
   case ast_ExprKind_Identifier: {
      ast_IdentifierExpr* i = ((ast_IdentifierExpr*)(e));
      ast_Decl* d = ast_IdentifierExpr_getDecl(i);
      c_generator_Generator_genDeclIfNeeded(gen, d);
      c_generator_Generator_emitDeclName(gen, out, d);
      break;
   }
   case ast_ExprKind_Type:
      c_generator_Generator_emitTypePre(gen, out, ast_Expr_getType(e));
      c_generator_Generator_emitTypePost(gen, out, ast_Expr_getType(e));
      break;
   case ast_ExprKind_Call:
      c_generator_Generator_emitCall(gen, out, e);
      break;
   case ast_ExprKind_InitList: {
      ast_InitListExpr* ile = ((ast_InitListExpr*)(e));
      string_buffer_Buf_add1(out, '{');
      uint32_t num_values = ast_InitListExpr_getNumValues(ile);
      ast_Expr** values = ast_InitListExpr_getValues(ile);
      bool newlines = false;
      if (num_values > 6) newlines = true;
      if ((num_values && ast_Expr_getKind(values[0]) == ast_ExprKind_InitList)) {
         newlines = true;
      }
      if (newlines) string_buffer_Buf_add1(out, '\n');
      else string_buffer_Buf_add1(out, ' ');
      for (uint32_t i = 0; i < num_values; i++) {
         if (newlines) string_buffer_Buf_indent(out, 1);
         c_generator_Generator_emitExpr(gen, out, values[i]);
         if (i + 1 != num_values) string_buffer_Buf_add1(out, ',');
         if (newlines) string_buffer_Buf_add1(out, '\n');
         else string_buffer_Buf_add1(out, ' ');
      }
      string_buffer_Buf_add1(out, '}');
      break;
   }
   case ast_ExprKind_FieldDesignatedInit:
      c_generator_Generator_emitFieldDesigExpr(gen, out, e);
      break;
   case ast_ExprKind_ArrayDesignatedInit:
      c_generator_Generator_emitArrayDesigExpr(gen, out, e);
      break;
   case ast_ExprKind_BinaryOperator:
      c_generator_Generator_emitBinaryOperator(gen, out, e);
      break;
   case ast_ExprKind_UnaryOperator:
      c_generator_Generator_emitUnaryOperator(gen, out, e);
      break;
   case ast_ExprKind_ConditionalOperator: {
      ast_ConditionalOperator* c = ((ast_ConditionalOperator*)(e));
      c_generator_Generator_emitExpr(gen, out, ast_ConditionalOperator_getCond(c));
      string_buffer_Buf_add(out, " ? ");
      c_generator_Generator_emitExpr(gen, out, ast_ConditionalOperator_getLHS(c));
      string_buffer_Buf_add(out, " : ");
      c_generator_Generator_emitExpr(gen, out, ast_ConditionalOperator_getRHS(c));
      break;
   }
   case ast_ExprKind_Builtin:
      c_generator_Generator_emitBuiltinExpr(gen, out, e);
      break;
   case ast_ExprKind_ArraySubscript: {
      ast_ArraySubscriptExpr* a = ((ast_ArraySubscriptExpr*)(e));
      c_generator_Generator_emitExpr(gen, out, ast_ArraySubscriptExpr_getBase(a));
      string_buffer_Buf_add1(out, '[');
      c_generator_Generator_emitExpr(gen, out, ast_ArraySubscriptExpr_getIndex(a));
      string_buffer_Buf_add1(out, ']');
      break;
   }
   case ast_ExprKind_Member:
      c_generator_Generator_emitMemberExpr(gen, out, e);
      break;
   case ast_ExprKind_Paren: {
      ast_ParenExpr* p = ((ast_ParenExpr*)(e));
      string_buffer_Buf_add1(out, '(');
      c_generator_Generator_emitExpr(gen, out, ast_ParenExpr_getInner(p));
      string_buffer_Buf_add1(out, ')');
      break;
   }
   case ast_ExprKind_BitOffset:
      break;
   case ast_ExprKind_ExplicitCast: {
      ast_ExplicitCastExpr* c = ((ast_ExplicitCastExpr*)(e));
      string_buffer_Buf_add(out, "((");
      c_generator_Generator_emitTypePre(gen, out, ast_Expr_getType(e));
      string_buffer_Buf_add(out, ")(");
      c_generator_Generator_emitExpr(gen, out, ast_ExplicitCastExpr_getInner(c));
      string_buffer_Buf_add(out, "))");
      break;
   }
   case ast_ExprKind_ImplicitCast: {
      ast_ImplicitCastExpr* i = ((ast_ImplicitCastExpr*)(e));
      c_generator_Generator_emitExpr(gen, out, ast_ImplicitCastExpr_getInner(i));
      break;
   }
   }
}

static void c_generator_Generator_emitBinaryOperator(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e)
{
   ast_BinaryOperator* b = ((ast_BinaryOperator*)(e));
   ast_BinaryOpcode opcode = ast_BinaryOperator_getOpcode(b);
   bool need_paren = ((opcode >= ast_BinaryOpcode_And && opcode <= ast_BinaryOpcode_LOr));
   if (need_paren) string_buffer_Buf_add1(out, '(');
   c_generator_Generator_emitExpr(gen, out, ast_BinaryOperator_getLHS(b));
   string_buffer_Buf_add1(out, ' ');
   string_buffer_Buf_add(out, ast_BinaryOperator_getOpcodeStr(b));
   string_buffer_Buf_add1(out, ' ');
   c_generator_Generator_emitExpr(gen, out, ast_BinaryOperator_getRHS(b));
   if (need_paren) string_buffer_Buf_add1(out, ')');
}

static void c_generator_Generator_emitUnaryOperator(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e)
{
   ast_UnaryOperator* u = ((ast_UnaryOperator*)(e));
   if (ast_UnaryOperator_isBefore(u)) {
      string_buffer_Buf_add(out, ast_UnaryOperator_getOpcodeStr(u));
      c_generator_Generator_emitExpr(gen, out, ast_UnaryOperator_getInner(u));
   } else {
      c_generator_Generator_emitExpr(gen, out, ast_UnaryOperator_getInner(u));
      string_buffer_Buf_add(out, ast_UnaryOperator_getOpcodeStr(u));
   }
}

static void c_generator_emitDotOrArrow(string_buffer_Buf* out, ast_QualType qt)
{
   if (ast_QualType_isPointerType(&qt)) string_buffer_Buf_add(out, "->");
   else string_buffer_Buf_add1(out, '.');
}

static void c_generator_Generator_emitMemberExpr(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e)
{
   ast_MemberExpr* m = ((ast_MemberExpr*)(e));
   bool need_dot = false;
   ast_QualType baseType = { };
   if (ast_MemberExpr_hasExpr(m)) {
      ast_Expr* base = ast_MemberExpr_getExprBase(m);
      c_generator_Generator_emitExpr(gen, out, base);
      baseType = ast_Expr_getType(base);
      need_dot = true;
   }
   uint32_t numrefs = ast_MemberExpr_getNumRefs(m);
   bool is_local = false;
   for (uint32_t i = 0; i < numrefs; i++) {
      ast_Decl* d = ast_MemberExpr_getDecl(m, i);
      switch (ast_Decl_getKind(d)) {
      case ast_DeclKind_Function:
         if (need_dot) c_generator_emitDotOrArrow(out, baseType);
         baseType = ast_Decl_getType(d);
         c_generator_Generator_emitCNameMod(gen, out, d, ast_Decl_getModule(d));
         break;
      case ast_DeclKind_Import:
         break;
      case ast_DeclKind_StructType: {
         if (need_dot) c_generator_emitDotOrArrow(out, baseType);
         ast_StructTypeDecl* std = ((ast_StructTypeDecl*)(d));
         if (!ast_StructTypeDecl_isGlobal(std)) {
            baseType = ast_Decl_getType(d);
            string_buffer_Buf_add(out, ast_Decl_getName(d));
            need_dot = true;
         }
         break;
      }
      case ast_DeclKind_EnumType:
         c_generator_Generator_genDeclIfNeeded(gen, d);
         break;
      case ast_DeclKind_EnumConstant:
         c_generator_Generator_emitCNameMod(gen, out, d, ast_Decl_getModule(d));
         break;
      case ast_DeclKind_FunctionType:
         assert(0);
         break;
      case ast_DeclKind_AliasType:
         c_generator_Generator_emitCNameMod(gen, out, d, ast_Decl_getModule(d));
         break;
      case ast_DeclKind_Var:
         if (need_dot) c_generator_emitDotOrArrow(out, baseType);
         baseType = ast_Decl_getType(d);
         if (is_local) {
            string_buffer_Buf_add(out, ast_Decl_getName(d));
         } else {
            c_generator_Generator_emitDeclName(gen, out, d);
         }
         need_dot = true;
         is_local = true;
         break;
      case ast_DeclKind_StaticAssert:
         break;
      }
   }
}

static void c_generator_Generator_emitMemberExprBase(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e)
{
   ast_MemberExpr* m = ((ast_MemberExpr*)(e));
   bool need_dot = false;
   ast_QualType baseType = { };
   if (ast_MemberExpr_hasExpr(m)) {
      ast_Expr* base = ast_MemberExpr_getExprBase(m);
      c_generator_Generator_emitExpr(gen, out, base);
      baseType = ast_Expr_getType(base);
      need_dot = true;
   }
   uint32_t numrefs = ast_MemberExpr_getNumRefs(m);
   numrefs -= 1;
   bool is_local = false;
   for (uint32_t i = 0; i < numrefs; i++) {
      ast_Decl* d = ast_MemberExpr_getDecl(m, i);
      switch (ast_Decl_getKind(d)) {
      case ast_DeclKind_Function:
         if (need_dot) c_generator_emitDotOrArrow(out, baseType);
         baseType = ast_Decl_getType(d);
         c_generator_Generator_emitCNameMod(gen, out, d, ast_Decl_getModule(d));
         break;
      case ast_DeclKind_Import:
         break;
      case ast_DeclKind_StructType:
         if (need_dot) c_generator_emitDotOrArrow(out, baseType);
         baseType = ast_Decl_getType(d);
         string_buffer_Buf_add(out, ast_Decl_getName(d));
         need_dot = true;
         break;
      case ast_DeclKind_EnumType:
         c_generator_Generator_genDeclIfNeeded(gen, d);
         break;
      case ast_DeclKind_EnumConstant:
         c_generator_Generator_emitCNameMod(gen, out, d, ast_Decl_getModule(d));
         break;
      case ast_DeclKind_FunctionType:
         assert(0);
         break;
      case ast_DeclKind_AliasType:
         c_generator_Generator_emitCNameMod(gen, out, d, ast_Decl_getModule(d));
         break;
      case ast_DeclKind_Var:
         if (need_dot) c_generator_emitDotOrArrow(out, baseType);
         baseType = ast_Decl_getType(d);
         if (is_local) {
            string_buffer_Buf_add(out, ast_Decl_getName(d));
         } else {
            c_generator_Generator_emitDeclName(gen, out, d);
         }
         need_dot = true;
         is_local = true;
         break;
      case ast_DeclKind_StaticAssert:
         break;
      }
   }
}

static void c_generator_Generator_emitFieldDesigExpr(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e)
{
   ast_FieldDesignatedInitExpr* fdi = ((ast_FieldDesignatedInitExpr*)(e));
   string_buffer_Buf_add1(out, '.');
   string_buffer_Buf_add(out, ast_FieldDesignatedInitExpr_getFieldName(fdi));
   string_buffer_Buf_add(out, " = ");
   c_generator_Generator_emitExpr(gen, out, ast_FieldDesignatedInitExpr_getInit(fdi));
}

static void c_generator_Generator_emitArrayDesigExpr(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e)
{
   ast_ArrayDesignatedInitExpr* ad = ((ast_ArrayDesignatedInitExpr*)(e));
   string_buffer_Buf_add1(out, '[');
   c_generator_Generator_emitExpr(gen, out, ast_ArrayDesignatedInitExpr_getDesignator(ad));
   string_buffer_Buf_add(out, "] = ");
   c_generator_Generator_emitExpr(gen, out, ast_ArrayDesignatedInitExpr_getInit(ad));
}

static void c_generator_Generator_emitCall(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e)
{
   ast_CallExpr* call = ((ast_CallExpr*)(e));
   bool is_sf = ast_CallExpr_isStructFunc(call);
   if (ast_CallExpr_isTemplateCall(call)) {
      ast_Expr* fn = ast_CallExpr_getFunc(call);
      ast_QualType qt = ast_Expr_getType(fn);
      ast_FunctionType* ft = ast_QualType_getFunctionTypeOrNil(&qt);
      ast_FunctionDecl* template_fd = ast_FunctionType_getDecl(ft);
      uint32_t idx = ast_CallExpr_getTemplateIdx(call);
      ast_FunctionDecl* instance = ast_Module_getInstance(gen->mod, template_fd, idx);
      assert(instance);
      ast_Decl* d = ast_FunctionDecl_asDecl(instance);
      if (!ast_Decl_isGenerated(d)) {
         c_generator_Generator_emitFunction(gen, instance);
         ast_Decl_setGenerated(ast_FunctionDecl_asDecl(instance));
      }
      c_generator_Generator_emitDeclName(gen, out, d);
      string_buffer_Buf_add1(out, '(');
   } else {
      ast_Expr* fn = ast_CallExpr_getFunc(call);
      if ((is_sf || ast_CallExpr_isStaticStructFunc(call))) {
         assert(ast_Expr_getKind(fn) == ast_ExprKind_ImplicitCast);
         ast_ImplicitCastExpr* ic = ((ast_ImplicitCastExpr*)(fn));
         fn = ast_ImplicitCastExpr_getInner(ic);
         assert(ast_Expr_getKind(fn) == ast_ExprKind_Member);
         ast_MemberExpr* m = ((ast_MemberExpr*)(fn));
         ast_Decl* d = ast_MemberExpr_getFullDecl(m);
         c_generator_Generator_emitCNameMod(gen, out, d, ast_Decl_getModule(d));
         string_buffer_Buf_add1(out, '(');
         if (is_sf) {
            ast_QualType baseType = ast_MemberExpr_getBaseType(m);
            if (!ast_QualType_isPointerType(&baseType)) string_buffer_Buf_add1(out, '&');
            c_generator_Generator_emitMemberExprBase(gen, out, fn);
         }
      } else {
         c_generator_Generator_emitExpr(gen, out, fn);
         string_buffer_Buf_add1(out, '(');
      }
   }
   uint32_t num_args = ast_CallExpr_getNumArgs(call);
   ast_Expr** args = ast_CallExpr_getArgs(call);
   for (uint32_t i = 0; i < num_args; i++) {
      if ((i != 0 || is_sf)) string_buffer_Buf_add(out, ", ");
      c_generator_Generator_emitExpr(gen, out, args[i]);
   }
   string_buffer_Buf_add1(out, ')');
}

static void c_generator_Generator_emitBuiltinExpr(c_generator_Generator* gen, string_buffer_Buf* out, ast_Expr* e)
{
   ast_BuiltinExpr* b = ((ast_BuiltinExpr*)(e));
   switch (ast_BuiltinExpr_getKind(b)) {
   case ast_BuiltinExprKind_Sizeof:
      string_buffer_Buf_add(out, "sizeof(");
      c_generator_Generator_emitExpr(gen, out, ast_BuiltinExpr_getInner(b));
      string_buffer_Buf_add1(out, ')');
      break;
   case ast_BuiltinExprKind_Elemsof:
      string_buffer_Buf_add(out, "ARRAY_SIZE(");
      c_generator_Generator_emitExpr(gen, out, ast_BuiltinExpr_getInner(b));
      string_buffer_Buf_add1(out, ')');
      break;
   case ast_BuiltinExprKind_EnumMin:
      string_buffer_Buf_print(out, "%u", ast_BuiltinExpr_getValue(b));
      break;
   case ast_BuiltinExprKind_EnumMax:
      string_buffer_Buf_print(out, "%u", ast_BuiltinExpr_getValue(b));
      break;
   case ast_BuiltinExprKind_OffsetOf:
      string_buffer_Buf_add(out, "offsetof(");
      c_generator_Generator_emitExpr(gen, out, ast_BuiltinExpr_getInner(b));
      string_buffer_Buf_add(out, ", ");
      c_generator_Generator_emitExpr(gen, out, ast_BuiltinExpr_getOffsetOfMember(b));
      string_buffer_Buf_add1(out, ')');
      break;
   case ast_BuiltinExprKind_ToContainer:
      string_buffer_Buf_add(out, "to_container(");
      c_generator_Generator_emitExpr(gen, out, ast_BuiltinExpr_getInner(b));
      string_buffer_Buf_add(out, ", ");
      c_generator_Generator_emitExpr(gen, out, ast_BuiltinExpr_getToContainerMember(b));
      string_buffer_Buf_add(out, ", ");
      c_generator_Generator_emitExpr(gen, out, ast_BuiltinExpr_getToContainerPointer(b));
      string_buffer_Buf_add1(out, ')');
      break;
   }
}

static void c_generator_Generator_emitStmt(c_generator_Generator* gen, ast_Stmt* s, uint32_t indent, bool newline)
{
   string_buffer_Buf* out = gen->out;
   if (newline) string_buffer_Buf_indent(out, indent);
   switch (ast_Stmt_getKind(s)) {
   case ast_StmtKind_Return: {
      string_buffer_Buf_add(out, "return");
      ast_ReturnStmt* r = ((ast_ReturnStmt*)(s));
      ast_Expr* val = ast_ReturnStmt_getValue(r);
      if (val) {
         string_buffer_Buf_add1(out, ' ');
         c_generator_Generator_emitExpr(gen, out, val);
      }
      string_buffer_Buf_add(out, ";\n");
      break;
   }
   case ast_StmtKind_Expr:
      c_generator_Generator_emitExpr(gen, out, ((ast_Expr*)(s)));
      if (newline) string_buffer_Buf_add(out, ";\n");
      break;
   case ast_StmtKind_If: {
      ast_IfStmt* i = ((ast_IfStmt*)(s));
      string_buffer_Buf_add(out, "if (");
      c_generator_Generator_emitStmt(gen, ast_IfStmt_getCond(i), 0, false);
      string_buffer_Buf_add(out, ") ");
      ast_Stmt* thenStmt = ast_IfStmt_getThen(i);
      c_generator_Generator_emitStmt(gen, thenStmt, indent, false);
      if (ast_Stmt_getKind(thenStmt) != ast_StmtKind_Compound) {
         if (!string_buffer_Buf_endsWith(out, '\n')) string_buffer_Buf_add1(out, ';');
      }
      ast_Stmt* elseStmt = ast_IfStmt_getElse(i);
      if (elseStmt) {
         if (ast_Stmt_getKind(thenStmt) == ast_StmtKind_Compound) {
            string_buffer_Buf_add1(out, ' ');
         } else {
            if (!string_buffer_Buf_endsWith(out, '\n')) string_buffer_Buf_add1(out, '\n');
            string_buffer_Buf_indent(out, indent);
         }
         string_buffer_Buf_add(out, "else ");
         c_generator_Generator_emitStmt(gen, elseStmt, indent, false);
         if (ast_Stmt_getKind(elseStmt) != ast_StmtKind_Compound) {
            if (!string_buffer_Buf_endsWith(out, '\n')) string_buffer_Buf_add1(out, ';');
         }
      }
      string_buffer_Buf_add1(out, '\n');
      break;
   }
   case ast_StmtKind_While: {
      ast_WhileStmt* w = ((ast_WhileStmt*)(s));
      string_buffer_Buf_add(out, "while (");
      c_generator_Generator_emitStmt(gen, ast_WhileStmt_getCond(w), 0, false);
      string_buffer_Buf_add(out, ") ");
      ast_Stmt* body = ast_WhileStmt_getBody(w);
      c_generator_Generator_emitStmt(gen, body, indent, false);
      if (ast_Stmt_getKind(body) != ast_StmtKind_Compound) {
         string_buffer_Buf_add1(out, ';');
      }
      string_buffer_Buf_add1(out, '\n');
      break;
   }
   case ast_StmtKind_Do: {
      ast_DoStmt* doStmt = ((ast_DoStmt*)(s));
      string_buffer_Buf_add(out, "do ");
      ast_Stmt* body = ast_DoStmt_getBody(doStmt);
      c_generator_Generator_emitStmt(gen, body, indent, false);
      if (ast_Stmt_getKind(body) == ast_StmtKind_Compound) {
         string_buffer_Buf_add1(out, ' ');
      } else {
         string_buffer_Buf_add(out, ";\n");
         string_buffer_Buf_indent(out, indent);
      }
      string_buffer_Buf_add(out, "while (");
      c_generator_Generator_emitStmt(gen, ast_DoStmt_getCond(doStmt), 0, false);
      string_buffer_Buf_add(out, ");\n");
      break;
   }
   case ast_StmtKind_For: {
      ast_ForStmt* f = ((ast_ForStmt*)(s));
      string_buffer_Buf_add(out, "for (");
      ast_Stmt* initStmt = ast_ForStmt_getInit(f);
      if (initStmt) {
         c_generator_Generator_emitStmt(gen, initStmt, 0, false);
      }
      string_buffer_Buf_add(out, "; ");
      c_generator_Generator_emitExpr(gen, out, ast_ForStmt_getCond(f));
      string_buffer_Buf_add(out, "; ");
      ast_Expr* incr = ast_ForStmt_getIncr(f);
      if (incr) {
         c_generator_Generator_emitExpr(gen, out, incr);
      }
      string_buffer_Buf_add(out, ") ");
      ast_Stmt* body = ast_ForStmt_getBody(f);
      c_generator_Generator_emitStmt(gen, body, indent, false);
      if (ast_Stmt_getKind(body) == ast_StmtKind_Compound) {
      } else {
         string_buffer_Buf_add1(out, ';');
      }
      string_buffer_Buf_add1(out, '\n');
      break;
   }
   case ast_StmtKind_Switch:
      c_generator_Generator_emitSwitchStmt(gen, s, indent);
      break;
   case ast_StmtKind_Case: {
      ast_CaseStmt* c = ((ast_CaseStmt*)(s));
      string_buffer_Buf_add(out, "case ");
      c_generator_Generator_emitExpr(gen, out, ast_CaseStmt_getCond(c));
      string_buffer_Buf_add1(out, ':');
      if (ast_CaseStmt_hasDecls(c)) string_buffer_Buf_add(out, " {");
      string_buffer_Buf_add1(out, '\n');
      uint32_t num_stmts = ast_CaseStmt_getNumStmts(c);
      ast_Stmt** stmts = ast_CaseStmt_getStmts(c);
      for (uint32_t i = 0; i < num_stmts; i++) {
         c_generator_Generator_emitStmt(gen, stmts[i], indent + 1, true);
      }
      if (ast_CaseStmt_hasDecls(c)) {
         string_buffer_Buf_indent(out, indent);
         string_buffer_Buf_add(out, "}\n");
      }
      break;
   }
   case ast_StmtKind_Default: {
      ast_DefaultStmt* d = ((ast_DefaultStmt*)(s));
      string_buffer_Buf_add(out, "default:");
      if (ast_DefaultStmt_hasDecls(d)) string_buffer_Buf_add(out, " {");
      string_buffer_Buf_add1(out, '\n');
      uint32_t num_stmts = ast_DefaultStmt_getNumStmts(d);
      ast_Stmt** stmts = ast_DefaultStmt_getStmts(d);
      for (uint32_t i = 0; i < num_stmts; i++) {
         c_generator_Generator_emitStmt(gen, stmts[i], indent + 1, true);
      }
      if (ast_DefaultStmt_hasDecls(d)) {
         string_buffer_Buf_indent(out, indent);
         string_buffer_Buf_add(out, "}\n");
      }
      break;
   }
   case ast_StmtKind_Break:
      string_buffer_Buf_add(out, "break;\n");
      break;
   case ast_StmtKind_Continue:
      string_buffer_Buf_add(out, "continue;\n");
      break;
   case ast_StmtKind_Fallthrough:
      string_buffer_Buf_add(out, "// fallthrough\n");
      break;
   case ast_StmtKind_Label: {
      ast_LabelStmt* l = ((ast_LabelStmt*)(s));
      string_buffer_Buf_add(out, ast_LabelStmt_getName(l));
      string_buffer_Buf_add(out, ":\n");
      break;
   }
   case ast_StmtKind_Goto: {
      ast_GotoStmt* g = ((ast_GotoStmt*)(s));
      string_buffer_Buf_add(out, "goto ");
      string_buffer_Buf_add(out, ast_GotoStmt_getName(g));
      string_buffer_Buf_add(out, ";\n");
      break;
   }
   case ast_StmtKind_Compound: {
      ast_CompoundStmt* c = ((ast_CompoundStmt*)(s));
      string_buffer_Buf_add(out, "{\n");
      uint32_t count = ast_CompoundStmt_getCount(c);
      ast_Stmt** stmts = ast_CompoundStmt_getStmts(c);
      for (uint32_t i = 0; i < count; i++) {
         c_generator_Generator_emitStmt(gen, stmts[i], indent + 1, true);
      }
      string_buffer_Buf_indent(out, indent);
      string_buffer_Buf_add1(out, '}');
      if (newline) string_buffer_Buf_add1(out, '\n');
      break;
   }
   case ast_StmtKind_Decl: {
      ast_DeclStmt* ds = ((ast_DeclStmt*)(s));
      ast_VarDecl* vd = ast_DeclStmt_getDecl(ds);
      ast_Decl* d = ((ast_Decl*)(vd));
      if (ast_VarDecl_hasLocalQualifier(vd)) string_buffer_Buf_add(out, "static ");
      c_generator_Generator_emitTypePre(gen, out, ast_Decl_getType(d));
      string_buffer_Buf_add1(out, ' ');
      string_buffer_Buf_add(out, ast_Decl_getName(d));
      c_generator_Generator_emitTypePost(gen, out, ast_Decl_getType(d));
      ast_Decl_setGenerated(d);
      ast_Expr* ie = ast_VarDecl_getInit(vd);
      if (ie) {
         string_buffer_Buf_add(out, " = ");
         c_generator_Generator_emitExpr(gen, out, ie);
      }
      if (newline) string_buffer_Buf_add(out, ";\n");
      break;
   }
   case ast_StmtKind_Assert: {
      ast_AssertStmt* a = ((ast_AssertStmt*)(s));
      string_buffer_Buf_add(out, "assert(");
      c_generator_Generator_emitExpr(gen, out, ast_AssertStmt_getInner(a));
      string_buffer_Buf_add(out, ");\n");
      break;
   }
   }
}

static void c_generator_Generator_emitSwitchStmt(c_generator_Generator* gen, ast_Stmt* s, uint32_t indent)
{
   string_buffer_Buf* out = gen->out;
   ast_SwitchStmt* sw = ((ast_SwitchStmt*)(s));
   uint32_t num_cases = ast_SwitchStmt_getNumCases(sw);
   ast_Stmt** cases = ast_SwitchStmt_getCases(sw);
   if (ast_SwitchStmt_isSSwitch(sw)) {
      string_buffer_Buf_add(out, "do {\n");
      string_buffer_Buf_indent(out, indent + 1);
      string_buffer_Buf_add(out, "const char* _tmp = ");
      c_generator_Generator_emitStmt(gen, ast_SwitchStmt_getCond(sw), 0, false);
      string_buffer_Buf_add(out, ";\n");
      for (uint32_t i = 0; i < num_cases; i++) {
         uint32_t num_stmts;
         ast_Stmt** stmts;
         if (i == 0) string_buffer_Buf_indent(out, indent + 1);
         else string_buffer_Buf_add(out, " else ");
         if (ast_Stmt_getKind(cases[i]) == ast_StmtKind_Case) {
            ast_CaseStmt* c = ((ast_CaseStmt*)(cases[i]));
            num_stmts = ast_CaseStmt_getNumStmts(c);
            stmts = ast_CaseStmt_getStmts(c);
            ast_Expr* cond = ast_CaseStmt_getCond(c);
            if (ast_Expr_getKind(cond) == ast_ExprKind_Nil) {
               string_buffer_Buf_add(out, "if (_tmp == NULL) {\n");
            } else {
               string_buffer_Buf_add(out, "if (strcmp(_tmp, ");
               c_generator_Generator_emitExpr(gen, out, cond);
               string_buffer_Buf_add(out, ") == 0) {\n");
            }
         } else {
            ast_DefaultStmt* c = ((ast_DefaultStmt*)(cases[i]));
            num_stmts = ast_DefaultStmt_getNumStmts(c);
            stmts = ast_DefaultStmt_getStmts(c);
            string_buffer_Buf_add(out, "{\n");
         }
         for (uint32_t j = 0; j < num_stmts; j++) {
            c_generator_Generator_emitStmt(gen, stmts[j], indent + 2, true);
         }
         string_buffer_Buf_indent(out, indent + 1);
         string_buffer_Buf_add1(out, '}');
      }
      string_buffer_Buf_add1(out, '\n');
      string_buffer_Buf_indent(out, indent);
      string_buffer_Buf_add(out, "} while (0);\n");
   } else {
      string_buffer_Buf_add(out, "switch (");
      c_generator_Generator_emitStmt(gen, ast_SwitchStmt_getCond(sw), 0, false);
      string_buffer_Buf_add(out, ") {\n");
      for (uint32_t i = 0; i < num_cases; i++) {
         c_generator_Generator_emitStmt(gen, cases[i], indent, true);
      }
      string_buffer_Buf_indent(out, indent);
      string_buffer_Buf_add(out, "}\n");
   }
}


// --- module qbe_generator ---

typedef struct qbe_generator_Fragment_ qbe_generator_Fragment;
typedef struct qbe_generator_Generator_ qbe_generator_Generator;

typedef struct qbe_generator_Var_ qbe_generator_Var;
typedef struct qbe_generator_Locals_ qbe_generator_Locals;

struct qbe_generator_Fragment_ {
   string_buffer_Buf* buf;
   linked_list_Element list;
};

struct qbe_generator_Locals_ {
   qbe_generator_Var* vars;
   uint32_t count;
   uint32_t capacity;
   uint32_t index;
};

struct qbe_generator_Generator_ {
   string_buffer_Buf* out;
   const char* target;
   const char* output_dir;
   bool cur_external;
   uint32_t func_idx;
   uint32_t string_idx;
   uint32_t substruct_idx;
   qbe_generator_Locals locals;
   qbe_generator_Fragment* start;
   qbe_generator_Fragment* data;
   linked_list_Element free_list;
   linked_list_Element used_list;
};

struct qbe_generator_Var_ {
   const ast_VarDecl* vd;
   uint32_t idx;
   uint32_t size;
   uint32_t align;
};

static qbe_generator_Fragment* qbe_generator_Fragment_create(void);
static void qbe_generator_Fragment_clear(qbe_generator_Fragment* f);
static void qbe_generator_Fragment_free(qbe_generator_Fragment* f);
static qbe_generator_Fragment* qbe_generator_Generator_getFragment(qbe_generator_Generator* gen);
static void qbe_generator_Generator_addFragment(qbe_generator_Generator* gen, qbe_generator_Fragment* f);
static void qbe_generator_Generator_freeFragment(qbe_generator_Generator* gen, qbe_generator_Fragment* f);
static void qbe_generator_addStructName(string_buffer_Buf* out, ast_Decl* d);
static void qbe_generator_addGlobalName(string_buffer_Buf* out, ast_Decl* d);
static void qbe_generator_addType(string_buffer_Buf* out, ast_QualType qt);
static uint32_t qbe_generator_Generator_addLocal(qbe_generator_Generator* gen, ast_VarDecl* vd);
static void qbe_generator_Generator_addParam(qbe_generator_Generator* gen, string_buffer_Buf* out, ast_VarDecl* vd);
static char qbe_generator_align2char(uint32_t align);
static char qbe_generator_align2store(uint32_t align);
static void qbe_generator_Generator_doFunctionBody(qbe_generator_Generator* gen, const ast_FunctionDecl* fd, string_buffer_Buf* out);
static void qbe_generator_Generator_handleFunction(qbe_generator_Generator* gen, ast_Decl* d);
static void qbe_generator_Generator_doArrayInit(qbe_generator_Generator* gen, string_buffer_Buf* out, const ast_ArrayType* at, const ast_Expr* e);
static void qbe_generator_Generator_doStructInit(qbe_generator_Generator* gen, string_buffer_Buf* out, const ast_StructType* st, const ast_Expr* e);
static uint32_t qbe_generator_Generator_createString(qbe_generator_Generator* gen, const ast_Expr* e);
static void qbe_generator_Generator_doExpr(qbe_generator_Generator* gen, string_buffer_Buf* out, const ast_Expr* e);
static void qbe_generator_Generator_handleVarDecl(qbe_generator_Generator* gen, ast_Decl* d);
static void qbe_generator_addMember(string_buffer_Buf* out, ast_QualType qt);
static uint32_t qbe_generator_Generator_createStruct(qbe_generator_Generator* gen, string_buffer_Buf* out, ast_StructTypeDecl* s, bool is_global);
static void qbe_generator_Generator_handleStruct(qbe_generator_Generator* gen, ast_Decl* d);
static void qbe_generator_Generator_on_decl(void* arg, ast_Decl* d);
static void qbe_generator_Generator_on_ast(void* arg, ast_AST* a);
static void qbe_generator_Generator_on_module(void* arg, ast_Module* m);
static void qbe_generator_Generator_init(qbe_generator_Generator* gen, const char* target, const char* output_dir);
static void qbe_generator_Generator_free(qbe_generator_Generator* gen);
static void qbe_generator_Generator_write(qbe_generator_Generator* gen, const char* output_dir, const char* filename);
static void qbe_generator_Generator_createMakefile(qbe_generator_Generator* gen, const char* output_dir);
static void qbe_generator_generate(const char* target, const char* output_dir, component_Component** comps, uint32_t count, bool print);
static void qbe_generator_build(const char* output_dir);
static void qbe_generator_Locals_init(qbe_generator_Locals* l);
static void qbe_generator_Locals_free(qbe_generator_Locals* l);
static void qbe_generator_Locals_clear(qbe_generator_Locals* l);
static void qbe_generator_Locals_resize(qbe_generator_Locals* l, uint32_t capacity);
static uint32_t qbe_generator_Locals_add(qbe_generator_Locals* l, ast_VarDecl* vd, uint32_t size, uint32_t align);
static uint32_t qbe_generator_Locals_next(qbe_generator_Locals* l, const ast_VarDecl* vd);
static qbe_generator_Var* qbe_generator_Locals_find(const qbe_generator_Locals* l, const ast_VarDecl* vd);
static uint32_t qbe_generator_Locals_getIdx(const qbe_generator_Locals* l, const ast_VarDecl* vd);

static const char qbe_generator_QBE_Dir[] = "qbe";

static const char qbe_generator_QBE_Filename[] = "main.qbe";

static const char qbe_generator_LogFile[] = "build.log";

static qbe_generator_Fragment* qbe_generator_Fragment_create(void)
{
   qbe_generator_Fragment* f = malloc(sizeof(qbe_generator_Fragment));
   f->buf = string_buffer_create(512, false, 1);
   return f;
}

static void qbe_generator_Fragment_clear(qbe_generator_Fragment* f)
{
   string_buffer_Buf_clear(f->buf);
}

static void qbe_generator_Fragment_free(qbe_generator_Fragment* f)
{
   string_buffer_Buf_free(f->buf);
   free(f);
}

static qbe_generator_Fragment* qbe_generator_Generator_getFragment(qbe_generator_Generator* gen)
{
   if (linked_list_Element_isEmpty(&gen->free_list)) {
      return qbe_generator_Fragment_create();
   }
   linked_list_Element* e = linked_list_Element_popFront(&gen->free_list);
   qbe_generator_Fragment* f = to_container(qbe_generator_Fragment, list, e);
   qbe_generator_Fragment_clear(f);
   return f;
}

static void qbe_generator_Generator_addFragment(qbe_generator_Generator* gen, qbe_generator_Fragment* f)
{
   linked_list_Element_addTail(&gen->used_list, &f->list);
}

static void qbe_generator_Generator_freeFragment(qbe_generator_Generator* gen, qbe_generator_Fragment* f)
{
   linked_list_Element_addTail(&gen->free_list, &f->list);
}

static void qbe_generator_addStructName(string_buffer_Buf* out, ast_Decl* d)
{
   assert(ast_Decl_getName(d));
   string_buffer_Buf_add1(out, ':');
   string_buffer_Buf_add(out, ast_Decl_getModuleName(d));
   string_buffer_Buf_add1(out, '_');
   string_buffer_Buf_add(out, ast_Decl_getName(d));
}

static void qbe_generator_addGlobalName(string_buffer_Buf* out, ast_Decl* d)
{
   assert(ast_Decl_getName(d));
   string_buffer_Buf_add1(out, '$');
   string_buffer_Buf_add(out, ast_Decl_getModuleName(d));
   string_buffer_Buf_add1(out, '_');
   string_buffer_Buf_add(out, ast_Decl_getName(d));
}

static void qbe_generator_addType(string_buffer_Buf* out, ast_QualType qt)
{
   const ast_StructType* s = ast_QualType_getStructTypeOrNil(&qt);
   if (s) {
      qbe_generator_addStructName(out, ((ast_Decl*)(ast_StructType_getDecl(s))));
   } else {
      if (ast_QualType_getAlignment(&qt) == 8) string_buffer_Buf_add1(out, 'l');
      else string_buffer_Buf_add1(out, 'w');
   }
}

static uint32_t qbe_generator_Generator_addLocal(qbe_generator_Generator* gen, ast_VarDecl* vd)
{
   ast_QualType qt = ast_Decl_getType(ast_VarDecl_asDecl(vd));
   string_buffer_Buf* start = gen->start->buf;
   const ast_StructType* s = ast_QualType_getStructTypeOrNil(&qt);
   uint32_t idx;
   if (s) {
      const ast_StructTypeDecl* std = ast_StructType_getDecl(s);
      idx = qbe_generator_Locals_add(&gen->locals, vd, ast_StructTypeDecl_getSize(std), ast_StructTypeDecl_getAlignment(std));
   } else {
      uint32_t w = ast_QualType_getAlignment(&qt);
      idx = qbe_generator_Locals_add(&gen->locals, vd, w, w);
      switch (w) {
      case 1:
         string_buffer_Buf_print(start, "\t%%.%u =l alloc4 1\n", idx);
         break;
      case 2:
         string_buffer_Buf_print(start, "\t%%.%u =l alloc4 2\n", idx);
         break;
      case 4:
         string_buffer_Buf_print(start, "\t%%.%u =l alloc4 4\n", idx);
         break;
      case 8:
         string_buffer_Buf_print(start, "\t%%.%u =l alloc8 8\n", idx);
         break;
      default:
         assert(0);
         break;
      }
   }
   return idx;
}

static void qbe_generator_Generator_addParam(qbe_generator_Generator* gen, string_buffer_Buf* out, ast_VarDecl* vd)
{
   ast_QualType qt = ast_Decl_getType(ast_VarDecl_asDecl(vd));
   string_buffer_Buf* start = gen->start->buf;
   const ast_StructType* s = ast_QualType_getStructTypeOrNil(&qt);
   uint32_t idx;
   if (s) {
      const ast_StructTypeDecl* std = ast_StructType_getDecl(s);
      qbe_generator_addStructName(out, ((ast_Decl*)(std)));
      idx = qbe_generator_Locals_add(&gen->locals, vd, ast_StructTypeDecl_getSize(std), ast_StructTypeDecl_getAlignment(std));
   } else {
      uint32_t w = ast_QualType_getAlignment(&qt);
      idx = qbe_generator_Locals_add(&gen->locals, vd, w, w);
      uint32_t next_idx = qbe_generator_Locals_next(&gen->locals, vd);
      switch (w) {
      case 1:
         string_buffer_Buf_add1(out, 'w');
         string_buffer_Buf_print(start, "\t%%.%u =l alloc4 1\n", next_idx);
         string_buffer_Buf_print(start, "\tstoreb %%.%u, %%.%u\n", idx, next_idx);
         break;
      case 2:
         string_buffer_Buf_add1(out, 'w');
         string_buffer_Buf_print(start, "\t%%.%u =l alloc4 2\n", next_idx);
         string_buffer_Buf_print(start, "\tstoreh %%.%u, %%.%u\n", idx, next_idx);
         break;
      case 4:
         string_buffer_Buf_add1(out, 'w');
         string_buffer_Buf_print(start, "\t%%.%u =l alloc4 4\n", next_idx);
         string_buffer_Buf_print(start, "\tstorew %%.%u, %%.%u\n", idx, next_idx);
         break;
      case 8:
         string_buffer_Buf_add1(out, 'l');
         string_buffer_Buf_print(start, "\t%%.%u =l alloc8 8\n", next_idx);
         string_buffer_Buf_print(start, "\tstorel %%.%u, %%.%u\n", idx, next_idx);
         break;
      default:
         assert(0);
         break;
      }
   }
   string_buffer_Buf_print(out, " %%.%u", idx);
}

static char qbe_generator_align2char(uint32_t align)
{
   if (align == 8) return 'l';

   if (align == 4) return 'w';

   if (align == 2) return 's';

   if (align == 1) return 'b';

   printf("WIDTH %u\n", align);
   assert(0);
   return '?';
}

static char qbe_generator_align2store(uint32_t align)
{
   if (align == 4) return 'w';

   if (align == 2) return 'h';

   if (align == 1) return 'b';

   return 'l';
}

static void qbe_generator_Generator_doFunctionBody(qbe_generator_Generator* gen, const ast_FunctionDecl* fd, string_buffer_Buf* out)
{
   uint32_t var_idx = ast_FunctionDecl_getNumParams(fd);
   bool have_ret = false;
   ast_CompoundStmt* body = ast_FunctionDecl_getBody(fd);
   const uint32_t num_stmts = ast_CompoundStmt_getCount(body);
   ast_Stmt** stmts = ast_CompoundStmt_getStmts(body);
   for (uint32_t i = 0; i < num_stmts; i++) {
      const ast_Stmt* s = stmts[i];
      switch (ast_Stmt_getKind(s)) {
      case ast_StmtKind_Return: {
         const ast_ReturnStmt* r = ((ast_ReturnStmt*)(s));
         string_buffer_Buf_add(out, "\tret");
         const ast_Expr* retval = ast_ReturnStmt_getValue(r);
         if (retval) {
            string_buffer_Buf_add1(out, ' ');
            qbe_generator_Generator_doExpr(gen, out, retval);
         }
         string_buffer_Buf_add1(out, '\n');
         have_ret = true;
         i = num_stmts;
         break;
      }
      case ast_StmtKind_Expr: {
         const ast_Expr* e = ((ast_Expr*)(s));
         if (ast_Expr_getKind(e) == ast_ExprKind_Call) {
            ast_CallExpr* ce = ((ast_CallExpr*)(e));
            uint32_t num_args = ast_CallExpr_getNumArgs(ce);
            ast_Expr** args = ast_CallExpr_getArgs(ce);
            const char* name = "$printf";
            string_buffer_Buf_print(out, "\t%%.%u = w call %s(", var_idx, name);
            var_idx++;
            for (uint32_t a = 0; a < num_args; a++) {
               const ast_Expr* arg = args[a];
               if (a != 0) string_buffer_Buf_add(out, ", ");
               string_buffer_Buf_add(out, "l ");
               qbe_generator_Generator_doExpr(gen, out, arg);
            }
            string_buffer_Buf_add(out, ")\n");
         }
         break;
      }
      case ast_StmtKind_Decl: {
         const ast_DeclStmt* ds = ((ast_DeclStmt*)(s));
         ast_VarDecl* vd = ast_DeclStmt_getDecl(ds);
         uint32_t idx = qbe_generator_Generator_addLocal(gen, vd);
         const ast_Expr* ie = ast_VarDecl_getInit(vd);
         if (ie) {
            if (ast_Expr_getKind(ie) == ast_ExprKind_IntegerLiteral) {
               ast_IntegerLiteral* il = ((ast_IntegerLiteral*)(ie));
               ast_QualType qt = ast_Decl_getType(ast_VarDecl_asDecl(vd));
               string_buffer_Buf_print(out, "\tstore%c ", qbe_generator_align2store(ast_QualType_getAlignment(&qt)));
               ast_IntegerLiteral_printDecimal(il, out);
               string_buffer_Buf_print(out, ", %%.%u\n", idx);
            }
         }
         break;
      }
      default:
         break;
      }
   }
   if (!have_ret) string_buffer_Buf_add(out, "\tret\n");
}

static void qbe_generator_Generator_handleFunction(qbe_generator_Generator* gen, ast_Decl* d)
{
   qbe_generator_Locals_clear(&gen->locals);
   qbe_generator_Fragment* f = qbe_generator_Generator_getFragment(gen);
   ast_FunctionDecl* fd = ((ast_FunctionDecl*)(d));
   string_buffer_Buf* out = f->buf;
   if (ast_Decl_isPublic(d)) string_buffer_Buf_add(out, "export ");
   string_buffer_Buf_add(out, "function ");
   if (ast_FunctionDecl_hasReturn(fd)) {
      qbe_generator_addType(out, ast_FunctionDecl_getRType(fd));
      string_buffer_Buf_add1(out, ' ');
   }
   string_buffer_Buf_add1(out, '$');
   bool has_prefix = (!ast_Decl_isPublic(d) || (strcmp(ast_Decl_getName(d), "main") != 0));
   if (has_prefix) {
      string_buffer_Buf_add(out, ast_Decl_getModuleName(d));
      string_buffer_Buf_add1(out, '_');
   }
   const ast_Ref* prefix = ast_FunctionDecl_getPrefix(fd);
   if (prefix) {
      string_buffer_Buf_add(out, ast_Ref_getName(prefix));
      string_buffer_Buf_add1(out, '_');
   }
   string_buffer_Buf_add(out, ast_Decl_getName(d));
   string_buffer_Buf_add1(out, '(');
   uint32_t num_params = ast_FunctionDecl_getNumParams(fd);
   gen->start = qbe_generator_Generator_getFragment(gen);
   string_buffer_Buf_print(gen->start->buf, "@start.%u\n", gen->func_idx);
   gen->data = qbe_generator_Generator_getFragment(gen);
   qbe_generator_Generator_addFragment(gen, gen->data);
   qbe_generator_Generator_addFragment(gen, f);
   qbe_generator_Generator_addFragment(gen, gen->start);
   ast_VarDecl** fn_params = ast_FunctionDecl_getParams(fd);
   for (uint32_t i = 0; i < num_params; i++) {
      if (i != 0) string_buffer_Buf_add(out, ", ");
      qbe_generator_Generator_addParam(gen, out, fn_params[i]);
   }
   if (ast_FunctionDecl_isVariadic(fd)) {
      if (num_params) string_buffer_Buf_add(out, ", ");
      string_buffer_Buf_add(out, "...");
   }
   string_buffer_Buf_add(out, ") {\n");
   qbe_generator_Fragment* body = qbe_generator_Generator_getFragment(gen);
   qbe_generator_Generator_addFragment(gen, body);
   out = body->buf;
   string_buffer_Buf_print(out, "@body.%u\n", gen->func_idx);
   qbe_generator_Generator_doFunctionBody(gen, fd, out);
   string_buffer_Buf_add(out, "}\n");
   string_buffer_Buf_add1(out, '\n');
   gen->func_idx++;
   gen->start = NULL;
   if (string_buffer_Buf_size(gen->data->buf)) string_buffer_Buf_add1(gen->data->buf, '\n');
}

static void qbe_generator_Generator_doArrayInit(qbe_generator_Generator* gen, string_buffer_Buf* out, const ast_ArrayType* at, const ast_Expr* e)
{
   uint32_t num_elems = ast_ArrayType_getSize(at);
   ast_QualType elem = ast_ArrayType_getElemType(at);
   uint32_t elemSize = ast_QualType_getSize(&elem);
   uint32_t len;
   if (ast_Expr_getKind(e) == ast_ExprKind_StringLiteral) {
      const ast_StringLiteral* str = ((ast_StringLiteral*)(e));
      const char* text = ast_StringLiteral_getText(str);
      string_buffer_Buf_print(out, "b \"%s\\000\"", text);
      len = ((uint32_t)(strlen(text))) + 1;
   } else {
      assert(ast_Expr_getKind(e) == ast_ExprKind_InitList);
      ast_InitListExpr* ile = ((ast_InitListExpr*)(e));
      uint32_t count = ast_InitListExpr_getNumValues(ile);
      ast_Expr** inits = ast_InitListExpr_getValues(ile);
      const ast_StructType* st = ast_QualType_getStructTypeOrNil(&elem);
      if (st) {
         for (uint32_t i = 0; i < count; i++) {
            if (i != 0) string_buffer_Buf_add(out, ", ");
            qbe_generator_Generator_doStructInit(gen, out, st, inits[i]);
         }
      } else {
         char name = qbe_generator_align2store(elemSize);
         for (uint32_t i = 0; i < count; i++) {
            if (i != 0) string_buffer_Buf_add(out, ", ");
            string_buffer_Buf_print(out, "%c ", name);
            qbe_generator_Generator_doExpr(gen, out, inits[i]);
         }
      }
      len = count * elemSize;
   }
   uint32_t num_zeroes = (num_elems * elemSize) - len;
   if (num_zeroes) {
      if (len) string_buffer_Buf_add(out, ", ");
      string_buffer_Buf_print(out, "z %u", num_zeroes);
   }
}

static void qbe_generator_Generator_doStructInit(qbe_generator_Generator* gen, string_buffer_Buf* out, const ast_StructType* st, const ast_Expr* e)
{
}

static uint32_t qbe_generator_Generator_createString(qbe_generator_Generator* gen, const ast_Expr* e)
{
   ast_StringLiteral* s = ((ast_StringLiteral*)(e));
   uint32_t idx = gen->string_idx;
   gen->string_idx++;
   string_buffer_Buf_print(gen->data->buf, "data $string.%u = align 1 { b \"%s\\000\" }\n", idx, ast_StringLiteral_getText(s));
   return idx;
}

static void qbe_generator_Generator_doExpr(qbe_generator_Generator* gen, string_buffer_Buf* out, const ast_Expr* e)
{
   switch (ast_Expr_getKind(e)) {
   case ast_ExprKind_IntegerLiteral: {
      const ast_IntegerLiteral* i = ((ast_IntegerLiteral*)(e));
      string_buffer_Buf_print(out, "%lu", ast_IntegerLiteral_getValue(i));
      return;
   }
   case ast_ExprKind_BooleanLiteral: {
      const ast_BooleanLiteral* b = ((ast_BooleanLiteral*)(e));
      string_buffer_Buf_print(out, "%u", ast_BooleanLiteral_getValue(b));
      return;
   }
   case ast_ExprKind_CharLiteral: {
      const ast_CharLiteral* c = ((ast_CharLiteral*)(e));
      string_buffer_Buf_print(out, "%u", ast_CharLiteral_getValue(c));
      return;
   }
   case ast_ExprKind_StringLiteral:
      string_buffer_Buf_print(out, "$string.%u", qbe_generator_Generator_createString(gen, e));
      return;
   case ast_ExprKind_Nil:
      string_buffer_Buf_add1(out, '0');
      return;
   case ast_ExprKind_Identifier: {
      ast_IdentifierExpr* i = ((ast_IdentifierExpr*)(e));
      qbe_generator_addGlobalName(out, ast_IdentifierExpr_getDecl(i));
      return;
   }
   case ast_ExprKind_Type:
      assert(0);
      return;
   case ast_ExprKind_Call:
      // fallthrough
   case ast_ExprKind_InitList:
      // fallthrough
   case ast_ExprKind_FieldDesignatedInit:
      // fallthrough
   case ast_ExprKind_ArrayDesignatedInit:
      // fallthrough
   case ast_ExprKind_BinaryOperator:
      // fallthrough
   case ast_ExprKind_UnaryOperator:
      // fallthrough
   case ast_ExprKind_ConditionalOperator:
      // fallthrough
   case ast_ExprKind_Builtin:
      // fallthrough
   case ast_ExprKind_ArraySubscript:
      // fallthrough
   case ast_ExprKind_Member:
      // fallthrough
   case ast_ExprKind_Paren:
      // fallthrough
   case ast_ExprKind_BitOffset:
      // fallthrough
   case ast_ExprKind_ExplicitCast:
      return;
   case ast_ExprKind_ImplicitCast: {
      ast_ImplicitCastExpr* ic = ((ast_ImplicitCastExpr*)(e));
      qbe_generator_Generator_doExpr(gen, out, ast_ImplicitCastExpr_getInner(ic));
      return;
   }
   }
   if (ast_Expr_isCtv(e)) {
      ctv_analyser_Value v = ctv_analyser_get_value(e);
      string_buffer_Buf_print(out, "%lu", v.uvalue);
   } else {
      string_buffer_Buf_add(out, "TODO");
   }
}

static void qbe_generator_Generator_handleVarDecl(qbe_generator_Generator* gen, ast_Decl* d)
{
   if (!ast_Decl_isUsed(d)) return;

   const ast_VarDecl* vd = ((ast_VarDecl*)(d));
   qbe_generator_Fragment* f = qbe_generator_Generator_getFragment(gen);
   string_buffer_Buf* out = f->buf;
   ast_QualType qt = ast_Decl_getType(d);
   uint32_t align = ast_QualType_getAlignment(&qt);
   uint32_t size = ast_QualType_getSize(&qt);
   if (ast_Decl_isPublic(d)) string_buffer_Buf_add(out, "export ");
   string_buffer_Buf_add(out, "data ");
   qbe_generator_addGlobalName(out, d);
   string_buffer_Buf_print(out, " = align %u { ", align);
   const ast_Expr* initExpr = ast_VarDecl_getInit(vd);
   if (initExpr) {
      ast_ArrayType* at = ast_QualType_getArrayTypeOrNil(&qt);
      const ast_StructType* st = ast_QualType_getStructTypeOrNil(&qt);
      if (at) {
         qbe_generator_Generator_doArrayInit(gen, out, at, initExpr);
      } else if (st) {
         qbe_generator_Generator_doStructInit(gen, out, st, initExpr);
      } else {
         string_buffer_Buf_print(out, "%c ", qbe_generator_align2store(size));
         qbe_generator_Generator_doExpr(gen, out, initExpr);
      }

   } else {
      string_buffer_Buf_print(out, "z %u", size);
   }
   string_buffer_Buf_add(out, " }\n");
   qbe_generator_Generator_addFragment(gen, f);
}

static void qbe_generator_addMember(string_buffer_Buf* out, ast_QualType qt)
{
   const ast_StructType* s = ast_QualType_getStructTypeOrNil(&qt);
   if (s) {
      qbe_generator_addStructName(out, ((ast_Decl*)(ast_StructType_getDecl(s))));
      return;
   }
   const ast_ArrayType* a = ast_QualType_getArrayTypeOrNil(&qt);
   uint32_t align = ast_QualType_getAlignment(&qt);
   if (a) {
      uint32_t size = ast_ArrayType_getSize(a);
      while (1) {
         qt = ast_ArrayType_getElemType(a);
         a = ast_QualType_getArrayTypeOrNil(&qt);
         if (!a) break;

         size *= ast_ArrayType_getSize(a);
      }
      string_buffer_Buf_add1(out, qbe_generator_align2char(align));
      string_buffer_Buf_print(out, " %u", size);
      return;
   }
   string_buffer_Buf_add1(out, qbe_generator_align2char(align));
}

static uint32_t qbe_generator_Generator_createStruct(qbe_generator_Generator* gen, string_buffer_Buf* out, ast_StructTypeDecl* s, bool is_global)
{
   if (ast_Decl_isGenerated(ast_StructTypeDecl_asDecl(s))) return 0;

   const uint32_t num_members = ast_StructTypeDecl_getNumMembers(s);
   ast_Decl** members = ast_StructTypeDecl_getMembers(s);
   uint32_t anon_id = 0;
   string_buffer_Buf_add(out, "type ");
   if (is_global) {
      qbe_generator_addStructName(out, ((ast_Decl*)(s)));
   } else {
      anon_id = gen->substruct_idx;
      string_buffer_Buf_print(out, ":anon%u", anon_id);
      gen->substruct_idx++;
   }
   string_buffer_Buf_add(out, " = { ");
   bool is_union = ast_StructTypeDecl_isUnion(s);
   const char* inter = is_union ? " " : ", ";
   for (uint32_t i = 0; i < num_members; i++) {
      ast_Decl* member = members[i];
      if (i != 0) string_buffer_Buf_add(out, inter);
      if (is_union) string_buffer_Buf_add1(out, '{');
      if (ast_Decl_getKind(member) == ast_DeclKind_StructType) {
         qbe_generator_Fragment* f = qbe_generator_Generator_getFragment(gen);
         uint32_t sub_id = qbe_generator_Generator_createStruct(gen, f->buf, ((ast_StructTypeDecl*)(member)), false);
         string_buffer_Buf_print(out, ":anon%u", sub_id);
         qbe_generator_Generator_addFragment(gen, f);
      } else {
         assert(ast_Decl_getKind(member) == ast_DeclKind_Var);
         ast_QualType qt = ast_Decl_getType(member);
         ast_StructType* st = ast_QualType_getStructTypeOrNil(&qt);
         if (st) {
            ast_StructTypeDecl* s2 = ast_StructType_getDecl(st);
            if (!ast_Decl_isGenerated(ast_StructTypeDecl_asDecl(s2))) {
               qbe_generator_Fragment* f = qbe_generator_Generator_getFragment(gen);
               qbe_generator_Generator_createStruct(gen, f->buf, s2, true);
               qbe_generator_Generator_addFragment(gen, f);
            }
         }
         qbe_generator_addMember(out, ast_Decl_getType(member));
      }
      if (is_union) string_buffer_Buf_add1(out, '}');
   }
   string_buffer_Buf_add(out, " }\n");
   ast_Decl_setGenerated(ast_StructTypeDecl_asDecl(s));
   return anon_id;
}

static void qbe_generator_Generator_handleStruct(qbe_generator_Generator* gen, ast_Decl* d)
{
   if (ast_Decl_isGenerated(d)) return;

   ast_StructTypeDecl* s = ((ast_StructTypeDecl*)(d));
   qbe_generator_Fragment* f = qbe_generator_Generator_getFragment(gen);
   qbe_generator_Generator_createStruct(gen, f->buf, s, true);
   qbe_generator_Generator_addFragment(gen, f);
}

static void qbe_generator_Generator_on_decl(void* arg, ast_Decl* d)
{
   qbe_generator_Generator* gen = arg;
   switch (ast_Decl_getKind(d)) {
   case ast_DeclKind_Function:
      if (!gen->cur_external) qbe_generator_Generator_handleFunction(gen, d);
      break;
   case ast_DeclKind_Import:
      return;
   case ast_DeclKind_StructType:
      qbe_generator_Generator_handleStruct(gen, d);
      break;
   case ast_DeclKind_EnumType:
      break;
   case ast_DeclKind_EnumConstant:
      break;
   case ast_DeclKind_FunctionType:
      break;
   case ast_DeclKind_AliasType:
      break;
   case ast_DeclKind_Var:
      qbe_generator_Generator_handleVarDecl(gen, d);
      break;
   case ast_DeclKind_StaticAssert:
      break;
   }
   while (!linked_list_Element_isEmpty(&gen->used_list)) {
      linked_list_Element* e = linked_list_Element_popFront(&gen->used_list);
      qbe_generator_Fragment* f = to_container(qbe_generator_Fragment, list, e);
      string_buffer_Buf_add2(gen->out, string_buffer_Buf_data(f->buf), string_buffer_Buf_size(f->buf));
      qbe_generator_Generator_freeFragment(gen, f);
   }
}

static void qbe_generator_Generator_on_ast(void* arg, ast_AST* a)
{
   ast_AST_visitDecls(a, qbe_generator_Generator_on_decl, arg);
}

static void qbe_generator_Generator_on_module(void* arg, ast_Module* m)
{
   if (ast_Module_isUsed(m)) {
      qbe_generator_Generator* gen = arg;
      string_buffer_Buf_print(gen->out, "\n# --- module %s ---\n\n", ast_Module_getName(m));
      ast_Module_visitASTs(m, qbe_generator_Generator_on_ast, arg);
   }
}

static void qbe_generator_Generator_init(qbe_generator_Generator* gen, const char* target, const char* output_dir)
{
   memset(gen, 0, sizeof(qbe_generator_Generator));
   gen->out = string_buffer_create(256 * 1024, false, 1);
   gen->target = target;
   gen->output_dir = output_dir;
   gen->func_idx = 1;
   gen->string_idx = 1;
   linked_list_Element_init(&gen->free_list);
   linked_list_Element_init(&gen->used_list);
   qbe_generator_Locals_init(&gen->locals);
}

static void qbe_generator_Generator_free(qbe_generator_Generator* gen)
{
   while (!linked_list_Element_isEmpty(&gen->free_list)) {
      linked_list_Element* e = linked_list_Element_popFront(&gen->free_list);
      qbe_generator_Fragment* f = to_container(qbe_generator_Fragment, list, e);
      qbe_generator_Fragment_free(f);
   }
   string_buffer_Buf_free(gen->out);
}

static void qbe_generator_Generator_write(qbe_generator_Generator* gen, const char* output_dir, const char* filename)
{
   char fullname[256];
   sprintf(fullname, "%s/%s", output_dir, filename);
   file_utils_Writer writer;
   bool ok = file_utils_Writer_write(&writer, fullname, ((uint8_t*)(string_buffer_Buf_data(gen->out))), string_buffer_Buf_size(gen->out));
   if (!ok) {
      fprintf(stderr, "error writing %s: %s\n", fullname, strerror(*__errno_location()));
   }
}

static void qbe_generator_Generator_createMakefile(qbe_generator_Generator* gen, const char* output_dir)
{
   string_buffer_Buf* out = gen->out;
   string_buffer_Buf_clear(out);
   string_buffer_Buf_add(out, "# This makefile is auto-generated, any modifications will be lost\n\n");
   string_buffer_Buf_print(out, "../%s: main.o\n", gen->target);
   string_buffer_Buf_print(out, "\t\tgcc main.o -o ../%s\n\n", gen->target);
   string_buffer_Buf_add(out, "main.o: main.s\n");
   string_buffer_Buf_add(out, "\t\tas main.s -o main.o\n\n");
   string_buffer_Buf_add(out, "main.s: main.qbe\n");
   string_buffer_Buf_add(out, "\t\tqbe -t amd64_sysv main.qbe -o main.s\n\n");
   string_buffer_Buf_add(out, "clean:\n");
   string_buffer_Buf_add(out, "\t\trm -f main.o main.s test\n\n");
   qbe_generator_Generator_write(gen, output_dir, "Makefile");
}

static void qbe_generator_generate(const char* target, const char* output_dir, component_Component** comps, uint32_t count, bool print)
{
   char qbe_dir[256];
   sprintf(qbe_dir, "%s/%s", output_dir, qbe_generator_QBE_Dir);
   int32_t err = file_utils_create_directory(qbe_dir);
   if (err) {
      fprintf(stderr, "Error creating directory %s: %s\n", qbe_dir, strerror(err));
      return;
   }
   qbe_generator_Generator gen;
   qbe_generator_Generator_init(&gen, target, qbe_dir);
   for (uint32_t i = 0; i < count; i++) {
      component_Component* c = comps[i];
      gen.cur_external = component_Component_isExternal(c);
      component_Component_visitModules(c, qbe_generator_Generator_on_module, &gen);
   }
   if (print) puts(string_buffer_Buf_data(gen.out));
   qbe_generator_Generator_write(&gen, qbe_dir, qbe_generator_QBE_Filename);
   qbe_generator_Generator_createMakefile(&gen, qbe_dir);
   qbe_generator_Generator_free(&gen);
}

static void qbe_generator_build(const char* output_dir)
{
   char dir[256];
   sprintf(dir, "%s/%s/", output_dir, qbe_generator_QBE_Dir);
   int32_t retval = process_utils_run(dir, "/usr/bin/make", qbe_generator_LogFile);
   if (retval != 0) {
      fprintf(stderr, "Error during external QBE compilation\n");
      fprintf(stderr, "see %s%s for defails\n", dir, qbe_generator_LogFile);
   }
}

static void qbe_generator_Locals_init(qbe_generator_Locals* l)
{
   qbe_generator_Locals_clear(l);
   qbe_generator_Locals_resize(l, 32);
}

static void qbe_generator_Locals_free(qbe_generator_Locals* l)
{
   free(l->vars);
}

static void qbe_generator_Locals_clear(qbe_generator_Locals* l)
{
   l->count = 0;
   l->index = 1;
}

static void qbe_generator_Locals_resize(qbe_generator_Locals* l, uint32_t capacity)
{
   l->capacity = capacity;
   qbe_generator_Var* vars2 = malloc(capacity * sizeof(qbe_generator_Var));
   if (l->count) {
      memcpy(vars2, l->vars, l->count * sizeof(qbe_generator_Var));
      free(l->vars);
   }
   l->vars = vars2;
}

static uint32_t qbe_generator_Locals_add(qbe_generator_Locals* l, ast_VarDecl* vd, uint32_t size, uint32_t align)
{
   if (l->count == l->capacity) qbe_generator_Locals_resize(l, l->capacity * 2);
   qbe_generator_Var* var = &l->vars[l->count];
   l->count++;
   uint32_t idx = l->index;
   l->index++;
   var->vd = vd;
   var->idx = idx;
   var->size = size;
   var->align = align;
   return idx;
}

static uint32_t qbe_generator_Locals_next(qbe_generator_Locals* l, const ast_VarDecl* vd)
{
   uint32_t idx = l->index;
   l->index++;
   qbe_generator_Var* var = qbe_generator_Locals_find(l, vd);
   assert(var);
   var->idx = idx;
   return idx;
}

static qbe_generator_Var* qbe_generator_Locals_find(const qbe_generator_Locals* l, const ast_VarDecl* vd)
{
   for (uint32_t i = 0; i < l->count; i++) {
      qbe_generator_Var* v = &l->vars[i];
      if (v->vd == vd) return v;

   }
   return NULL;
}

static uint32_t qbe_generator_Locals_getIdx(const qbe_generator_Locals* l, const ast_VarDecl* vd)
{
   qbe_generator_Var* var = qbe_generator_Locals_find(l, vd);
   if (var) return var->idx;

   return 0;
}


// --- module ast_builder ---

typedef struct ast_builder_Builder_ ast_builder_Builder;
typedef struct ast_builder_VarDeclAttrs_ ast_builder_VarDeclAttrs;

struct ast_builder_VarDeclAttrs_ {
   attr_Value aligned;
   attr_Value section;
   attr_Value cname;
   src_loc_SrcLoc weak_loc;
   bool export;
   bool unused;
   bool weak;
   bool has_attrs;
};

struct ast_builder_Builder_ {
   ast_context_Context* context;
   diagnostics_Diags* diags;
   component_Component* comp;
   ast_Module* mod;
   ast_AST* ast;
   uint32_t ast_idx;
   bool is_interface;
   ast_builder_VarDeclAttrs var_attrs;
};

static ast_builder_Builder* ast_builder_create(ast_context_Context* context, diagnostics_Diags* diags);
static void ast_builder_Builder_free(ast_builder_Builder* b);
static void ast_builder_Builder_setComponent(ast_builder_Builder* b, component_Component* comp);
static void ast_builder_Builder_actOnModule(ast_builder_Builder* b, uint32_t mod_name, src_loc_SrcLoc mod_loc, const char* filename);
static void ast_builder_Builder_actOnImport(ast_builder_Builder* b, uint32_t mod_name, src_loc_SrcLoc mod_loc, uint32_t alias_name, src_loc_SrcLoc alias_loc, bool islocal);
static ast_Decl* ast_builder_Builder_actOnAliasType(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* ref);
static ast_Decl* ast_builder_Builder_actOnFunctionTypeDecl(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* rtype, ast_VarDecl** params, uint32_t num_params, bool is_variadic);
static ast_StructTypeDecl* ast_builder_Builder_actOnStructType(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, bool is_struct, bool is_global, ast_VarDecl** members, uint32_t num_members);
static ast_VarDecl* ast_builder_Builder_actOnStructMember(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* ref, ast_Expr* bitfield);
static void ast_builder_Builder_actOnGlobalVarDecl(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, ast_TypeRefHolder* ref, ast_Expr* initValue);
static ast_VarDecl* ast_builder_Builder_actOnFunctionParam(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* ref);
static ast_Stmt* ast_builder_Builder_actOnVarDeclStmt(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, const ast_TypeRefHolder* ref, ast_Expr* initValue, bool has_local);
static void ast_builder_Builder_storeAttr(ast_builder_Builder* b, ast_Decl* d, attr_AttrKind kind, const attr_Value* value);
static void ast_builder_Builder_actOnFunctionAttr(ast_builder_Builder* b, ast_Decl* d, attr_AttrKind kind, src_loc_SrcLoc loc, const attr_Value* value);
static void ast_builder_Builder_actOnStructAttr(ast_builder_Builder* b, ast_Decl* d, attr_AttrKind kind, src_loc_SrcLoc loc, const attr_Value* value);
static void ast_builder_Builder_actOnTypeAttr(ast_builder_Builder* b, ast_Decl* d, attr_AttrKind kind, src_loc_SrcLoc loc, const attr_Value* value);
static void ast_builder_Builder_actOnVarAttr(ast_builder_Builder* b, attr_AttrKind kind, src_loc_SrcLoc loc, const attr_Value* value);
static void ast_builder_Builder_actOnAttr(ast_builder_Builder* b, ast_Decl* d, uint32_t name, src_loc_SrcLoc loc, const attr_Value* value);
static ast_QualType ast_builder_Builder_actOnBuiltinType(ast_builder_Builder* _arg0, ast_BuiltinKind kind);
static ast_QualType ast_builder_Builder_actOnPointerType(ast_builder_Builder* _arg0, ast_QualType inner);
static ast_QualType ast_builder_Builder_actOnArrayType(ast_builder_Builder* b, ast_QualType elem, bool has_size, uint32_t size);
static ast_FunctionDecl* ast_builder_Builder_actOnFunctionDecl(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* rtype, const ast_Ref* prefix, ast_VarDecl** params, uint32_t num_params, bool is_variadic);
static ast_FunctionDecl* ast_builder_Builder_actOnTemplateFunctionDecl(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* rtype, uint32_t template_name, src_loc_SrcLoc template_loc, ast_VarDecl** params, uint32_t num_params, bool is_variadic);
static void ast_builder_Builder_actOnFunctionBody(ast_builder_Builder* _arg0, ast_FunctionDecl* f, ast_CompoundStmt* body);
static ast_EnumConstantDecl* ast_builder_Builder_actOnEnumConstant(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, ast_Expr* init_expr);
static ast_Decl* ast_builder_Builder_actOnEnumType(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, bool is_incr, ast_QualType implType, ast_EnumConstantDecl** constants, uint32_t num_constants);
static ast_CompoundStmt* ast_builder_Builder_actOnCompoundStmt(ast_builder_Builder* b, src_loc_SrcLoc endLoc, ast_Stmt** stmts, uint32_t count);
static ast_Stmt* ast_builder_Builder_actOnReturnStmt(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* ret);
static ast_Stmt* ast_builder_Builder_actOnIfStmt(ast_builder_Builder* b, ast_Stmt* cond, ast_Stmt* then, ast_Stmt* else_stmt);
static ast_Stmt* ast_builder_Builder_actOnDoStmt(ast_builder_Builder* b, ast_Stmt* cond, ast_Stmt* then);
static ast_Stmt* ast_builder_Builder_actOnWhileStmt(ast_builder_Builder* b, ast_Stmt* cond, ast_Stmt* then);
static ast_Stmt* ast_builder_Builder_actOnForStmt(ast_builder_Builder* b, ast_Stmt* init_, ast_Expr* cond, ast_Expr* incr, ast_Stmt* body);
static ast_Stmt* ast_builder_Builder_actOnSwitchStmt(ast_builder_Builder* b, ast_Stmt* cond, ast_Stmt** cases, uint32_t num_cases, bool is_sswitch);
static ast_Stmt* ast_builder_Builder_actOnDefaultStmt(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Stmt** stmts, uint32_t num_stmts);
static ast_Stmt* ast_builder_Builder_actOnCaseStmt(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* cond, ast_Stmt** stmts, uint32_t num_stmts);
static ast_Stmt* ast_builder_Builder_actOnAssertStmt(ast_builder_Builder* b, ast_Expr* inner);
static ast_Stmt* ast_builder_Builder_actOnBreakStmt(ast_builder_Builder* b, src_loc_SrcLoc loc);
static ast_Stmt* ast_builder_Builder_actOnContinueStmt(ast_builder_Builder* b, src_loc_SrcLoc loc);
static ast_Stmt* ast_builder_Builder_actOnFallthroughStmt(ast_builder_Builder* b, src_loc_SrcLoc loc);
static ast_Stmt* ast_builder_Builder_actOnLabelStmt(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc);
static ast_Stmt* ast_builder_Builder_actOnGotoStmt(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc);
static ast_IdentifierExpr* ast_builder_Builder_actOnIdentifier(ast_builder_Builder* b, src_loc_SrcLoc loc, uint32_t name);
static ast_Expr* ast_builder_Builder_actOnIntegerLiteral(ast_builder_Builder* b, src_loc_SrcLoc loc, uint8_t radix, uint64_t value);
static ast_Expr* ast_builder_Builder_actOnCharLiteral(ast_builder_Builder* b, src_loc_SrcLoc loc, uint8_t value);
static ast_Expr* ast_builder_Builder_actOnStringLiteral(ast_builder_Builder* b, src_loc_SrcLoc loc, uint32_t value, uint32_t len);
static ast_Expr* ast_builder_Builder_actOnNilExpr(ast_builder_Builder* b, src_loc_SrcLoc loc);
static ast_Expr* ast_builder_Builder_actOnParenExpr(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* inner);
static ast_Expr* ast_builder_Builder_actOnUnaryOperator(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_UnaryOpcode opcode, ast_Expr* inner);
static ast_Expr* ast_builder_Builder_actOnPostFixUnaryOperator(ast_builder_Builder* b, src_loc_SrcLoc loc, token_Kind kind, ast_Expr* inner);
static ast_Expr* ast_builder_Builder_actOnBinaryOperator(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_BinaryOpcode opcode, ast_Expr* lhs, ast_Expr* rhs);
static ast_Expr* ast_builder_Builder_actOnConditionalOperator(ast_builder_Builder* b, src_loc_SrcLoc questionLoc, src_loc_SrcLoc colonLoc, ast_Expr* cond, ast_Expr* lhs, ast_Expr* rhs);
static ast_Expr* ast_builder_Builder_actOnBooleanConstant(ast_builder_Builder* b, src_loc_SrcLoc loc, bool value);
static ast_Expr* ast_builder_Builder_actOnBuiltinExpr(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* inner, ast_BuiltinExprKind kind);
static ast_Expr* ast_builder_Builder_actOnOffsetOfExpr(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* structExpr, ast_Expr* member);
static ast_Expr* ast_builder_Builder_actOnToContainerExpr(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* structExpr, ast_Expr* member, ast_Expr* pointer);
static ast_Expr* ast_builder_Builder_actOnTypeExpr(ast_builder_Builder* b, src_loc_SrcLoc loc, const ast_TypeRefHolder* ref);
static ast_Expr* ast_builder_Builder_actOnBitOffsetExpr(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* lhs, ast_Expr* rhs);
static ast_Expr* ast_builder_Builder_actOnArraySubscriptExpr(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* base, ast_Expr* idx);
static ast_Expr* ast_builder_Builder_actOnCallExpr(ast_builder_Builder* b, src_loc_SrcLoc endLoc, ast_Expr* fn, ast_Expr** args, uint32_t num_args);
static ast_Expr* ast_builder_Builder_actOnTemplateCallExpr(ast_builder_Builder* b, src_loc_SrcLoc endLoc, ast_Expr* fn, ast_Expr** args, uint32_t num_args, const ast_TypeRefHolder* ref);
static ast_Expr* ast_builder_Builder_actOnExplicitCast(ast_builder_Builder* b, src_loc_SrcLoc loc, const ast_TypeRefHolder* ref, ast_Expr* inner);
static ast_Expr* ast_builder_Builder_actOnMemberExpr(ast_builder_Builder* b, ast_Expr* base, const ast_Ref* refs, uint32_t refcount);
static ast_Expr* ast_builder_Builder_actOnInitList(ast_builder_Builder* b, src_loc_SrcLoc left, src_loc_SrcLoc right, ast_Expr** values, uint32_t num_values);
static ast_Expr* ast_builder_Builder_actOnFieldDesignatedInit(ast_builder_Builder* b, uint32_t field, src_loc_SrcLoc loc, ast_Expr* initValue);
static ast_Expr* ast_builder_Builder_actOnArrayDesignatedInit(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* designator, ast_Expr* initValue);
static void ast_builder_Builder_actOnStaticAssert(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* lhs, ast_Expr* rhs);
static void ast_builder_Builder_insertImplicitCast(ast_builder_Builder* b, ast_ImplicitCastKind kind, ast_Expr** e_ptr, ast_QualType qt);
static void ast_builder_Builder_addSymbol(ast_builder_Builder* b, uint32_t name_idx, ast_Decl* d);

static ast_builder_Builder* ast_builder_create(ast_context_Context* context, diagnostics_Diags* diags)
{
   ast_builder_Builder* b = calloc(1, sizeof(ast_builder_Builder));
   b->context = context;
   b->diags = diags;
   return b;
}

static void ast_builder_Builder_free(ast_builder_Builder* b)
{
   free(b);
}

static void ast_builder_Builder_setComponent(ast_builder_Builder* b, component_Component* comp)
{
   b->comp = comp;
   b->mod = NULL;
   b->is_interface = component_Component_isExternal(comp);
}

static void ast_builder_Builder_actOnModule(ast_builder_Builder* b, uint32_t mod_name, src_loc_SrcLoc mod_loc, const char* filename)
{
   assert(b->comp);
   b->mod = component_Component_getOrAddModule(b->comp, mod_name);
   if (!b->mod) {
      diagnostics_Diags_error(b->diags, mod_loc, "module %s is already defined in another component", mod_name);
      exit(-1);
   }
   b->ast = ast_Module_add(b->mod, filename);
   b->ast_idx = ast_AST_getIdx(b->ast);
   ast_ImportDecl* i = ast_ImportDecl_create(b->context, mod_name, mod_loc, 0, 0, b->ast_idx, true);
   ast_Decl* d = ((ast_Decl*)(i));
   ast_Decl_setUsed(d);
   ast_Decl_setChecked(d);
   ast_ImportDecl_setDest(i, b->mod);
   ast_Decl_setType(d, ast_QualType_init(((ast_Type*)(ast_Module_getType(b->mod)))));
   ast_AST_addImport(b->ast, i);
}

static void ast_builder_Builder_actOnImport(ast_builder_Builder* b, uint32_t mod_name, src_loc_SrcLoc mod_loc, uint32_t alias_name, src_loc_SrcLoc alias_loc, bool islocal)
{
   if (ast_AST_getNameIdx(b->ast) == mod_name) {
      diagnostics_Diags_error(b->diags, mod_loc, "cannot import own module %s", ast_idx2name(mod_name));
      return;
   }
   ast_ImportDecl* old = ast_AST_findImport(b->ast, mod_name);
   if (old) {
      diagnostics_Diags_error(b->diags, mod_loc, "duplicate import of module %s", ast_idx2name(mod_name));
      diagnostics_Diags_note(b->diags, ast_Decl_getLoc(ast_ImportDecl_asDecl(old)), "previous import is here");
      return;
   }
   ast_ImportDecl* d = ast_ImportDecl_create(b->context, mod_name, mod_loc, alias_name, alias_loc, b->ast_idx, islocal);
   ast_AST_addImport(b->ast, d);
}

static ast_Decl* ast_builder_Builder_actOnAliasType(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* ref)
{
   is_public |= b->is_interface;
   ast_AliasTypeDecl* d = ast_AliasTypeDecl_create(b->context, name, loc, is_public, b->ast_idx, ref);
   ast_AST_addTypeDecl(b->ast, ast_AliasTypeDecl_asDecl(d));
   ast_Decl* dd = ((ast_Decl*)(d));
   if (b->is_interface) ast_Decl_setExternal(dd);
   ast_builder_Builder_addSymbol(b, name, dd);
   return dd;
}

static ast_Decl* ast_builder_Builder_actOnFunctionTypeDecl(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* rtype, ast_VarDecl** params, uint32_t num_params, bool is_variadic)
{
   is_public |= b->is_interface;
   ast_FunctionDecl* fn = ast_FunctionDecl_create(b->context, name, loc, is_public, b->ast_idx, rtype, NULL, params, num_params, is_variadic);
   ast_FunctionTypeDecl* d = ast_FunctionTypeDecl_create(b->context, fn);
   ast_AST_addTypeDecl(b->ast, ast_FunctionTypeDecl_asDecl(d));
   ast_Decl* dd = ((ast_Decl*)(d));
   if (b->is_interface) {
      ast_Decl_setExternal(dd);
      ast_Decl_setExternal(ast_FunctionDecl_asDecl(fn));
   }
   ast_builder_Builder_addSymbol(b, name, dd);
   return dd;
}

static ast_StructTypeDecl* ast_builder_Builder_actOnStructType(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, bool is_struct, bool is_global, ast_VarDecl** members, uint32_t num_members)
{
   is_public |= b->is_interface;
   ast_StructTypeDecl* d = ast_StructTypeDecl_create(b->context, name, loc, is_public, b->ast_idx, is_struct, is_global, members, num_members);
   if (is_global) {
      ast_AST_addTypeDecl(b->ast, ast_StructTypeDecl_asDecl(d));
      ast_builder_Builder_addSymbol(b, name, ast_StructTypeDecl_asDecl(d));
   }
   if (b->is_interface) ast_Decl_setExternal(ast_StructTypeDecl_asDecl(d));
   return d;
}

static ast_VarDecl* ast_builder_Builder_actOnStructMember(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* ref, ast_Expr* bitfield)
{
   is_public |= b->is_interface;
   return ast_VarDecl_createStructMember(b->context, name, loc, is_public, ref, b->ast_idx, bitfield);
}

static void ast_builder_Builder_actOnGlobalVarDecl(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, ast_TypeRefHolder* ref, ast_Expr* initValue)
{
   is_public |= b->is_interface;
   ast_VarDecl* vd = ast_VarDecl_create(b->context, ast_VarDeclKind_GlobalVar, name, loc, is_public, ref, b->ast_idx, initValue);
   ast_Decl* d = ast_VarDecl_asDecl(vd);
   ast_AST_addVarDecl(b->ast, d);
   ast_builder_Builder_addSymbol(b, name, d);
   if (b->is_interface) ast_Decl_setExternal(d);
   ast_builder_VarDeclAttrs* attrs = &b->var_attrs;
   if (attrs->has_attrs) {
      if (attrs->export) ast_Decl_setAttrExport(d);
      if (attrs->unused) ast_Decl_setAttrUnused(d);
      if (attrs->weak) {
         if (!ast_Decl_isPublic(d)) diagnostics_Diags_error(b->diags, attrs->weak_loc, "weak declarations must be public");
         ast_VarDecl_setAttrWeak(vd);
      }
      if (attrs->aligned.text_idx) ast_builder_Builder_storeAttr(b, d, attr_AttrKind_Aligned, &attrs->section);
      if (attrs->section.text_idx) ast_builder_Builder_storeAttr(b, d, attr_AttrKind_Section, &attrs->section);
      if (attrs->cname.text_idx) ast_builder_Builder_storeAttr(b, d, attr_AttrKind_CName, &attrs->section);
      memset(attrs, 0, sizeof(ast_builder_VarDeclAttrs));
   }
}

static ast_VarDecl* ast_builder_Builder_actOnFunctionParam(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* ref)
{
   is_public |= b->is_interface;
   return ast_VarDecl_create(b->context, ast_VarDeclKind_FunctionParam, name, loc, is_public, ref, b->ast_idx, NULL);
}

static ast_Stmt* ast_builder_Builder_actOnVarDeclStmt(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, const ast_TypeRefHolder* ref, ast_Expr* initValue, bool has_local)
{
   ast_VarDecl* d = ast_VarDecl_create(b->context, ast_VarDeclKind_LocalVar, name, loc, false, ref, b->ast_idx, initValue);
   ast_VarDecl_setLocal(d, has_local);
   return ((ast_Stmt*)(ast_DeclStmt_create(b->context, d)));
}

static void ast_builder_Builder_storeAttr(ast_builder_Builder* b, ast_Decl* d, attr_AttrKind kind, const attr_Value* value)
{
   ast_Decl_setHasAttr(d);
   ast_AST_storeAttr(b->ast, d, kind, value);
}

static void ast_builder_Builder_actOnFunctionAttr(ast_builder_Builder* b, ast_Decl* d, attr_AttrKind kind, src_loc_SrcLoc loc, const attr_Value* value)
{
   ast_FunctionDecl* fd = ((ast_FunctionDecl*)(d));
   switch (kind) {
   case attr_AttrKind_Export:
      ast_Decl_setAttrExport(d);
      break;
   case attr_AttrKind_Unused:
      ast_Decl_setAttrUnused(d);
      break;
   case attr_AttrKind_UnusedParams:
      ast_FunctionDecl_setAttrUnusedParams(fd);
      break;
   case attr_AttrKind_Section:
      ast_builder_Builder_storeAttr(b, d, kind, value);
      break;
   case attr_AttrKind_NoReturn:
      ast_FunctionDecl_setAttrNoReturn(fd);
      break;
   case attr_AttrKind_Inline:
      ast_FunctionDecl_setAttrInline(fd);
      break;
   case attr_AttrKind_Weak:
      if (!ast_Decl_isPublic(d)) diagnostics_Diags_error(b->diags, loc, "weak declarations must be public");
      ast_FunctionDecl_setAttrWeak(fd);
      break;
   case attr_AttrKind_CName:
      ast_builder_Builder_storeAttr(b, d, kind, value);
      break;
   default:
      diagnostics_Diags_error(b->diags, loc, "attribute %s not applicable to functions", attr_kind2name(kind));
      break;
   }
}

static void ast_builder_Builder_actOnStructAttr(ast_builder_Builder* b, ast_Decl* d, attr_AttrKind kind, src_loc_SrcLoc loc, const attr_Value* value)
{
   ast_StructTypeDecl* std = ((ast_StructTypeDecl*)(d));
   switch (kind) {
   case attr_AttrKind_Export:
      ast_Decl_setAttrExport(d);
      break;
   case attr_AttrKind_Packed:
      ast_StructTypeDecl_setPacked(std);
      break;
   case attr_AttrKind_Unused:
      ast_Decl_setAttrUnused(d);
      break;
   case attr_AttrKind_Aligned:
      ast_StructTypeDecl_setAttrAlignment(std, value->number);
      break;
   case attr_AttrKind_Opaque:
      ast_StructTypeDecl_setOpaque(std);
      break;
   case attr_AttrKind_CName:
      ast_builder_Builder_storeAttr(b, d, kind, value);
      break;
   case attr_AttrKind_NoTypeDef:
      if (b->is_interface) {
         ast_StructTypeDecl_setAttrNoTypeDef(std);
      } else {
         diagnostics_Diags_error(b->diags, loc, "attribute %s can only be used in interfaces", attr_kind2name(kind));
      }
      break;
   default:
      diagnostics_Diags_error(b->diags, loc, "attribute %s not applicable to structs", attr_kind2name(kind));
      break;
   }
}

static void ast_builder_Builder_actOnTypeAttr(ast_builder_Builder* b, ast_Decl* d, attr_AttrKind kind, src_loc_SrcLoc loc, const attr_Value* value)
{
   switch (kind) {
   case attr_AttrKind_Export:
      ast_Decl_setAttrExport(d);
      break;
   case attr_AttrKind_Unused:
      ast_Decl_setAttrUnused(d);
      break;
   case attr_AttrKind_CName:
      ast_builder_Builder_storeAttr(b, d, kind, value);
      break;
   default:
      diagnostics_Diags_error(b->diags, loc, "attribute %s not applicable to Enum/Alias types", attr_kind2name(kind));
      break;
   }
}

static void ast_builder_Builder_actOnVarAttr(ast_builder_Builder* b, attr_AttrKind kind, src_loc_SrcLoc loc, const attr_Value* value)
{
   ast_builder_VarDeclAttrs* attrs = &b->var_attrs;
   attrs->has_attrs = true;
   switch (kind) {
   case attr_AttrKind_Export:
      attrs->export = true;
      break;
   case attr_AttrKind_Unused:
      attrs->unused = true;
      break;
   case attr_AttrKind_Section:
      attrs->section = *value;
      break;
   case attr_AttrKind_Aligned:
      attrs->aligned = *value;
      break;
   case attr_AttrKind_Weak:
      attrs->weak = true;
      attrs->weak_loc = loc;
      break;
   case attr_AttrKind_CName:
      attrs->cname = *value;
      break;
   default:
      diagnostics_Diags_error(b->diags, loc, "attribute %s not applicable to variables", attr_kind2name(kind));
      break;
   }
}

static void ast_builder_Builder_actOnAttr(ast_builder_Builder* b, ast_Decl* d, uint32_t name, src_loc_SrcLoc loc, const attr_Value* value)
{
   attr_AttrKind kind = attr_find(name);
   if (kind != attr_AttrKind_Unknown) {
      attr_AttrReq req = attr_check(kind, value);
      switch (req) {
      case attr_AttrReq_NoArg:
         diagnostics_Diags_error(b->diags, loc, "attribute %s has no argument", attr_kind2name(kind));
         return;
      case attr_AttrReq_Arg:
         diagnostics_Diags_error(b->diags, loc, "attribute %s needs an argument", attr_kind2name(kind));
         return;
      case attr_AttrReq_Number:
         diagnostics_Diags_error(b->diags, value->loc, "attribute %s needs a number argument", attr_kind2name(kind));
         return;
      case attr_AttrReq_String:
         diagnostics_Diags_error(b->diags, value->loc, "attribute %s needs a string argument", attr_kind2name(kind));
         return;
      case attr_AttrReq_Power2:
         diagnostics_Diags_error(b->diags, value->loc, "requested alignment is not a power of 2");
         return;
      case attr_AttrReq_Ok:
         break;
      }
      ast_DeclKind dk = d ? ast_Decl_getKind(d) : ast_DeclKind_Var;
      switch (dk) {
      case ast_DeclKind_Function:
         ast_builder_Builder_actOnFunctionAttr(b, d, kind, loc, value);
         break;
      case ast_DeclKind_StructType:
         ast_builder_Builder_actOnStructAttr(b, d, kind, loc, value);
         break;
      case ast_DeclKind_EnumType:
         ast_builder_Builder_actOnTypeAttr(b, d, kind, loc, value);
         break;
      case ast_DeclKind_FunctionType: {
         ast_builder_Builder_actOnTypeAttr(b, d, kind, loc, value);
         ast_FunctionTypeDecl* ftd = ((ast_FunctionTypeDecl*)(d));
         ast_builder_Builder_actOnTypeAttr(b, ((ast_Decl*)(ast_FunctionTypeDecl_getDecl(ftd))), kind, loc, value);
         break;
      }
      case ast_DeclKind_AliasType:
         ast_builder_Builder_actOnTypeAttr(b, d, kind, loc, value);
         break;
      case ast_DeclKind_Var:
         ast_builder_Builder_actOnVarAttr(b, kind, loc, value);
         break;
      default:
         assert(0);
         return;
      }
   } else {
      bool allow_unknown = false;
      if (allow_unknown) {
         if (d) {
            ast_builder_Builder_storeAttr(b, d, kind, value);
            ast_Decl_setHasAttr(d);
         } else {
            diagnostics_Diags_error(b->diags, loc, "TODO unknown attributes for VarDecl");
         }
      } else {
         diagnostics_Diags_error(b->diags, loc, "unknown attributes are not allowed");
      }
   }
}

static ast_QualType ast_builder_Builder_actOnBuiltinType(ast_builder_Builder* _arg0, ast_BuiltinKind kind)
{
   switch (kind) {
   case ast_BuiltinKind_Char:
      return ast_g_char;
   case ast_BuiltinKind_Int8:
      return ast_g_i8;
   case ast_BuiltinKind_Int16:
      return ast_g_i16;
   case ast_BuiltinKind_Int32:
      return ast_g_i32;
   case ast_BuiltinKind_Int64:
      return ast_g_i64;
   case ast_BuiltinKind_UInt8:
      return ast_g_u8;
   case ast_BuiltinKind_UInt16:
      return ast_g_u16;
   case ast_BuiltinKind_UInt32:
      return ast_g_u32;
   case ast_BuiltinKind_UInt64:
      return ast_g_u64;
   case ast_BuiltinKind_Float32:
      return ast_g_f32;
   case ast_BuiltinKind_Float64:
      return ast_g_f64;
   case ast_BuiltinKind_ISize:
      return ast_g_isize;
   case ast_BuiltinKind_USize:
      return ast_g_usize;
   case ast_BuiltinKind_Bool:
      return ast_g_bool;
   case ast_BuiltinKind_Void:
      return ast_g_void;
   }
   assert(0);
   return ast_QualType_Invalid;
}

static ast_QualType ast_builder_Builder_actOnPointerType(ast_builder_Builder* _arg0, ast_QualType inner)
{
   ast_QualType ptr = ast_QualType_init(ast_getPointerType(inner));
   ast_QualType canon = ast_QualType_getCanonicalType(&inner);
   if (ast_QualType_getTypeOrNil(&inner) == ast_QualType_getTypeOrNil(&canon)) {
      canon = ptr;
   } else {
      canon = ast_QualType_init(ast_getPointerType(canon));
      ast_QualType_setCanonicalType(&canon, canon);
   }
   ast_QualType_setCanonicalType(&ptr, canon);
   return ptr;
}

static ast_QualType ast_builder_Builder_actOnArrayType(ast_builder_Builder* b, ast_QualType elem, bool has_size, uint32_t size)
{
   ast_ArrayType* t = ast_ArrayType_create(b->context, elem, has_size, size);
   ast_QualType a = ast_QualType_init(((ast_Type*)(t)));
   ast_QualType canon = ast_QualType_getCanonicalType(&elem);
   if (ast_QualType_getTypeOrNil(&elem) == ast_QualType_getTypeOrNil(&canon)) {
      canon = a;
   } else {
      ast_ArrayType* t2 = ast_ArrayType_create(b->context, canon, has_size, size);
      canon = ast_QualType_init(((ast_Type*)(t2)));
   }
   ast_QualType_setCanonicalType(&a, canon);
   return a;
}

static ast_FunctionDecl* ast_builder_Builder_actOnFunctionDecl(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* rtype, const ast_Ref* prefix, ast_VarDecl** params, uint32_t num_params, bool is_variadic)
{
   is_public |= b->is_interface;
   ast_FunctionDecl* f = ast_FunctionDecl_create(b->context, name, loc, is_public, b->ast_idx, rtype, prefix, params, num_params, is_variadic);
   ast_AST_addFunc(b->ast, f);
   if (!prefix) ast_builder_Builder_addSymbol(b, name, ast_FunctionDecl_asDecl(f));
   if (b->is_interface) ast_Decl_setExternal(ast_FunctionDecl_asDecl(f));
   return f;
}

static ast_FunctionDecl* ast_builder_Builder_actOnTemplateFunctionDecl(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, const ast_TypeRefHolder* rtype, uint32_t template_name, src_loc_SrcLoc template_loc, ast_VarDecl** params, uint32_t num_params, bool is_variadic)
{
   if (b->is_interface) diagnostics_Diags_error(b->diags, loc, "template functions are not allow in interfaces");
   is_public |= b->is_interface;
   ast_FunctionDecl* f = ast_FunctionDecl_createTemplate(b->context, name, loc, is_public, b->ast_idx, rtype, template_name, template_loc, params, num_params, is_variadic);
   ast_AST_addFunc(b->ast, f);
   ast_builder_Builder_addSymbol(b, name, ast_FunctionDecl_asDecl(f));
   if (b->is_interface) ast_Decl_setExternal(ast_FunctionDecl_asDecl(f));
   return f;
}

static void ast_builder_Builder_actOnFunctionBody(ast_builder_Builder* _arg0, ast_FunctionDecl* f, ast_CompoundStmt* body)
{
   ast_FunctionDecl_setBody(f, body);
}

static ast_EnumConstantDecl* ast_builder_Builder_actOnEnumConstant(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, ast_Expr* init_expr)
{
   return ast_EnumConstantDecl_create(b->context, name, loc, is_public, b->ast_idx, init_expr);
}

static ast_Decl* ast_builder_Builder_actOnEnumType(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc, bool is_public, bool is_incr, ast_QualType implType, ast_EnumConstantDecl** constants, uint32_t num_constants)
{
   ast_EnumTypeDecl* d = ast_EnumTypeDecl_create(b->context, name, loc, is_public, b->ast_idx, implType, is_incr, constants, num_constants);
   ast_AST_addTypeDecl(b->ast, ast_EnumTypeDecl_asDecl(d));
   ast_builder_Builder_addSymbol(b, name, ast_EnumTypeDecl_asDecl(d));
   return ((ast_Decl*)(d));
}

static ast_CompoundStmt* ast_builder_Builder_actOnCompoundStmt(ast_builder_Builder* b, src_loc_SrcLoc endLoc, ast_Stmt** stmts, uint32_t count)
{
   return ast_CompoundStmt_create(b->context, endLoc, stmts, count);
}

static ast_Stmt* ast_builder_Builder_actOnReturnStmt(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* ret)
{
   return ((ast_Stmt*)(ast_ReturnStmt_create(b->context, loc, ret)));
}

static ast_Stmt* ast_builder_Builder_actOnIfStmt(ast_builder_Builder* b, ast_Stmt* cond, ast_Stmt* then, ast_Stmt* else_stmt)
{
   return ((ast_Stmt*)(ast_IfStmt_create(b->context, cond, then, else_stmt)));
}

static ast_Stmt* ast_builder_Builder_actOnDoStmt(ast_builder_Builder* b, ast_Stmt* cond, ast_Stmt* then)
{
   return ((ast_Stmt*)(ast_DoStmt_create(b->context, cond, then)));
}

static ast_Stmt* ast_builder_Builder_actOnWhileStmt(ast_builder_Builder* b, ast_Stmt* cond, ast_Stmt* then)
{
   return ((ast_Stmt*)(ast_WhileStmt_create(b->context, cond, then)));
}

static ast_Stmt* ast_builder_Builder_actOnForStmt(ast_builder_Builder* b, ast_Stmt* init_, ast_Expr* cond, ast_Expr* incr, ast_Stmt* body)
{
   return ((ast_Stmt*)(ast_ForStmt_create(b->context, init_, cond, incr, body)));
}

static ast_Stmt* ast_builder_Builder_actOnSwitchStmt(ast_builder_Builder* b, ast_Stmt* cond, ast_Stmt** cases, uint32_t num_cases, bool is_sswitch)
{
   return ((ast_Stmt*)(ast_SwitchStmt_create(b->context, cond, cases, num_cases, is_sswitch)));
}

static ast_Stmt* ast_builder_Builder_actOnDefaultStmt(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Stmt** stmts, uint32_t num_stmts)
{
   return ((ast_Stmt*)(ast_DefaultStmt_create(b->context, loc, stmts, num_stmts)));
}

static ast_Stmt* ast_builder_Builder_actOnCaseStmt(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* cond, ast_Stmt** stmts, uint32_t num_stmts)
{
   return ((ast_Stmt*)(ast_CaseStmt_create(b->context, loc, cond, stmts, num_stmts)));
}

static ast_Stmt* ast_builder_Builder_actOnAssertStmt(ast_builder_Builder* b, ast_Expr* inner)
{
   return ((ast_Stmt*)(ast_AssertStmt_create(b->context, inner)));
}

static ast_Stmt* ast_builder_Builder_actOnBreakStmt(ast_builder_Builder* b, src_loc_SrcLoc loc)
{
   return ((ast_Stmt*)(ast_BreakStmt_create(b->context, loc)));
}

static ast_Stmt* ast_builder_Builder_actOnContinueStmt(ast_builder_Builder* b, src_loc_SrcLoc loc)
{
   return ((ast_Stmt*)(ast_ContinueStmt_create(b->context, loc)));
}

static ast_Stmt* ast_builder_Builder_actOnFallthroughStmt(ast_builder_Builder* b, src_loc_SrcLoc loc)
{
   return ((ast_Stmt*)(ast_FallthroughStmt_create(b->context, loc)));
}

static ast_Stmt* ast_builder_Builder_actOnLabelStmt(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc)
{
   return ((ast_Stmt*)(ast_LabelStmt_create(b->context, name, loc)));
}

static ast_Stmt* ast_builder_Builder_actOnGotoStmt(ast_builder_Builder* b, uint32_t name, src_loc_SrcLoc loc)
{
   return ((ast_Stmt*)(ast_GotoStmt_create(b->context, name, loc)));
}

static ast_IdentifierExpr* ast_builder_Builder_actOnIdentifier(ast_builder_Builder* b, src_loc_SrcLoc loc, uint32_t name)
{
   return ast_IdentifierExpr_create(b->context, loc, name);
}

static ast_Expr* ast_builder_Builder_actOnIntegerLiteral(ast_builder_Builder* b, src_loc_SrcLoc loc, uint8_t radix, uint64_t value)
{
   return ((ast_Expr*)(ast_IntegerLiteral_create(b->context, loc, radix, value)));
}

static ast_Expr* ast_builder_Builder_actOnCharLiteral(ast_builder_Builder* b, src_loc_SrcLoc loc, uint8_t value)
{
   return ((ast_Expr*)(ast_CharLiteral_create(b->context, loc, value)));
}

static ast_Expr* ast_builder_Builder_actOnStringLiteral(ast_builder_Builder* b, src_loc_SrcLoc loc, uint32_t value, uint32_t len)
{
   return ((ast_Expr*)(ast_StringLiteral_create(b->context, loc, value, len)));
}

static ast_Expr* ast_builder_Builder_actOnNilExpr(ast_builder_Builder* b, src_loc_SrcLoc loc)
{
   return ((ast_Expr*)(ast_NilExpr_create(b->context, loc)));
}

static ast_Expr* ast_builder_Builder_actOnParenExpr(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* inner)
{
   return ((ast_Expr*)(ast_ParenExpr_create(b->context, loc, inner)));
}

static ast_Expr* ast_builder_Builder_actOnUnaryOperator(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_UnaryOpcode opcode, ast_Expr* inner)
{
   return ((ast_Expr*)(ast_UnaryOperator_create(b->context, loc, opcode, inner)));
}

static ast_Expr* ast_builder_Builder_actOnPostFixUnaryOperator(ast_builder_Builder* b, src_loc_SrcLoc loc, token_Kind kind, ast_Expr* inner)
{
   ast_UnaryOpcode opcode = (kind == token_Kind_PlusPlus) ? ast_UnaryOpcode_PostInc : ast_UnaryOpcode_PostDec;
   return ((ast_Expr*)(ast_UnaryOperator_create(b->context, loc, opcode, inner)));
}

static ast_Expr* ast_builder_Builder_actOnBinaryOperator(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_BinaryOpcode opcode, ast_Expr* lhs, ast_Expr* rhs)
{
   return ((ast_Expr*)(ast_BinaryOperator_create(b->context, loc, opcode, lhs, rhs)));
}

static ast_Expr* ast_builder_Builder_actOnConditionalOperator(ast_builder_Builder* b, src_loc_SrcLoc questionLoc, src_loc_SrcLoc colonLoc, ast_Expr* cond, ast_Expr* lhs, ast_Expr* rhs)
{
   return ((ast_Expr*)(ast_ConditionalOperator_create(b->context, questionLoc, colonLoc, cond, lhs, rhs)));
}

static ast_Expr* ast_builder_Builder_actOnBooleanConstant(ast_builder_Builder* b, src_loc_SrcLoc loc, bool value)
{
   return ((ast_Expr*)(ast_BooleanLiteral_create(b->context, loc, value)));
}

static ast_Expr* ast_builder_Builder_actOnBuiltinExpr(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* inner, ast_BuiltinExprKind kind)
{
   return ((ast_Expr*)(ast_BuiltinExpr_create(b->context, loc, inner, kind)));
}

static ast_Expr* ast_builder_Builder_actOnOffsetOfExpr(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* structExpr, ast_Expr* member)
{
   return ((ast_Expr*)(ast_BuiltinExpr_createOffsetOf(b->context, loc, structExpr, member)));
}

static ast_Expr* ast_builder_Builder_actOnToContainerExpr(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* structExpr, ast_Expr* member, ast_Expr* pointer)
{
   return ((ast_Expr*)(ast_BuiltinExpr_createToContainer(b->context, loc, structExpr, member, pointer)));
}

static ast_Expr* ast_builder_Builder_actOnTypeExpr(ast_builder_Builder* b, src_loc_SrcLoc loc, const ast_TypeRefHolder* ref)
{
   return ((ast_Expr*)(ast_TypeExpr_create(b->context, loc, ref)));
}

static ast_Expr* ast_builder_Builder_actOnBitOffsetExpr(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* lhs, ast_Expr* rhs)
{
   return ((ast_Expr*)(ast_BitOffsetExpr_create(b->context, loc, lhs, rhs)));
}

static ast_Expr* ast_builder_Builder_actOnArraySubscriptExpr(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* base, ast_Expr* idx)
{
   return ((ast_Expr*)(ast_ArraySubscriptExpr_create(b->context, loc, base, idx)));
}

static ast_Expr* ast_builder_Builder_actOnCallExpr(ast_builder_Builder* b, src_loc_SrcLoc endLoc, ast_Expr* fn, ast_Expr** args, uint32_t num_args)
{
   return ((ast_Expr*)(ast_CallExpr_create(b->context, endLoc, fn, args, num_args)));
}

static ast_Expr* ast_builder_Builder_actOnTemplateCallExpr(ast_builder_Builder* b, src_loc_SrcLoc endLoc, ast_Expr* fn, ast_Expr** args, uint32_t num_args, const ast_TypeRefHolder* ref)
{
   return ((ast_Expr*)(ast_CallExpr_createTemplate(b->context, endLoc, fn, args, num_args, ref)));
}

static ast_Expr* ast_builder_Builder_actOnExplicitCast(ast_builder_Builder* b, src_loc_SrcLoc loc, const ast_TypeRefHolder* ref, ast_Expr* inner)
{
   return ((ast_Expr*)(ast_ExplicitCastExpr_create(b->context, loc, ref, inner)));
}

static ast_Expr* ast_builder_Builder_actOnMemberExpr(ast_builder_Builder* b, ast_Expr* base, const ast_Ref* refs, uint32_t refcount)
{
   return ((ast_Expr*)(ast_MemberExpr_create(b->context, base, refs, refcount)));
}

static ast_Expr* ast_builder_Builder_actOnInitList(ast_builder_Builder* b, src_loc_SrcLoc left, src_loc_SrcLoc right, ast_Expr** values, uint32_t num_values)
{
   return ((ast_Expr*)(ast_InitListExpr_create(b->context, left, right, values, num_values)));
}

static ast_Expr* ast_builder_Builder_actOnFieldDesignatedInit(ast_builder_Builder* b, uint32_t field, src_loc_SrcLoc loc, ast_Expr* initValue)
{
   return ((ast_Expr*)(ast_FieldDesignatedInitExpr_create(b->context, field, loc, initValue)));
}

static ast_Expr* ast_builder_Builder_actOnArrayDesignatedInit(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* designator, ast_Expr* initValue)
{
   return ((ast_Expr*)(ast_ArrayDesignatedInitExpr_create(b->context, loc, designator, initValue)));
}

static void ast_builder_Builder_actOnStaticAssert(ast_builder_Builder* b, src_loc_SrcLoc loc, ast_Expr* lhs, ast_Expr* rhs)
{
   ast_StaticAssertDecl* d = ast_StaticAssertDecl_create(b->context, b->ast_idx, loc, lhs, rhs);
   ast_AST_addStaticAssert(b->ast, d);
}

static void ast_builder_Builder_insertImplicitCast(ast_builder_Builder* b, ast_ImplicitCastKind kind, ast_Expr** e_ptr, ast_QualType qt)
{
   ast_Expr* inner = *e_ptr;
   ast_Expr* ic = ((ast_Expr*)(ast_ImplicitCastExpr_create(b->context, ast_Expr_getLoc(inner), kind, inner)));
   ast_Expr_setType(ic, qt);
   *e_ptr = ic;
}

static void ast_builder_Builder_addSymbol(ast_builder_Builder* b, uint32_t name_idx, ast_Decl* d)
{
   ast_Decl* old = ast_Module_findSymbol(b->mod, name_idx);
   if (old) {
      diagnostics_Diags_error(b->diags, ast_Decl_getLoc(d), "redefinition of '%s'", ast_idx2name(name_idx));
      diagnostics_Diags_note(b->diags, ast_Decl_getLoc(old), "previous definition is here");
   } else {
      ast_Module_addSymbol(b->mod, name_idx, d);
   }
}


// --- module conversion_checker ---

typedef struct conversion_checker_Checker_ conversion_checker_Checker;

struct conversion_checker_Checker_ {
   diagnostics_Diags* diags;
   ast_builder_Builder* builder;
   src_loc_SrcLoc loc;
   ast_QualType lhs;
   ast_QualType rhs;
   ast_Expr** expr_ptr;
};

static void conversion_checker_Checker_init(conversion_checker_Checker* c, diagnostics_Diags* diags, ast_builder_Builder* builder);
static bool conversion_checker_Checker_check(conversion_checker_Checker* c, ast_QualType lhs, ast_QualType rhs, ast_Expr** e_ptr, src_loc_SrcLoc loc);
static bool conversion_checker_Checker_checkTypes(conversion_checker_Checker* c, const ast_Type* lcanon, const ast_Type* rcanon);
static bool conversion_checker_Checker_checkBuiltins(conversion_checker_Checker* c, const ast_Type* lcanon, const ast_Type* rcanon);
static bool conversion_checker_Checker_checkBuiltin2Pointer(conversion_checker_Checker* c, const ast_Type* lcanon, const ast_Type* rcanon);
static bool conversion_checker_Checker_checkPointer2Builtin(conversion_checker_Checker* c, const ast_Type* lcanon, const ast_Type* rcanon);
static bool conversion_checker_Checker_checkPointers(conversion_checker_Checker* c, const ast_Type* lcanon, const ast_Type* rcanon);
static bool conversion_checker_Checker_checkEnum2Int(conversion_checker_Checker* c, const ast_Type* lcanon, const ast_Type* rcanon);
static bool conversion_checker_checkFunc2Func(const ast_Type* lcanon, const ast_Type* rcanon);

static const uint8_t conversion_checker_Conversions[8][8] = {
   {
   2,
   3,
   1,
   1,
   1,
   1,
   0,
   0
},
   {
   4,
   5,
   1,
   1,
   1,
   6,
   0,
   0
},
   {
   1,
   7,
   8,
   1,
   1,
   1,
   0,
   0
},
   {
   1,
   1,
   1,
   9,
   1,
   1,
   0,
   0
},
   {
   10,
   1,
   1,
   1,
   1,
   1,
   0,
   0
},
   {
   1,
   11,
   1,
   1,
   1,
   12,
   0,
   0
},
   {
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0
},
   {
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0
}
};

static const uint8_t conversion_checker_BuiltinConversions[15][15] = {
   {
   0,
   1,
   1,
   1,
   1,
   3,
   3,
   3,
   3,
   1,
   1,
   1,
   3,
   1,
   2
},
   {
   1,
   0,
   1,
   1,
   1,
   3,
   3,
   3,
   3,
   1,
   1,
   1,
   3,
   1,
   2
},
   {
   4,
   4,
   0,
   1,
   1,
   3,
   3,
   3,
   3,
   1,
   1,
   1,
   3,
   1,
   2
},
   {
   4,
   4,
   4,
   0,
   1,
   3,
   3,
   3,
   3,
   1,
   1,
   1,
   3,
   1,
   2
},
   {
   4,
   4,
   4,
   4,
   0,
   3,
   3,
   3,
   3,
   1,
   1,
   1,
   3,
   1,
   2
},
   {
   3,
   3,
   3,
   3,
   3,
   0,
   1,
   1,
   1,
   1,
   1,
   3,
   1,
   1,
   2
},
   {
   3,
   3,
   3,
   3,
   3,
   1,
   0,
   1,
   1,
   1,
   1,
   3,
   1,
   1,
   2
},
   {
   3,
   3,
   3,
   3,
   3,
   1,
   1,
   0,
   1,
   1,
   1,
   3,
   1,
   1,
   2
},
   {
   3,
   3,
   3,
   3,
   3,
   1,
   1,
   1,
   0,
   1,
   1,
   3,
   1,
   1,
   2
},
   {
   5,
   5,
   5,
   5,
   5,
   5,
   5,
   5,
   5,
   0,
   1,
   5,
   5,
   2,
   2
},
   {
   5,
   5,
   5,
   5,
   5,
   5,
   5,
   5,
   5,
   6,
   0,
   5,
   5,
   2,
   2
},
   {
   4,
   4,
   4,
   4,
   1,
   3,
   3,
   3,
   3,
   1,
   1,
   0,
   3,
   1,
   2
},
   {
   3,
   3,
   3,
   3,
   3,
   1,
   1,
   1,
   1,
   1,
   1,
   3,
   0,
   1,
   2
},
   {
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   1,
   0,
   2
},
   {
   2,
   2,
   2,
   2,
   2,
   2,
   2,
   2,
   2,
   2,
   2,
   2,
   2,
   2,
   0
}
};

static void conversion_checker_Checker_init(conversion_checker_Checker* c, diagnostics_Diags* diags, ast_builder_Builder* builder)
{
   c->diags = diags;
   c->builder = builder;
}

static bool conversion_checker_Checker_check(conversion_checker_Checker* c, ast_QualType lhs, ast_QualType rhs, ast_Expr** e_ptr, src_loc_SrcLoc loc)
{
   assert(lhs.ptr);
   assert(rhs.ptr);
   ast_QualType t1 = ast_QualType_getCanonicalType(&lhs);
   ast_QualType t2 = ast_QualType_getCanonicalType(&rhs);
   const ast_Type* lcanon = ast_QualType_getTypeOrNil(&t1);
   const ast_Type* rcanon = ast_QualType_getTypeOrNil(&t2);
   if (lcanon == rcanon) {
      uint32_t lquals = ast_QualType_getQuals(&lhs);
      uint32_t rquals = ast_QualType_getQuals(&rhs);
      if ((ast_Type_getKind(lcanon) == ast_TypeKind_Pointer && ((((~lquals & 0x3)) & rquals)))) {
         diagnostics_Diags_error(c->diags, c->loc, "conversion discards const qualifier");
         return false;
      }
      return true;
   }
   c->lhs = lhs;
   c->rhs = rhs;
   c->expr_ptr = e_ptr;
   c->loc = loc;
   return conversion_checker_Checker_checkTypes(c, lcanon, rcanon);
}

static bool conversion_checker_Checker_checkTypes(conversion_checker_Checker* c, const ast_Type* lcanon, const ast_Type* rcanon)
{
   uint8_t res = conversion_checker_Conversions[ast_Type_getKind(rcanon)][ast_Type_getKind(lcanon)];
   switch (res) {
   case 0:
      diagnostics_Diags_error(c->diags, c->loc, "SHOULD NOT HAPPEN (%u - %u)\n", ast_Type_getKind(lcanon), ast_Type_getKind(rcanon));
      ast_QualType_dump_full(&c->lhs);
      ast_QualType_dump_full(&c->rhs);
      assert(0);
      return false;
   case 1:
      diagnostics_Diags_error(c->diags, c->loc, "invalid type conversion from '%s' to '%s'", ast_QualType_diagName(&c->rhs), ast_QualType_diagName(&c->lhs));
      return false;
   case 2:
      return conversion_checker_Checker_checkBuiltins(c, lcanon, rcanon);
   case 3:
      return conversion_checker_Checker_checkBuiltin2Pointer(c, lcanon, rcanon);
   case 4:
      return conversion_checker_Checker_checkPointer2Builtin(c, lcanon, rcanon);
   case 5:
      return conversion_checker_Checker_checkPointers(c, lcanon, rcanon);
   case 7:
      diagnostics_Diags_note(c->diags, c->loc, "SHOULD NOT HAPPEN (Array -> Ptr)");
      assert(0);
      return false;
   case 8:
      diagnostics_Diags_error(c->diags, c->loc, "invalid type conversion from '%s' to '%s'", ast_QualType_diagName(&c->rhs), ast_QualType_diagName(&c->lhs));
      return false;
   case 9:
      diagnostics_Diags_error(c->diags, c->loc, "conversion between struct of different types ('%s' to '%s')", ast_QualType_diagName(&c->rhs), ast_QualType_diagName(&c->lhs));
      return false;
   case 10:
      return conversion_checker_Checker_checkEnum2Int(c, lcanon, rcanon);
   case 12: {
      bool ok = conversion_checker_checkFunc2Func(lcanon, rcanon);
      if (!ok) diagnostics_Diags_error(c->diags, c->loc, "invalid function conversion from '%s' to '%s'", ast_QualType_diagName(&c->rhs), ast_QualType_diagName(&c->lhs));
      return ok;
   }
   default:
      diagnostics_Diags_note(c->diags, c->loc, "TODO CONVERSION  %u)", res);
      return false;
   }
   return true;
}

static bool conversion_checker_Checker_checkBuiltins(conversion_checker_Checker* c, const ast_Type* lcanon, const ast_Type* rcanon)
{
   const ast_BuiltinType* lbuiltin = ((ast_BuiltinType*)(lcanon));
   const ast_BuiltinType* rbuiltin = ((ast_BuiltinType*)(rcanon));
   if (ast_Expr_isCtv((*c->expr_ptr))) {
      return true;
   }
   uint8_t res = conversion_checker_BuiltinConversions[ast_BuiltinType_getKind(rbuiltin)][ast_BuiltinType_getKind(lbuiltin)];
   switch (res) {
   case 0:
      printf("BUILTIN SHOULD NOT HAPPEN (%u - %u)\n", ast_Type_getKind(lcanon), ast_Type_getKind(rcanon));
      assert(0);
      return false;
   case 1:
      break;
   case 2:
      diagnostics_Diags_error(c->diags, c->loc, "invalid builtin conversion from %s to %s", ast_QualType_diagName(&c->rhs), ast_QualType_diagName(&c->lhs));
      return false;
   case 3:
      break;
   case 4:
      diagnostics_Diags_note(c->diags, c->loc, "loss of integer precision");
      break;
   case 5:
      diagnostics_Diags_note(c->diags, c->loc, "float to integer conversion");
      break;
   case 6:
      diagnostics_Diags_note(c->diags, c->loc, "loss of floating-point precision");
      break;
   }
   return true;
}

static bool conversion_checker_Checker_checkBuiltin2Pointer(conversion_checker_Checker* c, const ast_Type* lcanon, const ast_Type* rcanon)
{
   const ast_PointerType* ptr = ((ast_PointerType*)(lcanon));
   const ast_BuiltinType* bi = ((ast_BuiltinType*)(rcanon));
   ast_QualType inner = ast_PointerType_getInner(ptr);
   bool ok = ast_QualType_isVoidType(&inner);
   ast_BuiltinKind kind = ast_BuiltinType_getKind(bi);
   ok &= ((kind == ast_BuiltinKind_USize || kind == ast_BuiltinKind_UInt64));
   if (!ok) {
      diagnostics_Diags_error(c->diags, c->loc, "incompatible integer to pointer conversion %s to %s", ast_QualType_diagName(&c->rhs), ast_QualType_diagName(&c->lhs));
      return false;
   }
   return true;
}

static bool conversion_checker_Checker_checkPointer2Builtin(conversion_checker_Checker* c, const ast_Type* lcanon, const ast_Type* rcanon)
{
   const ast_BuiltinType* bi = ((ast_BuiltinType*)(lcanon));
   ast_BuiltinKind kind = ast_BuiltinType_getKind(bi);
   if (kind == ast_BuiltinKind_Bool) {
      ast_builder_Builder_insertImplicitCast(c->builder, ast_ImplicitCastKind_PointerToBoolean, c->expr_ptr, ast_g_bool);
      return true;
   }
   const ast_PointerType* ptr = ((ast_PointerType*)(rcanon));
   ast_QualType inner = ast_PointerType_getInner(ptr);
   bool ok = ast_QualType_isVoidType(&inner);
   ok &= ((kind == ast_BuiltinKind_USize || kind == ast_BuiltinKind_UInt64));
   if (!ok) {
      diagnostics_Diags_error(c->diags, c->loc, "incompatible pointer to integer conversion %s to %s", ast_QualType_diagName(&c->rhs), ast_QualType_diagName(&c->lhs));
      return false;
   }
   ast_builder_Builder_insertImplicitCast(c->builder, ast_ImplicitCastKind_PointerToInteger, c->expr_ptr, ast_g_usize);
   return true;
}

static bool conversion_checker_Checker_checkPointers(conversion_checker_Checker* c, const ast_Type* lcanon, const ast_Type* rcanon)
{
   const ast_PointerType* ltype = ((ast_PointerType*)(lcanon));
   const ast_PointerType* rtype = ((ast_PointerType*)(rcanon));
   ast_QualType linner = ast_PointerType_getInner(ltype);
   ast_QualType rinner = ast_PointerType_getInner(rtype);
   const ast_Type* in1 = ast_QualType_getTypeOrNil(&linner);
   const ast_Type* in2 = ast_QualType_getTypeOrNil(&rinner);
   if (in1 == in2) {
      uint32_t rquals = ast_QualType_getQuals(&rinner);
      if (rquals == 0) return true;

      uint32_t lquals = ast_QualType_getQuals(&linner);
      if (((((~lquals) & rquals)) & 0x3)) {
         diagnostics_Diags_error(c->diags, c->loc, "pointer conversion discards const qualifier");
         return false;
      }
      return true;
   }
   if (ast_Type_isVoidType(in1)) {
      return true;
   }
   if (ast_Type_isVoidType(in2)) {
      return true;
   }
   diagnostics_Diags_error(c->diags, c->loc, "invalid pointer conversion from %s to %s", ast_QualType_diagName(&c->rhs), ast_QualType_diagName(&c->lhs));
   return false;
}

static bool conversion_checker_Checker_checkEnum2Int(conversion_checker_Checker* c, const ast_Type* lcanon, const ast_Type* rcanon)
{
   const ast_BuiltinType* bi = ((ast_BuiltinType*)(lcanon));
   uint32_t width = ast_BuiltinType_getWidth(bi);
   if (width == 64) return true;

   if (ast_Expr_isCtv((*c->expr_ptr))) {
      const ctv_analyser_Limit* limit = ctv_analyser_getLimit(width);
      ctv_analyser_Value value = ctv_analyser_get_value((*c->expr_ptr));
      if (value.uvalue > limit->max_val) {
         diagnostics_Diags_error(c->diags, c->loc, "constant value %lu out-of-bounds for type %s, range [%s, %s]", value.uvalue, ast_QualType_diagName(&c->lhs), limit->min_str, limit->max_str);
         return false;
      }
   } else {
      const ast_EnumType* et = ((ast_EnumType*)(rcanon));
      const ast_EnumTypeDecl* etd = ast_EnumType_getDecl(et);
      ast_QualType impl = ast_EnumTypeDecl_getImplType(etd);
      return conversion_checker_Checker_check(c, c->lhs, impl, c->expr_ptr, c->loc);
   }
   return true;
}

static bool conversion_checker_checkFunc2Func(const ast_Type* lcanon, const ast_Type* rcanon)
{
   ast_FunctionType* ftl = ((ast_FunctionType*)(lcanon));
   ast_FunctionDecl* fdl = ast_FunctionType_getDecl(ftl);
   ast_FunctionType* ftr = ((ast_FunctionType*)(rcanon));
   ast_FunctionDecl* fdr = ast_FunctionType_getDecl(ftr);
   ast_QualType ql = ast_FunctionDecl_getRType(fdl);
   ast_QualType qr = ast_FunctionDecl_getRType(fdr);
   if (ql.ptr != qr.ptr) return false;

   uint32_t num1 = ast_FunctionDecl_getNumParams(fdl);
   uint32_t num2 = ast_FunctionDecl_getNumParams(fdr);
   if (num1 != num2) return false;

   ast_Decl** args1 = ((ast_Decl**)(ast_FunctionDecl_getParams(fdl)));
   ast_Decl** args2 = ((ast_Decl**)(ast_FunctionDecl_getParams(fdr)));
   for (uint32_t i = 0; i < num1; i++) {
      ast_Decl* a1 = args1[i];
      ast_Decl* a2 = args2[i];
      ql = ast_Decl_getType(a1);
      qr = ast_Decl_getType(a2);
      if (ql.ptr != qr.ptr) return false;

   }
   if (ast_FunctionDecl_isVariadic(fdl) != ast_FunctionDecl_isVariadic(fdr)) return false;

   return true;
}


// --- module module_sorter ---

typedef struct module_sorter_ModuleSorter_ module_sorter_ModuleSorter;

struct module_sorter_ModuleSorter_ {
   component_Component* comp;
   uint32_t num_mods;
   ast_Module** modules;
   uint8_t* array;
   uint32_t cur_mod_idx;
};

static void module_sorter_sort(component_Component* c, diagnostics_Diags* diags);
static void module_sorter_ModuleSorter_print(const module_sorter_ModuleSorter* s);
static void module_sorter_ModuleSorter_handleModule(void* arg, ast_Module* mod);
static void module_sorter_ModuleSorter_handleImport(void* arg, ast_ImportDecl* d);
static uint32_t module_sorter_ModuleSorter_mod2idx(const module_sorter_ModuleSorter* s, const ast_Module* mod);

static void module_sorter_sort(component_Component* c, diagnostics_Diags* diags)
{
   const uint32_t count = component_Component_getNumModules(c);
   if (count <= 1) return;

   module_sorter_ModuleSorter s;
   s.num_mods = count;
   s.cur_mod_idx = 0;
   s.comp = c;
   ast_Module** orig = component_Component_getModules(c);
   s.modules = malloc(count * sizeof(ast_Module*));
   memcpy(((void*)(s.modules)), ((void*)(orig)), count * sizeof(ast_Module*));
   s.array = calloc(1, count * (count + 2));
   component_Component_visitModules(c, module_sorter_ModuleSorter_handleModule, &s);
   uint8_t* sorted = &s.array[count * (count + 1)];
   uint8_t* ringbuf = &s.array[count * count];
   uint32_t head = 0;
   uint32_t size = count;
   for (uint8_t i = 0; i < count; i++) ringbuf[i] = i;
   uint32_t iterations = 0;
   while (size) {
      if (iterations > size) {
         diagnostics_Diags_error(diags, 0, "circular dependency between modules");
         module_sorter_ModuleSorter_print(&s);
         goto out;
      }
      uint8_t idx = ringbuf[head];
      head = (head + 1) % count;
      uint32_t offset = idx * count;
      bool has_deps = false;
      for (uint32_t j = 0; j < count; j++) {
         if (s.array[offset + j] != 0) {
            has_deps = true;
            break;
         }
      }
      if (has_deps) {
         ringbuf[(head + size - 1) % count] = idx;
         iterations++;
      } else {
         sorted[count - size] = ((uint8_t)(idx));
         iterations = 0;
         size--;
         for (uint32_t x = 0; x < count; x++) s.array[x * count + idx] = 0;
      }
   }
   for (uint32_t i = 0; i < count; i++) orig[i] = s.modules[sorted[i]];
   out:
   free(((void*)(s.modules)));
   free(s.array);
}

static void module_sorter_ModuleSorter_print(const module_sorter_ModuleSorter* s)
{
   printf("                     |");
   for (uint32_t y = 0; y < s->num_mods; y++) {
      printf(" %2u", y);
   }
   printf("\n");
   for (uint32_t y = 0; y < s->num_mods; y++) {
      printf("%16s  %2u |", ast_Module_getName(s->modules[y]), y);
      for (uint32_t x = 0; x < s->num_mods; x++) {
         uint8_t value = s->array[y * s->num_mods + x];
         printf("  %c", value ? 'X' : ' ');
      }
      printf("\n");
   }
}

static void module_sorter_ModuleSorter_handleModule(void* arg, ast_Module* mod)
{
   module_sorter_ModuleSorter* s = arg;
   ast_Module_visitImports(mod, module_sorter_ModuleSorter_handleImport, arg);
   s->cur_mod_idx++;
}

static void module_sorter_ModuleSorter_handleImport(void* arg, ast_ImportDecl* d)
{
   module_sorter_ModuleSorter* s = arg;
   ast_Module* dest = ast_ImportDecl_getDest(d);
   assert(dest);
   if (!component_Component_hasModule(s->comp, dest)) return;

   uint32_t dest_idx = module_sorter_ModuleSorter_mod2idx(s, dest);
   uint32_t offset = s->cur_mod_idx * s->num_mods + dest_idx;
   s->array[offset] = 1;
}

static uint32_t module_sorter_ModuleSorter_mod2idx(const module_sorter_ModuleSorter* s, const ast_Module* mod)
{
   for (uint32_t i = 0; i < s->num_mods; i++) {
      if (s->modules[i] == mod) return i;

   }
   assert(0);
   return 0;
}


// --- module c2_parser ---

typedef struct c2_parser_Info_ c2_parser_Info;
typedef struct c2_parser_Parser_ c2_parser_Parser;




struct c2_parser_Info_ {
   source_mgr_SourceMgr* sm;
   diagnostics_Diags* diags;
   string_pool_Pool* pool;
   const string_list_List* features;
};

struct c2_parser_Parser_ {
   c2_tokenizer_Tokenizer tokenizer;
   token_Token tok;
   int32_t file_id;
   ast_builder_Builder* builder;
   bool is_interface;
   c2_parser_Info* info;
   __jmp_buf_tag jmpbuf;
};

typedef enum {
   c2_parser_Prec_Unknown = 0,
   c2_parser_Prec_Comma = 1,
   c2_parser_Prec_Assignment = 2,
   c2_parser_Prec_Conditional = 3,
   c2_parser_Prec_LogicalAndOr = 4,
   c2_parser_Prec_Relational = 5,
   c2_parser_Prec_Additive = 6,
   c2_parser_Prec_Bitwise = 7,
   c2_parser_Prec_Shift = 8,
   c2_parser_Prec_Multiplicative = 9,
} __attribute__((packed)) c2_parser_Prec;

static void c2_parser_Info_init(c2_parser_Info* info, source_mgr_SourceMgr* sm, diagnostics_Diags* diags, string_pool_Pool* pool, const string_list_List* features);
static c2_parser_Parser* c2_parser_create(c2_parser_Info* info, ast_builder_Builder* builder);
static void c2_parser_Parser_free(c2_parser_Parser* p);
static void c2_parser_Parser_parse(c2_parser_Parser* p, int32_t file_id, bool is_interface);
static void c2_parser_Parser_consumeToken(c2_parser_Parser* p);
static void c2_parser_Parser_expectAndConsume(c2_parser_Parser* p, token_Kind kind);
static void c2_parser_Parser_expectIdentifier(c2_parser_Parser* p);
static void c2_parser_Parser_error(c2_parser_Parser* p, const char* format, ...);
static void c2_parser_Parser_parseModule(c2_parser_Parser* p);
static void c2_parser_Parser_parseImports(c2_parser_Parser* p);
static void c2_parser_Parser_parseTopLevel(c2_parser_Parser* p);
static void c2_parser_Parser_parseOptionalAttributes(c2_parser_Parser* p, ast_Decl* d);
static void c2_parser_Parser_parseFuncDecl(c2_parser_Parser* p, bool is_public);
static bool c2_parser_Parser_parseFunctionParams(c2_parser_Parser* p, ast_DeclList* params, bool is_public);
static ast_VarDecl* c2_parser_Parser_parseParamDecl(c2_parser_Parser* p, bool is_public);
static void c2_parser_Parser_parseTypeSpecifier(c2_parser_Parser* p, ast_TypeRefHolder* ref, bool allow_qualifier, bool allow_array);
static void c2_parser_Parser_parseOptionalArray(c2_parser_Parser* p, ast_TypeRefHolder* ref, ast_QualType base, bool allow_array);
static void c2_parser_Parser_parseVarDecl(c2_parser_Parser* p, bool is_public);
static void c2_parser_Parser_parseStaticAssert(c2_parser_Parser* p);
static bool c2_parser_Parser_parseOptionalAccessSpecifier(c2_parser_Parser* p);
static uint32_t c2_parser_Parser_parseOptionalTypeQualifier(c2_parser_Parser* p);
static ast_BuiltinKind c2_parser_tokKindToBuiltinKind(token_Kind kind);
static void c2_parser_Parser_parseSingleTypeSpecifier(c2_parser_Parser* p, ast_TypeRefHolder* ref, bool allow_qualifier);
static void c2_parser_Parser_parseFullTypeIdentifier(c2_parser_Parser* p, ast_TypeRefHolder* ref);
static void c2_parser_Parser_dump_token(c2_parser_Parser* p, const token_Token* tok);
static bool c2_parser_checkName(const char* name, bool isInterface);
static void c2_parser_Parser_parseTypeDecl(c2_parser_Parser* p, bool is_public);
static void c2_parser_Parser_parseFunctionType(c2_parser_Parser* p, uint32_t name, src_loc_SrcLoc loc, bool is_public);
static void c2_parser_Parser_parseStructType(c2_parser_Parser* p, bool is_struct, uint32_t name, src_loc_SrcLoc loc, bool is_public);
static void c2_parser_Parser_parseStructBlock(c2_parser_Parser* p, ast_DeclList* members, bool is_public);
static void c2_parser_Parser_parseEnumType(c2_parser_Parser* p, uint32_t name, src_loc_SrcLoc loc, bool is_public);
static void c2_parser_Parser_parseAliasType(c2_parser_Parser* p, uint32_t name, src_loc_SrcLoc loc, bool is_public);
static ast_Stmt* c2_parser_Parser_parseStmt(c2_parser_Parser* p);
static bool c2_parser_Parser_isTypeSpec(c2_parser_Parser* p);
static uint32_t c2_parser_Parser_skipArray(c2_parser_Parser* p, uint32_t lookahead);
static ast_Stmt* c2_parser_Parser_parseDeclOrStmt(c2_parser_Parser* p);
static ast_CompoundStmt* c2_parser_Parser_parseCompoundStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseAsmStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseAssertStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseBreakStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseContinueStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseFallthroughStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseCondition(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseIfStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseReturnStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseSwitchStmt(c2_parser_Parser* p, bool is_sswitch);
static ast_Stmt* c2_parser_Parser_parseCaseStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseDefaultStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseDoStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseForStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseWhileStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseDeclStmt(c2_parser_Parser* p, bool checkSemi, bool allowLocal);
static ast_Stmt* c2_parser_Parser_parseExprStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseLabelStmt(c2_parser_Parser* p);
static ast_Stmt* c2_parser_Parser_parseGotoStmt(c2_parser_Parser* p);
static bool c2_parser_Parser_isDeclaration(c2_parser_Parser* p);
static bool c2_parser_Parser_builtinArgIsType(c2_parser_Parser* p);
static ast_Expr* c2_parser_Parser_parseExpr(c2_parser_Parser* p);
static ast_Expr* c2_parser_Parser_parseAssignmentExpression(c2_parser_Parser* p);
static ast_Expr* c2_parser_Parser_parseRHSOfBinaryExpression(c2_parser_Parser* p, ast_Expr* lhs, c2_parser_Prec minPrec);
static ast_UnaryOpcode c2_parser_convertTokenToUnaryOpcode(token_Kind kind);
static ast_Expr* c2_parser_Parser_parseCastExpr(c2_parser_Parser* p, bool _arg1, bool _arg2);
static ast_Expr* c2_parser_Parser_parsePostfixExprSuffix(c2_parser_Parser* p, ast_Expr* lhs, bool couldBeTemplateCall);
static ast_Expr* c2_parser_Parser_parseCallExpr(c2_parser_Parser* p, ast_Expr* fn);
static ast_Expr* c2_parser_Parser_parseTemplateCallExpr(c2_parser_Parser* p, ast_Expr* fn, const ast_TypeRefHolder* ref);
static ast_Expr* c2_parser_Parser_parseImpureMemberExpr(c2_parser_Parser* p, ast_Expr* base);
static ast_Expr* c2_parser_Parser_parsePureMemberExpr(c2_parser_Parser* p);
static ast_IdentifierExpr* c2_parser_Parser_parseIdentifier(c2_parser_Parser* p);
static ast_Expr* c2_parser_Parser_parseStringLiteral(c2_parser_Parser* p);
static ast_Expr* c2_parser_Parser_parseParenExpr(c2_parser_Parser* p);
static bool c2_parser_Parser_isTemplateFunctionCall(c2_parser_Parser* p);
static bool c2_parser_couldBeType(token_Token tok);
static ast_Expr* c2_parser_Parser_parseSizeof(c2_parser_Parser* p);
static ast_Expr* c2_parser_Parser_parseElemsof(c2_parser_Parser* p);
static ast_Expr* c2_parser_Parser_parseInitValue(c2_parser_Parser* p, bool* need_semi, bool allow_designators);
static ast_Expr* c2_parser_Parser_parseInitList(c2_parser_Parser* p);
static ast_Expr* c2_parser_Parser_parseFieldDesignator(c2_parser_Parser* p, bool* need_semi);
static ast_Expr* c2_parser_Parser_parseArrayDesignator(c2_parser_Parser* p, bool* need_semi);
static ast_Expr* c2_parser_Parser_parseExplicitCastExpr(c2_parser_Parser* p);
static ast_Expr* c2_parser_Parser_parseEnumMinMax(c2_parser_Parser* p, bool is_min);
static ast_Expr* c2_parser_Parser_parseOffsetOfExpr(c2_parser_Parser* p);
static ast_Expr* c2_parser_Parser_parseToContainerExpr(c2_parser_Parser* p);
static ast_Expr* c2_parser_Parser_parseFullIdentifier(c2_parser_Parser* p);

static const ast_BuiltinKind c2_parser_Tok2builtin[] = {
   ast_BuiltinKind_Bool,
   ast_BuiltinKind_Char,
   ast_BuiltinKind_Int8,
   ast_BuiltinKind_Int16,
   ast_BuiltinKind_Int32,
   ast_BuiltinKind_Int64,
   ast_BuiltinKind_UInt8,
   ast_BuiltinKind_UInt16,
   ast_BuiltinKind_UInt32,
   ast_BuiltinKind_UInt64,
   ast_BuiltinKind_UInt8,
   ast_BuiltinKind_UInt16,
   ast_BuiltinKind_UInt32,
   ast_BuiltinKind_UInt64,
   ast_BuiltinKind_ISize,
   ast_BuiltinKind_USize,
   ast_BuiltinKind_Float32,
   ast_BuiltinKind_Float64,
   ast_BuiltinKind_Void
};

static const c2_parser_Prec c2_parser_BinOpPrecLookup[128] = {
   [token_Kind_Comma] = c2_parser_Prec_Comma,
   [token_Kind_Equal] = c2_parser_Prec_Assignment,
   [token_Kind_StarEqual] = c2_parser_Prec_Assignment,
   [token_Kind_SlashEqual] = c2_parser_Prec_Assignment,
   [token_Kind_PercentEqual] = c2_parser_Prec_Assignment,
   [token_Kind_PlusEqual] = c2_parser_Prec_Assignment,
   [token_Kind_MinusEqual] = c2_parser_Prec_Assignment,
   [token_Kind_LessLessEqual] = c2_parser_Prec_Assignment,
   [token_Kind_GreaterGreaterEqual] = c2_parser_Prec_Assignment,
   [token_Kind_AmpEqual] = c2_parser_Prec_Assignment,
   [token_Kind_CaretEqual] = c2_parser_Prec_Assignment,
   [token_Kind_PipeEqual] = c2_parser_Prec_Assignment,
   [token_Kind_Question] = c2_parser_Prec_Conditional,
   [token_Kind_PipePipe] = c2_parser_Prec_LogicalAndOr,
   [token_Kind_AmpAmp] = c2_parser_Prec_LogicalAndOr,
   [token_Kind_ExclaimEqual] = c2_parser_Prec_Relational,
   [token_Kind_EqualEqual] = c2_parser_Prec_Relational,
   [token_Kind_LessEqual] = c2_parser_Prec_Relational,
   [token_Kind_Less] = c2_parser_Prec_Relational,
   [token_Kind_Greater] = c2_parser_Prec_Relational,
   [token_Kind_GreaterEqual] = c2_parser_Prec_Relational,
   [token_Kind_Plus] = c2_parser_Prec_Additive,
   [token_Kind_Minus] = c2_parser_Prec_Additive,
   [token_Kind_Pipe] = c2_parser_Prec_Bitwise,
   [token_Kind_Caret] = c2_parser_Prec_Bitwise,
   [token_Kind_Amp] = c2_parser_Prec_Bitwise,
   [token_Kind_LessLess] = c2_parser_Prec_Shift,
   [token_Kind_GreaterGreater] = c2_parser_Prec_Shift,
   [token_Kind_Percent] = c2_parser_Prec_Multiplicative,
   [token_Kind_Slash] = c2_parser_Prec_Multiplicative,
   [token_Kind_Star] = c2_parser_Prec_Multiplicative
};

static const ast_BinaryOpcode c2_parser_BinOpTokenLookup[128] = {
   [token_Kind_Star] = ast_BinaryOpcode_Multiply,
   [token_Kind_Slash] = ast_BinaryOpcode_Divide,
   [token_Kind_Percent] = ast_BinaryOpcode_Reminder,
   [token_Kind_Plus] = ast_BinaryOpcode_Add,
   [token_Kind_Minus] = ast_BinaryOpcode_Subtract,
   [token_Kind_LessLess] = ast_BinaryOpcode_ShiftLeft,
   [token_Kind_GreaterGreater] = ast_BinaryOpcode_ShiftRight,
   [token_Kind_Less] = ast_BinaryOpcode_LessThan,
   [token_Kind_Greater] = ast_BinaryOpcode_GreaterThan,
   [token_Kind_LessEqual] = ast_BinaryOpcode_LessEqual,
   [token_Kind_GreaterEqual] = ast_BinaryOpcode_GreaterEqual,
   [token_Kind_EqualEqual] = ast_BinaryOpcode_Equal,
   [token_Kind_ExclaimEqual] = ast_BinaryOpcode_NotEqual,
   [token_Kind_Amp] = ast_BinaryOpcode_And,
   [token_Kind_Caret] = ast_BinaryOpcode_Xor,
   [token_Kind_Pipe] = ast_BinaryOpcode_Or,
   [token_Kind_AmpAmp] = ast_BinaryOpcode_LAnd,
   [token_Kind_PipePipe] = ast_BinaryOpcode_LOr,
   [token_Kind_Equal] = ast_BinaryOpcode_Assign,
   [token_Kind_StarEqual] = ast_BinaryOpcode_MulAssign,
   [token_Kind_SlashEqual] = ast_BinaryOpcode_DivAssign,
   [token_Kind_PercentEqual] = ast_BinaryOpcode_RemAssign,
   [token_Kind_PlusEqual] = ast_BinaryOpcode_AddAssign,
   [token_Kind_MinusEqual] = ast_BinaryOpcode_SubAssign,
   [token_Kind_LessLessEqual] = ast_BinaryOpcode_ShlAssign,
   [token_Kind_GreaterGreaterEqual] = ast_BinaryOpcode_ShrASsign,
   [token_Kind_AmpEqual] = ast_BinaryOpcode_AndAssign,
   [token_Kind_CaretEqual] = ast_BinaryOpcode_XorAssign,
   [token_Kind_PipeEqual] = ast_BinaryOpcode_OrAssign
};

static const uint8_t c2_parser_CastExprTokenLookup[128] = {
   [token_Kind_Identifier] = 1,
   [token_Kind_NumberLiteral] = 2,
   [token_Kind_CharLiteral] = 3,
   [token_Kind_StringLiteral] = 4,
   [token_Kind_LParen] = 5,
   [token_Kind_Star] = 6,
   [token_Kind_Tilde] = 6,
   [token_Kind_Minus] = 6,
   [token_Kind_Exclaim] = 6,
   [token_Kind_Amp] = 7,
   [token_Kind_KW_cast] = 8,
   [token_Kind_Plus] = 9,
   [token_Kind_PlusPlus] = 10,
   [token_Kind_MinusMinus] = 10,
   [token_Kind_KW_elemsof] = 11,
   [token_Kind_KW_enum_min] = 12,
   [token_Kind_KW_enum_max] = 12,
   [token_Kind_KW_false] = 13,
   [token_Kind_KW_true] = 13,
   [token_Kind_KW_nil] = 14,
   [token_Kind_KW_offsetof] = 15,
   [token_Kind_KW_sizeof] = 16,
   [token_Kind_KW_to_container] = 17
};

static void c2_parser_Info_init(c2_parser_Info* info, source_mgr_SourceMgr* sm, diagnostics_Diags* diags, string_pool_Pool* pool, const string_list_List* features)
{
   info->sm = sm;
   info->diags = diags;
   info->pool = pool;
   info->features = features;
}

static c2_parser_Parser* c2_parser_create(c2_parser_Info* info, ast_builder_Builder* builder)
{
   c2_parser_Parser* p = calloc(1, sizeof(c2_parser_Parser));
   p->info = info;
   p->builder = builder;
   return p;
}

static void c2_parser_Parser_free(c2_parser_Parser* p)
{
   free(p);
}

static void c2_parser_Parser_parse(c2_parser_Parser* p, int32_t file_id, bool is_interface)
{
   p->file_id = file_id;
   p->is_interface = is_interface;
   int32_t res = setjmp(&p->jmpbuf);
   if (res == 0) {
      c2_tokenizer_Tokenizer_init(&p->tokenizer, p->info->pool, source_mgr_SourceMgr_get_content(p->info->sm, p->file_id), source_mgr_SourceMgr_get_offset(p->info->sm, p->file_id), p->info->features);
      token_Token_init(&p->tok);
      c2_parser_Parser_consumeToken(p);
      c2_parser_Parser_parseModule(p);
      c2_parser_Parser_parseImports(p);
      while (p->tok.more) {
         c2_parser_Parser_parseTopLevel(p);
      }
   }
}

static void c2_parser_Parser_consumeToken(c2_parser_Parser* p)
{
   c2_tokenizer_Tokenizer_lex(&p->tokenizer, &p->tok);
   if (p->tok.kind == token_Kind_Error) c2_parser_Parser_error(p, p->tok.error_msg);
}

static void c2_parser_Parser_expectAndConsume(c2_parser_Parser* p, token_Kind kind)
{
   if (p->tok.kind == kind) {
      c2_parser_Parser_consumeToken(p);
      return;
   }
   c2_parser_Parser_error(p, "expected %s got %s", token_kind2str(kind), token_kind2str(p->tok.kind));
}

static void c2_parser_Parser_expectIdentifier(c2_parser_Parser* p)
{
   if (p->tok.kind == token_Kind_Identifier) return;

   c2_parser_Parser_error(p, "expected identifier");
}

static void c2_parser_Parser_error(c2_parser_Parser* p, const char* format, ...)
{
   char msg[256];
   va_list args;
   va_start(args, format);
   vsnprintf(msg, sizeof(msg) - 1, format, args);
   va_end(args);
   diagnostics_Diags_error(p->info->diags, p->tok.loc, msg);
   longjmp(&p->jmpbuf, 1);
}

static void c2_parser_Parser_parseModule(c2_parser_Parser* p)
{
   c2_parser_Parser_expectAndConsume(p, token_Kind_KW_module);
   c2_parser_Parser_expectIdentifier(p);
   ast_builder_Builder_actOnModule(p->builder, p->tok.text_idx, p->tok.loc, source_mgr_SourceMgr_getFileName(p->info->sm, p->file_id));
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
}

static void c2_parser_Parser_parseImports(c2_parser_Parser* p)
{
   while (p->tok.kind == token_Kind_KW_import) {
      c2_parser_Parser_consumeToken(p);
      c2_parser_Parser_expectIdentifier(p);
      uint32_t mod_name = p->tok.text_idx;
      src_loc_SrcLoc mod_loc = p->tok.loc;
      uint32_t alias_name = 0;
      src_loc_SrcLoc alias_loc = 0;
      c2_parser_Parser_consumeToken(p);
      if (p->tok.kind == token_Kind_KW_as) {
         c2_parser_Parser_consumeToken(p);
         c2_parser_Parser_expectIdentifier(p);
         alias_name = p->tok.text_idx;
         alias_loc = p->tok.loc;
         c2_parser_Parser_consumeToken(p);
      }
      bool islocal = false;
      if (p->tok.kind == token_Kind_KW_local) {
         c2_parser_Parser_consumeToken(p);
         islocal = true;
      }
      c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
      ast_builder_Builder_actOnImport(p->builder, mod_name, mod_loc, alias_name, alias_loc, islocal);
   }
}

static void c2_parser_Parser_parseTopLevel(c2_parser_Parser* p)
{
   bool is_public = c2_parser_Parser_parseOptionalAccessSpecifier(p);
   switch (p->tok.kind) {
   case token_Kind_KW_assert:
      c2_parser_Parser_error(p, "assert can only be used inside a function");
      break;
   case token_Kind_KW_func:
      c2_parser_Parser_parseFuncDecl(p, is_public);
      break;
   case token_Kind_KW_import:
      c2_parser_Parser_error(p, "import after decls");
      break;
   case token_Kind_KW_static_assert:
      if (is_public) c2_parser_Parser_error(p, "static_assert cannot be public");
      c2_parser_Parser_parseStaticAssert(p);
      break;
   case token_Kind_KW_type:
      c2_parser_Parser_parseTypeDecl(p, is_public);
      break;
   case token_Kind_Eof:
      break;
   default:
      c2_parser_Parser_parseVarDecl(p, is_public);
      break;
   }
}

static void c2_parser_Parser_parseOptionalAttributes(c2_parser_Parser* p, ast_Decl* d)
{
   if (p->tok.kind != token_Kind_At) return;

   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_LParen);
   while (1) {
      c2_parser_Parser_expectIdentifier(p);
      uint32_t attr_id = p->tok.text_idx;
      src_loc_SrcLoc attr_loc = p->tok.loc;
      c2_parser_Parser_consumeToken(p);
      attr_Value value;
      attr_Value* value_ptr = NULL;
      if (p->tok.kind == token_Kind_Equal) {
         c2_parser_Parser_consumeToken(p);
         switch (p->tok.kind) {
         case token_Kind_StringLiteral:
            value.is_number = false;
            value.text_idx = p->tok.text_idx;
            value.loc = p->tok.loc;
            c2_parser_Parser_consumeToken(p);
            value_ptr = &value;
            break;
         case token_Kind_NumberLiteral:
            value.is_number = true;
            value.number = ((uint32_t)(p->tok.number_value));
            value.loc = p->tok.loc;
            c2_parser_Parser_consumeToken(p);
            value_ptr = &value;
            break;
         default:
            c2_parser_Parser_error(p, "expected attribute argument");
            return;
         }
      }
      ast_builder_Builder_actOnAttr(p->builder, d, attr_id, attr_loc, value_ptr);
      if (p->tok.kind != token_Kind_Comma) break;

      c2_parser_Parser_consumeToken(p);
   }
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
}

static void c2_parser_Parser_parseFuncDecl(c2_parser_Parser* p, bool is_public)
{
   c2_parser_Parser_consumeToken(p);
   ast_TypeRefHolder rtype;
   ast_TypeRefHolder_init(&rtype);
   c2_parser_Parser_parseSingleTypeSpecifier(p, &rtype, true);
   c2_parser_Parser_expectIdentifier(p);
   uint32_t func_name = p->tok.text_idx;
   src_loc_SrcLoc func_loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   ast_Ref prefix_ref;
   ast_Ref* prefix = NULL;
   if (p->tok.kind == token_Kind_Dot) {
      c2_parser_Parser_consumeToken(p);
      c2_parser_Parser_expectIdentifier(p);
      prefix_ref.loc = func_loc;
      prefix_ref.name_idx = func_name;
      prefix_ref.decl = NULL;
      prefix = &prefix_ref;
      func_name = p->tok.text_idx;
      func_loc = p->tok.loc;
      c2_parser_Parser_consumeToken(p);
   }
   if (!c2_parser_checkName(string_pool_Pool_idx2str(p->info->pool, func_name), p->is_interface)) {
      p->tok.loc = func_loc;
      c2_parser_Parser_error(p, "a function name must start with a lower-case letter");
   }
   ast_DeclList params;
   ast_DeclList_init(&params, 4);
   bool is_variadic = c2_parser_Parser_parseFunctionParams(p, &params, is_public);
   ast_FunctionDecl* f;
   if (p->tok.kind == token_Kind_KW_template) {
      c2_parser_Parser_consumeToken(p);
      c2_parser_Parser_expectIdentifier(p);
      uint32_t template_name = p->tok.text_idx;
      src_loc_SrcLoc template_loc = p->tok.loc;
      c2_parser_Parser_consumeToken(p);
      f = ast_builder_Builder_actOnTemplateFunctionDecl(p->builder, func_name, func_loc, is_public, &rtype, template_name, template_loc, ((ast_VarDecl**)(ast_DeclList_getDecls(&params))), ast_DeclList_size(&params), is_variadic);
   } else {
      f = ast_builder_Builder_actOnFunctionDecl(p->builder, func_name, func_loc, is_public, &rtype, prefix, ((ast_VarDecl**)(ast_DeclList_getDecls(&params))), ast_DeclList_size(&params), is_variadic);
   }
   ast_DeclList_free(&params);
   c2_parser_Parser_parseOptionalAttributes(p, ((ast_Decl*)(f)));
   if (p->is_interface) {
      if (p->tok.kind == token_Kind_LBrace) {
         c2_parser_Parser_error(p, "interface functions cannot have a function body");
      }
      c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
      return;
   }
   ast_CompoundStmt* body = c2_parser_Parser_parseCompoundStmt(p);
   ast_builder_Builder_actOnFunctionBody(p->builder, f, body);
}

static bool c2_parser_Parser_parseFunctionParams(c2_parser_Parser* p, ast_DeclList* params, bool is_public)
{
   c2_parser_Parser_expectAndConsume(p, token_Kind_LParen);
   if (p->tok.kind == token_Kind_RParen) {
      c2_parser_Parser_consumeToken(p);
      return false;
   }
   bool is_variadic = false;
   while (1) {
      ast_VarDecl* decl = c2_parser_Parser_parseParamDecl(p, is_public);
      ast_DeclList_add(params, ast_VarDecl_asDecl(decl));
      if (p->tok.kind != token_Kind_Comma) break;

      c2_parser_Parser_consumeToken(p);
      if (p->tok.kind == token_Kind_Ellipsis) {
         is_variadic = true;
         c2_parser_Parser_consumeToken(p);
         break;
      }
   }
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   return is_variadic;
}

static ast_VarDecl* c2_parser_Parser_parseParamDecl(c2_parser_Parser* p, bool is_public)
{
   if (p->tok.kind == token_Kind_KW_local) c2_parser_Parser_error(p, "invalid local");
   ast_TypeRefHolder ref;
   ast_TypeRefHolder_init(&ref);
   c2_parser_Parser_parseTypeSpecifier(p, &ref, true, false);
   uint32_t name = 0;
   src_loc_SrcLoc loc = p->tok.loc;
   if (p->tok.kind == token_Kind_Identifier) {
      name = p->tok.text_idx;
      c2_parser_Parser_consumeToken(p);
   }
   if (p->tok.kind == token_Kind_Equal) {
      c2_parser_Parser_error(p, "default parameter values are not allowed");
   }
   return ast_builder_Builder_actOnFunctionParam(p->builder, name, loc, is_public, &ref);
}

static void c2_parser_Parser_parseTypeSpecifier(c2_parser_Parser* p, ast_TypeRefHolder* ref, bool allow_qualifier, bool allow_array)
{
   c2_parser_Parser_parseSingleTypeSpecifier(p, ref, allow_qualifier);
   c2_parser_Parser_parseOptionalArray(p, ref, ast_QualType_Invalid, allow_array);
}

static void c2_parser_Parser_parseOptionalArray(c2_parser_Parser* p, ast_TypeRefHolder* ref, ast_QualType base, bool allow_array)
{
   if (p->tok.kind != token_Kind_LSquare) return;

   if (!allow_array) c2_parser_Parser_error(p, "array types are not allowed here");
   if (ast_TypeRefHolder_getNumArrays(ref) == 3) c2_parser_Parser_error(p, "arrays cannot have more than 3 dimensions");
   c2_parser_Parser_consumeToken(p);
   bool is_incremental = false;
   ast_Expr* size = NULL;
   if (p->tok.kind == token_Kind_RSquare) {
      ast_TypeRefHolder_addArray(ref, NULL);
      c2_parser_Parser_consumeToken(p);
   } else if (p->tok.kind == token_Kind_Plus) {
      c2_parser_Parser_consumeToken(p);
      ast_TypeRefHolder_setIncrArray(ref);
      is_incremental = true;
      c2_parser_Parser_expectAndConsume(p, token_Kind_RSquare);
   } else {
      size = c2_parser_Parser_parseExpr(p);
      ast_TypeRefHolder_addArray(ref, size);
      c2_parser_Parser_expectAndConsume(p, token_Kind_RSquare);
   }

   c2_parser_Parser_parseOptionalArray(p, ref, base, true);
}

static void c2_parser_Parser_parseVarDecl(c2_parser_Parser* p, bool is_public)
{
   if (p->tok.kind == token_Kind_KW_local) c2_parser_Parser_error(p, "local cannot be used at file scope");
   ast_TypeRefHolder ref;
   ast_TypeRefHolder_init(&ref);
   c2_parser_Parser_parseTypeSpecifier(p, &ref, true, true);
   c2_parser_Parser_expectIdentifier(p);
   uint32_t name = p->tok.text_idx;
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   bool need_semi = true;
   ast_Expr* initValue = NULL;
   c2_parser_Parser_parseOptionalAttributes(p, NULL);
   if (p->tok.kind == token_Kind_Equal) {
      c2_parser_Parser_consumeToken(p);
      initValue = c2_parser_Parser_parseInitValue(p, &need_semi, false);
   }
   if (need_semi) c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
   ast_builder_Builder_actOnGlobalVarDecl(p->builder, name, loc, is_public, &ref, initValue);
}

static void c2_parser_Parser_parseStaticAssert(c2_parser_Parser* p)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_LParen);
   ast_Expr* lhs = c2_parser_Parser_parseExpr(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Comma);
   ast_Expr* rhs = c2_parser_Parser_parseExpr(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
   ast_builder_Builder_actOnStaticAssert(p->builder, loc, lhs, rhs);
}

static bool c2_parser_Parser_parseOptionalAccessSpecifier(c2_parser_Parser* p)
{
   if (p->tok.kind == token_Kind_KW_public) {
      c2_parser_Parser_consumeToken(p);
      return true;
   }
   return false;
}

static uint32_t c2_parser_Parser_parseOptionalTypeQualifier(c2_parser_Parser* p)
{
   uint32_t qualifiers = 0;
   if (p->tok.kind == token_Kind_KW_const) {
      c2_parser_Parser_consumeToken(p);
      qualifiers |= ast_QualType_Const;
   }
   if (p->tok.kind == token_Kind_KW_volatile) {
      c2_parser_Parser_consumeToken(p);
      qualifiers |= ast_QualType_Volatile;
   }
   return qualifiers;
}

static ast_BuiltinKind c2_parser_tokKindToBuiltinKind(token_Kind kind)
{
   return c2_parser_Tok2builtin[kind - token_Kind_KW_bool];
}

static void c2_parser_Parser_parseSingleTypeSpecifier(c2_parser_Parser* p, ast_TypeRefHolder* ref, bool allow_qualifier)
{
   uint32_t type_qualifier = 0;
   if (allow_qualifier) {
      type_qualifier = c2_parser_Parser_parseOptionalTypeQualifier(p);
      ast_TypeRefHolder_setQualifiers(ref, type_qualifier);
   }
   token_Kind kind = p->tok.kind;
   if ((kind >= token_Kind_KW_bool && kind <= token_Kind_KW_void)) {
      ast_TypeRefHolder_setBuiltin(ref, c2_parser_tokKindToBuiltinKind(p->tok.kind));
      c2_parser_Parser_consumeToken(p);
   } else if (kind == token_Kind_Identifier) {
      c2_parser_Parser_parseFullTypeIdentifier(p, ref);
   } else {
      c2_parser_Parser_error(p, "expected type specifier");
   }

   uint32_t depth = 0;
   while (p->tok.kind == token_Kind_Star) {
      depth++;
      if (depth > 3) c2_parser_Parser_error(p, "pointers have a maximum nesting of 3");
      ast_TypeRefHolder_addPointer(ref);
      c2_parser_Parser_consumeToken(p);
   }
}

static void c2_parser_Parser_parseFullTypeIdentifier(c2_parser_Parser* p, ast_TypeRefHolder* ref)
{
   ast_TypeRefHolder_setUser(ref, p->tok.loc, p->tok.text_idx);
   c2_parser_Parser_consumeToken(p);
   if (p->tok.kind == token_Kind_Dot) {
      c2_parser_Parser_consumeToken(p);
      c2_parser_Parser_expectIdentifier(p);
      ast_TypeRefHolder_setPrefix(ref, p->tok.loc, p->tok.text_idx);
      c2_parser_Parser_consumeToken(p);
   }
}

static void c2_parser_Parser_dump_token(c2_parser_Parser* p, const token_Token* tok)
{
   if ((tok->kind >= token_Kind_KW_as && tok->kind <= token_Kind_KW_while)) {
      printf("%s%10s%s  %s\n", color_Green, token_kind2str(tok->kind), color_Normal, source_mgr_SourceMgr_loc2str(p->info->sm, tok->loc));
      return;
   }
   printf("%10s  %s  ", token_kind2str(tok->kind), source_mgr_SourceMgr_loc2str(p->info->sm, tok->loc));
   switch (tok->kind) {
   case token_Kind_Identifier:
      printf("  %s%s%s", color_Cyan, string_pool_Pool_idx2str(p->info->pool, tok->text_idx), color_Normal);
      break;
   case token_Kind_NumberLiteral:
      printf("  (%u) %s%s%s", tok->radix, color_Cyan, string_pool_Pool_idx2str(p->info->pool, tok->text_idx), color_Normal);
      break;
   case token_Kind_CharLiteral:
      printf("  %s'%c'%s", color_Cyan, tok->char_value, color_Normal);
      break;
   case token_Kind_StringLiteral:
      printf("  %s\"%s\"%s", color_Cyan, string_pool_Pool_idx2str(p->info->pool, tok->text_idx), color_Normal);
      break;
   case token_Kind_Warning:
      printf("  %s%s%s", color_Yellow, tok->error_msg, color_Normal);
      break;
   case token_Kind_Error:
      printf("  %s%s%s", color_Red, tok->error_msg, color_Normal);
      break;
   default:
      break;
   }
   printf("\n");
}

static bool c2_parser_checkName(const char* name, bool isInterface)
{
   char c = name[0];
   if (islower(c)) return true;

   if ((c == '_' && isInterface)) return true;

   return false;
}

static void c2_parser_Parser_parseTypeDecl(c2_parser_Parser* p, bool is_public)
{
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectIdentifier(p);
   uint32_t type_name = p->tok.text_idx;
   src_loc_SrcLoc type_loc = p->tok.loc;
   const char* name = string_pool_Pool_idx2str(p->info->pool, type_name);
   if (!isupper(name[0])) c2_parser_Parser_error(p, "type name must start with upper-case character");
   c2_parser_Parser_consumeToken(p);
   switch (p->tok.kind) {
   case token_Kind_KW_func:
      c2_parser_Parser_parseFunctionType(p, type_name, type_loc, is_public);
      break;
   case token_Kind_KW_struct:
      c2_parser_Parser_parseStructType(p, true, type_name, type_loc, is_public);
      break;
   case token_Kind_KW_union:
      c2_parser_Parser_parseStructType(p, false, type_name, type_loc, is_public);
      break;
   case token_Kind_KW_enum:
      c2_parser_Parser_parseEnumType(p, type_name, type_loc, is_public);
      break;
   default:
      c2_parser_Parser_parseAliasType(p, type_name, type_loc, is_public);
      break;
   }
}

static void c2_parser_Parser_parseFunctionType(c2_parser_Parser* p, uint32_t name, src_loc_SrcLoc loc, bool is_public)
{
   c2_parser_Parser_consumeToken(p);
   ast_TypeRefHolder rtype;
   ast_TypeRefHolder_init(&rtype);
   c2_parser_Parser_parseSingleTypeSpecifier(p, &rtype, true);
   ast_DeclList params;
   ast_DeclList_init(&params, 4);
   bool is_variadic = c2_parser_Parser_parseFunctionParams(p, &params, is_public);
   ast_Decl* f = ast_builder_Builder_actOnFunctionTypeDecl(p->builder, name, loc, is_public, &rtype, ((ast_VarDecl**)(ast_DeclList_getDecls(&params))), ast_DeclList_size(&params), is_variadic);
   c2_parser_Parser_parseOptionalAttributes(p, f);
   ast_DeclList_free(&params);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
}

static void c2_parser_Parser_parseStructType(c2_parser_Parser* p, bool is_struct, uint32_t name, src_loc_SrcLoc loc, bool is_public)
{
   c2_parser_Parser_consumeToken(p);
   ast_DeclList members;
   ast_DeclList_init(&members, 8);
   c2_parser_Parser_parseStructBlock(p, &members, is_public);
   ast_StructTypeDecl* d = ast_builder_Builder_actOnStructType(p->builder, name, loc, is_public, is_struct, true, ((ast_VarDecl**)(ast_DeclList_getDecls(&members))), ast_DeclList_size(&members));
   ast_DeclList_free(&members);
   c2_parser_Parser_parseOptionalAttributes(p, ((ast_Decl*)(d)));
}

static void c2_parser_Parser_parseStructBlock(c2_parser_Parser* p, ast_DeclList* members, bool is_public)
{
   c2_parser_Parser_expectAndConsume(p, token_Kind_LBrace);
   while (1) {
      if (p->tok.kind == token_Kind_RBrace) break;

      if ((p->tok.kind == token_Kind_KW_union || p->tok.kind == token_Kind_KW_struct)) {
         bool is_struct = p->tok.kind == token_Kind_KW_struct;
         c2_parser_Parser_consumeToken(p);
         uint32_t name = 0;
         src_loc_SrcLoc loc = 0;
         if (p->tok.kind == token_Kind_Identifier) {
            name = p->tok.text_idx;
            loc = p->tok.loc;
            c2_parser_Parser_consumeToken(p);
         }
         ast_DeclList sub_members;
         ast_DeclList_init(&sub_members, 4);
         c2_parser_Parser_parseStructBlock(p, &sub_members, is_public);
         ast_StructTypeDecl* member = ast_builder_Builder_actOnStructType(p->builder, name, loc, is_public, is_struct, false, ((ast_VarDecl**)(ast_DeclList_getDecls(&sub_members))), ast_DeclList_size(&sub_members));
         ast_DeclList_free(&sub_members);
         ast_DeclList_add(members, ast_StructTypeDecl_asDecl(member));
      } else {
         ast_TypeRefHolder ref;
         ast_TypeRefHolder_init(&ref);
         c2_parser_Parser_parseTypeSpecifier(p, &ref, true, true);
         uint32_t name = 0;
         src_loc_SrcLoc loc;
         if (p->tok.kind == token_Kind_Colon) {
            loc = p->tok.loc;
         } else {
            c2_parser_Parser_expectIdentifier(p);
            name = p->tok.text_idx;
            loc = p->tok.loc;
            c2_parser_Parser_consumeToken(p);
         }
         ast_Expr* bitfield = NULL;
         if (p->tok.kind == token_Kind_Colon) {
            c2_parser_Parser_consumeToken(p);
            bitfield = c2_parser_Parser_parseExpr(p);
         }
         ast_VarDecl* member = ast_builder_Builder_actOnStructMember(p->builder, name, loc, is_public, &ref, bitfield);
         ast_DeclList_add(members, ast_VarDecl_asDecl(member));
         c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
      }
   }
   c2_parser_Parser_expectAndConsume(p, token_Kind_RBrace);
}

static void c2_parser_Parser_parseEnumType(c2_parser_Parser* p, uint32_t name, src_loc_SrcLoc loc, bool is_public)
{
   c2_parser_Parser_consumeToken(p);
   switch (p->tok.kind) {
   case token_Kind_KW_char:
      // fallthrough
   case token_Kind_KW_f32:
      // fallthrough
   case token_Kind_KW_f64:
      c2_parser_Parser_error(p, "enum type must be an integer");
      break;
   case token_Kind_KW_i8:
      // fallthrough
   case token_Kind_KW_i16:
      // fallthrough
   case token_Kind_KW_i32:
      // fallthrough
   case token_Kind_KW_i64:
      // fallthrough
   case token_Kind_KW_isize:
      break;
   case token_Kind_KW_reg8:
      // fallthrough
   case token_Kind_KW_reg16:
      // fallthrough
   case token_Kind_KW_reg32:
      // fallthrough
   case token_Kind_KW_reg64:
      c2_parser_Parser_error(p, "enum type must be an integer");
      break;
   case token_Kind_KW_u8:
      // fallthrough
   case token_Kind_KW_u16:
      // fallthrough
   case token_Kind_KW_u32:
      // fallthrough
   case token_Kind_KW_u64:
      // fallthrough
   case token_Kind_KW_usize:
      break;
   case token_Kind_KW_void:
      c2_parser_Parser_error(p, "enum type must be an integer");
      break;
   default:
      c2_parser_Parser_error(p, "expected enum type");
      break;
   }
   ast_QualType implType = ast_builder_Builder_actOnBuiltinType(p->builder, c2_parser_tokKindToBuiltinKind(p->tok.kind));
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_LBrace);
   bool is_incr = false;
   ast_DeclList constants;
   ast_DeclList_init(&constants, 16);
   if (p->tok.kind == token_Kind_Plus) {
      is_incr = true;
      c2_parser_Parser_consumeToken(p);
      c2_parser_Parser_error(p, "TODO incremental enum");
   } else {
      while (p->tok.kind == token_Kind_Identifier) {
         uint32_t const_name = p->tok.text_idx;
         src_loc_SrcLoc const_loc = p->tok.loc;
         c2_parser_Parser_consumeToken(p);
         ast_Expr* init_expr = NULL;
         if (p->tok.kind == token_Kind_Equal) {
            c2_parser_Parser_consumeToken(p);
            init_expr = c2_parser_Parser_parseExpr(p);
         }
         ast_EnumConstantDecl* constant = ast_builder_Builder_actOnEnumConstant(p->builder, const_name, const_loc, is_public, init_expr);
         ast_DeclList_add(&constants, ast_EnumConstantDecl_asDecl(constant));
         if (p->tok.kind != token_Kind_Comma) break;

         c2_parser_Parser_consumeToken(p);
      }
   }
   if (ast_DeclList_size(&constants) == 0) {
      c2_parser_Parser_error(p, "enum without constants");
   }
   c2_parser_Parser_expectAndConsume(p, token_Kind_RBrace);
   ast_Decl* d = ast_builder_Builder_actOnEnumType(p->builder, name, loc, is_public, is_incr, implType, ((ast_EnumConstantDecl**)(ast_DeclList_getDecls(&constants))), ast_DeclList_size(&constants));
   ast_DeclList_free(&constants);
   c2_parser_Parser_parseOptionalAttributes(p, d);
}

static void c2_parser_Parser_parseAliasType(c2_parser_Parser* p, uint32_t name, src_loc_SrcLoc loc, bool is_public)
{
   ast_TypeRefHolder ref;
   ast_TypeRefHolder_init(&ref);
   c2_parser_Parser_parseTypeSpecifier(p, &ref, true, true);
   ast_Decl* d = ast_builder_Builder_actOnAliasType(p->builder, name, loc, is_public, &ref);
   c2_parser_Parser_parseOptionalAttributes(p, d);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
}

static ast_Stmt* c2_parser_Parser_parseStmt(c2_parser_Parser* p)
{
   switch (p->tok.kind) {
   case token_Kind_Identifier:
      return c2_parser_Parser_parseDeclOrStmt(p);
   case token_Kind_LBrace:
      return ((ast_Stmt*)(c2_parser_Parser_parseCompoundStmt(p)));
   case token_Kind_RBrace:
      c2_parser_Parser_error(p, "expected stmt");
      break;
   case token_Kind_KW_asm:
      return c2_parser_Parser_parseAsmStmt(p);
   case token_Kind_KW_assert:
      return c2_parser_Parser_parseAssertStmt(p);
   case token_Kind_KW_break:
      return c2_parser_Parser_parseBreakStmt(p);
   case token_Kind_KW_continue:
      return c2_parser_Parser_parseContinueStmt(p);
   case token_Kind_KW_do:
      return c2_parser_Parser_parseDoStmt(p);
   case token_Kind_KW_fallthrough:
      return c2_parser_Parser_parseFallthroughStmt(p);
   case token_Kind_KW_for:
      return c2_parser_Parser_parseForStmt(p);
   case token_Kind_KW_goto:
      return c2_parser_Parser_parseGotoStmt(p);
   case token_Kind_KW_if:
      return c2_parser_Parser_parseIfStmt(p);
   case token_Kind_KW_return:
      return c2_parser_Parser_parseReturnStmt(p);
   case token_Kind_KW_switch:
      return c2_parser_Parser_parseSwitchStmt(p, false);
   case token_Kind_KW_sswitch:
      return c2_parser_Parser_parseSwitchStmt(p, true);
   case token_Kind_KW_bool:
      // fallthrough
   case token_Kind_KW_char:
      // fallthrough
   case token_Kind_KW_const:
      // fallthrough
   case token_Kind_KW_i8:
      // fallthrough
   case token_Kind_KW_i16:
      // fallthrough
   case token_Kind_KW_i32:
      // fallthrough
   case token_Kind_KW_i64:
      // fallthrough
   case token_Kind_KW_isize:
      // fallthrough
   case token_Kind_KW_f32:
      // fallthrough
   case token_Kind_KW_f64:
      // fallthrough
   case token_Kind_KW_local:
      // fallthrough
   case token_Kind_KW_reg8:
      // fallthrough
   case token_Kind_KW_reg16:
      // fallthrough
   case token_Kind_KW_reg32:
      // fallthrough
   case token_Kind_KW_reg64:
      // fallthrough
   case token_Kind_KW_u8:
      // fallthrough
   case token_Kind_KW_u16:
      // fallthrough
   case token_Kind_KW_u32:
      // fallthrough
   case token_Kind_KW_u64:
      // fallthrough
   case token_Kind_KW_usize:
      // fallthrough
   case token_Kind_KW_volatile:
      // fallthrough
   case token_Kind_KW_void:
      return c2_parser_Parser_parseDeclStmt(p, true, true);
   case token_Kind_KW_while:
      return c2_parser_Parser_parseWhileStmt(p);
   default:
      return c2_parser_Parser_parseExprStmt(p);
   }
   return NULL;
}

static bool c2_parser_Parser_isTypeSpec(c2_parser_Parser* p)
{
   assert(p->tok.kind == token_Kind_Identifier);
   token_Token t;
   uint32_t state = 0;
   uint32_t lookahead = 1;
   while (1) {
      token_Token t2 = c2_tokenizer_Tokenizer_lookahead(&p->tokenizer, lookahead);
      switch (t2.kind) {
      case token_Kind_Identifier:
         goto type_done;
      case token_Kind_LSquare:
         lookahead = c2_parser_Parser_skipArray(p, lookahead);
         state = 3;
         break;
      case token_Kind_Star:
         if (state == 3) return false;

         state = 2;
         lookahead++;
         break;
      case token_Kind_Dot:
         if (state == 0) {
            token_Token t3 = c2_tokenizer_Tokenizer_lookahead(&p->tokenizer, lookahead + 1);
            if (t3.kind != token_Kind_Identifier) {
               return false;
            }
            state = 2;
            lookahead += 2;
         } else {
            return false;
         }
         break;
      default:
         goto type_done;
      }
   }
   type_done:
   t = c2_tokenizer_Tokenizer_lookahead(&p->tokenizer, lookahead);
   return t.kind == token_Kind_Identifier;
}

static uint32_t c2_parser_Parser_skipArray(c2_parser_Parser* p, uint32_t lookahead)
{
   lookahead++;
   uint32_t depth = 1;
   while (depth) {
      token_Token next = c2_tokenizer_Tokenizer_lookahead(&p->tokenizer, lookahead);
      switch (next.kind) {
      case token_Kind_LSquare:
         depth++;
         break;
      case token_Kind_RSquare:
         depth--;
         break;
      case token_Kind_Eof:
         c2_parser_Parser_error(p, "unexpected end-of-file");
         break;
      default:
         break;
      }
      lookahead++;
   }
   return lookahead;
}

static ast_Stmt* c2_parser_Parser_parseDeclOrStmt(c2_parser_Parser* p)
{
   assert(p->tok.kind == token_Kind_Identifier);
   bool isDecl = c2_parser_Parser_isTypeSpec(p);
   if (isDecl) return c2_parser_Parser_parseDeclStmt(p, true, true);

   token_Token next = c2_tokenizer_Tokenizer_lookahead(&p->tokenizer, 1);
   if (next.kind == token_Kind_Colon) return c2_parser_Parser_parseLabelStmt(p);

   return c2_parser_Parser_parseExprStmt(p);
}

static ast_CompoundStmt* c2_parser_Parser_parseCompoundStmt(c2_parser_Parser* p)
{
   c2_parser_Parser_expectAndConsume(p, token_Kind_LBrace);
   ast_utils_StmtList stmts;
   ast_utils_StmtList_init(&stmts);
   while (p->tok.kind != token_Kind_RBrace) {
      ast_utils_StmtList_add(&stmts, c2_parser_Parser_parseStmt(p));
   }
   src_loc_SrcLoc endLoc = p->tok.loc;
   c2_parser_Parser_expectAndConsume(p, token_Kind_RBrace);
   ast_CompoundStmt* s = ast_builder_Builder_actOnCompoundStmt(p->builder, endLoc, ast_utils_StmtList_getData(&stmts), ast_utils_StmtList_size(&stmts));
   ast_utils_StmtList_free(&stmts);
   return s;
}

static ast_Stmt* c2_parser_Parser_parseAsmStmt(c2_parser_Parser* p)
{
   c2_parser_Parser_consumeToken(p);
   uint32_t quals = c2_parser_Parser_parseOptionalTypeQualifier(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_LParen);
   uint32_t depth = 1;
   while (1) {
      if (p->tok.kind == token_Kind_Eof) break;

      if (p->tok.kind == token_Kind_LParen) depth++;
      if (p->tok.kind == token_Kind_RParen) depth--;
      if (depth == 0) break;

      c2_parser_Parser_consumeToken(p);
   }
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
   return NULL;
}

static ast_Stmt* c2_parser_Parser_parseAssertStmt(c2_parser_Parser* p)
{
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_LParen);
   ast_Expr* inner = c2_parser_Parser_parseExpr(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
   return ast_builder_Builder_actOnAssertStmt(p->builder, inner);
}

static ast_Stmt* c2_parser_Parser_parseBreakStmt(c2_parser_Parser* p)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
   return ast_builder_Builder_actOnBreakStmt(p->builder, loc);
}

static ast_Stmt* c2_parser_Parser_parseContinueStmt(c2_parser_Parser* p)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
   return ast_builder_Builder_actOnContinueStmt(p->builder, loc);
}

static ast_Stmt* c2_parser_Parser_parseFallthroughStmt(c2_parser_Parser* p)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
   return ast_builder_Builder_actOnFallthroughStmt(p->builder, loc);
}

static ast_Stmt* c2_parser_Parser_parseCondition(c2_parser_Parser* p)
{
   c2_parser_Parser_expectAndConsume(p, token_Kind_LParen);
   ast_Stmt* s;
   if (c2_parser_Parser_isDeclaration(p)) {
      s = c2_parser_Parser_parseDeclStmt(p, false, false);
   } else {
      ast_Expr* cond = c2_parser_Parser_parseExpr(p);
      s = ast_Expr_asStmt(cond);
   }
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   return s;
}

static ast_Stmt* c2_parser_Parser_parseIfStmt(c2_parser_Parser* p)
{
   c2_parser_Parser_consumeToken(p);
   ast_Stmt* cond = c2_parser_Parser_parseCondition(p);
   ast_Stmt* then = c2_parser_Parser_parseStmt(p);
   ast_Stmt* else_stmt = NULL;
   if (p->tok.kind == token_Kind_KW_else) {
      c2_parser_Parser_consumeToken(p);
      else_stmt = c2_parser_Parser_parseStmt(p);
   }
   return ast_builder_Builder_actOnIfStmt(p->builder, cond, then, else_stmt);
}

static ast_Stmt* c2_parser_Parser_parseReturnStmt(c2_parser_Parser* p)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   ast_Expr* ret = NULL;
   if (p->tok.kind != token_Kind_Semicolon) {
      ret = c2_parser_Parser_parseExpr(p);
   }
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
   return ast_builder_Builder_actOnReturnStmt(p->builder, loc, ret);
}

static ast_Stmt* c2_parser_Parser_parseSwitchStmt(c2_parser_Parser* p, bool is_sswitch)
{
   c2_parser_Parser_consumeToken(p);
   ast_Stmt* cond = c2_parser_Parser_parseCondition(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_LBrace);
   ast_utils_StmtList cases;
   ast_utils_StmtList_init(&cases);
   while (p->tok.kind != token_Kind_RBrace) {
      ast_Stmt* c = NULL;
      switch (p->tok.kind) {
      case token_Kind_KW_case:
         c = c2_parser_Parser_parseCaseStmt(p);
         break;
      case token_Kind_KW_default:
         c = c2_parser_Parser_parseDefaultStmt(p);
         break;
      default:
         c2_parser_Parser_error(p, "expected case or default");
         break;
      }
      ast_utils_StmtList_add(&cases, c);
   }
   c2_parser_Parser_expectAndConsume(p, token_Kind_RBrace);
   ast_Stmt* s = ast_builder_Builder_actOnSwitchStmt(p->builder, cond, ast_utils_StmtList_getData(&cases), ast_utils_StmtList_size(&cases), is_sswitch);
   ast_utils_StmtList_free(&cases);
   return s;
}

static ast_Stmt* c2_parser_Parser_parseCaseStmt(c2_parser_Parser* p)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   ast_Expr* cond = c2_parser_Parser_parseExpr(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Colon);
   ast_utils_StmtList stmts;
   ast_utils_StmtList_init(&stmts);
   bool more = true;
   while (more) {
      switch (p->tok.kind) {
      case token_Kind_RBrace:
         // fallthrough
      case token_Kind_KW_case:
         // fallthrough
      case token_Kind_KW_default:
         more = false;
         break;
      default:
         ast_utils_StmtList_add(&stmts, c2_parser_Parser_parseStmt(p));
         break;
      }
   }
   ast_Stmt* s = ast_builder_Builder_actOnCaseStmt(p->builder, loc, cond, ast_utils_StmtList_getData(&stmts), ast_utils_StmtList_size(&stmts));
   ast_utils_StmtList_free(&stmts);
   return s;
}

static ast_Stmt* c2_parser_Parser_parseDefaultStmt(c2_parser_Parser* p)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Colon);
   ast_utils_StmtList stmts;
   ast_utils_StmtList_init(&stmts);
   bool more = true;
   while (more) {
      switch (p->tok.kind) {
      case token_Kind_RBrace:
         // fallthrough
      case token_Kind_KW_case:
         // fallthrough
      case token_Kind_KW_default:
         more = false;
         break;
      default:
         ast_utils_StmtList_add(&stmts, c2_parser_Parser_parseStmt(p));
         break;
      }
   }
   ast_Stmt* s = ast_builder_Builder_actOnDefaultStmt(p->builder, loc, ast_utils_StmtList_getData(&stmts), ast_utils_StmtList_size(&stmts));
   ast_utils_StmtList_free(&stmts);
   return s;
}

static ast_Stmt* c2_parser_Parser_parseDoStmt(c2_parser_Parser* p)
{
   c2_parser_Parser_consumeToken(p);
   ast_Stmt* then = c2_parser_Parser_parseStmt(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_KW_while);
   ast_Stmt* cond = c2_parser_Parser_parseCondition(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
   return ast_builder_Builder_actOnDoStmt(p->builder, cond, then);
}

static ast_Stmt* c2_parser_Parser_parseForStmt(c2_parser_Parser* p)
{
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_LParen);
   ast_Stmt* init_ = NULL;
   if (p->tok.kind != token_Kind_Semicolon) {
      if (c2_parser_Parser_isDeclaration(p)) {
         init_ = c2_parser_Parser_parseDeclStmt(p, false, false);
      } else {
         init_ = ast_Expr_asStmt(c2_parser_Parser_parseExpr(p));
      }
   }
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
   ast_Expr* cond = NULL;
   if (p->tok.kind != token_Kind_Semicolon) {
      cond = c2_parser_Parser_parseExpr(p);
   }
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
   ast_Expr* incr = NULL;
   if (p->tok.kind != token_Kind_RParen) {
      incr = c2_parser_Parser_parseExpr(p);
   }
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   ast_Stmt* body = c2_parser_Parser_parseStmt(p);
   return ast_builder_Builder_actOnForStmt(p->builder, init_, cond, incr, body);
}

static ast_Stmt* c2_parser_Parser_parseWhileStmt(c2_parser_Parser* p)
{
   c2_parser_Parser_consumeToken(p);
   ast_Stmt* cond = c2_parser_Parser_parseCondition(p);
   ast_Stmt* then = c2_parser_Parser_parseStmt(p);
   return ast_builder_Builder_actOnWhileStmt(p->builder, cond, then);
}

static ast_Stmt* c2_parser_Parser_parseDeclStmt(c2_parser_Parser* p, bool checkSemi, bool allowLocal)
{
   bool has_local = false;
   if (p->tok.kind == token_Kind_KW_local) {
      has_local = true;
      if (!allowLocal) c2_parser_Parser_error(p, "local is not allowed here");
      c2_parser_Parser_consumeToken(p);
   }
   ast_TypeRefHolder ref;
   ast_TypeRefHolder_init(&ref);
   c2_parser_Parser_parseTypeSpecifier(p, &ref, true, true);
   c2_parser_Parser_expectIdentifier(p);
   uint32_t name = p->tok.text_idx;
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   bool need_semi = true;
   ast_Expr* initValue = NULL;
   if (p->tok.kind == token_Kind_Equal) {
      c2_parser_Parser_consumeToken(p);
      initValue = c2_parser_Parser_parseInitValue(p, &need_semi, false);
   }
   ast_Stmt* s = ast_builder_Builder_actOnVarDeclStmt(p->builder, name, loc, &ref, initValue, has_local);
   if ((checkSemi && need_semi)) {
      c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
   }
   return s;
}

static ast_Stmt* c2_parser_Parser_parseExprStmt(c2_parser_Parser* p)
{
   ast_Expr* e = c2_parser_Parser_parseExpr(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
   return ast_Expr_asStmt(e);
}

static ast_Stmt* c2_parser_Parser_parseLabelStmt(c2_parser_Parser* p)
{
   uint32_t name = p->tok.text_idx;
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Colon);
   return ast_builder_Builder_actOnLabelStmt(p->builder, name, loc);
}

static ast_Stmt* c2_parser_Parser_parseGotoStmt(c2_parser_Parser* p)
{
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectIdentifier(p);
   uint32_t name = p->tok.text_idx;
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Semicolon);
   return ast_builder_Builder_actOnGotoStmt(p->builder, name, loc);
}

static bool c2_parser_Parser_isDeclaration(c2_parser_Parser* p)
{
   const token_Kind kind = p->tok.kind;
   if (kind == token_Kind_Identifier) return c2_parser_Parser_isTypeSpec(p);

   if ((kind >= token_Kind_KW_bool && kind <= token_Kind_KW_void)) return true;

   return false;
}

static bool c2_parser_Parser_builtinArgIsType(c2_parser_Parser* p)
{
   const token_Kind kind = p->tok.kind;
   if ((kind >= token_Kind_KW_bool && kind <= token_Kind_KW_void)) return true;

   uint32_t lookahead = 1;
   while (1) {
      token_Token t2 = c2_tokenizer_Tokenizer_lookahead(&p->tokenizer, lookahead);
      switch (t2.kind) {
      case token_Kind_Identifier:
         break;
      case token_Kind_Star:
         return true;
      case token_Kind_Dot:
         break;
      case token_Kind_LSquare:
         return false;
      default:
         return false;
      }
      lookahead++;
   }
   return false;
}

static ast_Expr* c2_parser_Parser_parseExpr(c2_parser_Parser* p)
{
   ast_Expr* lhs = c2_parser_Parser_parseAssignmentExpression(p);
   return c2_parser_Parser_parseRHSOfBinaryExpression(p, lhs, c2_parser_Prec_Comma);
}

static ast_Expr* c2_parser_Parser_parseAssignmentExpression(c2_parser_Parser* p)
{
   ast_Expr* lhs = c2_parser_Parser_parseCastExpr(p, false, false);
   return c2_parser_Parser_parseRHSOfBinaryExpression(p, lhs, c2_parser_Prec_Assignment);
}

static ast_Expr* c2_parser_Parser_parseRHSOfBinaryExpression(c2_parser_Parser* p, ast_Expr* lhs, c2_parser_Prec minPrec)
{
   c2_parser_Prec nextTokPrec = c2_parser_BinOpPrecLookup[p->tok.kind];
   src_loc_SrcLoc colonLoc = 0;
   while (1) {
      if (nextTokPrec < minPrec) return lhs;

      if (p->tok.kind == token_Kind_Comma) return lhs;

      token_Token opToken = p->tok;
      c2_parser_Parser_consumeToken(p);
      ast_Expr* ternaryMiddle = NULL;
      if (nextTokPrec == c2_parser_Prec_Conditional) {
         if (p->tok.kind == token_Kind_Colon) {
            c2_parser_Parser_error(p, "TODO conditional expr");
         } else {
            ternaryMiddle = c2_parser_Parser_parseExpr(p);
         }
         if (p->tok.kind == token_Kind_Colon) {
            colonLoc = p->tok.loc;
            c2_parser_Parser_consumeToken(p);
         }
      }
      ast_Expr* rhs = c2_parser_Parser_parseCastExpr(p, false, false);
      c2_parser_Prec thisPrec = nextTokPrec;
      nextTokPrec = c2_parser_BinOpPrecLookup[p->tok.kind];
      bool isRightAssoc = ((thisPrec == c2_parser_Prec_Conditional || thisPrec == c2_parser_Prec_Assignment));
      if ((thisPrec < nextTokPrec || ((thisPrec == nextTokPrec && isRightAssoc)))) {
         rhs = c2_parser_Parser_parseRHSOfBinaryExpression(p, rhs, thisPrec + !isRightAssoc);
         nextTokPrec = c2_parser_BinOpPrecLookup[p->tok.kind];
      }
      if (ternaryMiddle) {
         lhs = ast_builder_Builder_actOnConditionalOperator(p->builder, opToken.loc, colonLoc, lhs, ternaryMiddle, rhs);
      } else {
         ast_BinaryOpcode opcode = c2_parser_BinOpTokenLookup[opToken.kind];
         lhs = ast_builder_Builder_actOnBinaryOperator(p->builder, opToken.loc, opcode, lhs, rhs);
      }
   }
   return NULL;
}

static ast_UnaryOpcode c2_parser_convertTokenToUnaryOpcode(token_Kind kind)
{
   switch (kind) {
   case token_Kind_Exclaim:
      return ast_UnaryOpcode_LNot;
   case token_Kind_Star:
      return ast_UnaryOpcode_Deref;
   case token_Kind_Amp:
      return ast_UnaryOpcode_AddrOf;
   case token_Kind_PlusPlus:
      return ast_UnaryOpcode_PreInc;
   case token_Kind_Minus:
      return ast_UnaryOpcode_Minus;
   case token_Kind_MinusMinus:
      return ast_UnaryOpcode_PreDec;
   case token_Kind_Tilde:
      return ast_UnaryOpcode_Not;
   default:
      assert(0);
      break;
   }
   return ast_UnaryOpcode_PreInc;
}

static ast_Expr* c2_parser_Parser_parseCastExpr(c2_parser_Parser* p, bool _arg1, bool _arg2)
{
   token_Kind savedKind = p->tok.kind;
   ast_Expr* res = NULL;
   bool couldBeTemplateCall = false;
   switch (c2_parser_CastExprTokenLookup[savedKind]) {
   case 0:
      c2_parser_Parser_error(p, "syntax error");
      break;
   case 1: {
      token_Token t2 = c2_tokenizer_Tokenizer_lookahead(&p->tokenizer, 1);
      if (t2.kind == token_Kind_Dot) {
         res = c2_parser_Parser_parsePureMemberExpr(p);
      } else {
         res = ast_IdentifierExpr_asExpr(c2_parser_Parser_parseIdentifier(p));
      }
      couldBeTemplateCall = true;
      break;
   }
   case 2:
      res = ast_builder_Builder_actOnIntegerLiteral(p->builder, p->tok.loc, p->tok.radix, p->tok.number_value);
      c2_parser_Parser_consumeToken(p);
      break;
   case 3:
      res = ast_builder_Builder_actOnCharLiteral(p->builder, p->tok.loc, p->tok.char_value);
      c2_parser_Parser_consumeToken(p);
      break;
   case 4:
      res = c2_parser_Parser_parseStringLiteral(p);
      break;
   case 5:
      res = c2_parser_Parser_parseParenExpr(p);
      break;
   case 6: {
      src_loc_SrcLoc loc = p->tok.loc;
      c2_parser_Parser_consumeToken(p);
      res = c2_parser_Parser_parseCastExpr(p, false, false);
      ast_UnaryOpcode opcode = c2_parser_convertTokenToUnaryOpcode(savedKind);
      return ast_builder_Builder_actOnUnaryOperator(p->builder, loc, opcode, res);
   }
   case 7: {
      src_loc_SrcLoc loc = p->tok.loc;
      c2_parser_Parser_consumeToken(p);
      res = c2_parser_Parser_parseCastExpr(p, false, true);
      ast_UnaryOpcode opcode = c2_parser_convertTokenToUnaryOpcode(savedKind);
      return ast_builder_Builder_actOnUnaryOperator(p->builder, loc, opcode, res);
   }
   case 8:
      return c2_parser_Parser_parseExplicitCastExpr(p);
   case 9:
      c2_parser_Parser_consumeToken(p);
      return c2_parser_Parser_parseCastExpr(p, false, false);
   case 10: {
      src_loc_SrcLoc loc = p->tok.loc;
      c2_parser_Parser_consumeToken(p);
      res = c2_parser_Parser_parseCastExpr(p, false, false);
      ast_UnaryOpcode opcode = c2_parser_convertTokenToUnaryOpcode(savedKind);
      return ast_builder_Builder_actOnUnaryOperator(p->builder, loc, opcode, res);
   }
   case 11:
      res = c2_parser_Parser_parseElemsof(p);
      break;
   case 12:
      return c2_parser_Parser_parseEnumMinMax(p, savedKind == token_Kind_KW_enum_min);
   case 13:
      res = ast_builder_Builder_actOnBooleanConstant(p->builder, p->tok.loc, savedKind == token_Kind_KW_true);
      c2_parser_Parser_consumeToken(p);
      break;
   case 14:
      res = ast_builder_Builder_actOnNilExpr(p->builder, p->tok.loc);
      c2_parser_Parser_consumeToken(p);
      break;
   case 15:
      return c2_parser_Parser_parseOffsetOfExpr(p);
   case 16:
      return c2_parser_Parser_parseSizeof(p);
   case 17:
      return c2_parser_Parser_parseToContainerExpr(p);
   }
   return c2_parser_Parser_parsePostfixExprSuffix(p, res, couldBeTemplateCall);
}

static ast_Expr* c2_parser_Parser_parsePostfixExprSuffix(c2_parser_Parser* p, ast_Expr* lhs, bool couldBeTemplateCall)
{
   while (1) {
      switch (p->tok.kind) {
      case token_Kind_Identifier:
         return lhs;
      case token_Kind_LParen:
         lhs = c2_parser_Parser_parseCallExpr(p, lhs);
         break;
      case token_Kind_LSquare: {
         c2_parser_Parser_consumeToken(p);
         ast_Expr* idx = c2_parser_Parser_parseExpr(p);
         if (p->tok.kind == token_Kind_Colon) {
            src_loc_SrcLoc colLoc = p->tok.loc;
            c2_parser_Parser_consumeToken(p);
            ast_Expr* rhs = c2_parser_Parser_parseExpr(p);
            idx = ast_builder_Builder_actOnBitOffsetExpr(p->builder, colLoc, idx, rhs);
         }
         src_loc_SrcLoc rloc = p->tok.loc;
         c2_parser_Parser_expectAndConsume(p, token_Kind_RSquare);
         lhs = ast_builder_Builder_actOnArraySubscriptExpr(p->builder, rloc, lhs, idx);
         break;
      }
      case token_Kind_Dot:
         lhs = c2_parser_Parser_parseImpureMemberExpr(p, lhs);
         break;
      case token_Kind_PlusPlus:
         // fallthrough
      case token_Kind_MinusMinus:
         lhs = ast_builder_Builder_actOnPostFixUnaryOperator(p->builder, p->tok.loc, p->tok.kind, lhs);
         c2_parser_Parser_consumeToken(p);
         break;
      case token_Kind_Less:
         if ((couldBeTemplateCall && c2_parser_Parser_isTemplateFunctionCall(p))) {
            c2_parser_Parser_consumeToken(p);
            ast_TypeRefHolder ref;
            ast_TypeRefHolder_init(&ref);
            c2_parser_Parser_parseTypeSpecifier(p, &ref, false, false);
            c2_parser_Parser_expectAndConsume(p, token_Kind_Greater);
            lhs = c2_parser_Parser_parseTemplateCallExpr(p, lhs, &ref);
         }
         return lhs;
      default:
         return lhs;
      }
   }
   assert(0);
   return NULL;
}

static ast_Expr* c2_parser_Parser_parseCallExpr(c2_parser_Parser* p, ast_Expr* fn)
{
   c2_parser_Parser_consumeToken(p);
   ast_ExprList args;
   ast_ExprList_init(&args, 4);
   while (p->tok.kind != token_Kind_RParen) {
      ast_ExprList_add(&args, c2_parser_Parser_parseExpr(p));
      if (p->tok.kind == token_Kind_RParen) break;

      c2_parser_Parser_expectAndConsume(p, token_Kind_Comma);
   }
   src_loc_SrcLoc endLoc = p->tok.loc;
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   ast_Expr* res = ast_builder_Builder_actOnCallExpr(p->builder, endLoc, fn, ast_ExprList_getExprs(&args), ast_ExprList_size(&args));
   ast_ExprList_free(&args);
   return res;
}

static ast_Expr* c2_parser_Parser_parseTemplateCallExpr(c2_parser_Parser* p, ast_Expr* fn, const ast_TypeRefHolder* ref)
{
   c2_parser_Parser_consumeToken(p);
   ast_ExprList args;
   ast_ExprList_init(&args, 4);
   while (p->tok.kind != token_Kind_RParen) {
      ast_ExprList_add(&args, c2_parser_Parser_parseExpr(p));
      if (p->tok.kind == token_Kind_RParen) break;

      c2_parser_Parser_expectAndConsume(p, token_Kind_Comma);
   }
   src_loc_SrcLoc endLoc = p->tok.loc;
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   ast_Expr* res = ast_builder_Builder_actOnTemplateCallExpr(p->builder, endLoc, fn, ast_ExprList_getExprs(&args), ast_ExprList_size(&args), ref);
   ast_ExprList_free(&args);
   return res;
}

static ast_Expr* c2_parser_Parser_parseImpureMemberExpr(c2_parser_Parser* p, ast_Expr* base)
{
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectIdentifier(p);
   ast_Ref ref[7];
   ref[0].loc = p->tok.loc;
   ref[0].name_idx = p->tok.text_idx;
   uint32_t refcount = 1;
   c2_parser_Parser_consumeToken(p);
   while (p->tok.kind == token_Kind_Dot) {
      c2_parser_Parser_consumeToken(p);
      c2_parser_Parser_expectIdentifier(p);
      if (refcount == ARRAY_SIZE(ref)) c2_parser_Parser_error(p, "max member depth is %u", ast_MemberExprMaxDepth);
      ref[refcount].loc = p->tok.loc;
      ref[refcount].name_idx = p->tok.text_idx;
      refcount++;
      c2_parser_Parser_consumeToken(p);
   }
   return ast_builder_Builder_actOnMemberExpr(p->builder, base, ref, refcount);
}

static ast_Expr* c2_parser_Parser_parsePureMemberExpr(c2_parser_Parser* p)
{
   ast_Ref ref[7];
   ref[0].loc = p->tok.loc;
   ref[0].name_idx = p->tok.text_idx;
   uint32_t refcount = 1;
   c2_parser_Parser_consumeToken(p);
   while (p->tok.kind == token_Kind_Dot) {
      c2_parser_Parser_consumeToken(p);
      c2_parser_Parser_expectIdentifier(p);
      if (refcount == ARRAY_SIZE(ref)) c2_parser_Parser_error(p, "max member depth is %u", ast_MemberExprMaxDepth);
      ref[refcount].loc = p->tok.loc;
      ref[refcount].name_idx = p->tok.text_idx;
      refcount++;
      c2_parser_Parser_consumeToken(p);
   }
   return ast_builder_Builder_actOnMemberExpr(p->builder, NULL, ref, refcount);
}

static ast_IdentifierExpr* c2_parser_Parser_parseIdentifier(c2_parser_Parser* p)
{
   ast_IdentifierExpr* e = ast_builder_Builder_actOnIdentifier(p->builder, p->tok.loc, p->tok.text_idx);
   c2_parser_Parser_consumeToken(p);
   return e;
}

static ast_Expr* c2_parser_Parser_parseStringLiteral(c2_parser_Parser* p)
{
   ast_Expr* e = ast_builder_Builder_actOnStringLiteral(p->builder, p->tok.loc, p->tok.text_idx, p->tok.text_len);
   c2_parser_Parser_consumeToken(p);
   return e;
}

static ast_Expr* c2_parser_Parser_parseParenExpr(c2_parser_Parser* p)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   ast_Expr* res = c2_parser_Parser_parseExpr(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   return ast_builder_Builder_actOnParenExpr(p->builder, loc, res);
}

static bool c2_parser_Parser_isTemplateFunctionCall(c2_parser_Parser* p)
{
   assert(p->tok.kind == token_Kind_Less);
   uint32_t ahead = 1;
   token_Token t = c2_tokenizer_Tokenizer_lookahead(&p->tokenizer, ahead);
   if ((t.kind >= token_Kind_KW_bool && t.kind <= token_Kind_KW_void)) return true;

   while (ahead < 8) {
      t = c2_tokenizer_Tokenizer_lookahead(&p->tokenizer, ahead);
      switch (t.kind) {
      case token_Kind_Identifier:
         // fallthrough
      case token_Kind_Star:
         // fallthrough
      case token_Kind_Dot:
         break;
      case token_Kind_Greater:
         t = c2_tokenizer_Tokenizer_lookahead(&p->tokenizer, ahead + 1);
         return t.kind == token_Kind_LParen;
      case token_Kind_KW_const:
         return true;
      default:
         return false;
      }
      ahead++;
   }
   return false;
}

static bool c2_parser_couldBeType(token_Token tok)
{
   if (tok.kind != token_Kind_Identifier) return false;

   const char* name = ast_idx2name(tok.text_idx);
   return isupper(name[0]) != 0;
}

static ast_Expr* c2_parser_Parser_parseSizeof(c2_parser_Parser* p)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_LParen);
   ast_Expr* res = NULL;
   if ((c2_parser_Parser_builtinArgIsType(p) || c2_parser_couldBeType(p->tok))) {
      ast_TypeRefHolder ref;
      ast_TypeRefHolder_init(&ref);
      c2_parser_Parser_parseTypeSpecifier(p, &ref, false, true);
      res = ast_builder_Builder_actOnTypeExpr(p->builder, p->tok.loc, &ref);
   } else {
      if (p->tok.kind != token_Kind_Identifier) {
         c2_parser_Parser_error(p, "expect a type or variable name");
      }
      res = c2_parser_Parser_parseFullIdentifier(p);
   }
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   return ast_builder_Builder_actOnBuiltinExpr(p->builder, loc, res, ast_BuiltinExprKind_Sizeof);
}

static ast_Expr* c2_parser_Parser_parseElemsof(c2_parser_Parser* p)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_LParen);
   c2_parser_Parser_expectIdentifier(p);
   ast_Expr* res = ast_IdentifierExpr_asExpr(c2_parser_Parser_parseIdentifier(p));
   res = c2_parser_Parser_parsePostfixExprSuffix(p, res, false);
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   return ast_builder_Builder_actOnBuiltinExpr(p->builder, loc, res, ast_BuiltinExprKind_Elemsof);
}

static ast_Expr* c2_parser_Parser_parseInitValue(c2_parser_Parser* p, bool* need_semi, bool allow_designators)
{
   switch (p->tok.kind) {
   case token_Kind_LBrace:
      *need_semi = false;
      return c2_parser_Parser_parseInitList(p);
   case token_Kind_Dot:
      if (!allow_designators) c2_parser_Parser_error(p, "designator not allowed here");
      return c2_parser_Parser_parseFieldDesignator(p, need_semi);
   case token_Kind_LSquare:
      if (!allow_designators) c2_parser_Parser_error(p, "designator not allowed here");
      return c2_parser_Parser_parseArrayDesignator(p, need_semi);
   default:
      break;
   }
   *need_semi = true;
   return c2_parser_Parser_parseAssignmentExpression(p);
}

static ast_Expr* c2_parser_Parser_parseInitList(c2_parser_Parser* p)
{
   src_loc_SrcLoc left = p->tok.loc;
   c2_parser_Parser_expectAndConsume(p, token_Kind_LBrace);
   ast_ExprList values;
   ast_ExprList_init(&values, 8);
   while (p->tok.kind != token_Kind_RBrace) {
      bool unused;
      ast_Expr* e = c2_parser_Parser_parseInitValue(p, &unused, true);
      ast_ExprList_add(&values, e);
      if (p->tok.kind == token_Kind_Comma) {
         c2_parser_Parser_consumeToken(p);
      } else {
         break;
      }
   }
   src_loc_SrcLoc right = p->tok.loc;
   c2_parser_Parser_expectAndConsume(p, token_Kind_RBrace);
   ast_Expr* e = ast_builder_Builder_actOnInitList(p->builder, left, right, ast_ExprList_getExprs(&values), ast_ExprList_size(&values));
   ast_ExprList_free(&values);
   return e;
}

static ast_Expr* c2_parser_Parser_parseFieldDesignator(c2_parser_Parser* p, bool* need_semi)
{
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectIdentifier(p);
   uint32_t field = p->tok.text_idx;
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Equal);
   ast_Expr* value = c2_parser_Parser_parseInitValue(p, need_semi, false);
   return ast_builder_Builder_actOnFieldDesignatedInit(p->builder, field, loc, value);
}

static ast_Expr* c2_parser_Parser_parseArrayDesignator(c2_parser_Parser* p, bool* need_semi)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   ast_Expr* designator = c2_parser_Parser_parseAssignmentExpression(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_RSquare);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Equal);
   ast_Expr* initValue = c2_parser_Parser_parseInitValue(p, need_semi, false);
   return ast_builder_Builder_actOnArrayDesignatedInit(p->builder, loc, designator, initValue);
}

static ast_Expr* c2_parser_Parser_parseExplicitCastExpr(c2_parser_Parser* p)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Less);
   ast_TypeRefHolder ref;
   ast_TypeRefHolder_init(&ref);
   c2_parser_Parser_parseTypeSpecifier(p, &ref, true, false);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Greater);
   c2_parser_Parser_expectAndConsume(p, token_Kind_LParen);
   ast_Expr* expr = c2_parser_Parser_parseExpr(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   return ast_builder_Builder_actOnExplicitCast(p->builder, loc, &ref, expr);
}

static ast_Expr* c2_parser_Parser_parseEnumMinMax(c2_parser_Parser* p, bool is_min)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_LParen);
   c2_parser_Parser_expectIdentifier(p);
   ast_Expr* expr = c2_parser_Parser_parseExpr(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   return ast_builder_Builder_actOnBuiltinExpr(p->builder, loc, expr, is_min ? ast_BuiltinExprKind_EnumMin : ast_BuiltinExprKind_EnumMax);
}

static ast_Expr* c2_parser_Parser_parseOffsetOfExpr(c2_parser_Parser* p)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_LParen);
   ast_Expr* structExpr = c2_parser_Parser_parseFullIdentifier(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Comma);
   ast_Expr* member = c2_parser_Parser_parseFullIdentifier(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   return ast_builder_Builder_actOnOffsetOfExpr(p->builder, loc, structExpr, member);
}

static ast_Expr* c2_parser_Parser_parseToContainerExpr(c2_parser_Parser* p)
{
   src_loc_SrcLoc loc = p->tok.loc;
   c2_parser_Parser_consumeToken(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_LParen);
   ast_Expr* structExpr = c2_parser_Parser_parseFullIdentifier(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Comma);
   ast_Expr* member = c2_parser_Parser_parseFullIdentifier(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_Comma);
   ast_Expr* pointer = c2_parser_Parser_parseExpr(p);
   c2_parser_Parser_expectAndConsume(p, token_Kind_RParen);
   return ast_builder_Builder_actOnToContainerExpr(p->builder, loc, structExpr, member, pointer);
}

static ast_Expr* c2_parser_Parser_parseFullIdentifier(c2_parser_Parser* p)
{
   c2_parser_Parser_expectIdentifier(p);
   token_Token t2 = c2_tokenizer_Tokenizer_lookahead(&p->tokenizer, 1);
   if (t2.kind == token_Kind_Dot) {
      return c2_parser_Parser_parsePureMemberExpr(p);
   }
   return ast_IdentifierExpr_asExpr(c2_parser_Parser_parseIdentifier(p));
}


// --- module module_analyser ---

typedef struct module_analyser_Analyser_ module_analyser_Analyser;
typedef struct module_analyser_NameVector_ module_analyser_NameVector;
typedef struct module_analyser_StackLayer_ module_analyser_StackLayer;
typedef struct module_analyser_MainMarker_ module_analyser_MainMarker;




struct module_analyser_NameVector_ {
   uint32_t* data;
   uint32_t count;
   uint32_t capacity;
};

struct module_analyser_StackLayer_ {
   ast_Decl* decl;
   scope_Scope* scope;
   ast_FunctionDecl* function;
};

struct module_analyser_Analyser_ {
   diagnostics_Diags* diags;
   conversion_checker_Checker checker;
   ast_context_Context* context;
   string_pool_Pool* astPool;
   ast_builder_Builder* builder;
   module_list_ModList* allmodules;
   const warning_flags_Flags* warnings;
   ast_Module* mod;
   uint32_t prefix_cache_name;
   uint32_t prefix_cache_idx;
   module_analyser_NameVector prefixes;
   struct_func_list_List* struct_decls;
   module_analyser_StackLayer checkStack[8];
   uint32_t checkIndex;
   scope_Scope* scope;
   ast_FunctionDecl* curFunction;
   bool has_error;
};

struct module_analyser_MainMarker_ {
   uint32_t name_idx;
   ast_Decl* main;
};

static module_analyser_Analyser* module_analyser_create(diagnostics_Diags* diags, ast_context_Context* context, string_pool_Pool* astPool, ast_builder_Builder* builder, module_list_ModList* allmodules, const warning_flags_Flags* warnings);
static void module_analyser_Analyser_free(module_analyser_Analyser* ma);
static void module_analyser_Analyser_check(module_analyser_Analyser* ma, ast_Module* mod);
static void module_analyser_NameVector_init(module_analyser_NameVector* v, uint32_t capacity);
static void module_analyser_NameVector_free(module_analyser_NameVector* v);
static void module_analyser_NameVector_resize(module_analyser_NameVector* v);
static uint32_t module_analyser_NameVector_add(module_analyser_NameVector* v, uint32_t name_idx);
static uint32_t module_analyser_NameVector_get(const module_analyser_NameVector* v, uint32_t idx);
static bool module_analyser_NameVector_find(module_analyser_NameVector* v, uint32_t name_idx, uint32_t* index);
static void module_analyser_Analyser_note(module_analyser_Analyser* ma, src_loc_SrcLoc loc, const char* format, ...);
static void module_analyser_Analyser_warn(module_analyser_Analyser* ma, src_loc_SrcLoc loc, const char* format, ...);
static void module_analyser_Analyser_error(module_analyser_Analyser* ma, src_loc_SrcLoc loc, const char* format, ...);
static void module_analyser_Analyser_errorRange(module_analyser_Analyser* ma, src_loc_SrcLoc loc, src_loc_SrcRange range, const char* format, ...);
static void module_analyser_Analyser_createGlobalScope(void* arg, ast_AST* a);
static void module_analyser_Analyser_deleteScope(void* _arg0, ast_AST* a);
static void module_analyser_Analyser_handleStructFunc(void* arg, ast_FunctionDecl* d);
static void module_analyser_Analyser_analyseFunctionProto(void* arg, ast_FunctionDecl* d);
static void module_analyser_Analyser_analyseFunctionBodies(void* arg, ast_FunctionDecl* d);
static void module_analyser_Analyser_analyseGlobalDecl(module_analyser_Analyser* ma, ast_Decl* d);
static void module_analyser_Analyser_handleTypeDecl(void* arg, ast_Decl* d);
static void module_analyser_Analyser_handleStaticAssert(void* arg, ast_Decl* d);
static void module_analyser_Analyser_handleVarDecl(void* arg, ast_VarDecl* v);
static void module_analyser_Analyser_checkName(module_analyser_Analyser* ma, ast_Decl* d, bool is_constant);
static void module_analyser_Analyser_analyseGlobalVarDecl(module_analyser_Analyser* ma, ast_VarDecl* v);
static bool module_analyser_Analyser_analyseInitExpr(module_analyser_Analyser* ma, ast_Expr** e_ptr, ast_QualType expectedType);
static bool module_analyser_Analyser_pushCheck(module_analyser_Analyser* ma, ast_Decl* d, scope_Scope* s, ast_FunctionDecl* fd);
static void module_analyser_Analyser_popCheck(module_analyser_Analyser* ma);
static bool module_analyser_Analyser_analyseInitListExpr(module_analyser_Analyser* ma, ast_InitListExpr* ile, ast_QualType expectedType);
static bool module_analyser_Analyser_analyseInitListArray(module_analyser_Analyser* ma, ast_InitListExpr* ile, ast_QualType expectedType);
static bool module_analyser_Analyser_analyseInitListStruct(module_analyser_Analyser* ma, ast_InitListExpr* ile, ast_QualType expectedType);
static void module_analyser_Analyser_analyseStructType(module_analyser_Analyser* ma, ast_StructTypeDecl* d);
static void module_analyser_Analyser_analyseStructMembers(module_analyser_Analyser* ma, ast_StructTypeDecl* d);
static void module_analyser_Analyser_analyseStructNames(module_analyser_Analyser* ma, ast_StructTypeDecl* d, module_analyser_NameVector* names, module_analyser_NameVector* locs);
static void module_analyser_Analyser_analyseFunctionType(module_analyser_Analyser* ma, ast_Decl* d);
static void module_analyser_Analyser_analyseFunction(module_analyser_Analyser* ma, ast_FunctionDecl* fd);
static void module_analyser_Analyser_analyseAliasType(module_analyser_Analyser* ma, ast_AliasTypeDecl* d);
static void module_analyser_Analyser_analyseEnumType(module_analyser_Analyser* ma, ast_EnumTypeDecl* d);
static void module_analyser_Analyser_analyseStructMember(module_analyser_Analyser* ma, ast_VarDecl* v);
static ast_QualType module_analyser_Analyser_analyseUserTypeRef(module_analyser_Analyser* ma, ast_TypeRef* ref);
static ast_QualType module_analyser_Analyser_analyseTypeRef(module_analyser_Analyser* ma, ast_TypeRef* ref);
static bool module_analyser_Analyser_checkOpaque(module_analyser_Analyser* ma, const ast_StructTypeDecl* std, src_loc_SrcLoc loc);
static void module_analyser_markMainUsed(void* arg, ast_FunctionDecl* fd);
static ast_Decl* module_analyser_Analyser_checkMain(module_analyser_Analyser* ma, bool test_mode, ast_Module* top, uint32_t name_idx);
static ast_QualType module_analyser_getPointerFromArray(ast_builder_Builder* builder, ast_QualType q);
static ast_QualType module_analyser_Analyser_analyseExpr(module_analyser_Analyser* ma, ast_Expr** e_ptr, bool need_rvalue);
static ast_QualType module_analyser_Analyser_analyseExprInner(module_analyser_Analyser* ma, ast_Expr** e_ptr);
static ast_Decl* module_analyser_Analyser_analyseIdentifier(module_analyser_Analyser* ma, ast_Expr** e_ptr);
static void module_analyser_create_template_name(char* name, const char* orig, uint16_t idx);
static ast_FunctionDecl* module_analyser_Analyser_instantiateTemplateFunction(module_analyser_Analyser* ma, ast_CallExpr* call, ast_FunctionDecl* fd);
static ast_QualType module_analyser_Analyser_analyseCallExpr(module_analyser_Analyser* ma, ast_Expr** e_ptr);
static ast_IdentifierKind module_analyser_Analyser_setExprFlags(module_analyser_Analyser* ma, ast_Expr** e_ptr, ast_Decl* d);
static bool module_analyser_Analyser_analyseArrayDesignatedInit(module_analyser_Analyser* ma, ast_Expr* e, ast_QualType expectedType);
static ast_IdentifierKind module_analyser_getInnerExprAddressOf(ast_Expr* e);
static bool module_analyser_Analyser_getIdentifierKind(module_analyser_Analyser* ma, ast_Expr* e);
static ast_QualType module_analyser_Analyser_analyseUnaryOperator(module_analyser_Analyser* ma, ast_Expr** e_ptr);
static ast_QualType module_analyser_Analyser_analyseConditionalOperator(module_analyser_Analyser* ma, ast_Expr** e_ptr);
static bool module_analyser_Analyser_checkAssignment(module_analyser_Analyser* ma, ast_Expr* assignee, ast_QualType tleft, const char* msg, src_loc_SrcLoc loc);
static ast_QualType module_analyser_usualUnaryConversions(ast_Expr* e);
static ast_QualType module_analyser_Analyser_analyseMemberExpr(module_analyser_Analyser* ma, ast_Expr** e_ptr);
static ast_Decl* module_analyser_Analyser_analyseMemberBase(module_analyser_Analyser* ma, src_loc_SrcLoc loc, ast_QualType baseType, bool ptr_deref);
static ast_QualType module_analyser_Analyser_analyseExplicitCast(module_analyser_Analyser* ma, ast_Expr** e_ptr);
static ast_QualType module_analyser_Analyser_analyseBuiltin(module_analyser_Analyser* ma, ast_Expr** e_ptr);
static ast_QualType module_analyser_Analyser_analyseArraySubscriptExpr(module_analyser_Analyser* ma, ast_Expr** e_ptr);
static ast_QualType module_analyser_Analyser_analyseEnumMinMax(module_analyser_Analyser* ma, ast_BuiltinExpr* b);
static uint32_t module_analyser_decl2offset(const ast_Decl* d);
static ast_Decl* module_analyser_Analyser_findMemberOffset(module_analyser_Analyser* ma, ast_BuiltinExpr* b, ast_StructTypeDecl* std, ast_Expr* member);
static ast_QualType module_analyser_Analyser_analyseOffsetOf(module_analyser_Analyser* ma, ast_BuiltinExpr* b);
static ast_QualType module_analyser_Analyser_analyseToContainer(module_analyser_Analyser* ma, ast_BuiltinExpr* b);
static ast_QualType module_analyser_Analyser_analyseSizeof(module_analyser_Analyser* ma, ast_BuiltinExpr* e);
static ast_QualType module_analyser_Analyser_analyseElemsof(module_analyser_Analyser* ma, ast_BuiltinExpr* b);
static ast_Decl* module_analyser_Analyser_findStructMember(module_analyser_Analyser* ma, ast_StructTypeDecl* s, uint32_t name_idx, src_loc_SrcLoc loc, bool allow_funcs);
static ast_Decl* module_analyser_Analyser_findStructMemberOffset(module_analyser_Analyser* ma, ast_StructTypeDecl* s, uint32_t name_idx, src_loc_SrcLoc loc, uint32_t* base);
static bool module_analyser_validBinOpKind(ast_QualType t);
static ast_QualType module_analyser_Analyser_checkBinopIntArgs(module_analyser_Analyser* ma, ast_BinaryOperator* b, ast_QualType lhs, ast_QualType rhs);
static ast_QualType module_analyser_Analyser_checkBinopLogical(module_analyser_Analyser* ma, ast_BinaryOperator* b, ast_QualType lhs, ast_QualType rhs);
static ast_QualType module_analyser_Analyser_checkBinopAddSubAssign(module_analyser_Analyser* ma, ast_BinaryOperator* b, ast_QualType lhs, ast_QualType rhs);
static ast_QualType module_analyser_Analyser_checkBinopAddArgs(module_analyser_Analyser* ma, ast_BinaryOperator* b, ast_QualType lhs, ast_QualType rhs);
static ast_QualType module_analyser_Analyser_checkBinopSubArgs(module_analyser_Analyser* ma, ast_BinaryOperator* b, ast_QualType lhs, ast_QualType rhs);
static ast_QualType module_analyser_Analyser_checkBinopComparison(module_analyser_Analyser* ma, ast_BinaryOperator* b, ast_QualType lhs, ast_QualType rhs);
static ast_QualType module_analyser_Analyser_analyseBinaryOperator(module_analyser_Analyser* ma, ast_Expr** e_ptr);
static void module_analyser_Analyser_analyseFunctionBody(module_analyser_Analyser* ma, ast_FunctionDecl* fd, scope_Scope* s);
static void module_analyser_Analyser_analyseStmt(module_analyser_Analyser* ma, ast_Stmt* s, bool checkEffect);
static void module_analyser_Analyser_analyseSwitchStmt(module_analyser_Analyser* ma, ast_Stmt* s);
static bool module_analyser_Analyser_analyseCaseStmt(module_analyser_Analyser* ma, ast_Stmt* s, ast_EnumTypeDecl* etd, bool lastCase, bool is_sswitch);
static bool module_analyser_Analyser_analyseDefaultStmt(module_analyser_Analyser* ma, ast_Stmt* s, bool is_sswitch);
static void module_analyser_Analyser_analyseBreakStmt(module_analyser_Analyser* ma, ast_Stmt* s);
static void module_analyser_Analyser_analyseContinueStmt(module_analyser_Analyser* ma, ast_Stmt* s);
static void module_analyser_Analyser_analyseFallthroughStmt(module_analyser_Analyser* ma, ast_Stmt* s);
static void module_analyser_Analyser_analyseCompoundStmt(module_analyser_Analyser* ma, ast_CompoundStmt* c);
static ast_QualType module_analyser_Analyser_analyseCondition(module_analyser_Analyser* ma, ast_Stmt** s_ptr);
static void module_analyser_Analyser_analyseIfStmt(module_analyser_Analyser* ma, ast_Stmt* s);
static void module_analyser_Analyser_analyseForStmt(module_analyser_Analyser* ma, ast_Stmt* s);
static void module_analyser_Analyser_analyseWhileStmt(module_analyser_Analyser* ma, ast_Stmt* s);
static void module_analyser_Analyser_analyseDoStmt(module_analyser_Analyser* ma, ast_Stmt* s);
static ast_QualType module_analyser_Analyser_analyseDeclStmt(module_analyser_Analyser* ma, ast_Stmt* s);
static void module_analyser_Analyser_analyseAssertStmt(module_analyser_Analyser* ma, ast_Stmt* s);
static void module_analyser_Analyser_analyseReturnStmt(module_analyser_Analyser* ma, ast_Stmt* s);

static const uint32_t module_analyser_MaxDepth = 8;

static const uint8_t module_analyser_CondOpTable[8][8] = {
   {
   2,
   1,
   0,
   1,
   3,
   0,
   0,
   0
},
   {
   1,
   4,
   0,
   1,
   1,
   1,
   0,
   0
},
   {
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0
},
   {
   1,
   1,
   0,
   5,
   1,
   1,
   0,
   0
},
   {
   6,
   1,
   0,
   1,
   7,
   1,
   0,
   0
},
   {
   0,
   0,
   0,
   1,
   0,
   0,
   0,
   0
}
};

static const uint8_t module_analyser_BinOpConvAddSubAss[8][8] = {
   {
   2,
   1,
   0,
   1,
   3,
   0,
   0,
   0
},
   {
   4,
   1,
   0,
   1,
   4,
   1,
   0,
   0
},
   {
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0
},
   {
   1,
   1,
   0,
   1,
   1,
   1,
   0,
   0
},
   {
   5,
   1,
   0,
   1,
   6,
   1,
   0,
   0
},
   {
   0,
   0,
   0,
   1,
   0,
   0,
   0,
   0
}
};

static const uint8_t module_analyser_BinOpConvAdd[8][8] = {
   {
   2,
   3,
   0,
   1,
   5,
   0,
   0,
   0
},
   {
   4,
   1,
   0,
   1,
   4,
   1,
   0,
   0
},
   {
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0
},
   {
   1,
   1,
   0,
   1,
   1,
   1,
   0,
   0
},
   {
   6,
   3,
   0,
   1,
   7,
   1,
   0,
   0
},
   {
   0,
   0,
   0,
   1,
   0,
   0,
   0,
   0
}
};

static const uint8_t module_analyser_BinOpConvSub[8][8] = {
   {
   2,
   1,
   0,
   1,
   2,
   0,
   0,
   0
},
   {
   4,
   5,
   0,
   1,
   4,
   6,
   0,
   0
},
   {
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0
},
   {
   1,
   1,
   0,
   1,
   1,
   1,
   0,
   0
},
   {
   6,
   1,
   0,
   1,
   7,
   1,
   0,
   0
},
   {
   0,
   0,
   0,
   1,
   0,
   0,
   0,
   0
}
};

static const uint8_t module_analyser_BinOpConvComparision[8][8] = {
   {
   2,
   1,
   0,
   1,
   4,
   0,
   0,
   0
},
   {
   1,
   3,
   0,
   1,
   1,
   1,
   0,
   0
},
   {
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0
},
   {
   1,
   1,
   0,
   1,
   1,
   1,
   0,
   0
},
   {
   5,
   1,
   0,
   1,
   6,
   1,
   0,
   0
},
   {
   0,
   0,
   0,
   1,
   0,
   0,
   0,
   0
}
};

static module_analyser_Analyser* module_analyser_create(diagnostics_Diags* diags, ast_context_Context* context, string_pool_Pool* astPool, ast_builder_Builder* builder, module_list_ModList* allmodules, const warning_flags_Flags* warnings)
{
   module_analyser_Analyser* ma = calloc(1, sizeof(module_analyser_Analyser));
   ma->diags = diags;
   conversion_checker_Checker_init(&ma->checker, diags, builder);
   ma->context = context;
   ma->astPool = astPool;
   ma->builder = builder;
   ma->allmodules = allmodules;
   ma->warnings = warnings;
   return ma;
}

static void module_analyser_Analyser_free(module_analyser_Analyser* ma)
{
   module_analyser_NameVector_free(&ma->prefixes);
   free(ma);
}

static void module_analyser_Analyser_check(module_analyser_Analyser* ma, ast_Module* mod)
{
   ma->mod = mod;
   ma->prefix_cache_name = 0;
   ma->prefix_cache_idx = 0;
   module_analyser_NameVector_free(&ma->prefixes);
   ma->checkIndex = 0;
   ma->scope = NULL;
   ma->curFunction = NULL;
   ma->has_error = false;
   ast_Module_visitASTs(mod, module_analyser_Analyser_createGlobalScope, ma);
   struct_func_list_List struct_decls = { };
   ma->struct_decls = &struct_decls;
   ast_Module_visitStructFunctions(mod, module_analyser_Analyser_handleStructFunc, ma);
   for (uint32_t i = 0; i < struct_decls.count; i++) {
      const struct_func_list_Info* info = &struct_decls.data[i];
      ast_StructTypeDecl* fd = ((ast_StructTypeDecl*)(info->decl));
      ast_StructTypeDecl_setStructFunctions(fd, ma->context, ast_FunctionDeclList_getDecls(&info->functions), ast_FunctionDeclList_size(&info->functions));
   }
   module_analyser_NameVector_free(&ma->prefixes);
   struct_func_list_List_free(&struct_decls);
   ma->struct_decls = NULL;
   ast_Module_visitTypeDecls(mod, module_analyser_Analyser_handleTypeDecl, ma);
   ast_Module_visitVarDecls(mod, module_analyser_Analyser_handleVarDecl, ma);
   ast_Module_visitStaticAsserts(mod, module_analyser_Analyser_handleStaticAssert, ma);
   ast_Module_visitFunctions(mod, module_analyser_Analyser_analyseFunctionProto, ma);
   if (ma->has_error) return;

   ast_Module_visitFunctions(mod, module_analyser_Analyser_analyseFunctionBodies, ma);
   ast_Module_visitASTs(mod, module_analyser_Analyser_deleteScope, ma);
}

static void module_analyser_NameVector_init(module_analyser_NameVector* v, uint32_t capacity)
{
   v->data = NULL;
   v->count = 0;
   v->capacity = capacity / 2;
   module_analyser_NameVector_resize(v);
}

static void module_analyser_NameVector_free(module_analyser_NameVector* v)
{
   free(v->data);
   v->count = 0;
   v->capacity = 0;
   v->data = NULL;
}

static void module_analyser_NameVector_resize(module_analyser_NameVector* v)
{
   v->capacity = v->capacity == 0 ? 4 : v->capacity * 2;
   void* data2 = malloc(v->capacity * sizeof(uint32_t));
   if (v->data) {
      memcpy(data2, v->data, v->count * sizeof(uint32_t));
      free(v->data);
   }
   v->data = data2;
}

static uint32_t module_analyser_NameVector_add(module_analyser_NameVector* v, uint32_t name_idx)
{
   if (v->count == v->capacity) module_analyser_NameVector_resize(v);
   uint32_t index = v->count;
   v->data[index] = name_idx;
   v->count++;
   return index;
}

static uint32_t module_analyser_NameVector_get(const module_analyser_NameVector* v, uint32_t idx)
{
   return v->data[idx];
}

static bool module_analyser_NameVector_find(module_analyser_NameVector* v, uint32_t name_idx, uint32_t* index)
{
   for (uint32_t i = 0; i < v->count; i++) {
      if (v->data[i] == name_idx) {
         *index = i;
         return true;
      }
   }
   return false;
}

static void module_analyser_Analyser_note(module_analyser_Analyser* ma, src_loc_SrcLoc loc, const char* format, ...)
{
   va_list args;
   va_start(args, format);
   diagnostics_Diags_note2(ma->diags, loc, format, args);
   va_end(args);
}

static void module_analyser_Analyser_warn(module_analyser_Analyser* ma, src_loc_SrcLoc loc, const char* format, ...)
{
   va_list args;
   va_start(args, format);
   diagnostics_Diags_warn2(ma->diags, loc, format, args);
   va_end(args);
}

static void module_analyser_Analyser_error(module_analyser_Analyser* ma, src_loc_SrcLoc loc, const char* format, ...)
{
   va_list args;
   va_start(args, format);
   diagnostics_Diags_error2(ma->diags, loc, format, args);
   va_end(args);
   ma->has_error = true;
}

static void module_analyser_Analyser_errorRange(module_analyser_Analyser* ma, src_loc_SrcLoc loc, src_loc_SrcRange range, const char* format, ...)
{
   va_list args;
   va_start(args, format);
   diagnostics_Diags_errorRange2(ma->diags, loc, range, format, args);
   va_end(args);
   ma->has_error = true;
}

static void module_analyser_Analyser_createGlobalScope(void* arg, ast_AST* a)
{
   module_analyser_Analyser* ma = arg;
   scope_Scope* s = scope_create(ma->allmodules, ma->diags, ast_AST_getImports(a), ma->mod, ast_Module_getSymbols(ma->mod), !ma->warnings->no_unused_variable);
   ast_AST_setPtr(a, s);
}

static void module_analyser_Analyser_deleteScope(void* _arg0, ast_AST* a)
{
   scope_Scope* s = ast_AST_getPtr(a);
   ast_AST_setPtr(a, NULL);
   scope_Scope_free(s);
}

static void module_analyser_Analyser_handleStructFunc(void* arg, ast_FunctionDecl* d)
{
   module_analyser_Analyser* ma = arg;
   ast_Ref* prefix = ast_FunctionDecl_getPrefix(d);
   assert(prefix);
   uint32_t prefix_name_idx = prefix->name_idx;
   assert(ma->struct_decls);
   uint32_t index = 0;
   if (prefix_name_idx == ma->prefix_cache_name) {
      index = ma->prefix_cache_idx;
   } else {
      bool found = false;
      found = module_analyser_NameVector_find(&ma->prefixes, prefix_name_idx, &index);
      if (!found) {
         ast_Decl* decl = ast_Module_findType(ma->mod, prefix_name_idx);
         if (!decl) {
            module_analyser_Analyser_error(ma, prefix->loc, "unknown type '%s'", ast_Ref_getName(prefix));
            return;
         }
         if (ast_Decl_getKind(decl) != ast_DeclKind_StructType) {
            module_analyser_Analyser_error(ma, prefix->loc, "struct-functions type must be a structs/union");
            return;
         }
         index = module_analyser_NameVector_add(&ma->prefixes, prefix_name_idx);
         struct_func_list_List_addDecl(ma->struct_decls, decl);
      }
      ma->prefix_cache_name = prefix_name_idx;
      ma->prefix_cache_idx = index;
   }
   struct_func_list_List_addFunc(ma->struct_decls, index, d);
   prefix->decl = struct_func_list_List_getDecl(ma->struct_decls, index);
}

static void module_analyser_Analyser_analyseFunctionProto(void* arg, ast_FunctionDecl* d)
{
   module_analyser_Analyser* ma = arg;
   module_analyser_Analyser_analyseGlobalDecl(ma, ((ast_Decl*)(d)));
}

static void module_analyser_Analyser_analyseFunctionBodies(void* arg, ast_FunctionDecl* d)
{
   module_analyser_Analyser* ma = arg;
   module_analyser_Analyser_analyseFunctionBody(ma, d, ast_AST_getPtr(ast_Decl_getAST(ast_FunctionDecl_asDecl(d))));
}

static void module_analyser_Analyser_analyseGlobalDecl(module_analyser_Analyser* ma, ast_Decl* d)
{
   if (ast_Decl_isChecked(d)) return;

   if (!module_analyser_Analyser_pushCheck(ma, d, ast_AST_getPtr(ast_Decl_getAST(d)), NULL)) return;

   switch (ast_Decl_getKind(d)) {
   case ast_DeclKind_Function:
      module_analyser_Analyser_analyseFunction(ma, ((ast_FunctionDecl*)(d)));
      break;
   case ast_DeclKind_Import:
      break;
   case ast_DeclKind_StructType:
      module_analyser_Analyser_analyseStructType(ma, ((ast_StructTypeDecl*)(d)));
      break;
   case ast_DeclKind_EnumType:
      module_analyser_Analyser_analyseEnumType(ma, ((ast_EnumTypeDecl*)(d)));
      break;
   case ast_DeclKind_EnumConstant:
      assert(0);
      break;
   case ast_DeclKind_FunctionType:
      module_analyser_Analyser_analyseFunctionType(ma, d);
      break;
   case ast_DeclKind_AliasType:
      module_analyser_Analyser_analyseAliasType(ma, ((ast_AliasTypeDecl*)(d)));
      break;
   case ast_DeclKind_Var:
      module_analyser_Analyser_analyseGlobalVarDecl(ma, ((ast_VarDecl*)(d)));
      break;
   case ast_DeclKind_StaticAssert:
      break;
   }
   ast_Decl_setChecked(d);
   module_analyser_Analyser_popCheck(ma);
}

static void module_analyser_Analyser_handleTypeDecl(void* arg, ast_Decl* d)
{
   module_analyser_Analyser* ma = arg;
   module_analyser_Analyser_analyseGlobalDecl(ma, d);
}

static void module_analyser_Analyser_handleStaticAssert(void* arg, ast_Decl* d)
{
   module_analyser_Analyser* ma = arg;
   ma->scope = ast_AST_getPtr(ast_Decl_getAST(d));
   ast_StaticAssertDecl* sa = ((ast_StaticAssertDecl*)(d));
   ast_Expr* lhs = ast_StaticAssertDecl_getLhs(sa);
   ast_Expr* rhs = ast_StaticAssertDecl_getRhs(sa);
   module_analyser_Analyser_analyseExpr(ma, &lhs, false);
   module_analyser_Analyser_analyseExpr(ma, &rhs, false);
   bool error = false;
   if (!ast_Expr_isCtv(lhs)) {
      module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(lhs), ast_Expr_getRange(lhs), "static_assert element is not a compile-time value");
      error = true;
   }
   if (!ast_Expr_isCtv(rhs)) {
      module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(rhs), ast_Expr_getRange(rhs), "static_assert element is not a compile-time value");
      error = true;
   }
   if (error) return;

   ctv_analyser_Value val1 = ctv_analyser_get_value(lhs);
   ctv_analyser_Value val2 = ctv_analyser_get_value(rhs);
   if (!ctv_analyser_Value_equals(&val1, &val2)) {
      module_analyser_Analyser_errorRange(ma, ast_Expr_getStartLoc(rhs), ast_Expr_getRange(rhs), "static_assert failed, expected %s, got %s", ctv_analyser_Value_str(&val1), ctv_analyser_Value_str(&val2));
   }
}

static void module_analyser_Analyser_handleVarDecl(void* arg, ast_VarDecl* v)
{
   module_analyser_Analyser* ma = arg;
   module_analyser_Analyser_analyseGlobalDecl(ma, ast_VarDecl_asDecl(v));
}

static void module_analyser_Analyser_checkName(module_analyser_Analyser* ma, ast_Decl* d, bool is_constant)
{
   const char* name = ast_Decl_getName(d);
   if (is_constant) {
      if (islower(name[0])) {
         module_analyser_Analyser_error(ma, ast_Decl_getLoc(d), "a global constant name must start with an upper case character");
      }
   } else {
      if (isupper(name[0])) {
         module_analyser_Analyser_error(ma, ast_Decl_getLoc(d), "a variable name must start with an lower case character");
      }
   }
}

static void module_analyser_Analyser_analyseGlobalVarDecl(module_analyser_Analyser* ma, ast_VarDecl* v)
{
   ast_Decl* d = ast_VarDecl_asDecl(v);
   ast_TypeRef* ref = ast_VarDecl_getTypeRef(v);
   ast_QualType res = module_analyser_Analyser_analyseTypeRef(ma, ref);
   if (ast_QualType_isInvalid(&res)) return;

   ast_Decl_setType(d, res);
   module_analyser_Analyser_checkName(ma, d, ast_QualType_isConstant(&res));
   if (ast_VarDecl_hasInit(v)) {
      module_analyser_Analyser_analyseInitExpr(ma, ast_VarDecl_getInit2(v), res);
   }
}

static bool module_analyser_Analyser_analyseInitExpr(module_analyser_Analyser* ma, ast_Expr** e_ptr, ast_QualType expectedType)
{
   ast_Expr* e = *e_ptr;
   if (ast_Expr_getKind(e) == ast_ExprKind_InitList) {
      return module_analyser_Analyser_analyseInitListExpr(ma, ((ast_InitListExpr*)(e)), expectedType);
   }
   if (ast_Expr_getKind(e) == ast_ExprKind_StringLiteral) {
      ast_ArrayType* at = ast_QualType_getArrayTypeOrNil(&expectedType);
      if (at) {
         ast_ArrayType* at2 = ast_QualType_getArrayType(&expectedType);
         ast_QualType elem = ast_ArrayType_getElemType(at);
         if ((ast_QualType_getTypeOrNil(&elem) != ast_QualType_getTypeOrNil(&ast_g_char) && ast_QualType_getTypeOrNil(&elem) != ast_QualType_getTypeOrNil(&ast_g_i8))) {
            module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(e), ast_Expr_getRange(e), "initializer-string for for non-char/int8 array");
            return false;
         }
         uint32_t rhs_len = ast_ArrayType_getSize(at2);
         if (ast_ArrayType_hasSize(at)) {
            uint32_t lhs_len = ast_ArrayType_getSize(at);
            if (rhs_len > lhs_len) {
               module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(e), ast_Expr_getRange(e), "initializer-string for char array is too long");
               return false;
            }
         } else {
            ast_ArrayType_setSize(at, rhs_len);
         }
         ast_Expr_setRValue(e);
      } else {
         ast_QualType result = module_analyser_getPointerFromArray(ma->builder, ast_Expr_getType(e));
         ast_builder_Builder_insertImplicitCast(ma->builder, ast_ImplicitCastKind_ArrayToPointerDecay, e_ptr, result);
         e = *e_ptr;
      }
      return true;
   }
   ast_QualType res = module_analyser_Analyser_analyseExpr(ma, e_ptr, true);
   if (ast_QualType_isInvalid(&res)) return false;

   e = *e_ptr;
   if ((!ma->curFunction && !ast_Expr_isCtv(e))) {
      if (!ast_Expr_isCtc(e)) {
         module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(e), ast_Expr_getRange(e), "initializer element is not a compile-time constant");
         return false;
      }
      if (ast_QualType_needsCtvInit(&expectedType)) {
         module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(e), ast_Expr_getRange(e), "initializer element is not a compile-time value");
         return false;
      }
   }
   return conversion_checker_Checker_check(&ma->checker, expectedType, res, e_ptr, ast_Expr_getLoc(e));
}

static bool module_analyser_Analyser_pushCheck(module_analyser_Analyser* ma, ast_Decl* d, scope_Scope* s, ast_FunctionDecl* fd)
{
   for (uint32_t i = 0; i < ma->checkIndex; i++) {
      if (ma->checkStack[i].decl == d) {
         for (uint32_t j = i; j < ma->checkIndex; j++) {
            module_analyser_Analyser_error(ma, ast_Decl_getLoc(d), "circular declaration dependency %s", ast_Decl_getName(d));
         }
         return false;
      }
   }
   ma->scope = s;
   module_analyser_StackLayer* top = &ma->checkStack[ma->checkIndex];
   top->decl = d;
   top->scope = ma->scope;
   top->function = fd;
   if (fd) ma->curFunction = fd;
   ma->checkIndex++;
   if (!ast_Decl_isChecked(d)) ast_Decl_setCheckState(d, ast_DeclCheckState_InProgress);
   assert(ma->checkIndex <= module_analyser_MaxDepth);
   return true;
}

static void module_analyser_Analyser_popCheck(module_analyser_Analyser* ma)
{
   assert(ma->checkIndex > 0);
   ma->checkIndex--;
   if (ma->checkIndex > 0) {
      module_analyser_StackLayer* top = &ma->checkStack[ma->checkIndex - 1];
      ma->scope = top->scope;
      ma->curFunction = top->function;
   } else {
      ma->scope = NULL;
      ma->curFunction = NULL;
   }
}

static bool module_analyser_Analyser_analyseInitListExpr(module_analyser_Analyser* ma, ast_InitListExpr* ile, ast_QualType expectedType)
{
   const ast_Type* t = ast_QualType_getTypeOrNil(&expectedType);
   if (ast_Type_isArrayType(t)) {
      return module_analyser_Analyser_analyseInitListArray(ma, ile, expectedType);
   }
   if (ast_Type_isStructType(t)) {
      return module_analyser_Analyser_analyseInitListStruct(ma, ile, expectedType);
   }
   ast_Expr* e = ((ast_Expr*)(ile));
   module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "cannot initialize variable of type %s with initializer list", ast_QualType_diagName(&expectedType));
   return false;
}

static bool module_analyser_Analyser_analyseInitListArray(module_analyser_Analyser* ma, ast_InitListExpr* ile, ast_QualType expectedType)
{
   ast_Expr* e = ((ast_Expr*)(ile));
   uint32_t numValues = ast_InitListExpr_getNumValues(ile);
   ast_Expr** values = ast_InitListExpr_getValues(ile);
   ast_QualType_clearQuals(&expectedType);
   ast_ArrayType* at = ast_QualType_getArrayType(&expectedType);
   ast_QualType et = ast_ArrayType_getElemType(at);
   ast_QualType_clearQuals(&et);
   bool ok = true;
   bool ctc = true;
   bool have_designators = false;
   for (uint32_t i = 0; i < numValues; i++) {
      ast_Expr* value = values[i];
      if (ast_Expr_getKind(value) == ast_ExprKind_FieldDesignatedInit) {
         module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(value), ast_Expr_getRange(value), "field designator cannot initialize an array");
         ok = false;
         continue;
      }
      if (ast_Expr_getKind(value) == ast_ExprKind_ArrayDesignatedInit) {
         ok |= module_analyser_Analyser_analyseArrayDesignatedInit(ma, value, et);
         have_designators = true;
         continue;
      }
      ok |= module_analyser_Analyser_analyseInitExpr(ma, &values[i], et);
      ctc |= ast_Expr_isCtc(values[i]);
   }
   if (ctc) ast_Expr_setCtc(e);
   if (!ok) return false;

   if (have_designators) {
   }
   if (ast_ArrayType_hasSize(at)) {
      uint32_t arraySize = ast_ArrayType_getSize(at);
      if (numValues > arraySize) {
         module_analyser_Analyser_error(ma, ast_Expr_getLoc(values[arraySize]), "excess elements in array initializer");
         return false;
      }
   } else {
      ast_ArrayType_setSize(at, numValues);
   }
   ast_Expr_setType(e, expectedType);
   return ok;
}

static bool module_analyser_Analyser_analyseInitListStruct(module_analyser_Analyser* ma, ast_InitListExpr* ile, ast_QualType expectedType)
{
   ast_Expr* e = ((ast_Expr*)(ile));
   uint32_t numValues = ast_InitListExpr_getNumValues(ile);
   ast_Expr** values = ast_InitListExpr_getValues(ile);
   ast_StructType* st = ast_QualType_getStructType(&expectedType);
   ast_StructTypeDecl* std = ast_StructType_getDecl(st);
   if (numValues == 0) {
      ast_Expr_setType(e, expectedType);
      return true;
   }
   const bool haveDesignators = (ast_Expr_getKind(values[0]) == ast_ExprKind_FieldDesignatedInit);
   if ((!haveDesignators && ast_StructTypeDecl_isUnion(std))) {
      module_analyser_Analyser_error(ma, ast_Expr_getLoc(values[0]), "union member initializer needs field designator");
      return false;
   }
   const uint32_t num_members = ast_StructTypeDecl_getNumMembers(std);
   ast_Decl** members = ast_StructTypeDecl_getMembers(std);
   for (uint32_t i = 0; i < numValues; i++) {
      ast_Expr* value = values[i];
      if (i >= num_members) {
         module_analyser_Analyser_error(ma, ast_Expr_getLoc(value), "excess initializer elements in struct");
         return false;
      }
      bool is_designator = ast_Expr_getKind(value) == ast_ExprKind_FieldDesignatedInit;
      if (haveDesignators != is_designator) {
         module_analyser_Analyser_error(ma, ast_Expr_getLoc(value), "mixing field designator with non-field designators");
         return false;
      }
      if (is_designator) {
         ast_FieldDesignatedInitExpr* fdi = ((ast_FieldDesignatedInitExpr*)(value));
         ast_Decl* member = module_analyser_Analyser_findStructMember(ma, std, ast_FieldDesignatedInitExpr_getField(fdi), ast_Expr_getLoc(value), false);
         if (!member) return false;

         ast_Decl_setUsed(member);
         bool ok = module_analyser_Analyser_analyseInitExpr(ma, ast_FieldDesignatedInitExpr_getInit2(fdi), ast_Decl_getType(member));
         if (!ok) return false;

         ast_Expr_setType(value, ast_Expr_getType(ast_FieldDesignatedInitExpr_getInit(fdi)));
      } else {
         ast_Decl_setUsed(members[i]);
         bool ok = module_analyser_Analyser_analyseInitExpr(ma, &values[i], ast_Decl_getType(members[i]));
         if (!ok) return false;

      }
   }
   ast_QualType_clearQuals(&expectedType);
   ast_Expr_setType(e, expectedType);
   return true;
}

static void module_analyser_Analyser_analyseStructType(module_analyser_Analyser* ma, ast_StructTypeDecl* d)
{
   module_analyser_NameVector names;
   module_analyser_NameVector_init(&names, ast_StructTypeDecl_getNumMembers(d));
   module_analyser_NameVector locs;
   module_analyser_NameVector_init(&locs, ast_StructTypeDecl_getNumMembers(d));
   module_analyser_Analyser_analyseStructNames(ma, d, &names, &locs);
   module_analyser_NameVector_free(&names);
   module_analyser_NameVector_free(&locs);
   module_analyser_Analyser_analyseStructMembers(ma, d);
}

static void module_analyser_Analyser_analyseStructMembers(module_analyser_Analyser* ma, ast_StructTypeDecl* d)
{
   uint32_t count = ast_StructTypeDecl_getNumMembers(d);
   ast_Decl** members = ast_StructTypeDecl_getMembers(d);
   for (uint32_t i = 0; i < count; i++) {
      ast_Decl* member = members[i];
      if (ast_Decl_getKind(member) == ast_DeclKind_Var) {
         ast_VarDecl* vd = ((ast_VarDecl*)(member));
         ast_Decl_setCheckState(member, ast_DeclCheckState_InProgress);
         module_analyser_Analyser_analyseStructMember(ma, vd);
         ast_Expr* bitfield = ast_VarDecl_getBitfield(vd);
         if (bitfield) {
            ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, &bitfield, false);
            if (ast_QualType_isInvalid(&qt)) return;

            if (!ast_Expr_isCtv(bitfield)) {
               module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(bitfield), ast_Expr_getRange(bitfield), "bitfield size is not a compile-time value");
               return;
            }
         }
         ast_Decl_setCheckState(member, ast_DeclCheckState_Checked);
      } else if (ast_Decl_getKind(member) == ast_DeclKind_StructType) {
         ast_StructTypeDecl* sub = ((ast_StructTypeDecl*)(member));
         module_analyser_Analyser_analyseStructMembers(ma, sub);
         ast_Decl_setCheckState(member, ast_DeclCheckState_Checked);
      }

   }
   size_analyser_TypeSize info = size_analyser_sizeOfStruct(d);
   ast_StructTypeDecl_setSize(d, info.size);
   ast_StructTypeDecl_setAlignment(d, info.align);
}

static void module_analyser_Analyser_analyseStructNames(module_analyser_Analyser* ma, ast_StructTypeDecl* d, module_analyser_NameVector* names, module_analyser_NameVector* locs)
{
   uint32_t count = ast_StructTypeDecl_getNumMembers(d);
   ast_Decl** members = ast_StructTypeDecl_getMembers(d);
   for (uint32_t i = 0; i < count; i++) {
      ast_Decl* member = members[i];
      uint32_t name_idx = ast_Decl_getNameIdx(member);
      uint32_t old_index;
      ast_StructTypeDecl* sub = NULL;
      if (ast_Decl_getKind(member) == ast_DeclKind_StructType) sub = ((ast_StructTypeDecl*)(member));
      if (name_idx == 0) {
         if (ast_Decl_getKind(member) == ast_DeclKind_StructType) {
            module_analyser_Analyser_analyseStructNames(ma, sub, names, locs);
         }
      } else {
         if (module_analyser_NameVector_find(names, name_idx, &old_index)) {
            module_analyser_Analyser_error(ma, ast_Decl_getLoc(member), "duplicate %s member '%s'", ast_StructTypeDecl_isStruct(d) ? "struct" : "union", ast_Decl_getName(member));
            module_analyser_Analyser_note(ma, module_analyser_NameVector_get(locs, old_index), "previous declaration is here");
            return;
         }
         module_analyser_NameVector_add(names, name_idx);
         module_analyser_NameVector_add(locs, ast_Decl_getLoc(member));
         if (ast_Decl_getKind(member) == ast_DeclKind_StructType) {
            module_analyser_NameVector sub_names;
            module_analyser_NameVector_init(&sub_names, ast_StructTypeDecl_getNumMembers(sub));
            module_analyser_NameVector sub_locs;
            module_analyser_NameVector_init(&sub_locs, ast_StructTypeDecl_getNumMembers(sub));
            module_analyser_Analyser_analyseStructNames(ma, sub, &sub_names, &sub_locs);
            module_analyser_NameVector_free(&sub_names);
            module_analyser_NameVector_free(&sub_locs);
         }
      }
   }
}

static void module_analyser_Analyser_analyseFunctionType(module_analyser_Analyser* ma, ast_Decl* d)
{
   ast_FunctionTypeDecl* ftd = ((ast_FunctionTypeDecl*)(d));
   ast_FunctionDecl* fd = ast_FunctionTypeDecl_getDecl(ftd);
   module_analyser_Analyser_analyseFunction(ma, fd);
}

static void module_analyser_Analyser_analyseFunction(module_analyser_Analyser* ma, ast_FunctionDecl* fd)
{
   if (ast_FunctionDecl_isTemplate(fd)) {
      scope_Scope_checkGlobalSymbol(ma->scope, ast_FunctionDecl_getTemplateNameIdx(fd), ast_FunctionDecl_getTemplateLoc(fd));
      return;
   }
   ast_QualType qt = module_analyser_Analyser_analyseTypeRef(ma, ast_FunctionDecl_getTypeRef(fd));
   if (ast_QualType_isInvalid(&qt)) return;

   ast_FunctionDecl_setRType(fd, qt);
   uint32_t num_params = ast_FunctionDecl_getNumParams(fd);
   ast_VarDecl** params = ast_FunctionDecl_getParams(fd);
   for (uint32_t i = 0; i < num_params; i++) {
      ast_VarDecl* v = params[i];
      ast_TypeRef* ref = ast_VarDecl_getTypeRef(v);
      ast_QualType res = module_analyser_Analyser_analyseTypeRef(ma, ref);
      if (ast_QualType_isInvalid(&res)) continue;

      ast_Decl* d = ((ast_Decl*)(v));
      ast_Decl_setType(d, res);
      ast_Decl_setChecked(d);
   }
   if ((num_params && ast_FunctionDecl_hasPrefix(fd))) {
      const ast_Ref* prefix = ast_FunctionDecl_getPrefix(fd);
      ast_QualType prefixType = ast_Decl_getType(prefix->decl);
      ast_TypeRef* ref = ast_VarDecl_getTypeRef(params[0]);
      if (ast_TypeRef_isPointerTo(ref, ast_QualType_getIndex(&prefixType))) {
         ast_FunctionDecl_setCallKind(fd, ast_CallKind_StructFunc);
      }
   }
}

static void module_analyser_Analyser_analyseAliasType(module_analyser_Analyser* ma, ast_AliasTypeDecl* d)
{
   ast_TypeRef* ref = ast_AliasTypeDecl_getTypeRef(d);
   ast_QualType res = module_analyser_Analyser_analyseTypeRef(ma, ref);
   ast_QualType qt = ast_Decl_getType(ast_AliasTypeDecl_asDecl(d));
   ast_Type* at = ast_QualType_getTypeOrNil(&qt);
   ast_QualType canon = ast_QualType_getCanonicalType(&res);
   ast_QualType_copyQuals(&canon, res);
   ast_Type_setCanonicalType(at, canon);
}

static void module_analyser_Analyser_analyseEnumType(module_analyser_Analyser* ma, ast_EnumTypeDecl* d)
{
   uint32_t num_constants = ast_EnumTypeDecl_getNumConstants(d);
   ast_EnumConstantDecl** constants = ast_EnumTypeDecl_getConstants(d);
   uint32_t value = 0;
   for (uint32_t i = 0; i < num_constants; i++) {
      ast_EnumConstantDecl* c = constants[i];
      const char* name = ast_Decl_getName(ast_EnumConstantDecl_asDecl(c));
      for (uint32_t j = 0; j < i; j++) {
         const ast_Decl* other = ast_EnumConstantDecl_asDecl(constants[j]);
         if (ast_Decl_getName(other) == name) {
            module_analyser_Analyser_error(ma, ast_Decl_getLoc(ast_EnumConstantDecl_asDecl(c)), "duplicate enum constant name '%s'", name);
            module_analyser_Analyser_note(ma, ast_Decl_getLoc(other), "previous declaration is here");
            return;
         }
      }
      ast_Expr* initval = ast_EnumConstantDecl_getInit(c);
      if (initval) {
         ast_QualType res = module_analyser_Analyser_analyseExpr(ma, ast_EnumConstantDecl_getInit2(c), true);
         if (ast_QualType_isInvalid(&res)) return;

         initval = ast_EnumConstantDecl_getInit(c);
         if (!ast_Expr_isCtv(initval)) {
            module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(initval), ast_Expr_getRange(initval), "initializer is not a compile-time value");
            return;
         }
         ctv_analyser_Value ctv = ctv_analyser_get_value(initval);
         if (ctv.uvalue < value) {
            module_analyser_Analyser_error(ma, ast_Expr_getLoc(initval), "enum constants need to increase (value %u, previous %u)", ctv.uvalue, value - 1);
            return;
         }
         value = ((uint32_t)(ctv.uvalue));
      }
      ast_EnumConstantDecl_setValue(c, value);
      ast_Decl_setChecked(ast_EnumConstantDecl_asDecl(c));
      value++;
   }
}

static void module_analyser_Analyser_analyseStructMember(module_analyser_Analyser* ma, ast_VarDecl* v)
{
   ast_TypeRef* ref = ast_VarDecl_getTypeRef(v);
   ast_QualType res = module_analyser_Analyser_analyseTypeRef(ma, ref);
   if (ast_QualType_isInvalid(&res)) return;

   ast_Decl_setType(ast_VarDecl_asDecl(v), res);
}

static ast_QualType module_analyser_Analyser_analyseUserTypeRef(module_analyser_Analyser* ma, ast_TypeRef* ref)
{
   const ast_Ref* prefix = ast_TypeRef_getPrefix(ref);
   const ast_Ref* user = ast_TypeRef_getUser(ref);
   ast_Decl* d = NULL;
   if (prefix) {
      ast_ImportDecl* i = scope_Scope_findModule(ma->scope, prefix->name_idx, prefix->loc);
      if (!i) {
         ma->has_error = true;
         return ast_QualType_Invalid;
      }
      ast_TypeRef_setPrefix(ref, ((ast_Decl*)(i)));
      ast_Module* mod = ast_ImportDecl_getDest(i);
      d = scope_Scope_findGlobalSymbolInModule(ma->scope, mod, user->name_idx, user->loc);
   } else {
      d = scope_Scope_findType(ma->scope, user->name_idx, user->loc);
   }
   if (!d) {
      ma->has_error = true;
      return ast_QualType_Invalid;
   }
   ast_TypeRef_setUser(ref, d);
   if (!ast_Decl_isTypeDecl(d)) {
      module_analyser_Analyser_error(ma, user->loc, "%s is not a type", ast_idx2name(user->name_idx));
      return ast_QualType_Invalid;
   }
   bool external = (ma->mod != ast_Decl_getModule(d));
   if ((!ast_Decl_isPublic(d) && external)) {
      module_analyser_Analyser_error(ma, user->loc, "%s is not public", ast_idx2name(user->name_idx));
      return ast_QualType_Invalid;
   }
   bool full = (ast_TypeRef_getNumPointers(ref) == 0);
   ast_DeclCheckState state = ast_Decl_getCheckState(d);
   if ((full && state == ast_DeclCheckState_InProgress)) {
      module_analyser_Analyser_error(ma, user->loc, "circular declaration");
      return ast_QualType_Invalid;
   }
   if ((full && state != ast_DeclCheckState_Checked)) {
      module_analyser_Analyser_analyseGlobalDecl(ma, d);
   }
   ast_Decl_setUsed(d);
   if (external) ast_Decl_setUsedPublic(d);
   return ast_Decl_getType(d);
}

static ast_QualType module_analyser_Analyser_analyseTypeRef(module_analyser_Analyser* ma, ast_TypeRef* ref)
{
   ast_QualType base = { };
   if (ast_TypeRef_isUser(ref)) {
      base = module_analyser_Analyser_analyseUserTypeRef(ma, ref);
      if (ast_QualType_isInvalid(&base)) return base;

      assert(ast_QualType_hasCanonicalType(&base));
   } else {
      ast_BuiltinKind kind = ast_TypeRef_getBuiltinKind(ref);
      base = ast_builder_Builder_actOnBuiltinType(ma->builder, kind);
      assert(ast_QualType_isValid(&base));
   }
   if (ast_TypeRef_isConst(ref)) ast_QualType_setConst(&base);
   if (ast_TypeRef_isVolatile(ref)) ast_QualType_setVolatile(&base);
   ast_QualType resolved = base;
   uint32_t num_ptrs = ast_TypeRef_getNumPointers(ref);
   for (uint32_t i = 0; i < num_ptrs; i++) {
      resolved = ast_builder_Builder_actOnPointerType(ma->builder, resolved);
   }
   uint32_t num_arrays = ast_TypeRef_getNumArrays(ref);
   for (uint32_t i = 0; i < num_arrays; i++) {
      ast_Expr* sizeExpr = ast_TypeRef_getArray(ref, i);
      uint32_t size = 0;
      if (sizeExpr) {
         module_analyser_Analyser_analyseExpr(ma, ast_TypeRef_getArray2(ref, i), true);
         sizeExpr = ast_TypeRef_getArray(ref, i);
         if (!ast_Expr_isCtv(sizeExpr)) {
            module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(sizeExpr), ast_Expr_getRange(sizeExpr), "array size is not a compile-time value");
         } else {
            ctv_analyser_Value value = ctv_analyser_get_value(sizeExpr);
            if (ctv_analyser_Value_isNegative(&value)) {
               module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(sizeExpr), ast_Expr_getRange(sizeExpr), "array size has negative value %d", value.svalue);
            }
            size = ((uint32_t)(value.uvalue));
         }
      }
      resolved = ast_builder_Builder_actOnArrayType(ma->builder, resolved, sizeExpr != NULL, size);
   }
   ast_TypeRef_setDest(ref, ast_QualType_getIndex(&base));
   return resolved;
}

static bool module_analyser_Analyser_checkOpaque(module_analyser_Analyser* ma, const ast_StructTypeDecl* std, src_loc_SrcLoc loc)
{
   return true;
   if (ast_StructTypeDecl_isOpaque(std)) {
      ast_Decl* d = ((ast_Decl*)(std));
      ast_Module* other = ast_Decl_getModule(d);
      if (other != ma->mod) {
         ast_QualType qt = ast_Decl_getType(d);
         module_analyser_Analyser_error(ma, loc, " cannot dereference opaque struct '%s'", ast_QualType_diagName(&qt));
         return false;
      }
   }
   return true;
}

static void module_analyser_markMainUsed(void* arg, ast_FunctionDecl* fd)
{
   module_analyser_MainMarker* m = (arg);
   ast_Decl* d = ((ast_Decl*)(fd));
   if (ast_Decl_getNameIdx(d) == m->name_idx) {
      m->main = d;
   }
}

static ast_Decl* module_analyser_Analyser_checkMain(module_analyser_Analyser* ma, bool test_mode, ast_Module* top, uint32_t name_idx)
{
   ast_Module_setUsed(top);
   module_analyser_MainMarker marker = { name_idx, NULL };
   ast_Module_visitFunctions(top, module_analyser_markMainUsed, &marker);
   if (!marker.main) {
      if (!test_mode) module_analyser_Analyser_error(ma, 0, "no main function found");
      return NULL;
   }
   ast_Decl_setUsed(marker.main);
   ast_Decl_setUsedPublic(marker.main);
   ast_Decl_setAttrExport(marker.main);
   return marker.main;
}

static ast_QualType module_analyser_getPointerFromArray(ast_builder_Builder* builder, ast_QualType q)
{
   const ast_ArrayType* a = ((ast_ArrayType*)(ast_QualType_getTypeOrNil(&q)));
   ast_QualType elem = ast_ArrayType_getElemType(a);
   ast_QualType res = ast_builder_Builder_actOnPointerType(builder, elem);
   return res;
}

static ast_QualType module_analyser_Analyser_analyseExpr(module_analyser_Analyser* ma, ast_Expr** e_ptr, bool need_rvalue)
{
   assert(e_ptr);
   ast_QualType result = module_analyser_Analyser_analyseExprInner(ma, e_ptr);
   if (ast_QualType_isInvalid(&result)) return result;

   ast_Expr* e = *e_ptr;
   ast_Expr_setType(e, result);
   if ((need_rvalue && ast_Expr_isLValue(e))) {
      ast_QualType qt = result;
      if (ast_QualType_isArrayType(&qt)) {
         result = module_analyser_getPointerFromArray(ma->builder, qt);
         ast_builder_Builder_insertImplicitCast(ma->builder, ast_ImplicitCastKind_ArrayToPointerDecay, e_ptr, result);
      } else {
         ast_builder_Builder_insertImplicitCast(ma->builder, ast_ImplicitCastKind_LValueToRValue, e_ptr, qt);
      }
   }
   return result;
}

static ast_QualType module_analyser_Analyser_analyseExprInner(module_analyser_Analyser* ma, ast_Expr** e_ptr)
{
   ast_Expr* e = *e_ptr;
   switch (ast_Expr_getKind(e)) {
   case ast_ExprKind_IntegerLiteral:
      return ast_Expr_getType(e);
   case ast_ExprKind_BooleanLiteral:
      return ast_g_bool;
   case ast_ExprKind_CharLiteral:
      return ast_g_i8;
   case ast_ExprKind_StringLiteral:
      return ast_Expr_getType(e);
   case ast_ExprKind_Nil:
      return ast_g_void_ptr;
   case ast_ExprKind_Identifier: {
      ast_Decl* d = module_analyser_Analyser_analyseIdentifier(ma, e_ptr);
      if (!d) break;

      return ast_Decl_getType(d);
   }
   case ast_ExprKind_Type:
      break;
   case ast_ExprKind_Call:
      return module_analyser_Analyser_analyseCallExpr(ma, e_ptr);
   case ast_ExprKind_InitList:
      ast_Expr_dump((*e_ptr));
      assert(0);
      break;
   case ast_ExprKind_FieldDesignatedInit:
      ast_Expr_dump((*e_ptr));
      assert(0);
      break;
   case ast_ExprKind_ArrayDesignatedInit:
      assert(0);
      break;
   case ast_ExprKind_BinaryOperator:
      return module_analyser_Analyser_analyseBinaryOperator(ma, e_ptr);
   case ast_ExprKind_UnaryOperator:
      return module_analyser_Analyser_analyseUnaryOperator(ma, e_ptr);
   case ast_ExprKind_ConditionalOperator:
      return module_analyser_Analyser_analyseConditionalOperator(ma, e_ptr);
   case ast_ExprKind_Builtin:
      return module_analyser_Analyser_analyseBuiltin(ma, e_ptr);
   case ast_ExprKind_ArraySubscript:
      return module_analyser_Analyser_analyseArraySubscriptExpr(ma, e_ptr);
   case ast_ExprKind_Member:
      return module_analyser_Analyser_analyseMemberExpr(ma, e_ptr);
   case ast_ExprKind_Paren: {
      ast_ParenExpr* p = ((ast_ParenExpr*)(e));
      ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, ast_ParenExpr_getInner2(p), false);
      ast_Expr* inner = ast_ParenExpr_getInner(p);
      ast_Expr_copyConstantFlags(e, inner);
      ast_Expr_copyValType(e, inner);
      return qt;
   }
   case ast_ExprKind_BitOffset:
      break;
   case ast_ExprKind_ExplicitCast:
      return module_analyser_Analyser_analyseExplicitCast(ma, e_ptr);
   case ast_ExprKind_ImplicitCast:
      break;
   }
   return ast_QualType_Invalid;
}

static ast_Decl* module_analyser_Analyser_analyseIdentifier(module_analyser_Analyser* ma, ast_Expr** e_ptr)
{
   ast_Expr* e = *e_ptr;
   ast_IdentifierExpr* i = ((ast_IdentifierExpr*)(e));
   ast_Decl* d = scope_Scope_find(ma->scope, ast_IdentifierExpr_getNameIdx(i), ast_Expr_getLoc(e));
   if (!d) {
      ma->has_error = true;
      return NULL;
   }
   if (!ast_Decl_isChecked(d)) {
      module_analyser_Analyser_analyseGlobalDecl(ma, d);
   }
   ast_QualType qt = ast_Decl_getType(d);
   assert(ast_QualType_isValid(&qt));
   ast_Expr_setType(e, qt);
   ast_IdentifierExpr_setDecl(i, d);
   ast_Decl_setUsed(d);
   ast_IdentifierKind kind = module_analyser_Analyser_setExprFlags(ma, e_ptr, d);
   ast_IdentifierExpr_setKind(i, kind);
   return d;
}

static void module_analyser_create_template_name(char* name, const char* orig, uint16_t idx)
{
   sprintf(name, "%s_%u_", orig, idx);
}

static ast_FunctionDecl* module_analyser_Analyser_instantiateTemplateFunction(module_analyser_Analyser* ma, ast_CallExpr* call, ast_FunctionDecl* fd)
{
   ast_TypeRef* template_arg = ast_CallExpr_getTemplateArg(call);
   ast_QualType templateType = module_analyser_Analyser_analyseTypeRef(ma, template_arg);
   if (ast_QualType_isInvalid(&templateType)) return NULL;

   ast_FunctionDecl* instance = ast_Module_findInstance(ma->mod, fd, templateType);
   if (!instance) {
      instance = ast_FunctionDecl_instantiate(fd, ma->context, template_arg);
      ast_Decl* d = ((ast_Decl*)(instance));
      module_analyser_Analyser_analyseFunction(ma, instance);
      if (ma->has_error) return NULL;

      ast_Decl_setChecked(d);
      module_analyser_Analyser* analyser = module_analyser_create(ma->diags, ma->context, ma->astPool, ma->builder, ma->allmodules, ma->warnings);
      scope_Scope* tmpScope = scope_create(ma->allmodules, ma->diags, ast_AST_getImports(ast_Decl_getAST(d)), ma->mod, ast_Module_getSymbols(ma->mod), !ma->warnings->no_unused_variable);
      module_analyser_Analyser_analyseFunctionBody(analyser, instance, tmpScope);
      scope_Scope_free(tmpScope);
      module_analyser_Analyser_free(analyser);
      if (ma->has_error) return NULL;

      uint16_t instance_idx = ast_Module_addInstance(ma->mod, fd, templateType, instance);
      ast_FunctionDecl_setTemplateInstanceIdx(instance, instance_idx);
      char name[64];
      module_analyser_create_template_name(name, ast_Decl_getName(d), instance_idx);
      ast_FunctionDecl_setInstanceName(instance, string_pool_Pool_addStr(ma->astPool, name, true));
   }
   ast_CallExpr_setTemplateIdx(call, ast_FunctionDecl_getTemplateInstanceIdx(instance));
   return instance;
}

static ast_QualType module_analyser_Analyser_analyseCallExpr(module_analyser_Analyser* ma, ast_Expr** e_ptr)
{
   ast_Expr* e = *e_ptr;
   ast_CallExpr* call = ((ast_CallExpr*)(e));
   ast_Expr** fn = ast_CallExpr_getFunc2(call);
   ast_Expr* origFn = ast_CallExpr_getFunc(call);
   ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, fn, true);
   if (ast_QualType_isInvalid(&qt)) return ast_QualType_Invalid;

   if (ast_Expr_isNValue(origFn)) {
      module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(origFn), ast_Expr_getRange(origFn), "called object is not a function of function pointer");
      return ast_QualType_Invalid;
   }
   ast_FunctionType* ft = ast_QualType_getFunctionTypeOrNil(&qt);
   if (!ft) {
      ast_Expr* fn2 = ast_CallExpr_getFunc(call);
      module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(fn2), ast_Expr_getRange(fn2), "called object type %s is not a function or function pointer", ast_QualType_diagName(&qt));
      return ast_QualType_Invalid;
   }
   ast_FunctionDecl* fd = ast_FunctionType_getDecl(ft);
   ast_Decl_setUsed(ast_FunctionDecl_asDecl(fd));
   if (ast_FunctionDecl_isTemplate(fd)) {
      if (!ast_CallExpr_getTemplateArg(call)) {
         module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(e), ast_Expr_getRange(e), "function %s requires a template argument", ast_Decl_getFullName(ast_FunctionDecl_asDecl(fd)));
         return ast_QualType_Invalid;
      }
      fd = module_analyser_Analyser_instantiateTemplateFunction(ma, call, fd);
      if (!fd) return ast_QualType_Invalid;

   } else {
      if (ast_CallExpr_getTemplateArg(call)) {
         module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(e), ast_Expr_getRange(e), "function %s is not a template function", ast_Decl_getFullName(ast_FunctionDecl_asDecl(fd)));
         return ast_QualType_Invalid;
      }
   }
   uint32_t func_num_args = ast_FunctionDecl_getNumParams(fd);
   uint32_t call_num_args = ast_CallExpr_getNumArgs(call);
   uint32_t funcIndex = 0;
   uint32_t callIndex = 0;
   bool isStructFuncCall = false;
   if (ast_Expr_getKind(origFn) == ast_ExprKind_Member) {
      const ast_MemberExpr* m = ((ast_MemberExpr*)(origFn));
      if (ast_MemberExpr_isStaticStructFunc(m)) {
         ast_CallExpr_setCallsStaticStructFunc(call);
      }
      if (ast_MemberExpr_isStructFunc(m)) {
         isStructFuncCall = true;
         ast_CallExpr_setCallsStructFunc(call);
         func_num_args--;
         funcIndex = 1;
      }
   }
   ast_VarDecl** func_args = ast_FunctionDecl_getParams(fd);
   ast_Expr** call_args = ast_CallExpr_getArgs(call);
   uint32_t min_args = (func_num_args < call_num_args) ? func_num_args : call_num_args;
   for (uint32_t i = 0; i < min_args; i++) {
      ast_QualType callType = module_analyser_Analyser_analyseExpr(ma, &call_args[callIndex], true);
      if (ast_QualType_isInvalid(&callType)) return ast_QualType_Invalid;

      ast_VarDecl* vd = func_args[funcIndex];
      ast_Expr* call_arg = call_args[callIndex];
      bool ok = conversion_checker_Checker_check(&ma->checker, ast_Decl_getType(ast_VarDecl_asDecl(vd)), callType, &call_args[callIndex], ast_Expr_getLoc(call_arg));
      if (!ok) return ast_QualType_Invalid;

      callIndex++;
      funcIndex++;
   }
   if (call_num_args > func_num_args) {
      if (!ast_FunctionDecl_isVariadic(fd)) {
         ast_Expr* call_arg = call_args[callIndex];
         module_analyser_Analyser_error(ma, ast_Expr_getLoc(call_arg), "too many arguments to function call, expected %u, have %u", func_num_args, call_num_args);
         module_analyser_Analyser_note(ma, ast_Decl_getLoc(ast_FunctionDecl_asDecl(fd)), "'%s' declared here", ast_Decl_getName(ast_FunctionDecl_asDecl(fd)));
         return ast_QualType_Invalid;
      }
      for (uint32_t i = min_args; i < call_num_args; i++) {
         ast_QualType callType = module_analyser_Analyser_analyseExpr(ma, &call_args[callIndex], true);
         if (ast_QualType_isInvalid(&callType)) return ast_QualType_Invalid;

         if (ast_QualType_isVoidType(&callType)) {
            ast_Expr* call_arg = call_args[callIndex];
            module_analyser_Analyser_error(ma, ast_Expr_getLoc(call_arg), "passing 'void' as variadic argument is invalid");
            return ast_QualType_Invalid;
         }
         callIndex++;
      }
   } else if (call_num_args < func_num_args) {
      module_analyser_Analyser_error(ma, ast_CallExpr_getEndLoc(call), "too few arguments to function call, expected %u, have %u", func_num_args, call_num_args);
      module_analyser_Analyser_note(ma, ast_Decl_getLoc(ast_FunctionDecl_asDecl(fd)), "'%s' declared here", ast_Decl_getName(ast_FunctionDecl_asDecl(fd)));
      return ast_QualType_Invalid;
   }

   return ast_FunctionDecl_getRType(fd);
}

static ast_IdentifierKind module_analyser_Analyser_setExprFlags(module_analyser_Analyser* ma, ast_Expr** e_ptr, ast_Decl* d)
{
   ast_Expr* e = *e_ptr;
   ast_IdentifierKind kind = ast_IdentifierKind_Unresolved;
   switch (ast_Decl_getKind(d)) {
   case ast_DeclKind_Function:
      ast_Expr_setCtc(e);
      ast_Expr_setRValue(e);
      ast_builder_Builder_insertImplicitCast(ma->builder, ast_ImplicitCastKind_FunctionToPointerDecay, e_ptr, ast_Decl_getType(d));
      kind = ast_IdentifierKind_Function;
      break;
   case ast_DeclKind_Import:
      ast_Expr_setCtc(e);
      kind = ast_IdentifierKind_Module;
      break;
   case ast_DeclKind_StructType:
      kind = ast_IdentifierKind_Type;
      break;
   case ast_DeclKind_EnumType:
      ast_Expr_setCtc(e);
      kind = ast_IdentifierKind_Type;
      break;
   case ast_DeclKind_EnumConstant:
      ast_Expr_setCtc(e);
      ast_Expr_setCtv(e);
      ast_Expr_setRValue(e);
      kind = ast_IdentifierKind_EnumConstant;
      break;
   case ast_DeclKind_FunctionType:
      ast_Expr_setCtc(e);
      kind = ast_IdentifierKind_Type;
      break;
   case ast_DeclKind_AliasType:
      kind = ast_IdentifierKind_Type;
      break;
   case ast_DeclKind_Var: {
      ast_VarDecl* vd = ((ast_VarDecl*)(d));
      ast_QualType t = ast_Decl_getType(ast_VarDecl_asDecl(vd));
      if (ast_VarDecl_isGlobal(vd)) ast_Expr_setCtc(e);
      ast_Expr_setLValue(e);
      const ast_Expr* init_ = ast_VarDecl_getInit(vd);
      if (((init_ && ast_QualType_isConst(&t)) && ast_Expr_isCtv(init_))) ast_Expr_setCtv(e);
      switch (ast_VarDecl_getKind(vd)) {
      case ast_VarDeclKind_GlobalVar:
         // fallthrough
      case ast_VarDeclKind_LocalVar:
         // fallthrough
      case ast_VarDeclKind_FunctionParam:
         kind = ast_IdentifierKind_Var;
         break;
      case ast_VarDeclKind_StructMember:
         kind = ast_IdentifierKind_StructMember;
         break;
      }
      break;
   }
   case ast_DeclKind_StaticAssert:
      break;
   }
   return kind;
}

static bool module_analyser_Analyser_analyseArrayDesignatedInit(module_analyser_Analyser* ma, ast_Expr* e, ast_QualType expectedType)
{
   ast_ArrayDesignatedInitExpr* ad = ((ast_ArrayDesignatedInitExpr*)(e));
   ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, ast_ArrayDesignatedInitExpr_getDesignator2(ad), false);
   if (ast_QualType_isInvalid(&qt)) return false;

   ast_Expr* de = ast_ArrayDesignatedInitExpr_getDesignator(ad);
   if (!ast_Expr_isCtv(de)) {
      module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(de), ast_Expr_getRange(de), "array index is not a compile-time value");
      return false;
   }
   qt = module_analyser_Analyser_analyseExpr(ma, ast_ArrayDesignatedInitExpr_getInit2(ad), false);
   if (ast_QualType_isInvalid(&qt)) return false;

   ast_Expr* val = ast_ArrayDesignatedInitExpr_getInit(ad);
   if (!ma->curFunction) {
      if (!ast_Expr_isCtc(val)) {
         module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(val), ast_Expr_getRange(val), "initializer element is not a compile-time constant");
         return false;
      }
      if ((!ast_Expr_isCtv(val) && ast_QualType_needsCtvInit(&expectedType))) {
         module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(val), ast_Expr_getRange(val), "initializer element is not a compile-time value");
         return false;
      }
   }
   conversion_checker_Checker_check(&ma->checker, expectedType, qt, ast_ArrayDesignatedInitExpr_getInit2(ad), ast_Expr_getLoc(val));
   val = ast_ArrayDesignatedInitExpr_getInit(ad);
   ast_Expr_copyConstantFlags(e, val);
   ast_Expr_setType(e, expectedType);
   return true;
}

static ast_IdentifierKind module_analyser_getInnerExprAddressOf(ast_Expr* e)
{
   switch (ast_Expr_getKind(e)) {
   case ast_ExprKind_IntegerLiteral:
      // fallthrough
   case ast_ExprKind_BooleanLiteral:
      // fallthrough
   case ast_ExprKind_CharLiteral:
      // fallthrough
   case ast_ExprKind_StringLiteral:
      // fallthrough
   case ast_ExprKind_Nil:
      break;
   case ast_ExprKind_Identifier: {
      ast_IdentifierExpr* i = ((ast_IdentifierExpr*)(e));
      return ast_IdentifierExpr_getKind(i);
   }
   case ast_ExprKind_Type:
      // fallthrough
   case ast_ExprKind_Call:
      // fallthrough
   case ast_ExprKind_InitList:
      // fallthrough
   case ast_ExprKind_FieldDesignatedInit:
      // fallthrough
   case ast_ExprKind_ArrayDesignatedInit:
      break;
   case ast_ExprKind_BinaryOperator:
      assert(0);
      break;
   case ast_ExprKind_UnaryOperator:
      assert(0);
      break;
   case ast_ExprKind_ConditionalOperator:
      assert(0);
      break;
   case ast_ExprKind_Builtin:
      break;
   case ast_ExprKind_ArraySubscript: {
      ast_ArraySubscriptExpr* a = ((ast_ArraySubscriptExpr*)(e));
      return module_analyser_getInnerExprAddressOf(ast_ArraySubscriptExpr_getBase(a));
   }
   case ast_ExprKind_Member: {
      ast_MemberExpr* m = ((ast_MemberExpr*)(e));
      return ast_MemberExpr_getKind(m);
   }
   case ast_ExprKind_Paren: {
      ast_ParenExpr* p = ((ast_ParenExpr*)(e));
      return module_analyser_getInnerExprAddressOf(ast_ParenExpr_getInner(p));
   }
   case ast_ExprKind_BitOffset:
      return ast_IdentifierKind_Unresolved;
   case ast_ExprKind_ExplicitCast: {
      ast_ExplicitCastExpr* c = ((ast_ExplicitCastExpr*)(e));
      return module_analyser_getInnerExprAddressOf(ast_ExplicitCastExpr_getInner(c));
   }
   case ast_ExprKind_ImplicitCast: {
      ast_ImplicitCastExpr* c = ((ast_ImplicitCastExpr*)(e));
      return module_analyser_getInnerExprAddressOf(ast_ImplicitCastExpr_getInner(c));
   }
   }
   return ast_IdentifierKind_Unresolved;
}

static bool module_analyser_Analyser_getIdentifierKind(module_analyser_Analyser* ma, ast_Expr* e)
{
   ast_IdentifierKind kind = module_analyser_getInnerExprAddressOf(e);
   const char* arg = "";
   switch (kind) {
   case ast_IdentifierKind_Unresolved: {
      ast_QualType qt = ast_Expr_getType(e);
      module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "cannot take the address of an rvalue of type %s", ast_QualType_diagName(&qt));
      return false;
   }
   case ast_IdentifierKind_Module:
      arg = "a module";
      break;
   case ast_IdentifierKind_Function:
      arg = "a function";
      break;
   case ast_IdentifierKind_Type:
      arg = "a type";
      break;
   case ast_IdentifierKind_Var:
      return true;
   case ast_IdentifierKind_EnumConstant:
      arg = "an enum constant";
      break;
   case ast_IdentifierKind_StructMember:
      return true;
   case ast_IdentifierKind_Label:
      arg = "a label";
      break;
   }
   module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "cannot take the address of %s", arg);
   return false;
}

static ast_QualType module_analyser_Analyser_analyseUnaryOperator(module_analyser_Analyser* ma, ast_Expr** e_ptr)
{
   ast_Expr* e = *e_ptr;
   ast_UnaryOperator* u = ((ast_UnaryOperator*)(e));
   bool need_rvalue = true;
   switch (ast_UnaryOperator_getOpcode(u)) {
   case ast_UnaryOpcode_PostInc:
      // fallthrough
   case ast_UnaryOpcode_PostDec:
      // fallthrough
   case ast_UnaryOpcode_PreInc:
      // fallthrough
   case ast_UnaryOpcode_PreDec:
      if (!ma->curFunction) {
         module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(e), ast_Expr_getRange(e), "initializer element is not a compile-time constant");
         return ast_QualType_Invalid;
      }
      need_rvalue = false;
      break;
   case ast_UnaryOpcode_AddrOf:
      need_rvalue = false;
      break;
   case ast_UnaryOpcode_Deref:
      // fallthrough
   case ast_UnaryOpcode_Minus:
      // fallthrough
   case ast_UnaryOpcode_Not:
      // fallthrough
   case ast_UnaryOpcode_LNot:
      break;
   }
   ast_QualType t = module_analyser_Analyser_analyseExpr(ma, ast_UnaryOperator_getInner2(u), need_rvalue);
   if (ast_QualType_isInvalid(&t)) {
      return ast_QualType_Invalid;
   }
   e = *e_ptr;
   ast_Expr* inner = ast_UnaryOperator_getInner(u);
   if (ast_QualType_isVoidType(&t)) {
      module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "invalid argument type %s to unary expression", "'void'");
      return ast_QualType_Invalid;
   }
   switch (ast_UnaryOperator_getOpcode(u)) {
   case ast_UnaryOpcode_PostInc:
      if (!module_analyser_Analyser_checkAssignment(ma, inner, t, "increment operand", ast_Expr_getLoc(e))) return ast_QualType_Invalid;

      break;
   case ast_UnaryOpcode_PostDec:
      if (!module_analyser_Analyser_checkAssignment(ma, inner, t, "decrement operand", ast_Expr_getLoc(e))) return ast_QualType_Invalid;

      break;
   case ast_UnaryOpcode_PreInc:
      if (!module_analyser_Analyser_checkAssignment(ma, inner, t, "increment operand", ast_Expr_getLoc(e))) return ast_QualType_Invalid;

      break;
   case ast_UnaryOpcode_PreDec:
      if (!module_analyser_Analyser_checkAssignment(ma, inner, t, "decrement operand", ast_Expr_getLoc(e))) return ast_QualType_Invalid;

      break;
   case ast_UnaryOpcode_AddrOf: {
      if (!module_analyser_Analyser_getIdentifierKind(ma, inner)) return ast_QualType_Invalid;

      ast_QualType canon = ast_QualType_getCanonicalType(&t);
      t = ast_builder_Builder_actOnPointerType(ma->builder, canon);
      ast_Expr_setCtc(e);
      break;
   }
   case ast_UnaryOpcode_Deref:
      if (ast_QualType_isPointerType(&t)) {
         ast_Expr_setLValue(e);
         t = ast_QualType_getCanonicalType(&t);
         const ast_PointerType* p = ast_QualType_getPointerType(&t);
         return ast_PointerType_getInner(p);
      } else {
         module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "indirection requires pointer operand (%s invalid)", ast_QualType_diagName(&t));
         return ast_QualType_Invalid;
      }
      break;
   case ast_UnaryOpcode_Minus:
      // fallthrough
   case ast_UnaryOpcode_Not:
      ast_Expr_copyConstantFlags(e, inner);
      t = module_analyser_usualUnaryConversions(inner);
      break;
   case ast_UnaryOpcode_LNot:
      ast_Expr_copyConstantFlags(e, inner);
      return ast_g_bool;
   }
   return t;
}

static ast_QualType module_analyser_Analyser_analyseConditionalOperator(module_analyser_Analyser* ma, ast_Expr** e_ptr)
{
   ast_Expr* e = *e_ptr;
   ast_ConditionalOperator* cond = ((ast_ConditionalOperator*)(e));
   ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, ast_ConditionalOperator_getCond2(cond), true);
   if (ast_QualType_isInvalid(&qt)) return ast_QualType_Invalid;

   conversion_checker_Checker_check(&ma->checker, ast_g_bool, qt, ast_ConditionalOperator_getCond2(cond), ast_Expr_getLoc(ast_ConditionalOperator_getCond(cond)));
   ast_QualType lhs = module_analyser_Analyser_analyseExpr(ma, ast_ConditionalOperator_getLHS2(cond), true);
   ast_QualType rhs = module_analyser_Analyser_analyseExpr(ma, ast_ConditionalOperator_getRHS2(cond), true);
   if ((ast_QualType_isInvalid(&lhs) || ast_QualType_isInvalid(&rhs))) return ast_QualType_Invalid;

   ast_QualType lcanon = ast_QualType_getCanonicalType(&lhs);
   ast_QualType rcanon = ast_QualType_getCanonicalType(&rhs);
   assert(ast_QualType_isValid(&lcanon));
   assert(ast_QualType_isValid(&rcanon));
   uint8_t res = module_analyser_CondOpTable[ast_QualType_getKind(&lcanon)][ast_QualType_getKind(&rcanon)];
   switch (res) {
   case 0:
      break;
   case 1:
      module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "invalid operands to ternary operator (%s and %s)", ast_QualType_diagName(&lhs), ast_QualType_diagName(&rhs));
      return ast_QualType_Invalid;
   case 2:
      return lhs;
   case 3:
      return lhs;
   case 4:
      return lhs;
   case 5: {
      bool ok = conversion_checker_Checker_check(&ma->checker, lhs, rhs, e_ptr, ast_Expr_getLoc(e));
      if (!ok) return ast_QualType_Invalid;

      return lhs;
   }
   case 6:
      return rhs;
   case 7: {
      bool ok = conversion_checker_Checker_check(&ma->checker, lhs, rhs, e_ptr, ast_Expr_getLoc(e));
      if (!ok) return ast_QualType_Invalid;

      return lhs;
   }
   }
   assert(0);
   return ast_QualType_Invalid;
}

static bool module_analyser_Analyser_checkAssignment(module_analyser_Analyser* ma, ast_Expr* assignee, ast_QualType tleft, const char* msg, src_loc_SrcLoc loc)
{
   if (ast_QualType_isConst(&tleft)) {
      module_analyser_Analyser_error(ma, loc, "cannot assign to variable with const-qualified type '%s'", ast_QualType_diagName(&tleft));
      return false;
   }
   if (!ast_Expr_isLValue(assignee)) {
      module_analyser_Analyser_error(ma, loc, "lvalue required as %s", msg);
      return false;
   }
   return true;
}

static ast_QualType module_analyser_usualUnaryConversions(ast_Expr* e)
{
   ast_QualType qt = ast_Expr_getType(e);
   ast_QualType canon = ast_QualType_getCanonicalType(&qt);
   if (ast_QualType_isBuiltinType(&canon)) {
      ast_BuiltinType* bi = ast_QualType_getBuiltinType(&canon);
      if (ast_BuiltinType_isPromotableIntegerType(bi)) return ast_g_i32;

   } else if (ast_QualType_isPointerType(&canon)) {
      return ast_g_u64;
   }

   return qt;
}

static ast_QualType module_analyser_Analyser_analyseMemberExpr(module_analyser_Analyser* ma, ast_Expr** e_ptr)
{
   ast_Expr* e = *e_ptr;
   ast_MemberExpr* m = ((ast_MemberExpr*)(e));
   ast_Decl* base = NULL;
   src_loc_SrcLoc lastLoc = 0;
   ast_ValType valtype = ast_ValType_NValue;
   ast_CallKind ck = ast_CallKind_Invalid;
   ast_Expr* exprBase = ast_MemberExpr_getExprBase(m);
   if (exprBase) {
      ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, &exprBase, false);
      if (ast_QualType_isInvalid(&qt)) return ast_QualType_Invalid;

      base = module_analyser_Analyser_analyseMemberBase(ma, ast_Expr_getLoc(exprBase), qt, false);
      if (!base) return ast_QualType_Invalid;

      valtype = ast_Expr_getValType(exprBase);
   }
   uint32_t refcount = ast_MemberExpr_getNumRefs(m);
   for (uint32_t i = 0; i < refcount; i++) {
      uint32_t name_idx = ast_MemberExpr_getNameIdx(m, i);
      src_loc_SrcLoc loc = ast_MemberExpr_getLoc(m, i);
      ast_Decl* d = NULL;
      if (!base) {
         d = scope_Scope_find(ma->scope, name_idx, loc);
      } else {
         if (ast_Decl_getKind(base) == ast_DeclKind_Var) {
            base = module_analyser_Analyser_analyseMemberBase(ma, loc, ast_Decl_getType(base), false);
            if (!base) return ast_QualType_Invalid;

            valtype = ast_ValType_LValue;
         }
         ast_QualType baseType = ast_Decl_getType(base);
         switch (ast_Decl_getKind(base)) {
         case ast_DeclKind_Function:
            valtype = ast_ValType_RValue;
            break;
         case ast_DeclKind_Import: {
            ast_ImportDecl* id = ((ast_ImportDecl*)(base));
            d = scope_Scope_findGlobalSymbolInModule(ma->scope, ast_ImportDecl_getDest(id), name_idx, loc);
            break;
         }
         case ast_DeclKind_StructType: {
            ast_StructType* st = ast_QualType_getStructType(&baseType);
            d = module_analyser_Analyser_findStructMember(ma, ast_StructType_getDecl(st), name_idx, loc, true);
            if (d) {
               if (ast_Decl_getKind(d) == ast_DeclKind_Function) {
                  ast_FunctionDecl* fd = ((ast_FunctionDecl*)(d));
                  ast_CallKind callkind = ast_FunctionDecl_getCallKind(fd);
                  assert(callkind != ast_CallKind_Normal);
                  switch (valtype) {
                  case ast_ValType_NValue:
                     if (callkind != ast_CallKind_StaticStructFunc) {
                     }
                     ck = ast_CallKind_StaticStructFunc;
                     break;
                  case ast_ValType_RValue:
                     if (callkind == ast_CallKind_StaticStructFunc) {
                        module_analyser_Analyser_error(ma, loc, "cannot access static struct function through variable");
                        return ast_QualType_Invalid;
                     }
                     assert(callkind == ast_CallKind_StructFunc);
                     ck = ast_CallKind_StructFunc;
                     break;
                  case ast_ValType_LValue:
                     if (callkind == ast_CallKind_StaticStructFunc) {
                        module_analyser_Analyser_error(ma, loc, "cannot access static struct function through variable");
                        return ast_QualType_Invalid;
                     }
                     assert(callkind == ast_CallKind_StructFunc);
                     ck = ast_CallKind_StructFunc;
                     break;
                  }
               } else {
                  if (!module_analyser_Analyser_checkOpaque(ma, ast_StructType_getDecl(st), loc)) return ast_QualType_Invalid;

                  if (valtype == ast_ValType_NValue) {
                     module_analyser_Analyser_error(ma, loc, "member access needs an instantiation of type '%s'", ast_QualType_diagName(&baseType));
                     return ast_QualType_Invalid;
                  }
               }
            }
            break;
         }
         case ast_DeclKind_EnumType: {
            ast_EnumTypeDecl* etd = ((ast_EnumTypeDecl*)(base));
            ast_EnumConstantDecl* ecd = ast_EnumTypeDecl_findConstant(etd, name_idx);
            if (!ecd) {
               module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "enum %s has no constant %s", ast_Decl_getName(base), ast_MemberExpr_getName(m, 0));
               return ast_QualType_Invalid;
            }
            d = ((ast_Decl*)(ecd));
            break;
         }
         case ast_DeclKind_EnumConstant:
            module_analyser_Analyser_error(ma, lastLoc, "invalid member reference base");
            return ast_QualType_Invalid;
         case ast_DeclKind_FunctionType:
            assert(0);
            break;
         case ast_DeclKind_AliasType:
            assert(0);
            break;
         case ast_DeclKind_Var:
            assert(0);
            break;
         case ast_DeclKind_StaticAssert:
            assert(0);
            break;
         }
      }
      if (!d) {
         ma->has_error = true;
         return ast_QualType_Invalid;
      }
      lastLoc = loc;
      if (!ast_Decl_isChecked(d)) {
         module_analyser_Analyser_analyseGlobalDecl(ma, d);
      }
      ast_Decl_setUsed(d);
      ast_MemberExpr_setDecl(m, d, i);
      base = d;
   }
   if ((ck == ast_CallKind_Invalid && ast_Decl_getKind(base) == ast_DeclKind_Function)) ck = ast_CallKind_Normal;
   if (ck == ast_CallKind_StructFunc) ast_MemberExpr_setIsStructFunc(m);
   if (ck == ast_CallKind_StaticStructFunc) ast_MemberExpr_setIsStaticStructFunc(m);
   ast_IdentifierKind kind = module_analyser_Analyser_setExprFlags(ma, e_ptr, base);
   ast_MemberExpr_setKind(m, kind);
   ast_Expr_setType(e, ast_Decl_getType(base));
   if (ast_Expr_isCtv(e)) ast_Expr_setRValue(e);
   return ast_Decl_getType(base);
}

static ast_Decl* module_analyser_Analyser_analyseMemberBase(module_analyser_Analyser* ma, src_loc_SrcLoc loc, ast_QualType baseType, bool ptr_deref)
{
   const ast_Type* t = ast_QualType_getTypeOrNil(&baseType);
   switch (ast_Type_getKind(t)) {
   case ast_TypeKind_Builtin:
      break;
   case ast_TypeKind_Pointer: {
      if (ptr_deref) break;

      ast_PointerType* pt = ((ast_PointerType*)(t));
      return module_analyser_Analyser_analyseMemberBase(ma, loc, ast_PointerType_getInner(pt), true);
   }
   case ast_TypeKind_Array:
      break;
   case ast_TypeKind_Struct: {
      ast_StructType* st = ast_QualType_getStructType(&baseType);
      ast_StructTypeDecl* std = ast_StructType_getDecl(st);
      if (!module_analyser_Analyser_checkOpaque(ma, std, loc)) return NULL;

      return ((ast_Decl*)(std));
   }
   case ast_TypeKind_Enum:
      assert(0);
      break;
   case ast_TypeKind_Function:
      break;
   case ast_TypeKind_Alias:
      assert(0);
      break;
   case ast_TypeKind_Module:
      assert(0);
      break;
   }
   module_analyser_Analyser_error(ma, loc, "invalid member reference base");
   return NULL;
}

static ast_QualType module_analyser_Analyser_analyseExplicitCast(module_analyser_Analyser* ma, ast_Expr** e_ptr)
{
   ast_Expr* e = *e_ptr;
   ast_ExplicitCastExpr* c = ((ast_ExplicitCastExpr*)(e));
   ast_TypeRef* ref = ast_ExplicitCastExpr_getTypeRef(c);
   ast_QualType destType = module_analyser_Analyser_analyseTypeRef(ma, ref);
   ast_QualType srcType = module_analyser_Analyser_analyseExpr(ma, ast_ExplicitCastExpr_getInner2(c), true);
   if ((ast_QualType_isInvalid(&srcType) || ast_QualType_isInvalid(&destType))) return ast_QualType_Invalid;

   ast_Expr* inner = ast_ExplicitCastExpr_getInner(c);
   ast_Expr_copyConstantFlags(e, inner);
   ast_Expr_copyValType(e, inner);
   return destType;
}

static ast_QualType module_analyser_Analyser_analyseBuiltin(module_analyser_Analyser* ma, ast_Expr** e_ptr)
{
   ast_Expr* e = *e_ptr;
   ast_BuiltinExpr* b = ((ast_BuiltinExpr*)(e));
   switch (ast_BuiltinExpr_getKind(b)) {
   case ast_BuiltinExprKind_Sizeof:
      return module_analyser_Analyser_analyseSizeof(ma, b);
   case ast_BuiltinExprKind_Elemsof:
      return module_analyser_Analyser_analyseElemsof(ma, b);
   case ast_BuiltinExprKind_EnumMin:
      // fallthrough
   case ast_BuiltinExprKind_EnumMax:
      return module_analyser_Analyser_analyseEnumMinMax(ma, b);
   case ast_BuiltinExprKind_OffsetOf:
      return module_analyser_Analyser_analyseOffsetOf(ma, b);
   case ast_BuiltinExprKind_ToContainer:
      return module_analyser_Analyser_analyseToContainer(ma, b);
   }
   return ast_QualType_Invalid;
}

static ast_QualType module_analyser_Analyser_analyseArraySubscriptExpr(module_analyser_Analyser* ma, ast_Expr** e_ptr)
{
   ast_Expr* e = *e_ptr;
   ast_ArraySubscriptExpr* sub = ((ast_ArraySubscriptExpr*)(e));
   ast_QualType q = module_analyser_Analyser_analyseExpr(ma, ast_ArraySubscriptExpr_getBase2(sub), true);
   if (ast_QualType_isInvalid(&q)) return q;

   if (!ast_QualType_isPointerType(&q)) {
      module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(e), ast_Expr_getRange(e), "subscripted value is not an array or pointer");
      return ast_QualType_Invalid;
   }
   ast_QualType qidx = module_analyser_Analyser_analyseExpr(ma, ast_ArraySubscriptExpr_getIndex2(sub), true);
   if (ast_QualType_isInvalid(&qidx)) return qidx;

   ast_PointerType* pt = ast_QualType_getPointerType(&q);
   return ast_PointerType_getInner(pt);
}

static ast_QualType module_analyser_Analyser_analyseEnumMinMax(module_analyser_Analyser* ma, ast_BuiltinExpr* b)
{
   ast_Expr* inner = ast_BuiltinExpr_getInner(b);
   ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, &inner, false);
   if (ast_QualType_isInvalid(&qt)) return ast_QualType_Invalid;

   ast_EnumType* et = ast_QualType_getEnumTypeOrNil(&qt);
   if (!et) {
      const char* kind = (ast_BuiltinExpr_getKind(b) == ast_BuiltinExprKind_EnumMin) ? "enum_min" : "enum_max";
      module_analyser_Analyser_error(ma, ast_Expr_getLoc(inner), "%s can only be used on enum types", kind);
      return ast_QualType_Invalid;
   }
   ast_EnumTypeDecl* etd = ast_EnumType_getDecl(et);
   uint32_t num = ast_EnumTypeDecl_getNumConstants(etd);
   ast_EnumConstantDecl** constants = ast_EnumTypeDecl_getConstants(etd);
   uint32_t index = 0;
   if (ast_BuiltinExpr_getKind(b) == ast_BuiltinExprKind_EnumMax) index = num - 1;
   ast_BuiltinExpr_setValue(b, ast_EnumConstantDecl_getValue(constants[index]));
   return ast_EnumTypeDecl_getImplType(etd);
}

static uint32_t module_analyser_decl2offset(const ast_Decl* d)
{
   if (ast_Decl_getKind(d) == ast_DeclKind_Var) {
      ast_VarDecl* vd = ((ast_VarDecl*)(d));
      return ast_VarDecl_getOffset(vd);
   }
   assert(ast_Decl_getKind(d) == ast_DeclKind_StructType);
   ast_StructTypeDecl* std = ((ast_StructTypeDecl*)(d));
   return ast_StructTypeDecl_getOffset(std);
}

static ast_Decl* module_analyser_Analyser_findMemberOffset(module_analyser_Analyser* ma, ast_BuiltinExpr* b, ast_StructTypeDecl* std, ast_Expr* member)
{
   uint32_t base_offset = 0;
   ast_Decl* d = NULL;
   if (ast_Expr_getKind(member) == ast_ExprKind_Identifier) {
      ast_IdentifierExpr* i = ((ast_IdentifierExpr*)(member));
      uint32_t name_idx = ast_IdentifierExpr_getNameIdx(i);
      d = module_analyser_Analyser_findStructMemberOffset(ma, std, name_idx, ast_Expr_getLoc(member), &base_offset);
      if (!d) return NULL;

      base_offset += module_analyser_decl2offset(d);
      ast_IdentifierExpr_setDecl(i, d);
      ast_Decl_setUsed(d);
      ast_Expr_setLValue(member);
      ast_IdentifierExpr_setKind(i, ast_IdentifierKind_StructMember);
   } else {
      assert(ast_Expr_getKind(member) == ast_ExprKind_Member);
      ast_MemberExpr* m = ((ast_MemberExpr*)(member));
      for (uint32_t i = 0; i < ast_MemberExpr_getNumRefs(m); i++) {
         uint32_t name_idx = ast_MemberExpr_getNameIdx(m, i);
         src_loc_SrcLoc loc = ast_MemberExpr_getLoc(m, i);
         d = module_analyser_Analyser_findStructMemberOffset(ma, std, name_idx, loc, &base_offset);
         if (!d) return NULL;

         if (ast_Decl_getKind(d) == ast_DeclKind_StructType) std = ((ast_StructTypeDecl*)(d));
         base_offset += module_analyser_decl2offset(d);
         ast_Decl_setUsed(d);
         ast_MemberExpr_setDecl(m, d, i);
      }
      ast_MemberExpr_setKind(m, ast_IdentifierKind_StructMember);
   }
   ast_Expr_setType(member, ast_Decl_getType(d));
   ast_BuiltinExpr_setValue(b, base_offset);
   return d;
}

static ast_QualType module_analyser_Analyser_analyseOffsetOf(module_analyser_Analyser* ma, ast_BuiltinExpr* b)
{
   ast_Expr* e = ((ast_Expr*)(b));
   ast_Expr* inner = ast_BuiltinExpr_getInner(b);
   ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, &inner, false);
   if (ast_QualType_isInvalid(&qt)) return ast_QualType_Invalid;

   ast_Expr_setType(e, ast_g_u32);
   ast_StructType* st = ast_QualType_getStructTypeOrNil(&qt);
   if (!st) {
      module_analyser_Analyser_error(ma, ast_Expr_getLoc(inner), "offsetof can only be used on struct types");
      return ast_QualType_Invalid;
   }
   ast_StructTypeDecl* std = ast_StructType_getDecl(st);
   if (!module_analyser_Analyser_checkOpaque(ma, std, ast_Expr_getLoc(inner))) return ast_QualType_Invalid;

   ast_Expr* member = ast_BuiltinExpr_getOffsetOfMember(b);
   ast_Decl* d = module_analyser_Analyser_findMemberOffset(ma, b, std, member);
   if (!d) return ast_QualType_Invalid;

   return ast_Expr_getType(e);
}

static ast_QualType module_analyser_Analyser_analyseToContainer(module_analyser_Analyser* ma, ast_BuiltinExpr* b)
{
   ast_Expr* inner = ast_BuiltinExpr_getInner(b);
   ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, &inner, false);
   if (ast_QualType_isInvalid(&qt)) return ast_QualType_Invalid;

   ast_StructType* st = ast_QualType_getStructTypeOrNil(&qt);
   if (!st) {
      module_analyser_Analyser_error(ma, ast_Expr_getLoc(inner), "to_container can only be used on struct types");
      return ast_QualType_Invalid;
   }
   ast_StructTypeDecl* std = ast_StructType_getDecl(st);
   if (!module_analyser_Analyser_checkOpaque(ma, std, ast_Expr_getLoc(inner))) return ast_QualType_Invalid;

   ast_Expr* member = ast_BuiltinExpr_getToContainerMember(b);
   ast_Decl* d = module_analyser_Analyser_findMemberOffset(ma, b, std, member);
   if (!d) return ast_QualType_Invalid;

   ast_QualType qmem = ast_Decl_getType(d);
   qmem = ast_builder_Builder_actOnPointerType(ma->builder, qmem);
   ast_QualType qptr = module_analyser_Analyser_analyseExpr(ma, ast_BuiltinExpr_getToContainerPointer2(b), false);
   if (ast_QualType_isInvalid(&qptr)) return ast_QualType_Invalid;

   if (ast_QualType_getType(&qmem) != ast_QualType_getType(&qptr)) {
      ast_Expr* ptr = ast_BuiltinExpr_getToContainerPointer(b);
      module_analyser_Analyser_error(ma, ast_Expr_getLoc(ptr), "3rd argument to to_container must be of type '%s'", ast_QualType_diagName(&qmem));
      return ast_QualType_Invalid;
   }
   return ast_builder_Builder_actOnPointerType(ma->builder, qt);
}

static ast_QualType module_analyser_Analyser_analyseSizeof(module_analyser_Analyser* ma, ast_BuiltinExpr* e)
{
   ast_Expr* inner = ast_BuiltinExpr_getInner(e);
   assert(inner);
   ast_QualType qt;
   if (ast_Expr_getKind(inner) == ast_ExprKind_Type) {
      ast_TypeExpr* te = ((ast_TypeExpr*)(inner));
      ast_TypeRef* ref = ast_TypeExpr_getTypeRef(te);
      qt = module_analyser_Analyser_analyseTypeRef(ma, ref);
      ast_Expr_setType(inner, qt);
   } else {
      qt = module_analyser_Analyser_analyseExpr(ma, &inner, false);
   }
   if (ast_QualType_isInvalid(&qt)) return ast_QualType_Invalid;

   size_analyser_TypeSize info = size_analyser_sizeOfType(qt);
   ast_BuiltinExpr_setValue(e, info.size);
   return ast_g_u32;
}

static ast_QualType module_analyser_Analyser_analyseElemsof(module_analyser_Analyser* ma, ast_BuiltinExpr* b)
{
   ast_Expr* inner = ast_BuiltinExpr_getInner(b);
   ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, &inner, false);
   if (ast_QualType_isInvalid(&qt)) return qt;

   const ast_ArrayType* at = ast_QualType_getArrayTypeOrNil(&qt);
   if (at) {
      ast_BuiltinExpr_setValue(b, ast_ArrayType_getSize(at));
      return ast_g_u32;
   }
   const ast_EnumType* et = ast_QualType_getEnumTypeOrNil(&qt);
   if (et) {
      const ast_EnumTypeDecl* etd = ast_EnumType_getDecl(et);
      ast_BuiltinExpr_setValue(b, ast_EnumTypeDecl_getNumConstants(etd));
      return ast_g_u32;
   }
   module_analyser_Analyser_error(ma, ast_Expr_getLoc(inner), "elemsof can only be used on arrays/enums");
   return ast_QualType_Invalid;
}

static ast_Decl* module_analyser_Analyser_findStructMember(module_analyser_Analyser* ma, ast_StructTypeDecl* s, uint32_t name_idx, src_loc_SrcLoc loc, bool allow_funcs)
{
   ast_Decl* d = ast_StructTypeDecl_findAny(s, name_idx);
   if ((!d || ((!allow_funcs && ast_Decl_getKind(d) == ast_DeclKind_Function)))) {
      module_analyser_Analyser_error(ma, loc, "%s %s does not have a member %s", ast_StructTypeDecl_isStruct(s) ? "struct" : "union", ast_Decl_getName(ast_StructTypeDecl_asDecl(s)), ast_idx2name(name_idx));
      return NULL;
   }
   return d;
}

static ast_Decl* module_analyser_Analyser_findStructMemberOffset(module_analyser_Analyser* ma, ast_StructTypeDecl* s, uint32_t name_idx, src_loc_SrcLoc loc, uint32_t* base)
{
   ast_Decl* d = ast_StructTypeDecl_findMember(s, name_idx, base);
   if (!d) {
      module_analyser_Analyser_error(ma, loc, "%s %s does not have a member %s", ast_StructTypeDecl_isStruct(s) ? "struct" : "union", ast_Decl_getName(ast_StructTypeDecl_asDecl(s)), ast_idx2name(name_idx));
      return NULL;
   }
   return d;
}

static bool module_analyser_validBinOpKind(ast_QualType t)
{
   switch (ast_QualType_getKind(&t)) {
   case ast_TypeKind_Builtin:
      return true;
   case ast_TypeKind_Pointer:
      return true;
   case ast_TypeKind_Array:
      assert(0);
      break;
   case ast_TypeKind_Struct:
      return true;
   case ast_TypeKind_Enum:
      return true;
   case ast_TypeKind_Function:
      break;
   case ast_TypeKind_Alias:
      return module_analyser_validBinOpKind(ast_QualType_getCanonicalType(&t));
   case ast_TypeKind_Module:
      break;
   }
   return false;
}

static ast_QualType module_analyser_Analyser_checkBinopIntArgs(module_analyser_Analyser* ma, ast_BinaryOperator* b, ast_QualType lhs, ast_QualType rhs)
{
   ast_QualType lcanon = ast_QualType_getCanonicalType(&lhs);
   ast_QualType rcanon = ast_QualType_getCanonicalType(&rhs);
   assert(ast_QualType_isValid(&lcanon));
   assert(ast_QualType_isValid(&rcanon));
   ast_BuiltinType* lb = ast_QualType_getBuiltinTypeOrNil(&lcanon);
   ast_BuiltinType* rb = ast_QualType_getBuiltinTypeOrNil(&rcanon);
   if ((((!lb || !rb) || ast_QualType_isVoidType(&lcanon)) || ast_QualType_isVoidType(&lcanon))) {
      ast_Expr* e = ((ast_Expr*)(b));
      module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "invalid operands to binary expression (%s and %s)", ast_QualType_diagName(&lhs), ast_QualType_diagName(&rhs));
      return ast_QualType_Invalid;
   }
   return ast_g_i32;
}

static ast_QualType module_analyser_Analyser_checkBinopLogical(module_analyser_Analyser* ma, ast_BinaryOperator* b, ast_QualType lhs, ast_QualType rhs)
{
   bool ok = conversion_checker_Checker_check(&ma->checker, ast_g_bool, lhs, ast_BinaryOperator_getLHS2(b), ast_Expr_getLoc(ast_BinaryOperator_getLHS(b)));
   ok &= conversion_checker_Checker_check(&ma->checker, ast_g_bool, rhs, ast_BinaryOperator_getRHS2(b), ast_Expr_getLoc(ast_BinaryOperator_getRHS(b)));
   if (ok) return ast_g_bool;

   ast_Expr* e = ((ast_Expr*)(b));
   module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "invalid operands to binary expression (%s and %s)", ast_QualType_diagName(&lhs), ast_QualType_diagName(&rhs));
   return ast_QualType_Invalid;
}

static ast_QualType module_analyser_Analyser_checkBinopAddSubAssign(module_analyser_Analyser* ma, ast_BinaryOperator* b, ast_QualType lhs, ast_QualType rhs)
{
   ast_QualType lcanon = ast_QualType_getCanonicalType(&lhs);
   ast_QualType rcanon = ast_QualType_getCanonicalType(&rhs);
   assert(ast_QualType_isValid(&lcanon));
   assert(ast_QualType_isValid(&rcanon));
   uint8_t res = module_analyser_BinOpConvAddSubAss[ast_QualType_getKind(&lcanon)][ast_QualType_getKind(&rcanon)];
   switch (res) {
   case 0:
      break;
   case 1: {
      ast_Expr* e = ((ast_Expr*)(b));
      module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "invalid operands to binary expression (%s and %s)", ast_QualType_diagName(&lhs), ast_QualType_diagName(&rhs));
      return ast_QualType_Invalid;
   }
   case 2:
      return lhs;
   case 3:
      return lhs;
   case 4:
      return lhs;
   case 5:
      return lhs;
   case 6:
      return lhs;
   }
   assert(0);
   return ast_QualType_Invalid;
}

static ast_QualType module_analyser_Analyser_checkBinopAddArgs(module_analyser_Analyser* ma, ast_BinaryOperator* b, ast_QualType lhs, ast_QualType rhs)
{
   ast_QualType lcanon = ast_QualType_getCanonicalType(&lhs);
   ast_QualType rcanon = ast_QualType_getCanonicalType(&rhs);
   assert(ast_QualType_isValid(&lcanon));
   assert(ast_QualType_isValid(&rcanon));
   uint8_t res = module_analyser_BinOpConvAdd[ast_QualType_getKind(&lcanon)][ast_QualType_getKind(&rcanon)];
   switch (res) {
   case 0:
      break;
   case 1: {
      ast_Expr* e = ((ast_Expr*)(b));
      module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "invalid operands to binary expression (%s and %s)", ast_QualType_diagName(&lhs), ast_QualType_diagName(&rhs));
      return ast_QualType_Invalid;
   }
   case 2:
      return lhs;
   case 3:
      return rhs;
   case 4:
      return lhs;
   case 5:
      return rhs;
   case 6:
      return lhs;
   case 7:
      return ast_g_u32;
   }
   assert(0);
   return ast_QualType_Invalid;
}

static ast_QualType module_analyser_Analyser_checkBinopSubArgs(module_analyser_Analyser* ma, ast_BinaryOperator* b, ast_QualType lhs, ast_QualType rhs)
{
   ast_QualType lcanon = ast_QualType_getCanonicalType(&lhs);
   ast_QualType rcanon = ast_QualType_getCanonicalType(&rhs);
   assert(ast_QualType_isValid(&lcanon));
   assert(ast_QualType_isValid(&rcanon));
   uint8_t res = module_analyser_BinOpConvSub[ast_QualType_getKind(&lcanon)][ast_QualType_getKind(&rcanon)];
   switch (res) {
   case 0:
      break;
   case 1: {
      ast_Expr* e = ((ast_Expr*)(b));
      module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "invalid operands to binary expression (%s and %s)", ast_QualType_diagName(&lhs), ast_QualType_diagName(&rhs));
      return ast_QualType_Invalid;
   }
   case 2:
      return lhs;
   case 3:
      return lhs;
   case 4:
      return lhs;
   case 5:
      return ast_g_isize;
   case 6:
      return lhs;
   case 7:
      return ast_g_i32;
   }
   assert(0);
   return ast_QualType_Invalid;
}

static ast_QualType module_analyser_Analyser_checkBinopComparison(module_analyser_Analyser* ma, ast_BinaryOperator* b, ast_QualType lhs, ast_QualType rhs)
{
   ast_QualType lcanon = ast_QualType_getCanonicalType(&lhs);
   ast_QualType rcanon = ast_QualType_getCanonicalType(&rhs);
   assert(ast_QualType_isValid(&lcanon));
   assert(ast_QualType_isValid(&rcanon));
   uint8_t res = module_analyser_BinOpConvComparision[ast_QualType_getKind(&lcanon)][ast_QualType_getKind(&rcanon)];
   switch (res) {
   case 0:
      break;
   case 1: {
      ast_Expr* e = ((ast_Expr*)(b));
      module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "invalid operands to binary expression (%s and %s)", ast_QualType_diagName(&lhs), ast_QualType_diagName(&rhs));
      return ast_QualType_Invalid;
   }
   case 2:
      return ast_g_bool;
   case 3:
      return ast_g_bool;
   case 4:
      return ast_g_bool;
   case 5:
      return ast_g_bool;
   case 6:
      if (ast_QualType_getTypeOrNil(&lcanon) != ast_QualType_getTypeOrNil(&rcanon)) {
         ast_Expr* e = ((ast_Expr*)(b));
         module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "comparing enums of different types (%s and %s)", ast_QualType_diagName(&lhs), ast_QualType_diagName(&rhs));
         return ast_QualType_Invalid;
      }
      return ast_g_bool;
   }
   assert(0);
   return ast_QualType_Invalid;
}

static ast_QualType module_analyser_Analyser_analyseBinaryOperator(module_analyser_Analyser* ma, ast_Expr** e_ptr)
{
   ast_Expr* e = *e_ptr;
   ast_BinaryOperator* b = ((ast_BinaryOperator*)(e));
   bool need_lhs_rvalue = true;
   if (ast_BinaryOperator_getOpcode(b) >= ast_BinaryOpcode_Assign) need_lhs_rvalue = false;
   ast_QualType ltype = module_analyser_Analyser_analyseExpr(ma, ast_BinaryOperator_getLHS2(b), need_lhs_rvalue);
   if (ast_QualType_isInvalid(&ltype)) return ast_QualType_Invalid;

   ast_QualType rtype = module_analyser_Analyser_analyseExpr(ma, ast_BinaryOperator_getRHS2(b), true);
   if (ast_QualType_isInvalid(&rtype)) return ast_QualType_Invalid;

   ast_Expr* lhs = ast_BinaryOperator_getLHS(b);
   ast_Expr* rhs = ast_BinaryOperator_getRHS(b);
   if ((((!module_analyser_validBinOpKind(ltype) || !module_analyser_validBinOpKind(rtype)) || ast_QualType_isVoidType(&ltype)) || ast_QualType_isVoidType(&rtype))) {
      ast_QualType tl = ast_Expr_getType(lhs);
      ast_QualType tr = ast_Expr_getType(rhs);
      module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "invalid operands to binary expression (%s and %s)", ast_QualType_diagName(&tl), ast_QualType_diagName(&tr));
      return ast_QualType_Invalid;
   }
   ast_QualType result = ast_QualType_Invalid;
   switch (ast_BinaryOperator_getOpcode(b)) {
   case ast_BinaryOpcode_Multiply:
      // fallthrough
   case ast_BinaryOpcode_Divide:
      // fallthrough
   case ast_BinaryOpcode_Reminder:
      result = module_analyser_Analyser_checkBinopIntArgs(ma, b, ltype, rtype);
      break;
   case ast_BinaryOpcode_Add:
      result = module_analyser_Analyser_checkBinopAddArgs(ma, b, ltype, rtype);
      break;
   case ast_BinaryOpcode_Subtract:
      result = module_analyser_Analyser_checkBinopSubArgs(ma, b, ltype, rtype);
      break;
   case ast_BinaryOpcode_ShiftLeft:
      // fallthrough
   case ast_BinaryOpcode_ShiftRight:
      result = module_analyser_Analyser_checkBinopIntArgs(ma, b, ltype, rtype);
      break;
   case ast_BinaryOpcode_LessThan:
      // fallthrough
   case ast_BinaryOpcode_GreaterThan:
      // fallthrough
   case ast_BinaryOpcode_LessEqual:
      // fallthrough
   case ast_BinaryOpcode_GreaterEqual:
      // fallthrough
   case ast_BinaryOpcode_Equal:
      // fallthrough
   case ast_BinaryOpcode_NotEqual:
      result = module_analyser_Analyser_checkBinopComparison(ma, b, ltype, rtype);
      break;
   case ast_BinaryOpcode_And:
      // fallthrough
   case ast_BinaryOpcode_Xor:
      // fallthrough
   case ast_BinaryOpcode_Or:
      result = module_analyser_Analyser_checkBinopIntArgs(ma, b, ltype, rtype);
      break;
   case ast_BinaryOpcode_LAnd:
      // fallthrough
   case ast_BinaryOpcode_LOr:
      result = module_analyser_Analyser_checkBinopLogical(ma, b, ltype, rtype);
      break;
   case ast_BinaryOpcode_Assign: {
      bool ok = conversion_checker_Checker_check(&ma->checker, ltype, rtype, ast_BinaryOperator_getRHS2(b), ast_Expr_getLoc(e));
      if (ok) result = ltype;
      break;
   }
   case ast_BinaryOpcode_MulAssign:
      // fallthrough
   case ast_BinaryOpcode_DivAssign:
      // fallthrough
   case ast_BinaryOpcode_RemAssign:
      result = module_analyser_Analyser_checkBinopIntArgs(ma, b, ltype, rtype);
      break;
   case ast_BinaryOpcode_AddAssign:
      // fallthrough
   case ast_BinaryOpcode_SubAssign:
      result = module_analyser_Analyser_checkBinopAddSubAssign(ma, b, ltype, rtype);
      break;
   case ast_BinaryOpcode_ShlAssign:
      // fallthrough
   case ast_BinaryOpcode_ShrASsign:
      // fallthrough
   case ast_BinaryOpcode_AndAssign:
      // fallthrough
   case ast_BinaryOpcode_XorAssign:
      // fallthrough
   case ast_BinaryOpcode_OrAssign:
      result = module_analyser_Analyser_checkBinopIntArgs(ma, b, ltype, rtype);
      break;
   }
   ast_Expr_combineConstantFlags(e, lhs, rhs);
   return result;
}

static void module_analyser_Analyser_analyseFunctionBody(module_analyser_Analyser* ma, ast_FunctionDecl* fd, scope_Scope* s)
{
   if (ast_FunctionDecl_isTemplate(fd)) return;

   ast_CompoundStmt* body = ast_FunctionDecl_getBody(fd);
   if (!body) return;

   ast_Decl* d = ((ast_Decl*)(fd));
   module_analyser_Analyser_pushCheck(ma, d, s, fd);
   scope_Scope_reset(ma->scope);
   scope_Scope_enter(ma->scope, (scope_Function | scope_Decl));
   uint32_t num_params = ast_FunctionDecl_getNumParams(fd);
   ast_VarDecl** params = ast_FunctionDecl_getParams(fd);
   for (uint32_t i = 0; i < num_params; i++) {
      ast_Decl* p = ((ast_Decl*)(params[i]));
      if (ast_Decl_getNameIdx(p)) {
         bool error = scope_Scope_add(ma->scope, p);
         if (error) return;

      }
   }
   ma->has_error = false;
   module_analyser_Analyser_analyseCompoundStmt(ma, body);
   ast_QualType rtype = ast_FunctionDecl_getRType(fd);
   if (!ast_QualType_isVoidType(&rtype)) {
      ast_Stmt* last = ast_CompoundStmt_getLastStmt(body);
      if ((!last || (ast_Stmt_getKind(last) != ast_StmtKind_Return))) {
         module_analyser_Analyser_error(ma, ast_CompoundStmt_getEndLoc(body), "control reaches end of non-void function");
      }
   }
   if (!ma->warnings->no_unused_parameter) {
      for (uint32_t i = 0; i < num_params; i++) {
         ast_Decl* p = ((ast_Decl*)(params[i]));
         if ((!ast_Decl_isUsed(p) && ast_Decl_getNameIdx(p))) {
            module_analyser_Analyser_warn(ma, ast_Decl_getLoc(p), "unused parameter");
         }
      }
   }
   scope_Scope_exit(ma->scope);
   module_analyser_Analyser_popCheck(ma);
}

static void module_analyser_Analyser_analyseStmt(module_analyser_Analyser* ma, ast_Stmt* s, bool checkEffect)
{
   switch (ast_Stmt_getKind(s)) {
   case ast_StmtKind_Return:
      module_analyser_Analyser_analyseReturnStmt(ma, s);
      break;
   case ast_StmtKind_Expr: {
      module_analyser_Analyser_analyseExpr(ma, ((ast_Expr**)(&s)), false);
      ast_Expr* e = ((ast_Expr*)(s));
      if ((checkEffect && !ast_Expr_hasEffect(e))) module_analyser_Analyser_errorRange(ma, ast_Expr_getLoc(e), ast_Expr_getRange(e), "expression without effect");
      break;
   }
   case ast_StmtKind_If:
      module_analyser_Analyser_analyseIfStmt(ma, s);
      break;
   case ast_StmtKind_While:
      module_analyser_Analyser_analyseWhileStmt(ma, s);
      break;
   case ast_StmtKind_Do:
      module_analyser_Analyser_analyseDoStmt(ma, s);
      break;
   case ast_StmtKind_For:
      module_analyser_Analyser_analyseForStmt(ma, s);
      break;
   case ast_StmtKind_Switch:
      module_analyser_Analyser_analyseSwitchStmt(ma, s);
      break;
   case ast_StmtKind_Case:
      assert(0);
      break;
   case ast_StmtKind_Default:
      assert(0);
      break;
   case ast_StmtKind_Break:
      module_analyser_Analyser_analyseBreakStmt(ma, s);
      break;
   case ast_StmtKind_Continue:
      module_analyser_Analyser_analyseContinueStmt(ma, s);
      break;
   case ast_StmtKind_Fallthrough:
      module_analyser_Analyser_analyseFallthroughStmt(ma, s);
      break;
   case ast_StmtKind_Label:
      break;
   case ast_StmtKind_Goto:
      break;
   case ast_StmtKind_Compound:
      scope_Scope_enter(ma->scope, scope_Decl);
      module_analyser_Analyser_analyseCompoundStmt(ma, ((ast_CompoundStmt*)(s)));
      scope_Scope_exit(ma->scope);
      break;
   case ast_StmtKind_Decl:
      module_analyser_Analyser_analyseDeclStmt(ma, s);
      break;
   case ast_StmtKind_Assert:
      module_analyser_Analyser_analyseAssertStmt(ma, s);
      break;
   }
}

static void module_analyser_Analyser_analyseSwitchStmt(module_analyser_Analyser* ma, ast_Stmt* s)
{
   ast_SwitchStmt* sw = ((ast_SwitchStmt*)(s));
   scope_Scope_enter(ma->scope, scope_Decl);
   ast_QualType ct = module_analyser_Analyser_analyseCondition(ma, ast_SwitchStmt_getCond2(sw));
   if (ma->has_error) return;

   bool is_sswitch = ast_SwitchStmt_isSSwitch(sw);
   const uint32_t numCases = ast_SwitchStmt_getNumCases(sw);
   ast_Stmt** cases = ast_SwitchStmt_getCases(sw);
   ast_EnumType* et = ast_QualType_getEnumTypeOrNil(&ct);
   ast_EnumTypeDecl* etd = NULL;
   if (et) etd = ast_EnumType_getDecl(et);
   ast_DefaultStmt* defaultStmt = NULL;
   for (uint32_t i = 0; i < numCases; i++) {
      ast_Stmt* stmt = cases[i];
      if (ast_Stmt_getKind(stmt) == ast_StmtKind_Case) {
         bool ok = module_analyser_Analyser_analyseCaseStmt(ma, stmt, etd, i + 1 == numCases, is_sswitch);
         if (!ok) return;

      } else {
         if (defaultStmt) {
            module_analyser_Analyser_error(ma, ast_DefaultStmt_getLoc(defaultStmt), "multiple default labels");
            return;
         }
         defaultStmt = ((ast_DefaultStmt*)(stmt));
         bool ok = module_analyser_Analyser_analyseDefaultStmt(ma, stmt, is_sswitch);
         if (!ok) return;

         if (i + 1 != numCases) {
            module_analyser_Analyser_error(ma, ast_DefaultStmt_getLoc(defaultStmt), "default case must be last in switch");
            return;
         }
      }
   }
   scope_Scope_exit(ma->scope);
}

static bool module_analyser_Analyser_analyseCaseStmt(module_analyser_Analyser* ma, ast_Stmt* s, ast_EnumTypeDecl* etd, bool lastCase, bool is_sswitch)
{
   ast_CaseStmt* c = ((ast_CaseStmt*)(s));
   ast_Expr* cond = ast_CaseStmt_getCond(c);
   if (etd) {
      if (ast_Expr_getKind(cond) != ast_ExprKind_Identifier) {
         if (ast_Expr_getKind(cond) == ast_ExprKind_Member) {
            module_analyser_Analyser_error(ma, ast_Expr_getLoc(cond), "enum constant may not be prefixed in case statement");
         } else {
            module_analyser_Analyser_error(ma, ast_Expr_getLoc(cond), "condition must be an enum constant when using enum type in switch");
         }
         return false;
      }
      ast_IdentifierExpr* i = ((ast_IdentifierExpr*)(cond));
      ast_EnumConstantDecl* ecd = ast_EnumTypeDecl_findConstant(etd, ast_IdentifierExpr_getNameIdx(i));
      if (!ecd) {
         module_analyser_Analyser_error(ma, ast_Expr_getLoc(cond), "enum %s has no constant %s", ast_Decl_getName(ast_EnumTypeDecl_asDecl(etd)), ast_IdentifierExpr_getName(i));
         return false;
      }
      ast_Decl* d = ((ast_Decl*)(ecd));
      ast_Decl_setUsed(d);
      ast_QualType qt = ast_Decl_getType(d);
      ast_Expr_setType(cond, qt);
      ast_Expr_setCtc(cond);
      ast_Expr_setCtv(cond);
      ast_Expr_setRValue(cond);
      ast_IdentifierExpr_setDecl(i, d);
      ast_IdentifierExpr_setKind(i, ast_IdentifierKind_EnumConstant);
   } else {
      ast_Expr* orig = ast_CaseStmt_getCond(c);
      ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, ast_CaseStmt_getCond2(c), true);
      if (ast_QualType_isInvalid(&qt)) return false;

      ast_Expr_setType(cond, qt);
      if (is_sswitch) {
         if ((ast_Expr_getKind(orig) != ast_ExprKind_Nil && ast_Expr_getKind(orig) != ast_ExprKind_StringLiteral)) {
            module_analyser_Analyser_error(ma, ast_Expr_getLoc(cond), "sswitch case can only have a string literal or nil as condition");
            return false;
         }
      } else {
         if (!ast_Expr_isCtv(cond)) {
            module_analyser_Analyser_error(ma, ast_Expr_getLoc(cond), "case condition is not compile-time constant");
            return false;
         }
      }
   }
   uint32_t flags = (scope_Decl | scope_Break);
   if (!is_sswitch) flags |= scope_Fallthrough;
   scope_Scope_enter(ma->scope, flags);
   uint32_t count = ast_CaseStmt_getNumStmts(c);
   ast_Stmt** stmts = ast_CaseStmt_getStmts(c);
   bool has_decls = false;
   for (uint32_t i = 0; i < count; i++) {
      ast_Stmt* st = stmts[i];
      module_analyser_Analyser_analyseStmt(ma, st, false);
      if (ma->has_error) return false;

      if (ast_Stmt_getKind(st) == ast_StmtKind_Decl) has_decls = true;
      if (ast_Stmt_getKind(st) == ast_StmtKind_Fallthrough) {
         ast_FallthroughStmt* f = ((ast_FallthroughStmt*)(st));
         if (lastCase) {
            module_analyser_Analyser_error(ma, ast_FallthroughStmt_getLoc(f), "'fallthrough' statement in last case");
            return false;
         }
         if (i + 1 != count) {
            module_analyser_Analyser_error(ma, ast_FallthroughStmt_getLoc(f), "'fallthrough' statement must be last statement in case");
            return false;
         }
      }
   }
   if (has_decls) ast_CaseStmt_setHasDecls(c);
   scope_Scope_exit(ma->scope);
   return true;
}

static bool module_analyser_Analyser_analyseDefaultStmt(module_analyser_Analyser* ma, ast_Stmt* s, bool is_sswitch)
{
   ast_DefaultStmt* def = ((ast_DefaultStmt*)(s));
   uint32_t flags = (scope_Decl | scope_Break);
   if (!is_sswitch) flags |= scope_Fallthrough;
   scope_Scope_enter(ma->scope, flags);
   uint32_t count = ast_DefaultStmt_getNumStmts(def);
   ast_Stmt** stmts = ast_DefaultStmt_getStmts(def);
   bool has_decls = false;
   for (uint32_t i = 0; i < count; i++) {
      ast_Stmt* st = stmts[i];
      module_analyser_Analyser_analyseStmt(ma, st, false);
      if (ma->has_error) return false;

      if (ast_Stmt_getKind(st) == ast_StmtKind_Decl) has_decls = true;
      if (ast_Stmt_getKind(st) == ast_StmtKind_Fallthrough) {
         if (i + 1 != count) {
            ast_FallthroughStmt* f = ((ast_FallthroughStmt*)(st));
            module_analyser_Analyser_error(ma, ast_FallthroughStmt_getLoc(f), "'fallthrough' statement in last case");
            return false;
         }
      }
   }
   if (has_decls) ast_DefaultStmt_setHasDecls(def);
   scope_Scope_exit(ma->scope);
   return true;
}

static void module_analyser_Analyser_analyseBreakStmt(module_analyser_Analyser* ma, ast_Stmt* s)
{
   if (!scope_Scope_allowBreak(ma->scope)) {
      ast_BreakStmt* b = ((ast_BreakStmt*)(s));
      module_analyser_Analyser_error(ma, ast_BreakStmt_getLoc(b), "'break' statement not in loop or switch statement");
   }
}

static void module_analyser_Analyser_analyseContinueStmt(module_analyser_Analyser* ma, ast_Stmt* s)
{
   if (!scope_Scope_allowContinue(ma->scope)) {
      ast_ContinueStmt* c = ((ast_ContinueStmt*)(s));
      module_analyser_Analyser_error(ma, ast_ContinueStmt_getLoc(c), "'continue' statement not in loop statement");
   }
}

static void module_analyser_Analyser_analyseFallthroughStmt(module_analyser_Analyser* ma, ast_Stmt* s)
{
   if (!scope_Scope_allowFallthrough(ma->scope)) {
      ast_FallthroughStmt* f = ((ast_FallthroughStmt*)(s));
      module_analyser_Analyser_error(ma, ast_FallthroughStmt_getLoc(f), "'fallthrough' statement not in switch statement");
   }
}

static void module_analyser_Analyser_analyseCompoundStmt(module_analyser_Analyser* ma, ast_CompoundStmt* c)
{
   uint32_t count = ast_CompoundStmt_getCount(c);
   ast_Stmt** stmts = ast_CompoundStmt_getStmts(c);
   for (uint32_t i = 0; i < count; i++) {
      ast_Stmt* s = stmts[i];
      module_analyser_Analyser_analyseStmt(ma, s, true);
      if (ma->has_error) break;

   }
}

static ast_QualType module_analyser_Analyser_analyseCondition(module_analyser_Analyser* ma, ast_Stmt** s_ptr)
{
   ast_Stmt* s = *s_ptr;
   if (ast_Stmt_getKind(s) == ast_StmtKind_Decl) {
      return module_analyser_Analyser_analyseDeclStmt(ma, s);
   }
   assert(ast_Stmt_getKind(s) == ast_StmtKind_Expr);
   ast_Expr* e = ((ast_Expr*)(s));
   ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, &e, true);
   if (ast_QualType_isValid(&qt)) conversion_checker_Checker_check(&ma->checker, ast_g_bool, qt, ((ast_Expr**)(s_ptr)), ast_Expr_getLoc(e));
   return qt;
}

static void module_analyser_Analyser_analyseIfStmt(module_analyser_Analyser* ma, ast_Stmt* s)
{
   ast_IfStmt* i = ((ast_IfStmt*)(s));
   scope_Scope_enter(ma->scope, scope_Decl);
   module_analyser_Analyser_analyseCondition(ma, ast_IfStmt_getCond2(i));
   if (ma->has_error) return;

   scope_Scope_enter(ma->scope, scope_Decl);
   module_analyser_Analyser_analyseStmt(ma, ast_IfStmt_getThen(i), false);
   scope_Scope_exit(ma->scope);
   ast_Stmt* else_ = ast_IfStmt_getElse(i);
   if (else_) {
      scope_Scope_enter(ma->scope, scope_Decl);
      module_analyser_Analyser_analyseStmt(ma, else_, false);
      scope_Scope_exit(ma->scope);
   }
   scope_Scope_exit(ma->scope);
}

static void module_analyser_Analyser_analyseForStmt(module_analyser_Analyser* ma, ast_Stmt* s)
{
   ast_ForStmt* f = ((ast_ForStmt*)(s));
   scope_Scope_enter(ma->scope, (((scope_Break | scope_Continue) | scope_Decl) | scope_Control));
   ast_Stmt** init_ = ast_ForStmt_getInit2(f);
   if (init_) {
      ast_QualType ct = module_analyser_Analyser_analyseCondition(ma, init_);
      if (ast_QualType_isInvalid(&ct)) return;

   }
   ast_Expr** cond = ast_ForStmt_getCond2(f);
   if (cond) {
      ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, cond, true);
      if (ast_QualType_isInvalid(&qt)) return;

      conversion_checker_Checker_check(&ma->checker, ast_g_bool, qt, cond, ast_Expr_getLoc((*cond)));
   }
   ast_Expr** incr = ast_ForStmt_getIncr2(f);
   if (incr) {
      ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, incr, true);
      if (ast_QualType_isInvalid(&qt)) return;

   }
   module_analyser_Analyser_analyseStmt(ma, ast_ForStmt_getBody(f), true);
   scope_Scope_exit(ma->scope);
}

static void module_analyser_Analyser_analyseWhileStmt(module_analyser_Analyser* ma, ast_Stmt* s)
{
   ast_WhileStmt* w = ((ast_WhileStmt*)(s));
   scope_Scope_enter(ma->scope, scope_Decl);
   module_analyser_Analyser_analyseCondition(ma, ast_WhileStmt_getCond2(w));
   if (ma->has_error) return;

   scope_Scope_enter(ma->scope, (((scope_Break | scope_Continue) | scope_Decl) | scope_Control));
   module_analyser_Analyser_analyseStmt(ma, ast_WhileStmt_getBody(w), true);
   scope_Scope_exit(ma->scope);
   scope_Scope_exit(ma->scope);
}

static void module_analyser_Analyser_analyseDoStmt(module_analyser_Analyser* ma, ast_Stmt* s)
{
   ast_DoStmt* d = ((ast_DoStmt*)(s));
   scope_Scope_enter(ma->scope, ((scope_Break | scope_Continue) | scope_Decl));
   module_analyser_Analyser_analyseStmt(ma, ast_DoStmt_getBody(d), true);
   scope_Scope_exit(ma->scope);
   module_analyser_Analyser_analyseStmt(ma, ast_DoStmt_getCond(d), false);
}

static ast_QualType module_analyser_Analyser_analyseDeclStmt(module_analyser_Analyser* ma, ast_Stmt* s)
{
   ast_DeclStmt* ds = ((ast_DeclStmt*)(s));
   ast_VarDecl* vd = ast_DeclStmt_getDecl(ds);
   ast_Decl* d = ((ast_Decl*)(vd));
   ast_TypeRef* ref = ast_VarDecl_getTypeRef(vd);
   ast_QualType res = module_analyser_Analyser_analyseTypeRef(ma, ref);
   if (ast_QualType_isInvalid(&res)) return ast_QualType_Invalid;

   ast_Decl_setType(d, res);
   module_analyser_Analyser_checkName(ma, d, false);
   ast_Expr** initExpr = ast_VarDecl_getInit2(vd);
   if (initExpr) {
      module_analyser_Analyser_analyseInitExpr(ma, initExpr, res);
   } else {
      if (ast_QualType_isConstant(&res)) {
         module_analyser_Analyser_error(ma, ast_Decl_getLoc(d), "constant variable %s must be initialized", ast_Decl_getName(d));
         return ast_QualType_Invalid;
      }
      const ast_ArrayType* at = ast_QualType_getArrayTypeOrNil(&res);
      if ((at && !ast_ArrayType_hasSize(at))) {
         module_analyser_Analyser_error(ma, ast_Decl_getLoc(d), "array-type variable %s needs an explicit size or an initializer", ast_Decl_getName(d));
         return ast_QualType_Invalid;
      }
   }
   ast_Decl_setChecked(d);
   ma->has_error = scope_Scope_add(ma->scope, d);
   return res;
}

static void module_analyser_Analyser_analyseAssertStmt(module_analyser_Analyser* ma, ast_Stmt* s)
{
   ast_AssertStmt* a = ((ast_AssertStmt*)(s));
   ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, ast_AssertStmt_getInner2(a), true);
   if (ast_QualType_isInvalid(&qt)) return;

   ast_Expr* inner = ast_AssertStmt_getInner(a);
   conversion_checker_Checker_check(&ma->checker, ast_g_bool, qt, ast_AssertStmt_getInner2(a), ast_Expr_getLoc(inner));
}

static void module_analyser_Analyser_analyseReturnStmt(module_analyser_Analyser* ma, ast_Stmt* s)
{
   ast_ReturnStmt* r = ((ast_ReturnStmt*)(s));
   ast_Expr** arg = ast_ReturnStmt_getValue2(r);
   if (ast_FunctionDecl_hasReturn(ma->curFunction)) {
      if (!arg) {
         module_analyser_Analyser_error(ma, ast_ReturnStmt_getLoc(r) + 6, "non-void function %s should return a value", ast_Decl_getName(ast_FunctionDecl_asDecl(ma->curFunction)));
         return;
      }
   } else {
      if (arg) {
         ast_Expr* e = ast_ReturnStmt_getValue(r);
         module_analyser_Analyser_error(ma, ast_Expr_getLoc(e), "void function %s should not return a value", ast_Decl_getName(ast_FunctionDecl_asDecl(ma->curFunction)));
         return;
      }
   }
   if (arg) {
      ast_QualType qt = module_analyser_Analyser_analyseExpr(ma, arg, true);
      if (ast_QualType_isInvalid(&qt)) return;

      conversion_checker_Checker_check(&ma->checker, ast_FunctionDecl_getRType(ma->curFunction), qt, arg, ast_Expr_getLoc((*arg)));
   }
}


// --- module compiler ---

typedef struct compiler_Options_ compiler_Options;
typedef struct compiler_LibLoader_ compiler_LibLoader;
typedef struct compiler_Compiler_ compiler_Compiler;

struct compiler_Options_ {
   bool check_only;
   bool print_ast;
   bool generate_c;
   bool generate_qbe;
   bool test_mode;
   bool print_c;
   bool print_qbe;
   bool print_lib_ast;
   bool print_modules;
   bool print_symbols;
   bool print_ast_stats;
   bool print_reports;
};

struct compiler_LibLoader_ {
   const char* libdir;
   source_mgr_SourceMgr* sm;
   c2_parser_Parser* parser;
   string_pool_Pool* auxPool;
   string_pool_Pool* astPool;
   component_Component* c;
};

struct compiler_Compiler_ {
   string_pool_Pool* auxPool;
   source_mgr_SourceMgr* sm;
   diagnostics_Diags* diags;
   const c2recipe_Target* target;
   const compiler_Options* opts;
   module_analyser_Analyser* analyser;
   ast_context_Context* context;
   string_pool_Pool* astPool;
   ast_builder_Builder* builder;
   module_list_ModList* allmodules;
   ast_Module* c2mod;
   c2_parser_Parser* parser;
   component_Component* mainComp;
   component_Component** components;
   uint32_t num_components;
   uint32_t max_components;
};

static void compiler_build(string_pool_Pool* auxPool, source_mgr_SourceMgr* sm, diagnostics_Diags* diags, c2recipe_Target* target, const compiler_Options* opts);
static void compiler_openLib(source_mgr_SourceMgr* sm, string_pool_Pool* auxPool, string_pool_Pool* astPool, component_Component* comp);
static void compiler_LibLoader_init(compiler_LibLoader* l, const char* libdir, source_mgr_SourceMgr* sm, c2_parser_Parser* parser, string_pool_Pool* auxPool, string_pool_Pool* astPool, component_Component* comp);
static void compiler_LibLoader_handleModule(void* arg, ast_Module* m);
static void compiler_loadLib(source_mgr_SourceMgr* sm, c2_parser_Parser* parser, ast_builder_Builder* builder, string_pool_Pool* auxPool, string_pool_Pool* astPool, component_Component* comp);
static void compiler_Compiler_handleModuleImports(void* arg, ast_Module* m);
static void compiler_Compiler_handleImport(void* arg, ast_ImportDecl* id);
static void compiler_Compiler_build(compiler_Compiler* c, string_pool_Pool* auxPool, source_mgr_SourceMgr* sm, diagnostics_Diags* diags, c2recipe_Target* target, const compiler_Options* opts);
static void compiler_Compiler_free(compiler_Compiler* c);
static void compiler_Compiler_resizeComponents(compiler_Compiler* c, uint32_t capacity);
static void compiler_Compiler_addComponent(compiler_Compiler* c, component_Component* comp);
static void compiler_Compiler_analyseModule(void* arg, ast_Module* m);
static void compiler_Compiler_analyseUsedModule(void* arg, ast_Module* m);
static void compiler_Compiler_checkUnused(void* arg, ast_Module* m);

static void compiler_build(string_pool_Pool* auxPool, source_mgr_SourceMgr* sm, diagnostics_Diags* diags, c2recipe_Target* target, const compiler_Options* opts)
{
   compiler_Compiler c;
   compiler_Compiler_build(&c, auxPool, sm, diags, target, opts);
   compiler_Compiler_free(&c);
}

static void compiler_openLib(source_mgr_SourceMgr* sm, string_pool_Pool* auxPool, string_pool_Pool* astPool, component_Component* comp)
{
   const char* libdir = getenv("C2_LIBDIR");
   if (!libdir) printf("Warning: environment variable C2_LIBDIR not set!\n");
   char manifestFilename[512];
   int32_t len = sprintf(manifestFilename, "%s/%s/%s", libdir, component_Component_getName(comp), constants_manifest_name);
   uint32_t name_idx = string_pool_Pool_add(auxPool, manifestFilename, ((size_t)(len)), 0);
   int32_t file_id = source_mgr_SourceMgr_open(sm, name_idx, 0, false);
   if (file_id == -1) return;

   manifest_parse(sm, file_id, astPool, comp);
   source_mgr_SourceMgr_close(sm, file_id);
}

static void compiler_LibLoader_init(compiler_LibLoader* l, const char* libdir, source_mgr_SourceMgr* sm, c2_parser_Parser* parser, string_pool_Pool* auxPool, string_pool_Pool* astPool, component_Component* comp)
{
   l->libdir = libdir;
   l->sm = sm;
   l->parser = parser;
   l->auxPool = auxPool;
   l->astPool = astPool;
   l->c = comp;
}

static void compiler_LibLoader_handleModule(void* arg, ast_Module* m)
{
   compiler_LibLoader* l = arg;
   if (!ast_Module_isUsed(m)) return;

   const char* libdir = getenv("C2_LIBDIR");
   if (!libdir) printf("Warning: environment variable C2_LIBDIR not set!\n");
   char filename[512];
   int32_t len = sprintf(filename, "%s/%s/%s.c2i", l->libdir, component_Component_getName(l->c), ast_Module_getName(m));
   uint32_t name = string_pool_Pool_add(l->auxPool, filename, ((size_t)(len)), 0);
   int32_t file_id = source_mgr_SourceMgr_open(l->sm, name, 0, false);
   if (file_id == -1) return;

   c2_parser_Parser_parse(l->parser, file_id, true);
   source_mgr_SourceMgr_close(l->sm, file_id);
}

static void compiler_loadLib(source_mgr_SourceMgr* sm, c2_parser_Parser* parser, ast_builder_Builder* builder, string_pool_Pool* auxPool, string_pool_Pool* astPool, component_Component* comp)
{
   const char* libdir = getenv("C2_LIBDIR");
   if (!libdir) printf("Warning: environment variable C2_LIBDIR not set!\n");
   ast_builder_Builder_setComponent(builder, comp);
   compiler_LibLoader loader;
   compiler_LibLoader_init(&loader, libdir, sm, parser, auxPool, astPool, comp);
   component_Component_visitModules(comp, compiler_LibLoader_handleModule, &loader);
}

static void compiler_Compiler_handleModuleImports(void* arg, ast_Module* m)
{
   compiler_Compiler* c = arg;
   ast_Module_visitImports(m, compiler_Compiler_handleImport, c);
}

static void compiler_Compiler_handleImport(void* arg, ast_ImportDecl* id)
{
   compiler_Compiler* c = arg;
   if (ast_ImportDecl_getDest(id)) return;

   ast_Decl* d = ((ast_Decl*)(id));
   uint32_t name_idx = ast_Decl_getNameIdx(d);
   ast_Module* m = module_list_ModList_find(c->allmodules, name_idx);
   if (m) {
      ast_ImportDecl_setDest(id, m);
      ast_Module_setUsed(m);
      ast_Decl_setChecked(d);
      ast_Decl_setType(d, ast_QualType_init(((ast_Type*)(ast_Module_getType(m)))));
   } else {
      diagnostics_Diags_error(c->diags, ast_Decl_getLoc(d), "unknown module '%s'", ast_idx2name(name_idx));
      exit(-1);
   }
}

static void compiler_Compiler_build(compiler_Compiler* c, string_pool_Pool* auxPool, source_mgr_SourceMgr* sm, diagnostics_Diags* diags, c2recipe_Target* target, const compiler_Options* opts)
{
   memset(c, 0, sizeof(compiler_Compiler));
   c->auxPool = auxPool;
   c->sm = sm;
   c->diags = diags;
   c->target = target;
   c->opts = opts;
   diagnostics_Diags_setWarningAsError(diags, c2recipe_Target_getWarnings(target)->are_errors);
   c->context = ast_context_create(16 * 1024);
   c->astPool = string_pool_create(128 * 1024, 2048);
   c->builder = ast_builder_create(c->context, diags);
   c->allmodules = module_list_create(false);
   compiler_Compiler_resizeComponents(c, 4);
   diagnostics_Diags_clear(c->diags);
   c->analyser = module_analyser_create(c->diags, c->context, c->astPool, c->builder, c->allmodules, c2recipe_Target_getWarnings(c->target));
   uint32_t wordsize = 8;
   ast_init(c->context, string_pool_Pool_getStart(c->astPool), wordsize);
   attr_init(c->astPool);
   c->c2mod = c2module_loader_load(c->context, c->astPool);
   module_list_ModList_add(c->allmodules, c->c2mod);
   component_Component* libcComponent = component_create(c->context, c->allmodules, "libc", true);
   compiler_Compiler_addComponent(c, libcComponent);
   ast_builder_Builder_setComponent(c->builder, libcComponent);
   compiler_openLib(sm, auxPool, c->astPool, libcComponent);
   const string_list_List* libs = c2recipe_Target_getLibs(target);
   for (uint32_t i = 0; i < string_list_List_length(libs); i++) {
      const char* libname = string_list_List_get(libs, i);
      component_Component* lib = component_create(c->context, c->allmodules, libname, true);
      compiler_Compiler_addComponent(c, lib);
      ast_builder_Builder_setComponent(c->builder, lib);
      compiler_openLib(sm, auxPool, c->astPool, lib);
   }
   c->mainComp = component_create(c->context, c->allmodules, "main", false);
   compiler_Compiler_addComponent(c, c->mainComp);
   ast_builder_Builder_setComponent(c->builder, c->mainComp);
   c2_parser_Info info;
   c2_parser_Info_init(&info, sm, diags, c->astPool, c2recipe_Target_getFeatures(target));
   c->parser = c2_parser_create(&info, c->builder);
   printf("parsing %s\n", string_pool_Pool_idx2str(c->auxPool, c2recipe_Target_getNameIdx(target)));
   uint64_t t1_start = utils_now();
   for (uint32_t j = 0; j < c2recipe_Target_numFiles(target); j++) {
      int32_t file_id = c2recipe_Target_openFile(target, j);
      if (file_id == -1) return;

      c2_parser_Parser_parse(c->parser, file_id, false);
      c2recipe_Target_closeFile(target, file_id);
   }
   uint64_t t1_end = utils_now();
   printf("parsing took %lu usec\n", t1_end - t1_start);
   if (c->opts->print_ast) component_Component_print(c->mainComp, true);
   if (!diagnostics_Diags_isOk(c->diags)) return;

   uint64_t t2_start = utils_now();
   component_Component_visitModules(c->mainComp, compiler_Compiler_handleModuleImports, c);
   module_sorter_sort(c->mainComp, c->diags);
   const warning_flags_Flags* warnings = c2recipe_Target_getWarnings(c->target);
   for (uint32_t i = 0; i < c->num_components - 1; i++) {
      component_Component* comp = c->components[i];
      compiler_loadLib(c->sm, c->parser, c->builder, c->auxPool, c->astPool, comp);
      component_Component_visitModules(comp, compiler_Compiler_handleModuleImports, c);
      module_sorter_sort(comp, c->diags);
      component_Component_visitModules(comp, compiler_Compiler_analyseUsedModule, c);
      if (c->opts->print_lib_ast) component_Component_print(comp, true);
   }
   component_Component_visitModules(c->mainComp, compiler_Compiler_analyseModule, c);
   ast_Module* top = component_Component_getTopModule(c->mainComp);
   ast_Decl* mainFunc = module_analyser_Analyser_checkMain(c->analyser, opts->test_mode, top, string_pool_Pool_addStr(c->astPool, "main", true));
   if ((mainFunc && !warnings->no_unused)) {
      component_Component_visitModules(c->mainComp, compiler_Compiler_checkUnused, c);
   }
   uint64_t t2_end = utils_now();
   printf("analysis took %lu usec\n", t2_end - t2_start);
   if ((diagnostics_Diags_isOk(c->diags) && !c->opts->check_only)) {
      char output_dir[256];
      sprintf(output_dir, "%s/%s", constants_output_dir, string_pool_Pool_idx2str(c->auxPool, c2recipe_Target_getNameIdx(target)));
      int32_t err = file_utils_create_directory(output_dir);
      if (err) {
         fprintf(stderr, "Error creating %s\n", output_dir);
         exit(-1);
      }
      uint64_t gen1 = utils_now();
      refs_generator_generate(c->sm, output_dir, c->components, c->num_components);
      uint64_t gen2 = utils_now();
      printf("refs generation took %lu usec\n", gen2 - gen1);
      if (opts->generate_c) {
         uint64_t gen3 = utils_now();
         const char* target_name = string_pool_Pool_idx2str(c->auxPool, c2recipe_Target_getNameIdx(target));
         c_generator_generate(target_name, output_dir, c->c2mod, c->components, c->num_components, mainFunc, string_pool_Pool_addStr(c->astPool, "stdarg", true), c->opts->print_c);
         uint64_t gen4 = utils_now();
         printf("C generation took %lu usec\n", gen4 - gen3);
         gen3 = utils_now();
         c_generator_build(output_dir);
         gen4 = utils_now();
         printf("C compilation took %lu usec\n", gen4 - gen3);
      }
      if (opts->generate_qbe) {
         uint64_t gen3 = utils_now();
         const char* target_name = string_pool_Pool_idx2str(c->auxPool, c2recipe_Target_getNameIdx(target));
         qbe_generator_generate(target_name, output_dir, c->components, c->num_components, opts->print_qbe);
         uint64_t gen4 = utils_now();
         printf("QBE generation took %lu usec\n", gen4 - gen3);
         gen3 = utils_now();
         qbe_generator_build(output_dir);
         gen4 = utils_now();
         printf("QBE compilation took %lu usec\n", gen4 - gen3);
      }
   }
   if (opts->print_reports) {
      source_mgr_SourceMgr_report(c->sm);
      ast_context_Context_report(c->context);
      string_pool_Pool_report(c->astPool);
   }
   diagnostics_Diags_printStatus(c->diags);
   bool show_functions = true;
   if (c->opts->print_ast) component_Component_print(c->mainComp, show_functions);
   if (c->opts->print_modules) {
      for (uint32_t i = 0; i < c->num_components; i++) {
         component_Component_printModules(c->components[i]);
      }
   }
   if (c->opts->print_symbols) {
      for (uint32_t i = 0; i < c->num_components; i++) {
         component_Component_printSymbols(c->components[i]);
      }
   }
   ast_deinit(c->opts->print_ast_stats);
}

static void compiler_Compiler_free(compiler_Compiler* c)
{
   for (uint32_t i = 0; i < c->num_components; i++) {
      component_Component_free(c->components[i]);
   }
   free(((void*)(c->components)));
   module_analyser_Analyser_free(c->analyser);
   c2_parser_Parser_free(c->parser);
   module_list_ModList_free(c->allmodules);
   ast_Module_free(c->c2mod);
   ast_builder_Builder_free(c->builder);
   string_pool_Pool_free(c->astPool);
   ast_context_Context_free(c->context);
}

static void compiler_Compiler_resizeComponents(compiler_Compiler* c, uint32_t capacity)
{
   c->max_components = capacity;
   component_Component** comps2 = malloc(c->max_components * sizeof(component_Component*));
   if (c->components) {
      memcpy(((void*)(comps2)), ((void*)(c->components)), c->num_components * sizeof(component_Component*));
      free(((void*)(c->components)));
   }
   c->components = comps2;
}

static void compiler_Compiler_addComponent(compiler_Compiler* c, component_Component* comp)
{
   if (c->num_components == c->max_components) compiler_Compiler_resizeComponents(c, c->max_components * 2);
   c->components[c->num_components] = comp;
   c->num_components++;
}

static void compiler_Compiler_analyseModule(void* arg, ast_Module* m)
{
   compiler_Compiler* c = arg;
   module_analyser_Analyser_check(c->analyser, m);
}

static void compiler_Compiler_analyseUsedModule(void* arg, ast_Module* m)
{
   compiler_Compiler* c = arg;
   if (ast_Module_isUsed(m)) {
      module_analyser_Analyser_check(c->analyser, m);
   }
}

static void compiler_Compiler_checkUnused(void* arg, ast_Module* m)
{
   compiler_Compiler* c = arg;
   unused_checker_check(c->diags, c2recipe_Target_getWarnings(c->target), m);
}


// --- module c2c_main ---

typedef struct c2c_main_Options_ c2c_main_Options;

struct c2c_main_Options_ {
   bool print_ast;
   bool check_only;
   bool generate_c;
   bool generate_qbe;
   bool test_mode;
   bool print_c;
   bool print_qbe;
   bool print_lib_ast;
   bool print_modules;
   bool print_symbols;
   bool print_ast_stats;
   bool print_reports;
   const char* target;
   const char* single_file;
   const char* other_dir;
};

static void c2c_main_usage(const char* me);
static void c2c_main_parse_long_opt(int32_t i, int32_t argc, char** argv, c2c_main_Options* opts);
static void c2c_main_parse_opts(int32_t argc, char** argv, c2c_main_Options* opts);
int32_t main(int32_t argc, char** argv);

static void c2c_main_usage(const char* me)
{
   printf("Usage: %s <options> <filename>\n", me);
   printf("Options\n");
   printf("\t-a        print ASTs\n");
   printf("\t-A        print Library ASTs\n");
   printf("\t-c        generate C-code\n");
   printf("\t-C        generate + print C-code\n");
   printf("\t-d [dir]  change to [dir] first\n");
   printf("\t-f        only parse single file\n");
   printf("\t-m        print modules\n");
   printf("\t-q        generate QBE\n");
   printf("\t-Q        generate + print QBE\n");
   printf("\t-r        print reports\n");
   printf("\t-s        print symbols\n");
   printf("\t-t        print AST statistics\n");
   printf("\t--test    test mode (dont check for main() function)\n");
   exit(-1);
}

static void c2c_main_parse_long_opt(int32_t i, int32_t argc, char** argv, c2c_main_Options* opts)
{
   const char* arg = argv[i];
   do {
      const char* _tmp = arg + 2;
      if (strcmp(_tmp, "test") == 0) {
         opts->test_mode = true;
      } else {
         c2c_main_usage(argv[0]);
      }
   } while (0);
}

static void c2c_main_parse_opts(int32_t argc, char** argv, c2c_main_Options* opts)
{
   for (int32_t i = 1; i < argc; i++) {
      const char* arg = argv[i];
      if (arg[0] == '-') {
         switch (arg[1]) {
         case '-':
            c2c_main_parse_long_opt(i, argc, argv, opts);
            break;
         case 'A':
            opts->print_lib_ast = true;
            break;
         case 'C':
            opts->generate_c = true;
            opts->print_c = true;
            break;
         case 'Q':
            opts->generate_qbe = true;
            opts->print_qbe = true;
            break;
         case 'a':
            opts->print_ast = true;
            break;
         case 'c':
            opts->generate_c = true;
            break;
         case 'd':
            if (i == argc - 1) c2c_main_usage(argv[0]);
            i++;
            opts->other_dir = argv[i];
            break;
         case 'f':
            if (i == argc - 1) c2c_main_usage(argv[0]);
            i++;
            opts->single_file = argv[i];
            break;
         case 'm':
            opts->print_modules = true;
            break;
         case 'q':
            opts->generate_qbe = true;
            break;
         case 'r':
            opts->print_reports = true;
            break;
         case 's':
            opts->print_symbols = true;
            break;
         case 't':
            opts->print_ast_stats = true;
            break;
         default:
            printf("unknown option\n");
            c2c_main_usage(argv[0]);
            break;
         }
      } else {
         if (opts->target) c2c_main_usage(argv[0]);
         opts->target = arg;
      }
   }
   if ((opts->target && opts->single_file)) c2c_main_usage(argv[0]);
}

int32_t main(int32_t argc, char** argv)
{
   c2c_main_Options opts = { };
   c2c_main_parse_opts(argc, argv, &opts);
   if (opts.other_dir) {
      if (chdir(opts.other_dir)) {
         fprintf(stderr, "cannot chdir to %s: %s\n", opts.other_dir, strerror(*__errno_location()));
         exit(-1);
      }
   }
   string_pool_Pool* auxPool = string_pool_create(32 * 1024, 0);
   source_mgr_SourceMgr* sm = source_mgr_create(auxPool, constants_Max_open_files);
   diagnostics_Diags* diags = diagnostics_create(sm, utils_useColor());
   c2recipe_Recipe* recipe = c2recipe_create(sm, auxPool);
   int32_t recipe_id = -1;
   bool hasError = false;
   if (opts.single_file) {
      c2recipe_Recipe_addDummyTarget(recipe, opts.single_file);
   } else {
      if (!utils_findProjectDir()) {
         fprintf(stderr, "c2c: error: cannot find C2 root dir\n");
         fprintf(stderr, "c2c requires a %s file in the project root\n", constants_recipe_name);
         fprintf(stderr, "Use argument -h for a list of available opts and usage of c2c\n");
         return -1;
      }
      uint32_t recipe_idx = string_pool_Pool_addStr(auxPool, constants_recipe_name, false);
      int32_t yaml_id = source_mgr_SourceMgr_open(sm, recipe_idx, 0, false);
      if (yaml_id == -1) return -1;

      if (!c2recipe_Recipe_parse(recipe, yaml_id)) return -1;

      recipe_id = yaml_id;
   }
   compiler_Options options = { };
   options.check_only = opts.check_only;
   options.print_ast = opts.print_ast;
   options.generate_c = opts.generate_c;
   options.generate_qbe = opts.generate_qbe;
   options.test_mode = opts.test_mode;
   options.print_c = opts.print_c;
   options.print_lib_ast = opts.print_lib_ast;
   options.print_modules = opts.print_modules;
   options.print_symbols = opts.print_symbols;
   options.print_ast_stats = opts.print_ast_stats;
   options.print_qbe = opts.print_qbe;
   options.print_reports = opts.print_reports;
   uint32_t num_build = 0;
   for (uint32_t i = 0; i < c2recipe_Recipe_numTargets(recipe); i++) {
      c2recipe_Target* target = c2recipe_Recipe_getTarget(recipe, i);
      const char* target_name = string_pool_Pool_idx2str(auxPool, c2recipe_Target_getNameIdx(target));
      if ((opts.target && strcmp(opts.target, target_name) != 0)) continue;

      printf("building %s\n", target_name);
      num_build++;
      compiler_build(auxPool, sm, diags, target, &options);
      hasError |= (diagnostics_Diags_getNumErrors(diags) != 0);
      if (recipe_id != -1) source_mgr_SourceMgr_clear(sm, ((uint32_t)(recipe_id)));
   }
   if ((opts.target && num_build == 0)) {
      fprintf(stderr, "No such target\n");
   }
   c2recipe_Recipe_free(recipe);
   string_pool_Pool_free(auxPool);
   diagnostics_Diags_free(diags);
   source_mgr_SourceMgr_free(sm);
   return hasError ? -1 : 0;
}

