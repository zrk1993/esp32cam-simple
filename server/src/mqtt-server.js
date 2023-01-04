var aedes = require('aedes')()
var server = require('net').createServer(aedes.handle)
var port = 8007

var pub = require('./channel-pub-sub')

server.listen(port, function () {
  console.log('mqtt server started and listening on port ', port)
})

// 身份验证
aedes.authenticate = function (client, username, password, callback) {
  callback(null, true);
}

// 客户端连接
aedes.on('client', function (client) {
  console.log('Client Connected: \x1b[33m' + (client ? client.id : client) + '\x1b[0m', 'to broker', aedes.id);
});

// 客户端断开
aedes.on('clientDisconnect', function (client) {
  console.log('Client Disconnected: \x1b[31m' + (client ? client.id : client) + '\x1b[0m', 'to broker', aedes.id);
});

aedes.on('subscribe', function (subscriptions, client) {
  console.log('subscriptions');
  console.log(subscriptions)
});

aedes.on('publish', function (packet, client) {
  console.log('publish');
  console.log(packet)
  if (packet.topic == 'jpeg') {
    pub.publish(client.id, packet.payload)
  }
});

aedes.send = function (id, topic, msg, cb) {
  var client = aedes.clients[id]
  if (client) {
    client.publish({
      cmd: 'publish',
      qos: 2,
      topic,
      payload: Buffer.from(msg)
    }, function (error) {
      if (cb) {
        cb(error)
      }
    })
  }
}

module.exports = aedes
