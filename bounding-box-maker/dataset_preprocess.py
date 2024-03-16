import os
import shutil
import glob
from zipfile import ZipFile
import re

def has_img(files: list[str]) -> bool:
    for file in files:
        if re.match("^.*\.(png|jpe?g)$", file):
            return True

    return False

def path_to_img(paths: list[(str, bool)] = [("./", False)]) -> list[(str, bool)]:
    dirs = [obj for obj in os.listdir()
               if os.path.isdir(obj)   ]
    files = [obj for obj in os.listdir()
               if os.path.isfile(obj)   ]
    
    # if dir has images
    if len(files) > 0 and has_img(files):
        path, _ = paths[-1]
        paths[-1] = (path, True)
    
    if len(dirs) > 0:
        current_dir, _ = paths[-1]
        for dir in dirs:
            os.chdir(f"./{dir}")
            paths.append((current_dir + dir + "/", False))
    
            try:
                paths = path_to_img(paths)
            except Exception as e:
                os.chdir("..")
                raise e
    
            os.chdir("..")

    return paths

def index_datasets():
    os.chdir("./datasets")
    try:
        datasets = [obj for obj in os.listdir()
                       if os.path.isdir(obj)   ]
        
        if "data" in datasets:
            print("Data is already flattened, quitting preprocessing")
            os.chdir("..")
            return
        
        zip_files = glob.glob("*.zip")

        print("\nExtracting datasets from .zip files...")

        for file in zip_files:
            if file[:-4] in datasets:
                print(f"  {file} is already unzipped, skipping")
                continue

            print(f"  Extracting from {file} ...")
            with ZipFile(file, mode='r') as zip:
                zip.extractall(path=f"./{file[:-4]}")
                print(f"    Extracted {file}!\n")

        datasets = [obj for obj in os.listdir()
                       if os.path.isdir(obj)   ]
        
        if len(datasets) == 0:
            print("\nThere are no datasets to index!\n")
            os.chdir("..")
            return
        
        print("Creating uniform file structure...")

        img_paths = []

        for dataset in datasets:
            os.chdir(f"./{dataset}")
            try:
                paths_with_imgs = [
                    path for path, img_present in 
                    path_to_img([(f"./{dataset}/", False)])
                    if img_present
                ]
                img_paths.extend(paths_with_imgs)
                os.chdir("..")

            except Exception as e:
                os.chdir("..")

        print("  Directories with images:", img_paths)
        print("  Flattening...")

        os.mkdir("./data")
        
        for path in img_paths:
            depth = len(path[2:-1].split("/"))
            img_parent: str = path[2:-1].split("/")[-1].replace(" ", "_")
            print(os.getcwd(), depth, img_parent)
            os.chdir(path)

            try:
                images = [
                    file for file in os.listdir() 
                    if os.path.isfile(file)
                    # only image files
                    and re.match("^.*\.(png|jpe?g)$", file, re.IGNORECASE)
                ]

                for img in images:
                    file_name = img.split(".")
                    file_name[-2] += "_" + img_parent
                    file_name = "".join([
                        val + "." if i != len(file_name) - 1 
                        else val for i, val in enumerate(file_name)
                    ])
                    
                    os.rename("./" + img, "./%sdata/%s" % ("../" * depth, file_name))

            except Exception as e:
                os.chdir("../" * depth)
                raise e
            
            os.chdir("../" * depth)

        for dir in datasets:
            shutil.rmtree(dir)

    except Exception as e:
        os.chdir("..")
        raise e
    
    os.chdir("..")


if __name__ == "__main__":
    index_datasets()