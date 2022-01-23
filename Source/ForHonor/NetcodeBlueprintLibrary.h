// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NetcodeBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class FORHONOR_API UNetcodeBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		UFUNCTION(BlueprintCallable, Category = "Netcode")
		static void replicateBool(UPARAM(ref) bool& a, bool new_value);
	
};
