#include "ofApp.h"
//https://github.com/scotty007/ProCamToolkit/tree/linuxbuild

//#include "SharedCode/GrayCodeGenerator.h"

using namespace ofxCv;
using namespace cv;

void ofApp::setup() {
	ofSetVerticalSync(true);
	ofHideCursor();
	//ofSetDataPathRoot("../../../../../SharedData/");
	ofSetLogLevel(OF_LOG_VERBOSE);
	
//    camWidth = 1280;
//    camHeight = 720;
    
    gui_main.setup("cameras", "gui_main.json");
    gui_main.setPosition(20,20);
    gui_main.add(bDebug.set("debug",false));
    ofSetVerticalSync(true);
    string folderPath = "left"; //half-calibration";
    decodeAndSave(folderPath);

    bDebug = true;
    
}


void ofApp::update() {

    
}

void ofApp::draw() {
	ofBackground(0);
	
    ofScale(0.75, 0.75);
           camImage.draw(0, 0);
           ofTranslate(0, 480);
           proImage.draw(0, 0);
    
//    if(bDebug){
//        stringstream ss;
//
//        ss<<"capturing: "<<capturing<<endl;
//        ss<<"needToCapture: "<<needToCapture<<endl;
//        ss<<"path: "<<curDirectory << ofToString(pattern) << ".png"<<endl;
//        ss<<"time: "<<(ofGetElapsedTimeMillis() - captureTime)<<endl;
//
//        ofSetColor(255);
//        ofDrawBitmapStringHighlight(ss.str(),ofGetWidth()/2,10);
//    }
}


void ofApp::keyPressed(int key) {
//	if(key == ' ') {
//		startCapture();
//	}
//	if(key == 'f') {
//		ofToggleFullscreen();
//	}
//
//    if(key == 'd') bDebug = !bDebug;
//
//    if(key == 'g'){
//        bShowGui = !bShowGui;
//        if(bShowGui == false){
//            gui_main.saveToFile("gui_main.json");
//        }
//    }
}

