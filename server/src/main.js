var http = require('http');
var dgram = require('dgram');

var udpServer = dgram.createSocket("udp4");
var mjpegServer = require('mjpeg-server');

var mjpegReqHandler = null;

var httpServer = http.createServer(function (req, res) {
  mjpegReqHandler = mjpegServer.createReqHandler(req, res);
})

httpServer.listen(8006);

udpServer.on("message", (msg, rinfo) => {
  //将接收到的消息返回客户端
  var strmsg = "你好，UDP客户端，消息已经收到！";
  // udpServer.send(strmsg, rinfo.port, rinfo.address);
  console.log("服务器接收到来自" + rinfo.address + ":" + rinfo.port + " 的消息：" + msg.toString());
});

udpServer.on("listening", () => {
  var adress = udpServer.address();
  console.log("服务器监听：", adress.adress + ":" + adress.port);
});

udpServer.on("error", (err) => {
  console.error("服务器异常错误：" + err.message);
});

udpServer.bind(8005, '150.158.27.240');
