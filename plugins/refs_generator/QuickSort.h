/* Copyright Bas van den Berg 2022 */

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

