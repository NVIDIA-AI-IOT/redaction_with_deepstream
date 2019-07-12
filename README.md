# An example of using DeepStream SDK for redaction #

<p align="center">
  <img src="images/redaction_teaser_graphics.png">
</p>

The example shows how to use DeepStream SDK 4.0 for redacting faces and license plates in video streams. 

The example uses ResNet-10 to detect faces and license plates in the scene on a frame by frame basis. The detected faces and license plates are then automatically redacted, and image composited with the resulting frames encoded to an MP4 file. The example demonstrates the use of the following plugins of the DeepStream SDK – nvcuvidh264dec, nvvidconv, nvinfer and nvosd.

Note that the networks in the examples are trained with limited datasets. These networks should be considered as sample networks to demonstrate the use of plugins in the DeepStream SDK 3.0, to create a redaction application. Developers should train their networks to achieve the level of accuracy needed in their applications.

---


### Pre-requisites: ###

- [DeepStream SDK 4.0](https://developer.nvidia.com/deepstream-sdk) for Tesla

### Installing Pre-requisites: ###

Download and install DeepStream SDK 4.0

1. Click `Download for Tesla` from [NVIDIA Deepstream SDK home page](https://developer.nvidia.com/deepstream-sdk)

2. Login to NVIDIA Developer account.

3. Agree to the terms of license agreement and download DeepStream SDK 4.0 for Tesla.

4. Follow the installation instructions in the REAME in the downloaded tar file.

5. Run the samples following the instructions in the README file to make sure that the DeepStream SDK has been properly installed.

### The Redaction App Pipeline Description ###

The Redaction pipeline implements the following steps:

* Decode the mp4 file.

* Detect faces and license plates using the networks provided. The “nvinfer” plugin uses the TensorRT for performing this detection. 

* Draw colored rectangles with solid fill to obscure the faces and license plates and thus redact them. The color can be customized by changing the corresponding RBG value in `deepstream_redaction_app.c` (line 106 - 108, line 115 - 117).

* Encode the frames back to an mp4 file.

* Write the file to disc.

* Provide a supplementary file in KITTI format enumerating the bounding boxes drawn for redacting the faces and license plates. This will be needed for manual verification and rectification of the automated redaction results.

The application pipeline is shown below:

![alt text](images/pipeline-output-to-mp4.png "pipeline")

The application will output its pipeline to the folder `DOT_DIR` by specifying the environment variable `GST_DEBUG_DUMP_DOT_DIR=DOT_DIR` when running the app.
One can generate the pipeline by using the following command:

`dot -Tpng DOT_DIR/<.dot file> > pipeline/pipeline.png`

A sample output video can be found in folder `sample_videos`.

### Running the Redaction App ###

1. Downloading the application

`cd <path-to-deepstream-sdk>/sources/apps` & git clone command & `cd redaction_with_deepstream`


2. Building the application

	`make`

3. Running the application

	```
	./deepstream-redaction-app -c <path-to-config-file> 
							   -i <path-to-input-mp4-file> 
							  [-o <path-to-output-mp4-file> 
							   -k <path-to-output-kitti-folder>]
	```

	run `./deepstream-redaction-app --help` for detailed usage.

### Application Performance ###

When converting the raw mp4 video to a redacted mp4 video, the application includes three major workloads: decoding, detection and encoding. 

* The decoding perf could be affected by the resolution of the input source.

* The detetion speed could be affected by various factors of the inference model including: input dimension, batch size, floating point precision, etc. (For more information about the input dimension of the provided model, check the section `Running Speed of the provided model` below.)

* The encoding perf could be affected by bitrate. (In this reference application we are not using GPU-accelerated encoder.)

The benchmark performance can be found below:

* Testing input stream: 1920x1080 H264 encoded mp4 file. (`sample_1080p_h264.mp4` included in deepstream 4.0 package at `<path-to-deepstream-sdk>/samples/streams/`)

* Batch size: 1

* Precision: FP32

| GPU = Quadro GV100 |           | encoder bitrate |       |       |
|:------------------:|:---------:|:---------------:|:-----:|:-----:|
|                    | perf(fps) |        8m       |   4m  |   2m  |
| detector input_dim | 1080x1920 |      27.56      | 30.99 | 33.48 |
|                    |  540x960  |      37.63      | 44.67 | 50.00 |
|                    |  270x480  |      40.31      | 48.08 | 54.26 |
|                    |           |                 |       |       | 
|   GPU = TITAN Xp   |           | encoder bitrate |       |       |
|                    | perf(fps) |        8m       |   4m  |   2m  |
| detector input_dim | 1080x1920 |      24.54      | 27.15 | 28.82 |
|                    |  540x960  |      36.38      | 42.94 | 48.01 |
|                    |  270x480  |      40.67      | 48.28 | 54.60 |


### Running Speed of the provided model ###

Just as a reference application, the redaction app doesn't implement the functionality to monitor perf. Below are some benchmark data points running the in-box deepstream-app offered by deepstream SDK with the redaction model.

The application will resize the input frame to the input dimension of the model then inference on the resized frame. The input dimension is defined in [`fd_lpd_model/fd_lpd.prototxt`](https://github.com/NVIDIA-AI-IOT/redaction_with_deepstream/blob/master/fd_lpd_model/fd_lpd.prototxt#L25-L26). The input dimension will impact the processing speed significantly. 

GPU: 1 Tesla T4 
Docker container: DS 3.0 (nvcr.io/nvidia/deepstream:3.0-18.11)
Input: 1 Full HD stream
Batch size: 1
Precision: FP16

| input_dim | processing speed (fps) | GPU Utilization (%) |
|-----------|------------------------|---------------------|
| 1080x1920 |           87           |          98         |
|  540x960  |           260          |          95         |
|  270x480  |           440          |          90         |

