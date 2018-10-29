/* Copyright 2018 Christoffer Lernö
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

#define FATAL_ERROR(fmt) \
  do { fprintf(stderr, "FATAL ERROR in %s:%d:%s(): " fmt "\n", __FILE__, \
      __LINE__, __func__); exit(-1); } while (0)

#define FATAL_ERRORF(fmt, ...) \
  do { fprintf(stderr, "FATAL ERROR in %s:%d:%s(): " fmt "\n", __FILE__, \
      __LINE__, __func__, __VA_ARGS__); exit(-1); } while (0)

#define TODO do { fprintf(stderr, "Exit due to TODO in %s:%d:%s()\n", __FILE__, __LINE__, __func__); exit(-2); } while (0)
