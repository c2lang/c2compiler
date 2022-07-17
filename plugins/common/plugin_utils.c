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

#include "common/plugin_utils.h"
#include <ctype.h>
#include <string.h>

void word_tokenizer_run(WordTokenizer* tok, const char* input) {
    tok->input = input;
    tok->word[0] = 0;
}

bool word_tokenizer_next(WordTokenizer* tok) {
    if (tok->input == NULL) return false;
    tok->word[0] = 0;

    // skip spaces
    while (isspace(*tok->input)) tok->input++;

    // find end
    const char* end = tok->input;
    while (!isspace(*end) && *end) end++;

    unsigned len = (unsigned)(end - tok->input);
    if (len && len < MAX_WORD) {
        memcpy(tok->word, tok->input, len);
        tok->word[len] = 0;
        tok->input = end;
        return true;
    }

    tok->input = NULL;
    return false;

}

const char* word_tokenizer_get(const WordTokenizer* tok) {
    return tok->word;
}

