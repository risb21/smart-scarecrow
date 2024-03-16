import os

def get_work(users: list[str], work_for: str) -> list[str]:
    if work_for not in users:
        raise ValueError("User is not in list of users!")
    
    idx = users.index(work_for)
    imgs = os.listdir("./static/data")
    imgs.sort()

    extras = len(imgs) % len(users)
    imgs_per_user = int(len(imgs) / len(users))

    start = imgs_per_user * idx + (idx if idx < extras else extras)
    end = start + imgs_per_user + (1 if idx < extras else 0)

    return imgs[start: end]