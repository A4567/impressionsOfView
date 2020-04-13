#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    //setting the background auto to false to keep old drawings and the colour to white as when the rectangles are drawn over it proves better for fading out the old lines
    ofSetBackgroundAuto(false);
    ofSetBackgroundColor(255);
    //setup functions for the api and the contour finder
    setContour();
    setApi();
    //setting the frame rate to 20 - making the system a bit slower
    ofSetFrameRate(20);
    
}
void ofApp::setContour(){
    //initialising the kinect - if using a camera this would be cam.setup()
    cam.init();
    cam.open();
    //setting the width and height values for ease of use further on
    camW = cam.width;
    camH = cam.height;
    
    //alloacting the image size for the contour finder images
    colorImg.allocate(camW,camH);
    grayImage.allocate(camW,camH);
    grayBg.allocate(camW,camH);
    grayDiff.allocate(camW,camH);
    //setting the threshold and number of contours
    thresh = 30;
    contnum = 1;
}

void ofApp::setApi(){
    //making the initial api call - api keys should be private - to acquire one visit the nasa api website
    apiKey = "";
    //making a string for the api call to use as the date
    string year = ofToString(ofGetYear());
    string month = ofToString(ofGetMonth());
    if(month.length() < 2){
        month.append("0");
        reverse(month.begin(), month.end());
    }
    string day = ofToString(ofGetDay());
    if(day.length() < 2){
        day.append("0");
        reverse(day.begin(), day.end());
    }
    date = year + "-" + month + "-" + day;
    
    url = "https://api.nasa.gov/neo/rest/v1/feed?start_date="+ date +"&end_date="+ date +"&api_key="+apiKey;
    result.open(url);
}

//--------------------------------------------------------------
void ofApp::update(){
    //keepign track of elapsed time
    time = ofGetElapsedTimef();
    //clearing the line and the points vector to stop them growing too big
    line.clear();
    blobpoints.clear();
    //updating the camera feed and converting it to cv images to run through the contourfinder
    cam.update();
    if (cam.isFrameNew()){
        colorImg.setFromPixels(cam.getPixels());
        colorImg.mirror(false,true);
        grayImage = colorImg; // convert our color image to a grayscale image
        if (bLearnBackground == true) {
            grayBg = grayImage; // update the background image
            bLearnBackground = false;
        }
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(thresh);
        contourFinder.findContours(grayDiff, 100, (camW*camH)/4, contnum, false, true);
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    // this scale and translation will depend on your output screen dimensions, i have done it here to have the feed in the middle of the screen the full height of it
    ofPushMatrix();
    ofTranslate(cam.width/2,0,0);
    ofScale(2, 2);
    //the function to draw the outlines
    contour();
    ofPopMatrix();
    //every 20s cover with a black rectangle whcih is see through
    if(time % 20 == 0){
        ofSetColor(0, 0, 0, 5);
        ofDrawRectangle(0,0, ofGetWidth(), ofGetHeight());
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    //key press functions for testing and tuning - b changes the background to black, v to white, space resets the bgimg for the contour finder, p and q adjust the number of contours and w,o are the grey threshold values.
    if(key == 'b'){
        ofClear(0);
    }
    if(key == 'v'){
        ofClear(255);
        ofSetColor(ofRandom(0,255),ofRandom(0,255),ofRandom(0,255),10);
    }
    if(key == ' '){
        bLearnBackground = true;
    }
    if(key == 'p'){
        contnum++;
    }
    if(key == 'q'){
        contnum--;
    }
    if(key == 'o'){
        thresh++;
    }
    if(key == 'w'){
        thresh--;
    }
    
}
//---
//function to draw lines of contours found
void ofApp::contour(){
    //calls the api fucntion to add the points to the line
    api();
    //if contours are found push the points to the blobpoints vector
    if(contourFinder.nBlobs > 0){
        for(int k = 0; k < contourFinder.nBlobs; k ++){
            for(int i = 0; i < contourFinder.blobs[k].pts.size(); i++){
                ofVec3f point(contourFinder.blobs[k].pts[i]);
                blobpoints.push_back(point);
            }
        }
        //for every point in the vector create a color based on its location and add the point to the line with a curve
        for (int j = 0; j < blobpoints.size(); j++) {
            
            ofColor lineColor;
            
            float pointAvg = (contourFinder.blobs[0].centroid.x + contourFinder.blobs[0].centroid.y)/2;//(blobpoints[j].y + blobpoints[j].x)/2;
            
            float screenAvg = (ofGetHeight() + ofGetWidth())/2;
            float camAvg = (cam.getHeight() + cam.getWidth())/2;
            float dist = ofDist(blobpoints[j].x, blobpoints[j].y, 0, 0);
            float hue = ofMap(pointAvg, 0, camAvg, 0, 255);
            
            
            lineColor.setHsb(hue, hue, 255, 5);
            ofSetColor(lineColor);
            
            line.curveTo(blobpoints[j]);
        }
        //close and draw the line
        line.close();
        
        
        line.draw();
        
    }
}



//--------------------------------------------------------------
void ofApp::api(){
    // get the number of near earth objects gets values for velocity and close approach date these are used to determine the speed at which the points move using perline noise and the color the lines will be when no people are detected
    for(int i = 0; i < result["near_earth_objects"][date].size(); i++){
        
        string kps = result["near_earth_objects"][date][i]["close_approach_data"][0]["relative_velocity"]["kilometers_per_second"].asString();
        float v = ofToFloat(kps);
        
        string cad = result["near_earth_objects"][date][i]["close_approach_data"][0]["close_approach_date_full"].asString();
        if(cad == ""){
            cad = ofToString(ofGetYear()) + "-" + "xxx" + "-" + ofToString(ofGetDay()) + " " + ofToString(ofGetHours()) + ":" + ofToString(ofGetMinutes());
        }
        cad.replace(0, 12, "");
        cad.erase(2,1);
        float hue = ofMap(ofToFloat(cad), 0000, 2359, 0, 255);
        ofColor colour;
        colour.setHsb(hue,0,255,10);
        
        float posX = (i*1.3) + cam.getWidth()/2;
        float posY = (i*1.4) + cam.getHeight()/2;
        
        float speed;
        if(contourFinder.nBlobs>0){
            speed = v/15;
        }else{
            speed = v/100;
        }
        
        
        ofVec2f point;
        point.x = ((cam.getWidth()*2) * ofNoise(ofGetElapsedTimef() * speed + posX)) - cam.getWidth()/2;
        point.y = ((cam.getHeight()*2) * ofNoise(ofGetElapsedTimef()  * speed + posY)) - cam.getHeight()/2;
        
        
        line.curveTo(point.x, point.y);
        
    }
    //if no contours are found it will draw the line anyway
    if(contourFinder.nBlobs < 1){
        line.close();
        line.draw();
    }
}
