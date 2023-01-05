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
  mqttServer.send(req.query.id, 'esp32cam', 'inr 1000')
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
    res.json({
      ok: !e
    })
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

app.listen(port, () => {
  console.log(`express app listening on port ${port}`)
})
