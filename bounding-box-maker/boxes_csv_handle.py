import csv
import os
import io

def get_index(path: str) -> int:
    start_idx = 0
    if path.split('/')[-1] in os.listdir("".join([i + '/' for i in path.split('/')[:-1]])):
        with open(path, "r") as csv_read:
            content = list(csv.reader(csv_read))
            start_idx = len(content)
    return start_idx

def file_closed(file: io.TextIOBase) -> bool:
    status = file.closed
    
    # The file handle may be an empty object, consider it closed
    if status != True and status != False:
        return True
    return status
    

if __name__ == "__main__":
    pass