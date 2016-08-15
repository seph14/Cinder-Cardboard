#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Hmd.h"
#include "VertexDistorter.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ci::cardboard;

class VertexDistortionApp : public App {
  public:
    double				mTime;
    
    gl::GlslProgRef		mShader;
    gl::GlslProgRef     mGridShader;
    gl::BatchRef		mCube, mCylinder, mTeapot, mTorus;
    gl::BatchRef        mWiredPlane;
    
    vec4				mLightWorldPosition;
    
    HmdRef              mHmd;
    VertexDistorterRef  mDistorter;
    
    static void prepareSettings( App::Settings *settings );
    
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
    
    void drawScene();
	void draw() override;
};

void VertexDistortionApp::setup()
{
    mHmd = Hmd::create(CardboardVersion::VERSION_2, true);
    mDistorter = mHmd->getVertexDistorter();
    
#if defined( CINDER_GL_ES )
    getSignalSupportedOrientations().connect( [] { return InterfaceOrientation::All; } );
    mShader = mDistorter->createDistortionShader(loadAsset("phongDistort_es2.vert"), loadAsset("phongDistort_es2.frag"));
    mGridShader = mDistorter->createDistortionShader(loadAsset("gridDistort_es2.vert"), loadAsset("gridDistort_es2.frag"));
#else
    mShader = mDistorter->createDistortionShader(loadAsset("phongDistort.vert"), loadAsset("phongDistort.frag"));
    mGridShader = mDistorter->createDistortionShader(loadAsset("gridDistort.vert"), loadAsset("gridDistort.frag"));
#endif
    
    mCube = gl::Batch::create( geom::Cube().subdivisions( 12 ), mShader );
    mCylinder = gl::Batch::create(geom::Cylinder().subdivisionsAxis(6).subdivisionsHeight(12).height(0.6f).radius(0.2f), mShader);
    mTeapot = gl::Batch::create(geom::Teapot(), mShader);
    mTorus = gl::Batch::create(geom::Torus().subdivisionsHeight(12).subdivisionsAxis(6), mShader);
    mWiredPlane = gl::Batch::create(geom::WirePlane().subdivisions(vec2(16)).size(vec2(6.f)), mGridShader);
    
    mHmd->setDefaultDirection(90.f);
}

void VertexDistortionApp::mouseDown( MouseEvent event )
{
}

void VertexDistortionApp::update()
{
    mTime = getElapsedSeconds();
    float t = float( mTime ) * 0.4f;
    mLightWorldPosition = vec4( math<float>::sin( t ), math<float>::sin( t * 4.0f ), math<float>::cos( t ), 1 );
    
#if defined( CINDER_GL_ES )
    mHmd->updateCamera(getOrientation());
#endif
}

void VertexDistortionApp::drawScene()
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

void VertexDistortionApp::draw()
{
	gl::clear(Color(0,0,0));
    
    mShader->uniform("uLightPosition",mLightWorldPosition);
    mShader->uniform("uSkyDirection", vec4(0,1,0,0));
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    mHmd->bindEye(Eye::LEFT);
    //this will set the distortion uniforms for you
    mDistorter->setDistortionUniforms(mShader);
    mDistorter->setDistortionUniforms(mGridShader);
    drawScene();
    
    mHmd->bindEye(Eye::RIGHT);
    mDistorter->setDistortionUniforms(mShader);
    mDistorter->setDistortionUniforms(mGridShader);
    drawScene();
    
    mHmd->unbind();
    
    gl::disableDepthRead();
    gl::disableDepthWrite();
    gl::color( Color::white() );
    auto x = toPixels(getWindowWidth())/2.f;
    gl::drawLine(vec2(x,0), vec2(x,toPixels(getWindowHeight())));
}

#if defined( CINDER_GL_ES )
void VertexDistortionApp::prepareSettings( App::Settings *settings )
{
    settings->setPowerManagementEnabled(false);
    settings->setHighDensityDisplayEnabled(true);
}
CINDER_APP( VertexDistortionApp, RendererGl( RendererGl::Options().msaa(0) ), VertexDistortionApp::prepareSettings )
#else
void VertexDistortionApp::prepareSettings( App::Settings *settings )
{
    settings->setTitle( "Cinder Cardboard" );
    settings->setWindowSize( 1136, 640 );
}

CINDER_APP( VertexDistortionApp, RendererGl( RendererGl::Options().msaa(0) ), VertexDistortionApp::prepareSettings )
#endif

