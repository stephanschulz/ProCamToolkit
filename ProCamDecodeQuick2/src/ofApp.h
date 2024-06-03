#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxProCamToolkit.h"

#include "ofxVideoGrabber.h"

#include "ofxGui.h"
#include "ofxDropdown.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();

    void loadPatterns();
       void decodePatterns();
       void findCorrespondences();
       void calibrateProjectorCamera();
       
    cv::Mat grayToBinary(const cv::Mat& gray);
    void testHomography();
  
    std::vector<ofImage> thresholdedImages;
       vector<ofImage> capturedImages;
       vector<ofImage> undistortedImages;
       bool isCapturing;
       int currentPattern;
       int totalPatterns;

       cv::Mat decodedImagesX;
       cv::Mat decodedImagesY;
       vector<cv::Point2f> camPoints;
       vector<cv::Point2f> projPoints;
    
    ofxPanel gui_main;
    bool bShowGui = true;
    
    ofParameter<bool> bDebug;
    ofxVideoGrabber videoSource;
    int camWidth, camHeight;
    
    ofParameter<string> selectedUid_videoSource = {"Selected videoSource", ""};
    unique_ptr<ofxDropdown> uidDropdown_videoSource;
    ofEventListener uidListener_videoSource;
    void selectedCameraChanged_videoSource(string &);
    
    ofFbo visualize_fbo;
};
