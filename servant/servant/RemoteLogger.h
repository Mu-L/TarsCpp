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

#include "servant/Global.h"
#include "servant/LogF.h"
#include "servant/PropertyReport.h"
#include "util/tc_base64.h"
#include "util/tc_file.h"
#include "util/tc_logger.h"
#include "util/tc_platform.h"
#include "util/tc_thread_rwlock.h"
#include "util/tc_singleton.h"

#define DYEING_DIR "tars_dyeing"
#define DYEING_FILE "dyeing"

namespace tars
{

/**
 * LOG的库说明:
 * 1 循环日志采用TLOGERROR(...),TLOGDEBUG(...)
 * 2 循环日志不上传到服务器
 * 3 按天日志采用DLOG, FDLOG来记录
 * 4 按天日志也可以不上传到远程服务器:DLOG("")->disableRemote();
 * 5 按天日志可以改变每天一个文件的方式:
 *   DLOG("abc3")->setFormat("%Y%m%d%H");
 *   每个小时一个文件
 */

/*****************************************************************************
实现方式说明(只介绍按时间的日志, 会写到tarslog):
    1 自定义时间日志的WriteT类:RemoteTimeWriteT
    2 在RemoteTimeWriteT类中, 写入到远程
    3 定义远程日志类:typedef TC_Logger<RemoteTimeWriteT, TC_RollByTime> RemoteTimeLogger;
    4 为了保证远程的写日志也是在单独线程处理,重新定义本地按天日志类
    5 自定义时间日志的WriteT类:TimeWriteT
    6 在TimeWriteT类中包含RemoteTimeLogger对象
    7 在TimeWriteT类的写入操作中, 写入本地文件后, 同时写入到RemoteTimeLogger对象中
    8 RemoteTimeLogger会在RemoteTimeWriteT对象中, 异步写入到远程
    9 从而本地文件写和远程写不在一个线程中.
*****************************************************************************/
//class LogProxy;
//typedef tars::TC_AutoPtr<LogProxy> LogPrx;

///////////////////////////////////////////////////////////////////////////////

class SVT_DLL_API RollWriteT
{
public:
    RollWriteT();
    ~RollWriteT();

    void operator()(ostream &of, const deque<pair<size_t, string> > &ds);

    void setDyeingLogInfo(const string &sApp, const string &sServer, const string & sLogPath, int iMaxSize, int iMaxNum, const LogPrx &logPrx);

protected:

    TC_RollLogger *_dyeingRollLogger;

    string _app;
    string _server;
    string _logPath;
    int _maxSize;
    int _maxNum;

    /**
     * 染色远程滚动日志代理
     */
    LogPrx                _logPrx;

    map<string, string>   _context;
};


/**
 * 本地日志帮助类, 单件
 * 循环日志单件是永生不死的, 保证任何地方都可以使用
 * 当该对象析够以后, 则直接cout出来
 */
class SVT_DLL_API LocalRollLogger : public TC_Singleton<LocalRollLogger, CreateUsingNew, PhoneixLifetime>
{
public:
    enum
    {
        NONE_LOG    = 1,    /**所有的log都不写*/
        ERROR_LOG   = 2,    /**写错误log*/
        WARN_LOG    = 3,    /**写错误,警告log*/
        INFO_LOG    = 4,    /**写错误,警告,调试,Info log*/
        DEBUG_LOG   = 5,    /**写错误,警告,调试log*/
        TARS_LOG    = 6     /**写错误,警告,调试,Info log*/
    };

    typedef TC_Logger<RollWriteT, TC_RollBySize> RollLogger;

    /**
     * 析构
     */
    ~LocalRollLogger();

    /**
     * 设置本地信息
     * @param app, 业务名称
     * @param server, 服务名称
     * @param logpath, 日志路径
     * @param iMaxSize, 文件最大大小,字节
     * @param iMaxNum, 文件最大数
     */
    void setLogInfo(const string &sApp, const string &sServer, const string &sLogpath, int iMaxSize = 1024*1024*50, int iMaxNum = 10, const CommunicatorPtr &comm=NULL, const string &sLogObj="");

    /**
     * 设置同步写日志
     *
     * @param bSync
     */
    void sync(bool bSync = true);

    /**
     * 获取循环日志
     *
     * @return RollLogger
     */
    RollLogger *logger()          { return &_logger; }

	/**
	 * 获取循环日志, 以suffix为文件名后缀
	 *
	 * @return RollLogger
	 */
	RollLogger *logger(const string &suffix);

    /**
     * 染色日志是否启用
     * @param bEnable
     */
    void enableDyeing(bool bEnable, const string& sDyeingKey = "");

    /**
     * 终止
     */
    void terminate();

  protected:
    /**
     * 应用
     */
    string                  _app;

    /**
     * 服务名称
     */
    string                  _server;

    /**
     * 日志路径
     */
    string                  _logpath;

    /**
     * log obj
     */
    string 					_logObj;

    /**
     * 循环日志
     */
    RollLogger              _logger;

    /**
     * 扩展日志
     */
    unordered_map<string, RollLogger*>	_logger_ex;

    /**
     * lock
     */
	TC_ThreadRWLocker		_mutex;

    /**
     * 本地线程组
     */
    TC_LoggerThreadGroup    _local;

	/**
	 * 染色远程滚动日志代理
	 */
	LogPrx                _logPrx;

    /**
     * 是否结束
     */
    bool _terminate = false;
};

///////////////////////////////////////////////////////////////////////////////////////
//
/**
 * 写日志线程
 * 将写本地日志和远程分开到不同的线程
 * 作为单件存在, 且是永生不死的单件
 */
class SVT_DLL_API TarsLoggerThread : public TC_Singleton<TarsLoggerThread, CreateUsingNew, PhoneixLifetime>
{
public:
    /**
     * 构造函数
     */
    TarsLoggerThread();

    /**
     * 析够函数
     */
    ~TarsLoggerThread();

    /**
     * 本地写日志线程
     */
    TC_LoggerThreadGroup *local();

    /**
     * 远程写日志线程
     *
     * @return TC_LoggerThreadGroup*
     */
    TC_LoggerThreadGroup *remote();

    /**
    * 终止
    */
    void terminate();

  protected:
    /**
     * 本地线程组
     */
    TC_LoggerThreadGroup _local;

    /**
     * 远程写线程组
     */
    TC_LoggerThreadGroup _remote;

    /**
     * 是否已经终止
     */
    bool _terminate = false;
};

///////////////////////////////////////////////////////////////////////////////////////
class TimeWriteT;

/**
 * 远程的Log写操作类
 */
class SVT_DLL_API RemoteTimeWriteT
{
public:
    RemoteTimeWriteT();
    ~RemoteTimeWriteT();

    /**
     * 构造函数
     */
    void setTimeWriteT(TimeWriteT *pTimeWrite);

    /**
     * 具体调用
     * @param of
     * @param buffer
     */
    void operator()(ostream &of, const deque<pair<size_t, string> > &buffer);

protected:
    /**
     * 同步到远程
     */
    void sync2remote(const LogInfo &stInfo, const vector<string> &buffer);

    /**
     * 染色日志同步到远程
     */
    void sync2remoteDyeing(const vector<string> &buffer);

protected:
    /**
     * 指针
     */
    TimeWriteT          *_timeWrite;

};

////////////////////////////////////////////////////////////////////////////
/**
 * 写Logger
 */
class SVT_DLL_API TimeWriteT
{
public:
    typedef TC_Logger<RemoteTimeWriteT, TC_RollByTime> RemoteTimeLogger;

    typedef TC_Logger<TC_DefaultWriteT, TC_RollByTime> DyeingTimeLogger;

    /**
     * 构造
     */
    TimeWriteT();

    /**
     * 析够
     */
    ~TimeWriteT();

    /**
     * 设置基本信息
     * @param app, 应用名称
     * @param server, 服务名称
     * @param file, 日志文件名
     * @param sFormat, 格式
     * @param setdivision,set名称
     * @param sLogType,日志记录类型
     */
    void setLogInfo(const LogPrx &logPrx, const string &sApp, const string &sServer, const string &sFile, const string &sLogpath, const string &sFormat, const string& setdivision = "", const string& sLogType = "", const PropertyReportPtr &reportSuccPtr = NULL, const PropertyReportPtr &reportFailPtr = NULL);

    /**
     * 设置代理
     * @param logPrx 代理信息
     */
    void setLogPrx(const LogPrx &logPrx);

    /**
     * 远程日志功能打开或关闭
     * @param bEnable
     */
    void enableRemote(bool bEnable)         { _remote = bEnable; }

    /**
     * 本地日志功能功能打开或关闭
     * @param bEnable
     */
    void enableLocal(bool bEnable);

    /**
     * 染色日志功能打开或关闭
     * @param bEnable
     */
    void enableDyeing (bool bEnable, const string& sDyeingKey = "");


    /**
     * @brief 日志文件名是否带.log后缀
     * @param bEnable
     */
    void enableSufix(bool bEnable=true){_hasSufix = bEnable;}
    /**
     * @brief 是否允许框架在日志文件名上增加业务相关的标识
     * @param bEnable
     */
    void enablePrefix(bool bEnable=true){_hasAppNamePrefix = bEnable;}
    /**
     * @brief 日志文件名中用户自定义字符与日期字符间的连接符，默认是"_"
     * @param str
     */
    void setFileNameConcatStr(const string& str) {_concatStr = str;}

    /**
     * @brief 框架中增加的日志内容之间的分割符，默认是"|"
     * @param str
     */
    void setSeparator(const string& str) {_separ = str;}

    /**
     * @brief 框架中日期和时间之间是否需要加中括号[],有些统计由特殊需求；默认不加
     * @param bEnable
     */
    void enableSqareWrapper(bool bEnable) {_hasSquareBracket = bEnable;}

    /**
     * 设置时间格式("%Y%m%d")
     * @param sFormat
     */
    void setFormat(const string &sFormat)   { _format = sFormat;}

    /**
     * 具体调用
     * @param of
     * @param buffer
     */
    void operator()(ostream &of, const deque<pair<size_t, string> > &buffer);

protected:

    /**
     * 友元
     */
    friend class RemoteTimeWriteT;

    /**
     * 记录错误文件
     * @param buffer
     */
    void writeError(const vector<string> &buffer);

    /**
     * 记录错误文件
     * @param buffer
     */
    void writeError(const deque<pair<size_t, string> > &buffer);

    /**
     * 初始化logger
     */
    void initError();

    /**
     * 初始化染色日志
     */
    void initDyeingLog();

protected:

    /**
     * 远程时间日志
     */
    RemoteTimeLogger    *_remoteTimeLogger;

    /**
     * 本地功能
     */
    bool                _local;

    /**
     * 远程功能
     */
    bool                _remote;

    /**
     * 远程服务句柄
     */
    LogPrx              _logPrx;

    /**
     * app名称
     */
    string              _app;

    /**
     * 服务名称
     */
    string              _server;

    /**
     * 日志文件名称
     */
    string              _file;

    /**
     * 时间格式
     */
    string              _format;

    /**
     * 具体文件
     */
    string              _filePath;

    /**
     * 错误文件
     */
    TC_DayLogger        _logger;

    /**
     * 缺省写模式
     */
    TC_DefaultWriteT    _wt;

    /**
     * 染色日志
     */
    static int          _dyeing;

    /**
     * 染色日志目录路径
     */
    string              _dyeingFilePath;

    /**
     * 远程时间日志
     */
    DyeingTimeLogger    *_dyeingTimeLogger;

    /**
     * set分组信息
     */
    string               _setDivision;

    /**
     * 日志文件名是否带.log后缀
     */
    bool                 _hasSufix;
    /**
     * 是否允许框架在日志文件名上增加业务相关的标识
     */
    bool                  _hasAppNamePrefix;

    /**
     * 日志文件名中用户自定义字符与日期字符间的连接符，默认是"_"
     */
    string                  _concatStr;
    /**
     * 分隔符
     */
     string              _separ;
    /**
     * 日期部分是否加上[]
     */
     bool                  _hasSquareBracket;

     /*
      * 本地日志的记录类型,格式为LogType.toString()返回值,如果不采用LogType，则该值为""
      */
     string              _logType;
     
     /*
     * 对于远程日志，上报同步到logser的成功量，默认不上报
     */
     PropertyReportPtr   _reportSuccPtr;

     /*
     * 对于远程日志，上报同步到logser的失败量，默认不上报
     */
     PropertyReportPtr   _reportFailPtr;
}; 

////////////////////////////////////////////////////////////////////////////
/**
 * 远程日志帮助类, 单件
 */
class SVT_DLL_API RemoteTimeLogger : public TC_HandleBase
                    , public TC_ThreadLock
                    , public TC_Singleton<RemoteTimeLogger, CreateUsingNew, DefaultLifetime>
{
public:

    //定义按时间滚动的日志
    typedef TC_Logger<TimeWriteT, TC_RollByTime> TimeLogger;

    /**
     * 构造
     */
    RemoteTimeLogger();

    /**
     * 析够
     */
    ~RemoteTimeLogger();

    /**
     * 设置本地信息
     * @param comm, 通信器
     * @param obj, 日志对象名称
     * @param app, 业务名称
     * @param server, 服务名称
     * @param logpath, 日志路径
     */
    void setLogInfo(const CommunicatorPtr &comm, const string &obj, const string &sApp, const string &sServer, const string &sLogpath,const string& setdivision="", const bool &bLogStatReport=false);

    /**
     * 初始化设置时间格式("%Y%m%d")
     * 不要动态修改, 线程不安全
     * 如果有需要, 初始化后直接修改
     * @param sFormat, 文件名称, 为空表示缺省的时间日志
     */
    void initFormat(const string &sFile, const string &sFormat,const LogTypePtr& logTypePtr=NULL);
    void initFormat(const string &sApp, const string &sServer,const string &sFile, const string &sFormat,const LogTypePtr& logTypePtr=NULL);
    /**
     * 初始化设置时间格式("%Y%m%d")
     * 不要动态修改, 线程不安全
     * 如果有需要, 初始化后直接修改
     * @param sFormat, 文件名称, 为空表示缺省的时间日志
     * @param frequency 支持每多少天/小时/分钟，详见TC_logger.h中关于TarsLogByDay,TarsLogByHour,TarsLogByMinute的描述
     *
     * 用法: 按两个小时记录日志
     * initFormat<TarsLogByHour>("logfile",TarsLogByHour::FORMAT,2);
     */
    template<class TLogType>
    void initFormatWithType(const string &sFile, const string &sFormat,size_t frequency)
    {
        LogTypePtr logTypePtr = new TLogType(sFormat,frequency);
        initFormat(sFile,sFormat,logTypePtr);
    }

    template<class TLogType>
    void initFormatWithType(const string &sApp, const string &sServer,const string &sFile, const string &sFormat,size_t frequency)
    {
        LogTypePtr logTypePtr = new TLogType(sFormat,frequency);
        initFormat(sApp,sServer,sFile,sFormat,logTypePtr);
    }
    /**
     * 获取时间日志
     * @param file
     */
    TimeLogger *logger(const string &sFile = "");

    /**
     * 获取时间日志
     * @param app, 业务名称
     * @param server, 服务名称
     * @param file
     */
    TimeLogger *logger(const string &sApp, const string &sServer,const string &sFile = "");

    /**
     * 同步写本地时间日志(远程日志一定是异步写的, 无法调整)
     * @param bSync
     */
    void sync(const string &sFile, bool bSync);

    /**
     * 远程时间日志
     * @param sFile, 文件名称, 为空表示缺省的时间日志
     * @param bEnable
     */
    void enableRemote(const string &sFile, bool bEnable);

    /**
     * 远程时间日志
     * @param sApp,应用名称
     * @param sServer,服务名称
     * @param sFile, 文件名称, 为空表示缺省的时间日志
     * @param bEnable
     */
    void enableRemoteEx(const string &sApp, const string &sServer,const string &sFile, bool bEnable);
    /**
     * 本地时间日志
     * @param sFile,文件名称, 为空表示缺省的时间日志
     * @param bEnable
     */
    void enableLocal(const string &sFile, bool bEnable);
    /**
     * 本地时间日志
     * @param sApp,应用名称
     * @param sServer,服务名称
     * @param sFile, 文件名称, 为空表示缺省的时间日志
     * @param bEnable
     */
    void enableLocalEx(const string &sApp, const string &sServer,const string &sFile, bool bEnable);

    /**
     * @brief 日志文件名是否带.log后缀,影响全部日志文件
     * @param bEnable
     */
    void enableSufix(bool bEnable=true){_hasSufix = bEnable;}
    /**
     * @brief 是否允许框架在日志文件名上增加业务相关的标识,影响全部日志文件
     * @param bEnable
     */
    void enablePrefix(bool bEnable=true){_hasAppNamePrefix = bEnable;}
    /**
     * @brief 日志文件名中用户自定义字符与日期字符间的连接符，默认是"_",影响全部日志文件
     * @param str
     */
    void setFileNameConcatStr(const string& str) {_concatStr = str;}

    /**
     * @brief 框架中增加的日志内容之间的分割符，默认是"|",影响全部日志文件
     * @param str
     */
    void setSeparator(const string& str) {_separ = str;}

    /**
     * @brief 框架中日期和时间之间是否需要加中括号[],有些统计由特殊需求；默认不加,影响全部日志文件
     * @param bEnable
     */
    void enableSqareWrapper(bool bEnable) {_hasSquareBracket = bEnable;}
    /**
     * @brief 是否输出本地日志文件，影响全部日志文件
     * @param bEnable
     */
    void enableLocalLog(bool bEnable) {_local = bEnable;}
    /**
     * @brief 是否输出远程日志文件,影响全部日志文件
     * @param bEnable
     */
    void enableRemoteLog(bool bEnable) {_remote = bEnable;}
    /**
     * 终止
     */
    void terminate();
protected:

    /**
     * 初始化时间日志
     * @param pTimeLogger
     * @param sFile
     * @param sFormat
     * @param frequence, 每多少天/小时/分钟,单位是秒
     */
    void initTimeLogger(TimeLogger *pTimeLogger, const string &sFile, const string &sFormat,const LogTypePtr& logTypePtr=NULL);

    /**
     * 初始化时间日志
     * @param pTimeLogger
     * @param sApp
     * @param sServer
     * @param sFile
     * @param sFormat
     * @param frequence, 每多少天/小时/分钟,单位是秒
     */

    void initTimeLogger(TimeLogger *pTimeLogger,const string &sApp, const string &sServer, const string &sFile, const string &sFormat,const LogTypePtr& logTypePtr=NULL);

protected:

    /**
     * 通信器
     */
    CommunicatorPtr         _comm;

    /**
     * 远程服务句柄
     */
    LogPrx                  _logPrx;

    /**
     * 应用
     */
    string                  _app;

    /**
     * 服务名称
     */
    string                  _server;

    /**
     * 日志路径
     */
    string                  _logpath;

    /**
     * 缺省按天日志
     */
    TimeLogger              *_defaultLogger;

    /**
     * 远程日志
     */
    map<string, TimeLogger*>  _loggers;
    /**
     * set分组信息
     */
    string                   _setDivision;

    /**
     * 是否带.log后缀
     */
    bool                     _hasSufix;
    /**
     * 是否允许框架在日志文件名上增加业务相关的标识
     */
    bool                      _hasAppNamePrefix;

    /**
     * 日志文件名中用户自定义字符与日期字符间的连接符，默认是"_"
     */
    string                      _concatStr;
    /**
     * 分隔符
     */
    string                   _separ;
    /**
     * 日期部分是否加上[]
     */
    bool                      _hasSquareBracket;
    /**
     * 是否输出本地日志
     */
    bool                     _local;
    /**
     * 是否输出远程日志
     */
    bool                     _remote;

    /*
    * 服务日志上报logser是否上报成功数量
    */
    bool                     _logStatReport;
	
    /**
     * 是否已经终止
     */
    bool _terminate = false;	
};

/**
 * 染色开关类，析构时关闭
 */
class SVT_DLL_API TarsDyeingSwitch
{
public:
    /**
     * 构造函数，默认不打开染色日志
     */
    TarsDyeingSwitch()
    :_needDyeing(false)
    {
    }

    /**
     * 析构函数，关闭已打开的染色日志
     */
    ~TarsDyeingSwitch();

    /**
     * 获取染色的key
     *
     * @param key
     * @return bool
     */
    static bool getDyeingKey(string & sDyeingkey);

    /**
     * 启用染色日志
     */
    void enableDyeing(const string & sDyeingKey = "");

protected:
    bool _needDyeing;
    string _dyeingKey;
};

/**
 * 循环日志
 */
#define LOG (LocalRollLogger::getInstance()->logger())

#define LOG_DEBUG LOG->debug() << FILE_FUNC_LINE << "|"
#define LOG_ERROR LOG->error() << FILE_FUNC_LINE << "|"

#define LOG_EX(x) (LocalRollLogger::getInstance()->logger(x))

#define LOG_DEBUG_EX(x) LOG_EX(x)->debug() << FILE_FUNC_LINE << "|"
#define LOG_ERROR_EX(x) LOG_EX(x)->error() << FILE_FUNC_LINE << "|"

/**
 * @brief 按级别循环日志宏
 *
 * @param level 日志等级,LocalRollLogger::INFO_LOG,LocalRollLogger::DEBUG_LOG,LocalRollLogger::WARN_LOG,LocalRollLogger::ERROR_LOG
 * @msg 日志内容语句,包括<<重定向符连接的语句,如 "Demo begin" << " testing !" <<endl;
 *
 * @用法:
 *       标准输出流方式: cout << "I have " << vApple.size() << " apples!"<<endl;
 *       框架宏方式:     LOGMSG(LocalRollLogger::INFO_LOG,"I have " << vApple.size() << " apples!"<<endl);
 */
#if TARGET_PLATFORM_WINDOWS
#define LOGMSG(level, ...)                  \
    do                                      \
    {                                       \
        if (LOG->isNeedLog(level))          \
            LOG->log(level) << __VA_ARGS__; \
    } while (0)
#define LOG_MSG(level, ...)                                          \
    do                                                               \
    {                                                                \
        if (LOG->isNeedLog(level))                                   \
            LOG->log(level) << FILE_FUNC_LINE << "|" << __VA_ARGS__; \
    } while (0)

#define LOGEXMSG(x, level, ...)                  \
    do                                      \
    {                                       \
        if (x->isNeedLog(level))          \
            x->log(level) << __VA_ARGS__; \
    } while (0)
#define LOGEX_MSG(x, level, ...)                                          \
    do                                                               \
    {                                                                \
        if (x->isNeedLog(level))                                   \
            x->log(level) << FILE_FUNC_LINE << "|" << __VA_ARGS__; \
    } while (0)

#else
#define LOGMSG(level, msg...)       \
    do                              \
    {                               \
        if (LOG->isNeedLog(level))  \
            LOG->log(level) << msg; \
    } while (0)
#define LOG_MSG(level, msg...)                               \
    do                                                       \
    {                                                        \
        if (LOG->isNeedLog(level))                           \
            LOG->log(level) << FILE_FUNC_LINE << "|" << msg; \
    } while (0)

#define LOGEXMSG(x, level, msg...)       \
    do                              \
    {                               \
        if (x->isNeedLog(level))  \
            x->log(level) << msg; \
    } while (0)
#define LOGEX_MSG(x, level, msg...)                               \
    do                                                       \
    {                                                        \
        if (x->isNeedLog(level))                           \
            x->log(level) << FILE_FUNC_LINE << "|" << msg; \
    } while (0)

#endif

/**
 * @brief 按级别循环日志
 *
 * @msg 日志内容语句,包括<<重定向符连接的语句,如 "Demo begin" << " testing !" <<endl;
 *
 * @用法:
 *       标准输出流方式: cout << "I have " << vApple.size() << " apples!"<<endl;
 *       框架宏方式:     TLOGINFO("I have " << vApple.size() << " apples!"<<endl);
 */
#if TARGET_PLATFORM_WINDOWS
#define TLOGTARS(...)    LOGMSG(LocalRollLogger::TARS_LOG,__VA_ARGS__)
#define TLOGINFO(...)    LOGMSG(LocalRollLogger::INFO_LOG,__VA_ARGS__)
#define TLOGDEBUG(...)   LOGMSG(LocalRollLogger::DEBUG_LOG,__VA_ARGS__)
#define TLOGWARN(...)    LOGMSG(LocalRollLogger::WARN_LOG,__VA_ARGS__)
#define TLOGERROR(...)   LOGMSG(LocalRollLogger::ERROR_LOG,__VA_ARGS__)
#define TLOG_TARS(...)    LOG_MSG(LocalRollLogger::TARS_LOG,__VA_ARGS__)
#define TLOG_INFO(...)    LOG_MSG(LocalRollLogger::INFO_LOG,__VA_ARGS__)
#define TLOG_DEBUG(...)   LOG_MSG(LocalRollLogger::DEBUG_LOG,__VA_ARGS__)
#define TLOG_WARN(...)    LOG_MSG(LocalRollLogger::WARN_LOG,__VA_ARGS__)
#define TLOG_ERROR(...)   LOG_MSG(LocalRollLogger::ERROR_LOG,__VA_ARGS__)

#define TLOGEXTARS(x, ...) LOGEXMSG(LOG_EX(x), LocalRollLogger::TARS_LOG, __VA_ARGS__)
#define TLOGEXTARS(x, ...) LOGEXMSG(LOG_EX(x), LocalRollLogger::TARS_LOG, __VA_ARGS__)
#define TLOGEXINFO(x, ...) LOGEXMSG(LOG_EX(x), LocalRollLogger::INFO_LOG, __VA_ARGS__)
#define TLOGEXDEBUG(x, ...) LOGEXMSG(LOG_EX(x), LocalRollLogger::DEBUG_LOG, __VA_ARGS__)
#define TLOGEXWARN(x, ...) LOGEXMSG(LOG_EX(x), LocalRollLogger::WARN_LOG, __VA_ARGS__)
#define TLOGEXERROR(x, ...) LOGEXMSG(LOG_EX(x), LocalRollLogger::ERROR_LOG, __VA_ARGS__)

#define TLOGEX_TARS(x, ...) LOGEX_MSG(LOG_EX(x), LocalRollLogger::TARS_LOG, __VA_ARGS__)
#define TLOGEX_TARS(x, ...) LOGEX_MSG(LOG_EX(x), LocalRollLogger::TARS_LOG, __VA_ARGS__)
#define TLOGEX_INFO(x, ...) LOGEX_MSG(LOG_EX(x), LocalRollLogger::INFO_LOG, __VA_ARGS__)
#define TLOGEX_DEBUG(x, ...) LOGEX_MSG(LOG_EX(x), LocalRollLogger::DEBUG_LOG, __VA_ARGS__)
#define TLOGEX_WARN(x, ...) LOGEX_MSG(LOG_EX(x), LocalRollLogger::WARN_LOG, __VA_ARGS__)
#define TLOGEX_ERROR(x, ...) LOGEX_MSG(LOG_EX(x), LocalRollLogger::ERROR_LOG, __VA_ARGS__)
#else
#define TLOGTARS(msg...)    LOGMSG(LocalRollLogger::TARS_LOG,msg)
#define TLOGINFO(msg...)    LOGMSG(LocalRollLogger::INFO_LOG,msg)
#define TLOGDEBUG(msg...)   LOGMSG(LocalRollLogger::DEBUG_LOG,msg)
#define TLOGWARN(msg...)    LOGMSG(LocalRollLogger::WARN_LOG,msg)
#define TLOGERROR(msg...)   LOGMSG(LocalRollLogger::ERROR_LOG,msg)
#define TLOG_TARS(msg...)    LOG_MSG(LocalRollLogger::TARS_LOG,msg)
#define TLOG_INFO(msg...)    LOG_MSG(LocalRollLogger::INFO_LOG,msg)
#define TLOG_DEBUG(msg...)   LOG_MSG(LocalRollLogger::DEBUG_LOG,msg)
#define TLOG_WARN(msg...)    LOG_MSG(LocalRollLogger::WARN_LOG,msg)
#define TLOG_ERROR(msg...)   LOG_MSG(LocalRollLogger::ERROR_LOG,msg)
#define TLOGEXTARS(x, msg...) LOGEXMSG(LOG_EX(x), LocalRollLogger::TARS_LOG, msg)
#define TLOGEXINFO(x, msg...) LOGEXMSG(LOG_EX(x), LocalRollLogger::INFO_LOG, msg)
#define TLOGEXDEBUG(x, msg...) LOGEXMSG(LOG_EX(x), LocalRollLogger::DEBUG_LOG, msg)
#define TLOGEXWARN(x, msg...) LOGEXMSG(LOG_EX(x), LocalRollLogger::WARN_LOG, msg)
#define TLOGEXERROR(x, msg...) LOGEXMSG(LOG_EX(x), LocalRollLogger::ERROR_LOG, msg)

#define TLOGEX_TARS(x, msg...) LOGEX_MSG(LOG_EX(x), LocalRollLogger::TARS_LOG, msg)
#define TLOGEX_INFO(x, msg...) LOGEX_MSG(LOG_EX(x), LocalRollLogger::INFO_LOG, msg)
#define TLOGEX_DEBUG(x, msg...) LOGEX_MSG(LOG_EX(x), LocalRollLogger::DEBUG_LOG, msg)
#define TLOGEX_WARN(x, msg...) LOGEX_MSG(LOG_EX(x), LocalRollLogger::WARN_LOG, msg)
#define TLOGEX_ERROR(x, msg...) LOGEX_MSG(LOG_EX(x), LocalRollLogger::ERROR_LOG, msg)
#endif

/**
 * 按天日志
 */
#define DLOG            (RemoteTimeLogger::getInstance()->logger()->any())
#define FDLOG(x)        (RemoteTimeLogger::getInstance()->logger(x)->any())
#define FFDLOG(x,y,z)   (RemoteTimeLogger::getInstance()->logger(x,y,z)->any())

//如果编译时指定了DEBUG，则不输出远程日志，直接输出到标准窗口
#ifdef DEBUG
#define DEBUG_DAY_LOG LOG_DEBUG
#define ERR_DAY_LOG LOG_ERROR
#define FDLOG_DEBUG(x) LOG_DEBUG
#else
#define DEBUG_DAY_LOG FDLOG("debug") << FILE_FUNC_LINE << "|"
#define ERR_DAY_LOG FDLOG("error") << FILE_FUNC_LINE << "|"
#define FDLOG_DEBUG(x) FDLOG(x) << FILE_FUNC_LINE << "|"
#endif

//////////////////////////////////////////////

/**
 *  按天日志局部使能开关，针对单个日志文件进行使能，请在所有按天日志输出前调用
 */
#define TENREMOTE_FDLOG(swith, sApp, sServer, sFile) (RemoteTimeLogger::getInstance()->enableRemoteEx(sApp, sServer, sFile, swith))
#define TENLOCAL_FDLOG(swith, sApp, sServer, sFile) (RemoteTimeLogger::getInstance()->enableLocalEx(sApp, sServer, sFile, swith))

/**
 * 按天日志全局使能开关，请在所有按天日志输出前调用
 */
#define TENREMOTE(swith) (RemoteTimeLogger::getInstance()->enableRemoteLog(swith))
#define TENLOCAL(swith) (RemoteTimeLogger::getInstance()->enableLocalLog(swith))
} 

