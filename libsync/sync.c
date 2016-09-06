/*
 *  sync.c
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

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <poll.h>
#include <linux/sync.h>
#include <linux/sw_sync.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

int sync_wait(int fd, int timeout)
{
    struct pollfd fds;
    int polled, ret = 0;

    if (fd < 0) {
        errno = EINVAL;
        return -1;
    }

    fds.fd = fd;
    fds.events = POLLIN;

    errno = 0;
    polled = poll(&fds, 1, timeout);
    if (!polled) {
        errno = ETIME;
        ret = -1;
    } else if (polled < 0) {
        ret = polled;
    }

    return ret;
}

int sync_merge(const char *name, int fd1, int fd2)
{
    struct sync_merge_data data;
    int err;

    data.fd2 = fd2;
    data.pad = 0;
    data.flags = 0;
    strlcpy(data.name, name, sizeof(data.name));

    err = ioctl(fd1, SYNC_IOC_MERGE, &data);
    if (err < 0)
        return err;

    return data.fence;
}

struct sync_file_info *sync_file_info(int fd)
{
    struct sync_file_info *info;
    struct sync_fence_info *fence_info;
    int err, num_fences;

    info = calloc(1, sizeof(*info));
    if (info == NULL)
        return NULL;

    err = ioctl(fd, SYNC_IOC_FILE_INFO, info);
    if (err < 0) {
        free(info);
        return NULL;
    }

    num_fences = info->num_fences;

    if (num_fences) {
        info->flags = 0;
        info->num_fences = num_fences;

        fence_info = calloc(num_fences, sizeof(*fence_info));
        if (!fence_info) {
            free(info);
            return NULL;
        }

        info->sync_fence_info = (uint64_t)(unsigned long) (fence_info);

        err = ioctl(fd, SYNC_IOC_FILE_INFO, info);
        if (err < 0) {
            free(fence_info);
            free(info);
            return NULL;
        }
    }

    return info;
}

void sync_file_info_free(struct sync_file_info *info)
{
    free((void *)(uintptr_t)info->sync_fence_info);
    free(info);
}

uint64_t sync_fence_timestamp(struct sync_file_info *info)
{
    uint64_t timestamp = 0;
    uint32_t i;
    intptr_t temp = ( intptr_t )info->sync_fence_info;
    struct sync_fence_info * fence_info = (struct sync_fence_info *)temp;
    for (i = 0 ; i < info->num_fences ; i++) {
        if (fence_info[i].timestamp_ns > timestamp) {
            timestamp = fence_info[i].timestamp_ns;
        }
    }

    return timestamp;
}

int sw_sync_timeline_create(void)
{
    return open("/dev/sw_sync", O_RDWR);
}

int sw_sync_timeline_inc(int fd, unsigned count)
{
    uint32_t arg = count;
    return ioctl(fd, SW_SYNC_IOC_INC, &arg);
}

int sw_sync_fence_create(int fd, const char *name, unsigned value)
{
    struct sw_sync_create_fence_data data;
    int err;

    data.value = value;
    strlcpy(data.name, name, sizeof(data.name));

    err = ioctl(fd, SW_SYNC_IOC_CREATE_FENCE, &data);
    if (err < 0)
        return err;

    return data.fence;
}
