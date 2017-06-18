#-*- coding: utf-8 -*-
import numpy as np
import ctypes

import sys
sys.path.append('./mnist')
from load_mnist import load_mnist

print("read dataset...")
images, labels = load_mnist("testing", path="./mnist")
images = images.astype(np.float32)/255.

NUM_TEST_IMAGES = 100

class Network(object):
	def __init__(self, net_path):
		self.lib = ctypes.cdll.LoadLibrary("./build/libpylib.so")
		self.net = self.lib.getNet(ctypes.c_char_p(net_path.encode('utf-8')))
		self.out_buf = np.zeros((10), dtype = np.float32)
		
	def __del__(self):
		self.lib.delNet(self.net)	
		
	def inference(self, input):
		self.lib.inference(self.net, input.ctypes.data, self.out_buf.ctypes.data)
		return self.out_buf

print("create network...")	

net = Network("./mnist_model/mnist.caffemodel")
net_256 = Network("./mnist_model/mnist_256.caffemodel")
net_128 = Network("./mnist_model/mnist_128.caffemodel")
net_64 = Network("./mnist_model/mnist_64.caffemodel")
net_32 = Network("./mnist_model/mnist_32.caffemodel")
net_16 = Network("./mnist_model/mnist_16.caffemodel")
model_list = [("ori", net), ("256", net_256), ("128", net_128), ("64", net_64), ("32", net_32), ("16", net_16)]

test_images = [images[idx, :,:].copy() for idx in xrange(NUM_TEST_IMAGES)]
print("run test...")		

import time

for model in model_list:
	name = model[0]
	model_net = model[1]

	start_time = time.time()

	cnt = 0
	ok = 0
	for idx in xrange(NUM_TEST_IMAGES):
		start_time_each = time.time()
		out = model_net.inference(test_images[idx])
		cnt += 1
		if labels[idx, :] == np.argmax(out):
			ok += 1
		else:
			start_time_each = time.time()
			print("image %d"%(idx))
			print("real number: %d"%(labels[idx, :]))
			print("prediction result: %d"%(np.argmax(out)))
			print("each elapsed: %f"%(time.time() - start_time_each))


	end_time = time.time()
	print("elapsed time: %f"%(end_time-start_time))
	print("Correct rate: " + str(float(ok) / float(cnt)))
