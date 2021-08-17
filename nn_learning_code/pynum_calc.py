import numpy as np

x = np.array([[1, 2], [3, 4]])
y = np.array([4.0, 5.0, 6.0, 7.0])
z = np.array([[[[1, 2], [3, 4]], [[5, 6], [7, 8]]], [[[9, 10], [11, 12]], [[13, 14], [15, 16]]]])

# print(x.shape)
# print(z.shape)
# print(x.dtype)
# print(x*y)
# print(sum(x*y))
print(z)
print('---------------------------------')
for raw in z:
     print(raw)
#print(x[1:3])
#print(z[1:, :, 1:, 1:])
#print(sum(sum(sum(sum(z)))))
