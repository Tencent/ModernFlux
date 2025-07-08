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
#ifndef FLUX_QUASERVER_FILEDATAMETA_H_
#define FLUX_QUASERVER_FILEDATAMETA_H_

#include <sstream>
#include <fstream>
#include "lib_md5.h"

#define TMPLT_DATA_OFFLINE_TIME (31*24*60*60)

template<typename F>
bool TmlptActLoadFile(const std::string& file_path, F& tact, std::stringstream& ss_err) {
    //  FTRACE
    static char s_file_buff[1024 * 1024 * 10] = {0};
    char strMyMD5[MD5_BIN_LEN + 1] = {0};
    std::ifstream is(file_path.c_str(), std::ifstream::binary);

    if (is) {
        is.seekg(0, is.end);
        uint32_t length = is.tellg();
        is.seekg(0, is.beg);

        if (length >= sizeof(s_file_buff)) {
            ss_err << file_path << ":file size exceeding:" << length;
            return false;
        }

        if (length <= (4 + MD5_BIN_LEN)) {
            ss_err << file_path << ":load file fail, file len:" << length;
            return false;
        }

        is.read(s_file_buff, length);

        if (!is) {
            ss_err << file_path << " open fail";
            is.close();
            return false;
        }

        char *pdata = s_file_buff + 4 + MD5_BIN_LEN;
        uint32_t *tmp = (uint32_t *)s_file_buff;
        uint32_t data_len = ntohl(*tmp);
        if (length != (4 + MD5_BIN_LEN + data_len)) {
            ss_err << file_path << " format error, file len:" <<
                    length << " data len:" << (data_len + 4) << " md5 length:" <<
                    MD5_BIN_LEN;
            is.close();
            return false;
        }

        OI_Md5HashBuffer(reinterpret_cast<uint8_t *>(strMyMD5),
            (const uint8_t *)(s_file_buff + 4 + MD5_BIN_LEN),
            data_len);

        if (memcmp(strMyMD5, s_file_buff + 4, MD5_BIN_LEN) != 0) {
            std::string file_md5(s_file_buff + 4, MD5_BIN_LEN);
            ss_err << file_path << " md5 check fail" << std::endl << "file:" << file_md5;
            ss_err << std::endl << " real:" << strMyMD5;
            ss_err << std::endl;
            is.close();
            return false;
        }

        if (!tact.ParseFromArray(pdata, data_len)) {
            ss_err << file_path << " ParseFromArray error";
            is.close();
            return false;
        }
    } else {
        ss_err << file_path << " open fail";
        return false;
    }

    is.close();

    return true;
}

#define TMPLT_DEFINE_ITEM(STUCTNAME, DATA)\
    typedef struct\
    {\
        DATA data;\
        time_t file_time;\
        time_t load_time;\
        time_t update_time;\
        std::string md5;\
    }STUCTNAME;

#define GROUP_NUM 2
#define TMPLT_DEFINE_ARR(STUCTNAME, ITEM)\
    typedef struct\
    {\
        ITEM data_arr[GROUP_NUM];\
        int cur;\
        void init()\
        {\
            cur = 0;\
            for (int i = 0; i < GROUP_NUM; i ++) {\
                data_arr[i].file_time = 0;\
                data_arr[i].load_time = 0;\
            }\
        }\
    } STUCTNAME;
#define TMPLT_DECLARE_FUNC(funcname, keytype, datatype, arrtype)\
	int funcname(string &filename, const keytype &keyname, datatype *&pdata,\
	int &index, std::map<keytype, arrtype> &gmap, uint32_t &flag, int64_t &delta);
#define TMPLT_DEFINE_FUNC(funcname, keytype, datatype, arrtype)\
    int funcname(string &filename, const keytype &keyname, datatype *&pdata, \
                int &index, std::map<keytype, arrtype> &gmap, uint32_t &flag, int64_t &delta)\
    {\
        ActTimeDelta acttd(delta);\
        std::stringstream ss_err;\
        struct stat stat_buf;\
        int ret = stat(filename.c_str(), &stat_buf);\
        if (ret !=0)\
            return -1;\
        time_t file_time = stat_buf.st_mtime;\
        flag = 0;\
        std::map<keytype, arrtype >::iterator it = gmap.find(keyname);\
        if (gmap.end() == it) {\
            arrtype dataitem;\
            dataitem.init();\
            int begin = 0;\
            if (TmlptActLoadFile(filename, dataitem.data_arr[begin].data, ss_err)) {\
                flag = 1;\
                dataitem.data_arr[begin].file_time = file_time;\
                dataitem.data_arr[begin].load_time = time(NULL);\
                dataitem.data_arr[begin].update_time = time(NULL);\
                dataitem.cur = begin;\
                gmap.insert(make_pair(keyname, dataitem));\
            } else {\
                flag = 6;\
                SF_LOG(LOG_ERROR, "Loading first fail! %s,%s\n", \
                filename.c_str(), ss_err.str().c_str());\
            }\
        } else {\
            int cur_index = it->second.cur;\
            time_t last_file_time = it->second.data_arr[cur_index].file_time;\
            if (last_file_time < file_time) {\
                time_t the_cur_time = time(NULL);\
                time_t last_time = it->second.data_arr[cur_index].load_time;\
                if ((the_cur_time - last_time) < INDEX_DIFF_TIME) {\
                    flag = 3;\
                    stringstream ss;\
                    ss << filename.c_str() << " update to fre:" << the_cur_time << ","\
                        << last_time << " cur:" << (the_cur_time - last_time)\
                        << " object:" << INDEX_DIFF_TIME << endl;\
                } else {\
                    datatype data;\
                    if (TmlptActLoadFile(filename, data, ss_err)) {\
                        flag = 2;\
                        int update_index = (cur_index+1)%GROUP_NUM;\
                        it->second.data_arr[update_index].data.Clear();\
                        it->second.data_arr[update_index].data = data;\
                        it->second.data_arr[update_index].file_time = file_time;\
                        it->second.data_arr[update_index].load_time = time(NULL);\
                        it->second.data_arr[update_index].update_time = time(NULL);\
                        it->second.cur = update_index;\
                    } else {\
                        flag = 5;\
                        SF_LOG(LOG_ERROR, "Loading update fail! %s,%s\n", \
                        filename.c_str(), ss_err.str().c_str());\
                    }\
                }\
            } else {\
                flag = 4;\
            }\
        }\
        std::map<keytype, arrtype>::iterator it1 = gmap.find(keyname);\
        if (it1 != gmap.end()) {\
            int cur = it1->second.cur;\
            index = cur;\
            pdata = &(it1->second.data_arr[cur].data);\
            SF_LOG(LOG_DEBUG, "Loading find:! flag: %s, index %d, %d, %s\n", \
                tmplt_flag_err[flag%tmplt_flag_err_len], cur, file_time, filename.c_str());\
            return 0;\
        }\
        SF_LOG(LOG_ERROR, "Loading not find! flag %d, %d, %s\n", \
            flag, file_time, filename.c_str());\
        return 1;\
    }

#define TMPLT_DEFINE_ERASE_FUNC(funcname, maptype, endname)\
    bool funcname(maptype &tmap) {\
        time_t cur_time = time(NULL);\
        SF_LOG(LOG_ERROR, "map size, %d\n", tmap.size());\
        maptype::iterator it = tmap.begin();\
        for (; it != tmap.end();) {\
            int cur = it->second.cur;\
            SF_LOG(LOG_DEBUG, "erasing, %s\n", \
                it->second.data_arr[cur].data.DebugString().c_str());\
            bool flag = (it->second.data_arr[cur].data.has_##endname() && \
                         (it->second.data_arr[cur].data.endname().size() > 0));\
            if ( !flag) {\
                ++it;\
                SF_LOG(LOG_DEBUG, "erasing, %s\n", "continue");\
                continue;\
            }\
            const string &str = it->second.data_arr[cur].data.endname();\
            time_t end_time = strtotime(str.c_str(), TIME_FORMAT_YmdHMS1);\
            if ((cur_time > end_time) && ((cur_time - end_time) > TMPLT_DATA_OFFLINE_TIME)) {\
                SF_LOG(LOG_ERROR, "erasing, %s\n", "erasing");\
                tmap.erase(it++);\
            } else {\
                ++it;\
                SF_LOG(LOG_DEBUG, "erasing, %s\n", "skip");\
            }\
        }\
        return true;\
    }

#define TMPLT_DEFINE_ERASE_FUNC_NUM(funcname, maptype, endname)\
    bool funcname(maptype &tmap) {\
        time_t cur_time = time(NULL);\
        SF_LOG(LOG_ERROR, "map size, %d\n", tmap.size());\
        maptype::iterator it = tmap.begin();\
        for (; it != tmap.end();) {\
            int cur = it->second.cur;\
            SF_LOG(LOG_DEBUG, "erasing, %s\n", \
                it->second.data_arr[cur].data.DebugString().c_str());\
            bool flag = (it->second.data_arr[cur].data.has_##endname() && \
                         (it->second.data_arr[cur].data.endname() > 0));\
            if ( !flag ) {\
                ++it;\
                SF_LOG(LOG_DEBUG, "erasing, %s\n", "continue");\
                continue;\
            }\
            int64_t end_time = it->second.data_arr[cur].data.endname();\
            if ((cur_time > end_time) && ((cur_time - end_time) > TMPLT_DATA_OFFLINE_TIME)) {\
                tmap.erase(it++);\
                SF_LOG(LOG_ERROR, "erasing, %s\n", "erasing");\
            } else {\
                ++it;\
                SF_LOG(LOG_DEBUG, "erasing, %s\n", "skip");\
            }\
        }\
        return true;\
    }
#endif  //  FLUX_QUASERVER_FILEDATAMETA_H_
