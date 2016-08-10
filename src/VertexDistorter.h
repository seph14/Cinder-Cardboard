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

#ifndef VertexDistorter_h
#define VertexDistorter_h

#include "cinder/gl/gl.h"
#include "cinder/gl/glslProg.h"
#include "cinder/Camera.h"

#include "Cardboard.h"
#include "Hmd.h"

namespace cinder { namespace cardboard {

class Hmd;
    
class VertexDistorter{
protected:
    std::array<float,12> mDistortionCoefficients;
    float mMaxFovSquared;
    ci::vec2 mDistortionFovOffset;
    ci::vec2 mDistortionFovScale;
    
public:
    VertexDistorter(std::array<float,12> distortionCoefficients);
    
    void updateWithFov(std::array<float,4> fov);
    
    const ci::gl::GlslProgRef createDistortionShader(const ci::DataSourceRef &vert,
                                                     const ci::DataSourceRef &frag);
    const std::string injectVertexShaderWithDistortion(const std::string &vertShader);
    const char* getDistortionInclude();
    void setDistortionUniforms(ci::gl::GlslProgRef shader);
};

}}
#endif /* VertexDistorter_h */
