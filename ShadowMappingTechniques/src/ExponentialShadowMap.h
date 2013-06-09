#pragma once

#include "ShadowMap.h"

class ExponentialShadowMap : public ShadowMap
{
public:
	ExponentialShadowMap(void) {}
	~ExponentialShadowMap(void) {}

	void	setup();
	void	update(  const ci::CameraPersp &cam );

	void	bindDepth();
	void	unbindDepth();

	void	bindShadow();
	void	unbindShadow();
private:
	std::string	getDepthVS() const;
	std::string	getDepthFS() const;
	
	std::string	getBlurVS() const;
	std::string	getBlurFS() const;
	
	std::string	getShadowVS() const;
	std::string	getShadowFS() const;
};

