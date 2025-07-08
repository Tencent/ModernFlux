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
#ifndef FLUX_QUASERVER_UDPAPI_H_
#define FLUX_QUASERVER_UDPAPI_H_

#include <stdio.h>
#include <string>
#include <map>

int SendUDP(const std::string& ip, const int& port, const std::string& src, int& fd);
int MapSendUDP(const std::string& ip, const int& port, const std::string& src,
                std::map<std::string, int>& mapfd);

bool  IsLocalIP(const std::string& ip);
int GetLocalNormalIp(std::string& local_ip);

#endif  //  FLUX_QUASERVER_UDPAPI_H_

