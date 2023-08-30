// Copyright 2023 Unreal Solutions Ltd. All Rights Reserved.

#include "RuntimeGifReader.h"
#include "HAL/RunnableThread.h"
#include "Async/Async.h"
#include "Texture2DAnimation/AnimatedTexture2D.h"
#include "RuntimeImageLoaderLog.h"

DEFINE_LOG_CATEGORY(RuntimeGifReader);

// ------------------------------------------------------

void URuntimeGifReader::Initialize()
{
	ThreadSemaphore = FPlatformProcess::GetSynchEventFromPool(false);
	Thread = FRunnableThread::Create(this, TEXT("RuntimeImageReader"), 0, TPri_SlightlyBelowNormal);

	UE_LOG(RuntimeGifReader, Log, TEXT("Gif reader thread started!"))
}

void URuntimeGifReader::Deinitialize()
{
	Clear();
	Stop();

	UE_LOG(RuntimeGifReader, Log, TEXT("Gif reader thread exited!"))
}

void URuntimeGifReader::AddRequest(const FString& GifFileName)
{
	Requests.Enqueue(GifFileName);

	bCompletedWork.AtomicSet(false);
}

bool URuntimeGifReader::GetResult(FGifReadResult& OutResult)
{
	FScopeLock ResultsLock(&ResultsMutex);

	if (Results.Num() > 0)
	{
		OutResult = Results.Pop();

		return true;
	}

	return false;
}

void URuntimeGifReader::Clear()
{
	Requests.Empty();

	{
		FScopeLock ResultsLock(&ResultsMutex);
		Results.Empty();
	}

	bCompletedWork = true;
}

void URuntimeGifReader::Stop()
{
	bStopThread = true;

	Trigger();
	Thread->WaitForCompletion();

	FPlatformProcess::ReturnSynchEventToPool(ThreadSemaphore);
}

bool URuntimeGifReader::IsWorkCompleted() const
{
	return bCompletedWork;
}

void URuntimeGifReader::Trigger()
{
	ThreadSemaphore->Trigger();
}

void URuntimeGifReader::BlockTillAllRequestsFinished()
{
	while (!bCompletedWork && !bStopThread)
	{
		FString GifFilename;
		while (Requests.Dequeue(GifFilename) && !bStopThread)
		{
			PendingReadResult.GifFilename = GifFilename;
			if (PendingReadResult.GifFilename.Len() > 0)
			{
				UE_LOG(RuntimeGifReader, Log, TEXT("Reading Gif from file: %s"), *PendingReadResult.GifFilename);
			}

			if (!ProcessRequest(GifFilename))
			{
				PendingReadResult.OutError = FString::Printf(TEXT("Error: Failed to Process Gif Request."));
				UE_LOG(RuntimeGifReader, Warning, TEXT("Failed to process request"));
			}

			FScopeLock ResultsLock(&ResultsMutex);
			Results.Add(PendingReadResult);

			PendingReadResult = FGifReadResult();
		}

		bCompletedWork.AtomicSet(Requests.IsEmpty());
	}
}

bool URuntimeGifReader::ProcessRequest(const FString& InGifFilename)
{
	Decoder = MakeUnique<FRuntimeGIFLoaderHelper>();
	check(Decoder.IsValid());

	bool bResult = Decoder->DecodeGIF(InGifFilename);
	if (!bResult)
	{
		PendingReadResult.OutError = FString::Printf(TEXT("Error: Failed to read Gif File."));
		return false;
	}

	UAnimatedTexture2D* Texture = UAnimatedTexture2D::Create(Decoder->GetWidth(), Decoder->GetHeight());
	if (!IsValid(Texture))
	{
		PendingReadResult.OutError = FString::Printf(TEXT("Error: Failed to Create Animated Texture Gif."));
		UE_LOG(RuntimeGifReader, Error, TEXT("Gif Decoder Unable to Decode Gif"));
	}

	Texture->SetDecoder(MoveTemp(Decoder));

	Texture->SRGB = true;
	Texture->UpdateResource();

	PendingReadResult.OutTexture = Texture;

	return true;
}

void URuntimeGifReader::Init(const FString& InGifFilename)
{
	Decoder = MakeUnique<FRuntimeGIFLoaderHelper>();
	check(Decoder.IsValid());

	CurrentTask = Async(
		EAsyncExecution::Thread,
		[this, InGifFilename]()
		{
			bool bRes = Decoder->DecodeGIF(InGifFilename);

			OnPostGifDecode(bRes);
		}
	);
}

void URuntimeGifReader::OnPostGifDecode(bool bRes)
{
	Async(
		EAsyncExecution::TaskGraphMainThread,
		[bRes, this]()
		{
			UAnimatedTexture2D* Texture = UAnimatedTexture2D::Create(Decoder->GetWidth(), Decoder->GetHeight());
			if (!bRes || !IsValid(Texture))
			{
				UE_LOG(RuntimeGifReader, Error, TEXT("Gif Decoder Unable to Decode Gif"));
				OnGifLoaded.Execute(Texture, Decoder->GetDecodeError());
			}

			Texture->SetDecoder(MoveTemp(Decoder));

			Texture->SRGB = true;
			Texture->UpdateResource();

			OnGifLoaded.Execute(Texture, TEXT(""));
		}
	).Wait();
}

void URuntimeGifReader::Cancel()
{
	CurrentTask.Reset();
}


bool URuntimeGifReader::Init()
{
	return true;
}

uint32 URuntimeGifReader::Run()
{
	while (!bStopThread)
	{
		ThreadSemaphore->Wait();

		BlockTillAllRequestsFinished();
	}

	return 0;
}

void URuntimeGifReader::Exit()
{

}
