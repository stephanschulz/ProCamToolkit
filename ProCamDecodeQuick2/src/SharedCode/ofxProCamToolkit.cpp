#include "ofxProCamToolkit.h"

using namespace ofxCv;
using namespace cv;

void getRemapPoints(string filename, int width, int height, vector<Point2f>& camImagePoints, vector<Point2f>& proImagePoints, vector<unsigned char>& colors, GrayCodeMode mode) {
    ofLog()<<"getRemapPoints() "<<filename;

    Mat cam;
    Mat codex, codey, mask;

//    grayDecode(string path, Mat& binaryCoded, Mat& cam, GrayCodeMode mode, const Mat& mask)
    grayDecode(filename + "vertical/", codex, cam, mode, mask);
    grayDecode(filename + "horizontal/", codey, cam, mode, mask);
    cv::threshold(cam, mask, 0, 255, CV_THRESH_OTSU);
    Mat remap = buildRemap(codex, codey, mask, width, height);
    for(int py = 0; py < height; py++) {
        for(int px = 0; px < width; px++) {
            Point2f camPoint = remap.at<Point2f>(py, px);
            if(camPoint != Point2f(0, 0)) {
                camImagePoints.push_back(camPoint);
                proImagePoints.push_back(Point2f(px, py));
                colors.push_back(cam.at<unsigned char>(camPoint));
            }
        }
    }
}

void getProCamImages(string filename, Mat& pro, Mat& cam, int width, int height, GrayCodeMode mode) {
    ofLog() << "getProCamImages() width " << width << " height " << height;
    Mat codex, codey, mask;
    grayDecode(filename + "vertical/", codex, cam, mode, mask);
    grayDecode(filename + "horizontal/", codey, cam, mode, mask);
    cv::threshold(cam, mask, 0, 255, CV_THRESH_OTSU);

    // Save the mask image for debugging
    ofImage mask_img;
    toOf(mask, mask_img);
    mask_img.save("threshold_mask.png");

    Mat remap = buildRemap(codex, codey, mask, width, height);
    Mat kernel(3, 3, CV_8U, cv::Scalar(1));
    morphologyEx(remap, remap, cv::MORPH_CLOSE, kernel);
    applyRemap(remap, cam, pro, width, height);
    medianBlur(pro, 3);

    // Save the remap image for debugging
    ofImage remap_img;
    toOf(remap, remap_img);
    remap_img.save("debug_remap.png");

    // Save the final projected image for debugging
    ofImage pro_img;
    toOf(pro, pro_img);
    pro_img.save("debug_pro.png");
}


void grayDecode(string path, Mat& binaryCoded, Mat& cam, GrayCodeMode mode, Mat& mask) {
    ofLog() << "grayDecode() path " << path << " mode " << mode;
    vector<Mat> thresholded;
    Mat minMat, maxMat;
    int n;
    if (mode == GRAYCODE_MODE_GRAY) {
        ofLog() << "mode == GRAYCODE_MODE_GRAY";

        ofDirectory dir;
        dir.allowExt("png"); // help ignore folders
        dir.listDir(path);
        dir.sort();
        n = dir.size();
        ofLog() << "dir.size() " << dir.size();
        thresholded.resize(n);
        ofImage image;

        for (int i = 0; i < n; i++) {
            ofLog() << "loading " << dir.getPath(i);
            image.load(dir.getPath(i));
            image.setImageType(OF_IMAGE_GRAYSCALE);
            Mat cur = toCv(image);
            imitate(cam, image);

            cv::threshold(cur, thresholded[i], 0, 255, CV_THRESH_TRIANGLE); // CV_THRESH_OTSU);
            if (i == 0) {
                cur.copyTo(minMat);
                cur.copyTo(maxMat);
            } else {
                cv::min(cur, minMat, minMat);
                cv::max(cur, maxMat, maxMat);
            }
        }

        // Create mask from the thresholded images
        mask = Mat::zeros(thresholded[0].size(), CV_8UC1);
        for (const auto& img : thresholded) {
            mask = mask | img;
        }
        cv::threshold(mask, mask, 1, 255, CV_THRESH_BINARY);
    } else {
        ofLog() << "mode == not !!! GRAYCODE_MODE_GRAY";

        ofDirectory dirNormal, dirInverse;
        dirNormal.listDir(path + "normal/");
        dirNormal.sort();
        n = dirNormal.size();
        ofLog() << "dirNormal " << dirNormal.size();

        dirInverse.listDir(path + "inverse/");
        dirInverse.sort();
        ofLog() << "dirInverse " << dirInverse.size();

        thresholded.resize(n);
        ofImage imageNormal, imageInverse;

        for (int i = 0; i < n; i++) {
            imageNormal.load(dirNormal.getPath(i));
            imageInverse.load(dirInverse.getPath(i));
            imageNormal.setImageType(OF_IMAGE_GRAYSCALE);
            imageInverse.setImageType(OF_IMAGE_GRAYSCALE);
            imitate(cam, imageNormal);
            Mat curNormal = toCv(imageNormal);
            Mat curInverse = toCv(imageInverse);
            thresholded[i] = curNormal > curInverse;
            if (i == 0) {
                minMat = min(curNormal, curInverse);
                maxMat = max(curNormal, curInverse);
            } else {
                min(minMat, curNormal, minMat);
                min(minMat, curInverse, minMat);
                max(maxMat, curNormal, maxMat);
                max(maxMat, curInverse, maxMat);
            }
        }

        // Create mask from the thresholded images
        mask = Mat::zeros(thresholded[0].size(), CV_8UC1);
        for (const auto& img : thresholded) {
            mask = mask | img;
        }
        cv::threshold(mask, mask, 1, 255, CV_THRESH_BINARY);
    }
    
    // Save the mask image for debugging
    ofImage mask_img;
    toOf(mask, mask_img);
    mask_img.save("debug_mask.png");
    
    max(maxMat, cam, cam);
    thresholdedToBinary(thresholded, binaryCoded, mask);
    
    grayToBinary(binaryCoded, thresholded.size());

    // Log a few sample values for debugging
    ofLog() << "Sample binaryCoded values after grayDecode:";
    unsigned short* binaryCodedPixels = binaryCoded.ptr<unsigned short>();
    for (int i = 0; i < min(10, binaryCoded.rows * binaryCoded.cols); i++) {
        ofLog() << "binaryCoded[" << i << "]: " << binaryCodedPixels[i];
    }

    // Save the thresholded images for debugging
    for (int i = 0; i < thresholded.size(); i++) {
        ofImage temp_img;
        toOf(thresholded[i], temp_img);
        temp_img.save("debug_thresholded_" + ofToString(i) + ".png");
    }

    // Save the binary coded image for debugging
    ofImage binary_img;
    toOf(binaryCoded, binary_img);
    binary_img.save("debug_binaryCoded.png");



    // Save the final cam image for debugging
    ofImage cam_img;
    toOf(cam, cam_img);
    cam_img.save("debug_cam.png");
}
void thresholdedToBinary(vector<Mat>& thresholded, Mat& binaryCoded, const Mat& mask) {
    ofLog() << "thresholdedToBinary()";
    int rows = thresholded[0].rows;
    int cols = thresholded[0].cols;
    int n = thresholded.size();
    int m = rows * cols;
    int tw = thresholded[0].cols;
    int th = thresholded[0].rows;

    binaryCoded = Mat::zeros(th, tw, CV_16UC1);
    unsigned short* binaryCodedPixels = binaryCoded.ptr<unsigned short>();

    for(int i = 0; i < n; i++) {
        unsigned char* curThresholdPixels = thresholded[i].ptr<unsigned char>();
        unsigned short curMask = 1 << (n - i - 1);
        for(int k = 0; k < m; k++) {
            if(curThresholdPixels[k] && mask.at<unsigned char>(k / cols, k % cols)) {
                binaryCodedPixels[k] |= curMask;
            }
        }
    }

    // Log a few sample values for debugging
    ofLog() << "Sample binaryCoded values after thresholdedToBinary:";
    for (int i = 0; i < min(10, m); i++) {
        ofLog() << "binaryCoded[" << i << "]: " << binaryCodedPixels[i];
    }

    // Save binary coded image for further inspection
    ofImage binary_img;
    toOf(binaryCoded, binary_img);
    binary_img.save("debug_binaryCoded_thresholdedToBinary.png");
}
void grayToBinary(Mat& binaryCoded, int n) {
    ofLog()<<"grayToBinary() n "<<n;
    unsigned short* binaryCodedPixels = binaryCoded.ptr<unsigned short>();

    // Built gray code to binary LUT
    int codes = 1 << n;
    vector<unsigned short> binaryLUT(codes);
    for (unsigned short binary = 0; binary < codes; binary++) {
        unsigned short gray = (binary >> 1) ^ binary;
        binaryLUT[gray] = binary;
    }

    // Convert gray code to binary using LUT
    int m = binaryCoded.rows * binaryCoded.cols;
    for (int i = 0; i < m; i++) {
        binaryCodedPixels[i] = binaryLUT[binaryCodedPixels[i]];
    }

    // Log a few sample values for debugging
    ofLog() << "Sample binaryCoded values after grayToBinary:";
    for (int i = 0; i < min(10, m); i++) {
        ofLog() << "binaryCoded[" << i << "]: " << binaryCodedPixels[i];
    }

    // Save binary coded image for further inspection
    ofImage binary_img;
    toOf(binaryCoded, binary_img);
    binary_img.save("debug_binaryCoded_grayToBinary.png");
}

Mat buildRemap(Mat& binaryCodedX, Mat& binaryCodedY, Mat& mask, int tw, int th) {
    int rows = binaryCodedX.rows;
    int cols = binaryCodedX.cols;
    ofLog() << "buildRemap() tw " << tw << " th " << th << " binaryCodedX cols " << cols << " rows " << rows;

    Mat remap(th, tw, CV_32FC2, Scalar::all(nan("")));
    Mat total(th, tw, CV_32FC2, Scalar::all(0));

    // Log dimensions and sample values of binaryCodedX, binaryCodedY, and mask
    ofLog() << "binaryCodedX size: " << binaryCodedX.size() << ", type: " << binaryCodedX.type();
    ofLog() << "binaryCodedY size: " << binaryCodedY.size() << ", type: " << binaryCodedY.type();
    ofLog() << "mask size: " << mask.size() << ", type: " << mask.type();
    
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            unsigned char curMask = mask.at<unsigned char>(row, col);
            if (curMask) {
                unsigned short tx = binaryCodedX.at<unsigned short>(row, col);
                unsigned short ty = binaryCodedY.at<unsigned short>(row, col);

                // Log the values of tx and ty for debugging
//                ofLogVerbose() << "binaryCodedX at (" << row << ", " << col << "): " << tx;
//                ofLogVerbose() << "binaryCodedY at (" << row << ", " << col << "): " << ty;

                // Ensure tx and ty are within the target dimensions
                if (tx < tw && ty < th) {
                    Point2f& remapPt = remap.at<Point2f>(ty, tx);
                    if (isnan(remapPt.x) && isnan(remapPt.y)) {
                        remapPt = Point2f(col, row);
                    } else {
                        remapPt += Point2f(col, row);
                    }
                    total.at<Point2f>(ty, tx) += Point2f(1, 1);
                } else {
//                    ofLogWarning() << "Out-of-range codex/cody: (" << tx << ", " << ty << ") at (" << col << ", " << row << ")";
                }
            }
        }
    }

    // Avoid division by zero and normalize the remap points
    for (int row = 0; row < remap.rows; row++) {
        for (int col = 0; col < remap.cols; col++) {
            Point2f& remapPt = remap.at<Point2f>(row, col);
            Point2f& totalPt = total.at<Point2f>(row, col);
            if (totalPt.x != 0 && totalPt.y != 0) {
                remapPt.x /= totalPt.x;
                remapPt.y /= totalPt.y;
            } else {
                remapPt = Point2f(nan(""), nan(""));
            }
        }
    }

    // Save the remap matrix for debugging
    ofImage remap_img;
    toOf(remap, remap_img);
    remap_img.save("debug_remap.png");

    return remap;
}


void applyRemap(Mat& remap, Mat& input, Mat& output, int width, int height) {
    ofLog()<<"applyRemap() remap.cols "<<remap.cols<<" rows "<<remap.rows<<" input "<<input.cols<<" rows "<<input.rows;
    output.create(height, width, input.type());
    int cnt_success = 0;

    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            Point2f cur = remap.at<Point2f>(y, x);
            // Debug log for current point
//            ofLogVerbose() << "Remap point (" << x << ", " << y << "): " << cur;

            // Ensure cur points are within valid input image range
            if(ofInRange(cur.x, 0, input.cols - 1) && ofInRange(cur.y, 0, input.rows - 1)){
                output.at<unsigned char>(y, x) = input.at<unsigned char>(cur.y, cur.x);
                cnt_success++;
            } else {
                // Log out-of-range points for further inspection
//                ofLogWarning() << "Out-of-range point: " << cur << " at (" << x << ", " << y << ")";
            }
        }
    }
    // Log the success rate
    ofLog()<<"cnt_success "<<cnt_success<<" / "<<(height*width)<<", perc: "<<ofMap(cnt_success, 0, (height*width), 0, 100);
}




