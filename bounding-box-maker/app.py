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

""" Flask Setup (application and session) """
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
boxes_file = io.TextIOWrapper
start_idx = 0

@application.route('/', methods = ['GET', 'POST'])
def main():
    global boxes_file, start_idx
    if request.method == 'POST':
        resp = request.form
        username = list(resp)[0]
        session['user'] = User(username, get_work(users, username))
        print(f"User: {username}")

        return redirect(url_for('task'))

    return render_template('index.html', users=users)

@application.route('/task', methods = ['GET', 'POST'])
def task():
    global boxes_file, start_idx
    csv_path = f"./static/{session['user'].name}_boxes.csv"

    """ Open the csv file if it is closed, whether initialized or not """
    if file_closed(boxes_file) or boxes_file.name != csv_path:
        if boxes_file.closed == True or boxes_file.closed == False:
            boxes_file.close()
        start_idx = get_index(csv_path)
        if f"{session['user'].name}_boxes.csv" not in os.listdir("./static/"):
            with open(csv_path, "w") as f:
                pass
        boxes_file = open(csv_path, 'a')

    if request.method  == 'POST':
        command = list(request.form.to_dict().keys())[0]

        # Close file handle and save work
        if 'close' == command:
            boxes_file.close()
            return redirect(url_for('end'))
        
        writer = csv.writer(boxes_file)
        to_write: list[str] = [session['user'].task[start_idx]]

        # Write bounding boxes to csv file
        if 'accept' == command:
            # Get data from form, append to csv file            
            if "data" in request.form.to_dict().keys():
                # Boxes are marked in the image
                data = request.form.to_dict()["data"]
                to_write.extend(data.split(" "))
            else:
                # There are no boxes in the image
                # appending no. of boxes
                to_write.append("0")
            writer.writerow(to_write)
            start_idx += 1
            return redirect(url_for('task'))
        
        # Invalid image, ignore for training
        if 'discard' == command:
            # Mark the no. of boxes in csv -1, for invalid image
            to_write.append("-1")
            writer.writerow(to_write)
            start_idx += 1
            return redirect(url_for('task'))
            
    elif request.method == 'GET':
        if "user" in session:
            return render_template(
                'task.html', 
                usr = session['user'].name,
                imgs_left = len(session['user'].task) - start_idx,
                imgs_done = start_idx,
                img_name = session['user'].task[start_idx],
                img_file = f"data/{(session['user'].task)[start_idx]}",
            )    
    
    return main()

@application.route("/end")
def end():
    return render_template('final.html', usr = session['user'].name)

if __name__ == "__main__":
    # Index all dataset images
    index_datasets()
    # Run application
    application.run(debug=True, port=2048)