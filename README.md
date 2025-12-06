<div align="center">

<img src="Resources/Icon128.png" alt="Runtime Image Loader" width="128" height="128">

# Runtime Image Loader

### Load images and GIFs into Unreal Engine at runtime — without hitches!

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.1%2B-blue?style=for-the-badge&logo=unrealengine&logoColor=white)](https://www.unrealengine.com/)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-4.27-lightgrey?style=for-the-badge&logo=unrealengine&logoColor=white)](https://www.unrealengine.com/)
[![Platform](https://img.shields.io/badge/Platforms-Win%20%7C%20Linux%20%7C%20Mac%20%7C%20Android-green?style=for-the-badge)](#supported-platforms)
[![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)](LICENSE)

[Features](#-features) • [Installation](#-installation) • [Usage](#-usage) • [Platforms](#-supported-platforms) • [Discord](https://discord.com/invite/pBDSCBcdgv)

---

</div>

## 🎯 Features

| Feature | Description |
|---------|-------------|
| 🖼️ **High Resolution** | Load up to **8K resolution** images without hitches |
| 🎬 **Animated Images** | GIF and WebP animation support at runtime |
| 🌍 **HDR / Cubemaps** | Import HDR images as Cubemaps *(Windows only)* |
| 🌐 **Flexible Sources** | Load from HTTP URLs, local files, or byte arrays (`TArray<uint8>`) |
| ✂️ **Transform on Load** | Apply transformations during the loading process |
| ⏹️ **Cancellation** | Cancel ongoing image requests *(Windows only)* |
| 🎨 **Wide Format Support** | PNG, JPEG, BMP, TGA, OpenEXR, TIFF, QOI |
| 🔢 **Bit Depth** | 8, 16, 32 bit per channel (up to 128-bit pixel depth) |
| 🖥️ **UI Ready** | Generate RGBA8 or float RGBA textures |
| ⚙️ **Texture Filtering** | Configurable texture filtering modes |
| 📘 **Blueprint Friendly** | Full Blueprint support for rapid prototyping |
| 📦 **Zero Dependencies** | No static libraries or external dependencies |

---

## 📥 Installation

### Option 1: Clone into Plugins folder
```bash
cd YourProject/Plugins
git clone https://github.com/RaiaN/ue4_runtimeimageloader.git RuntimeImageLoader
```

### Option 2: Download as ZIP
1. Download the [latest release](https://github.com/RaiaN/ue4_runtimeimageloader/releases)
2. Extract to `YourProject/Plugins/RuntimeImageLoader`

### Option 3: Git Submodule
```bash
git submodule add https://github.com/RaiaN/ue4_runtimeimageloader.git Plugins/RuntimeImageLoader
```

After installation, regenerate your project files and rebuild.

---

## 🚀 Usage

### Blueprint Example

Load images easily in your Blueprints:

<div align="center">
<img src="Resources/Blueprint_node.PNG" alt="Blueprint Example" width="600">
</div>

---

## 🎮 Supported Platforms

| Platform | Status | Notes |
|----------|--------|-------|
| **Windows** | ✅ Full Support | DirectX 11/12, Vulkan |
| **Linux** | ✅ Full Support | Vulkan |
| **macOS** | ✅ Full Support | Metal |
| **Android** | ✅ Full Support | Vulkan |
| **Oculus VR** | 🧪 Experimental | — |

### Supported Engine Versions

| Version | Status |
|---------|--------|
| **Unreal Engine 5.1+** | ✅ Supported |
| **Unreal Engine 4.27** | ✅ Supported |

### Supported RHIs

- DirectX 11
- DirectX 12
- Vulkan
- Metal

---

## 🤝 Community

<div align="center">

[![Discord](https://img.shields.io/badge/Join%20our-Discord-5865F2?style=for-the-badge&logo=discord&logoColor=white)](https://discord.com/invite/pBDSCBcdgv)

**Using this plugin?** We'd love to hear about your project!

[![Share Your Project](https://img.shields.io/badge/Share%20Your%20Project-📝-orange?style=for-the-badge)](https://tally.so/r/n94z6V)

</div>

---

<div align="center">

Made with ❤️ for the Unreal Engine community

</div>
