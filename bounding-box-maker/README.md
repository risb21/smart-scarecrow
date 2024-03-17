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
1. Download all datasets with desired images in the `./static` directory
2. Run the web server in this directory
   ```bash
   python app.py --debug
   ```
3. Open the locally hosted website on a browser
4. Choose user and begin defining boudning boxes
5. The user can confirm or undo bounding boxes, or simply discard the image
6. Once the user is done, they can click on `stop file` to close the open csv file and safely close the web server