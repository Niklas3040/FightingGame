//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//
///**
// * 
// */
//class SMOOTHSYNCPLUGIN_API NetworkUtil
//{
//public:
//
//	template<class T>
//	static void WriteToBuffer(TArray<uint8>& buffer, int& offset, T thing)
//	{
//		uint8* thingAsChars = (uint8*)(&thing);
//		for (int i = 0; i < sizeof(T); i++)
//		{
//			buffer.Add(thingAsChars[i]);
//		}
//
//		offset += sizeof(T);
//	}
//
//	template <>
//	static void WriteToBuffer<FVector>(TArray<uint8>& buffer, int& offset, FVector thing)
//	{
//		WriteToBuffer(buffer, offset, thing.X);
//		WriteToBuffer(buffer, offset, thing.Y);
//		WriteToBuffer(buffer, offset, thing.Z);
//	}
//
//	template<class T>
//	static void ReadFromBuffer(TArray<uint8>& buffer, int& offset, T* thing)
//	{
//		auto size = sizeof(T);
//		if (buffer.Num() < offset + size)
//		{
//			UE_LOG(LogTemp, Error, TEXT("Nothing in buffer to read"));
//			return;
//		}
//		if (thing == nullptr)
//		{
//			UE_LOG(LogTemp, Error, TEXT("Invalid destination for deserialization"));
//			return;
//		}
//
//		auto thingAsChars = (uint8*)thing;
//		for (int i = 0; i < size; i++)
//		{
//			thingAsChars[i] = buffer[offset + i];
//		}
//		offset += size;
//	}
//
//	template <>
//	static void ReadFromBuffer<FVector>(TArray<uint8>& buffer, int& offset, FVector* thing)
//	{
//		ReadFromBuffer(buffer, offset, &(thing->X));
//		ReadFromBuffer(buffer, offset, &(thing->Y));
//		ReadFromBuffer(buffer, offset, &(thing->Z));
//	}
//};
