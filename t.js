const fs = require('fs')
const path = require('path')
const dgram = require('dgram');

const client = dgram.createSocket('udp4');

client.on('close', () => {
  console.log('socket已关闭');
});

client.on('error', (err) => {
  console.log(err);
});

client.on('message', (msg, rinfo) => {
  console.log(`接收到来自：${rinfo.address}:${rinfo.port} 的消息： ${msg}`);
});
// 150.158.27.240
client.connect(8004, '127.0.0.1', () => {
  var jpg1 = null
  var jpg2 = null
  var jpg3 = null
  fs.readFile(path.resolve(__dirname, '1.jpg'), (err, data) => {
    if (err) {
      console.log(err)
      return
    }
    jpg1 = data
  })
  fs.readFile(path.resolve(__dirname, '2.jpg'), (err, data) => {
    if (err) {
      console.log(err)
      return
    }
    jpg2 = data
  })
  fs.readFile(path.resolve(__dirname, '3.jpg'), (err, data) => {
    if (err) {
      console.log(err)
      return
    }
    jpg3 = data
  })
  setInterval(() => {
    setTimeout(() => {
      client.send(jpg1, (err) => {
        if (err) console.log(err)
      })
    }, 0)
    setTimeout(() => {
      client.send(jpg2, (err) => {
        if (err) console.log(err)
      })
    }, 100)
    setTimeout(() => {
      client.send(jpg3, (err) => {
        if (err) console.log(err)
      })
    }, 200)
  }, 300)
})