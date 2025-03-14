﻿/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */
#pragma once

#include "util/tc_socket.h"
#include "util/tc_clientsocket.h"
#include "servant/EndpointF.h"

#if TARGET_PLATFORM_WINDOWS
#include <WS2tcpip.h>
#endif

using namespace std;

namespace tars
{
/**
 * 地址信息IP:Port
 */
class SVT_DLL_API EndpointInfo
{
public:

    /**
     * 构造函数
     */
    EndpointInfo();

    /**
     *
     * @param ep
     */
	EndpointInfo(const TC_Endpoint &ep, const string &setDivision="");

	/**
     * 构造函数
     * @param host
     * @param port
     * @param type
     */
	EndpointInfo(const EndpointF &ep);

    /**
     * 转换
     * @param ep
     * @return
     */
    static TC_Endpoint toEndpointF(const EndpointF &ep);

    /**
     * 转换
     * @param ep
     * @return
     */
    static EndpointF toEndpointF(const TC_Endpoint &ep, const string &nodeName);

    /**
     * 转换数据结构
     * 如果是0.0.0.0 则换成本机实际ipv4的所有地址(多网卡), 注意去掉了127.0.0.1
     * 如果是::1, 则换成本机实际ipv6的所有地址(多网卡), 注意去掉了::1
     * 如果是*, 则换成本机所有的ipv4/ipv6的所有地址(多网卡), 注意去掉了127.0.0.1 和 ::1
     * @param ep
     * @param nodeName
     * @return
     */
    static vector<EndpointF> toEndpointFs(const TC_Endpoint &ep, const string &nodeName);

    /**
     * 转换成字符串, 方便输出
     * @param eps
     * @param complete: 是否完整转换
     * @return complete: false->ip1:port,ip2:port2,...  complete: true -> tcp -h .. -p ..:tcp -h .. -p ..
     */
    static string vectorEndpointFToStr(const vector<EndpointF> &eps, bool complete = true);

    /**
     * 字符串转换成vector<EndpointF>
     * @param str, tcp -h .. -p ..:tcp -h .. -p ..
     * @param nodeName
     * @return
     */
    static vector<EndpointF> strToVectorEndpointF(const string &str, const string &nodeName);

    /**
     * 按照节点名称归并
     * @return <节点名称, [地址]>
     */
    static map<string, vector<EndpointF>> toNodeEndpointF(const vector<tars::EndpointF> &eps);

    /**
     * get endpoint
     * @return
     */
	inline const TC_Endpoint &getEndpoint() const { return _ep; }

	/**
	 *
	 * @return
	 */
	inline TC_Endpoint &getEndpoint() { return _ep; }


	/**
     * 地址的字符串描述(用于比较)
     *
     * @return string
     */
	inline const string & cmpDesc() const { return _cmpDesc; }

	/**
     * 地址的字符串描述
     *
     * @return string
     */
    inline const string & desc() const { return _desc; }

    /**
     *
     * @return
     */
	inline bool isTcp() const { return _ep.isTcp(); }

    /**
     *
     * @return
     */
	inline bool isSsl() const { return _ep.isSSL(); }

    /**
     * 
     *
     */  
	inline bool isUdp() const { return _ep.isUdp(); }

	/**
     * 获取主机名
     *
     * @return const string&
     */
    const string &host() const { return _ep.getHost(); }

    /**
     * 获取端口号
     *
     * @return uint16_t
     */
    uint16_t port() const { return _ep.getPort(); }

    /**
     * 获取路由状态(不再使用)
     * @return int32_t
     */
    inline int32_t grid() const { return _ep.getGrid(); }

    /*
     * 获取qos的descp值
     */
    inline int32_t qos() const {return _ep.getQos();}

    /*
     * 获取节点的静态权重值
     */
    inline int weight() const {return _ep.getWeight();}

    /**
     * @brief 获取节点的权重使用方式
     */
    inline unsigned int getWeightType() const { return _ep.getWeightType(); }


    /**
     * 返回端口类型
     *
     * @return EndpointInfo::EType
     */
    inline TC_Endpoint::EType type() const { return _ep.getType(); }


    /**
    *返回set分组信息
    *
    *@return string
    */
    inline const string& setDivision() const { return _setDivision; }

    /*
     * 获取认证类型
     */
    inline TC_Endpoint::AUTH_TYPE authType() const  { return _ep.getAuthType(); }

    /**
     * @brief is ipv6 socket or not
     * @return true if is ipv6
     */
    inline bool isIPv6() const  { return _ep.isIPv6(); }

    /**
     * 等于
     * @param r
     *
     * @return bool
     */
    inline bool operator == (const EndpointInfo& r) const { return (_cmpDesc == r._cmpDesc); }

    /**
     * 小于
     * @param r
     *
     * @return bool
     */
    inline bool operator < (const EndpointInfo& r) const { return (_cmpDesc < r._cmpDesc); }

protected:

    /**
     * 地址的字符串描述
     * @param bWithSetInfo,标识
     * @return string
     */
    string createDesc() const { return _ep.toString(); }

    /**
     * 详细地址字符串描述
     * 
     * @return string 
     */
    string createCompareDesc();

private:
	/**
	 * 服务地址
	 */
    TC_Endpoint _ep;


	/**
	 *set分组信息
	 */
    string                 _setDivision;


    /**
     * 比较的地址字符串描述
     */
    string                 _cmpDesc;

    /**
     * 地址的字符串描述
     */
    string                 _desc;


};
/////////////////////////////////////////////////////////////////////////////
}
