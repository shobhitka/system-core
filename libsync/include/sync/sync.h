/*
 *  sync.h
 *
 *   Copyright 2012 Google, Inc
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __SYS_CORE_SYNC_H
#define __SYS_CORE_SYNC_H

#include <sys/cdefs.h>
#include <stdint.h>

__BEGIN_DECLS

// XXX: These structs are copied from the header "linux/sync.h".

struct sync_file_info {
 char name[32];
 uint32_t status;
 uint32_t flags;
 uint32_t num_fences;
 uint32_t pad;

 intptr_t sync_fence_info;
};

struct sync_fence_info {
 char obj_name[32];
 char driver_name[32];
 uint32_t status;
 uint32_t flags;
 uint64_t timestamp_ns;
};

/* timeout in msecs */
int sync_wait(int fd, int timeout);
int sync_merge(const char *name, int fd1, int fd2);
struct sync_file_info *sync_file_info(int fd);
void sync_file_info_free(struct sync_file_info *info);
uint64_t sync_fence_timestamp(struct sync_file_info *info);

__END_DECLS

#endif /* __SYS_CORE_SYNC_H */
