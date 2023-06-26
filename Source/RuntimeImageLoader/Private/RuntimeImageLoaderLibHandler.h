// Copyright Unreal Solution Ltd, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "HAL/PlatformProcess.h"

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
	typedef const nsgif_info_t* (*nsgif_get_info_FnPtr)(const nsgif_t* gif);

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

class FLibnsgifHandler
{
public:
	static bool Initialize();
	static bool IsInitialized();
	static void Shutdown();

private:
	/** Handle to the libnsgif dll we will load */
	static void* LibnsgifHandle; /** DLL handle for libnsgif.dll */
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