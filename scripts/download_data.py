import gdown
import os
import pathlib


def download(id):
    gdown.download(id=id, output="data.7z")
    os.system("7z x data.7z -odata")
    pathlib.Path("data.7z").unlink()


download("1PT4Lw48TRSeDEPacHNT8UmHL6o0i_eqN")
download("1yxAhYw-ViJnJTKVnvjtYOA78ZXQ_T6io")
download("1XHoPvNPg0gJLxkdFrh9xnJqFoPBKZsyv")
download("1hw40ZghCKAGrsJQN2ICm5sXPhcuxMEE7")
