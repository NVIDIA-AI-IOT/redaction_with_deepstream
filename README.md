# Fast Video Redaction Using NVIDIA Deepstream #

This repo contains: 

* An NVIDIA developed model for face and liscence plate detection.

* Source codes for a fast video redaction app.

For the detection model:

* It has fully converlutional structure

* It detects person faces as well as vehicle license plates

* It is optimized by TRT for fast inference during runtime of the app.

For the redaction app:

* It is developed using NVIDIA Deepstream SDK 2.0

* It reads in one .mp4 file and outputs one .mp4 file with faces and license plates redacted.

* It outputs the bbox locations of the redacted objects as kitti format for future reference.

* It has fast processing speed (>70 fps on 1080P). 

* It can be served as a video processing tool for data privacy protection purpose according to the [new EU GDPR guideline](https://www.eugdpr.org/).

* It demos how to build practical iva applications using NVIDIA deepstream SDK. 

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

### the Redaction App ###

![alt text](pipeline/pipeline-playing.png "pipeline")

### Running the Redaction App ###

1. Build app

	`make`

2. run app

	```
	./deepstream-redaction-app -c <path-to-config-file> 
							   -i <path-to-input-mp4-file> 
							  [-o <path-to-output-mp4-file> 
							   -k <path-to-output-kitti-folder>]
	```

run `./deepstream-redaction-app --help` for detailed usage.