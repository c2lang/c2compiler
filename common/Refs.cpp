/* Copyright 2013-2022 Bas van den Berg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common/Refs.h"
#include "common/QuickSort.h"

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

/*
    Design:
    - the files consists of sections (Files, Tags, Locations, ..)
    - each section starts with a 32-bit section-size
    - to load/store, read/write the whole section
    - Files contain the filenames and start-tags
    - Tags contain the original source locations + length, points to Location idx
    - Locations contain the symbol locations
*/

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

static u32 section_size(const void* section) { return *((u32*)section); }

static void* section_load(MapFile* f, u32 minSize) {
    u32 size = (u32)f->size;
    if (size < minSize) return NULL;

    u32 load_size = section_size(f->map);
    if (load_size > size) return NULL;

    void* section = malloc(load_size);
    memcpy(section, f->map, load_size);

    f->map = ((uint8_t*)f->map) + load_size;
    f->size -= load_size;

    return section;
}

static bool section_write(int fd, void* section) {
    const u32 size = section_size(section);
    ssize_t written = write(fd, section, size);
    if (size != written) {
        int err = errno;
        close(fd);
        errno = err;
        return false;
    }
    return true;
}

static void section_free(void* t) {
    free(t);
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

typedef void (*FileTagsVisitor)(void* arg, u32 start, u32 count);

static void files_visitTags(const RefFiles* f, FileTagsVisitor visitor, void* arg) {
    const u32* indexes = FILES_TAGS(f);
    u32 count = f->idx_count;
    for (u32 i=0; i<count; i++) {
        u32 start = indexes[i];
        u32 end = NOT_FOUND;
        if (i+1 != count) end = indexes[i+1];
        // NOTE: for the last one, we dont know the end, pass NOT_FOUND
        if (end > start) {
            visitor(arg, start, end);
        }
    }
}

static const char* files_idx2name(const RefFiles* f, u32 file_id) {
    if (file_id >= f->idx_count) return NULL;
    const char* names = FILES_NAMES(f);
    return names + f->name_indexes[file_id];
}

// NOTE: return (-1) if not found
static u32 files_name2idx(const RefFiles* f, const char* filename) {
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
    RefSrc src;
    u32 loc_idx;
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

static void tags_sort(void* arg, u32 start, u32 end) {
    Tags* t = (Tags*)arg;
    if (end > t->count) end = t->count;
    if (end - start <= 1) return;
    quicksort(t->tags + start, end - start, sizeof(Tag), tag_compare, NULL);
}

static Tags* tags_trim(Tags* t) {
    if (t->count == t->capacity) return t;

    return tags_resize(t, t->count);
}

static void tags_add(Tags** t_ptr, const RefSrc* src, u32 loc_idx) {
    Tags* t = *t_ptr;
    if (t->count == t->capacity) {
        t = tags_resize(t, t->count * 2);
        *t_ptr = t;
    }

    Tag* cur = &t->tags[t->count];
    cur->src = *src;
    cur->loc_idx = loc_idx;
    t->count++;
}

static bool tags_find_line(const Tag* tags, u32* left, u32* right, const u32 line) {
    while (*left != *right) {
        const u32 middle = (*left + *right) / 2;
        const u32 mline = tags[middle].src.line;
        if (line < mline) {
            *right = middle;
        } else if (line > mline) {
            *left = middle;
        } else {
            u32 l = middle;
            u32 r = middle;
            while (tags[l].src.line == line && l >= *left) l--;
            while (tags[r].src.line == line && r < *right) r++;
            *left = l;
            *right = r;
            return true;
        }
    }
    return false;
}

// Note: including last
static u32 tags_find(const Tags* t, const RefDest* origin, u32 start, u32 last) {
    if (last > t->count) last = t->count -1;

    last++;  // must be outside for binary search to work
    if (!tags_find_line(t->tags, &start, &last, origin->line)) return NOT_FOUND;

    // first entry for column
    for (u32 i=start; i<=last; i++) {
        const Tag* tag = &t->tags[i];
        const RefSrc* src = &tag->src;
        if (src->col <= origin->col && (src->col + src->len) >= origin->col) {
            return tag->loc_idx;
        }
    }
    return NOT_FOUND;
}

static void tags_dump(const Tags* t, bool verbose) {
    printf("Tags: %u bytes  %u/%u tags\n", t->total_size, t->count, t->capacity);
    if (verbose) {
        for (u32 i=0; i<t->count; i++) {
            const Tag* tt = &t->tags[i];
            printf("  %u:%u:%u -> %u\n", tt->src.line, tt->src.col, tt->src.len, tt->loc_idx);
        }
    }
}


// ------ LOCATIONS ------

typedef struct {
    u32 line;
    u16 file_id;
    u16 col;
} __attribute__((packed)) Loc;

typedef struct {
    u32 total_size;
    u32 count;
    u32 capacity;
    Loc locs[0];    // caps times
} Locs;

static Locs* locs_create(u32 capacity) {
    u32 size = sizeof(Locs) + capacity * sizeof(Loc);
    size = ROUND4(size);
    Locs* t = (Locs*)malloc(size);
    t->total_size = size;
    t->count = 0;
    t->capacity = capacity;
    return t;
}

//static u32 locs_getCount(const Locs* t) { return t->count; }

static Locs* locs_resize(Locs* t, u32 capacity) {
    Locs* t2 = locs_create(capacity);
    if (t->count) {
        t2->count = t->count;
        memcpy(t2->locs, t->locs, t->count * sizeof(Loc));
    }
    free(t);
    return t2;
}

static Locs* locs_trim(Locs* l) {
    if (l->count == l->capacity) return l;

    return locs_resize(l, l->count);
}

static u32 locs_add(Locs** t_ptr, u32 file_id, u32 line, u16 col) {
    Locs* t = *t_ptr;

    // Filter duplicate entries
    u32 idx = NOT_FOUND;
    for (u32 i=0; i<t->count; i++) {
        const Loc* loc = &t->locs[i];
        if (loc->line == line && loc->file_id == file_id && loc->col == col) {
            idx = i;
            break;
        }
    }
    if (idx != NOT_FOUND) return idx;

    if (t->count == t->capacity) {
        t = locs_resize(t, t->count * 2);
        *t_ptr = t;
    }

    idx = t->count;
    Loc* loc = &t->locs[idx];
    loc->line = line;
    loc->file_id = file_id;
    loc->col = col;
    t->count++;
    return idx;
}

const Loc* loc_idx2loc(const Locs* t, u32 idx) { return &t->locs[idx]; }

static void locs_dump(const Locs* t, bool verbose) {
    printf("Locs: %u bytes  %u/%u locs\n", t->total_size, t->count, t->capacity);
    if (verbose) {
        for (u32 i=0; i<t->count; i++) {
            const Loc* d = &t->locs[i];
            printf("  [%5u] %u:%u:%u\n", i, d->file_id, d->line, d->col);
        }
    }
}


// ------ Refs ------

struct Refs_ {
    RefFiles* files;
    Tags* tags;
    Locs* locs;
};

Refs* refs_create(void) {
    Refs* r = (Refs*)calloc(1, sizeof(Refs));
    r->files = files_create(32, 2048);
    r->tags = tags_create(128);
    r->locs = locs_create(64);
    return r;
}

static Refs* refs_load_internal(MapFile f) {
    RefFiles* files = (RefFiles*)section_load(&f, sizeof(RefFiles));
    if (!files) return NULL;

    Tags* tags = (Tags*)section_load(&f, sizeof(Tags));
    if (!tags) {
        section_free(files);
        return NULL;
    }

    Locs* locs = (Locs*)section_load(&f, sizeof(Locs));
    if (!locs) {
        section_free(files);
        section_free(tags);
        return NULL;
    }

    Refs* r = (Refs*)calloc(1, sizeof(Refs));
    r->files = files;
    r->tags = tags;
    r->locs = locs;
    return r;
}

Refs* refs_load(const char* filename) {
    MapFile f = open_file(filename);
    if (!f.map) return NULL;

    Refs* r = refs_load_internal(f);    // NOTE: must be copy, since it will be modified
    close_file(f);
    return r;
}

bool refs_write(const Refs* r, const char* filename) {
    int fd = open(filename, O_CREAT | O_WRONLY | O_CLOEXEC | O_TRUNC, 0660);
    if (fd == -1) return false;

    if (!section_write(fd, r->files)) return false;
    if (!section_write(fd, r->tags)) return false;
    if (!section_write(fd, r->locs)) return false;

    close(fd);
    return true;
}

void refs_add_file(Refs* r, const char* filename) {
    files_add(&r->files, filename, tags_getCount(r->tags));
}

void refs_add_tag(Refs* r, const RefSrc* src, const RefDest* dest) {
    // Caching dest.fileName ptr + idx does not work:
    // C2C native does have same ptr for each string, but C2C C++ does not.
    u32 file_idx = files_add(&r->files, dest->filename, NOT_FOUND);
    u32 loc_idx = locs_add(&r->locs, file_idx, dest->line, dest->col);
    tags_add(&r->tags, src, loc_idx);
}

void refs_trim(Refs* r) {
    r->files = files_trim(r->files);
    r->tags = tags_trim(r->tags);
    r->locs = locs_trim(r->locs);
    files_visitTags(r->files, tags_sort, r->tags);
}

void refs_free(Refs* r) {
    section_free(r->files);
    section_free(r->tags);
    free(r);
}

RefDest refs_findRef(const Refs* r, const RefDest* origin) {
    RefDest result = { NULL, 0, 0 };
    u32 file_id = files_name2idx(r->files, origin->filename);
    if (file_id == NOT_FOUND) return result;

    u32 start_tag = files_getTagStart(r->files, file_id);
    if (start_tag == NOT_FOUND) return result;

    u32 end_tag = files_getTagEnd(r->files, file_id);
    u32 loc_idx = tags_find(r->tags, origin, start_tag, end_tag);
    if (loc_idx == NOT_FOUND) return result;

    const Loc* loc = loc_idx2loc(r->locs, loc_idx);
    result.filename = files_idx2name(r->files, loc->file_id);
    result.line = loc->line;
    result.col = loc->col;

    return result;
}

void refs_dump(const Refs* r, bool verbose) {
    files_dump(r->files, verbose);
    tags_dump(r->tags, verbose);
    locs_dump(r->locs, verbose);
}

/*
    - remove NOT_FOUND, just use 0 (skip first Tag/Loc entry to allow this?)
    - add a new section: names, used to search by Name, get either Loc or All refs
        Name -> strings (like FileIndex) + u32 array (like indexes)
            how to handle duplicates?
    - To see all references of the name, simply search all Tags for usage (using Files_visitor)
*/

