# An example of using DeepStream SDK for redaction #

<p align="center">
  <img src="images/redaction_teaser_graphics.png">
</p>

The example shows how to use DeepStream SDK 4.0 for redacting faces and license plates in video streams. 

The example uses ResNet-10 to detect faces and license plates in the scene on a frame by frame basis. The detected faces and license plates are then automatically redacted. The image composited with the resulting frames can be displayed on screen or be encoded to an MP4 file by the choice of user. The example runs on both NVIDIA dGPUs as well as NVIDIA jetson platforms (tested on Jetson Xavier and Jetson Nano). The example demonstrates the use of the following plugins of the DeepStream SDK – nvcuvidh264dec, nvvidconv, nvinfer and nvosd.

Note that the networks in the examples are trained with limited datasets. These networks should be considered as sample networks to demonstrate the use of plugins in the DeepStream SDK 4.0, to create a redaction application. Developers should train their networks to achieve the level of accuracy needed in their applications.

---


### Pre-requisites: ###

- [DeepStream SDK 4.0](https://developer.nvidia.com/deepstream-sdk)

### Installing Pre-requisites: ###

Download and install DeepStream SDK 4.0

1. Click `Download` from [NVIDIA Deepstream SDK home page](https://developer.nvidia.com/deepstream-sdk), then select `DeepStream 4.0 for T4 and V100` if you work on NVIDIA dGPUS or select `DeepStream 4.0 for Jetson` if you work on NVIDIA Jetson platforms. 

2. Login to NVIDIA Developer account.

3. Agree to the terms of license agreement and download DeepStream SDK 4.0.

4. Follow the installation instructions in the REAME in the downloaded tar file.

5. Run the samples following the instructions in the README file to make sure that the DeepStream SDK has been properly installed on your system.

### The Redaction App Pipeline Description ###

The Redaction pipeline implements the following steps:

* Decode the mp4 file or read the stream from a webcam (tested with C920 Pro HD Webcam from Logitech).

* Detect faces and license plates using the networks provided. The “nvinfer” plugin uses the TensorRT for performing this detection. 

* Draw colored rectangles with solid fill to obscure the faces and license plates and thus redact them. The color can be customized by changing the corresponding RBG value in `deepstream_redaction_app.c` (line 109 - 111, line 118 - 120).

* Display the frames on screen or encode the frames back to an mp4 file and then write the file to disc.

* User can choose to ouput supplementary files in KITTI format enumerating the bounding boxes drawn for redacting the faces and license plates. This will be needed for manual verification and rectification of the automated redaction results.

The application pipeline is shown below:

![alt text](images/pipeline-output-to-mp4.png "pipeline")

The application will output its pipeline to the folder `DOT_DIR` by specifying the environment variable `GST_DEBUG_DUMP_DOT_DIR=DOT_DIR` when running the app.
One can generate the pipeline by using the following command:

`dot -Tpng DOT_DIR/<.dot file> > pipeline/pipeline.png`

A sample output video can be found in folder `sample_videos`.

### Running the Redaction App ###

1. Downloading the application

`cd <path-to-deepstream-sdk>/sources/apps` & git clone command & `cd redaction_with_deepstream`

(if you want to use it with Deepstream 3.0, do `$ git checkout 324e34c1da7149210eea6c8208f2dc70fb7f952a`)

2. Building the application

	`make`

3. Running the application

	```
	./deepstream-redaction-app -c <path-to-config-file> 
							  [-i <path-to-input-mp4-file> 
							   -o <path-to-output-mp4-file> 
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
|                    |           |                 |       |       | 
|   Jetson Xavier    |           | encoder bitrate |       |       |
|                    | perf(fps) |        8m       |   4m  |   2m  |
| detector input_dim |  540x960  |      14.17      | 16.25 | 18.13 |
|                    |  270x480  |      14.24      | 16.39 | 18.52 |

Note: the app is using a non-gpu accelerated encoder provided from gstreamer, so that the end-to-end performance is bounded by the encoder, specially for the Jetson Xavier case. The end-to-end performance with NVIDIA's gpu accelerated encoder will be tested in the future.

### Running Speed of the provided model ###

The application will resize the input frame to the input dimension of the model then inference on the resized frame. The input dimension is defined in [`fd_lpd_model/fd_lpd.prototxt`](https://github.com/NVIDIA-AI-IOT/redaction_with_deepstream/blob/master/fd_lpd_model/fd_lpd.prototxt#L25-L26). The input dimension will impact the processing speed significantly. 

Just as a reference application, the redaction app doesn't implement the functionality to monitor perf. Below are some benchmark data points running the in-box deepstream-app offered by deepstream SDK with the redaction model:

* GPU: 1 Tesla T4 

* Docker container: DS 3.0 (nvcr.io/nvidia/deepstream:3.0-18.11)

* Input: 1 Full HD stream

* Batch size: 1

* Precision: FP16

| input_dim | processing speed (fps) | GPU Utilization (%) |
|-----------|------------------------|---------------------|
| 1080x1920 |           87           |          98         |
|  540x960  |           260          |          95         |
|  270x480  |           440          |          90         |

