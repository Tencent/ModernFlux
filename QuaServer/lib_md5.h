//  Copyright (C) 2019 Tencent. All rights reserved.
//
//  Licensed under the BSD 3-Clause License (the "License"); you may not use this file
//  except in compliance with the License. You may obtain a copy of the License at
//
//  https://opensource.org/licenses/BSD-3-Clause
//
//  Unless required by applicable law or agreed to in writing, software distributed
//  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
//  language governing permissions and limitations under the License.
//
#ifndef FLUX_QUASERVER_LIB_MD5_H_
#define FLUX_QUASERVER_LIB_MD5_H_

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#define MD5_BIN_LEN 16
#ifdef __cplusplus
extern "C" {
#endif
struct OI_MD5Context {
    u_int32_t buf[4];
    u_int32_t bits[2];
    unsigned char in[64];
};

#define MD5InitA(context) OI_MD5InitA(context)
#define MD5UpdateA(context, buf, len) OI_MD5UpdateA(context, buf, len)
#define MD5FinalA(digest, context) OI_MD5FinalA(digest, context)
#define MD5TransformA(buf, in) OI_MD5TransformA(buf, in)
#define Md5HashBuffer(outBuffer, inBuffer, length) OI_Md5HashBuffer(outBuffer, inBuffer, length)
#define Md5SumFile(outBuffer, inFileName) OI_Md5SumFile(outBuffer, inFileName)

void OI_MD5InitA(struct OI_MD5Context *context);
void OI_MD5UpdateA(struct OI_MD5Context *context, unsigned char const *buf, unsigned len);
void OI_MD5FinalA(unsigned char digest[MD5_BIN_LEN], struct OI_MD5Context *context);
void OI_MD5TransformA(u_int32_t buf[4], u_int32_t const in[MD5_BIN_LEN]);

/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */
typedef struct OI_MD5Context OI_MD5_CTX_A;
typedef struct OI_MD5Context OI_MD5_CTX;

// inBuffer->outBuffer , outBuffer [MD5_BIN_LEN]
void OI_Md5HashBuffer(uint8_t *outBuffer, const uint8_t *inBuffer, int length);
int OI_Md5SumFile(uint8_t *outBuffer, const char *cchpFileName);

#ifdef __cplusplus
}
#endif

#endif      // FLUX_QUASERVER_LIB_MD5_H_
