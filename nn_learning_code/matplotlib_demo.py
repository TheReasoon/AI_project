import numpy as np
import matplotlib.pyplot as plt

x = np.arange(0, 6, 0.1)
# 以0.1为单位，生成0到6的数据

y1 = np.sin(x)
y2 = np.cos(x)
y3 = np.tan(x)

# 绘制图形：
plt.plot(x, y1, label="sin")    # 添加label :sin 默认实线绘制
plt.plot(x, y2, linestyle="--", label="cos")    # 用--绘制
plt.plot(x, y3, linestyle="-.", label="tan")    # 用_.绘制

plt.xlabel("X")  # x轴标签
plt.ylabel("Y")  # y轴标签
plt.title('sin and cos and x^2', fontsize=20)  # 标题

plt.legend()

#from matplotlib.image import imread
#img = imread('picture/orange.jpg')
#plt.imshow(img)

plt.show()

