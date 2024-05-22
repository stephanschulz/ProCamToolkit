#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"

#include "ofxVideoGrabber.h"

#include "ofxGui.h"
#include "ofxDropdown.h"

using namespace ofxCv;
using namespace cv;

enum GrayCodeMode {
    GRAYCODE_MODE_NORMAL,
    GRAYCODE_MODE_INVERSE
};

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
    
    ofImage camImage, proImage;
    
    void decodeAndSave(string folderPath) {
        ofLog()<<"decodeAndSave() folderPath "<<folderPath;
        camImage.allocate(1920, 1080, OF_IMAGE_GRAYSCALE);
        camImage.getPixels().set(0);
        
//        proImage.allocate(1024 / 2, 768 / 2, OF_IMAGE_GRAYSCALE);
        proImage.allocate(1920,1080, OF_IMAGE_GRAYSCALE);
        proImage.getPixels().set(0);
        
        getProCamImages(folderPath, proImage, camImage, 1920,1080);
        
        camImage.save("results/" + folderPath + "-camImage.png");
        camImage.update();
        
        proImage.save("results/" + folderPath + "-proImage.png");
        proImage.update();
    }

//
//    void draw() {
//        ofScale(0.75, 0.75);
//        camImage.draw(0, 0);
//        ofTranslate(0, 480);
//        proImage.draw(0, 0);
//    }

    void getProCamImages(string folderPath, ofImage& pro, ofImage& cam, int width, int height) {
        
        ofLog()<<"getProCamImages() folderPath "<<folderPath<<" GRAYCODE_MODE_NORMAL ";
        Mat camMat = toCv(cam);
        Mat proMat = toCv(pro);  // Create a temporary cv::Mat variable for the projector image
        Mat codex, codey, mask;

        // Specify the correct paths for the vertical and horizontal graycode images
        string verticalNormalPath = folderPath + "/vertical/normal/";
        string verticalInversePath = folderPath + "/vertical/inverse/";
        string horizontalNormalPath = folderPath + "/horizontal/normal/";
        string horizontalInversePath = folderPath + "/horizontal/inverse/";

        grayDecode(verticalNormalPath, verticalInversePath, codex, camMat, GRAYCODE_MODE_NORMAL);
        grayDecode(horizontalNormalPath, horizontalInversePath, codey, camMat, GRAYCODE_MODE_NORMAL);

        cv::threshold(camMat, mask, 0, 255, CV_THRESH_OTSU);
        Mat remap = buildRemap(codex, codey, mask, width, height);
        applyRemap(remap, camMat, proMat, width, height);  // Use the temporary variable here
        pro.update();
        cam.update();
    }

    void grayDecode(string normalPath, string inversePath, Mat& binaryCoded, Mat& cam, GrayCodeMode mode) {
        ofLogVerbose() << "grayDecode()";
        vector<Mat> thresholded;
        Mat minMat, maxMat;
        int n;

        ofDirectory dirNormal, dirInverse;
        dirNormal.listDir(normalPath);
        dirInverse.listDir(inversePath);
        n = dirNormal.size();

        thresholded.resize(n);
        ofImage imageNormal, imageInverse;

        for(int i = 0; i < n; i++) {
            ofLogVerbose() << "loading " << dirNormal.getPath(i) << " and " << dirInverse.getPath(i);
            imageNormal.load(dirNormal.getPath(i));
            imageInverse.load(dirInverse.getPath(i));
            imageNormal.setImageType(OF_IMAGE_GRAYSCALE);
            imageInverse.setImageType(OF_IMAGE_GRAYSCALE);
            Mat curNormal = toCv(imageNormal);
            Mat curInverse = toCv(imageInverse);
            imitate(cam, imageNormal);

            thresholded[i] = curNormal > curInverse;
            if(i == 0) {
                minMat = min(curNormal, curInverse);
                maxMat = max(curNormal, curInverse);
            } else {
                min(minMat, curNormal, minMat);
                min(minMat, curInverse, minMat);
                max(maxMat, curNormal, maxMat);
                max(maxMat, curInverse, maxMat);
            }
        }

        max(maxMat, cam, cam);
        thresholdedToBinary(thresholded, binaryCoded);
        grayToBinary(binaryCoded, n);

        // Debugging: Check for invalid values in cam
        for (int i = 0; i < cam.rows * cam.cols; i++) {
            if (isnan(cam.ptr<float>()[i]) || isinf(cam.ptr<float>()[i])) {
                ofLog() << "Invalid cam value at index " << i;
                cam.ptr<float>()[i] = 0;  // Set to a default value
            }
        }
    }

    void thresholdedToBinary(vector<Mat>& thresholded, Mat& binaryCoded) {
        ofLogVerbose() << "thresholdedToBinary()";
        int rows = thresholded[0].rows;
        int cols = thresholded[0].cols;
        int n = thresholded.size();
        binaryCoded = Mat::zeros(rows, cols, CV_16UC1);
        unsigned short* binaryCodedPixels = binaryCoded.ptr<unsigned short>();

        for(int i = 0; i < n; i++) {
            unsigned char* curThresholdPixels = thresholded[i].ptr<unsigned char>();
            unsigned short curMask = 1 << (n - i - 1);
            for(int k = 0; k < rows * cols; k++) {
                if(curThresholdPixels[k]) {
                    binaryCodedPixels[k] |= curMask;
                }
            }
        }

        // Debugging: Check for invalid values
        for(int k = 0; k < rows * cols; k++) {
            if (isnan(binaryCodedPixels[k]) || isinf(binaryCodedPixels[k])) {
                ofLog() << "Invalid binary coded value at index " << k;
                binaryCodedPixels[k] = 0;  // Set to a default value
            }
        }
    }

    void grayToBinary(Mat& binaryCoded, int n) {
        unsigned short* binaryCodedPixels = binaryCoded.ptr<unsigned short>();
        int codes = 1 << n;
        vector<unsigned short> binaryLUT(codes);
        for(unsigned short binary = 0; binary < codes; binary++) {
            unsigned short gray = (binary >> 1) ^ binary;
            binaryLUT[gray] = binary;
        }
        int m = binaryCoded.rows * binaryCoded.cols;
        for(int i = 0; i < m; i++) {
            binaryCodedPixels[i] = binaryLUT[binaryCodedPixels[i]];
        }
    }

    Mat buildRemap(Mat& binaryCodedX, Mat& binaryCodedY, Mat& mask, int width, int height) {
        int rows = binaryCodedX.rows;
        int cols = binaryCodedX.cols;
        Mat remap(height, width, CV_32FC2);
        Mat total(height, width, CV_32FC2, Scalar::all(1));  // Initialize total with ones to avoid division by zero

        for(int row = 0; row < rows; row++) {
            for(int col = 0; col < cols; col++) {
                unsigned char curMask = mask.at<unsigned char>(row, col);
                if(curMask) {
                    unsigned short tx = binaryCodedX.at<unsigned short>(row, col);
                    unsigned short ty = binaryCodedY.at<unsigned short>(row, col);

                    if (tx >= 0 && tx < width && ty >= 0 && ty < height) {
                        remap.at<Point2f>(ty, tx) += Point2f(col, row);
                        total.at<Point2f>(ty, tx) += Point2f(1, 1);
                    } else {
                        ofLog() << "Invalid remap coordinates: (" << tx << ", " << ty << ")";
                    }
                }
            }
        }

        divide(remap, total, remap);

        // Check for any remaining invalid values and set them to a default value
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Point2f &cur = remap.at<Point2f>(y, x);
                if (isnan(cur.x) || isinf(cur.x) || isnan(cur.y) || isinf(cur.y)) {
                    ofLog() << "Invalid value after remap division: (" << cur.x << ", " << cur.y << ")";
                    cur = Point2f(-1, -1);  // Set to an invalid default value
                }
            }
        }

        return remap;
    }


    void applyRemap(Mat& remap, Mat& input, Mat& output, int width, int height) {
        output.create(height, width, input.type());
        for(int y = 0; y < height; y++) {
            for(int x = 0; x < width; x++) {
                Point2f cur = remap.at<Point2f>(y, x);
                // Debugging output
                if (cur.x < 0 || cur.x >= input.cols || cur.y < 0 || cur.y >= input.rows) {
                    ofLog() << "Out of bounds: (" << cur.x << ", " << cur.y << ") at (" << x << ", " << y << ")";
                }
                // Check if the coordinates are within bounds
                if (cur.x >= 0 && cur.x < input.cols && cur.y >= 0 && cur.y < input.rows) {
                    output.at<unsigned char>(y, x) = input.at<unsigned char>(cur.y, cur.x);
                } else {
                    output.at<unsigned char>(y, x) = 0; // Assign a default value for out-of-bounds coordinates
                }
            }
        }
    }
};
