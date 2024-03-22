# Ray Tracer - Photorealistic Rendering

Ray Tracer which supports 4 BRDFs, implict/explicit shape rendering, advanced lighting, texture mapping,anti-aliasing, super-sampling,  filtering, depth of field, soft shadow,  acceleration, parallelization, etc. This project associates with Brown CSCI 2230 Computer Graphics course and all project handouts can be found [here](https://browncsci1230.github.io/projects).

## Part 1: Ray Tracing and BRDFs

The handout for this part can be found [here](https://cs1230.graphics/projects/ray/1).

### Output Comparison
Run the program with the specified `.ini` file to compare your output (it should automatically save to the correct path).
> If your program can't find certain files or you aren't seeing your output images appear, make sure to:<br/>
> 1. Set your working directory to the project directory
> 2. Set the command-line argument in Qt Creator to `template_inis/intersect/<ini_file_name>.ini`
> 3. Clone the `scenefiles` submodule. If you forgot to do this when initially cloning this repository, run `git submodule update --init --recursive` in the project directory

<!-- > Note: once all images are filled in, the images will be the same size in the expected and student outputs. -->

| File/Method To Produce Output | Expected Output | Your Output |
| :---------------------------------------: | :--------------------------------------------------: | :-------------------------------------------------: |
| unit_cone.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/intersect/required_outputs/unit_cone.png) | ![Place unit_cone.png in student_outputs/intersect/required folder](student_outputs/intersect/required/unit_cone.png) |
| unit_cone_cap.ini | ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/intersect/required_outputs/unit_cone_cap.png) | ![Place unit_cone_cap.png in student_outputs/intersect/required folder](student_outputs/intersect/required/unit_cone_cap.png) |
| unit_cube.ini | ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/intersect/required_outputs/unit_cube.png) | ![Place unit_cube.png in student_outputs/intersect/required folder](student_outputs/intersect/required/unit_cube.png) |
| unit_cylinder.ini | ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/intersect/required_outputs/unit_cylinder.png) | ![Place unit_cylinder.png in student_outputs/intersect/required folder](student_outputs/intersect/required/unit_cylinder.png) |
| unit_sphere.ini | ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/intersect/required_outputs/unit_sphere.png) | ![Place unit_sphere.png in student_outputs/intersect/required folder](student_outputs/intersect/required/unit_sphere.png) |
| parse_matrix.ini | ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/intersect/required_outputs/parse_matrix.png) | ![Place parse_matrix.png in student_outputs/intersect/required folder](student_outputs/intersect/required/parse_matrix.png) |
| ambient_total.ini | ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/intersect/required_outputs/ambient_total.png) | ![Place ambient_total.png in student_outputs/intersect/required folder](student_outputs/intersect/required/ambient_total.png) |
| diffuse_total.ini | ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/intersect/required_outputs/diffuse_total.png) | ![Place diffuse_total.png in student_outputs/intersect/required folder](student_outputs/intersect/required/diffuse_total.png) |
| specular_total.ini | ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/intersect/required_outputs/specular_total.png) | ![Place specular_total.png in student_outputs/intersect/required folder](student_outputs/intersect/required/specular_total.png) |
| phong_total.ini | ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/intersect/required_outputs/phong_total.png) | ![Place phong_total.png in student_outputs/intersect/required folder](student_outputs/intersect/required/phong_total.png) |
| directional_light_1.ini | ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/intersect/required_outputs/directional_light_1.png) | ![Place directional_light_1.png in student_outputs/intersect/required folder](student_outputs/intersect/required/directional_light_1.png) |
| directional_light_2.ini | ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/intersect/required_outputs/directional_light_2.png) | ![Place directional_light_2.png in student_outputs/intersect/required folder](student_outputs/intersect/required/directional_light_2.png) |

### Design Choices

#### Functionality

- **Scene Parser** is integrated and can correctly parse the scene tree data to generate render data with calculated CTMs.
- **Camera** is implemented in ```src/camera/```, which stores camera related parameters (e.g., position, width/height, aspect ratio, view matrix, and etc.). The view matrix calculation is performed when a camera instance is created.
- **Phong Illumination** is implemented as ```calculateLighting``` function in ```src/raytracer/raytracer.cpp/```, which calculates illumination info for directional lighting.
- **Shape Implementations** can be found in ```src/primitive/primitivefunction.cpp/```, where intersections (both intersection point and normal info) with 4 primitive shapes (cube, cylinder, sphere, and cone) are calculated.
- **Intersection pipeline** is implemented as ```calculateIntersection``` function in ```src/raytracer/raytracer.cpp```, which calculates intersection for a ray with shapes. It stores the nearest intersection and returns both the intersection point and normal info.

#### Software Engineering, Efficiency, Stability

- Code is arranged in folders and classes based on the functionalities. Functions are properly designed to focus on single functionality for better adaptibility.
- Detailed comments and annotations are included, especially for explaining the algorithms in functions.
- Both parallerization and acceleration data structures are used to enable efficient rendering. Code structures are revised to reduce reduncdant calculation.
- Code has been tested on multiple scene files (including mesh) and all of them render correctly when compared to TA solutions.

<!-- ### Collaboration/References

I clarify that there is no collaboration include when I do this project.

References for mathematical formulas only (no code are referenced):
- Ray-triangle intersection: https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm

References for code doc (no complete code are referenced):
- Qt based parallel: https://doc.qt.io/qt-6/qthread.html

References for helper functions from labs and previous projects:
- project raster: code for filters (bilateral filter and mean filter).
- lab05: code for recursively parse the scene file and setup render data.
- lab06: code for Phong illumination. -->

### Known Bugs

Currently there are no obvious bugs related to functionality.

### Extra Credit

#### Acceleration data structure

Implemented BVH acceleration. BVH implementation can be found in ```src/acceleration/```, where ```BVH``` class contains build and traverse functions and ```BVHNode``` and ```AABB``` are helper classes that construct BVH.

The overall algorithn of this BVH implementation follows the top-down implementation, which it first create bounding box that contains all shapes and then recursively divide the space to create smaller bounding boxes. When dividing the space (for a parent box), it divides along the longest axis of the box to enable more balanced and efficient spacial division arrangement.

#### Parallelization

Implemented Qt-based parallelization, which divides the render image plane into blocks and parallely render the blocks for speedup. The block sizes are dynamically determined by ```QThread::idealThreadCount()``` as the number of cores to maximize the parallelization capability. Access to shared data are protected by mutex lock to prevent racing conditions and ```QtFutureWatcher``` is used to monitor the concurrent computing process.

#### Anti-aliasing

Implemented Bilaterial filter as a post-processing anti-aliasing technique. The code for filter can be found in ```src/antialias/```.

The reason for choosing bilateral filter is that it preserves details while smoothing the image. I also included other smoothing filters including median filter to satisfy specific post-processing needs.

#### Super-sampling

Implemented adaptive super-sampling by first calculating the variance of intensity around a pixel on image plane and then super-sample (shooting rays at corners of this pixel and average) on this pixel if the variance is large.

The variance of a pixel (the ray shot from it) is calculated by the sum of differences of neigbbor pixel intensities with the average intensity. If the variance exceeds a threshold and then super-sampling is conducted by shooting rays from 4 corners and the center of a pixel and then averaged to get the final color of the pixel.

#### Create your own scene file

Created a scene file ```football_player.json``` which contains a football player and a ball. The head, neck, shoulder, arms, body, hips, and legs of the players are represented by different geometries.

| File/Method To Produce Output | Expected Output | Your Output |
| :---------------------------------------: | :--------------------------------------------------: | :-------------------------------------------------: |
| football_player.ini |  ![](student_outputs/intersect/extra_credit/football_player_TA.png) | ![Place bunny_mesh.png in student_outputs/intersect/extra_credit folder](student_outputs/intersect/extra_credit/football_player.png) |

#### Render mesh

Mesh rendering is implemented by creating a BVH (mesh specific version in ```src/acceleration/BVH.cpp```) for each mesh (helper functions in ```src/primitive/mesh.cpp```) when initialzing the shape list in render data, and traverse the mesh BVH when calculating the intersection with a mesh primitive. The intersection is calculated by ray-triangle intersection in ```src/primitive/primitivefunction.cpp```.

The mesh loading is implemented as loading in cache (```src/primitive/meshcache.cpp```) which reduce multiple file reading and can access mesh data directly from the cach. The design for creating a mesh-specific BVH for each mesh primitive instead of creating primitives for each triangle is to reduce the redundant specification of materal info associated with each primitive.

| File/Method To Produce Output | Expected Output | Your Output |
| :---------------------------------------: | :--------------------------------------------------: | :-------------------------------------------------: |
| bunny_mesh.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/intersect/extra_credit_outputs/bunny_mesh.png) | ![Place bunny_mesh.png in student_outputs/intersect/extra_credit folder](student_outputs/intersect/extra_credit/bunny_mesh.png) |
| mesh.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/intersect/extra_credit_outputs/mesh.png) | ![Place mesh.png in student_outputs/intersect/extra_credit folder](student_outputs/intersect/extra_credit/mesh.png) |

## Part 2: Advanced Lighting, Textures, and more

### Advanced Lighting, Textures
Run the program with the specified `.ini` file to compare your output (it should automatically save to the correct path).
> If your program can't find certain files or you aren't seeing your output images appear, make sure to:<br/>
> 1. Set your working directory to the project directory
> 2. Set the command-line argument in Qt Creator to `template_inis/illuminate/<ini_file_name>.ini`
> 3. Clone the `scenefiles` submodule. If you forgot to do this when initially cloning this repository, run `git submodule update --init --recursive` in the project directory

> Note: once all images are filled in, the images will be the same size in the expected and student outputs.

| File/Method To Produce Output | Expected Output | Your Output |
| :---------------------------------------: | :--------------------------------------------------: | :-------------------------------------------------: | 
| point_light_1.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/point_light/point_light_1.png) | ![Place point_light_1.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/point_light_1.png) |
| point_light_2.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/point_light/point_light_2.png) | ![Place point_light_2.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/point_light_2.png) |
| spot_light_1.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/spot_light/spot_light_1.png) | ![Place spot_light_1.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/spot_light_1.png) |
| spot_light_2.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/spot_light/spot_light_2.png) | ![Place spot_light_2.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/spot_light_2.png) |
| simple_shadow.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/shadow/simple_shadow.png) | ![Place simple_shadow.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/simple_shadow.png) |
| shadow_test.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/shadow/shadow_test.png) | ![Place shadow_test.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/shadow_test.png) |
| shadow_special_case.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/shadow/shadow_special_case.png) | ![Place shadow_special_case.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/shadow_special_case.png) |
| reflections_basic.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/reflection/reflections_basic.png) | ![Place reflections_basic.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/reflections_basic.png) |
| reflections_complex.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/reflection/reflections_complex.png) | ![Place reflections_complex.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/reflections_complex.png) |
| texture_cone.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/texture_tests/texture_cone.png) | ![Place texture_cone.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/texture_cone.png) |
| texture_cone2.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/texture_tests/texture_cone2.png) | ![Place texture_cone2.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/texture_cone2.png) |
| texture_cube.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/texture_tests/texture_cube.png) | ![Place texture_cube.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/texture_cube.png) |
| texture_cube2.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/texture_tests/texture_cube2.png) | ![Place texture_cube2.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/texture_cube2.png) |
| texture_cyl.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/texture_tests/texture_cyl.png) | ![Place texture_cyl.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/texture_cyl.png) |
| texture_cyl2.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/texture_tests/texture_cyl2.png) | ![Place texture_cyl2.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/texture_cyl2.png) |
| texture_sphere.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/texture_tests/texture_sphere.png) | ![Place texture_sphere.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/texture_sphere.png) |
| texture_sphere2.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/required_outputs/texture_tests/texture_sphere2.png) | ![Place texture_sphere2.png in student_outputs/illuminate/required folder](student_outputs/illuminate/required/texture_sphere2.png) |


### Design Choices

#### Functinality
- Complete Phong illumination models (point light, spot light, and attenuation) are implemented in ```raytracer/raytracer.cpp/computeRayColor``` and ```raytracer/raytracer.cpp/calculateLighting```.
- Reflection, refraction, and shadows are implemented in ```raytracer/raytracer.cpp/computeRayColor``` and ```raytracer/raytracer.cpp/calculateLighting```
- Texture mapping functions for each primitive are implemented in ```primitive/primitiveFunction.cpp```.

#### Software Engineering, Efficiency, Stability
- Code is arranged in folders and classes based on the functionalities. Functions are properly designed to focus on single functionality for better adaptibility.
- Detailed comments and annotations are included, especially for explaining the algorithms in functions.

<!-- ### Collaboration/References

I clarify that there is no collaboration include when I do this project.

References for mathematical formulas only (no code are referenced):
- Bicubic filtering: https://en.wikipedia.org/wiki/Horner%27s_method -->

### Known Bugs

- Handling the flag. I tried to follow the stencil implementation of setting up flags but the new flag values don't change as input .ini file changes. Currently one needs to manually setup flags for ```enableSoftShadows```.
- Refraction not exactly the same as TA solution for refraction2.ini. Refraction works properly for refraction1.ini, depth_of_field.ini, and some objects in refraction2.ini. 3 of 4 spheres in refraction2.ini show generally correct refraction patterns but the nearest one does not.
- Depth of field. Current rendered depth of field has some noise, which could be the different sampling settings when compared to TA solutions.

### Extra Credit

#### Texture filtering

Texture filtering is implemented by bilinearly or bicubically interpolation the neighbor pixels instead of simply mapping a position to a single nearest pixel.

##### Bilinear filtering

The bilinear filtering uses ease function \( \text{ease} = 3\alpha^2 - 2\alpha^3 \) on each interpolating direction and get the per-direction result by \( \text{result} = A + \text{ease} \times (B - A) \). The interpolation is conducted once for each x, y direction. Interpolation function is located in ```prmitive/primitivefunction.cpp/```.

| File/Method To Produce Output | No-filter | Filtered Zoom In |
| :---------------------------------------: | :--------------------------------------------------: | :-------------------------------------------------: | 
| texture_cyl.ini |  ![](student_outputs/illuminate/extra_credit/cly_original.png) | ![Place point_light_1.png in student_outputs/illuminate/required folder](student_outputs/illuminate/extra_credit/cly_bilinear.png) |

##### Bicubic filtering

The bicubic filtering uses:
1. Coefficients:
   - \( a = 3p_1 - 3p_2 + p_3 - p_0 \)
   - \( b = 2p_0 - 5p_1 + 4p_2 - p_3 \)
   - \( c = p_2 - p_0 \)
   - \( d = 2p_1 \)

2. Interpolated Result:
   - \( \text{result} = (((a \times \alpha + b) \times \alpha) + c) \times \alpha + d \)

3. Final Result:
   - \( \text{result\_final} = 0.5 \times \text{result} \)

which uses 4 interpolation points and utilized Horner's method for polynomial evaluation. The interpolation is conducted once for each x, y direction. Interpolation function is located in ```prmitive/primitivefunction.cpp/```.

| File/Method To Produce Output | No-filter | Filtered Zoom In |
| :---------------------------------------: | :--------------------------------------------------: | :-------------------------------------------------: | 
| texture_cyl.ini |  ![](student_outputs/illuminate/extra_credit/cly_original.png) | ![Place point_light_1.png in student_outputs/illuminate/required folder](student_outputs/illuminate/extra_credit/cly_bicubic.png) |

#### Refraction 

Refraction is implemented using recursive ray tracing and Snell's law to calculate the refract direction. The main impplementation can be found in ```prmitive/primitivefunction.cpp/computeRayColor```. The refract direction is calculated by first checking whether the ray is shooting from outside to the inside or the inverse, and then applies Snell's law using appropriate parameters (the implementation can be found in ```prmitive/primitivefunction.cpp/refractDirection```). I implemented ```checkIntersectInside``` function for each primitive (in ```prmitive/primitivefunction.cpp```) to efficiently compute self-intersection from inside and isolate it from the usual intersect checking functions.

| File/Method To Produce Output | Expected Output | Your Output |
| :---------------------------------------: | :--------------------------------------------------: | :-------------------------------------------------: | 
| refraction1.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/extra_credit_outputs/refract1.png) | ![Place point_light_1.png in student_outputs/illuminate/required folder](student_outputs/illuminate/extra_credit/refract1.png) |
| refraction2.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/extra_credit_outputs/refract2.png) | ![Place point_light_1.png in student_outputs/illuminate/required folder](student_outputs/illuminate/extra_credit/refract2.png) |

#### Depth of field

Depth of field is implemented by pushing the image plane (where ray focus) further into the scene by focal length, and sample ray sources (i.e., camera position) from a circle which radius is determined by apeture. For each pixel, there are 9 rays being shooted and the final color is the average of them. The implementation is located in ```raytracer/raytracer.cpp/calculateRayInfo```

| File/Method To Produce Output | Expected Output | Your Output |
| :---------------------------------------: | :--------------------------------------------------: | :-------------------------------------------------: | 
| depth_of_field.ini |  ![](https://raw.githubusercontent.com/BrownCSCI1230/scenefiles/main/illuminate/extra_credit_outputs/depth_of_field.png) | ![Place point_light_1.png in student_outputs/illuminate/required folder](student_outputs/illuminate/extra_credit/depth_of_field.png) |


#### Soft Shadow

Soft shadow is implemented by tracing ray to a finite area of light source instead of a single point. Note that this is only available for point light and spot light since direcectional light has infinite area. For each sampling, it samples a fixed number of points on light sources and then averages to get the shadow value. The implementation is located in ```raytracer/raytracer.cpp/calculateLighting```.

| File/Method To Produce Output | Ouput | Zoom In |
| :---------------------------------------: | :--------------------------------------------------: | :-------------------------------------------------: | 
| point_light_2_softshadow.ini |  ![](student_outputs/illuminate/extra_credit/point_light_2_softshadow.png) | ![Place point_light_1.png in student_outputs/illuminate/required folder](student_outputs/illuminate/extra_credit/point_light_2_softshadow_zoomin.png) |
