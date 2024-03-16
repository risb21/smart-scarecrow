# Directory to store datasets

Image extraction from datasets occurs on running the bounding box making web-interface server, instructions for which are mentioned in the parent directory.

## Adding datasets to the tool
All the datasets which have images for which bounding boxes have to be drawn can be downloaded to this directory in `.zip` format.<br>
The images from these datasets will automatically be extracted to a new subdirectory, `./data`.

## Re-doing dataset extraction
If there are changes in the datasets, i.e, they are added, deleted or modified, the previously generated `./data` directory can be deleted to redo image extraction.