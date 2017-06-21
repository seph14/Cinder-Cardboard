/*
 Cinder-Cardboard
 This code is intended for use with Cinder. And is a direct port of Google's VRView,

 https://github.com/google/vrview
 http://libcinder.org
 https://github.com/seph14
 
 Copyright (c) 2016, Seph Li - All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "Hmd.h"

#include "VertexDistorter.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"

#ifdef CINDER_GL_ES_3
#include "cinder/MotionManager.h"

static const char* BarrelDistortionVertex =
"#version 300 es\n"
"uniform mat4 ciModelViewProjection;\n"
"in vec4 ciPosition;\n"
"in vec2 ciTexCoord0;\n"
"out highp vec2	TexCoord;\n"
"void main( void ) {\n"
"   gl_Position	= ciModelViewProjection * ciPosition;\n"
"   TexCoord	= ciTexCoord0;\n"
"}\n";

static const char* BarrelDistortionFrag =
"#version 300 es\n"
"precision highp float;\n"
"uniform sampler2D tex0;\n"

"uniform highp vec2 distortion;\n"
"uniform highp vec4 backgroundColor;\n"
"uniform highp vec4 projectionLeft;\n"
"uniform highp vec4 unprojectionLeft;\n"

"in highp vec2 TexCoord;\n"
"out vec4 oColor;\n"

"float poly(float val) {\n"
"   return (val < 0.0000010) ? 10000.0 : (1.0 + (distortion.x + distortion.y * val) * val);\n"
"}\n"

"vec2 barrel(vec2 v, vec4 projection, vec4 unprojection) {\n"
"   vec2 w = (v + unprojection.zw) / unprojection.xy;\n"
"   return projection.xy * (poly(dot(w, w)) * w) - projection.zw;\n"
"}\n"

"void main() {\n"
// right projections are shifted and vertically mirrored relative to left
"   vec4 projectionRight = \n"
"   (projectionLeft + vec4(0.0, 0.0, 1.0, 0.0)) * vec4(1.0, 1.0, -1.0, 1.0);\n"
"   vec4 unprojectionRight = \n"
"   (unprojectionLeft + vec4(0.0, 0.0, 1.0, 0.0)) * vec4(1.0, 1.0, -1.0, 1.0);\n"

"   vec2 a = (TexCoord.x < 0.5) ? \n"
"   barrel(vec2(TexCoord.x / 0.5, TexCoord.y), projectionLeft, unprojectionLeft) : \n"
"   barrel(vec2((TexCoord.x - 0.5) / 0.5, TexCoord.y), projectionRight, unprojectionRight);\n"

"   if (a.x < 0.0 || a.x > 1.0 || a.y < 0.0 || a.y > 1.0) {\n"
"       oColor = backgroundColor;\n"
"   } else {\n"
"       oColor = texture(tex0, vec2(a.x * 0.5 + (TexCoord.x < 0.5 ? 0.0 : 0.5), a.y));\n"
"   }\n"
"}\n";
#elif CINDER_GL_ES

#include "cinder/MotionManager.h"

static const char* BarrelDistortionVertex =
"#version 100\n"
"uniform mat4 ciModelViewProjection;\n"
"attribute vec4 ciPosition;\n"
"attribute vec2 ciTexCoord0;\n"
"varying highp vec2	TexCoord;\n"
"void main( void ) {\n"
"   gl_Position	= ciModelViewProjection * ciPosition;\n"
"   TexCoord	= ciTexCoord0;\n"
"}\n";

static const char* BarrelDistortionFrag =
"#version 100\n"
"precision highp float;\n"
"uniform sampler2D tex0;\n"

"uniform highp vec2 distortion;\n"
"uniform highp vec4 backgroundColor;\n"
"uniform highp vec4 projectionLeft;\n"
"uniform highp vec4 unprojectionLeft;\n"

"varying highp vec2 TexCoord;\n"

"float poly(float val) {\n"
"   return (val < 0.0000010) ? 10000.0 : (1.0 + (distortion.x + distortion.y * val) * val);\n"
"}\n"

"vec2 barrel(vec2 v, vec4 projection, vec4 unprojection) {\n"
"   vec2 w = (v + unprojection.zw) / unprojection.xy;\n"
"   return projection.xy * (poly(dot(w, w)) * w) - projection.zw;\n"
"}\n"

"void main() {\n"
// right projections are shifted and vertically mirrored relative to left
"   vec4 projectionRight = \n"
"   (projectionLeft + vec4(0.0, 0.0, 1.0, 0.0)) * vec4(1.0, 1.0, -1.0, 1.0);\n"
"   vec4 unprojectionRight = \n"
"   (unprojectionLeft + vec4(0.0, 0.0, 1.0, 0.0)) * vec4(1.0, 1.0, -1.0, 1.0);\n"

"   vec2 a = (TexCoord.x < 0.5) ? \n"
"   barrel(vec2(TexCoord.x / 0.5, TexCoord.y), projectionLeft, unprojectionLeft) : \n"
"   barrel(vec2((TexCoord.x - 0.5) / 0.5, TexCoord.y), projectionRight, unprojectionRight);\n"

"   if (a.x < 0.0 || a.x > 1.0 || a.y < 0.0 || a.y > 1.0) {\n"
"       gl_FragColor = backgroundColor;\n"
"   } else {\n"
"       gl_FragColor = texture2D(tex0, vec2(a.x * 0.5 + (TexCoord.x < 0.5 ? 0.0 : 0.5), a.y));\n"
"   }\n"
"}\n";
#else
static const char* BarrelDistortionVertex =
"#version 150\n"
"uniform mat4 ciModelViewProjection;\n"
"in vec4 ciPosition;\n"
"in vec2 ciTexCoord0;\n"
"out highp vec2	TexCoord;\n"
"void main( void ) {\n"
"   gl_Position	= ciModelViewProjection * ciPosition;\n"
"   TexCoord	= ciTexCoord0;\n"
"}\n";

static const char* BarrelDistortionFrag =
"uniform sampler2D tex0;\n"

"uniform vec2 distortion;\n"
"uniform vec4 backgroundColor;\n"
"uniform vec4 projectionLeft;\n"
"uniform vec4 unprojectionLeft;\n"

"in vec2  TexCoord;\n"
"out vec4 oColor;\n"

"float poly(float val) {\n"
"   return (val < 0.0000010) ? 10000.0 : (1.0 + (distortion.x + distortion.y * val) * val);\n"
"}\n"

"vec2 barrel(vec2 v, vec4 projection, vec4 unprojection) {\n"
"   vec2 w = (v + unprojection.zw) / unprojection.xy;\n"
"   return projection.xy * (poly(dot(w, w)) * w) - projection.zw;\n"
"}\n"

"void main() {\n"
    // right projections are shifted and vertically mirrored relative to left
"   vec4 projectionRight = \n"
"   (projectionLeft + vec4(0.0, 0.0, 1.0, 0.0)) * vec4(1.0, 1.0, -1.0, 1.0);\n"
"   vec4 unprojectionRight = \n"
"   (unprojectionLeft + vec4(0.0, 0.0, 1.0, 0.0)) * vec4(1.0, 1.0, -1.0, 1.0);\n"

"   vec2 a = (TexCoord.x < 0.5) ? \n"
"   barrel(vec2(TexCoord.x / 0.5, TexCoord.y), projectionLeft, unprojectionLeft) : \n"
"   barrel(vec2((TexCoord.x - 0.5) / 0.5, TexCoord.y), projectionRight, unprojectionRight);\n"

"   if (a.x < 0.0 || a.x > 1.0 || a.y < 0.0 || a.y > 1.0) {\n"
"       oColor = backgroundColor;\n"
"   } else {\n"
"       oColor = texture(tex0, vec2(a.x * 0.5 + (TexCoord.x < 0.5 ? 0.0 : 0.5), a.y));\n"
"   }\n"
"}\n";
#endif

using namespace std;
using namespace ci;
using namespace ci::cardboard;
using namespace ci::gl;
using namespace ci::app;

// Constants from vrtoolkit: https://github.com/googlesamples/cardboard-java.
//static const float DEFAULT_INTERPUPILLARY_DISTANCE = 0.06f;
//static const float DEFAULT_FIELD_OF_VIEW = 40.f;

static const int SKIP_UNSTABLE_FRAMES = 30;
static const int CALC_MOTION_FRAMES = 5;

//TODO: this is not the best way to init structs, try find a better way
static const CardboardParams CardboardV2 = CardboardParams{
    "Cardboard-V2",
    60.f,
    0.064f,
    0.035f,
    0.039f,
    {0.34f,0.55f},
    {
        -0.33836704f, -0.18162185f, 0.862655f, -1.2462051f,
        1.0560602f, -0.58208317f, 0.21609078f, -0.05444823f,
        0.009177956f, -0.0009904169f, 0.00006183535f, -0.0000016981803f
    }
};

CardboardParams CardboardV1 = CardboardParams{
    "Cardboard-V1",
    40.f,
    0.060f,
    0.035f,
    0.042f,
    {0.441f,0.156f},
    {
        -0.4410035f, 0.42756155f, -0.4804439f, 0.5460139f,
        -0.58821183f, 0.5733938f, -0.48303202f, 0.33299083f,
        -0.17573841f, 0.0651772f, -0.01488963f, 0.001559834f
    }
};

Device Nexus5 = Device{ 0.110f, 0.062f, 0.004f };
Device Nexus6 = Device { 0.133f, 0.074f, 0.004f };
Device GalaxyS6 = Device { 0.114f, 0.0635f, 0.0035f };
Device GalaxyNote4 = Device { 0.125f, 0.0705f, 0.0045f };
Device LGG3 = Device { 0.121f, 0.068f, 0.003f };

Device iPhone4 = Device { 0.075f, 0.050f, 0.0045f };
Device iPhone5 = Device { 0.089f, 0.050f, 0.0045f };
Device iPhone6 = Device { 0.104f, 0.058f, 0.005f };
Device iPhone6p = Device { 0.112f, 0.068f, 0.005f };

Hmd::Hmd(CardboardParams params, bool initVertexDistortion){
    init(params, initVertexDistortion);
}

Hmd::Hmd(CardboardVersion version, bool initVertexDistortion){
    CardboardParams params;
    
    if(version == VERSION_2){
        params = CardboardV2;
    }else{
        params = CardboardV1;
    }
    
    init(params, initVertexDistortion);
}

Hmd::~Hmd(){
    
}

void Hmd::init(CardboardParams params, bool initVertexDistortion){
    int scrWidth = toPixels(getWindowWidth());
    int scrHeight= toPixels(getWindowHeight());
    if(scrWidth < scrHeight){
        int tmp = scrHeight;
        scrHeight = scrWidth;
        scrWidth = tmp;
    }
    
    mDeviceName = params.DeviceName;
    mFov = params.Fov;
    mInterLensDistance = params.InterLensDistance;
    mBaselineLensDistance = params.BaselineLensDistance;
    mScreenLensDistance = params.ScreenLensDistance;
    mDistortionCoefficients = params.DistortionCoefficients;
    mInverseCoefficients = params.InverseCoefficients;
    mCurrEye = Eye::UNKNOWN;
    
    gl::enableVerticalSync(false);
    setFrameRate(60.f);
    mBackgroundColor = Color::black();
    
#if defined( CINDER_GL_ES )
    MotionManager::enable( 60.0f );
    mMotionReady = false;
    if(scrWidth < 1000){
        mWidthMeters = iPhone4.width;
        mHeightMeters= iPhone4.height;
        mBevelMeters = iPhone4.border;
    }else if(scrWidth < 1200){
        mWidthMeters = iPhone5.width;
        mHeightMeters= iPhone5.height;
        mBevelMeters = iPhone5.border;
    }else if(scrWidth < 1400){
        mWidthMeters = iPhone6.width;
        mHeightMeters= iPhone6.height;
        mBevelMeters = iPhone6.border;
    }else{
        mWidthMeters = iPhone6p.width;
        mHeightMeters= iPhone6p.height;
        mBevelMeters = iPhone6p.border;
    }
#else
    mMotionReady = true;
    const float METERS_PER_INCH = 0.0254f;
    const float metersPerPixelX = METERS_PER_INCH / 317.1f;
    const float metersPerPixelY = METERS_PER_INCH / 320.2f;
     
    mWidthMeters = metersPerPixelX * scrWidth;
    mHeightMeters= metersPerPixelY * scrHeight;
    mBevelMeters = 0.003f;
#endif
    
    if(!initVertexDistortion){
        /*init render buffer and barrel distortion shader*/
        mBarrelDistortionShader = GlslProg::create(BarrelDistortionVertex,BarrelDistortionFrag);
        updateBarrelUniforms();
        
        gl::Texture2d::Format tfmt;
        tfmt.setMinFilter( GL_LINEAR );
        tfmt.setMagFilter( GL_LINEAR );
        tfmt.setInternalFormat(GL_RGB8);
        gl::Fbo::Format fmt;
        fmt.setColorTextureFormat( tfmt );
        fmt.depthTexture();
        try {
            mRenderBuffer = gl::Fbo::create( scrWidth, scrHeight, fmt );
        }catch( const std::exception& e ) {
            console() << "FBO ERROR: " << e.what() << std::endl;
        }
    }
    mUseVertexDistorter = initVertexDistortion;
    
    //is 80 fov correct?
    mCamera.setPerspective(68.07361f, 0.5f*scrWidth/scrHeight, 0.3f, 1000.0f);
    mCamera.lookAt( vec3( 0.f ), vec3( 0.f, 0.f, -1.0f ) );
    mCamera.setEyeSeparation(mInterLensDistance);
    
    //change the parameter here to set with different initial direction
    mDefaultAngle = glm::atan(-1.f, 0.f);
    mDefaultAngle = (mDefaultAngle > 0 ? mDefaultAngle : (2*M_PI + mDefaultAngle));
    
    mInitDirection = vec3(0.f);
    mMotionCount = 0;
}

#if defined( CINDER_GL_ES )
void Hmd::updateCamera(const app::InterfaceOrientation &orientation){
    if( !MotionManager::isEnabled()) return;
    
    mCamera.setOrientation( MotionManager::getRotation( orientation ) );
    if(mDesiredDirectionApplied && !mMotionReady && MotionManager::isDataAvailable()){
        mMotionCount += 1;
        //skip first few unstable frames to get the correct initial facing direction
        if(mMotionCount > SKIP_UNSTABLE_FRAMES)
            mInitDirection += mCamera.getViewDirection();
        if(mMotionCount >= CALC_MOTION_FRAMES + SKIP_UNSTABLE_FRAMES){
            mInitDirection /= CALC_MOTION_FRAMES;
            float ang = glm::atan(mInitDirection.z, mInitDirection.x);
            ang = (ang > 0 ? ang : (2*M_PI + ang));
            mDefaultAngle = mDesiredDirection + mDefaultAngle - ang;
            mMotionReady = true;
        }
    }
}
#endif

bool Hmd::hasDirectionApplied(){
    return (mMotionReady && mDesiredDirectionApplied) || !mDesiredDirectionApplied;
}

void Hmd::setDefaultDirection(float angle){
#if defined( CINDER_GL_ES )
    if(!mMotionReady){
        mDesiredDirectionApplied = true;
        mDesiredDirection = toRadians(angle);
    }
#else
    if(!mMotionReady){
        mDesiredDirectionApplied = true;
        mMotionReady = true;
        mDefaultAngle = toRadians(angle);
    }
#endif
}

void Hmd::setCameraPosition(ci::vec3 pos){
    mCamera.setEyePoint(pos);
}

const vec4 Hmd::projectionMatrixToVector(mat4 mat){
    vec4 res;
    const float *pSource = (const float*)glm::value_ptr(mat);
    res[0] = pSource[4*0+0];
    res[1] = pSource[4*1+1];
    res[2] = pSource[4*2+0];
    res[3] = pSource[4*2+1];
    return res;
}

void Hmd::bindEye(Eye eye){
    
    auto width = toPixels(getWindowWidth());
    auto height= toPixels(getWindowHeight());

    if(eye == Eye::LEFT){
        mCamera.enableStereoLeft();
        gl::viewport( 0, 0, width / 2, height );
        if(mUseVertexDistorter){
            mDistorter->updateWithFov(getFieldOfViewLeftEye());
        }
    }else{
        mCamera.enableStereoRight();
        gl::viewport( width / 2, 0, width / 2, height );
        if(mUseVertexDistorter){
            mDistorter->updateWithFov(getFieldOfViewRightEye());
        }
    }
    
    if(!mUseVertexDistorter && mCurrEye == Eye::UNKNOWN){
        mRenderBuffer->bindFramebuffer();
        gl::clear(Color(0,0,0));
    }

    mCurrEye = eye;
    gl::setMatrices( mCamera );
    if(mMotionReady && mDesiredDirectionApplied)
        gl::rotate(mDefaultAngle,vec3(0,1,0));
}

void Hmd::unbind(){
    mCurrEye = Eye::UNKNOWN;
    if(!mUseVertexDistorter){
        //render buffer to screen with barrel distortion shader
        mRenderBuffer->unbindFramebuffer();
    }
    auto size = toPixels(getWindowSize());
    gl::viewport(size);
    gl::setMatricesWindow(size);
}

void Hmd::render(){
    gl::ScopedGlslProg scope(mBarrelDistortionShader);
    gl::ScopedTextureBind texBind(mRenderBuffer->getColorTexture(), 0);
    mBarrelDistortionShader->uniform("tex0", 0);
    gl::drawSolidRect(toPixels(getWindowBounds()));
}

HmdRef Hmd::create(CardboardVersion version, bool initVertexDistortion){
    return std::make_shared<Hmd>(version,initVertexDistortion);
}

void Hmd::updateBarrelUniforms(){
    auto distortedProj = getProjectionMatrixLeftEye(false);
    auto undistortedProj = getProjectionMatrixLeftEye(true);
    auto viewport = getUndistortedViewportLeftEye();
    
    int scrWidth = toPixels(getWindowWidth());
    int scrHeight= toPixels(getWindowHeight());
    
    float xScale = viewport[2] / (scrWidth/2);
    float yScale = viewport[3] / scrHeight;
    float xTrans = (2 * (viewport[0] + viewport[2] / 2) / (scrWidth/2)) - 1.f;
    float yTrans = (2 * (viewport[1] + viewport[3] / 2) / scrHeight) - 1.f;
    
    vec4 projectionLeft = projectionMatrixToVector(distortedProj);
    vec4 unprojLeft     = projectionMatrixToVector(undistortedProj);
    
    projectionLeft[2] -= 1;
    projectionLeft[3] -= 1;
    projectionLeft /= 2.0f;
    
    unprojLeft[0] *= xScale;
    unprojLeft[1] *= yScale;
    unprojLeft[2] -= 1+xTrans;
    unprojLeft[3] -= 1+yTrans;
    unprojLeft /= 2.0f;
    
    mBarrelDistortionShader->uniform("distortion", vec2(mDistortionCoefficients[0],
                                                        mDistortionCoefficients[1]));
    mBarrelDistortionShader->uniform("backgroundColor", vec4(mBackgroundColor.r,mBackgroundColor.g,mBackgroundColor.b,1.f));
    mBarrelDistortionShader->uniform("projectionLeft", projectionLeft);
    mBarrelDistortionShader->uniform("unprojectionLeft", unprojLeft);
    mBarrelDistortionShader->uniform("tex0", 0);
}

VertexDistorterRef Hmd::getVertexDistorter(){
    mDistorter = std::make_shared<VertexDistorter>(mInverseCoefficients);
    return mDistorter;
}

const mat4 Hmd::getProjectionMatrixLeftEye(bool distorted){
    auto fov = !distorted ? getDistortedFieldOfViewLeftEye() : getFieldOfViewLeftEye();
    float near = 0.1f;
    float far = 1000.f;
    float left = glm::tan(toRadians(fov[0])) * near;
    float right = glm::tan(toRadians(fov[1])) * near;
    float bottom = glm::tan(toRadians(fov[2])) * near;
    float top = glm::tan(toRadians(fov[3])) * near;
    // makeFrustum expects units in tan-angle space.
    return glm::frustum(-left, right, -bottom, top, near, far);
}

const std::array<float, 4> Hmd::getUndistortedViewportLeftEye(){
    auto p = getUndistortedParams();
    
    int scrWidth = toPixels(getWindowWidth());
    int scrHeight= toPixels(getWindowHeight());
    // Distances stored in local variables are in tan-angle units unless otherwise
    // noted.
    float eyeToScreenDistance = mScreenLensDistance;
    float screenWidth = mWidthMeters / eyeToScreenDistance;
    float screenHeight = mHeightMeters / eyeToScreenDistance;
    float xPxPerTanAngle = scrWidth / screenWidth;
    float yPxPerTanAngle = scrHeight / screenHeight;
    
    float x = glm::round((p[4] - p[0]) * xPxPerTanAngle);
    float y = glm::round((p[5] - p[3]) * yPxPerTanAngle);
    
    std::array< float, 4 > result;
    result[0] = x;
    result[1] = y;
    result[2] = glm::round((p[4]+p[1]) * xPxPerTanAngle)-x;
    result[3] = glm::round((p[5]+p[2]) * yPxPerTanAngle)-y;
    return result;
}

const std::array<float, 4> Hmd::getDistortedFieldOfViewLeftEye(){
    auto eyeToScreenDistance = mScreenLensDistance;
    
    float outerDist = (mWidthMeters - mInterLensDistance) / 2.f;
    float innerDist = mInterLensDistance / 2.f;
    float bottomDist = mBaselineLensDistance - mBevelMeters;
    float topDist = mHeightMeters - bottomDist;
    
    float outerAngle = toDegrees(glm::atan(distort(outerDist/eyeToScreenDistance)));
    float innerAngle = toDegrees(glm::atan(distort(innerDist/eyeToScreenDistance)));
    float bottomAngle = toDegrees(glm::atan(distort(bottomDist/eyeToScreenDistance)));
    float topAngle = toDegrees(glm::atan(distort(topDist/eyeToScreenDistance)));
    
    std::array< float, 4 > result;
    result[0] = glm::min(outerAngle, mFov);
    result[1] = glm::min(innerAngle, mFov);
    result[2] = glm::min(bottomAngle, mFov);
    result[3] = glm::min(topAngle, mFov);
    return result;
}

const std::array<float, 4> Hmd::getUndistortedFieldOfViewEye(){
    auto p = getUndistortedParams();
    std::array< float, 4 > result;
    //left degrees
    result[0] = toDegrees(glm::atan(p[0]));
    //right degrees
    result[1] = toDegrees(glm::atan(p[1]));
    //bottom degrees
    result[2] = toDegrees(glm::atan(p[3]));
    //top degrees
    result[3] = toDegrees(glm::atan(p[2]));
    return result;
}

const std::array<float, 12> Hmd::getDistortionCoefficients(){
    return mInverseCoefficients;
}

const std::array<float,6> Hmd::getUndistortedParams(){
    std::array< float, 6 > result;
    
    float eyeToScreenDistance = mScreenLensDistance;
    float halfLensDistance = mInterLensDistance / 2.f / eyeToScreenDistance;
    float screenWidth = mWidthMeters / eyeToScreenDistance;
    float screenHeight = mHeightMeters / eyeToScreenDistance;
    
    float eyePosX = screenWidth / 2.f - halfLensDistance;
    float eyePosY = (mBaselineLensDistance - mBevelMeters) / eyeToScreenDistance;
    
    float maxFov = mFov;
    float viewerMax = distortInverse(glm::tan(toRadians(maxFov)));
    
    float outerDist = glm::min(eyePosX, viewerMax);
    float innerDist = glm::min(halfLensDistance, viewerMax);
    float bottomDist = glm::min(eyePosY, viewerMax);
    float topDist = glm::min(screenHeight - eyePosY, viewerMax);
    
    result[0] = outerDist;
    result[1] = innerDist;
    result[2] = topDist;
    result[3] = bottomDist;
    result[4] = eyePosX;
    result[5] = eyePosY;
    
    return result;
}

const float Hmd::distortInverse(float radius){
    // Secant method.
    float r0 = radius / 0.9f;
    float r1 = radius * 0.9f;
    float dr0 = radius - distort(r0);
    while (glm::abs(r1 - r0) > 0.0001f ) {
        float dr1 = radius - distort(r1);
        float r2 = r1 - dr1 * ((r1 - r0) / (dr1 - dr0));
        r0 = r1;
        r1 = r2;
        dr0 = dr1;
    }
    return r1;
}

const float Hmd::distortionFactor(float radius){
    float result = 1.0f;
    float rFactor = 1.0f;
    float rSquared = radius * radius;
    size_t len = mDistortionCoefficients.size();
    
    for (size_t i = 0; i < len; i++) {
        float ki = mDistortionCoefficients[i];
        rFactor *= rSquared;
        result += ki * rFactor;
    }
    
    return result;
}

const float Hmd::distort(float radius){
    return radius * distortionFactor(radius);
}


const std::array<float,4> Hmd::getFieldOfViewLeftEye(){
    return getUndistortedFieldOfViewEye();
}

const std::array<float,4> Hmd::getFieldOfViewRightEye(){
    auto fov = getUndistortedFieldOfViewEye();
    float tmp = fov[0];
    fov[0] = fov[1];
    fov[1] = tmp;
    return fov;
}



