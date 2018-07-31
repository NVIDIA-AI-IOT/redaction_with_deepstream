# An example of using DeepStream SDK for redaction #

The example shows how to use DeepStream 2.0 for redacting faces and license plates in video streams. The example uses ResNet-10 to detect faces and LPs in the scene on a frame by frame basis. The detected faces and LP are then filled, image composited, and  resulting frames encoded to  an MP4 file. The example demonstrates the use of the SDK's  following plugins – nvcuvidh264dec, nvvidconv, nvinfer and nvosd.
Note that the networks in the examples are trained with limited datasets and are not guaranteed for any accuracy. They are purely offered as an example to demonstrate the use of plugins in the DeepStream SDK 2.0

---


### Pre-requisites: ###

- [DeepStream 2.0](https://developer.nvidia.com/deepstream-sdk) for Tesla

### Installing Pre-requisites: ###

Download and install DeepStream 2.0

1. Click `Download for Tesla` from [NVIDIA Deepstream SDK home page](https://developer.nvidia.com/deepstream-sdk)

2. Login to NVIDIA Developer account.

3. Agree the terms of liscence agreement and download deepstream SDK 2.0 for Tesla

4. Follow the installation instructions in the REAME in the downloaded tar file.

5. Key points during the DS installation include: build opencv with CUDA9.2 and gstreamer enabled; install TRT 4.0.

6. Make sure deepstream SDK is properly installed by running the samples following the intrucitons in README.

Once DeepStream SDK is installed, you have the ability to develop your own fast video processing applications.

### The Redaction App ###

In this app, we decode mp4 file, detection faces and license plates using "nvinfer", referred as the pgie, draw color rectangles on the original video to cover the objects of interests for redaction purpose (black patches on license plates and skin-tone patches on faces), encode back to mp4 file, and output bounding boxes of objects of interests to file in kitti format. nvinfer element attach some MetaData to the buffer. By attaching the probe function after nvinfer, one can extract meaningful information from this inference. Please refer the "osd_sink_pad_buffer_probe" function in the sample code. For details on the Metadata format, refer to the file "gstnvdsmeta.h"

The pipeline of the app is shown below:

![alt text](pipeline/pipeline-playing.png "pipeline")

The app will ouput its pipeline to the folder `DOT_DIR` while running.
One can generate the pipeline by `dot -Tpng DOT_DIR/<.dot file> > pipeline/pipeline.png`

A sample output video can be found in folder `sample_videos`.

### Running the Redaction App ###

1. `cd <path-to-deepstream-sdk>/sources/apps` & download this repo & `cd redaction_from_deepstream`


2. Build app

	`make`

3. Run app

	```
	./deepstream-redaction-app -c <path-to-config-file> 
							   -i <path-to-input-mp4-file> 
							  [-o <path-to-output-mp4-file> 
							   -k <path-to-output-kitti-folder>]
	```

	run `./deepstream-redaction-app --help` for detailed usage.

