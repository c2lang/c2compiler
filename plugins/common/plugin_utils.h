/* Copyright 2022 Bas van den Berg
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

#ifndef PLUGIN_UTILS_H
#define PLUGIN_UTILS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_WORD 64

typedef struct {
    const char* input;
    char word[MAX_WORD];
} WordTokenizer;

void word_tokenizer_run(WordTokenizer* tok, const char* input);

bool word_tokenizer_next(WordTokenizer* tok);

const char* word_tokenizer_get(const WordTokenizer* tok);

#ifdef __cplusplus
}
#endif

#endif
