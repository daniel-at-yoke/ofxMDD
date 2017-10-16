# ofxMDD

Read and write .mdd vertex animation files in openFrameworks

## Description
ofxMDD provides means of reading and writing .mdd files, a common format for per-vertex-per-frame animation data.

Quoting from [the modo 701 manual](http://modo.docs.thefoundry.co.uk/modo/701/help/pages/animation/deformers/MDDInfluence.html)  

> MDD files were invented by Daisuke Ino during his work on a dynamics engine built as a plug in to a host 3D application. These files are a very simple point cache that simply stores a location in space for every vertex in a model for every frame in the animation. [...]

> Today there are MDD exporters for almost every major 3D application on the market.

## Requirements
Built against openFrameworks 0.9.8 on macOS, but it's all fairly simple, and _should_ work with other versions/different platforms.

## Installation
Clone or download and unzip this repo into you OpenFrameworks `addons` folder and rename the folder from `ofxMDD-master` to `ofxMDD`.

Add the addon to your OF project with the project generator, and include it with

    #include "ofxMDD.h"

## Usage
No real docs yet, but the example should provide a starting point.

### Limitations and known issues
- OpenFrameworks's bundled 3D model loader (`ofxAssimpModelLoader`) has its own idea about vertex ordering. Which is quite problematic, since .mdd files specify everything assuming a fixed, repeatable vertex order. So a 3D package may write an .mdd based on its vertex order, but even if the 3D model is exported with that vertex order, `ofxAssimpModelLoader` seems to scramble it when loading, meaning the .mdd applies incorrectly.  
  One way to get around this is to export a basic .obj file of your mesh from your OF app, which serializes the vertex order that your app is using. 3D packages will respect this order, and write .mdd files using that order.

- Some 3D packages appear to write an extra frame to the .mdd. The declared frame count matches the number of frame times in the "header", but then there's an extra frame of vector data. Since it's not declared with a frame time, it's ignored of ofxMDD, but a warning is written to the log.

- There's currently no way to _remove_ a frame from an `ofxMDD` instance. It just wasn't immediately needed.

- ofxMDD loads the entire file immediately, and .mdd files can be big, so bad things might happen.

- Files are assumed to be well-formed, with frames being sequential in time. ofxMDD relies on this for time-seeking frames and inserting frames at the correct spots.

## Version history

### 0.0.2
Fixed bug in `ofxMDD::insertFrame()`

### 0.0.1
Initial release.

## License
See LICENSE.md
