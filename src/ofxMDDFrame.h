#pragma once

#include <ofMain.h>

class ofxMDD;

class ofxMDDFrame {
    friend class ofxMDD;
    
public:
    
    ofxMDDFrame() {}
    
    /// Create a new frame with the given frame time and vertices
    ofxMDDFrame(float frameTime, vector<ofVec3f> &points) {
        this->frameTime = frameTime;
        this->points = points;
    }
    
    /// Get the frame's timestamp
    float getFrameTime() const {
        return frameTime;
    }
    
    /// Get number of points/vertices
    int getNumPoints() const {
        return points.size();
    }
    
    /// Get a ofVec3f point by its index
    ofVec3f & operator[](int i) {
        return points[i];
    }
    
    /// Get the points vector
    const vector<ofVec3f> & getPoints() const {
        return points;
    }
    
    /// Set the mesh's vertices to this frame's point positions
    /// Only the 
    void applyToMesh(ofMesh & mesh) {
        int min = MIN(mesh.getNumVertices(), getNumPoints());
        for(int i = 0; i < min; i++) {
            mesh.setVertex(i, points[i]);
        }
    }

private:
    
    vector<ofVec3f> points;

    float frameTime;
};
