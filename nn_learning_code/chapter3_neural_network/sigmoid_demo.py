import numpy as np
import matplotlib.pylab as plt


def step_funtion(x):
    return np.array(x > 0, dtype=np.int)

def sigmoid(x):
    return 1 / (1 + np.exp(-x))


# x = np.array([-1.0, 1.0, 2.0])
# print(sigmoid(x))

x = np.arange(-5.0, 5.0, 0.1)
y1 = step_funtion(x)
y2 = sigmoid(x)

plt.xlabel("x")
plt.ylabel("y")
plt.plot(x, y1, linestyle="--", label="step")
plt.plot(x, y2, linestyle="-.", label="sigmoid")
plt.title('step & sigmoid', fontsize=20)
plt.legend()
plt.ylim(-0.1, 1.1)
plt.show()
