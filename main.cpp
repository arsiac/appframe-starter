#include <iostream>
#include <string>
#include "afdef.h"
#include "common.h"
#include "Properties.h"

#if defined(WINDOWS)

#include <Windows.h>
#include <cstring>

#elif defined(UNIX) || defined(LINUX)
#include <io.h>
#endif

#define MAX_PATH_LENGTH 1024

std::string programDirectory;
std::string tomcatLocation;
std::string javaHome;
std::string javaOptions;
std::string warFile;
std::string bsHomeDirectory;
std::string targetDirectory;

std::string tomcatShutdownPort;
std::string tomcatHttpPort;
std::string tomcatHttpsPort;
std::string tomcatJmxPort;
std::string tomcatAjpPort;

/**
 * 确认可以使用环境变量配置的必须项
 *
 * @param name 环境变量名称
 * @param properties 配置信息
 * @param key 配置键
 * @param value 获取到的值
 * @return 是否配置成功
 */
bool checkEnv(const char *name, Properties *properties, const char *key, std::string &value) {
    // check configuration
    value = convent2string(properties->get(key));
    if (isBlank(value)) {
        if (enableDebug) {
            std::cout << "[DEBUG] " << key << " not found." << std::endl;
        }
        // check environment variable
        value = convent2string(getenv(name));
        if (isBlank(value)) {
            std::cout << key << " is \"\", please set " << name << " or " << key << std::endl;
            return false;
        } else {
            printKeyValue(name, value);
        }
    } else {
        printKeyValue(name, value);
    }

    return true;
}

/**
 * 确认非必需项
 *
 * @param properties 配置信息
 * @param key 键
 * @param value 值
 * @param defaultValue 默认值
 */
void checkNoRequired(Properties *properties, const char *key, std::string &value, const char *defaultValue) {
    value = convent2string(properties->get(key));
    if (isBlank(value)) {
        if (enableDebug) {
            std::cout << "[DEBUG] " << key << " not found, use default value: "
                      << defaultValue << std::endl;
        }
        value = defaultValue;
    } else {
        if (enableDebug) {
            std::cout << "[DEBUG] " << key << ": " << value << std::endl;
        }
    }
}

/**
 * 确认必须的配置项
 *
 * @param properties 配置信息
 * @return 是否配置正确
 */
bool checkCommonConfiguration(Properties *properties) {
    // debug
    std::string enableDebugStr;
    checkNoRequired(properties, COMMON_DEBUG_ENABLE, enableDebugStr, "true");
    if (enableDebugStr == "true") {
        enableDebug = true;
        std::cout << "[INFO ] enable debug." << std::endl;
    } else if (enableDebugStr == "false") {
        enableDebug = false;
        std::cout << "[INFO ] disable debug." << std::endl;
    } else {
        std::cout << "[ERROR] " << COMMON_DEBUG_ENABLE
                  << " cannot be " << enableDebugStr
                  << "." << std::endl;
        return false;
    }

    // CATALINA_HOME
    bool envSuccess = checkEnv("CATALINA_HOME", properties, COMMON_TOMCAT_LOCATION, tomcatLocation);
    if (!envSuccess) {
        return false;
    }

    if (!fileExist(tomcatLocation + TOMCAT_SERVER_XML)) {
        std::cout << "[ERROR] it's not a tomcat home directory: " << tomcatLocation << std::endl;
        return false;
    }

    // JAVA_HOME
    envSuccess = checkEnv("JAVA_HOME", properties, COMMON_JAVA_HOME, javaHome);
    if (!envSuccess) {
        return false;
    }

    // 检查JRE_HOME是否存在
    std::string jreHome = convent2string(getenv("JRE_HOME"));
    if (!isBlank(jreHome)) {
        printKeyValue("JRE_HOME", jreHome);
        std::cout << "[Warn] Tomcat uses the JRE_HOME in preference."
                  << "If you want to change Java version, please remove the JRE_HOME environment variable."
                  << std::endl;
    }

    // JAVA_OPTS + common.java.options
    javaOptions = convent2string(getenv("JAVA_OPTS"));
    if (isBlank(javaOptions)) {
        if (enableDebug) {
            std::cout << "[DEBUG] environment variable JAVA_OPTS not found." << std::endl;
        }
        checkNoRequired(properties, COMMON_JVM_OPTIONS, javaOptions, "");
        if (isBlank(javaOptions)) {
            if (enableDebug) {
                std::cout << "[DEBUG] " << COMMON_JVM_OPTIONS << " not found." << std::endl;
            }
            std::cout << "[Warn] JVM options is empty, you can set it by environment variable JAVA_OPTS or "
                      << COMMON_JVM_OPTIONS << std::endl;
        }
    } else {
        std::string javaOptionsConf = convent2string(properties->get(COMMON_JVM_OPTIONS));
        if (!isBlank(javaOptionsConf)) {
            javaOptions.append(" ").append(javaOptionsConf);
        } else {
            if (enableDebug) {
                std::cout << "[DEBUG] " << COMMON_JVM_OPTIONS << " not found." << std::endl;
            }
        }
    }
    printKeyValue("JAVA_OPTS", javaOptions);

    return true;
}

/**
 * 检查命令行参数
 *
 * @param argc 参数数量
 * @param argv 参数列表
 * @return 是否符合要求
 */
bool checkArguments(int argc, char *argv[], Properties *properties) {
    // key prefix
    std::string regionName = argv[1];
    if (enableDebug) {
        std::cout << "[DEBUG] region: " << regionName << std::endl;
    }
    // appframe.war
    std::string warKey = regionName + APPFRAME_WAR_LOCATION;
    warFile = convent2string(properties->get(warKey.c_str()));
    if (isBlank(warFile)) {
        std::cout << "[ERROR] .war file path is empty, it can be set by " << warKey << std::endl;
        return false;
    }
    if (!fileExist(warFile)) {
        std::cout << "[ERROR] file or directory not exist["
                  << warFile << "], please check " << warKey << std::endl;
        return false;
    }
    if (isDirectory(warFile)) {
        std::cout << "[ERROR] it's a directory: " << warFile << std::endl;
        return false;
    }

    printKeyValue("APPFRAME-WEB", warFile);

    // BossSoft Home
    std::string bsHomeKey = regionName + APPFRAME_BSHOME_LOCATION;
    bsHomeDirectory = convent2string(properties->get(bsHomeKey.c_str()));
    if (isBlank(bsHomeDirectory)) {
        std::cout << "[ERROR] BOSSSOFT_HOME not found, it can be set by " << bsHomeKey << "." << std::endl;
        return false;
    }
    if (!fileExist(bsHomeDirectory)) {
        std::cout << "[ERROR] file or directory not exist["
                  << bsHomeDirectory << "], please check " << bsHomeKey << std::endl;
        return false;
    }
    if (!isDirectory(bsHomeDirectory)) {
        std::cout << "[ERROR] it's not a directory: " << bsHomeDirectory << std::endl;
        return false;
    }

    printKeyValue("BOSSSOFT_HOME", bsHomeDirectory);

    // shutdown port
    checkNoRequired(properties, (regionName + APPDRAME_SHUTDOWN_PORT).c_str(), tomcatShutdownPort, "8005");

    // http port
    checkNoRequired(properties, (regionName + APPFRAME_HTTP_PORT).c_str(), tomcatHttpPort, "8080");

    // https port
    checkNoRequired(properties, (regionName + APPFRAME_HTTPS_PORT).c_str(), tomcatHttpsPort, "");

    // AJP port
    checkNoRequired(properties, (regionName + APPFRAME_AJP_PORT).c_str(), tomcatAjpPort, "");

    // JMX port
    checkNoRequired(properties, (regionName + APPFRAME_JMX_PORT).c_str(), tomcatJmxPort, "");
    return true;
}

bool generateVirtualTomcat(const std::string &region) {
    targetDirectory = programDirectory + region + "_appframe";
    printKeyValue("CATALINA_BASE", targetDirectory);

    // check targetDirectory
    if (!checkDirectory(targetDirectory)) {
        return false;
    }

    // check targetDirectory/conf
#if defined(WINDOWS)
    std::string targetConf = targetDirectory + "\\conf\\";
#elif defined(UNIX) || defined(LINUX)
    std::string targetConf = targetDirectory + "/conf/";
#endif
    if (!checkDirectory(targetConf)) {
        return false;
    }

    // check targetDirectory/logs
#if defined(WINDOWS)
    std::string targetLogs = targetDirectory + "\\logs\\";
#elif defined(UNIX) || defined(LINUX)
    std::string targetLogs = targetDirectory + "/logs/";
#endif
    if (!checkDirectory(targetLogs)) {
        return false;
    }

    // check targetDirectory/work
#if defined(WINDOWS)
    std::string targetWork = targetDirectory + "\\work\\";
#elif defined(UNIX) || defined(LINUX)
    std::string targetWork = targetDirectory + "/work/";
#endif
    if (!checkDirectory(targetWork)) {
        return false;
    }

    // check targetDirectory/webapps
#if defined(WINDOWS)
    std::string targetWebapps = targetDirectory + "\\webapps\\";
#elif defined(UNIX) || defined(LINUX)
    std::string targetWebapps = targetDirectory + "/webapps/";
#endif
    if (!checkDirectory(targetWebapps)) {
        return false;
    }

    // 确认/conf下的配置文件
#if defined(WINDOWS)
    std::string tomcatConf = tomcatLocation + "\\conf\\";
#elif defined(UNIX) || defined(LINUX)
    std::string tomcatConf = tomcatLocation + "/webapps/";
#endif
    for (auto &confFileName : CONF_COPY_FILE) {
        std::string sourceConfFile = tomcatConf + confFileName;
        std::string targetConfFile = targetConf + confFileName;

        if (enableDebug) {
            std::cout << "[DEBUG] check file: " << targetConfFile << std::endl;
        }
        // 文件不存在或文件不一致
        if (!fileExist(targetConfFile)) {
            if (enableDebug) {
                std::cout << "[DEBUG] file not exist: " << targetConfFile << std::endl;
            }
            // 复制文件
            if (!copyFile(sourceConfFile, targetConfFile)) {
                std::cout << "[ERROR] copy file failed: " << confFileName;
                return false;
            }
        } else if (!sameFile(sourceConfFile, targetConfFile)) {
            if (enableDebug) {
                std::cout << "[DEBUG] file was changed: " << sourceConfFile << std::endl;
            }
            // 复制文件
            if (!copyFile(sourceConfFile, targetConfFile)) {
                std::cout << "[ERROR] copy file failed: " << confFileName;
                return false;
            }
        }
    }

    // 检查war包
    char aMd5[MD5_STRING_SIZE + 1];
    char bMd5[MD5_STRING_SIZE + 1];
    std::string targetWarPath = targetWebapps + "appframe.war";
    // war包不存在
    if (!fileExist(targetWarPath)) {
        if (enableDebug) {
            std::cout << "[DEBUG] appframe package not exist: " << targetWebapps << std::endl;
        }
        if (!copyFile(warFile, targetWebapps + "appframe.war")) {
            std::cout << "[ERROR] copy appframe package failed: " << warFile;
            return false;
        }
    }
        // war包存在但不相同
    else if (!sameFile(warFile, targetWarPath, aMd5, bMd5)) {
        std::cout << "[INFO ] appframe package was changed: " << warFile << std::endl;
        printKeyValue("\tSource War MD5", aMd5);
        printKeyValue("\tTarget War MD5", bMd5);
        if (!copyFile(warFile, targetWebapps + "appframe.war")) {
            std::cout << "[ERROR] copy appframe package failed: " << warFile;
            return false;
        }
    }

    // create server.xml
    std::string targetServerXmlPath = targetDirectory + TOMCAT_SERVER_XML;
    if (enableDebug) {
        std::cout << "[DEBUG] create server.xml: " << targetServerXmlPath << std::endl;
    }
    std::string serverXml = generateServerXml(targetWebapps, tomcatShutdownPort, tomcatHttpPort);
    FILE *serverXmlFile = fopen(targetServerXmlPath.c_str(), "wb");
    if (serverXmlFile == nullptr) {
        std::cout << "[ERROR] create server.xml failed." << std::endl;
    }
    fwrite(serverXml.c_str(), 1, serverXml.length(), serverXmlFile);
    fclose(serverXmlFile);
    return true;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cout << "[ERROR] please enter the region of appframe." << std::endl;
        return 1;
    }

    if (argc > 2) {
        std::cout << "[ERROR] too much arguments." << std::endl;
        return 1;
    }

    char *cwdDir = new char[MAX_PATH_LENGTH];

#if defined(WINDOWS)
    GetModuleFileNameA(nullptr, cwdDir, MAX_PATH_LENGTH);

    // remove *.exe
    size_t len = strlen(cwdDir);
    for (size_t i = len - 1; i > 0; i--) {
        if (cwdDir[i] == '\\') {
            cwdDir[i] = '\0';
            break;
        }
    }
#elif defined(UNIX) || defined(LINUX)
    getcwd(cwdDir, MAX_PATH_LENGTH);
#endif
    programDirectory = std::string(cwdDir);
    delete[] cwdDir;
    printKeyValue("PROGRAM_HOME", programDirectory);
#if defined(WINDOWS)
    programDirectory.append("\\");
#elif defined(UNIX) || defined(LINUX)
    programDirectory.append("/");
#endif
    std::string configFilePath = programDirectory + CONFIG_FILE;
    printKeyValue("CONFIG_FILE", configFilePath);
    Properties properties(configFilePath.c_str());

    if (!properties.isInitSuccess()) {
        return 1;
    }

    if (!checkCommonConfiguration(&properties)) {
        return 2;
    }

    if (!checkArguments(argc, argv, &properties)) {
        return 3;
    }

    std::string regionName = argv[1];
    if (!generateVirtualTomcat(regionName)) {
        return 4;
    }

    // run virtual tomcat
    std::string command = generateCommand(javaHome, tomcatLocation, targetDirectory, javaOptions, bsHomeDirectory);
    std::cout << "[INFO ] COMMAND: " << command << std::endl << std::endl
              << "=========== VIRTUAL TOMCAT ===========" << std::endl;
    system(command.c_str());
    return 0;
}
