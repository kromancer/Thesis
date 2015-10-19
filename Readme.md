Application
===========
Convolutional Neural Network. It will be able to identify a number of objects (up till 1000) within a video stream (30 fps, 1MP per frame).
	* What kind of application do we want?
	* Can we give any real time guarantees - Can we assemble a real time kernel for the Tegra?

ForSyDe
======
So far the best practise is to desribe a CNN in [Caffe] (http://caffe.berkeleyvision.org/tutorial/net_layer_blob.html), as a number of layers and their interconnections.
	* I claim that we could be able to fit these semantics within ForSyDe SDF.
	* Are the skeleton/pattern vocabulary sufficient to describe the CNN application? 


F2CC
=====
Can we synthesize our CNN model, in its SDF description?
	* Algorithms for identifying the patterns within the process network?
	* A basic autotuning framework

