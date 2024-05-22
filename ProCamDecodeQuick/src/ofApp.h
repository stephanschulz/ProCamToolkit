#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxProCamToolkit.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void decodeAndSave(int i, string filename);
	
//	static const int
//		pw = 1024, ph = 768,
//		cw = 3456, ch = 2304;
//    static const int
//        pw = 1280, ph = 720,
//        cw = 1280, ch = 720;
    static const int
        pw = 1920, ph = 1080,
        cw = 1920, ch = 1080;
	ofImage camImage, proImage;
};
