from app import application
from dataset_preprocess import index_datasets

if __name__ == "__main__":
    index_datasets()
    application.run(port=2048)