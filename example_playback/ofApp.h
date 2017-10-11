#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"

#include "ofxMDD.h"

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();

    ofxMDD mdd;

    ofMesh mesh;

    ofEasyCam camera;
    ofLight light;
    ofTexture texture;
};
