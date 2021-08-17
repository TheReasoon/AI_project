import numpy as np
import matplotlib.pylab as plt


# def step_function(x):
#     if x > 0:
#         return 1
#     else:
#         return 0
#
#
# print(step_function(2))
#
#
# def step_fuction_nympy(x):
#     y = x > 0
#     return y.astype(np.int)
#
# x = np.array([-1.0, 1.0, 2.0])
#
# print(step_fuction_nympy(x))

def step_funtion(a):
    return np.array(a > 0, dtype=np.int)


x = np.arange(-5.0, 5.0, 0.1)
y = step_funtion(x)

plt.plot(x, y)
plt.ylim(-0.1, 1.1)  # 指定y轴范围

plt.show()
