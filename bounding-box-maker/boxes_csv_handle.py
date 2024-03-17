import csv
import os

def get_file_append(path: str):
    return open(path, "a+")

def get_index(path: str):
    start_idx = 0
    if path.split('/')[-1] in os.listdir("".join([i + '/' for i in path.split('/')[:-1]])):
        with open(path, "r") as csv_read:
            content = list(csv.reader(csv_read))
            start_idx = len(content)
            print(content)
    return start_idx

if __name__ == "__main__":
    pass