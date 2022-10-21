/* Copyright Bas van den Berg 2022 */

#include "QuickSort.h"

#include <stdint.h>

static void swap(uint8_t* item, uint8_t* other, size_t size) {
    const uint8_t* end = item + size;
    while (item < end) {
        uint8_t tmp = *other;
        *other++ = *item;
        *item++ = tmp;
    }
}

void quicksort(void* items, size_t count, size_t item_size, quicksort_compare_fn is_less, void *arg)
{
    if (count <= 1) return;

    uint8_t* begin = (uint8_t*)items;
    uint8_t* end = begin + count * item_size;

    uint8_t* left = begin;
    uint8_t* pivot = begin + (count / 2) * item_size;
    uint8_t* right = end - item_size;

    do {
        while (is_less(arg, left, pivot)) left += item_size;
        while (is_less(arg, pivot, right)) right -= item_size;

        if (left < right) {
            swap(left, right, item_size);
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
        quicksort(begin, part_items, item_size, is_less, arg);
    }

    if (left < end) {
        size_t part_items = (end - left) / item_size;
        quicksort(left, part_items, item_size, is_less, arg);
    }
}

