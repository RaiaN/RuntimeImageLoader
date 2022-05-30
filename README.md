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

Grab it from Unreal Marketplace: https://www.unrealengine.com/marketplace/en-US/product/runtime-image-loader !

## Features
- Supports loading of up to 8k resolution images in hitch-less manner
- Can load image over HTTP or from local file storage
- Can transform image during loading
- Supports PNG, JPEG, BMP, TGA, OpenEXR, TIFF and QOI
- Supports 8, 16, 32 bit per channel (or up to 128 bit *pixel depth* images)
- Can generate UI ready texture format (RGBA8 or 'float' RGBA)
- Blueprint friendly
- No static libraries or external dependencies (except for single-header libraries)

## Tested on
- RHIs: DirectX 11&12, Vulkan
- Unreal engine versions: 4.27, 5.0
- Platforms: Windows

# Blueprints

Below is the example of how to use this plugin for loading images in your blueprints/scripts:

<img src="Resources/Blueprint_node.PNG">
