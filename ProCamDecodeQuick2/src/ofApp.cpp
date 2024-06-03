#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

void ofApp::setup(){
    
    camWidth = 1920; //1280;
    camHeight = 1080; //720;
    
    gui_main.setup("cameras", "gui_main.json");
    gui_main.setPosition(20,20);
    gui_main.add(bDebug.set("debug",false));
    uidDropdown_videoSource = make_unique<ofxDropdown>(selectedUid_videoSource);
    uidDropdown_videoSource->disableMultipleSelection();
    uidDropdown_videoSource->enableCollapseOnSelection();
    
    //get back a list of devices.
    vector<ofVideoDevice> devices = videoSource.listDevices();
    for(size_t i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            uidDropdown_videoSource->add( devices[i].serialID, devices[i].deviceName + " _ " + devices[i].serialID);
        }
    }
    
    
    gui_main.add(uidDropdown_videoSource.get());
    
    // this goes before loading the gui, so if it actually loads ofApp::selectedCameraChanged will get called.
    uidListener_videoSource = selectedUid_videoSource.newListener(this, &ofApp::selectedCameraChanged_videoSource);
    
    if(ofFile::doesFileExist("gui_main.json")){
        gui_main.loadFromFile("gui_main.json");
    }

    bDebug = true;
    
   
    isCapturing = false;
    currentPattern = 0;
    totalPatterns = 10; // Adjust based on the number of Gray code patterns

    loadPatterns();
    decodePatterns();
    findCorrespondences();
    calibrateProjectorCamera();
}

//--------------------------------------------------------------
void ofApp::selectedCameraChanged_videoSource(string &){
    videoSource.close();
    videoSource.setUniqueID(selectedUid_videoSource);
    videoSource.setup(camWidth, camHeight);
    ofLog()<<"videoSource res "<<videoSource.getWidth()<<" x "<<videoSource.getHeight();
}

void ofApp::update(){
    videoSource.update();
}

void ofApp::draw(){
    ofPushMatrix();
    ofScale(0.3);
    videoSource.draw(0, 0);
    if(!undistortedImages.empty()){
        undistortedImages.back().draw(videoSource.getWidth(), 0);
    } else{
        // Test with synthetic data
              
    }
   
    visualize_fbo.draw(videoSource.getWidth(),videoSource.getHeight());
    ofPushMatrix();
    ofTranslate(videoSource.getWidth(), videoSource.getHeight()*2);
    testHomography();
    ofPopMatrix();
    
    int temp_x = 0;
    int temp_y = videoSource.getHeight();
    int temp_w = 1920/10;
    int temp_h = 1080/10;
    
    for(int i=0; i<capturedImages.size(); i++){
        capturedImages[i].draw(temp_x,temp_y,temp_w,temp_h);
        temp_x+= temp_w;
        if(temp_x >= videoSource.getWidth()){
            temp_x = 0;
            temp_y+= temp_h;
        }
    }
    
    temp_x = 0;
    temp_y+= 15;
    for(int i=0; i<thresholdedImages.size(); i++){
        thresholdedImages[i].draw(temp_x,temp_y,temp_w,temp_h);
        temp_x+= temp_w;
        if(temp_x >= videoSource.getWidth()){
            temp_x = 0;
            temp_y+= temp_h;
        }
    }
    
    
    
    ofPopMatrix();
    gui_main.draw();
}
void ofApp::loadPatterns(){
    capturedImages.clear();

    // Load horizontal normal patterns
    for(int i = 0; i < totalPatterns; i++){
        ofImage img;
        std::string filename = "horizontal/normal/" + ofToString(i) + ".png";
        bool loaded = img.load(filename);
        if (loaded) {
            capturedImages.push_back(img);
            ofLog() << "Loaded: " << filename;
        } else {
            ofLogError() << "Failed to load: " << filename;
        }
    }

    // Load horizontal inverse patterns
    for(int i = 0; i < totalPatterns; i++){
        ofImage img;
        std::string filename = "horizontal/inverse/" + ofToString(i) + ".png";
        bool loaded = img.load(filename);
        if (loaded) {
            capturedImages.push_back(img);
            ofLog() << "Loaded: " << filename;
        } else {
            ofLogError() << "Failed to load: " << filename;
        }
    }

    // Load vertical normal patterns
    for(int i = 0; i < totalPatterns; i++){
        ofImage img;
        std::string filename = "vertical/normal/" + ofToString(i) + ".png";
        bool loaded = img.load(filename);
        if (loaded) {
            capturedImages.push_back(img);
            ofLog() << "Loaded: " << filename;
        } else {
            ofLogError() << "Failed to load: " << filename;
        }
    }

    // Load vertical inverse patterns
    for(int i = 0; i < totalPatterns; i++){
        ofImage img;
        std::string filename = "vertical/inverse/" + ofToString(i) + ".png";
        bool loaded = img.load(filename);
        if (loaded) {
            capturedImages.push_back(img);
            ofLog() << "Loaded: " << filename;
        } else {
            ofLogError() << "Failed to load: " << filename;
        }
    }
}
void ofApp::decodePatterns(){
    int numPatterns = totalPatterns;

    // Ensure the decoded images are initialized correctly with integer type
    decodedImagesX = cv::Mat::zeros(videoSource.getHeight(), videoSource.getWidth(), CV_32S);
    decodedImagesY = cv::Mat::zeros(videoSource.getHeight(), videoSource.getWidth(), CV_32S);

    std::vector<ofImage> thresholdedImagesX;
    std::vector<ofImage> thresholdedImagesY;

    // Preconvert all images to single-channel cv::Mat
    std::vector<cv::Mat> normalMatsX(numPatterns);
    std::vector<cv::Mat> inverseMatsX(numPatterns);
    std::vector<cv::Mat> normalMatsY(numPatterns);
    std::vector<cv::Mat> inverseMatsY(numPatterns);

    for(int k = 0; k < numPatterns; k++){
        cv::Mat normalMatX = ofxCv::toCv(capturedImages[k]);
        cv::Mat inverseMatX = ofxCv::toCv(capturedImages[k + numPatterns]);
        if (normalMatX.channels() > 1) {
            cv::cvtColor(normalMatX, normalMatsX[k], cv::COLOR_BGR2GRAY);
            cv::cvtColor(inverseMatX, inverseMatsX[k], cv::COLOR_BGR2GRAY);
        } else {
            normalMatsX[k] = normalMatX;
            inverseMatsX[k] = inverseMatX;
        }

        cv::Mat normalMatY = ofxCv::toCv(capturedImages[k + 2 * numPatterns]);
        cv::Mat inverseMatY = ofxCv::toCv(capturedImages[k + 3 * numPatterns]);
        if (normalMatY.channels() > 1) {
            cv::cvtColor(normalMatY, normalMatsY[k], cv::COLOR_BGR2GRAY);
            cv::cvtColor(inverseMatY, inverseMatsY[k], cv::COLOR_BGR2GRAY);
        } else {
            normalMatsY[k] = normalMatY;
            inverseMatsY[k] = inverseMatY;
        }
    }

    // Print sizes and types of initial matrices for debugging
    ofLog() << "decodedImagesX size: " << decodedImagesX.size() << " type: " << decodedImagesX.type();
    ofLog() << "decodedImagesY size: " << decodedImagesY.size() << " type: " << decodedImagesY.type();

    // Decode X patterns
    for(int k = 0; k < numPatterns; k++){
        cv::Mat diffMatX, thresholdMatX;
        cv::absdiff(normalMatsX[k], inverseMatsX[k], diffMatX);
        cv::threshold(diffMatX, thresholdMatX, 0, 255, cv::THRESH_BINARY);

        // Print sizes and types of diffMatX and thresholdMatX for debugging
        ofLog() << "diffMatX size: " << diffMatX.size() << " type: " << diffMatX.type();
        ofLog() << "thresholdMatX size: " << thresholdMatX.size() << " type: " << thresholdMatX.type();

        ofImage thresholdedX;
        ofxCv::toOf(thresholdMatX, thresholdedX);
        thresholdedX.update();
        thresholdedImagesX.push_back(thresholdedX);

        // Ensure thresholdMatX has the same size and type as decodedImagesX
        if (thresholdMatX.size() == decodedImagesX.size() && thresholdMatX.type() == CV_8UC1) {
            thresholdMatX.convertTo(thresholdMatX, CV_32S);
            thresholdMatX /= 255;
            decodedImagesX += thresholdMatX * (1 << k);
        } else {
            ofLogError() << "Size or type mismatch between thresholdMatX and decodedImagesX";
        }
    }

    // Decode Y patterns
    for(int k = 0; k < numPatterns; k++){
        cv::Mat diffMatY, thresholdMatY;
        cv::absdiff(normalMatsY[k], inverseMatsY[k], diffMatY);
        cv::threshold(diffMatY, thresholdMatY, 0, 255, cv::THRESH_BINARY);

        // Print sizes and types of diffMatY and thresholdMatY for debugging
        ofLog() << "diffMatY size: " << diffMatY.size() << " type: " << diffMatY.type();
        ofLog() << "thresholdMatY size: " << thresholdMatY.size() << " type: " << thresholdMatY.type();

        ofImage thresholdedY;
        ofxCv::toOf(thresholdMatY, thresholdedY);
        thresholdedY.update();
        thresholdedImagesY.push_back(thresholdedY);

        // Ensure thresholdMatY has the same size and type as decodedImagesY
        if (thresholdMatY.size() == decodedImagesY.size() && thresholdMatY.type() == CV_8UC1) {
            thresholdMatY.convertTo(thresholdMatY, CV_32S);
            thresholdMatY /= 255;
            decodedImagesY += thresholdMatY * (1 << k);
        } else {
            ofLogError() << "Size or type mismatch between thresholdMatY and decodedImagesY";
        }
    }

    // Convert Gray code to binary using vectorized operations
    decodedImagesX = grayToBinary(decodedImagesX);
    decodedImagesY = grayToBinary(decodedImagesY);

    // Print some decoded values for verification
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            ofLog() << "Decoded X at (" << i << "," << j << "): " << decodedImagesX.at<int>(i, j);
            ofLog() << "Decoded Y at (" << i << "," << j << "): " << decodedImagesY.at<int>(i, j);
        }
    }

    // Save thresholded images for visualization
    thresholdedImages = thresholdedImagesX;
    thresholdedImages.insert(thresholdedImages.end(), thresholdedImagesY.begin(), thresholdedImagesY.end());
}

cv::Mat ofApp::grayToBinary(const cv::Mat& gray){
    cv::Mat binary = gray.clone();
    int cols = binary.cols;
    int rows = binary.rows;

    for (int shift = 1; shift < cols; shift *= 2) {
        cv::Mat shifted = cv::Mat::zeros(rows, cols, CV_32S);
        binary(cv::Rect(0, 0, cols - shift, rows)).copyTo(shifted(cv::Rect(shift, 0, cols - shift, rows)));
        cv::bitwise_xor(binary, shifted, binary);
    }

    return binary;
}

void ofApp::findCorrespondences(){
    camPoints.clear();
    projPoints.clear();

    for(int i = 0; i < videoSource.getHeight(); i+=10){
        for(int j = 0; j < videoSource.getWidth(); j+=10){
            float projX = decodedImagesX.at<float>(i, j);
            float projY = decodedImagesY.at<float>(i, j);

            if(projX >= 0 && projY >= 0){
                camPoints.push_back(cv::Point2f(j, i));
                projPoints.push_back(cv::Point2f(projX, projY));
            }
        }
    }
    
    ofLog() << "Total correspondences found: " << camPoints.size();


    visualize_fbo.clear();
    visualize_fbo.allocate(camWidth,camHeight,GL_RGBA);
   
    visualize_fbo.begin();
    ofSetColor(ofColor::black);
    ofDrawRectangle(0, 0, camWidth, camHeight);
   
    // Visualize correspondences
    for (size_t i = 0; i < camPoints.size(); i+=10000) {
        ofSetColor(ofColor::yellow);
        ofDrawCircle(camPoints[i].x, camPoints[i].y, 1);
        ofSetColor(ofColor::blue);
        ofDrawCircle(projPoints[i].x, projPoints[i].y, 1);
        ofSetColor(100);
        ofDrawLine(camPoints[i].x, camPoints[i].y, projPoints[i].x, projPoints[i].y);
        float dist = cv::norm(camPoints[i] - projPoints[i]);
        ofLog() << "Dist " << dist;
    }
    visualize_fbo.end();
}


void ofApp::calibrateProjectorCamera(){
    if(camPoints.size() >= 4){
        cv::Mat homography = cv::findHomography(camPoints, projPoints, cv::RANSAC);
        if (homography.empty()) {
            ofLogError() << "Failed to compute homography.";
            return;
        }

        ofLog() << "Computed homography: " << homography;

        // Apply the homography to the captured image to undistort it
        cv::Mat inputImage = ofxCv::toCv(videoSource.getPixels());
        cv::Mat undistortedImage;
        cv::warpPerspective(inputImage, undistortedImage, homography, inputImage.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar());

        // Print dimensions and some pixel values
        ofLog() << "Input Image Size: " << inputImage.cols << " x " << inputImage.rows;
        ofLog() << "Undistorted Image Size: " << undistortedImage.cols << " x " << undistortedImage.rows;
        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 10; ++j) {
                ofLog() << "Undistorted Pixel Value at (" << i << "," << j << "): " << undistortedImage.at<cv::Vec3b>(i, j);
            }
        }

        // Check if undistorted image is valid
        if (undistortedImage.empty()) {
            ofLogError() << "Undistorted image is empty.";
            return;
        }

        // Convert back to an ofImage to display
        ofImage undistortedOfImage;
        ofxCv::toOf(undistortedImage, undistortedOfImage);
        undistortedOfImage.update();

        // Check if undistortedOfImage is valid
        if (undistortedOfImage.isAllocated()) {
            undistortedImages.push_back(undistortedOfImage);
            ofLog() << "Undistorted image added.";
        } else {
            ofLogError() << "Failed to allocate undistorted image.";
        }
    } else {
        ofLogError() << "Not enough points for homography calculation.";
    }
}


void ofApp::testHomography(){
    // Create a synthetic image (e.g., a white square on a black background)
    cv::Mat syntheticImage = cv::Mat::zeros(500, 500, CV_8UC3);
    cv::rectangle(syntheticImage, cv::Point(100, 100), cv::Point(400, 400), cv::Scalar(255, 255, 255), -1);

    // Apply the homography to the synthetic image
    cv::Mat homography = (cv::Mat_<double>(3, 3) << -0.0773262579499968, -0.2309011178381709, 131.7261864346432,
                                                     -0.4002649063541741, -1.195883638781617, 681.7184452591396,
                                                     -0.0005870919049672487, -0.001754274015191777, 1);
    cv::Mat transformedImage;
    cv::warpPerspective(syntheticImage, transformedImage, homography, syntheticImage.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar());

    // Convert to ofImage and display
    ofImage transformedOfImage;
    ofxCv::toOf(transformedImage, transformedOfImage);
    transformedOfImage.update();
    transformedOfImage.draw(0, 0);
}
