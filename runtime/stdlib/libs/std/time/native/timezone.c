/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef __ohos__
#include <dlfcn.h>

#define TIMESERVICE_NDK "libtime_service_ndk.so"
#define NDK_NAME "OH_TimeService_GetTimeZone"
#endif

#define MAX_BUF_LENGTH 64

char* CODE_TIME_GetLocalTimeZoneProperty()
{
#ifdef __android__
    FILE *fp = popen("getprop persist.sys.timezone", "r");
    if (!fp) {
        return NULL;
    }
    char buf[MAX_BUF_LENGTH];
    if (fgets(buf, sizeof(buf), fp)) {
        pclose(fp);
        buf[strcspn(buf, "\n")] = '\0';
        return strdup(buf); // Need to free
    }
    pclose(fp);
    return NULL;
#elif defined __ohos__
    void* timeNdk = dlopen(TIMESERVICE_NDK, RTLD_LAZY);
    if (timeNdk == NULL) {
        return NULL;
    }
    int (*getZoneFunc) (char*, uint32_t) = dlsym(timeNdk, NDK_NAME);
    if (getZoneFunc == NULL) {
        dlclose(timeNdk);
        return NULL;
    }
    char buf[MAX_BUF_LENGTH] = {0};
    int code = getZoneFunc(buf, MAX_BUF_LENGTH);
    dlclose(timeNdk);
    if (code != 0) {
        return NULL;
    }
    return strdup(buf);
#else
    return NULL;
#endif
}
