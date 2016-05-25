#include "VDWarp.h"

#include "cinder/gl/Texture.h"
#include "cinder/Xml.h"

using namespace ci;
using namespace ci::app;

namespace VideoDromm {
/*	VDWarp::VDWarp(MixType aType)
		: mFbosPath("fbos.xml")
		, mName("")
		, mFlipV(false)
		, mFlipH(false)
		, mWidth(640)
		, mHeight(480)
	{
		// Settings
		mVDSettings = VDSettings::create();
		mFbosFilepath = getAssetPath("") / mFbosPath;
		if (fs::exists(mFbosFilepath)) {
			// load textures from file if one exists
			mFbos = VDFbo::readSettings(loadFile(mFbosFilepath));
		}
		else {
			mFbos.push_back(VDFbo::create());
		}

		if (mName.length() == 0) {
			mName = mFbosPath;
		}
		mPosX = mPosY = 0.0f;
		mZoom = 1.0f;
		// init the fbo whatever happens next
		gl::Fbo::Format format;
		//format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
		mMixFbo = gl::Fbo::create(mWidth, mHeight, format.depthTexture());
		mLeftFbo = gl::Fbo::create(mWidth, mHeight, format.depthTexture());
		mRightFbo = gl::Fbo::create(mWidth, mHeight, format.depthTexture());
		// init with passthru shader
		mMixShaderName = "mixshader";
		// load shadertoy uniform variables declarations
		shaderInclude = loadString(loadAsset("shadertoy.inc"));
		try
		{
			fs::path vertexFile = getAssetPath("") / "passthru.vert";
			if (fs::exists(vertexFile)) {
				mPassthruVextexShaderString = loadString(loadAsset("passthru.vert"));
				CI_LOG_V("passthru.vert loaded");
			}
			else
			{
				CI_LOG_V("passthru.vert does not exist, should quit");
			}
		}
		catch (gl::GlslProgCompileExc &exc)
		{
			mError = string(exc.what());
			CI_LOG_V("unable to load/compile passthru vertex shader:" + string(exc.what()));
		}
		catch (const std::exception &e)
		{
			mError = string(e.what());
			CI_LOG_V("unable to load passthru vertex shader:" + string(e.what()));
		}
		// load passthru fragment shader
		try
		{
			fs::path fragFile = getAssetPath("") / "mixfbo.frag";
			if (fs::exists(fragFile)) {
				mMixFragmentShaderString = loadString(loadAsset("mixfbo.frag"));
			}
			else
			{
				mError = "mixfbo.frag does not exist, should quit";
				CI_LOG_V(mError);
			}
			mMixShader = gl::GlslProg::create(mPassthruVextexShaderString, mMixFragmentShaderString);
			mMixShader->setLabel(mMixShaderName);
			CI_LOG_V("mixfbo.frag loaded and compiled");
		}
		catch (gl::GlslProgCompileExc &exc)
		{
			mError = string(exc.what());
			CI_LOG_V("unable to load/compile mixfbo fragment shader:" + string(exc.what()));
		}
		catch (const std::exception &e)
		{
			mError = string(e.what());
			CI_LOG_V("unable to load mixfbo fragment shader:" + string(e.what()));
		}
	}
	VDWarp::~VDWarp(void) {

	}
	int VDWarp::loadFboFragmentShader(string aFilePath, bool right)
	{
		return mFbos[0]->loadPixelFragmentShader(aFilePath);// TODO right or left
	}

	VDWarpList VDWarp::readSettings(const DataSourceRef &source)
	{
		XmlTree			doc;
		VDWarpList	VDWarplist;

		// try to load the specified xml file
		try { doc = XmlTree(source); }
		catch (...) { return VDWarplist; }

		// check if this is a valid file 
		bool isOK = doc.hasChild("mixes");
		if (!isOK) return VDWarplist;

		//
		if (isOK) {

			XmlTree mixXml = doc.getChild("mixes");

			// iterate textures
			for (XmlTree::ConstIter child = mixXml.begin("mix"); child != mixXml.end(); ++child) {
				// create mix of the correct type
				std::string mixtype = child->getAttributeValue<std::string>("mixtype", "mix");
				XmlTree detailsXml = child->getChild("details");

				if (mixtype == "mix") {
					VDWarpRef t(new VDWarp());
					t->fromXml(detailsXml);
					VDWarplist.push_back(t);
				}

			}
		}

		return VDWarplist;
	}

	void VDWarp::writeSettings(const VDWarpList &VDWarplist, const ci::DataTargetRef &target) {

		// create config document and root <textures>
		XmlTree			doc;
		doc.setTag("mixes");
		doc.setAttribute("version", "1.0");

		// 
		for (unsigned int i = 0; i < VDWarplist.size(); ++i) {
			// create <texture>
			XmlTree			mix;
			mix.setTag("mix");
			mix.setAttribute("id", i + 1);
			switch (VDWarplist[i]->mType) {
			case MIX: mix.setAttribute("mixtype", "mix"); break;
			default: mix.setAttribute("mixtype", "unknown"); break;
			}
			// details specific to texture type
			mix.push_back(VDWarplist[i]->toXml());

			// add fbo to doc
			doc.push_back(mix);
		}

		// write file
		doc.write(target);
	}
	XmlTree	VDWarp::toXml() const
	{
		XmlTree		xml;
		xml.setTag("details");
		xml.setAttribute("fbopath", mFbosPath);
		xml.setAttribute("width", mWidth);
		xml.setAttribute("height", mHeight);

		return xml;
	}

	void VDWarp::fromXml(const XmlTree &xml)
	{
		mType = MIX;
		// retrieve fbos specific to this mixfbo
		mFbosPath = xml.getAttributeValue<string>("fbopath", "fbos.xml");
		if (mFbosPath.length() > 0) {
			fs::path fboSettingsPath = getAssetPath("") / mFbosPath;// TODO / mVDSettings->mAssetsPath
			try {
				VDFbo::readSettings(loadFile(fboSettingsPath));
				CI_LOG_V("successfully loaded " + mFbosPath);
			}
			catch (Exception &exc) {
				CI_LOG_EXCEPTION("error loading ", exc);
			}
		}

	}
	void VDWarp::setPosition(int x, int y) {
		mPosX = ((float)x / (float)mWidth) - 0.5;
		mPosY = ((float)y / (float)mHeight) - 0.5;
		for (auto &fbo : mFbos)
		{
			fbo->setPosition(mPosX, mPosY);
		}
	}
	void VDWarp::setZoom(float aZoom) {
		mZoom = aZoom;
		for (auto &fbo : mFbos)
		{
			fbo->setZoom(mZoom);
		}
	}
	int VDWarp::getTextureWidth() {
		return mWidth;
	};

	int VDWarp::getTextureHeight() {
		return mHeight;
	};
	unsigned int VDWarp::getInputTexturesCount(unsigned int aFboIndex) {
		if (aFboIndex > mFbos.size() - 1) aFboIndex = mFbos.size() - 1;
		return mFbos[aFboIndex]->getInputTexturesCount();
	}
	string VDWarp::getInputTextureName(unsigned int aFboIndex, unsigned int aTextureIndex) {
		if (aFboIndex > mFbos.size() - 1) aFboIndex = mFbos.size() - 1;
		return mFbos[aFboIndex]->getInputTextureName(aTextureIndex);
	}
	string VDWarp::getFboName(unsigned int aFboIndex) {
		if (aFboIndex > mFbos.size() - 1) aFboIndex = mFbos.size() - 1;
		return mFbos[aFboIndex]->getName();
	}
	int VDWarp::getFboTextureWidth(unsigned int aFboIndex) {
		if (aFboIndex > mFbos.size() - 1) aFboIndex = mFbos.size() - 1;
		return mFbos[aFboIndex]->getTextureWidth();
	};

	int VDWarp::getFboTextureHeight(unsigned int aFboIndex) {
		if (aFboIndex > mFbos.size() - 1) aFboIndex = mFbos.size() - 1;
		return mFbos[aFboIndex]->getTextureHeight();
	};

	ci::ivec2 VDWarp::getSize() {
		return mMixFbo->getSize();
	}

	ci::Area VDWarp::getBounds() {
		return mMixFbo->getBounds();
	}

	GLuint VDWarp::getId() {
		return mMixFbo->getId();
	}

	std::string VDWarp::getName(){
		return mName;
	}
	// Render left FBO
	void VDWarp::renderLeftFbo()
	{
		gl::ScopedFramebuffer fbScp(mLeftFbo);
		// clear out the FBO with blue
		gl::clear(Color(0.25, 0.5f, 1.0f));

		// setup the viewport to match the dimensions of the FBO
		gl::ScopedViewport scpVp(ivec2(30), mLeftFbo->getSize());

		// render the left fbo
		gl::ScopedGlslProg shaderScp(gl::getStockShader(gl::ShaderDef().texture()));
		gl::ScopedTextureBind tex(mFbos[0]->getTexture());
		gl::drawSolidRect(Rectf(0, 0, mWidth, mHeight));
	}
	// Render left FBO
	void VDWarp::renderRightFbo()
	{
		gl::ScopedFramebuffer fbScp(mRightFbo);
		// clear out the FBO with red
		gl::clear(Color(1.0, 0.0f, 0.0f));

		// setup the viewport to match the dimensions of the FBO
		gl::ScopedViewport scpVp(ivec2(30), mRightFbo->getSize());

		// render the right fbo
		gl::ScopedGlslProg shaderScp(gl::getStockShader(gl::ShaderDef().texture()));
		if (mFbos.size() > 1) {
			gl::ScopedTextureBind tex(mFbos[1]->getTexture());

		}
		else {
			CI_LOG_W("renderRightFbo: only one fbo, right renders left...");
			gl::ScopedTextureBind tex(mFbos[0]->getTexture());

		}
		gl::drawSolidRect(Rectf(0, 0, mWidth, mHeight));
	}
	ci::gl::TextureRef VDWarp::getRightFboTexture() {
		return mLeftFbo->getColorTexture();
	}
	ci::gl::TextureRef VDWarp::getLeftFboTexture() {
		return mRightFbo->getColorTexture();
	}
	void VDWarp::loadImageFile(string aFile, unsigned int aFboIndex, unsigned int aTextureIndex, bool right) {
		if (aFboIndex > mFbos.size() - 1) aFboIndex = mFbos.size() - 1;
		mFbos[aFboIndex]->loadImageFile(aFile, aTextureIndex);

	}

	ci::gl::Texture2dRef VDWarp::getFboTexture(unsigned int aFboIndex) {
		if (aFboIndex > mFbos.size() - 1) aFboIndex = mFbos.size() - 1;
		return mFbos[aFboIndex]->getTexture();
	}
	ci::gl::Texture2dRef VDWarp::getFboInputTexture(unsigned int aFboIndex, unsigned int aFboInputTextureIndex) {
		if (aFboIndex > mFbos.size() - 1) aFboIndex = mFbos.size() - 1;
		return mFbos[aFboIndex]->getInputTexture(aFboInputTextureIndex);
	}
	void VDWarp::setCrossfade(float aCrossfade) {
		mVDAnimation->controlValues[21] = aCrossfade;
	}
	ci::gl::TextureRef VDWarp::getTexture() {
		renderLeftFbo();
		renderRightFbo();
		iChannelResolution0 = vec3(mPosX, mPosY, 0.5);
		gl::ScopedFramebuffer fbScp(mMixFbo);
		gl::clear(Color::white());
		// setup the viewport to match the dimensions of the FBO
		gl::ScopedViewport scpVp(ivec2(5), mMixFbo->getSize());
		gl::ScopedGlslProg shaderScp(mMixShader);
		mRightFbo->bindTexture(0);
		mLeftFbo->bindTexture(1);

		mMixShader->uniform("iGlobalTime", mVDSettings->iGlobalTime);
		mMixShader->uniform("iResolution", vec3(mWidth, mHeight, 1.0));
		mMixShader->uniform("iChannelResolution[0]", iChannelResolution0);
		mMixShader->uniform("iMouse", vec4(mVDSettings->mRenderPosXY.x, mVDSettings->mRenderPosXY.y, mVDSettings->iMouse.z, mVDSettings->iMouse.z));//iMouse =  Vec3i( event.getX(), mRenderHeight - event.getY(), 1 );
		mMixShader->uniform("iZoom", mZoom);
		mMixShader->uniform("iChannel0", 0);
		mMixShader->uniform("iChannel1", 1);
		mMixShader->uniform("iAudio0", 0);
		mMixShader->uniform("iFreq0", mVDAnimation->iFreqs[0]);
		mMixShader->uniform("iFreq1", mVDAnimation->iFreqs[1]);
		mMixShader->uniform("iFreq2", mVDAnimation->iFreqs[2]);
		mMixShader->uniform("iFreq3", mVDAnimation->iFreqs[3]);
		mMixShader->uniform("iChannelTime", mVDSettings->iChannelTime, 4);
		mMixShader->uniform("iColor", vec3(mVDAnimation->controlValues[1], mVDAnimation->controlValues[2], mVDAnimation->controlValues[3]));// mVDSettings->iColor);
		mMixShader->uniform("iBackgroundColor", vec3(mVDAnimation->controlValues[5], mVDAnimation->controlValues[6], mVDAnimation->controlValues[7]));// mVDSettings->iBackgroundColor);
		mMixShader->uniform("iSteps", (int)mVDAnimation->controlValues[20]);
		mMixShader->uniform("iRatio", mVDAnimation->controlValues[11]);//check if needed: +1;//mVDSettings->iRatio); 
		mMixShader->uniform("width", 1);
		mMixShader->uniform("height", 1);
		mMixShader->uniform("iRenderXY", vec2(0.0, 0.0));
		mMixShader->uniform("iAlpha", mVDAnimation->controlValues[4]);
		mMixShader->uniform("iBlendmode", mVDSettings->iBlendMode);
		mMixShader->uniform("iRotationSpeed", mVDAnimation->controlValues[19]);
		mMixShader->uniform("iCrossfade", mVDAnimation->controlValues[21]);
		mMixShader->uniform("iPixelate", mVDAnimation->controlValues[15]);
		mMixShader->uniform("iExposure", mVDAnimation->controlValues[14]);
		mMixShader->uniform("iFade", (int)mVDSettings->iFade);
		mMixShader->uniform("iToggle", (int)mVDAnimation->controlValues[46]);
		mMixShader->uniform("iLight", (int)mVDSettings->iLight);
		mMixShader->uniform("iLightAuto", (int)mVDSettings->iLightAuto);
		mMixShader->uniform("iGreyScale", (int)mVDSettings->iGreyScale);
		mMixShader->uniform("iTransition", mVDSettings->iTransition);
		mMixShader->uniform("iAnim", mVDSettings->iAnim.value());
		mMixShader->uniform("iRepeat", (int)mVDSettings->iRepeat);
		mMixShader->uniform("iVignette", (int)mVDAnimation->controlValues[47]);
		mMixShader->uniform("iInvert", (int)mVDAnimation->controlValues[48]);
		mMixShader->uniform("iDebug", (int)mVDSettings->iDebug);
		mMixShader->uniform("iShowFps", (int)mVDSettings->iShowFps);
		mMixShader->uniform("iFps", mVDSettings->iFps);
		// TODO mMixShader->uniform("iDeltaTime", mVDAnimation->iDeltaTime);
		// TODO mMixShader->uniform("iTempoTime", mVDAnimation->iTempoTime);
		mMixShader->uniform("iTempoTime", 1.0f);
		mMixShader->uniform("iGlitch", (int)mVDAnimation->controlValues[45]);
		mMixShader->uniform("iBeat", mVDSettings->iBeat);
		mMixShader->uniform("iSeed", mVDSettings->iSeed);
		mMixShader->uniform("iFlipH", mFlipH);
		mMixShader->uniform("iFlipV", mFlipV);


		mMixShader->uniform("iTrixels", mVDAnimation->controlValues[16]);
		mMixShader->uniform("iGridSize", mVDAnimation->controlValues[17]);
		mMixShader->uniform("iRedMultiplier", mVDSettings->iRedMultiplier);
		mMixShader->uniform("iGreenMultiplier", mVDSettings->iGreenMultiplier);
		mMixShader->uniform("iBlueMultiplier", mVDSettings->iBlueMultiplier);
		mMixShader->uniform("iParam1", mVDSettings->iParam1);
		mMixShader->uniform("iParam2", mVDSettings->iParam2);
		mMixShader->uniform("iXorY", mVDSettings->iXorY);
		mMixShader->uniform("iBadTv", mVDSettings->iBadTv);

		gl::drawSolidRect(Rectf(0, 0, mWidth, mHeight));
		return mMixFbo->getColorTexture();
	}*/
} // namespace VideoDromm
