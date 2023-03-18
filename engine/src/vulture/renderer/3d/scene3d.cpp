/**
 * @author Nikita Mochalov (github.com/tralf-strues)
 * @file scene3d.cpp
 * @date 2022-05-01
 *
 * The MIT License (MIT)
 * Copyright (c) vulture-project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <vulture/renderer/3d/scene3d.hpp>

using namespace vulture;

Scene3D::~Scene3D() {
  for (auto camera : cameras_) {
    delete camera;
  }

  for (auto light_source : light_sources_) {
    delete light_source;
  }

  for (auto mesh : meshes_) {
    delete mesh;
  }
}

void Scene3D::SetMainCamera(CameraNode3D* camera) { main_camera_ = camera; }
CameraNode3D* Scene3D::GetMainCamera() { return main_camera_; }

void Scene3D::AddCamera(CameraNode3D* camera) { cameras_.push_back(camera); }
void Scene3D::RemoveCamera(CameraNode3D* camera) { cameras_.remove(camera); }
const std::list<CameraNode3D*>& Scene3D::GetCameras() const { return cameras_; }

void Scene3D::AddLightSource(LightSourceNode3D* light_source) { light_sources_.push_back(light_source); }
void Scene3D::RemoveLightSource(LightSourceNode3D* light_source) { light_sources_.remove(light_source); }
const std::list<LightSourceNode3D*>& Scene3D::GetLightSources() const { return light_sources_; }

void Scene3D::AddMesh(MeshNode3D* mesh) { meshes_.push_back(mesh); }
void Scene3D::RemoveMesh(MeshNode3D* mesh) { meshes_.remove(mesh); }
const std::list<MeshNode3D*>& Scene3D::GetMeshes() const { return meshes_; }