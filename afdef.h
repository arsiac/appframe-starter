//
// Created by arsia on 2021/6/24.
//

#ifndef APPFRAME_STARTER_AFDEF_H
#define APPFRAME_STARTER_AFDEF_H

#define WINDOWS

const char *CONFIG_FILE = "appframe-starter.conf";

#if defined(WINDOWS)
const char *TOMCAT_CATALINA = "\\bin\\catalina.bat";
#elif defined(UNIX) || defined(LINUX)
const char *TOMCAT_CATALINA = "/bin/catalina.sh";
#endif

#if defined(WINDOWS)
const char *TOMCAT_SERVER_XML = "\\conf\\server.xml";
#elif defined(UNIX) || defined(LINUX)
const char *TOMCAT_SERVER_XML = "/conf/server.xml";
#endif

// default: JAVA_HOME
const char *COMMON_JAVA_HOME = "common.java.home";

// default: ""
const char *COMMON_JVM_OPTIONS = "common.java.options";

// default: CATALINA_HOME
const char *COMMON_TOMCAT_LOCATION = "common.tomcat.location";

// required
const char *APPFRAME_WAR_LOCATION = ".war.location";

// required
const char *APPFRAME_BSHOME_LOCATION = ".bshome.location";

// default: 8005
const char *APPDRAME_SHUTDOWN_PORT = ".shutdown.port";

// default: 8080
const char *APPFRAME_HTTP_PORT = ".http.port";

// default: ""
const char *APPFRAME_HTTPS_PORT = ".https.port";

// default: 1099
const char *APPFRAME_JMX_PORT = ".jmx.port";

// default: ""
const char *APPFRAME_AJP_PORT = ".ajp.port";


const int CONF_COPY_FILE_NUMBER = 9;
const char *CONF_COPY_FILE[] = {
        "catalina.policy",
        "catalina.properties",
        "context.xml",
        "jaspic-providers.xml",
        "jaspic-providers.xsd",
        "logging.properties",
        "tomcat-users.xml",
        "tomcat-users.xsd",
        "web.xml"
};

#endif //APPFRAME_STARTER_AFDEF_H
