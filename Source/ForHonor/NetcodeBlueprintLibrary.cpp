// Fill out your copyright notice in the Description page of Project Settings.


#include "NetcodeBlueprintLibrary.h"

void UNetcodeBlueprintLibrary::replicateBool(UPARAM(ref) bool& a, bool new_value)
{
	a = new_value;
}
