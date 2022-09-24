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

#ifndef YAML_PARSER_H
#define YAML_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct YamlParser_ YamlParser;

YamlParser* yaml_create(const char* input, uint32_t size);

void yaml_destroy(YamlParser* p);

bool yaml_parse(YamlParser* p);

const char* yaml_get_error(const YamlParser* p);

void yaml_dump(const YamlParser* p, bool verbose);

// ------ ITERATORS ------

typedef struct YamlNode_ YamlNode;

typedef struct {
    const void* data;
    const YamlNode* node;
} YamlIter;

const YamlNode* yaml_get_root(const YamlParser* p);
const YamlNode* yaml_get_next_root(const YamlParser* p, const YamlNode* root);

// returns NULL if node is not a scalar
const char* yaml_get_scalar_value(const YamlParser* p, const char* path);
const YamlNode* yaml_find_node(const YamlParser* p, const char* path);
//YamlIter yaml_get_iter(const YamlParser* p, const char* path);

bool yaml_node_is_scalar(const YamlNode* node);
bool yaml_node_is_sequence(const YamlNode* node);
bool yaml_node_is_mapping(const YamlNode* node);
const char* yaml_node_get_name(const YamlParser* parser, const YamlNode* node);
const char* yaml_node_get_scalar_value(const YamlParser* p, const YamlNode* node);
YamlIter yaml_node_get_child_iter(const YamlParser* p, const YamlNode* n);
//YamlIter yaml_get_node_iter(const YamlNode* node);

void yaml_iter_next(YamlIter* iter);
bool yaml_iter_done(const YamlIter* iter);
const char* yaml_iter_get_name(const YamlIter* iter);
const char* yaml_iter_get_scalar_value(const YamlIter* iter);
const char* yaml_iter_get_child_scalar_value(const YamlIter* iter, const char* name);
//const YamlNode* yaml_iter_get_value(const YamlIter* iter);

#ifdef __cplusplus
}
#endif

#endif

