/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
    int total_count;
    int cur_count;
    struct ucollection_process_filter filter;
    struct ucollection_process_cpu_item datas[];
};

struct ucollection_cpu_dmips {
    int magic;
    int total_count;
    char dmips[];
};

#define PROCESS_TOTAL_COUNT 2500

#define IOCTRL_COLLECT_ALL_PROC_CPU_MAGIC 1
#define IOCTRL_COLLECT_THE_PROC_CPU_MAGIC 1
#define IOCTRL_SET_CPU_DMIPS_MAGIC 1
#define DMIPS_NUM 128

#define IOCTRL_COLLECT_CPU_BASE 0
#define IOCTRL_COLLECT_ALL_PROC_CPU _IOR(IOCTRL_COLLECT_CPU_BASE, 1, struct ucollection_process_cpu_entry)
#define IOCTRL_COLLECT_THE_PROC_CPU _IOR(IOCTRL_COLLECT_CPU_BASE, 2, struct ucollection_process_cpu_entry)
#define IOCTRL_SET_CPU_DMIPS _IOW(IOCTRL_COLLECT_CPU_BASE, 3, struct ucollection_cpu_dmips)
#endif // FRAMEWORK_NATIVE_UNIFIED_COLLECTION_COLLECTOR_UNIFIED_COLLECTION_DATA
