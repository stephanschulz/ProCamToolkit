#pragma once

#include "ofMain.h"
#include "ofxVideoGrabber.h"

#include "ofxGui.h"
#include "ofxDropdown.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	
	void startCapture();
	void stopCapture();
	
    
    ofxPanel gui_main;
    bool bShowGui = true;
    
    ofParameter<bool> bDebug;
    
    ofParameter<string> selectedUid_1 = {"Selected Cam 1", ""};
    unique_ptr<ofxDropdown> uidDropdown_1;
    ofEventListener uidListener_1;
    void selectedCameraChanged_1(string &);
    
    const static int tw = 1920; //1280; //1024;
    const static int th = 1080; //720; //768;
	
    ofxVideoGrabber cam1;
	ofImage curFrame;
    int camWidth, camHeight;
    bool needToCapture;
};
