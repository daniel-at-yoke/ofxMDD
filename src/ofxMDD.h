#pragma once

#include <ofMain.h>

#include "ofxMDDFrame.h"

// Module name used for log messages
#define OFX_MDD_MODULE_LOG_NAME "ofxMDD"

class ofxMDD {
public:
    struct NoFrameException : public exception {
        const char * what () const throw () {
            return "No frame for the given query";
        }
    };

    struct ReadException : public exception {
        const char * what () const throw () {
            return "Failed to read";
        }
    };

    struct WriteException : public exception {
        const char * what () const throw () {
            return "Failed to read";
        }
    };

    ofxMDD();

    /// Load .mdd file from the given path
    /// Returns true on success, false otherwise (and logs a warning)
    bool load(string path, const float pointScale = 1.f) throw(ReadException);

    /// Save .mdd file at the given path
    bool save(string path, const float pointScale = 1.f) throw(WriteException);

    /// Insert a frame into this MDD's frames. Returns false if point counts do not match.
    bool insertFrame(ofxMDDFrame const & frame);

    /// Insert a mesh's vertices as a frame. Returns false if point counts do not match.
    bool insertFrameFromMesh(const float timestamp, ofMesh &mesh);

    /// Get frame at the given index
    ofxMDDFrame & operator[](int i);

    /// Get number of frames
    int getNumFrames() const;

    /// Get number of points/vertices
    int getNumPoints() const;

    /// Get frames
    const vector<ofxMDDFrame> & getFrames() const;

    /// Get the timestamp of the first frame
    const float getFirstFrameTime() const;

    /// Get the timestamp of the last frame
    const float getLastFrameTime() const;

    /// Get the mean frame delta time (returns 0 if there are fewer than 2 frames)
    float getMeanFrameTime() const;

    /// Get approximate FPS
    /// Assumes frame times are in seconds
    float getApproximateFps() const;

    /// Get the approximate duration (first frame to last frame + mean frame time)
    /// Assumes frame times are in seconds
    float getApproximateDuration() const;

    /// Get frame index at the given time offset. Returns -1 if there are no frames
    int getFrameIndexAtTime(float time);

    /// Get frame at the given time offset. Returns nullptr if there are no frames
    ofxMDDFrame * getFrameAtTime(float time);

    /// Get a linearly interpolated frame at the given time offset. Throws NoFrameException if there are no frames
    ofxMDDFrame getInterpolatedFrameAtTime(float time) throw(NoFrameException);

private:
    unsigned int totalFrames;
    unsigned int totalPoints;
    vector<ofxMDDFrame> frames;
};
