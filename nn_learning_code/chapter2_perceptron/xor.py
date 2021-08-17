import numpy as np

def AND(x3, x4):
    x = np.array([x3, x4])
    w = np.array([0.5, 0.5])
    b = -0.7
    tmp = np.sum(w * x) + b

    if tmp <= 0:
        return 0
    else:
        return 1

def NAND(x3, x4):
    x = np.array([x3, x4])
    w = np.array([-0.5, -0.5])
    b = 0.7
    tmp = np.sum(w * x) + b

    if tmp <= 0:
        return 0
    else:
        return 1

def OR(x3, x4):
    x = np.array([x3, x4])
    w = np.array([0.5, 0.5])
    b = -0.2
    tmp = np.sum(w * x) + b

    if tmp <= 0:
        return 0
    else:
        return 1

def XOR(x1, x2):
    s1 = NAND(x1, x2)
    s2 = OR(x1, x2)
    y = AND(s1, s2)
    return y

print(XOR(1, 1))
