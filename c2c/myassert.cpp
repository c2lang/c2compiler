/* Copyright 2013 Bas van den Berg
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myassert.h"

using namespace C2;
using namespace std;

void Assert:: Assert::assert_equal(int expected, int real, const char* caller, int line) {
    if (real == expected) return;
    fprintf(stderr, "%s:%d  FAILED: expected %d, got %d\n", caller, line, expected, real);
    exit(-1);
}


void Assert::assert_equal(const void* expected, const void* real, const char* caller, int line) {
    if (real == expected) return;
    fprintf(stderr, "%s:%d  FAILED: expected %p, got %p\n", caller, line, expected, real);
    exit(-1);
}


void Assert::assert_not_equal(int expected, int real, const char* caller, int line) {
    if (real != expected) return;
    fprintf(stderr, "%s:%d  FAILED: should not be %d\n", caller, line, expected);
    exit(-1);
}


void Assert::assert_not_null(const void* real, const char* caller, int line) {
    if (real != NULL) return;
    fprintf(stderr, "%s:%d  FAILED: should not be NULL\n", caller, line);
    exit(-1);
}


void Assert::assert_null(const void* real, const char* caller, int line) {
    if (real == NULL) return;
    fprintf(stderr, "%s:%d  FAILED: should be NULL\n", caller, line);
    exit(-1);
}


void Assert::assert_true(bool real, const char* caller, int line) {
    if (real) return;
    fprintf(stderr, "%s:%d  FAILED: should be true\n", caller, line);
    exit(-1);
}


void Assert::assert_false(bool real, const char* caller, int line) {
    if (!real) return;
    fprintf(stderr, "%s:%d  FAILED: should be false\n", caller, line);
    exit(-1);
}


void Assert::assert_str_equal(const char* expected, const char* real, const char* caller, int line) {
    if (strcmp(real, expected) == 0) return;
    fprintf(stderr, "%s:%d  FAILED: expected: '\n%s' got: '\n%s'\n", caller, line, expected, real);
    exit(-1);
}


void Assert::assert_str_equal(const char* expected, const string& real, const char* caller, int line) {
    assert_str_equal(expected, real.c_str(), caller, line);
}


void Assert::assert_str_equal(const string& expected, const string& real, const char* caller, int line) {
    assert_str_equal(expected.c_str(), real.c_str(), caller, line);
}


void Assert::assert_str_not_equal(const char* expected, const char* real, const char* caller, int line) {
    if (strcmp(real, expected) != 0) return;
    fprintf(stderr, "%s:%d  FAILED: should not get '%s'\n", caller, line, real);
    exit(-1);
}


void Assert::assert_str_not_equal(const char* expected, const string& real, const char* caller, int line) {
    assert_str_not_equal(expected, real.c_str(), caller, line);
}


void Assert::assert_fail(const char* caller, int line) {
    fprintf(stderr, "%s:%d  FAILED: should not come here\n", caller, line);
    exit(-1);
}

