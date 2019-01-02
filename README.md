# An example of using DeepStream SDK for redaction #

<p align="center">
  <img src="images/redaction_teaser_graphics.png">
</p>

The example shows how to use DeepStream SDK 3.0 for redacting faces and license plates in video streams. 

The example uses ResNet-10 to detect faces and license plates in the scene on a frame by frame basis. The detected faces and license plates are then automatically redacted, and image composited with the resulting frames encoded to an MP4 file. The example demonstrates the use of the following plugins of the DeepStream SDK – nvcuvidh264dec, nvvidconv, nvinfer and nvosd.

Note that the networks in the examples are trained with limited datasets. These networks should be considered as sample networks to demonstrate the use of plugins in the DeepStream SDK 3.0, to create a redaction application. Developers should train their networks to achieve the level of accuracy needed in their applications.

---


### Pre-requisites: ###

- [DeepStream SDK 3.0](https://developer.nvidia.com/deepstream-sdk) for Tesla

### Installing Pre-requisites: ###

Download and install DeepStream SDK 3.0

1. Click `Download for Tesla` from [NVIDIA Deepstream SDK home page](https://developer.nvidia.com/deepstream-sdk)

2. Login to NVIDIA Developer account.

3. Agree to the terms of license agreement and download DeepStream SDK 3.0 for Tesla.

4. Follow the installation instructions in the REAME in the downloaded tar file.

5. Run the samples following the instructions in the README file to make sure that the DeepStream SDK has been properly installed.

### The Redaction App Pipeline Description ###

The Redaction pipeline implements the following steps:

* Decode the mp4 file.

* Detect faces and license plates using the networks provided. The “nvinfer” plugin uses the TensorRT for performing this detection. 

* Draw colored rectangles with solid fill to obscure the faces and license plates and thus redact them. You may customize the color by changing the corresponding RBG value in `deepstream_redaction_app.c` (line 117 - 119, line 126 - 128).

* Encode the frames back to an mp4 file.

* Write the file to disc.

* Provide a supplementary file in KITTI format enumerating the bounding boxes drawn for redacting the faces and license plates. This will be needed for manual verification and rectification of the automated redaction results.

The application pipeline is shown below:

![alt text](images/pipeline-output-to-mp4.png "pipeline")

The application will output its pipeline to the folder `DOT_DIR` while running.
One can generate the pipeline by using the following command

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

