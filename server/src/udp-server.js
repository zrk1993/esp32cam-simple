var dgram = require('dgram');

var udpServer = dgram.createSocket("udp4");

var pub = require('./channel-pub-sub')

var host = '10.0.16.3'
var port = 8005

udpServer.on("message", (msg, rinfo) => {
  console.log("服务器接收到来自" + rinfo.address + ":" + rinfo.port + " 的消息：" + msg.length);
  pub.publish(rinfo.port, msg, rinfo)
});

udpServer.on("listening", () => {
  var adress = udpServer.address();
  console.log("UDP服务器监听：", host + ":" + port);
});

udpServer.on("error", (err) => {
  console.error("服务器异常错误：" + err.message);
});

udpServer.bind(port, host);

module.exports = udpServer
