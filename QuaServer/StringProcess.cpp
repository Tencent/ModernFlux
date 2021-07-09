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
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "TypeTransform.h"
#include "StringProcess.h"
using namespace std;

int RemoveTheTail(char *pSource, char *pTail) {
    int iSrcLen = strlen(pSource);
    int iTailLen = strlen(pTail);

    if (iSrcLen > iTailLen) {
        char *pEnd = pSource + (iSrcLen - iTailLen);
        if (strcmp(pEnd, pTail) == 0) {
            *pEnd = 0;
            return 1;
        }
    }
    return -1;
}

string ToString(int iNum) {
    char szNUM[128] = {0};
    snprintf(szNUM, sizeof(szNUM) - 1, "%d", iNum);
    return szNUM;
}

void _ToString(int iNum, string& val) {
    char szNUM[32] = {0};
    snprintf(szNUM, sizeof(szNUM) - 1, "%d", iNum);
    val = szNUM;
    return;
}

string ULLToString(uint64_t num) {
    return TypeTransform::ToString(num);
}

void _ULLToString(uint64_t num, std::string& val) {
    val = TypeTransform::ToString(num);
    return;
}

string ULToString(uint32_t iNum) {
    return TypeTransform::ToString(iNum);;
}

void _ULToString(uint32_t iNum, std::string& val) {
    val = TypeTransform::ToString(iNum);
    return;
}

string UINT32ToString(uint32_t iNum) {
    return TypeTransform::ToString(iNum);
}

void _UINT32ToString(uint32_t iNum, std::string& val) {
    val = TypeTransform::ToString(iNum);
    return;
}

string USToString(unsigned short iNum) {
    char szNUM[32] = {0};
    snprintf(szNUM, sizeof(szNUM) - 1, "%hu", iNum);
    return szNUM;
}

std::string INTToString(int iNum) {
    char szNUM[32] = {0};
    snprintf(szNUM, sizeof(szNUM) - 1, "%d", iNum);
    return szNUM;
}

void _USToString(unsigned short iNum, std::string& val) {
    char szNUM[32] = {0};
    snprintf(szNUM, sizeof(szNUM) - 1, "%hu", iNum);
    val = szNUM;
    return;
}

string INT64ToString(int64_t num) {
    return TypeTransform::ToString(num);
}

void _INT64ToString(int64_t num, std::string& val) {
    val = TypeTransform::ToString(num);
    return;
}

uint64_t ToUINT64(string& strValue) {
    char *endptr;
    return std::strtoll(strValue.c_str(), &endptr, 10);
}

uint32_t ToUINT32(const string& strValue) {
    return ToUINT32(strValue.c_str());
}

uint32_t ToUINT32(const char szArr[]) {
    char *endptr;
    uint64_t tmp = std::strtoll(szArr, &endptr, 10);
    if (tmp >= 0xFFFFFFFF) {
        tmp = 0xFFFFFFFF;
        //  logE(DEBUG,"[F:%s][L:%d]  ToUINT32 %s \n", __FILE__,__LINE__, szArr);
    }
    return tmp;
}

int RReplaceNString(string& source, const string& strFind, const string& strReplace, int num) {
    for (int i = 0; i < num; i++) {
        size_t found = source.rfind(strFind);
        if (found != std::string::npos) {
            source.erase(found, std::string::npos);
        }
    }
    source += strReplace;

    return 0;
}

bool IsSubString(const string& total, const string& sub) {
    size_t found = total.find(sub);
    if (found != string::npos)
        return true;

    return false;
}

int SplitString(const string& src, const string& delimiter, vector<string>& vectdest) {
    for (size_t index = 0; index < src.size();) {
        size_t found = src.find(delimiter, index);
        if (found == string::npos) {
            if (index <= (src.size() - 1))
                vectdest.push_back(src.substr(index, string::npos));

            break;
        } else {
            if (found > index)
                vectdest.push_back(src.substr(index, found - index));

            if ((found + delimiter.size()) <= (src.size() - 1)) {
                index = found + delimiter.size();
            } else {
                break;
            }
        }
    }

    return 0;
}

int SplitString2(const string& src, const string& delimiter, vector<string>& vectdest) {
    for (size_t index = 0; index < src.size();) {
        size_t found = src.find(delimiter, index);
        if (found == string::npos) {
            if (index <= (src.size() - 1)) {
                vectdest.push_back(src.substr(index, string::npos));
            }

            break;
        } else {
            if (found >= index) {
                vectdest.push_back(src.substr(index, found - index));
            }

            if ((found + delimiter.size()) <= (src.size() - 1)) {
                index = found + delimiter.size();
            } else {
                vectdest.push_back("");
                break;
            }
        }
    }

    return 0;
}

int ParseKV(const std::string& src, const std::string& delimiter1, const std::string& delimiter2,
            std::map<std::string, std::string>& mapKV) {
    vector<string> vecRes;
    SplitString(src, delimiter1, vecRes);
    for (size_t i = 0; i < vecRes.size(); i++) {
        vector<string> vecKV;
        SplitString(vecRes[i], delimiter2, vecKV);
        if (vecKV.size() == 2) {
            if (vecKV[0].size() > 0) {
                if (vecKV[1].size() > 0) {
                    mapKV.insert(pair<string, string>(vecKV[0], vecKV[1]));
                } else {
                    mapKV.insert(pair<string, string>(vecKV[0], ""));
                }
            }
        } else if (vecKV.size() == 1) {
            if (vecKV[0].size() > 0)
                mapKV.insert(pair<string, string>(vecKV[0], ""));
        } else if (vecKV.size() == 3) {
            if (vecKV[0].size() > 0)
                mapKV.insert(pair<string, string>(vecKV[0], vecKV[1] + "=" + vecKV[2]));
        }
    }
    return 0;
}


int32_t GetDlmNum(const string& src, const string& delimiter) {
    int32_t num = 0;
    for (size_t index = 0; index < src.size();) {
        size_t found = src.find(delimiter, index);
        if (found == string::npos) {
            break;
        } else {
            num++;
            if ((found + delimiter.size()) <= (src.size() - 1))
                index = found + delimiter.size();
            else
                break;
        }
    }

    return num;
}

bool StringCompare(const std::string& first, const std::string& second) {
    return (first < second);
}



int FilterString(const std::string& src, std::string& des, const char delimiter) {
    for (size_t i = 0; i < src.size(); i++) {
        if (src[i] != delimiter)
            des += src[i];
    }

    return 0;
}

int RemoveToken(std::string& src, const char delimiter) {
    string des;
    FilterString(src, des, delimiter);
    src = des;

    return 0;
}


uint32_t HashTime33(char const *str, int len) {
    unsigned long  hash = 5381;
    for (int i = 0; i < len; i++) {
        hash += ((hash << 5) & 0x7FFFFFF) + str[i];
    }
    return (hash & 0x7FFFFFF);
}

