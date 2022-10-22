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

#ifndef QUICKSORT_H
#define QUICKSORT_H

#include <stdbool.h>
#include <stddef.h>

#ifdef   __cplusplus
extern "C"
{
#endif

typedef bool (*quicksort_compare_fn)(void* arg, const void* left, const void* right);

void quicksort(void* items, size_t count, size_t item_size, quicksort_compare_fn is_less, void *arg);

#ifdef   __cplusplus
}
#endif

#endif

