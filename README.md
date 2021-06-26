# AppFrame-Starter

### Usage

```shell
appframe-starter [region]
```

### Configuration

- location: ./appframe-starter.conf

```shell
#default: true
common.debug.enable=true
# default: JAVA_HOME
common.java.home=/path/to/java

# default: ""
common.java.options=-Xmx2048 -Dfile.encoding=UTF-8

# required
common.tomcat.location=/path/to/tomcat


[region].war.location=/path/to/war

[region].bshome.location=/path/to/BOSSSOFT_HOME

# default: 8080
[region].http.port=0

# default: 8005
[region].shutdown.port=0
```