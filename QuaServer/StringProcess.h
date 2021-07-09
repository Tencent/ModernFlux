//  Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
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
#ifndef FLUX_QUASERVER_STRINGPROCESS_H_
#define FLUX_QUASERVER_STRINGPROCESS_H_

#include <sys/time.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <map>

int RemoveTheTail(char *pSource, char *pTail);
std::string ToString(int iNum);
void _ToString(int iNum, std::string& val);
std::string USToString(unsigned short iNum);
std::string INTToString(int iNum);
std::string ULLToString(uint64_t num);
void _ULLToString(uint64_t num, std::string& val);
std::string ULToString(uint32_t iNum);
void _ULToString(uint32_t iNum, std::string& val);
std::string UINT32ToString(uint32_t iNum);
void _UINT32ToString(uint32_t iNum, std::string& val);
void _INT64ToString(int64_t num, std::string& val);
std::string INT64ToString(int64_t num);
uint64_t ToUINT64(std::string& strValue);

uint32_t ToUINT32(const std::string& strValue);

uint32_t ToUINT32(const char szArr[]);

int RReplaceNString(std::string& source, const std::string& strFind,
                    const std::string& strReplace, int num = 1);

bool IsSubString(const std::string& total, const std::string& sub);

int SplitString(const std::string& src, const std::string& delimiter,
                std::vector<std::string>& vectdest);
int SplitString2(const std::string& src, const std::string& delimiter,
                std::vector<std::string>& vectdest);
int ParseKV(const std::string& src, const std::string& delimiter, const std::string& delimiter2,
            std::map<std::string, std::string>& mapKV);
int32_t GetDlmNum(const std::string& src, const std::string& delimiter);
bool StringCompare(const std::string& first, const std::string& second);
int FilterString(const std::string& src, std::string& des, const char delimiter);
int RemoveToken(std::string& src, const char delimiter);

uint32_t HashTime33(char const *str, int len);

#endif  //  FLUX_QUASERVER_STRINGPROCESS_H_

