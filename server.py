#!/usr/bin/env python
from datetime import datetime
import os, csv, cv2, sys, base64
import faceHunter
from flask import Flask, render_template, Response, Request, jsonify, send_from_directory   
import numpy as np

def base64_To_Numpy(jpg_as_text):
	encoded_data = jpg_as_text
	nparr = np.fromstring(encoded_data.decode('base64'), np.uint8)
	img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

	return img

faceHunter.loadDependencies()

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')

def gen():
    i=1
    while i<10:
        yield (b'--frame\r\n'
            b'Content-Type: text/plain\r\n\r\n'+str(i)+b'\r\n')
        i+=1

def get_frame(camera_id):
    camera_port=camera_id

    if(camera_port.isdigit()):
        camera_port = int(camera_port)

    ramp_frames=100

    #test with ip camera "http://150.165.80.62:8080/videostream.cgi?user=admin&pwd=senha123"
    camera=cv2.VideoCapture(camera_port)
	
    i=1
    while True:
        retVal, img = camera.read()
        retVal, buf = cv2.imencode('.jpg', img)
        
        jpg_as_text = base64.b64encode(buf)
        jpg_as_text = faceHunter.faceHunterAction(jpg_as_text)	
        jpg_original = base64_To_Numpy(jpg_as_text)
        
        imgencode=cv2.imencode('.jpg',jpg_original)[1]
        stringData=imgencode.tostring()

        yield (b'--frame\r\n'
            b'Content-Type: text/plain\r\n\r\n'+stringData+b'\r\n')
        i+=1
    
    del(camera)

@app.route('/calc/<path:camera_id>')
def calc(camera_id):
    camera_id = str(camera_id)
    camera_id = camera_id.replace("$%$", "?")
    return Response(get_frame(camera_id),mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/maleCounter')
def maleCounter():
    result = faceHunter.getGenderMascCounter()
    return str(result)

@app.route('/femaleCounter')
def femaleCounter():
    result = faceHunter.getGenderFemCounter()
    return str(result)

@app.route('/smileCounter')
def smileCounter():
    result = faceHunter.getSmileCounter()
    return str(result)

@app.route('/notSmileCounter')
def notSmileCounter():
    result = faceHunter.getNotSmileCounter()
    return str(result)

@app.route('/getDate')
def getDate():
    now = datetime.now()
    dt_string = now.strftime("%B %d, %Y")
    return dt_string

@app.route('/getHour')
def getHour():
    hour = datetime.now()
    dt_string = hour.strftime("%H:%M:%S")
    return dt_string

app.jinja_env.globals.update(maleCounter=maleCounter)
app.jinja_env.globals.update(femaleCounter=femaleCounter)
app.jinja_env.globals.update(smileCounter=smileCounter)
app.jinja_env.globals.update(notSmileCounter=notSmileCounter)
app.jinja_env.globals.update(getDate=getDate)
app.jinja_env.globals.update(getHour=getHour)

male24Hours = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
female24Hours = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
smile24Hours = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
notSmile24Hours = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

male1Hour = [0, 0, 0, 0, 0, 0]
female1Hour = [0, 0, 0, 0, 0, 0]
smile1Hour = [0, 0, 0, 0, 0, 0]
notSmile1Hour = [0, 0, 0, 0, 0, 0]

male10Minutes = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
female10Minutes = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
smile10Minutes = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
notSmile10Minutes = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

def getMinute():
    currentMinute = datetime.now()
    return currentMinute.minute

def getHour():
    currentHour = datetime.now()
    return currentHour.hour

def getIndex10Minutes(minute):
    if minute < 10:
        return 0
    elif minute < 20:
        return 1
    elif minute < 30:
        return 2
    elif minute < 40:
        return 3
    elif minute < 50:
        return 4
    elif minute < 60:
        return 5

def getIndex1Minute(minute):
    if minute in [0, 10, 20, 30, 40, 50]:
        return 0
    elif minute in [1, 12, 21, 31, 41, 51]:
        return 1
    elif minute in [2, 12, 22, 32, 42, 52]:
        return 2
    elif minute in [3, 13, 23, 33, 43, 5]:
        return 3
    elif minute in [4, 14, 24, 34, 44, 54]:
        return 4
    elif minute in [5, 15, 25, 35, 45, 55]:
        return 5
    elif minute in [6, 16, 26, 36, 46, 56]:
        return 6
    elif minute in [7, 17, 27, 37, 47, 57]:
        return 7
    elif minute in [8, 18, 28, 38, 48, 58]:
        return 8
    elif minute in [9, 19, 29, 39, 49, 59]:
        return 9
    
def updateList(l, index, value):
    if index -1 < 0:
        l[index] = int(value)
    else:
        new_value = int(value) 
        for i in range(index, 0, -1):
            new_value -= l[i]
        l[index] += new_value

def updateDataByHour(index):
    male = maleCounter()
    updateList(male1Hour, index, male)

    female = femaleCounter()
    updateList(female1Hour, index, female)

    smile = smileCounter()
    updateList(smile1Hour, index, smile)

    notSmile = notSmileCounter()
    updateList(notSmile1Hour, index, notSmile)

hour_started = 0

def isNextDay(current_hour):
    if current_hour > hour_started:
        current_hour = hour
        for i in range(len(male24Hours)):
            male24Hours[i] = 0
            female24Hours[i] = 0
            smile24Hours[i] = 0
            notSmile24Hours[i] = 0
    else:
        hour_started = current_hour

def updateDataByHour24Hours(index):
    male = maleCounter()
    updateList(male24Hours, index, male)

    female = femaleCounter()
    updateList(female24Hours, index, female)

    smile = smileCounter()
    updateList(smile24Hours, index, smile)

    notSmile = notSmileCounter()
    updateList(notSmile24Hours, index, notSmile)

def updateDataBy10Minutes(index):
    male = maleCounter()
    updateList(male10Minutes, index, male)

    female = femaleCounter()
    updateList(female10Minutes, index, female)

    smile = smileCounter()
    updateList(smile10Minutes, index, smile)

    notSmile = notSmileCounter()
    updateList(notSmile10Minutes, index, notSmile)

@app.route('/data24hours')
def data24hours():
    hour = getHour()
    #isNextDay(hour)
    updateDataByHour24Hours(hour)
    return jsonify({'male': male24Hours, 'female': female24Hours, 'smile': smile24Hours, 'notSmile': notSmile24Hours})

@app.route('/data1Hour')
def data1Hour():
    minute = getMinute()
    index = getIndex10Minutes(minute)
    updateDataByHour(index)
    return jsonify({'male': male1Hour, 'female': female1Hour, 'smile': smile1Hour, 'notSmile': notSmile1Hour})

@app.route('/data10Mininutes')
def data10Mininutes():
    minute = getMinute()
    index = getIndex1Minute(minute)
    updateDataBy10Minutes(index)
    return jsonify({'male': male10Minutes, 'female': female10Minutes, 'smile': smile10Minutes, 'notSmile': notSmile10Minutes})

@app.route('/csv/<path:filename>', methods=['GET', 'POST'])
def download(filename): 
    return send_from_directory(directory='csv', filename=filename)

@app.route('/csvFiles')
def csvFiles():
    files = os.listdir('csv')
    return render_template('csvFiles.html', files=files)

def writeCSV(filename):
    wr = csv.writer(open('csv/' + filename + '.csv','w'), delimiter=';')
    row = []
    row.append('male')
    row.append('female')
    row.append('smile')
    row.append('notSmile')
    wr.writerow(row)
    
    for i in range(len(male24Hours)):
        row_data = []
        row_data.append(male24Hours[i])
        row_data.append(female24Hours[i])
        row_data.append(smile24Hours[i])
        row_data.append(notSmile24Hours[i])
        wr.writerow(row_data)

def getDateFormated(date):
    if date < 10:
        new_date = '0' + str(date)
        return new_date
    else:
        return str(date)

@app.route('/updateCSV')
def updateCSV():
    time = datetime.now()
    day = getDateFormated(time.day)
    month = getDateFormated(time.month)
    year = str(time.year)
    filename = year + '_' + month + '_' + day
    writeCSV(filename)

app.jinja_env.globals.update(updateCSV=updateCSV)

if __name__ == '__main__':
    app.run(host='localhost', debug=True, threaded=True)

