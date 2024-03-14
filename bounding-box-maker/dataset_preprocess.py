import os

def index_datasets():
    os.chdir("./datasets")
    try:
        datasets = os.listdir()
        datasets.remove("README.md")

        datasets = [obj for obj in datasets
                     if os.path.isdir(obj) ]
        
        if len(datasets) == 0:
            print("\nThere are no datasets to index!\n")
            os.chdir("..")
            return
        
        # If there are datasets present to index
        for dataset in datasets:
            os.chdir(f"./{dataset}")
            try:
                # do something here
                print(os.listdir())
                os.chdir("..")
            except Exception as e:
                os.chdir("..")
                raise e


        os.chdir("..")

    except Exception as e:
        os.chdir("..")
        raise e


if __name__ == "__main__":
    index_datasets()