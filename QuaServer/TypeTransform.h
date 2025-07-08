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
/* -*- C++ -*- */
//=============================================================================
/**
 *  @file       type_transform.h
 *  @brief      类型转换工具头文件
 *  @author     jimzhou
 *  @date       2016/08/04
 */
//=============================================================================

#ifndef FLUX_QUASERVER_TYPETRANSFORM_H_
#define FLUX_QUASERVER_TYPETRANSFORM_H_

#include <stdlib.h>
#include <stdint.h>
#include <sstream>
#include <algorithm>

/*! \class TypeTransform
 *  \brief 类型转换工具类
 */
class TypeTransform {
 public:
    template <class T>
    static std::string ToString(T Src) {
        std::stringstream sTemp;
        sTemp << Src;
        return sTemp.str();
    }

    //=============================================================================

    /*! \brief 将string转成整数
    * \param[in] sSour 要转换的string
    * \return 转换后的整数
    * \sa IntToString
    */
    static int StringToInt(const std::string& sSour) {
        return atoi(sSour.c_str());
    }


    /*! \brief 将整数格式化为string
    * \param[in] nSour 要格式化的整数
    * \return 格式化后的string
    * \sa StringToInt
    */
    static std::string IntToString(int nSour) {
        return TypeTransform::ToString(nSour);
    }


    //=============================================================================

    /*! \brief 将string转成长整数
    * \param[in] sSour 要转换的string
    * \return 转换后的长整数
    * \sa LongToString
    */
    static long StringToLong(const std::string& sSour) {
        return atol(sSour.c_str());
    }

    /*! \brief 将长整数格式化为string
    * \param[in] nSour 要格式化的长整数
    * \return 格式化后的string
    * \sa StringToLong
    */
    static std::string LongToString(long nSour) {
        return TypeTransform::ToString(nSour);
    }


    //=============================================================================

    /*! \brief 将string转成64位整数
    * \param[in] sSour 要转换的string
    * \return 转换后的64位整数
    * \sa LongToString
    */
    static long long StringToLongLong(const std::string& sSour) {
        return atoll(sSour.c_str());
    }

    /*! \brief 将64位整数格式化为string
    * \param[in] nSour 要格式化的64位整数
    * \return 格式化后的string
    * \sa StringToLong
    */
    static std::string LongLongToString(long long nSour) {
        return TypeTransform::ToString(nSour);
    }


    //=============================================================================

    /*! \brief 将string转成浮点数
    * \param[in] sSour 要转换的string
    * \return 转换后的浮点数
    * \sa FloatToString
    */
    static float StringToFloat(const std::string& sSour) {
        return (atof(sSour.c_str()));
    }

    /*! \brief 将浮点数格式化为string
    * \param[in] nSour 要格式化的浮点数
    * \return 格式化后的string
    * \sa StringToFloat
    */
    static std::string FloatToString(float nSour) {
        return TypeTransform::ToString(nSour);
    }


    //=============================================================================

    /*! \brief 将string转成双精度浮点数
    * \param[in] sSour 要转换的string
    * \return 转换后的双精度浮点数
    * \sa DoubleToString
    */
    static double StringToDouble(const std::string& sSour) {
        return (atof(sSour.c_str()));
    }

    /*! \brief 将双精度浮点数格式化为string
    * \param[in] nSour 要格式化的双精度浮点数
    * \return 格式化后的string
    * \sa StringToDouble
    */
    static std::string DoubleToString(double nSour) {
        return TypeTransform::ToString(nSour);
    }

    static unsigned long int StringToULong(const std::string& sSour)                 {
        return strtoul(sSour.c_str(), NULL, 10);
    }
    static unsigned long long int StringToULLong(const std::string& sSour)                 {
        return strtoull(sSour.c_str(), NULL, 10);
    }
    static std::string UShortToString(unsigned short nSour) {
        return TypeTransform::ToString(nSour);
    }
    static std::string ULongToString(unsigned long int nSour) {
        return TypeTransform::ToString(nSour);
    }
    static std::string ULLongToString(unsigned long long int nSour) {
        return TypeTransform::ToString(nSour);
    }
    static std::string LLongToString(long long int nSour) {
        return LongLongToString(nSour);
    }
    static std::string USToString(unsigned short nSour) {
        return UShortToString(nSour);
    }
    static std::string ULLToString(unsigned long long int nSour) {
        return ULLongToString(nSour);
    }
    static std::string ULToString(unsigned long int nSour) {
        return ULongToString(nSour);
    }
    static std::string UINT32ToString(uint32_t nSour) {
        return TypeTransform::ToString(nSour);
    }
    static std::string INT64ToString(uint64_t nSour) {
        return TypeTransform::ToString(nSour);
    }
    static uint64_t ToUINT64(const std::string& sSour)                 {
        return strtoull(sSour.c_str(), NULL, 10);
    }
    static uint32_t ToUINT32(const std::string& sSour)                 {
        return strtoul(sSour.c_str(), NULL, 10);
    }

    //=============================================================================

    /*! \brief 将char转成16进制的表示形式
    * \param[in] c 源字符
    * \return 源字符的16进制string
    * \sa HexToChar
    */
    static std::string CharToHex(char c) {
        std::string sValue;
        static char MAPX[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        sValue += MAPX[(c >> 4) & 0x0F];
        sValue += MAPX[c & 0x0F];
        return sValue;
    }

    /*! \brief 将字符的16进制的表示形式转换成char
    * \param[in] sHex 要转换的字符16进制的表示形式
    * \return 目标字符
    * \sa CharToHex
    */
    static char HexToChar(const std::string& sHex) {
        unsigned char c = 0;
        for (unsigned int i = 0; i < std::min<unsigned int>(sHex.size(), (unsigned int)2); ++i) {
            unsigned char c1 = std::toupper(sHex[i]);
            unsigned char c2 = (c1 >= 'A') ? (c1 - ('A' - 10)) : (c1 - '0');
            (c <<= 4) += c2;
        }
        return c;
    }
};

/*!
 *  \brief define a short name for TypeTransform
 */
typedef TypeTransform TypeTrans;


#endif  //  FLUX_QUASERVER_TYPETRANSFORM_H_
