# import flask
from flask import *
from flask_session import Session
import time
import random
import os
import csv
import io

from dataset_preprocess import index_datasets
from work_distribution import get_work
from boxes_csv_handle import get_index, file_closed

random.seed(time.time_ns())

application = Flask(__name__)
application.secret_key = random.randint(1, 1 << 31)
application.config["SESSION_TYPE"] = "filesystem"
Session(application)

class User:
    name: str
    task: list[str]

    def __init__(self, name: str, task: list[str] = []) -> None:
        self.name = name
        self.task = task

users = ['Tanvi', 'Rishabh', 'Chelsi']
# File handle for a user's bounding box csv file
# Initially empty object, needs special handling to know if 
# file is open or not
boxes_file = io.TextIOBase
start_idx = 0

@application.route('/', methods = ['GET', 'POST'])
def main():
    global boxes_file, start_idx
    if request.method == 'POST':
        resp = request.form
        username = list(resp)[0]
        session['user'] = User(username, get_work(users, username))
        print(f"User: {username}")

        # csv_path = f"./static/{username}_boxes.csv"
        
        # if not file_closed(boxes_file):
        #     boxes_file.close()
        # start_idx = get_index(csv_path)
        # boxes_file = open(csv_path, 'a+')
        # print("is the file closed?:", file_closed(boxes_file))
        # boxes_file.close()
        # print("is the file closed?:", file_closed(boxes_file))

        return redirect(url_for('task'))

    return render_template('index.html', users=users)

@application.route('/task', methods = ['GET', 'POST'])
def task():
    global boxes_file, start_idx
    csv_path = f"./static/{session['user'].name}_boxes.csv"
    if file_closed(boxes_file) or boxes_file.name != csv_path:
        if not file_closed(boxes_file):
            boxes_file.close()
        start_idx = get_index(csv_path)
        boxes_file = open(csv_path, 'a+')

    if request.method  == 'POST':
        resp = list(request.form)[0]
        if resp == 'close':
            boxes_file.close()
            return redirect(url_for('end'))
        
        if resp == 'accept':
            # Get data from form, append to csv file
            start_idx += 1
            return redirect(url_for('task'))
        
        if resp == 'discard':
            # Mark the no. of boxes in csv -1, for invalid image
            start_idx += 1
            return redirect(url_for('task'))
            

    elif request.method == 'GET':
        if "user" in session:
            return render_template(
                'task.html', 
                usr = session['user'].name,
                imgs_left = len(session['user'].task) - start_idx,
                img_file = f"data/{(session['user'].task)[start_idx]}"
            )    
    
    return main()

@application.route("/end")
def end():
    return render_template('final.html', usr = session['user'].name)

if __name__ == "__main__":
    # Index all dataset images
    index_datasets()

    if 'boxes.csv' in os.listdir("./static/"):
        with open("./static/boxes.csv", "r") as csv_read:
            content = list(csv.reader(csv_read))
            print(len(content))
            print(content)
    
        
    # Run application
    application.run(debug=True, port=2048)