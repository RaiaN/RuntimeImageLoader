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
- Fast & hitch-less & zero-freeze runtime texture loading up to 8k resolution
- Blueprint friendly (see below)
- Supports PNG, JPEG, BMP, TGA, OpenEXR, TIFF and QOI
- Supports 8, 16, 32 bit depth per channel images  (or up to 128 bit *pixel depth* images)
- Output texture is in RGBA8 format if option bForUI = true
- Resizes loaded image if SizeX and SizeY params are set
- No static libraries or external dependencies except for single-header libraries
- Supported RHIs: DirectX 11&12, Vulkan
- Supported Unreal engine versions: 4.27, 5.0
- Supported platforms: Windows

# Blueprints

Below is the example of how to use this plugin for loading images from your blueprints/scripts:

<img src="Resources/Blueprint_node.PNG">
