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

#ifndef MYASSERT_H
#define MYASSERT_H

#include <string>
#include <stdlib.h>
#include <stdio.h>

namespace C2 {

class Assert {
public:
    static void assert_equal(int expected, int real, const char* caller, int line);
    static void assert_equal(const void* expected, const void* real, const char* caller, int line);
#define ASSERT_EQUAL(a,b) Assert::assert_equal(a, b, __FILE__, __LINE__)

    static void assert_not_equal(int expected, int real, const char* caller, int line);
#define ASSERT_NOT_EQUAL(a,b) Assert::assert_not_equal(a, b, __FILE__, __LINE__)

    static void assert_not_null(const void* real, const char* caller, int line);
#define ASSERT_NOT_NULL(a)  Assert::assert_not_null(a, __FILE__, __LINE__)

    static void assert_null(const void* real, const char* caller, int line);
#define ASSERT_NULL(a)  Assert::assert_null(a, __FILE__, __LINE__)

    static void assert_true(bool real, const char* caller, int line);
#define ASSERT_TRUE(a)  Assert::assert_true(a, __FILE__, __LINE__)

    static void assert_false(bool real, const char* caller, int line);
#define ASSERT_FALSE(a)  Assert::assert_false(a, __FILE__, __LINE__)
    
    static void assert_str_equal(const char* expected, const char* real, const char* caller, int line);
    static void assert_str_equal(const char* expected, const std::string& real, const char* caller, int line);
    static void assert_str_equal(const std::string& expected, const std::string& real, const char* caller, int line);
#define ASSERT_STR_EQUAL(a,b)  Assert::assert_str_equal(a, b, __FILE__, __LINE__)

    static void assert_str_not_equal(const char* expected, const char* real, const char* caller, int line);
    static void assert_str_not_equal(const char* expected, const std::string& real, const char* caller, int line);
#define ASSERT_STR_NOT_EQUAL(a,b)  Assert::assert_str_not_equal(a, b, __FILE__, __LINE__)

    template <typename T> static void assert_class_equal(const T& a, const T& b,  const char* caller, int line)
    {
        if (a == b) return;
        printf("%s:%d  FAILED: not equal\n", caller, line);
        exit(-1);
    }
#define ASSERT_CLASS_EQUAL(a,b)  Assert::assert_class_equal(a, b , __FILE__, __LINE__)

    static void assert_fail(const char* caller, int line);
#define ASSERT_FAIL()  Assert::assert_fail(__FILE__, __LINE__)
};

}

#endif

