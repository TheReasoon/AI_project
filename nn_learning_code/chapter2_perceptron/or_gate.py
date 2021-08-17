import numpy as np


def OR(x3, x4):
    x = np.array([x3, x4])
    w = np.array([0.5, 0.5])
    b = -0.2
    tmp = np.sum(w * x) + b
    print("tmp = ")
    print(tmp)
    if tmp <= 0:
        return 0
    else:
        return 1


print(OR(1, 0))
