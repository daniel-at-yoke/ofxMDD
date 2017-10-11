#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofDisableArbTex();
    ofLoadImage(texture, "of.png");

    // create a simple plane with 2500 vertices and get its mesh
    ofPlanePrimitive plane;
    plane.set(100, 100, 50, 50);
    mesh = plane.getMesh();
    mesh.enableTextures();

    // load the .mdd
    if (mdd.load("cloth.mdd", 5000.f)) {
        // load ok

    } else {
        // Uh-oh, failed to load .mdd. Check the log
        ofSystemAlertDialog("Could not load .mdd file");
        ofExit(1);
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    // loop animation by getting the current time mod the mdd
    // animation's approx. duration, but slowed x10
    float dur = mdd.getApproximateDuration();
    float now = fmod(ofGetElapsedTimef() / 10.f, dur);

    try {
        // get the mdd frame at the current point in time and
        // apply it to the mesh
        ofxMDDFrame frame = mdd.getInterpolatedFrameAtTime(now);
        frame.applyToMesh(mesh);
    } catch (ofxMDD::NoFrameException& e) {
        // if we're here, it probably means that the .mdd completely
        // failed to load, and there are no frames
        ofLog() << "No MDD frame for time " << time;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255);
    ofEnableDepthTest();
    ofEnableLighting();

    // draw the mesh
    camera.begin();
    light.enable();
    texture.bind();
    mesh.drawFaces();
    texture.unbind();
    light.disable();
    camera.end();

    // show some info
    ostringstream oss;
    oss << "MDD contents:" << endl;
    oss << "Frames: " << mdd.getNumFrames() << endl;
    oss << "Points: " << mdd.getNumPoints() << endl;
    oss << "Approx. FPS: " << mdd.getApproximateFps() << endl;
    oss << "Approx. duration: " << mdd.getApproximateDuration() << endl;
    oss << "First frame: " << mdd.getFirstFrameTime() << endl;
    oss << "Last frame: " << mdd.getLastFrameTime() << endl;

    ofSetColor(0);
    ofDrawBitmapString(oss.str(), 10, 20);
}
