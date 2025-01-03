/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#ifndef FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_UNIFIED_COLLECTION_DATA
#define FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_UNIFIED_COLLECTION_DATA

#include <cstdint>
#include <string>
#include <sys/ioctl.h>

// kernel struct, modify at the same time
struct ucollection_process_cpu_item {
    int pid;
    unsigned int thread_total;
    unsigned long long min_flt;
    unsigned long long maj_flt;
    unsigned long long cpu_usage_utime;
    unsigned long long cpu_usage_stime;
    unsigned long long cpu_load_time;
};

struct ucollection_process_filter {
    int uid;
    int pid;
    int tid;
};

struct ucollection_process_cpu_entry {
    int magic;
    unsigned int total_count;
    unsigned int cur_count;
    struct ucollection_process_filter filter;
    struct ucollection_process_cpu_item datas[];
};

struct ucollection_process_thread_count {
    int pid;
    unsigned int thread_count;
};

struct ucollection_thread_cpu_item {
    int tid;
    char name[16]; // 16 ：max length of thread name
    unsigned long long cpu_usage_utime;
    unsigned long long cpu_usage_stime;
    unsigned long long cpu_load_time;
};

struct ucollection_thread_filter {
    int uid;
    int pid;
    int tid;
};

struct ucollection_thread_cpu_entry {
    int magic;
    unsigned int total_count;
    unsigned int cur_count;
    struct ucollection_thread_filter filter;
    struct ucollection_thread_cpu_item datas[];
};

enum collection_type {
    COLLECT_ALL_PROC = 1,
    COLLECT_THE_PROC,
    COLLECT_APP_PROC,
    COLLECT_PROC_COUNT,
    COLLECT_THREAD_COUNT,
    COLLECT_APP_THREAD,
    COLLECT_THE_THREAD,
    COLLECT_APP_THREAD_COUNT,
};

#define PROCESS_TOTAL_COUNT 2500
#define IOCTRL_COLLECT_CPU_BASE 0
#define IOCTRL_COLLECT_ALL_PROC_CPU _IOR(IOCTRL_COLLECT_CPU_BASE, COLLECT_ALL_PROC, \
    struct ucollection_process_cpu_entry)
#define IOCTRL_COLLECT_THE_PROC_CPU _IOR(IOCTRL_COLLECT_CPU_BASE, COLLECT_THE_PROC, \
    struct ucollection_process_cpu_entry)
#define IOCTRL_COLLECT_PROC_COUNT _IOR(IOCTRL_COLLECT_CPU_BASE, COLLECT_PROC_COUNT, unsigned int)
#define IOCTRL_COLLECT_THREAD_COUNT _IOR(IOCTRL_COLLECT_CPU_BASE, COLLECT_THREAD_COUNT, \
    struct ucollection_process_thread_count)
#define IOCTRL_COLLECT_APP_THREAD_COUNT _IOR(IOCTRL_COLLECT_CPU_BASE, COLLECT_APP_THREAD_COUNT, \
    struct ucollection_process_thread_count)
#define IOCTRL_COLLECT_APP_THREAD _IOR(IOCTRL_COLLECT_CPU_BASE, COLLECT_APP_THREAD, struct ucollection_thread_cpu_entry)
#define IOCTRL_COLLECT_THE_THREAD _IOR(IOCTRL_COLLECT_CPU_BASE, COLLECT_THE_THREAD, struct ucollection_thread_cpu_entry)
#endif // FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_UNIFIED_COLLECTION_DATA
