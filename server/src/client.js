const dgram = require("dgram");
 
const client = dgram.createSocket("udp4");
 
client.on("message",(msg,rinfo)=>{
    console.log("接收来自："+rinfo.address+":"+rinfo.port+"的消息："+msg.toString());
});
 
client.on("error",(err)=>{
  console.error("客户端错误："+err.message);
});
 
client.on("close",()=>{
    console.log("socket已关闭");
});
 
client.send("我是UDP客户端！",8004,"150.158.27.240",(err)=>{
    if(err) client.close();
});