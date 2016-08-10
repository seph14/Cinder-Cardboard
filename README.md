# Cinder-Cardboard
use Cinder with Google Cardboard VR

####[Google Cardboard VRview](https://github.com/google/vrview) wrapped for use with Cinder 0.9.

This block is designed to quickly build/test out VR apps with Cinder for iOS. (Android helps are welcome.)
The block supports both vertex distortion and fragment distortion. For more information on these 2 different methods, please read: [VR DISTORTION CORRECTION USING VERTEX DISPLACEMENT](https://ustwo.com/blog/vr-distortion-correction-using-vertex-displacement)

#####Namespace
All code has been wrapped inside cinder::cardboard.

#####Usage
This block need to be used with the MotionManager block at the same time. Though if you only want to test the lens distortion, this block will run on desktop devices as well.


####User Vertex Distortion
![Image](/screenshots/vertex_distortion.png)

#####Initialization
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

#####Render
The benefit of using vertex distortion is it doesn't need an FBO, so you could do rendering by only 1 pass.
```c++
void VRApp:drawp(){
  mHmd->bindEye(Eye::LEFT);
  mDistorter->setDistortionUniforms(mShader); //this will set distortion uniforms
  drawScene(); //draw you objects
    
  mHmd->bindEye(Eye::RIGHT);
  mDistorter->setDistortionUniforms(mShader);
  drawScene();
    
  mHmd->unbind(); //unbind hmd to restore 2D viewport/matrices
}
```


####Use Fragment Distortion
![Image](/screenshots/fragment_distortion.png)

#####Initialization
```c++
void VRApp:setup(){
  mHmd = Hmd::create(CardboardVersion::VERSION_2, false);
}
```

#####Render scene
When using fragment distortion, you need to draw your scenes firstly into the fbo.
```c++
void VRApp:update(){
  //bing and draw for each eye
  mHmd->bindEye(Eye::LEFT);
  drawScene();
  mHmd->bindEye(Eye::RIGHT);
  drawScene();
  //unbind hmd so it unbind the fbo internally
  mHmd->unbind();
}
```
And then you need to draw the output fbo inside the draw function
```c++
void VRApp:draw(){
  mHmd->render();
}
```

####Note
Vertex and Fragment distortion usually yields different results. See the reading [VR DISTORTION CORRECTION USING VERTEX DISPLACEMENT](https://ustwo.com/blog/vr-distortion-correction-using-vertex-displacement), so this is uninteded that you mix 2 methods at the sametime. Therefore once the Hmd is created, you can not change the distortion method. 
