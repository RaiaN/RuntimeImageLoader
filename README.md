<br/>
<p align="center">
  <a href="https://github.com/RaiaN/ue4_runtimeimageloader">
    <img src="Resources/Icon128.png" alt="Logo" width="128" height="128">
  </a>

  <h3 align="center">Runtime Image Loader</h3>

  <p align="center">
    Load images into Unreal at runtime without hitches!
    <br/>
    <br/>
  </p>
</p>

Grab it from Unreal Marketplace: https://www.unrealengine.com/marketplace/en-US/product/runtime-image-loader 

Developer's UPDATE: **The active development of this free plugin is completed! It provides an intuitive Blueprint interface that allows to load images without facing issues that exist in vanilla Unreal Engine. That was my main intention and I'm not planning to support more advanced workflows, for example, cache layers, mip generation, rare image formats and OS besides Windows and Android (experimental). However, you are free to use this plugin plus I always welcome pull requests targeting new features.**

## Features
- Supports loading of up to 8k resolution images in hitch-less manner
- Allows to import HDR images aka Cubemaps (Windows only)
- Can load image over HTTP or from local file storage
- Can transform image during loading
- Can cancel all ongoing image loading requests (Windows only)
- Supports PNG, JPEG, BMP, TGA, OpenEXR, TIFF and QOI
- Supports 8, 16, 32 bit per channel (or up to 128 bit *pixel depth* images)
- Can generate UI ready texture format (RGBA8 or 'float' RGBA)
- Blueprint friendly
- No static libraries or external dependencies (except for single-header libraries)

## Tested on
- RHIs: DirectX 11&12, Vulkan
- Unreal engine versions: 4.27, 5.0
- Platforms: Windows, Android (experimental)

## Blueprints

Below is the example of how to use this plugin for loading images in your blueprints/scripts:

<img src="Resources/Blueprint_node.PNG">
