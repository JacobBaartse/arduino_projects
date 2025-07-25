
# A very simple Flask Hello World app for you to get started with...

from flask import Flask
from datetime import datetime

app = Flask(__name__)

@app.route('/')
def hello_world():
    return 'Hay from Piet!'

@app.route('/localtime')
def local_timestamp():
    now = datetime.now() # current date and time
    return now

@app.route('/localdatetime')
def local_datetime():
    now = datetime.now() # current date and time
    date_time = now.strftime("%d/%m/%Y, %H:%M:%S")
    return date_time  # render_template('date-time.html', date_time=date_time)
