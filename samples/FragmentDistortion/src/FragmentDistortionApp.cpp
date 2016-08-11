#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Hmd.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ci::cardboard;

class FragmentDistortionApp : public App {
  public:
    double				mTime;
    
    gl::GlslProgRef		mShader;
    gl::GlslProgRef     mGridShader;
    gl::BatchRef		mCube, mCylinder, mTeapot, mTorus;
    gl::BatchRef        mWiredPlane;
    
    vec4				mLightWorldPosition;
    
    HmdRef              mHmd;
    
    static void prepareSettings( App::Settings *settings );
    
    void setup() override;
    void mouseDown( MouseEvent event ) override;
    void update() override;
    
    void drawScene();
    void draw() override;
};

void FragmentDistortionApp::setup()
{
    mHmd = Hmd::create(CardboardVersion::VERSION_2, false);
    
#if defined( CINDER_GL_ES )
    getSignalSupportedOrientations().connect( [] { return InterfaceOrientation::All; } );
    mShader = gl::GlslProg::create(loadAsset("phong_es2.vert"), loadAsset("phong_es2.frag"));
    mGridShader = gl::GlslProg::create(loadAsset("grid_es2.vert"), loadAsset("grid_es2.frag"));
#else
    mShader = gl::GlslProg::create(loadAsset("phong.vert"), loadAsset("phong.frag"));
    mGridShader = gl::GlslProg::create(loadAsset("grid.vert"), loadAsset("grid.frag"));
#endif
    
    mCube = gl::Batch::create( geom::Cube().subdivisions( 12 ), mShader );
    mCylinder = gl::Batch::create(geom::Cylinder().subdivisionsAxis(6).subdivisionsHeight(12).height(0.6f).radius(0.2f), mShader);
    mTeapot = gl::Batch::create(geom::Teapot(), mShader);
    mTorus = gl::Batch::create(geom::Torus().subdivisionsHeight(12).subdivisionsAxis(6), mShader);
    mWiredPlane = gl::Batch::create(geom::WirePlane().subdivisions(vec2(16)).size(vec2(6.f)), mGridShader);
}

void FragmentDistortionApp::mouseDown( MouseEvent event )
{
}

void FragmentDistortionApp::update()
{
    mTime = getElapsedSeconds();
    float t = float( mTime ) * 0.4f;
    mLightWorldPosition = vec4( math<float>::sin( t ), math<float>::sin( t * 4.0f ), math<float>::cos( t ), 1 );
    
#if defined( CINDER_GL_ES )
    mHmd->updateCamera(getOrientation());
#endif
    
    {//render contents into render buffer
        gl::enableDepthRead();
        gl::enableDepthWrite();
    
        mShader->uniform("uLightPosition",mLightWorldPosition);
        mShader->uniform("uSkyDirection", vec4(0,1,0,0));
    
        mHmd->bindEye(Eye::LEFT);
        drawScene();
        mHmd->bindEye(Eye::RIGHT);
        drawScene();
    
        mHmd->unbind();
    }
}

void FragmentDistortionApp::drawScene()
{
    {
        gl::ScopedModelMatrix push;
        gl::translate(vec3(0,0,-2.f));
        gl::rotate( (float)mTime, vec3( -0.3f, -1.0f, 0.2f ) );
        mCube->draw();
    }
    
    {
        gl::ScopedModelMatrix push;
        gl::translate(vec3(0,0,+2.f));
        gl::rotate( (float)mTime, vec3( -0.3f, -1.0f, 0.2f ) );
        mCylinder->draw();
    }
    
    {
        gl::ScopedModelMatrix push;
        gl::translate(vec3(+2.f,0,0));
        gl::rotate( (float)mTime, vec3( -0.3f, -1.0f, 0.2f ) );
        mTorus->draw();
    }
    
    {
        gl::ScopedModelMatrix push;
        gl::translate(vec3(-2.f,0,0));
        gl::rotate( (float)mTime, vec3( -0.3f, -1.0f, 0.2f ) );
        mTeapot->draw();
    }
    
    {
        gl::ScopedModelMatrix push;
        gl::translate(vec3(0,0,-4.f));
        gl::rotate(toRadians(90.f), vec3(1,0,0));
        mWiredPlane->draw();
    }
}

void FragmentDistortionApp::draw()
{
    gl::clear(Color(0,0,0));
    
    //let hmd render the buffer with lens distortion
    mHmd->render();
    
    gl::disableDepthRead();
    gl::disableDepthWrite();
    gl::color( Color::white() );
    auto x = toPixels(getWindowWidth())/2.f;
    gl::drawLine(vec2(x,0), vec2(x,toPixels(getWindowHeight())));
}

#if defined( CINDER_GL_ES )
void FragmentDistortionApp::prepareSettings( App::Settings *settings )
{
    settings->setPowerManagementEnabled(false);
    settings->setHighDensityDisplayEnabled(true);
}
CINDER_APP( FragmentDistortionApp, RendererGl( RendererGl::Options().msaa(0) ), FragmentDistortionApp::prepareSettings )
#else
void FragmentDistortionApp::prepareSettings( App::Settings *settings )
{
    settings->setTitle( "Cinder Cardboard" );
    settings->setWindowSize( 1136, 640 );
}

CINDER_APP( FragmentDistortionApp, RendererGl( RendererGl::Options().msaa(0) ), FragmentDistortionApp::prepareSettings )
#endif

