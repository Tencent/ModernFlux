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
#ifndef FLUX_QUASERVER_DATAWRAP_H_
#define FLUX_QUASERVER_DATAWRAP_H_

#include<string>
#include<map>

class WrapMap {
 private:
    std::map<std::string, std::string>& m_map;
    std::string object;
 public:
    explicit WrapMap(std::map<std::string, std::string>& tmap): m_map(tmap) {}
    const std::string& operator[](const std::string& key) {
        return  m_map.find(key) != m_map.end() ? m_map[key] : object;
    }
};


class WrapString {
 private:
    std::string& m_str;
 public:
    explicit WrapString(std::string& tstr): m_str(tstr) {}
    int AddString(const std::string& key, const std::string& val) {
        if ((key.size() > 0) && (val.size() > 0)) {
            if (m_str.size() > 0)
                m_str += "&";
            m_str += key + "=" + val;
        }
        return 0;
    }
};

class WrapMemo {
 private:
    char *m_pdata;
    int m_len;
    WrapMemo() {}
 public:
    explicit WrapMemo(int len) {
        m_pdata = NULL;
        m_len = 0;
        if (len > 0) {
            m_pdata = new char[len];
            //  SF_LOG(LOG_DEBUG, "WrapMemo new:%d", len);
            m_pdata[0] = 0;
            m_len = len;
        }
    }
    ~WrapMemo() {
        if (m_pdata) {
            delete [] m_pdata;
            //  SF_LOG(LOG_DEBUG, "WrapMemo delete:%d", m_len);
        }
    }
    char *GetMem() {
        return m_pdata;
    }

    int GetLen() {
        return m_len;
    }
};
#endif  //  FLUX_QUASERVER_DATAWRAP_H_

