// Copyright 2022 Peter Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformProcess.h"
#include "Modules/ModuleManager.h"

THIRD_PARTY_INCLUDES_START
	#include "nsgif.h"
THIRD_PARTY_INCLUDES_END

extern "C"
{
	/** Function pointer type for nsgif_create */
	typedef nsgif_error(*nsgif_create_FnPtr)(
		const nsgif_bitmap_cb_vt* bitmap_vt,
		nsgif_bitmap_fmt_t bitmap_fmt,
		nsgif_t** gif_out1
		);

	/** Function pointer type for nsgif_destroy */
	typedef void(*nsgif_destroy_FnPtr)(nsgif_t* gif);

	/** Function pointer type for nsgif_data_scan */
	typedef nsgif_error(*nsgif_data_scan_FnPtr)(
		nsgif_t* gif,
		size_t size,
		const uint8_t* data
		);

	/** Function pointer type for nsgif_data_complete */
	typedef void(*nsgif_data_complete_FnPtr)(nsgif_t* gif);

	/** Function pointer type for nsgif_frame_prepare */
	typedef nsgif_error(*nsgif_frame_prepare_FnPtr)(
		nsgif_t* gif,
		nsgif_rect_t* area,
		uint32_t* delay_cs,
		uint32_t* frame_new
		);

	/** Function pointer type for nsgif_frame_decode */
	typedef nsgif_error(*nsgif_frame_decode_FnPtr)(
		nsgif_t* gif,
		uint32_t frame,
		nsgif_bitmap_t** bitmap
		);

	/** Function pointer type for nsgif_reset */
	typedef nsgif_error(*nsgif_reset_FnPtr)(nsgif_t* gif);

	/** Function pointer type for nsgif_get_info */
	typedef const nsgif_info_t*(*nsgif_get_info_FnPtr)(const nsgif_t* gif);

}

/** Pointer to nsgif_create function. */
static nsgif_create_FnPtr Fn_nsgif_create = nullptr;
/** Pointer to nsgif_destroy function. */
static nsgif_destroy_FnPtr Fn_nsgif_destroy = nullptr;
/** Pointer to nsgif_data_scan function. */
static nsgif_data_scan_FnPtr Fn_nsgif_data_scan = nullptr;
/** Pointer to nsgif_data_complete function. */
static nsgif_data_complete_FnPtr Fn_nsgif_data_complete = nullptr;
/** Pointer to nsgif_frame_prepare function. */
static nsgif_frame_prepare_FnPtr Fn_nsgif_frame_prepare = nullptr;
/** Pointer to nsgif_frame_decode function. */
static nsgif_frame_decode_FnPtr Fn_nsgif_frame_decode = nullptr;
/** Pointer to nsgif_reset function. */
static nsgif_reset_FnPtr Fn_nsgif_reset = nullptr;
/** Pointer to nsgif_get_info function. */
static nsgif_get_info_FnPtr Fn_nsgif_get_info = nullptr;

class FRuntimeImageLoaderModule : public IModuleInterface
{
public:
	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline FRuntimeImageLoaderModule& Get()
	{
		static const FName ModuleName = "RuntimeImageLoader";
		return FModuleManager::LoadModuleChecked<FRuntimeImageLoaderModule>(ModuleName);
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() during shutdown if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		static const FName ModuleName = "RuntimeImageLoader";
		return FModuleManager::Get().IsModuleLoaded(ModuleName);
	}

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Handle to the libnsgif dll we will load */
	void* LibnsgifHandle; /** DLL handle for libnsgif.dll */
public:
	static nsgif_create_FnPtr FunctionPointerNsgifCreate();
	static nsgif_destroy_FnPtr FunctionPointerNsgifDestroy();
	static nsgif_data_scan_FnPtr FunctionPointerNsgifDataScan();
	static nsgif_data_complete_FnPtr FunctionPointerNsgifDataComplete();
	static nsgif_frame_prepare_FnPtr FunctionPointerNsgifFramePrepare();
	static nsgif_frame_decode_FnPtr FunctionPointerNsgifFrameDecode();
	static nsgif_reset_FnPtr FunctionPointerNsgifReset();
	static nsgif_get_info_FnPtr FunctionPointerNsgifGetInfo();
};