#include "ofApp.h"
//https://github.com/scotty007/ProCamToolkit/tree/linuxbuild

#include "SharedCode/GrayCodeGenerator.h"

GrayCodeGenerator generator;
bool capturing = false;
int totalProjectors = 1;
int totalDirection = 2;
int totalInverse = 2;

// every combination of these four properties
// for a total of: 10x2x2x2 = 80 images
int projector = 0;
int direction = 0;
int inverse = 0;
int pattern = 0;

string curDirectory;
long bufferTime = 500;
bool needToCapture = false;
unsigned long captureTime = 0;

void generate() {
	generator.setSize(ofApp::tw, ofApp::th);
	generator.setOrientation(direction == 0 ? PatternGenerator::VERTICAL : PatternGenerator::HORIZONTAL);
	generator.setInverse(inverse == 0);
	generator.generate();
	stringstream dirStr;
	dirStr <<
	(projector == 0 ? "left/" : "right/") <<
	(direction == 0 ? "vertical/" : "horizontal/") <<
	(inverse == 0 ? "inverse/" : "normal/");
	curDirectory = dirStr.str();
	ofDirectory::createDirectory(curDirectory, true, true);
}

bool nextState() {
    ofLog()<<"nextState() "<<pattern;
	pattern++;
	if(pattern == generator.size()) {
		pattern = 0;
		inverse++;
		if(inverse == totalInverse) {
			inverse = 0;
			direction++;
			if(direction == totalDirection) {
				direction = 0;
				projector++;
				if(projector == totalProjectors) {
					projector = 0;
					return false;
				}
			}
		}
		generate();
	}
	return true;
}

void ofApp::setup() {
	ofSetVerticalSync(true);
//	ofHideCursor();
	//ofSetDataPathRoot("../../../../../SharedData/");
	ofSetLogLevel(OF_LOG_VERBOSE);
	
    camWidth = 1920; //1280;
    camHeight = 1080; //720;
    
    gui_main.setup("cameras", "gui_main.json");
    gui_main.setPosition(20,20);
    gui_main.add(bDebug.set("debug",false));
    uidDropdown_1 = make_unique<ofxDropdown>(selectedUid_1);
    uidDropdown_1->disableMultipleSelection();
    uidDropdown_1->enableCollapseOnSelection();
    //    uidDropdown_2 = make_unique<ofxDropdown>(selectedUid_2);
    //    uidDropdown_2->disableMultipleSelection();
    //    uidDropdown_2->enableCollapseOnSelection();
    
    //get back a list of devices.
    vector<ofVideoDevice> devices = cam1.listDevices();
    
    for(size_t i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            uidDropdown_1->add( devices[i].serialID, devices[i].deviceName + " _ " + devices[i].serialID);
            //            uidDropdown_2->add( devices[i].serialID, devices[i].deviceName + " _ " + devices[i].serialID);
        }
    }
    
    
    gui_main.add(uidDropdown_1.get());
    //    gui_main.add(uidDropdown_2.get());
    
    // this goes before loading the gui, so if it actually loads ofApp::selectedCameraChanged will get called.
    uidListener_1 = selectedUid_1.newListener(this, &ofApp::selectedCameraChanged_1);
    
    if(ofFile::doesFileExist("gui_main.json")){
        gui_main.loadFromFile("gui_main.json");
    }

    bDebug = true;
    
	generate();
}

//--------------------------------------------------------------
void ofApp::selectedCameraChanged_1(string &){
    cam1.close();
    cam1.setUniqueID(selectedUid_1);
    //    cam_1.setup(camWidth, camHeight);
    cam1.setup(camWidth, camHeight);
    ofLog()<<"cam1 res "<<cam1.getWidth()<<" x "<<cam1.getHeight();
}

void ofApp::update() {

    cam1.update();
	// instead of a fixed delay, we could just wait until a frame after we see a difference
    
    
     needToCapture = capturing && (ofGetElapsedTimeMillis() - captureTime) > bufferTime;
    
    if(cam1.isFrameNew()){
        curFrame.setFromPixels(cam1.getPixels());
        if(needToCapture == true){
            curFrame.save(curDirectory + ofToString(pattern) + ".png");
            if(nextState()) {
                captureTime = ofGetElapsedTimeMillis();
            } else {
                stopCapture();
            }
        }
    }
    
}

void ofApp::draw() {
	ofBackground(0);
	
	if(!capturing) {
        ofSetColor(ofColor::pink);
        ofDrawRectangle(0,0,ofGetWidth(),ofGetHeight());
		ofPushMatrix();
		//ofScale(.25, .25);
        ofSetColor(255);
		curFrame.draw(0, 0);
		ofPopMatrix();
        
        ofPushMatrix();
        ofTranslate(0,ofGetHeight()/2);
        ofScale(0.2);
        generator.get(pattern).draw(projector * tw, 0);
        ofPopMatrix();
        
        if(bShowGui) gui_main.draw();
    } else {
        ofSetColor(0);
        ofDrawRectangle(0,0,ofGetWidth(),ofGetHeight());
        ofSetColor(255);
        generator.get(pattern).draw(0, 0);
    }
    
    if(bDebug){
        stringstream ss;
        ss<<"key f for fullscreen"<<endl;
        ss<<"key SPACE to start capture"<<endl;
        ss<<"key d for debug info"<<endl;
        ss<<"capturing: "<<capturing<<endl;
        ss<<"needToCapture: "<<needToCapture<<endl;
        ss<<"path: "<<curDirectory << ofToString(pattern) << ".png"<<endl;
        ss<<"time: "<<(ofGetElapsedTimeMillis() - captureTime)<<endl;
        
        ofSetColor(255);
        ofDrawBitmapStringHighlight(ss.str(),ofGetWidth()/2,10);
    }
}

void ofApp::startCapture() {
    ofHideCursor();
    bDebug = false;
    ofSetFullscreen(true);
	capturing = true;
	captureTime = ofGetElapsedTimeMillis();
}

void ofApp::stopCapture() {
    ofShowCursor();
	capturing = false;
}

void ofApp::keyPressed(int key) {
	if(key == ' ') {
		startCapture();
	}
	if(key == 'f') {
		ofToggleFullscreen();
	}
    
    if(key == 'd') bDebug = !bDebug;
    
    if(key == 'g'){
        bShowGui = !bShowGui;
        if(bShowGui == false){
            gui_main.saveToFile("gui_main.json");
        }
    }
}
