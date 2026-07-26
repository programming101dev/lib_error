#ifndef LIBP101_ERROR_ATTRIBUTES_H
#define LIBP101_ERROR_ATTRIBUTES_H

/*
 * Copyright 2026 D'Arcy Smith.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if defined(__cplusplus) && !defined(restrict)
    #define restrict
#endif

#ifndef P101_ATTR_NORETURN
    #if defined(__cplusplus) && __cplusplus >= 201103L
        #define P101_ATTR_NORETURN [[noreturn]]
    #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
        #define P101_ATTR_NORETURN _Noreturn
    #elif defined(__GNUC__) || defined(__clang__)
        #define P101_ATTR_NORETURN __attribute__((noreturn))
    #else
        #define P101_ATTR_NORETURN
    #endif
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define P101_ATTR_PRINTF(format_index, first_arg) __attribute__((format(printf, format_index, first_arg)))
    #define P101_ATTR_SCANF(format_index, first_arg) __attribute__((format(scanf, format_index, first_arg)))
    #define P101_ATTR_STRFTIME(format_index) __attribute__((format(strftime, format_index, 0)))
    #define P101_ATTR_MALLOC __attribute__((malloc))
    #define P101_ATTR_ALLOC_SIZE(...) __attribute__((alloc_size(__VA_ARGS__)))
    #define P101_ATTR_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
    #define P101_ATTR_PRINTF(format_index, first_arg)
    #define P101_ATTR_SCANF(format_index, first_arg)
    #define P101_ATTR_STRFTIME(format_index)
    #define P101_ATTR_MALLOC
    #define P101_ATTR_ALLOC_SIZE(...)
    #define P101_ATTR_WARN_UNUSED_RESULT
#endif

#endif    // LIBP101_ERROR_ATTRIBUTES_H
