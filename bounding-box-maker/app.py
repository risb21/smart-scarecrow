# import flask
from flask import *
from flask_session import Session
import time
import random

from dataset_preprocess import index_datasets

random.seed(time.time_ns())

application = Flask(__name__)
application.secret_key = random.randint(1, 1 << 20)
application.config["SESSION_TYPE"] = "filesystem"
Session(application)

# @application.route("/")
# def main():
#     return render_template('index.html')

@application.route('/', methods = ['GET', 'POST'])
def main():
    if request.method == 'POST':
        resp = request.form
        session['user'] = list(resp.to_dict())[0]
        print(f"User: {session['user']}")
        return redirect(url_for('task'))

    return render_template('index.html')

@application.route('/task', methods = ['GET', 'POST'])
def task():
    if request.method == 'POST':
        if "user" in session:
            return render_template('task.html', 
                                   usr=session['user'],
                                   imgs_left=0)
    
    return main()

if __name__ == "__main__":
    # Index all dataset images
    index_datasets()
    # Run applcation
    application.run(debug=True, port=2048)