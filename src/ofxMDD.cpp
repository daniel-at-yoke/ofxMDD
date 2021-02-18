#include "ofxMDD.h"

#define VALUE_32BIT_WIDTH 4
#define VECTOR_COMPONENTS 3

/// MDD files are big-endian, so this function flips the byte-order of ints and floats
template <typename T>
static T flip32BitEndian(const T &input)
{
    assert(sizeof(T) == VALUE_32BIT_WIDTH);

    T output;
    char *inBytes = (char *)&input;
    char *outBytes = (char *)&output;
    outBytes[0] = inBytes[3];
    outBytes[1] = inBytes[2];
    outBytes[2] = inBytes[1];
    outBytes[3] = inBytes[0];
    return output;
}

/// Read a big-endian int/float from buffer and convert to the host's endianness
template <typename T>
static T read32BitBigEndian(filebuf *buffer)
{
    assert(sizeof(T) == VALUE_32BIT_WIDTH);

    T input;

    char *readBytes = (char *)&input;

    int bytesRead = buffer->sgetn(readBytes, VALUE_32BIT_WIDTH);

    if (bytesRead != VALUE_32BIT_WIDTH)
    {
        throw ofxMDD::ReadException();
    }

#ifdef TARGET_LITTLE_ENDIAN
    return flip32BitEndian<T>(input);
#else
    return input;
#endif
}

/// Write a host-endian int/float value as big-endian to buffer
template <typename T>
static void write32BitBigEndian(filebuf *buffer, const T &value)
{
    assert(sizeof(T) == VALUE_32BIT_WIDTH);

#ifdef TARGET_LITTLE_ENDIAN
    T flipped = flip32BitEndian<T>(value);
    char *writeBytes = (char *)&flipped;
#else
    char *writeBytes = (char *)&value;
#endif

    int writtenBytes = buffer->sputn(writeBytes, VALUE_32BIT_WIDTH);

    if (writtenBytes != VALUE_32BIT_WIDTH)
    {
        throw ofxMDD::WriteException();
    }
}

/// Predicate used for finding/inserting frames by time
bool frameTimeLowerBound(ofxMDDFrame const &frame, const float time)
{
    return (frame.getFrameTime() < time);
}

ofxMDD::ofxMDD()
{
    totalFrames = 0;
    totalPoints = 0;
}

bool ofxMDD::load(string path, const float pointScale)
{
    ofFile file(path);

    if (!file.canRead())
    {
        ofLogError(OFX_MDD_MODULE_LOG_NAME) << "Can't read file " << path;
        return false;
    }

    uint64_t length = file.getSize();

    // need at least 2 32-bit ints for frame and point counts
    if (length < 2 * VALUE_32BIT_WIDTH)
    {
        ofLogError(OFX_MDD_MODULE_LOG_NAME) << "File is too short " << path;
        return false;
    }

    filebuf *buf = file.getFileBuffer();

    if (!buf->is_open())
    {
        ofLogError(OFX_MDD_MODULE_LOG_NAME) << "Failed to open file " << path;
        return false;
    }

    totalFrames = read32BitBigEndian<uint32_t>(buf);
    totalPoints = read32BitBigEndian<uint32_t>(buf);

    // check size
    uint64_t expectedSize = 2 * VALUE_32BIT_WIDTH;                                                         // already read
    expectedSize += (uint64_t)totalFrames * VALUE_32BIT_WIDTH;                                             // frame times
    expectedSize += (uint64_t)totalFrames * (uint64_t)totalPoints * VECTOR_COMPONENTS * VALUE_32BIT_WIDTH; // points

    if (length < expectedSize)
    { // not enough bytes
        ofLogError(OFX_MDD_MODULE_LOG_NAME) << "File is too short " << path;
        totalFrames = 0;
        totalPoints = 0;
        return false;
    }
    else if (length > expectedSize)
    { // too many bytes?!
        // oddly, my 3d package appears to output *more* frames than
        // there are frame times... No idea what that's about, but log
        // it if it happens
        int frameSize = totalPoints * VALUE_32BIT_WIDTH * 3;
        int leftover = length - expectedSize;
        if (leftover % frameSize == 0)
        {
            int extra = leftover / frameSize;
            ofLogWarning(OFX_MDD_MODULE_LOG_NAME) << "File contains " << extra << " extra " << (extra == 1 ? "frame" : "frames");
        }
        else
        {
            ofLogWarning(OFX_MDD_MODULE_LOG_NAME) << "File contains an odd number of extra bytes; might be corrupted";
        }
    }

    // read frame times
    for (unsigned int i = 0; i < totalFrames; i++)
    {
        ofxMDDFrame frame;
        frame.frameTime = read32BitBigEndian<float>(buf);
        frames.push_back(frame);
    }

    // read vectors
    for (ofxMDDFrame &frame : frames)
    {
        for (unsigned int j = 0; j < totalPoints; j++)
        {
            ofVec3f point;
            point.x = read32BitBigEndian<float>(buf) * pointScale;
            point.y = read32BitBigEndian<float>(buf) * pointScale;
            point.z = read32BitBigEndian<float>(buf) * pointScale;
            frame.points.push_back(point);
        }
    }

    buf->close();

    return true;
}

bool ofxMDD::save(string path, const float pointScale)
{
    ofFile file(path, ofFile::WriteOnly);

    if (!file.canWrite())
    {
        ofLogError(OFX_MDD_MODULE_LOG_NAME) << "Can't write file " << path;
        return false;
    }

    filebuf *buf = file.getFileBuffer();

    if (!buf->is_open())
    {
        ofLogError(OFX_MDD_MODULE_LOG_NAME) << "Failed to open file " << path;
        return false;
    }

    write32BitBigEndian<int32_t>(buf, (int32_t)totalFrames);
    write32BitBigEndian<int32_t>(buf, (int32_t)totalPoints);

    for (ofxMDDFrame &frame : frames)
    {
        write32BitBigEndian<float>(buf, frame.getFrameTime());
    }

    for (ofxMDDFrame &frame : frames)
    {
        for (ofVec3f const &point : frame.getPoints())
        {
            write32BitBigEndian<float>(buf, point.x * pointScale);
            write32BitBigEndian<float>(buf, point.y * pointScale);
            write32BitBigEndian<float>(buf, point.z * pointScale);
        }
    }

    buf->close();

    return true;
}

bool ofxMDD::insertFrame(ofxMDDFrame const &frame)
{
    if (!frames.empty() && frame.getNumPoints() != totalPoints)
    {
        return false;
    }

    if (frames.empty() || frame.getFrameTime() > frames.back().getFrameTime())
    {
        frames.push_back(frame);
    }
    else if (frame.getFrameTime() < frames.front().getFrameTime())
    {
        frames.insert(frames.begin(), frame);
    }
    else
    {
        int index = getFrameIndexAtTime(frame.getFrameTime());
        vector<ofxMDDFrame>::iterator pos = frames.begin() + index + 1;
        frames.insert(pos, frame);
    }

    totalFrames = (unsigned int)frames.size();
    if (totalFrames == 1)
    {
        totalPoints = frames[0].getNumPoints();
    }

    return true;
}

bool ofxMDD::insertFrameFromMesh(const float timestamp, ofMesh &mesh)
{
    if (!frames.empty() && mesh.getNumVertices() != totalPoints)
    {
        return false;
    }

    vector<ofVec3f> verts;
    for (auto v : mesh.getVertices())
    {
        verts.push_back(v);
    }

    ofxMDDFrame frame(timestamp, verts);
    return insertFrame(frame);
}

ofxMDDFrame &ofxMDD::operator[](int i)
{
    return frames[i];
}

int ofxMDD::getNumFrames() const
{
    return totalFrames;
}

int ofxMDD::getNumPoints() const
{
    return totalPoints;
}

const vector<ofxMDDFrame> &ofxMDD::getFrames() const
{
    return frames;
}

const float ofxMDD::getFirstFrameTime() const
{
    if (frames.empty())
    {
        return 0.f;
    }

    return frames[0].getFrameTime();
}

const float ofxMDD::getLastFrameTime() const
{
    if (frames.empty())
    {
        return 0.f;
    }

    return frames[frames.size() - 1].getFrameTime();
}

float ofxMDD::getMeanFrameTime() const
{
    if (frames.size() < 2)
    {
        return 0.f;
    }

    float sum = 0.f;
    for (int i = 1; i < frames.size(); i++)
    {
        float delta = frames[i].getFrameTime() - frames[i - 1].getFrameTime();
        sum += delta;
    }

    return sum / (float)(frames.size() - 1);
}

float ofxMDD::getApproximateFps() const
{
    if (frames.size() < 2)
    {
        return 0.f;
    }

    return 1.f / getMeanFrameTime();
}

float ofxMDD::getApproximateDuration() const
{
    if (frames.size() < 2)
    {
        return 0.f;
    }

    return getLastFrameTime() + getMeanFrameTime() - getFirstFrameTime();
}

int ofxMDD::getFrameIndexAtTime(float time)
{
    if (frames.empty())
    {
        return -1;
    }

    vector<ofxMDDFrame>::iterator iter = lower_bound(frames.begin() + 1, frames.end() - 1, time, frameTimeLowerBound);
    return iter - frames.begin() - 1;
}

ofxMDDFrame *ofxMDD::getFrameAtTime(float time)
{
    int index = getFrameIndexAtTime(time);
    if (index == -1)
    {
        ofxMDDFrame frame;
        return nullptr;
        ;
    }
    else
    {
        return &frames[index];
    }
}

ofxMDDFrame ofxMDD::getInterpolatedFrameAtTime(float time)
{
    int index = getFrameIndexAtTime(time);

    if (index == -1)
    {
        throw NoFrameException();
    }

    if (index == frames.size() - 1)
    {
        return frames[index];
    }

    float diff = frames[index + 1].getFrameTime() - frames[index].getFrameTime();
    float ratio = (time - frames[index].getFrameTime()) / diff;

    vector<ofVec3f> interpolated;
    for (int i = 0; i < getNumPoints(); i++)
    {
        ofVec3f v = frames[index][i].getInterpolated(frames[index + 1][i], ratio);
        interpolated.push_back(v);
    }

    ofxMDDFrame frame(time, interpolated);
    return frame;
}

#undef VALUE_32BIT_WIDTH
#undef VECTOR_COMPONENTS
