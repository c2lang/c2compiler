/* Copyright Bas van den Berg (2022) */

#ifndef REFS_H
#define REFS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef   __cplusplus
extern "C"
{
#endif

typedef struct {
    uint32_t line;
    uint16_t col;
    uint16_t len;
} __attribute__((packed)) RefSrc;

typedef struct {
    const char* filename;
    uint32_t line;
    uint16_t col;
} RefDest;

typedef struct Refs_ Refs;

Refs* refs_create(void);

Refs* refs_load(const char* filename);

bool refs_write(const Refs* r, const char* filename);

void refs_free(Refs* r);

void refs_add_file(Refs* r, const char* filename);

// note: source file must already be set by add_file()
void refs_add_tag(Refs* r, const RefSrc* src, const RefDest* dest);

void refs_trim(Refs* r);

// Returns NULL as filename if not found
RefDest refs_findRef(const Refs* r, const RefDest* origin);

void refs_dump(const Refs* r, bool verbose);

#ifdef   __cplusplus
}
#endif

#endif

