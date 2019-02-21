const cv = require('opencv4nodejs');
const path = require('path')
const express = require('express');
const app = express();
const server = require('http').Server(app);
const io = require('socket.io')(server);
const {drawBlueRect} = require('./utils');

const FPS = 20;
const wCap = new cv.VideoCapture(0);
wCap.set(cv.CAP_PROP_FRAME_WIDTH, 300);
wCap.set(cv.CAP_PROP_FRAME_HEIGHT, 300);

app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'index.html'));
});

setInterval(() => {
  const frame = wCap.read();
  const classifier = new cv.CascadeClassifier(cv.HAAR_FRONTALFACE_ALT2);
  
  const { objects, numDetections } = classifier.detectMultiScale(frame.bgrToGray());
  //console.log('faceRects:', objects);
  //console.log('confidences:', numDetections);

  if (!objects.length) {
    //throw new Error('No faces detected!');
  }
  else {
    // draw detection
    const numDetectionsTh = 10;
    objects.forEach((rect, i) => {
      const thickness = numDetections[i] < numDetectionsTh ? 1 : 2;
      drawBlueRect(frame, rect, { thickness });
    });
  }

  const image = cv.imencode('.jpg', frame).toString('base64');
  io.emit('image', image);
}, 1000 / FPS)

server.listen(3000);
