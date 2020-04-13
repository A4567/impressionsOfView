#pragma once
//including the OpenCv addons dependencies along with the JSON and Kinect addons
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "opencv2/opencv.hpp"
#include "ofxJSON.h"
#include "ofxKinect.h"

class ofApp : public ofBaseApp{

    public:
        void setup();
        void update();
        void draw();
   //custom functions for setting up the api and the contour drawing
    void api();
    void setApi();
    void setContour();
    void contour();
    
    void keyPressed(int key);
    
//using the kinect as the camera in this case as it is more mobile than my laptop webcam - could also use ofVideoGrabber instead

    ofxKinect cam;
    //width and height of the video source to set sizes for the cv images
    int camW,camH;
    
//setting the variables for the API functions - the url for the API call, the key and the date which will be set as the present day in the setup and the JSON elemnt which will read the returned data
    string url,apiKey,date;
    ofxJSONElement result;
    

    //the polyline which will be drawn - the contours and the api result
    ofPolyline line;
   // contour finder CV images
    
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage, grayBg, grayDiff;
    ofxCvContourFinder contourFinder;
    //sets the background image to be compared by for finding contours
    bool bLearnBackground;
    //values for tuning the contour finder - greyThreshold and the maximum contours to find
    int thresh,contnum;
    //the points found in the contour outline which will be added to the polyline for drawing - this gets the contours found without the bounding rectangle
    vector<ofVec3f> blobpoints;
    // the elapsed time as an integer so it can be used to trigger the fade out
    int time;
};
