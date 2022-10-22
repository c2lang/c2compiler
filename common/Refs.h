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

