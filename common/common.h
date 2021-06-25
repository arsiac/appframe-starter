//
// Created by arsia on 2021/6/24.
//

#ifndef APPFRAME_STARTER_COMMON_H
#define APPFRAME_STARTER_COMMON_H

#include <cctype>
#include <string>
#include <iostream>
#include <cstdio>
#include <sys/stat.h>
#include <direct.h>

/**
 * 是否为空白
 *
 * @param str 字符串
 * @return 是否为空白
 */
bool isBlank(const std::string &str) {
    if (str.empty()) {
        return true;
    }

    const char *tempStr = str.c_str();
    for (int i = 0; i < str.length(); i++) {
        if (!isspace(tempStr[i])) {
            return false;
        }
    }
    return true;
}

/**
 * char* 转化为 std::string, 检查 nullptr
 *
 * @param value char 数组
 * @return std::string 字符串
 */
std::string convent2string(const char *value) {
    return value == nullptr ? "" : value;
}

/**
 * 打印键值对
 *
 * @param key 键
 * @param value 值
 * @param endLine 是否换行
 */
void printKeyValue(const char *key, const std::string &value, bool endLine = true) {
    std::cout << key << ": ";

    if (value.empty()) {
        std::cout << "\"\"";
    } else {
        std::cout << value;
    }

    if (endLine) {
        std::cout << std::endl;
    } else {
        std::cout << "; ";
    }
}

/**
 * 文件是否存在
 *
 * @param path 路径
 * @return 是否存在
 */
bool fileExist(const std::string &path) {
    struct stat status{};
    return 0 == stat(path.c_str(), &status);
}

/**
 * 是否是文件夹
 *
 * @param path 路径
 * @return 是否是文件夹
 */
bool isDirectory(const std::string &path) {
    struct stat status{};
    if (0 != stat(path.c_str(), &status)) {
        return false;
    }

    return S_ISDIR(status.st_mode);
}

/**
 * 确认文件夹是否存在，不存在则创建
 *
 * @param path 文件夹路径
 * @return 文件夹不存在且创建失败
 */
bool checkDirectory(const std::string &path) {
    if (!fileExist(path) && -1 == mkdir(path.c_str())) {
        std::cout << "[Error] create folder failed: " << path << std::endl;
        return false;
    }
    return true;
}

/**
 * 获取文件大小
 *
 * @param file 文件
 * @return 文件大小
 */
size_t fileSize(FILE *file) {
    if (file == nullptr) {
        return -1;
    }

    long locationCache;
    size_t size;

    locationCache = ftell(file);
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, locationCache, SEEK_SET);
    return size;
}

/**
 * 复制文件
 *
 * @param src 源文件
 * @param dest 目标文件
 * @return 是否成功
 */
bool copyFile(const std::string &src, const std::string &dest) {

    FILE *srcFile = fopen(src.c_str(), "rb");
    if (srcFile == nullptr) {
        std::cout << "[Error] file not exist: " << src << std::endl;
        return false;
    }
    FILE *destFile = fopen(dest.c_str(), "wb");
    if (destFile == nullptr) {
        std::cout << "[Error] create file failed: " << dest << std::endl;
        return false;
    }

    size_t size = fileSize(srcFile);
    char *buffer = new char[size + 1];

    fread(buffer, 1, size, srcFile);
    fwrite(buffer, 1, size, destFile);

    fclose(srcFile);
    fclose(destFile);
    return true;
}

/**
 * 生成 server.xml
 *
 * @param catlinaHome tomcat位置
 * @param shutdownPort 关闭端口
 * @param httpPort http端口
 * @return
 */
std::string generateServerXml(const std::string &catalinaHome,
                              const std::string &shutdownPort,
                              const std::string &httpPort) {
    return "<Server port=\"" + shutdownPort + "\" shutdown=\"SHUTDOWN\">\n" +
           "  <Listener className=\"org.apache.catalina.startup.VersionLoggerListener\" />\n" +
           "  <Listener className=\"org.apache.catalina.core.AprLifecycleListener\" SSLEngine=\"on\" />\n" +
           "  <Listener className=\"org.apache.catalina.core.JreMemoryLeakPreventionListener\" />\n" +
           "  <Listener className=\"org.apache.catalina.mbeans.GlobalResourcesLifecycleListener\" />\n" +
           "  <Listener className=\"org.apache.catalina.core.ThreadLocalLeakPreventionListener\" />\n" +
           "  <GlobalNamingResources>\n" +
           "    <Resource name=\"UserDatabase\" auth=\"Container\" \n" +
           "      type=\"org.apache.catalina.UserDatabase\" \n" +
           "      description=\"User database that can be updated and saved\" \n" +
           "      factory=\"org.apache.catalina.users.MemoryUserDatabaseFactory\" \n" +
           "      pathname=\"conf/tomcat-users.xml\" />\n" +
           "  </GlobalNamingResources>\n" +
           "  <Service name=\"Catalina\">\n" +
           "    <Connector port=\"" + httpPort +
           "\" protocol=\"HTTP/1.1\" connectionTimeout=\"20000\" redirectPort=\"8443\" />\n" +
           "    <Engine name=\"Catalina\" defaultHost=\"localhost\">\n" +
           "      <Realm className=\"org.apache.catalina.realm.LockOutRealm\">\n" +
           "        <Realm className=\"org.apache.catalina.realm.UserDatabaseRealm\" resourceName=\"UserDatabase\" />\n" +
           "      </Realm>\n" +
           R"(      <Host name="localhost" appBase=")" + catalinaHome + "\" \n" +
           "        unpackWARs=\"true\" autoDeploy=\"true\" deployOnStartup=\"false\" deployIgnore=\"^(?!(manager)|(tomee)$).*\">\n" +
           "        <Valve className=\"org.apache.catalina.valves.AccessLogValve\" \n" +
           "          directory=\"logs\" prefix=\"localhost_access_log\" \n" +
           "          suffix=\".txt\" pattern=\"%h %l %u %t &quot;%r&quot; %s %b\" />\n" +
           R"(         <Context path="/appframe-web" docBase=")" + catalinaHome +
           "/appframe.war\" reloadable=\"true\"/>\n" +
           "      </Host>\n" +
           "    </Engine>\n" +
           "  </Service>\n" +
           "</Server>";
}

std::string generateCommand(const std::string &javaHome,
                            const std::string &catalinaHome,
                            const std::string &catalinaBase,
                            const std::string &javaOpts,
                            const std::string &bsHome) {
#if defined(WINDOWS)
    std::string cmd = "set JAVA_HOME=" + javaHome +
                      "&set CATALINA_HOME=" + catalinaHome +
                      "&set CATALINA_BASE=" + catalinaBase +
                      "&set JAVA_OPTS=";
    if (!isBlank(javaOpts)) {
        cmd.append(javaOpts).append(" ");
    }
    cmd.append("-DBOSSSOFT_HOME=").append(bsHome)
            .append("&").append(catalinaHome).append(TOMCAT_CATALINA).append(" run");
#elif defined(UNIX) || defined(LINUX)
    std::string cmd = "JAVA_HOME=" + javaHome +
                      "&CATALINA_HOME=" + catalinaHome +
                      "&CATALINA_BASE=" + catalinaBase +
                      "&JAVA_OPTS=";
    if (!isBlank(javaOpts)) {
        cmd.append(javaOpts).append(" ");
    }
    cmd.append("-DBOSSSOFT_HOME=").append(bsHome)
            .append("&").append(catalinaHome).append(TOMCAT_CATALINA).append(" run");
#endif
    return cmd;
}

#endif //APPFRAME_STARTER_COMMON_H
