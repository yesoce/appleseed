
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2010-2013 Francois Beaune, Jupiter Jazz Limited
// Copyright (c) 2014 Francois Beaune, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// Interface header.
#include "defaultproject.h"

// appleseed.renderer headers.
#include "renderer/modeling/camera/camera.h"
#include "renderer/modeling/camera/pinholecamera.h"
#include "renderer/modeling/frame/frame.h"
#include "renderer/modeling/project/project.h"
#include "renderer/modeling/scene/assembly.h"
#include "renderer/modeling/scene/assemblyinstance.h"
#include "renderer/modeling/scene/containers.h"
#include "renderer/modeling/scene/scene.h"
#include "renderer/utility/paramarray.h"

using namespace foundation;

namespace renderer
{

//
// DefaultProjectFactory class implementation.
//

auto_release_ptr<Project> DefaultProjectFactory::create()
{
    // Create a project.
    auto_release_ptr<Project> project(ProjectFactory::create("default"));

    // Add default configurations to the project.
    project->add_default_configurations();

    // Create a scene.
    auto_release_ptr<Scene> scene(SceneFactory::create());

    // Create an assembly.
    auto_release_ptr<Assembly> assembly(AssemblyFactory::create("assembly", ParamArray()));

    // Create an instance of the assembly and insert it into the scene.
    scene->assembly_instances().insert(
        AssemblyInstanceFactory::create(
            "assembly_inst",
            ParamArray(),
            "assembly"));

    // Insert the assembly into the scene.
    scene->assemblies().insert(assembly);

    //
    // Camera.
    //

    {
        // Create a pinhole camera.
        // Film dimensions are 0.980 in ?0.735 in (24.892 mm x 18.669 mm).
        // Reference: http://en.wikipedia.org/wiki/Aspect_ratio_(image).
        ParamArray params;
        params.insert("film_dimensions", "0.024892 0.018669");
        params.insert("focal_length", "0.035");
        auto_release_ptr<Camera> camera(PinholeCameraFactory().create("camera", params));

        // Attach the camera to the scene.
        scene->set_camera(camera);
    }

    //
    // Frame.
    //

    {
        // Create a frame.
        ParamArray params;
        params.insert("camera", scene->get_camera()->get_name());
        params.insert("resolution", "640 480");
        auto_release_ptr<Frame> frame(FrameFactory::create("beauty", params));

        // Attach the frame to the project.
        project->set_frame(frame);
    }

    // Attach the scene to the project.
    project->set_scene(scene);

    // Return the newly created project.
    return project;
}

}   // namespace renderer
