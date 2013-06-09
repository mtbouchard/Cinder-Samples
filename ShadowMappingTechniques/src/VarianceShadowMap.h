#pragma once

#include "ShadowMap.h"

class VarianceShadowMap : public ShadowMap
{
public:
	VarianceShadowMap(void);
	~VarianceShadowMap(void);

	void	setup();
	void	update(  const ci::CameraPersp &cam );

	void	bindDepth();
	void	unbindDepth();

	void	bindShadow();
	void	unbindShadow();
private:
	std::string	getDepthVS() const;
	std::string	getDepthFS() const;
	
	std::string	getShadowVS() const;
	std::string	getShadowFS() const;
};

