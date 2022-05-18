#pragma once

#include "SourcePool.h"
#include <AL/alc.h>
#include <glm/vec3.hpp>

using Vec3f = glm::vec3;

namespace sound {

class AudioContext {
public:
	AudioContext();
	~AudioContext();

	bool Create(ALCdevice* device);
	bool Destroy();

	bool MakeCurrentPlaying();

	void GetLocation(Vec3f& loc);
	void GetOrientation(Vec3f& at, Vec3f& up);
	float GetVolume();

	//void SetAttunation(int key);
	void SetLocation(const Vec3f& location);

	void SetOrientation(const Vec3f& at, const Vec3f& up);/*const float& atx, const float& aty, const float& atz, const float& upx, const float& upy, const float& upz*/
	void SetVolume(const float& val);

	Source* CreateSource();
	void DestroySource(Source* source);

private:
	SourcePool* pool_;
	ALCcontext* context_;
};

} //namespace sound
