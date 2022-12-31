# SimpleVideoServer

javac -cp src  src/org/btik/server/video/UDP2Main.java -d target -encoding utf-8
java -classpath target  org.btik.server.video.UDP2Main

camServer 的demo使用esp32cam自己提供http服务和视频流，会有较大性能开销。而且需要内网穿透才能外网访问。本项目提供服务端合成视频流的方式，esp32cam只需要不停拍照发送给服务端，服务端处理浏览器查看视频的请求，合成视频流给浏览器。
我们可以把服务端程序部署在公网，可以通过这样的方式远程访问。

```mermaid
graph LR
e(esp32cam)
S(SimpleVideoServer)
e -->|帧|S
B(浏览器)
S -->|mjpeg视频流|B
```

本项目提供了，服务端代码与esp32cam的代码。

`simpleVideoRecorder`目录下载为esp32cam的代码。

_如果支持该项目，请点star ⭐ 而不是 关注通知,后者只会让你收到关于该项目的动态的邮件通知。_

## 运行服务端

根据自己需要选择以下任意方式运行服务端

* 下载 [windows服务端发行版](https://gitcode.net/qq_26700087/simpleVideoServer/uploads/bcafcb53130a277f3b6f92f8c83249a7/win_release.zip)

  解压之后，进入对应目录点击run.bat文件启动服务器。

* 下载[linux服务端发行版](https://gitcode.net/qq_26700087/simpleVideoServer/uploads/83402dcffb269aac326a2f5649ef6970/linux_release.tar.gz)

  需要unzip或p7z等可以解压zip的应用

  运行

  ```shell
  tar -zxvf linux_release.tar.gz
  cd linux_release/
  sh run.sh
  ```

* 发行版内部仅仅是一些java17 版本的class文件和jre以及启动脚本，你也可以使用任意其它的jre17运行这些class文件。并非需要发行版。

  > 比如你希望在macos运行当前服务端时，将macos的jdk17或者jre17复制到linux发行版的目录中，替代其jre目录，参考run.sh中命令调用jre下java运行服务端。

* [全部发行版](https://gitcode.net/qq_26700087/simpleVideoServer/-/releases?spm=1033.2243.3001.5877)

* 访问视频服务

  每个摄像头处在不同频道，访问不同的摄像头需要不同的地址。

  但这个版本没有测试多频道，只能说理论上没有问题。

  如鄙人执行的服务端日志打印含有esp32Cam接入的后的相关打印如下：

  ```
  D:\Users\immor\idea\SimpleVideoServer\out\win_release>.\jre\bin\java -classpath SimpleVideoServer org.btik.server.video.Main
  bio video server started
  bio Device Channel started
  new channel:
  http://127.0.0.1:8003/video/441793EE3C08
  http://192.168.0.116:8003/video/441793EE3C08
  http://192.168.137.1:8003/video/441793EE3C08
  start /192.168.137.234:53051
  
  
  ```

  每接入一个esp32Cam会新建一个频道，在`new channel:`的打印后会出现，相关可以访问视频流的地址。

  你可以在本机，或者局域网的其他设备访问。

  部署在云服务器的同学把端口打开后，把内网ip替换成公网ip，或者域名即可。

  

* 关于如何把视频界面嵌入其它网页

  如果你擅长web开发，或者不喜欢在多个窗口查看多个摄像头可以参考以下方法增加自己的内容。

  

  本视频流是允许跨域的，若希望在自己的网页里面加入本服务端提供esp32Cam视频窗口，

  其实不用html的 `iframe`标签，`img`标签即可。

  比如以下html代码，新建一个文件比如`a.html` 复制以下内容，根据实际情况，替换`img`标签`src`属性的内容。

  ```html
  <!DOCTYPE html>
  <html lang="en">
  <head>
      <meta charset="UTF-8">
      <title>直播间</title>
      <style>
          .videoContainer{
              display: inline-block;
          }
      </style>
  </head>
  <body>
  <div style="padding: 0;margin:30px auto; width: 1300px">
      <div class="videoContainer">
          <img src="http://127.0.0.1:8003/video/441793EE3C08">
      </div>
      <div class="videoContainer">
          <img src="http://127.0.0.1:8003/video/58BF2581F024">
      </div>
  </div>
  
  </body>
  </html>
  ```

  以上是我的两个esp32Cam的视野，效果如下：

  ![esp32Cam doublevideo](https://img-blog.csdnimg.cn/ca9e5e19fa5742b8a7c698845078e0ab.gif#pic_center)

  也就是说，你可以把该项目植入任何其它可以用到web前端的项目。通过查询在线设备，可以动态打开每个摄像头的视频。

  

## 配置项

`light-video.properties`

里面含有三个配置

```properties
http.port=8003
http.clients.limit=10
stream.port=8004
```

**http.port** 为http的端口。

**http.clients.limit** 摄像头在线接入限制数。本意是想限制客户端参与的数量故名为http.clients.limit，实际的实现是限制了摄像头的数量

**stream.port** esp32cam像服务端发送照片的端口，在本项目中使用该默认端口，如果需要修改，一并修改esp32cam代码中连接的端口。一般无需修改。



## 注意事项

* <span style="color:#f00">本代码未考虑安全场景，无鉴权，也无加密。无论是抓包还是发起访问都是毫无阻挡。若需要在公网使用，请自行设计安全机制。</span>

* **src目录为服务端java源代码**

  没有其他需要可以下载上面的发型版本，已包含64位jre17，也可以使用自定义jre17运行。

  代码理论上java8也可以编译，若有需要可以自行编译。

  

* **simpleVideoRecorder目录为esp32cam的代码**

  使用platformio基于ardunio框架开发。

  使用platformio开发的同学请按实际情况修改simpleVideoRecorder 目录下的platformio.ini的串口

  

  使用ardunio IDE开发的请注意 选择开发板 为esp32cam，需要安装esp32环境，为了保险起见，你可以把源码中`main.cpp`重命名为`main.ino`

  

* 如果在局域网也可以使用PC作为服务端，查看充当服务端的PC的ip。

  打开命令行，执行ipconfig。找到wifi所在网络的网卡ip.

  

* 修改main.cpp中的wifi的ssid和密码

  
  
* 可以使用linux的同学,自行根据实际组网查看和配置网络

  
  
  
