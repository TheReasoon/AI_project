import numpy as np


# method 1:
def AND(x1, x2):
    w1, w2, theta = 0.5, 0.5, 0.7
    tmp0 = x1 * w1 + x2 * w2
    if tmp0 <= theta:
        return 0
    elif tmp0 > theta:
        return 1
print(AND(1, 1))


print('----------------------------------weight and bias-------------------------------------')
x0 = np.array([0, 1])    # enter
w0 = np.array([0.5, 0.5])    # weight
b0 = -0.7
print(w0 * x0)
print(np.sum(w0 * x0))
print(np.sum(w0 * x0) + b0)
print('-------------------------------------------------------------------------------------')
def AND_WB(x3, x4):
    x = np.array([x3, x4])
    w = np.array([0.5, 0.5])
    b = -0.7
    tmp = np.sum(w * x) + b
    if tmp <= 0:
        return 0
    else:
        return 1
print(AND_WB(1, 0))
