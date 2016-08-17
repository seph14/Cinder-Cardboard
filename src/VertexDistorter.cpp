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

#include "VertexDistorter.h"
#include "cinder/app/App.h"
#include "cinder/Utilities.h"

using namespace std;
using namespace ci;
using namespace ci::gl;
using namespace ci::cardboard;

static const string replaceMark = "#include \"VertexDistortion\"";

static const char* DistortionInclude =
"uniform float uDistortionCoefficients[12];\n"
"uniform float uDistortionMaxFovSquared;\n"
"uniform vec2  uDistortionFovOffset;\n"
"uniform vec2  uDistortionFovScale;\n"

// Returns a scalar to distort a point; computed in reverse via the polynomial approximation:
//   r' = 1 + Σ_i (uDistortionCoefficients[i] rSquared^(i+1))  i=[0..11]
// where rSquared is the squared radius of an undistorted point in tan-angle space.
// See {@link Distortion} for more information.
"float DistortionFactor(float rSquared) {\n"
"   float ret = 0.0;\n"
"   rSquared = min(uDistortionMaxFovSquared, rSquared);\n"
"   ret = rSquared * (ret + uDistortionCoefficients[11]);\n"
"   ret = rSquared * (ret + uDistortionCoefficients[10]);\n"
"   ret = rSquared * (ret + uDistortionCoefficients[9]);\n"
"   ret = rSquared * (ret + uDistortionCoefficients[8]);\n"
"   ret = rSquared * (ret + uDistortionCoefficients[7]);\n"
"   ret = rSquared * (ret + uDistortionCoefficients[6]);\n"
"   ret = rSquared * (ret + uDistortionCoefficients[5]);\n"
"   ret = rSquared * (ret + uDistortionCoefficients[4]);\n"
"   ret = rSquared * (ret + uDistortionCoefficients[3]);\n"
"   ret = rSquared * (ret + uDistortionCoefficients[2]);\n"
"   ret = rSquared * (ret + uDistortionCoefficients[1]);\n"
"   ret = rSquared * (ret + uDistortionCoefficients[0]);\n"
"   return ret + 1.0;\n"
"}\n"

// Given a point in clip space, distort the point according to the coefficients stored in
// uDistortionCoefficients and the field of view (FOV) specified in uDistortionFovOffset and
// uDistortionFovScale.
// Returns the distorted point in clip space, with its Z untouched.
"vec4 Distort(vec4 point) {\n"
// Put point into normalized device coordinates (NDC), [(-1, -1, -1) to (1, 1, 1)].
"   vec3 pointNdc = point.xyz / point.w;\n"
// Throw away the Z coordinate and map the point to the unit square, [(0, 0) to (1, 1)].
"   vec2 pointUnitSquare = (pointNdc.xy + vec2(1.0)) / 2.0;\n"
// Map the point into FOV tan-angle space.
"   vec2 pointTanAngle = pointUnitSquare * uDistortionFovScale - uDistortionFovOffset;\n"
"   float radiusSquared = dot(pointTanAngle, pointTanAngle);\n"
"   float distortionFactor = DistortionFactor(radiusSquared);\n"
//'float distortionFactor = 2.0;',
"   vec2 distortedPointTanAngle = pointTanAngle * distortionFactor;\n"
// Reverse the mappings above to bring the distorted point back into NDC space.
"   vec2 distortedPointUnitSquare = (distortedPointTanAngle + uDistortionFovOffset) / uDistortionFovScale;\n"
"   vec3 distortedPointNdc = vec3(distortedPointUnitSquare * 2.0 - vec2(1.0), pointNdc.z);\n"
// Convert the point into clip space before returning in case any operations are done after.
"   return vec4(distortedPointNdc, 1.0) * point.w;\n"
"}\n";

VertexDistorter::VertexDistorter(const std::array<float,12> &distortionCoefficients){
    mDistortionCoefficients = distortionCoefficients;
}

void VertexDistorter::updateWithFov(const std::array<float,4> &fov){
    float maxFov = hypotf(glm::tan(toRadians(glm::max(fov[0], fov[1]))),
                          glm::tan(toRadians(glm::max(fov[2], fov[3]))));
    mMaxFovSquared = maxFov * maxFov;
    
    mDistortionFovOffset.x = glm::tan(toRadians(fov[0]));
    mDistortionFovOffset.y = glm::tan(toRadians(fov[2]));
    
    float left = glm::tan(toRadians(fov[0]));
    float right = glm::tan(toRadians(fov[1]));
    float up = glm::tan(toRadians(fov[3]));
    float down = glm::tan(toRadians(fov[2]));
    mDistortionFovScale = vec2(left+right, up+down);
}

const ci::gl::GlslProgRef VertexDistorter::createDistortionShader(const ci::DataSourceRef &vert,
                                                                  const ci::DataSourceRef &frag){
    auto vertStr = injectVertexShaderWithDistortion(loadString(vert));
    auto fragStr = loadString(frag);
    return GlslProg::create(vertStr,fragStr);
}

const std::string VertexDistorter::injectVertexShaderWithDistortion(const std::string &vertShader){
    size_t length=replaceMark.size();
    size_t index = vertShader.find(replaceMark);
    return vertShader.substr(0,index) + (string)DistortionInclude + vertShader.substr(index+length);
}

const char* VertexDistorter::getDistortionInclude(){
    return DistortionInclude;
}

void VertexDistorter::setDistortionUniforms(ci::gl::GlslProgRef shader){
    shader->uniform("uDistortionMaxFovSquared", mMaxFovSquared);
    shader->uniform("uDistortionFovOffset", mDistortionFovOffset);
    shader->uniform("uDistortionFovScale", mDistortionFovScale);
    shader->uniform("uDistortionCoefficients", &mDistortionCoefficients[0], 12);
}
