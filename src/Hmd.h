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

#ifndef Hmd_h
#define Hmd_h

#include "cinder/gl/gl.h"
#include "cinder/gl/glslProg.h"
#include "cinder/Camera.h"
#include "cinder/gl/Fbo.h"

#include "Cardboard.h"
#include "VertexDistorter.h"

namespace cinder { namespace cardboard {
    
    class Hmd;
    using HmdRef = std::shared_ptr<Hmd>;
    
    class VertexDistorter;
    using VertexDistorterRef = std::shared_ptr<VertexDistorter>;
    
    class Hmd{
    public:
        Hmd(CardboardVersion version = VERSION_2, bool initVertexDistortion = true);
        Hmd(CardboardParams params, bool initVertexDistortion = true);
        ~Hmd();
        
        static HmdRef create(CardboardVersion version = VERSION_2, bool initVertexDistortion = true);
        
        void setDefaultDirection(float angle);
        bool hasDirectionApplied();
        
        void setBackgroundColor(const ci::Color &color) { mBackgroundColor = color;}
        
#if defined( CINDER_GL_ES )
        void updateCamera(const app::InterfaceOrientation &orientation);
#endif
        CameraStereo getCamera() { return mCamera; }
        
        void bindEye(Eye eye);
        void unbind();
        void render();
        
        const std::string getDeviceName(){ return mDeviceName; }
        
        VertexDistorterRef getVertexDistorter();
        
    protected:
        bool mMotionReady,mDesiredDirectionApplied;
        ci::vec3 mInitDirection;
        float mDefaultAngle, mDesiredDirection;
        int  mMotionCount;
        ci::CameraStereo mCamera;
        
        std::string mDeviceName;
        
        bool                mUseVertexDistorter;
        ci::Color           mBackgroundColor;
        VertexDistorterRef  mDistorter;
        ci::gl::GlslProgRef mBarrelDistortionShader;
        ci::gl::FboRef      mRenderBuffer;
        
        Eye   mCurrEye;
        float mWidthMeters;
        float mHeightMeters;
        float mBevelMeters;
        
        float mFov;
        float mInterLensDistance;
        float mBaselineLensDistance;
        float mScreenLensDistance;
        
        std::array<float,2>  mDistortionCoefficients;
        std::array<float,12> mInverseCoefficients;
        
        void init(CardboardParams params, bool initVertexDistortion);
        
        const vec4 projectionMatrixToVector(mat4 mat);
        void updateBarrelUniforms();
        const mat4 getProjectionMatrixLeftEye(bool distorted);
        
        const std::array<float, 4> getUndistortedFieldOfViewEye();
        const std::array<float, 4> getDistortedFieldOfViewLeftEye();
        const std::array<float, 4> getUndistortedViewportLeftEye();
        
        const std::array<float,6> getUndistortedParams();
        const std::array<float, 12> getDistortionCoefficients();
        
        const std::array<float,4> getFieldOfViewLeftEye();
        const std::array<float,4> getFieldOfViewRightEye();
        
        const float distortInverse(float radius);
        const float distortionFactor(float radius);
        const float distort(float radius);
    };
}}

#endif /* Hmd_h */
