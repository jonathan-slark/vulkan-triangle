# Vulkan Triangle in C99 and Win32

A minimalist implementation of the [Vulkan Triangle tutorial](https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/00_Base_code.html) using C99 instead of C++ and Win32 instead of glfw. It is a complete and well-documented example of Vulkan initialization and triangle rendering.

![Triangle Screenshot](screenshot.png)

## Overview

This project implements the Vulkan tutorial up to and including "Swap chain recreation", resulting in a fully functional Vulkan program. While it may seem complex (over 1200 lines) for drawing a triangle, it demonstrates proper Vulkan initialization and rendering setup.

## Goal

The primary aim of this project is to evaluate Vulkan's suitability for solo-developed game projects, emphasizing low-level control with minimal external dependencies. I have chosen C99 over C++ and Win32 over GLFW, utilizing MSYS2 as the build environment. This choice reflects my preference for procedural programming and simplicity in language design, which I find advantageous over C++'s enforced object-oriented paradigm. Additionally, due to limited support for C99 in Visual Studio, MSYS2 provides an optimal Linux-style build environment on Windows platforms.

## Postmortem

Adopting C99 has proven to be a solid choice, especially for desktop games that need to compile on Windows, macOS, and Linux. While I’m unlikely to target platforms beyond desktop, I found that working with Vulkan’s low-level API was both challenging and rewarding. Using Win32 became straightforward once I discovered some well-documented examples.

One notable challenge with Vulkan is its requirement for a deep understanding of the graphics pipeline. For developers whose primary focus isn’t on graphics programming—or for those building game engines—using a higher-level API like OpenGL might be more appropriate. Even though OpenGL is considered deprecated by some, and Vulkan is designed for modern GPUs and multi-threaded CPUs, both have their merits and will likely coexist for the foreseeable future.

## Building

The following MSYS2 packages are required:
- make
- mingw-w64-ucrt-x86_64-gcc
- mingw-w64-x86_64-vulkan-headers
- mingw-w64-x86_64-vulkan-loader
- mingw-w64-x86_64-vulkan-validation-layers

The project uses the `glslc.exe` compiler from the Vulkan SDK for shader compilation.

## License

This project is licensed under the MIT License - see `LICENSE.txt`.
