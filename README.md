# ITK Playground

![GitHub License](https://img.shields.io/github/license/kurijlj/Cmake-CLI-Framework)
[![GitHub Super-Linter](https://github.com/kurijlj/Cmake-CLI-Framework/actions/workflows/code-syntax-style-check.yml/badge.svg)](https://github.com/marketplace/actions/super-linter)
![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/kurijlj/Cmake-CLI-Framework/cmake-multi-platform.yml?branch=main&event=push&style=flat&logo=cmake&label=CMake%20build&labelColor=%23064F8C)
![Static Badge](https://img.shields.io/badge/-v17-%23ffffff?style=flat&logo=cplusplus&labelColor=%2300599C)
![Static Badge](https://img.shields.io/badge/-3.12-%23ffffff?style=flat&logo=cmake&labelColor=%23064F8C)

This repository chronicles my self-paced exploration of the ITK (Insight
Toolkit) library.

## Getting Started

To start using the framework in this repository, follow these steps:

1. **Clone the Repository:** Clone this repository to your local machine using
the following command:

    ``` shell
    git clone https://github.com/kurijlj/ITK_Playground.git
    ```

2. **Navigate to a Project Tree:** Browse through the project tree and add
business logic and CMake build instructions required for your app.

3. **Compile the code:** Build as a regular CMake project:

   1. Create a build directory and `cd` into it.
   2. Create a directory structure and project makefiles using (optionally you
   can specify the generator by invoking with the -G switch):

       ``` shell
       cmake -G <Generator> -B . -S <project_source_tree> -DITK_SKIP_PATH_LENGTH_CHECKS:BOOL=[ON|OFF] -DBUILD_SHARED_LIBS:BOOL=[ON|OFF] -DBUILD_TESTS:BOOL=[ON|OFF]
       ```

   3. Build executable using:

       ```shell
       cmake --build . --config [Debug|RelWithDebInfo|Release|MinSizeRel]
       ```

## Third-party Library Integration

- **[ITK](https://itk.org/):** Open-source, cross-platform library for
  processing, segmentation, and registration of scientific images in two, three,
  or more dimensions.
- **[clipp](https://github.com/muellan/clipp):** Command line interfaces library
  for modern C++.

## Build Targets

- **dummy:** A dummy target that does nothing. Useful for testing
  the build tool.
- **create_image:** A simple test of ITK create image and write to image
  facilities.
- **create_image_from_buffer:** Create ITK image object from a buffer and write it to a file.
- **create_step_wedge:** Create computational optical density step wedge image.
- **image_affine_transform:** Rotate and translate an image using ITK.
- **rgb_to_luminance:** Convert RGB image to luminance image.
- **split_channels:** Split color channels of an image.
- **all**: Build all abovementioned targets.

## License

This repository is licensed under the [GNU General Public License
v3.0](LICENSE), ensuring that the code remains open-source and accessible to the
community.
