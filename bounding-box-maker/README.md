# Bounding box making interface
An interface to draw and log all bounding boxes in the images in datasets.

## The purpose of the web server
The web server allows for an easy to code and use interface to define bounding boxes around images present in the datasets. <br>
The functions fulfilled by the web server are:
- Extraction of datasets from zip files
- Recursively find and move images present in all datasets to a uniform location
- Distribute work among all human users of the system
- Log each user and present images to them according to their allocated work
- Store bounding box information for each image
- Ability to resume work from where the user left off

## Steps to run the web server
1. Download all datasets in `.zip` format with desired images in the `./static` directory
2. Install `virtualenv` using pip, create a virtual environment in this directory, activate it and install all requirements, using `requirements.txt`
   
   ```sh
   pip install virtualenv
   python -m venv venv
   source ./venv/bin/activate
   pip install -r requirements.txt
   ```
3. Run the web server in this directory
   ```sh
   python app.py --debug
   ```
4. Open the locally hosted website on a browser
5. Choose user and begin defining boudning boxes
6. The user can confirm or undo bounding boxes, or simply discard the image
7. Once the user is done, they can click on `stop file` to close the open csv file and safely close the web server
8. After stopping the webserver, you may deactivate the virtual environment
   ```sh
   deactivate
   ```

You may use the flattened dataset images in the `./static/data/` directory and the bounding boxes for each user in `{name of user}_boxes.csv` file in `./static`.

Ensure that you click on the `Save Work` button on the `/task` page of the webapp before shutting it down, or else all work will be discarded.