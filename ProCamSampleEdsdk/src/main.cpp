#include "testApp.h"
#include "ofAppGlutWindow.h"

int main() {
	ofAppGlutWindow window;
	//ofSetupOpenGL(&window, testApp::tw * 2, testApp::th, OF_FULLSCREEN);
	ofSetupOpenGL(&window, testApp::tw, testApp::th, OF_FULLSCREEN);
	ofRunApp(new testApp());
}
