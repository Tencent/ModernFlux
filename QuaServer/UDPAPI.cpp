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
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <string>
#include <sstream>
#include <iostream>
#include "UDPAPI.h"

using namespace std;
int SendUDP(const string& ip, const int& port, const string& src, int& fd) {
    if (0 == fd) {
        struct sockaddr_in servaddr;
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        connect(fd, reinterpret_cast<sockaddr *>(&servaddr), sizeof(servaddr));
    }

    int len = send(fd, src.c_str(), src.size(), 0);

    if (len <= 0) {
        close(fd);
        fd = 0;
    }

    return len;
}

int MapSendUDP(const string& ip, const int& port, const string& src, map<string, int>& mapfd) {
    stringstream ss;
    ss << ip << "_" << port;
    map<string, int>::iterator it = mapfd.find(ss.str());
    if (mapfd.end() == it) {
        mapfd.insert(make_pair(ss.str(), 0));
    }

    if (0 == mapfd[ss.str()]) {
        struct sockaddr_in servaddr;
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        connect(fd, reinterpret_cast<sockaddr *>(&servaddr), sizeof(servaddr));
        mapfd[ss.str()] = fd;
    }

    int len = send(mapfd[ss.str()], src.c_str(), src.size(), 0);

    if (len <= 0) {
        close(mapfd[ss.str()]);
        mapfd[ss.str()]  = 0;
    }

    return len;
}

static char s_interip_pre[] = "127.0.0.";
int GetLocalNormalIp(std::string& local_ip) {
    struct ifaddrs *address = NULL;
    struct ifaddrs *address_free = NULL;
    void *tmp_address_ptr = NULL;

    int ret = ::getifaddrs(&address);
    if (ret)
        return -1;

    address_free = address;

    while (address != NULL) {
        if (address->ifa_addr->sa_family == AF_INET) {
            tmp_address_ptr = &((struct sockaddr_in *)address->ifa_addr)->sin_addr;
            char address_buffer[INET_ADDRSTRLEN] = {0};
            inet_ntop(AF_INET, tmp_address_ptr, address_buffer, INET_ADDRSTRLEN);

            if (strstr(address->ifa_name, "tunl") || strstr(address->ifa_name, "lo")
                || strstr(address->ifa_name, "tunnat")) {
                address = address->ifa_next;
                continue;
            }
            if (strncmp(address_buffer, s_interip_pre, strlen(s_interip_pre)) == 0) {
                address = address->ifa_next;
                continue;
            }

            local_ip = address_buffer;
            ::freeifaddrs(address_free);
            return 0;
        }
        address = address->ifa_next;
    }

    ::freeifaddrs(address_free);
    return -2;
}

bool  IsLocalIP(const std::string& ip) {
    //  10.0.0.0-10.255.255.255
    if (strstr(ip.c_str(), "10.") == ip.c_str() || strstr(ip.c_str(), "192.168.") == ip.c_str() ||
            strstr(ip.c_str(), "169.254.") == ip.c_str()) {
        return true;
    }
    //  172.16.0.0-172.31.255.255
    if (strstr(ip.c_str(), "172.") == ip.c_str()) {
        std::string str = ip.substr(4, 3);
        int num = atoi(str.c_str());
        if (num > 15 && num < 32) {
            return true;
        }
    }
    //  100.64.0.0   100.127.255.255
    if (strstr(ip.c_str(), "100.") == ip.c_str()) {
        std::string str = ip.substr(4, 3);
        int num = atoi(str.c_str());
        if (num > 63 && num < 128) {
            return true;
        }
    }
    //  add 9.  for special use
    if (strstr(ip.c_str(), "9.") == ip.c_str()) {
        return true;
    }
    return false;
}


