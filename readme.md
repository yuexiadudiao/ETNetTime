#ET外星人自动校时

##1.校时原理

调研了三种校时原理

###方法一：NTP协议校时

安装NTP协议的服务器就是NTP服务器，功能是提供UTC时间服务，比如ntp1.aliyun.com就是一个NTP服务器。NTP协议使用udp的123端口提供服务，返回NTP报文，解析NTP报文即可获取时间。

相关资料：
1.	http://blog.csdn.net/loongee/article/details/24271129
2.	http://www.zhimengzhe.com/bianchengjiaocheng/cbiancheng/336723.html

###方法二：TP协议校时

RFC868 Time Protocol协议，使用tcp或udp的37端口，返回32bit的时间UTC1900时间。

相关资料：
1.	http://blog.sina.com.cn/s/blog_6e48f3460100mgiv.html

###方法三：http协议校时

http响应报文的Date字段就有时间，向知名的网站请求即可获取到时间。红叶自动校时v3.61.exe就是采用这种校时方法，从监听的网络连接中可以看出程序访问了知名网站的http服务。

##2.功能设计

1，查询校时
2，添加删除服务器(配置文件)
3，开机启动
4，最小化启动
5，启动n秒后自动校时
6，校时成功后自动退出

##3.功能的跨平台

使用qt提供的宏来判断平台

```
#if defined(Q_OS_WIN32)
  ;//win平台代码或平台头文件
#elif defined(Q_OS_LINUX)
  ;//win平台代码或平台头文件
#endif
```

##4.界面设计

##5.一些疑问

###UTC时间到底是1900年为基准开始1970年？

###UTC与GMT时间区别？

UTC比GMT更精确，但是对我们普通的使用者来说基本没有区别。
