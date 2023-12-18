// Copyright 2023 Petr Leontev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialExpressionTextureSampleParameter.h"
#include "AnimatedTexture2DSampler.generated.h"

UCLASS(collapsecategories, hidecategories = Object)
class RUNTIMEIMAGELOADER_API UAnimatedTexture2DSampler : public UMaterialExpressionTextureSampleParameter
{
	GENERATED_UCLASS_BODY()

	//~ Begin UMaterialExpression Interface
#if WITH_EDITOR
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
#endif // WITH_EDITOR
	//~ End UMaterialExpression Interface

	//~ Begin UMaterialExpressionTextureSampleParameter Interface
	virtual bool TextureIsValid(UTexture* InTexture, FString& OutMessage);
	virtual void SetDefaultTexture();
	//~ End UMaterialExpressionTextureSampleParameter Interface
	
};