import numpy as np

# def softmax_nan(a):
#     exp_a = np.exp(a)
#     sum_exp_a = np.sum(exp_a)
#     y = exp_a / sum_exp_a
#     return y
#  如果计算数组过大会导致计算溢出，为了防止计算溢出，采用如下方法，指数减去数组最大值：

def softmax(a):
    c = np.max(a)  # C
    exp_a = np.exp(a-c)
    sum_exp_a = np.sum(exp_a)
    y = exp_a / sum_exp_a
    return y


a = np.array([1010, 1000, 990])

# a1 = softmax_nan(a)
# print(a1)  # y = exp_a / sum_exp_a = [nan nan nan]  #  结果溢出

a2 = softmax(a)
print(a2)  # [9.99954600e-01 4.53978686e-05 2.06106005e-09]
print(sum(a2))  # 1.0  输出总和为 1 ，是softmax函数的一个重要特性，因此才可以将softmax函数的输出解释为“概率”
b = np.array([0.3, 2.9, 4.0])
b1 = softmax(b)
print(b1)
