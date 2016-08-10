# Cinder-Cardboard
use Cinder with Google Cardboard VR

####[Google Cardboard VRview](https://github.com/google/vrview) wrapped for use with Cinder 0.9.

This block is designed to quickly build/test out VR apps with Cinder for iOS. (Android helps are welcome.)
The block supports both vertex distortion and fragment distortion. For more information on these 2 different methods, please read: [VR DISTORTION CORRECTION USING VERTEX DISPLACEMENT](https://ustwo.com/blog/vr-distortion-correction-using-vertex-displacement)

#####Namespace
All code has been wrapped inside cinder::cardboard.

#####Usage
This block need to be used with the MotionManager block at the same time. Though if you only want to test the lens distortion, this block will run on desktop devices as well.

#####Initialization
If you want to use vertex distorter:
```c++
void VRApp:setup(){
  mHmd = Hmd::create(CardboardVersion::VERSION_2, true);
  mDistorter = mHmd->getVertexDistorter();
  
  //generate shaders using VertexDistorter's helper function
  mShader = mDistorter->createDistortionShader(loadAsset("phong.vert"), loadAsset("phong.frag"));
}
```
In your vertex shader, you need to add this include
```glsl
#include "VertexDistortion"
```
And before you output gl_Position, use this function
```glsl
gl_Position = Distort(position);
```
to distort your vertex positions. 

If you want to use fragment distortions:
```c++
void VRApp:setup(){
  mHmd = Hmd::create(CardboardVersion::VERSION_2, false);
}
```



