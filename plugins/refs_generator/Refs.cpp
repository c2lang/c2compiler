/* Copyright Bas van den Berg (2022) */

#include "Refs.h"
#include "QuickSort.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

typedef uint32_t u32;
typedef uint16_t u16;

#define ROUND4(x) ((x + 3) & ~0x3)
#define NOT_FOUND ((u32)-1)

// ------ MMap ------

typedef struct {
    void* map;
    int size;
} MapFile;

static MapFile open_file(const char* filename) {
    MapFile result = { NULL, 0 };
    int fd = open(filename, O_RDONLY | O_CLOEXEC);
    if (fd == -1) return result;

    struct stat statbuf;
    int err = stat(filename, &statbuf);
    if (err) return result;

    int size = (int)statbuf.st_size;
    int prot = PROT_READ | PROT_WRITE;
    int flags = MAP_PRIVATE;
    void* map = mmap(NULL, (size_t)size, prot, flags, fd, 0);
    if (map == (void*)-1) {
        err = errno;
        close(fd);
        errno = err;
        return result;
    }
    close(fd);
    result.map = map;
    result.size = (size_t)size;
    return result;
}

static void close_file(MapFile f) {
    munmap(f.map, (size_t)f.size);
}


// ------ Files ------

#define FILES_TAGS(x) ((u32*)(((char*) x->name_indexes) +  x->idx_cap * sizeof(u32)))
#define FILES_NAMES(x) (((char*) x->name_indexes) +  x->idx_cap * sizeof(u32) * 2)

typedef struct {
    u32 total_size;     // of whole RefFiles
    u32 idx_count;      // current number of entries
    u32 idx_cap;        // max number of entries
    u32 names_len;      // total len of 0-terminated names
    u32 names_cap;
    u32 name_indexes[0];      // offsets in names
    // u32 tag_indexes[0]     // offsets in Tags
    // names (each 0-terminated)
} RefFiles;

static RefFiles* files_create(u32 max_idx, u32 max_data) {
    u32 size = (u32)(sizeof(RefFiles) + (max_idx * sizeof(u32) * 2) + max_data);
    size = ROUND4(size);

    RefFiles* f = (RefFiles*)malloc(size);
    f->total_size = size;
    f->idx_count = 0;
    f->idx_cap = max_idx;
    f->names_len = 0;
    f->names_cap = max_data;
    return f;
}

static RefFiles* files_load(void* input, u32 size) {
    if (size < sizeof(RefFiles)) return NULL;

    RefFiles* src = (RefFiles*)input;
    if (src->total_size > size) return NULL;

    void* files = malloc(src->total_size);
    memcpy(files, input, src->total_size);

    return (RefFiles*)files;
}

static void files_free(RefFiles* f) {
    free(f);
}

static u32 files_getSize(const RefFiles* f) { return f->total_size; }

static u32 files_getCount(const RefFiles* f) { return f->idx_count; }

static const u32* files_getTagStarts(const RefFiles* f) {
    return FILES_TAGS(f);
}

static u32 files_getTagStart(const RefFiles* f, u32 idx) {
    const u32* tags = FILES_TAGS(f);
    return tags[idx];
}

static u32 files_getTagEnd(const RefFiles* f, u32 idx) {
    const u32* tags = FILES_TAGS(f);
    idx++;  // get start of next one
    if (idx >= f->idx_count) return NOT_FOUND;
    return tags[idx] - 1;
}

static RefFiles* files_resize(RefFiles* f, u32 max_idx, u32 max_data) {
    RefFiles* f2 = files_create(max_idx, max_data);
    if (f->idx_count) {
        f2->idx_count = f->idx_count;
        memcpy(f2->name_indexes, f->name_indexes, f->idx_count * sizeof(u32));
        memcpy(FILES_TAGS(f2), FILES_TAGS(f), f->idx_count * sizeof(u32));
    }
    if (f->names_len) {
        f2->names_len = f->names_len;
        memcpy(FILES_NAMES(f2), FILES_NAMES(f), f->names_len);
    }
    free(f);
    return f2;
}

static const char* files_idx2name(const RefFiles* f, u32 file_id) {
    if (file_id >= f->idx_count) return NULL;
    const char* names = FILES_NAMES(f);
    return names + f->name_indexes[file_id];
}

// NOTE: return (-1) if not found
static u32 files_name2idx(const RefFiles* f, const char* filename) {
    // TODO use fast lookup index? (put after normal name_indexes)
    const char* names = FILES_NAMES(f);
    for (u32 i=0; i<f->idx_count; i++) {
        const char* name = names + f->name_indexes[i];
        if (strcmp(name, filename) == 0) return i;
    }
    return (u32)-1;
}

// Note: if tag_start == NOT_FOUND, dont set
static u32 files_add(RefFiles** f_ptr, const char* filename, u32 tag_start) {
    // Could already have been added as dest_filename of Tag
    RefFiles* f = *f_ptr;

    u32 idx = files_name2idx(f, filename);
    u32* tags = FILES_TAGS(f);
    if (idx == NOT_FOUND) {

        u32 len = strlen(filename) + 1;
        if (f->idx_count == f->idx_cap || f->names_len + len == f->names_cap) {
            // For now just resize both name_indexes and names
            f = files_resize(f, f->idx_count * 2, f->names_len * 2);
            *f_ptr = f;
            tags = FILES_TAGS(f);
        }

        idx = f->idx_count;
        f->name_indexes[idx] = f->names_len;
        f->idx_count++;

        tags[idx] = NOT_FOUND;

        char* names = FILES_NAMES(f);
        memcpy(names + f->names_len, filename, len);
        f->names_len += len;

    }

    if (tag_start != NOT_FOUND) tags[idx] = tag_start;

    return idx;
}


static RefFiles* files_trim(RefFiles* f) {
    if (f->idx_count == f->idx_cap && f->names_len == f->names_cap) return f;

    return files_resize(f, f->idx_count, f->names_len);
}

static void files_dump(const RefFiles* f, bool verbose) {
    printf("Files: %u bytes  %u/%u entries  %u/%u data\n",
        f->total_size,
        f->idx_count, f->idx_cap,
        f->names_len, f->names_cap);
    if (verbose) {
        const u32* tags = FILES_TAGS(f);
        const char* names = FILES_NAMES(f);
        for (u32 i=0; i<f->idx_count; i++) {
            u32 idx = f->name_indexes[i];
            printf("  [%5u] %5u %s\n", idx, tags[i], names+idx);
        }
    }
}


// ------ TAGS ------

typedef struct {
    u32 line;
    u16 file_id;
    u16 col;
} __attribute__((packed)) Dest;

typedef struct {
    RefSrc src;
    Dest dest;
} __attribute__((packed)) Tag;

typedef struct {
    u32 total_size;
    u32 count;
    u32 capacity;
    Tag tags[0];    // caps times
} Tags;

static Tags* tags_create(u32 capacity) {
    u32 size = sizeof(Tags) + capacity * sizeof(Tag);
    size = ROUND4(size);
    Tags* t = (Tags*)malloc(size);
    t->total_size = size;
    t->count = 0;
    t->capacity = capacity;
    return t;
}

static Tags* tags_load(void* input, u32 size) {
    if (size < sizeof(Tags)) return NULL;

    Tags* src = (Tags*)input;
    if (src->total_size > size) return NULL;

    void* files = malloc(src->total_size);
    memcpy(files, input, src->total_size);

    return (Tags*)files;
}

static void tags_free(Tags* t) {
    free(t);
}

static u32 tags_getSize(const Tags* t) { return t->total_size; }

static u32 tags_getCount(const Tags* t) { return t->count; }

static Tags* tags_resize(Tags* t, u32 capacity) {
    Tags* t2 = tags_create(capacity);
    if (t->count) {
        t2->count = t->count;
        memcpy(t2->tags, t->tags, t->count * sizeof(Tag));
    }
    free(t);
    return t2;
}

static bool tag_compare(void* arg, const void* left, const void* right) {
    const Tag* l = (Tag*)left;
    const Tag* r = (Tag*)right;

    const u32 line1 = l->src.line;
    const u32 line2 = r->src.line;
    if (line1 < line2) return true;
    if (line1 > line2) return false;

    return l->src.col < r->src.col;
}

static void tags_sort(Tags* t, const u32* indexes, u32 count) {
    for (u32 i=0; i<count; i++) {
        u32 start = indexes[i];
        u32 end = t->count;
        if (i+1 != count) end = indexes[i+1];
        if (end > start) {
            quicksort(t->tags + start, end - start, sizeof(Tag), tag_compare, NULL);
        }
    }
}

static Tags* tags_trim(Tags* t) {
    if (t->count == t->capacity) return t;

    return tags_resize(t, t->count);
}

static void tags_add(Tags** t_ptr, const RefSrc* src, u32 dst_file, u32 dst_line, u16 dst_col) {
    Tags* t = *t_ptr;
    if (t->count == t->capacity) {
        t = tags_resize(t, t->count * 2);
        *t_ptr = t;
    }

    // TODO convert src.len -> src.max_col (faster check)
    Tag* cur = &t->tags[t->count];
    cur->src = *src;

    cur->dest.line = dst_line;
    cur->dest.file_id = dst_file;
    cur->dest.col = dst_col;
    t->count++;
}

const Dest* tags_find(const Tags* t, const RefDest* origin, u32 start, u32 last) {
    if (last > t->count) last = t->count -1;
    // TODO we can do binary search for line here! (between start/last tag
    for (u32 i=start; i<=last; i++) {
        const Tag* tag = &t->tags[i];
        const RefSrc* src = &tag->src;
        if (src->line == origin->line) {
            if (src->col <= origin->col && (src->col + src->len) >= origin->col) {
                return &tag->dest;
            }
        }
    }
    return NULL;
}

static void tags_dump(const Tags* t, bool verbose) {
    printf("Tags: %u bytes  %u/%u tags\n", t->total_size, t->count, t->capacity);
    if (verbose) {
        for (u32 i=0; i<t->count; i++) {
            const Tag* tt = &t->tags[i];
            printf("  %u:%u:%u -> %u:%u:%u\n",
                tt->src.line, tt->src.col, tt->src.len,
                tt->dest.file_id, tt->dest.line, tt->dest.col);
        }
    }
}


// ------ Refs ------

struct Refs_ {
    RefFiles* files;
    Tags* tags;
    u32 cur_file;
};

Refs* refs_create(void) {
    Refs* w = (Refs*)calloc(1, sizeof(Refs));
    w->files = files_create(32, 2048);
    w->tags = tags_create(128);
    return w;
}

static Refs* refs_load_internal(void* data, u32 size) {
    RefFiles* files = files_load(data, size);
    if (!files) return NULL;

    u32 files_size = files_getSize(files);
    data = (char*) data + files_size;
    size -= files_size;

    Tags* tags = tags_load(data, size);
    if (!tags) {
        files_free(files);
        return NULL;
    }

    Refs* r = (Refs*)calloc(1, sizeof(Refs));
    r->files = files;
    r->tags = tags;
    return r;
}

Refs* refs_load(const char* filename) {
    MapFile f = open_file(filename);
    if (!f.map) return NULL;

    Refs* r = refs_load_internal(f.map, f.size);
    close_file(f);
    return r;
}

static bool write_checked(int fd, void* data, u32 size) {
    ssize_t written = write(fd, data, size);
    if (size != written) {
        int err = errno;
        close(fd);
        errno = err;
        return false;
    }
    return true;
}

bool refs_write(const Refs* r, const char* filename) {
    int fd = open(filename, O_CREAT | O_WRONLY | O_CLOEXEC | O_TRUNC, 0660);
    if (fd == -1) return false;

    if (!write_checked(fd, r->files, files_getSize(r->files))) return false;
    if (!write_checked(fd, r->tags, tags_getSize(r->tags))) return false;

    close(fd);
    return true;
}

void refs_add_file(Refs* w, const char* filename) {
    w->cur_file = files_add(&w->files, filename, tags_getCount(w->tags));
}

void refs_add_tag(Refs* w, const RefSrc* src, const RefDest* dest) {
    // TODO cache last dest_filename, if same (ptr) return last entry
    // Note: filename might already be added, but files_add checks for dups
    u32 dst_idx = files_add(&w->files, dest->filename, NOT_FOUND);
    tags_add(&w->tags, src, dst_idx, dest->line, dest->col);
}

void refs_trim(Refs* w) {
    w->files = files_trim(w->files);
    w->tags = tags_trim(w->tags);

    tags_sort(w->tags, files_getTagStarts(w->files), files_getCount(w->files));
}

void refs_free(Refs* w) {
    files_free(w->files);
    tags_free(w->tags);
    free(w);
}

RefDest refs_findRef(const Refs* w, const RefDest* origin) {
    RefDest result = { NULL, 0, 0 };
    u32 file_id = files_name2idx(w->files, origin->filename);
    if (file_id == NOT_FOUND) return result;

    u32 start_tag = files_getTagStart(w->files, file_id);
    if (start_tag == NOT_FOUND) return result;

    u32 end_tag = files_getTagEnd(w->files, file_id);
    const Dest* dest = tags_find(w->tags, origin, start_tag, end_tag);
    if (dest) {
        result.filename = files_idx2name(w->files, dest->file_id);
        result.line = dest->line;
        result.col = dest->col;
    }

    return result;
}

void refs_dump(const Refs* w, bool verbose) {
    files_dump(w->files, verbose);
    tags_dump(w->tags, verbose);
}

