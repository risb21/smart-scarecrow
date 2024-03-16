# import flask
from flask import *
from flask_session import Session
import time
import random
import os
import csv

from dataset_preprocess import index_datasets
from work_distribution import get_work
from boxes_csv_handle import get_index

random.seed(time.time_ns())

application = Flask(__name__)
application.secret_key = random.randint(1, 1 << 20)
application.config["SESSION_TYPE"] = "filesystem"
Session(application)

users = ['Tanvi', 'Rishabh', 'Chelsi']
boxes_file = 0
start_idx = 0

@application.route('/', methods = ['GET', 'POST'])
def main():
    if request.method == 'POST':
        resp = request.form
        session['user'] = list(resp.to_dict())[0]
        print(f"User: {session['user']}")
        return redirect(url_for('task'))

    return render_template('index.html', users=users)

@application.route('/task', methods = ['GET', 'POST'])
def task():
    
    if request.method  == 'POST':
        resp = list(request.form.to_dict())[0]
        if resp == 'close':
            boxes_file.close()
            return redirect(url_for('end'))

    elif request.method == 'GET':
        if "user" in session:
            files_left = get_work(users, session['user'])
            return render_template(
                'task.html', 
                usr = session['user'],
                imgs_left = len(files_left),
            )
    
    
    return main()

@application.route("/end")
def end():
    return render_template('final.html', usr = session['user'])

if __name__ == "__main__":
    # Index all dataset images
    index_datasets()

    if 'boxes.csv' in os.listdir("./static/"):
        with open("./static/boxes.csv", "r") as csv_read:
            content = list(csv.reader(csv_read))
            print(len(content))
            print(content)

    start_idx = get_index("./static/boxes.csv")

    boxes_file = open("./static/boxes.csv", "a+")
    
        
    # Run application
    application.run(debug=True, port=2048)