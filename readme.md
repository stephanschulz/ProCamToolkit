ProCamToolkit is a collection software and code for [openFrameworks](http://openframeworks.cc/) aimed at making high level projector-camera calibration more accessible. It provides examples manual projector calibration using a model of a scene, projector-camera calibration using a reference pattern and gray code scanning, and multi-projector calibration using gray code scanning. A program called "mapamok" has emerged from ProCamToolkit as the most user-friendly app of the collection, and it's useful for experimenting with extremely fast projection mapping in situations where the scene can be modeled quickly (e.g., a collection of cuboids) or where the scene is modeled in advance (e.g., a 3D print, or a manufactured object like a car). Most of ProCamToolkit is written in an extremely modular way, making it possible to borrow snippets of code, including some chunks that are written with the goal of being contributed to the openFrameworks core.

ProCamToolkit also includes work in progress towards new installations being developed during Guest Research Projector v.1 at [YCAM Interlab](http://interlab.ycam.jp/en/). This includes experiments with augmented shadows using multiple projectors, and projection mapping in the YCAM library.

ProCamToolkit is available under the [MIT License](https://secure.wikimedia.org/wikipedia/en/wiki/Mit_license).

# Apps

## Geometry

### mapamok

This is a complex app that will allow you to load a COLLADA file called `model.dae`, and then specify some number of points between the model and the corresponding location in the projection. After enough points have been selected, it will solve for the projector location and set the OpenGL viewport to render with the same intrinsics as the projector. For more details about mapamok, see [the wiki](https://github.com/YCAMInterlab/ProCamToolkit/wiki).

### ProCamSampleEdsdk and ProCamSampleLibdc

This app is the first step in capturing the data to calibrate a projector-camera system. It will project and capture gray code patterns. If you are scanning a scene, you only need to do this once. If you are calibrating before scanning a scene, you will need to take one scan per detected pattern.

ProCamSampleEdsdk works with a Canon DSLR, while ProCamSampleLibdc works with a Firewire camera. After compiling the app you want to use, run the app, hit `f` to fullscreen on the projector, then the space bar to start capturing. Before you start capturing, make sure the camera is not clipping on the brights or darks. In order to capture calibration data from two projectors, change `totalProjectors` from `1` to `2`.

### CameraCalibrate

This app will calibrate a camera from a sequence of images stored on disk, and save the calibration information into a `.yml` file. To use the application, load a sequence of images into the `data/images/` directory. Then edit the `data/settings.yml` file to reflect the calibration pattern.

In the `settings.xml` file, `xCount` and `yCount` refer to the number of circles or corners in the calibration pattern. `squareSize` describes the spacing between the circles or corners.  Because calibration is unitless,`squareSize` can be in whatever units you want the calibration data to be reported as (normally, millimeters). `patternSize` is `0`, `1`, or `2`, which means `CHESSBOARD`, `CIRCLES_GRID` and `ASYMMETRIC_CIRCLES_GRID`.

### ShadowCast

ShadowCast is work in progress on an installation that explores augmented shadows using multiple projectors. To work properly, it requires two projectors that have been calibrated to a single camera. This calibration information is captured with ProCamSampleEdsdk or ProCamSampleLibdc, then processed by ProjectorGeometryCalibrate, and finally loaded from `.exr` displacement maps in the `SharedData/` folder. To explore the scenes inside `ShadowCast` without calibrating a pair of projectors, run the `Scenes` project.

### ProCamScan

Decodes scan data and projection maps onto the point cloud.

## Color

### CameraGammaSampleLibdc

This is app is the first step in capturing the data to gamma calibrate a camera.

### CameraGammaCalibrate

Once you've captured the data from the above app, you can feed it into this one to recover the gamma curve of the camera.

# Examples

### ModelSelectionExample

Demonstrates how screen-space model selection is implemented for the ModelProjector app.

### RecursiveAverageExample

Recursively averages all the images in a folder. Useful for extracting gray images from a sequence of gray code scans.

# Other

### SharedCode

Includes `ofxProCamToolkit`, which is the core code backing the decoding involved in the other apps. Also `PatternGenerator` and `GrayCodeGenerator` are responsible for creating the projected patterns used in the other apps. Finally, it includes `scenes/`, a variety of generative visuals driven by user interaction. Each of these is used as a test scene for projector alignment.

### SharedData

Camera calibration and gamma calibration information shared between apps is saved here. There is also a small collection of useful shaders.

### Scenes

Demonstrates all of the scenes inside `SharedCode/scenes/`.

# Undocumented

### ProCamCalibrate
### ProCamDecode*
### ProCamScanGuess
### RecursiveAverageExample
### ProjectorGeometryCalibrate

- - --

*ProCamToolkit is codeveloped by [YCAM Interlab](http://interlab.ycam.jp/en/).*