var mjpegServer = require('mjpeg-server');
var pub = require('./channel-pub-sub')

var mqttServer = require('./mqtt-server')

const express = require('express')
const app = express()
const port = 8006

app.get('/', (req, res) => {
  res.send('Hello World!')
})

app.get('/video', (req, res) => {
  var mjpegReqHandler = mjpegServer.createReqHandler(req, res);
  pub.subscribe(req.query.id, function (msg, rinfo) {
    console.log(msg.length)
    console.log(res.writable)
    mjpegReqHandler.write(msg, function (error) {
      if (error) {
        console.error(error)
      }
    });
  })
})

app.get('/capture', (req, res) => {
  mqttServer.send(req.query.id, 'esp32cam', 'capture', function (e) {
    if (e) {
      res.json({
        ok: false,
        msg: e.message
      })
    } else {
      res.json({
        ok: true
      })
    }
  })
})

app.get('/jpeg', (req, res) => {
  var id = req.query.id
  var jpeg = mqttServer.jpegMap[id] || Buffer.from("error")
  res.writeHead(200,{
    'Content-Type': 'image/jpeg',
    'Content-Length': jpeg.length
  })
  res.end(jpeg)
})

app.get('/mqtt/publish', (req, res) => {
  var topic = req.query.topic
  var msg = req.query.msg
  mqttServer.send(req.query.id, 'esp32cam', msg, function (e) {
    res.json({
      ok: !e
    })
  })
})

app.listen(port, () => {
  console.log(`express app listening on port ${port}`)
})
