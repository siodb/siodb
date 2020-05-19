// Copyright (C) 2019-2020 Siodb GmbH. All rights reserved.
// Use of this source code is governed by a license that can be found
// in the LICENSE file.

#include <openssl/md5.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void computeDatabaseUuid(const char* databaseName, time_t createTimestamp, unsigned char* uuid)
{
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, databaseName, strlen(databaseName));
    MD5_Update(&ctx, &createTimestamp, sizeof(createTimestamp));
    MD5_Final(uuid, &ctx);
}

int main()
{
    unsigned char uuid[16];
    computeDatabaseUuid("SYS", 1, uuid);
    printf("const Uuid Instance::kSystemDatabaseUuid { { ");
    for (size_t i = 0; i < sizeof(uuid); ++i) {
        const unsigned short b = uuid[i];
        if (i > 0) printf(", ");
        printf("0x%hx", b);
    }
    printf(" } };\n");
    return 0;
}
