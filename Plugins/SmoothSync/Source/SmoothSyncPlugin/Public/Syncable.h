//// Copyright 2018-2019 Noble Whale, LLC. All Rights Reserved.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include <functional>
//#include "NetworkUtil.h"
//#include "Kismet/GameplayStatics.h"
//#include <type_traits>
//
///**
// *
// */
// // Forward declaration of Syncable so that SyncableValue can use it
//template<class Property>
//class Syncable;
//
//// Forward declaration of SyncableGroup so that SyncableGroupValue can use it
//template<class Property, class ChildProperty>
//class SyncableGroup;
//
//// For keeping track of when a property is at rest.
//UENUM(BlueprintType)
//enum class RestState : uint8
//{
//	AT_REST, JUST_STARTED_MOVING, MOVING
//};
//
//// The extrapolation mode.
//UENUM(BlueprintType)
//enum class ExtrapolationMode : uint8
//{
//	UNLIMITED, LIMITED, NONE
//};
//
///// <summary>The variables that will be synced.</summary>
//UENUM(BlueprintType)
//enum class SyncMode : uint8
//{
//	NONE = 0b000,
//	X = 0b001,
//	Y = 0b010,
//	Z = 0b100,
//	XY = 0b011,
//	XZ = 0b101,
//	YZ = 0b110,
//	XYZ = 0b111
//};
//ENUM_CLASS_FLAGS(SyncMode)
//
//// Represents a syncable property and its associated value
//template<class T> class SyncableValue
//{
//public:
//
//	// The value of the property
//	T value;
//	// The syncable object that tells us how to sync the property
//	Syncable<T>* syncable = nullptr;
//	bool isAtRest = false;
//
//	// Store a reference to the backing syncable object
//	SyncableValue(Syncable<T>* syncable)
//	{
//		this->syncable = syncable;
//	}
//
//	// Read the value from a buffer starting at the offset
//	// offset is incremented by sizeof(Property) after reading.
//	void Deserialize(TArray<uint8>& buffer, int& offset)
//	{
//		if (syncable->isCompressed)
//		{
//			syncable->DeserializeCompressedValue(buffer, offset, &value);
//			syncable->PostProcessCompressedValue(value);
//		}
//		else
//		{
//			syncable->DeserializeValue(buffer, offset, &value);
//		}
//
//		syncable->valueLastDeserialized = value;
//		syncable->hasEverDeserializedValue = true;
//	}
//
//	// Set the synced value on non-owners.
//	// This uses the customizable ApplyValue method from the backing syncable object
//	// so what this actually does is dependent on the configuration. For example for 
//	// the position syncable this will call setPosition() on a SmoothSync instance.
//	void ApplyValue(T value)
//	{
//		if (syncable != nullptr && syncable->ApplyValue != nullptr)
//		{
//			syncable->ApplyValue(value);
//		}
//	}
//
//	// Apply easing and snapping before calling ApplyValue() to set the synced value on non-owners
//	// The value is only applied if it IsOverReceiveThreshold()
//	void Apply(bool forceSnapping = false)
//	{
//		T currentLocalValue = syncable->GetValue();
//		if (syncable->IsOverReceiveThreshold == nullptr || syncable->IsOverReceiveThreshold(value, currentLocalValue))
//		{
//			float actualEasing = syncable->easing;
//			if (forceSnapping)
//			{
//				actualEasing = 1;
//			}
//			else if (syncable->IsOverSnapThreshold != nullptr && syncable->IsOverSnapThreshold(value, currentLocalValue))
//			{
//				actualEasing = 1;
//			}
//			T easedValue = syncable->Interpolate(currentLocalValue, value, actualEasing);
//			ApplyValue(easedValue);
//		}
//	}
//};
//
///**
// * Represents anything that can be synced
// * Each Syncable has a number of callbacks that are used to determine
// * where the value to sync comes from and how and when it should be sent
// * over the network.
// */
//template<class Property> class SMOOTHSYNCPLUGIN_API ISyncable
//{
//public:
//	// Used to get the local version of the value being synced
//	// This is called on the owner as part of serialization
//	std::function<Property()> GetValue = nullptr;
//	// Used to apply the deserialized and interpolated value on non-owners
//	std::function<void(Property)> ApplyValue = nullptr;
//	// You can use this to customize Serialization but you probably won't need to
//	std::function<void(TArray<uint8>&, int&, Property)> SerializeValue = nullptr;
//	// You can use this to customize Deserialization but you probably won't need to
//	std::function<void(TArray<uint8>&, int&, Property*)> DeserializeValue = nullptr;
//	std::function<void(TArray<uint8>&, int&, Property)> SerializeCompressedValue = nullptr;
//	// You can use this to customize Deserialization of compressed values but you probably won't need to
//	std::function<void(TArray<uint8>&, int&, Property*)> DeserializeCompressedValue = nullptr;
//	std::function<bool(Property, Property)> IsOverSendThreshold = nullptr;
//	std::function<bool(Property, Property)> IsOverReceiveThreshold = nullptr;
//	std::function<bool(Property, Property)> IsOverSnapThreshold = nullptr;
//	std::function<bool(Property, Property)> IsOverRestThreshold = nullptr;
//	std::function<Property(Property, Property, float)> Interpolate = nullptr;
//	std::function<void(Property&)> PreProcessCompressedValue = nullptr;
//	std::function<void(Property&)> PostProcessCompressedValue = nullptr;
//	std::function<void(Property)> LogValue = nullptr;
//
//	ExtrapolationMode extrapolationMode = ExtrapolationMode::LIMITED;
//	RestState restState = RestState::MOVING;
//	int sameValueCount = 0;
//	FString propertyName = "";
//
//	Property valueLastTeleportedTo;
//	Property valueLastSerialized;
//	Property valueLastDeserialized;
//	Property valueLastFrame;
//
//	float easing = 1;
//	float sendThreshold = 0;
//	float receiveThreshold = 0;
//	float snapThreshold = 0;
//	float restValueThreshold = 0;
//	float restTimeThreshold = .5f;
//	bool isCompressed = false;
//	bool canRest = false;
//	bool hasEverDeserializedValue = false;
//
//	// Set up the default Serialization and Deserialization callbacks
//	ISyncable(float easing = 1, bool isCompressed = false)
//	{
//		this->easing = easing;
//		this->isCompressed = isCompressed;
//		SerializeValue = [this](TArray<uint8>& buffer, int& offset, Property value) {
//			NetworkUtil::WriteToBuffer(buffer, offset, value);
//		};
//		DeserializeValue = NetworkUtil::ReadFromBuffer<Property>;
//
//		Interpolate = [this](Property v1, Property v2, float t) { return _Interpolate(v1, v2, t); };
//	}
//
//	~ISyncable() { }
//
//	bool ShouldSend()
//	{
//		if (restState == RestState::AT_REST) return false;
//		if (IsOverSendThreshold != nullptr && !IsOverSendThreshold(GetValue(), valueLastSerialized)) return false;
//
//		return true;
//	}
//
//	static Property _Interpolate(Property v1, Property v2, float t)
//	{
//		return FMath::Lerp(v1, v2, t);
//	}
//
//	void UpdatePreviousValue()
//	{
//		if (sameValueCount == 0 && restState != RestState::AT_REST)
//		{
//			valueLastFrame = GetValue();
//		}
//	}
//
//	bool ShouldSendRestMessage()
//	{
//		if (extrapolationMode != ExtrapolationMode::NONE)
//		{
//			if (restState == RestState::AT_REST)
//			{
//				return true;
//			}
//		}
//		return false;
//	}
//
//	void Teleport()
//	{
//		valueLastTeleportedTo = GetValue();
//	}
//
//	void Serialize(TArray<uint8>& buffer, int& offset)
//	{
//		Serialize(buffer, offset, GetValue());
//	}
//
//	void Serialize(TArray<uint8>& buffer, int& offset, Property value)
//	{
//		if (propertyName.Equals("Position"))
//		{
//			LogValue(value);
//		}
//
//		if (extrapolationMode != ExtrapolationMode::NONE)
//		{
//			if (restState == RestState::JUST_STARTED_MOVING)
//			{
//				value = valueLastSerialized;
//			}
//		}
//
//		if (isCompressed)
//		{
//			Property processedValue = value;
//			PreProcessCompressedValue(processedValue);
//			SerializeCompressedValue(buffer, offset, processedValue);
//		}
//		else
//		{
//			SerializeValue(buffer, offset, value);
//		}
//
//		valueLastSerialized = value;
//	}
//
//	void Tick(float deltaTime, bool& forceSend)
//	{
//		if (extrapolationMode != ExtrapolationMode::NONE && IsOverRestThreshold != nullptr && restValueThreshold != -1)
//		{
//			if (!IsOverRestThreshold(GetValue(), valueLastFrame))
//			{
//				if (restState != RestState::AT_REST)
//				{
//					sameValueCount += deltaTime;
//				}
//				if (sameValueCount >= restTimeThreshold)
//				{
//					sameValueCount = 0;
//					restState = RestState::AT_REST;
//					forceSend |= true;
//				}
//			}
//			else
//			{
//				if (restState == RestState::AT_REST && GetValue() != valueLastTeleportedTo)
//				{
//					restState = RestState::JUST_STARTED_MOVING;
//					forceSend |= true;
//				}
//				else if (restState == RestState::JUST_STARTED_MOVING)
//				{
//					restState = RestState::MOVING;
//				}
//				else
//				{
//					sameValueCount = 0;
//				}
//			}
//		}
//	}
//};
//
//template<class Property> class Syncable : public ISyncable<Property>
//{
//public:
//	Syncable(float easing = 1, bool isCompressed = false) : ISyncable<Property>(easing, isCompressed) { }
//};
//
//template<class Property> class SyncablePrimitive : public ISyncable<Property>
//{
//public:
//	SyncablePrimitive(float easing = 1, bool isCompressed = false) : ISyncable<Property>(easing, isCompressed)
//	{
//		IsOverSendThreshold = [this](Property oldValue, Property newValue) {
//			return _IsOverThreshold(oldValue, newValue, sendThreshold);
//		};
//		IsOverReceiveThreshold = [this](Property oldValue, Property newValue) {
//			return _IsOverThreshold(oldValue, newValue, receiveThreshold);
//		};
//		IsOverSnapThreshold = [this](Property oldValue, Property newValue) {
//			if (snapThreshold == 0) return false;
//			return _IsOverThreshold(oldValue, newValue, snapThreshold);
//		};
//		IsOverRestThreshold = [this](Property oldValue, Property newValue) {
//			if (!canRest) return false;
//			return _IsOverThreshold(oldValue, newValue, restValueThreshold);
//		};
//
//		LogValue = [this](Property v) { UE_LOG(LogTemp, Warning, TEXT("%s: %f"), *propertyName, (float)v); };
//	}
//
//	bool _IsOverThreshold(Property oldValue, Property newValue, float threshold)
//	{
//		if (threshold == 0) return true;
//
//		Property delta = FMath::Abs(newValue - oldValue);
//		return delta > threshold;
//	}
//};
//
//// Many simple types will work automatically by extending from SyncablePrimitive
//template<> class Syncable<char> : public SyncablePrimitive<char> { using SyncablePrimitive::SyncablePrimitive; };
//template<> class Syncable<int> : public SyncablePrimitive<int> { using SyncablePrimitive::SyncablePrimitive; };
//template<> class Syncable<short> : public SyncablePrimitive<short> { using SyncablePrimitive::SyncablePrimitive; };
//template<> class Syncable<long> : public SyncablePrimitive<long> { using SyncablePrimitive::SyncablePrimitive; };
//template<> class Syncable<double> : public SyncablePrimitive<double> { using SyncablePrimitive::SyncablePrimitive; };
//template<> class Syncable<FFloat16> : public SyncablePrimitive<FFloat16> { using SyncablePrimitive::SyncablePrimitive; };
//
//// Float syncables are specialized to supply compression / decompression methods
//// This can be done with any type, you just need to define how to compress the values.
//// For floats the value is divided by 100 and converted to an FFload16 to compress
//// and the reverse is done to decompress
//template<> class Syncable<float> : public SyncablePrimitive<float>
//{
//public:
//	Syncable(float easing = 1, bool isCompressed = false) : SyncablePrimitive<float>(easing, isCompressed)
//	{
//		SerializeCompressedValue = _SerializeCompressed;
//		DeserializeCompressedValue = _DeserializeCompressed;
//		PreProcessCompressedValue = _PreProcessCompressedValue;
//		PostProcessCompressedValue = _PostProcessCompressedValue;
//	}
//
//	static void _DeserializeCompressed(TArray<uint8>& buffer, int& offset, float* value)
//	{
//		FFloat16 temp;
//		NetworkUtil::ReadFromBuffer(buffer, offset, &(temp));
//		(*value) = float(temp);
//	}
//
//	static void _SerializeCompressed(TArray<uint8>& buffer, int& offset, float value)
//	{
//		NetworkUtil::WriteToBuffer(buffer, offset, FFloat16(value));
//	}
//
//	static void _PreProcessCompressedValue(float& value)
//	{
//		value /= 100.0f;
//	}
//
//	static void _PostProcessCompressedValue(float& value)
//	{
//		value *= 100.0f;
//	}
//};
//
//// FVector is a more complex type requiring a bit of specialization
//// The X, Y, and Z values as serialized as floats.
//// The _IsOverThresholdDistance method allows thresholds to use distance.
//// By default the receive and snap thresholds use _IsOverThresholdDistance
//// The other thresholds instead check if any of the X, Y, or Z components
//// are individually over the threshold value.
//template<>
//class SMOOTHSYNCPLUGIN_API Syncable<FVector> : public ISyncable<FVector>
//{
//public:
//	bool syncX, syncY, syncZ;
//
//	Syncable(float easing = 1, bool isCompressed = false, SyncMode syncMode = SyncMode::XYZ) : ISyncable<FVector>(easing, isCompressed)
//	{
//		syncX = (syncMode & SyncMode::X) != SyncMode::NONE;
//		syncY = (syncMode & SyncMode::Y) != SyncMode::NONE;
//		syncZ = (syncMode & SyncMode::Z) != SyncMode::NONE;
//		IsOverReceiveThreshold = [this](FVector oldValue, FVector newValue) {
//			return _IsOverThresholdDistance(oldValue, newValue, receiveThreshold);
//		};
//		IsOverSnapThreshold = [this](FVector oldValue, FVector newValue) {
//			if (snapThreshold == 0) return false;
//			return _IsOverThresholdDistance(oldValue, newValue, snapThreshold);
//		};
//		IsOverSendThreshold = [this](FVector oldValue, FVector newValue) {
//			return _IsOverThreshold(oldValue, newValue, sendThreshold);
//		};
//		IsOverRestThreshold = [this](FVector oldValue, FVector newValue) {
//			if (!canRest) return false;
//			return _IsOverThreshold(oldValue, newValue, restValueThreshold);
//		};
//		SerializeValue = [this](TArray<uint8>& buffer, int& offset, FVector value) {
//			_SerializeValue(buffer, offset, value);
//		};
//		DeserializeValue = [this](TArray<uint8>& buffer, int& offset, FVector* value) {
//			_DeserializeValue(buffer, offset, value);
//		};
//		SerializeCompressedValue = [this](TArray<uint8>& buffer, int& offset, FVector value) {
//			_SerializeCompressed(buffer, offset, value);
//		};
//		DeserializeCompressedValue = [this](TArray<uint8>& buffer, int& offset, FVector* value) {
//			_DeserializeCompressed(buffer, offset, value);
//		};
//		PreProcessCompressedValue = _PreProcessCompressedValue;
//		PostProcessCompressedValue = _PostProcessCompressedValue;
//
//		LogValue = [this](FVector v) { UE_LOG(LogTemp, Warning, TEXT("%s: %s"), *propertyName, *v.ToString()); };
//	}
//
//	~Syncable() { }
//
//	static bool _IsOverThreshold(FVector oldValue, FVector newValue, float threshold)
//	{
//		if (threshold == 0) return true;
//
//		if (FMath::Abs(newValue.X - oldValue.X) > threshold) return true;
//		if (FMath::Abs(newValue.Y - oldValue.Y) > threshold) return true;
//		if (FMath::Abs(newValue.Z - oldValue.Z) > threshold) return true;
//		return false;
//	}
//
//	static bool _IsOverThresholdDistance(FVector oldValue, FVector newValue, float threshold)
//	{
//		if (threshold == 0) return true;
//		float delta = FVector::Distance(newValue, oldValue);
//		return delta > threshold;
//	}
//
//	void _DeserializeValue(TArray<uint8>& buffer, int& offset, FVector* thing)
//	{
//		// Start by getting the current values as a default so that variables we are not syncing won't change
//		(*thing) = GetValue();
//		// Deserialize x, y, z
//		if (syncX) NetworkUtil::ReadFromBuffer<float>(buffer, offset, &(thing->X));
//		if (syncY) NetworkUtil::ReadFromBuffer<float>(buffer, offset, &(thing->Y));
//		if (syncZ) NetworkUtil::ReadFromBuffer<float>(buffer, offset, &(thing->Z));
//	}
//
//	void _SerializeValue(TArray<uint8>& buffer, int& offset, FVector value)
//	{
//		if (syncX) NetworkUtil::WriteToBuffer(buffer, offset, value.X);
//		if (syncY) NetworkUtil::WriteToBuffer(buffer, offset, value.Y);
//		if (syncZ) NetworkUtil::WriteToBuffer(buffer, offset, value.Z);
//	}
//
//	void _DeserializeCompressed(TArray<uint8>& buffer, int& offset, FVector* thing)
//	{
//		// Start by getting the current values as a default so that variables we are not syncing won't change
//		(*thing) = GetValue();
//		FFloat16 temp;
//		if (syncX)
//		{
//			NetworkUtil::ReadFromBuffer(buffer, offset, &(temp));
//			thing->X = float(temp);
//		}
//		if (syncY)
//		{
//			NetworkUtil::ReadFromBuffer(buffer, offset, &(temp));
//			thing->Y = float(temp);
//		}
//		if (syncZ)
//		{
//			NetworkUtil::ReadFromBuffer(buffer, offset, &(temp));
//			thing->Z = float(temp);
//		}
//	}
//
//	void _SerializeCompressed(TArray<uint8>& buffer, int& offset, FVector value)
//	{
//		if (syncX) NetworkUtil::WriteToBuffer(buffer, offset, FFloat16(value.X));
//		if (syncY) NetworkUtil::WriteToBuffer(buffer, offset, FFloat16(value.Y));
//		if (syncZ) NetworkUtil::WriteToBuffer(buffer, offset, FFloat16(value.Z));
//	}
//
//	static void _PreProcessCompressedValue(FVector& value)
//	{
//		value /= 100.0f;
//	}
//
//	static void _PostProcessCompressedValue(FVector& value)
//	{
//		value *= 100.0f;
//	}
//};
//
//// FQuat is a more complex type requiring a bit of specialization
//// The X, Y, and Z values as serialized as floats.
//// The _IsOverThresholdDistance method allows thresholds to use distance.
//// By default the receive and snap thresholds use _IsOverThresholdDistance
//// The other thresholds instead check if any of the X, Y, or Z components
//// are individually over the threshold value.
//template<>
//class Syncable<FQuat> : public ISyncable<FQuat>
//{
//public:
//	bool syncX, syncY, syncZ;
//
//	Syncable(float easing = 1, bool isCompressed = false, SyncMode syncMode = SyncMode::XYZ) : ISyncable<FQuat>(easing, isCompressed)
//	{
//		syncX = (syncMode & SyncMode::X) != SyncMode::NONE;
//		syncY = (syncMode & SyncMode::Y) != SyncMode::NONE;
//		syncZ = (syncMode & SyncMode::Z) != SyncMode::NONE;
//
//		IsOverReceiveThreshold = [this](FQuat oldValue, FQuat newValue) {
//			return _IsOverThresholdAngle(oldValue, newValue, receiveThreshold);
//		};
//		IsOverSnapThreshold = [this](FQuat oldValue, FQuat newValue) {
//			if (snapThreshold == 0) return false;
//			return _IsOverThresholdAngle(oldValue, newValue, snapThreshold);
//		};
//		IsOverSendThreshold = [this](FQuat oldValue, FQuat newValue) {
//			return _IsOverThreshold(oldValue, newValue, sendThreshold);
//		};
//		IsOverRestThreshold = [this](FQuat oldValue, FQuat newValue) {
//			return _IsOverThreshold(oldValue, newValue, restValueThreshold);
//		};
//		SerializeValue = [this](TArray<uint8>& buffer, int& offset, FQuat value) {
//			_SerializeValue(buffer, offset, value);
//		};
//		DeserializeValue = [this](TArray<uint8>& buffer, int& offset, FQuat* value) {
//			_DeserializeValue(buffer, offset, value);
//		};
//		SerializeCompressedValue = [this](TArray<uint8>& buffer, int& offset, FQuat value) {
//			_SerializeCompressed(buffer, offset, value);
//		};
//		DeserializeCompressedValue = [this](TArray<uint8>& buffer, int& offset, FQuat* value) {
//			_DeserializeCompressed(buffer, offset, value);
//		};
//		PreProcessCompressedValue = _PreProcessCompressedValue;
//		PostProcessCompressedValue = _PostProcessCompressedValue;
//
//		LogValue = [this](FQuat v) { UE_LOG(LogTemp, Warning, TEXT("%s: %s"), *propertyName, *v.ToString()); };
//	}
//
//	static bool _IsOverThreshold(FQuat oldValue, FQuat newValue, float threshold)
//	{
//		if (threshold == 0) return true;
//		FVector oldValueEuler = oldValue.Euler();
//		FVector newValueEuler = newValue.Euler();
//		if (FMath::Abs(newValueEuler.X - oldValueEuler.X) > threshold) return true;
//		if (FMath::Abs(newValueEuler.Y - oldValueEuler.Y) > threshold) return true;
//		if (FMath::Abs(newValueEuler.Z - oldValueEuler.Z) > threshold) return true;
//		return false;
//	}
//
//	static bool _IsOverThresholdAngle(FQuat oldValue, FQuat newValue, float threshold)
//	{
//		if (threshold == 0) return true;
//		auto angleDifference = oldValue.AngularDistance(newValue);
//		angleDifference *= (180.0f / PI);
//		if (angleDifference > threshold) return true;
//		return false;
//	}
//
//	void _DeserializeValue(TArray<uint8>& buffer, int& offset, FQuat* thing)
//	{
//		// Start by getting the current values as a default so that variables we are not syncing won't change
//		FVector euler = GetValue().Euler();
//		// Deserialize x, y, z
//		if (syncX) NetworkUtil::ReadFromBuffer<float>(buffer, offset, &(euler.X));
//		if (syncY) NetworkUtil::ReadFromBuffer<float>(buffer, offset, &(euler.Y));
//		if (syncZ) NetworkUtil::ReadFromBuffer<float>(buffer, offset, &(euler.Z));
//		(*thing) = FQuat::MakeFromEuler(euler);
//	}
//
//	void _SerializeValue(TArray<uint8>& buffer, int& offset, FQuat value)
//	{
//		FVector eulerValue = value.Euler();
//		if (syncX) NetworkUtil::WriteToBuffer(buffer, offset, eulerValue.X);
//		if (syncY) NetworkUtil::WriteToBuffer(buffer, offset, eulerValue.Y);
//		if (syncZ) NetworkUtil::WriteToBuffer(buffer, offset, eulerValue.Z);
//	}
//
//	void _DeserializeCompressed(TArray<uint8>& buffer, int& offset, FQuat* thing)
//	{
//		// Start by getting the current values as a default so that variables we are not syncing won't change
//		FVector euler = GetValue().Euler();
//		FFloat16 temp;
//		if (syncX)
//		{
//			NetworkUtil::ReadFromBuffer(buffer, offset, &(temp));
//			euler.X = float(temp);
//		}
//		if (syncY)
//		{
//			NetworkUtil::ReadFromBuffer(buffer, offset, &(temp));
//			euler.Y = float(temp);
//		}
//		if (syncZ)
//		{
//			NetworkUtil::ReadFromBuffer(buffer, offset, &(temp));
//			euler.Z = float(temp);
//		}
//		(*thing) = FQuat::MakeFromEuler(euler);
//	}
//
//	void _SerializeCompressed(TArray<uint8>& buffer, int& offset, FQuat value)
//	{
//		FVector eulerValue = value.Euler();
//		if (syncX) NetworkUtil::WriteToBuffer(buffer, offset, FFloat16(eulerValue.X));
//		if (syncY) NetworkUtil::WriteToBuffer(buffer, offset, FFloat16(eulerValue.Y));
//		if (syncZ) NetworkUtil::WriteToBuffer(buffer, offset, FFloat16(eulerValue.Z));
//	}
//
//	static void _PreProcessCompressedValue(FQuat& value)
//	{
//		value /= 100.0f;
//	}
//
//	static void _PostProcessCompressedValue(FQuat& value)
//	{
//		value *= 100.0f;
//	}
//
//};
//
//class SyncableCollection
//{
//public:
//
//	TMap<FString, Syncable<char>*> syncableChars;
//	TMap<FString, Syncable<short>*> syncableShorts;
//	TMap<FString, Syncable<int>*> syncableInts;
//	TMap<FString, Syncable<long>*> syncableLongs;
//	TMap<FString, Syncable<FFloat16>*> syncableHalfs;
//	TMap<FString, Syncable<float>*> syncableFloats;
//	TMap<FString, Syncable<double>*> syncableDoubles;
//	TMap<FString, Syncable<FVector>*> syncableVectors;
//	TMap<FString, Syncable<FQuat>*> syncableQuats;
//
//	bool ShouldSendAny()
//	{
//		for (auto kv : syncableChars) if (kv.Value->ShouldSend()) return true;
//		for (auto kv : syncableShorts) if (kv.Value->ShouldSend()) return true;
//		for (auto kv : syncableInts) if (kv.Value->ShouldSend()) return true;
//		for (auto kv : syncableLongs) if (kv.Value->ShouldSend()) return true;
//		for (auto kv : syncableHalfs) if (kv.Value->ShouldSend()) return true;
//		for (auto kv : syncableFloats) if (kv.Value->ShouldSend()) return true;
//		for (auto kv : syncableDoubles) if (kv.Value->ShouldSend()) return true;
//		for (auto kv : syncableVectors) if (kv.Value->ShouldSend()) return true;
//		for (auto kv : syncableQuats) if (kv.Value->ShouldSend()) return true;
//		return false;
//	}
//
//	void AddSyncable(FString name, Syncable<char>* syncable) 
//	{ 
//		syncable->propertyName = name;
//		syncableChars.Add(name, syncable); 
//	}
//	void AddSyncable(FString name, Syncable<short>* syncable) 
//	{ 
//		syncable->propertyName = name;
//		syncableShorts.Add(name, syncable); 
//	}
//	void AddSyncable(FString name, Syncable<int>* syncable) 
//	{ 
//		syncable->propertyName = name;
//		syncableInts.Add(name, syncable); 
//	}
//	void AddSyncable(FString name, Syncable<long>* syncable) 
//	{
//		syncable->propertyName = name;
//		syncableLongs.Add(name, syncable); 
//	}
//	void AddSyncable(FString name, Syncable<FFloat16>* syncable) 
//	{ 
//		syncable->propertyName = name;
//		syncableHalfs.Add(name, syncable); 
//	}
//	void AddSyncable(FString name, Syncable<float>* syncable) 
//	{ 
//		syncable->propertyName = name;
//		syncableFloats.Add(name, syncable); 
//	}
//	void AddSyncable(FString name, Syncable<double>* syncable) 
//	{
//		syncable->propertyName = name;
//		syncableDoubles.Add(name, syncable); 
//	}
//	void AddSyncable(FString name, Syncable<FVector>* syncable) 
//	{
//		syncable->propertyName = name;
//		syncableVectors.Add(name, syncable); 
//	}
//	void AddSyncable(FString name, Syncable<FQuat>* syncable) 
//	{
//		syncable->propertyName = name;
//		syncableQuats.Add(name, syncable); 
//	}
//
//	template<class T> Syncable<T>* GetSyncable(FString name) { return nullptr; }
//	template<> Syncable<char>* GetSyncable(FString name) { return syncableChars[name]; }
//	template<> Syncable<short>* GetSyncable(FString name) { return syncableShorts[name]; }
//	template<> Syncable<int>* GetSyncable(FString name) { return syncableInts[name]; }
//	template<> Syncable<long>* GetSyncable(FString name) { return syncableLongs[name]; }
//	template<> Syncable<FFloat16>* GetSyncable(FString name) { return syncableHalfs[name]; }
//	template<> Syncable<float>* GetSyncable(FString name) { return syncableFloats[name]; }
//	template<> Syncable<double>* GetSyncable(FString name) { return syncableDoubles[name]; }
//	template<> Syncable<FVector>* GetSyncable(FString name) { return syncableVectors[name]; }
//	template<> Syncable<FQuat>* GetSyncable(FString name) { return syncableQuats[name]; }
//
//	void Tick(float deltaTime, bool& forceSend)
//	{
//		for (auto kv : syncableChars) kv.Value->Tick(deltaTime, forceSend);
//		for (auto kv : syncableShorts) kv.Value->Tick(deltaTime, forceSend);
//		for (auto kv : syncableInts) kv.Value->Tick(deltaTime, forceSend);
//		for (auto kv : syncableLongs) kv.Value->Tick(deltaTime, forceSend);
//		for (auto kv : syncableHalfs) kv.Value->Tick(deltaTime, forceSend);
//		for (auto kv : syncableFloats) kv.Value->Tick(deltaTime, forceSend);
//		for (auto kv : syncableDoubles) kv.Value->Tick(deltaTime, forceSend);
//		for (auto kv : syncableVectors) kv.Value->Tick(deltaTime, forceSend);
//		for (auto kv : syncableQuats) kv.Value->Tick(deltaTime, forceSend);
//	}
//
//	void UpdatePreviousValues()
//	{
//		for (auto kv : syncableChars) kv.Value->UpdatePreviousValue();
//		for (auto kv : syncableShorts) kv.Value->UpdatePreviousValue();
//		for (auto kv : syncableInts) kv.Value->UpdatePreviousValue();
//		for (auto kv : syncableLongs) kv.Value->UpdatePreviousValue();
//		for (auto kv : syncableHalfs) kv.Value->UpdatePreviousValue();
//		for (auto kv : syncableFloats) kv.Value->UpdatePreviousValue();
//		for (auto kv : syncableDoubles) kv.Value->UpdatePreviousValue();
//		for (auto kv : syncableVectors) kv.Value->UpdatePreviousValue();
//		for (auto kv : syncableQuats) kv.Value->UpdatePreviousValue();
//	}
//
//	bool AnyAtRest()
//	{
//		if (Any<char>(syncableChars, IsAtRest<char>)) return true;
//		if (Any<short>(syncableShorts, IsAtRest<short>)) return true;
//		if (Any<int>(syncableInts, IsAtRest<int>)) return true;
//		if (Any<long>(syncableLongs, IsAtRest<long>)) return true;
//		if (Any<FFloat16>(syncableHalfs, IsAtRest<FFloat16>)) return true;
//		if (Any<float>(syncableFloats, IsAtRest<float>)) return true;
//		if (Any<double>(syncableDoubles, IsAtRest<double>)) return true;
//		if (Any<FVector>(syncableVectors, IsAtRest<FVector>)) return true;
//		if (Any<FQuat>(syncableQuats, IsAtRest<FQuat>)) return true;
//		return false;
//	}
//
//	bool AnyCanRest()
//	{
//		if (Any<char>(syncableChars, CanRest<char>)) return true;
//		if (Any<short>(syncableShorts, CanRest<short>)) return true;
//		if (Any<int>(syncableInts, CanRest<int>)) return true;
//		if (Any<long>(syncableLongs, CanRest<long>)) return true;
//		if (Any<FFloat16>(syncableHalfs, CanRest<FFloat16>)) return true;
//		if (Any<float>(syncableFloats, CanRest<float>)) return true;
//		if (Any<double>(syncableDoubles, CanRest<double>)) return true;
//		if (Any<FVector>(syncableVectors, CanRest<FVector>)) return true;
//		if (Any<FQuat>(syncableQuats, CanRest<FQuat>)) return true;
//		return false;
//	}
//
//	bool AllAtRest()
//	{
//		if (AnyCanRest())
//		{
//			if (!All<char>(syncableChars, IsAtRest<char>)) return false;
//			if (!All<short>(syncableShorts, IsAtRest<short>)) return false;
//			if (!All<int>(syncableInts, IsAtRest<int>)) return false;
//			if (!All<long>(syncableLongs, IsAtRest<long>)) return false;
//			if (!All<FFloat16>(syncableHalfs, IsAtRest<FFloat16>)) return false;
//			if (!All<float>(syncableFloats, IsAtRest<float>)) return false;
//			if (!All<double>(syncableDoubles, IsAtRest<double>)) return false;
//			if (!All<FVector>(syncableVectors, IsAtRest<FVector>)) return false;
//			if (!All<FQuat>(syncableQuats, IsAtRest<FQuat>)) return false;
//		}
//		return true;
//	}
//
//	bool AnyJustStartedMoving()
//	{
//		if (Any<char>(syncableChars, JustStartedMoving<char>)) return true;
//		if (Any<short>(syncableShorts, JustStartedMoving<short>)) return true;
//		if (Any<int>(syncableInts, JustStartedMoving<int>)) return true;
//		if (Any<long>(syncableLongs, JustStartedMoving<long>)) return true;
//		if (Any<FFloat16>(syncableHalfs, JustStartedMoving<FFloat16>)) return true;
//		if (Any<float>(syncableFloats, JustStartedMoving<float>)) return true;
//		if (Any<double>(syncableDoubles, JustStartedMoving<double>)) return true;
//		if (Any<FVector>(syncableVectors, JustStartedMoving<FVector>)) return true;
//		if (Any<FQuat>(syncableQuats, JustStartedMoving<FQuat>)) return true;
//		return false;
//	}
//
//	void Teleport()
//	{
//		for (auto kv : syncableChars) TeleportSyncable(kv.Value);
//		for (auto kv : syncableShorts) TeleportSyncable(kv.Value);
//		for (auto kv : syncableInts) TeleportSyncable(kv.Value);
//		for (auto kv : syncableLongs) TeleportSyncable(kv.Value);
//		for (auto kv : syncableHalfs) TeleportSyncable(kv.Value);
//		for (auto kv : syncableFloats) TeleportSyncable(kv.Value);
//		for (auto kv : syncableDoubles) TeleportSyncable(kv.Value);
//		for (auto kv : syncableVectors) TeleportSyncable(kv.Value);
//		for (auto kv : syncableQuats) TeleportSyncable(kv.Value);
//	}
//
//	void WriteEncodingBytes(TArray<uint8>& buffer, int& offset)
//	{
//		char encodingByte = 0;
//		char mask = 1;
//		bool hasWrittenAnything = false;
//
//		hasWrittenAnything |= WriteShouldSyncBytes(buffer, offset, syncableChars, encodingByte, mask);
//		hasWrittenAnything |= WriteShouldSyncBytes(buffer, offset, syncableShorts, encodingByte, mask);
//		hasWrittenAnything |= WriteShouldSyncBytes(buffer, offset, syncableInts, encodingByte, mask);
//		hasWrittenAnything |= WriteShouldSyncBytes(buffer, offset, syncableLongs, encodingByte, mask);
//		hasWrittenAnything |= WriteShouldSyncBytes(buffer, offset, syncableHalfs, encodingByte, mask);
//		hasWrittenAnything |= WriteShouldSyncBytes(buffer, offset, syncableFloats, encodingByte, mask);
//		hasWrittenAnything |= WriteShouldSyncBytes(buffer, offset, syncableDoubles, encodingByte, mask);
//		hasWrittenAnything |= WriteShouldSyncBytes(buffer, offset, syncableVectors, encodingByte, mask);
//		hasWrittenAnything |= WriteShouldSyncBytes(buffer, offset, syncableQuats, encodingByte, mask);
//
//		hasWrittenAnything |= WriteRestBytes(buffer, offset, syncableChars, encodingByte, mask);
//		hasWrittenAnything |= WriteRestBytes(buffer, offset, syncableShorts, encodingByte, mask);
//		hasWrittenAnything |= WriteRestBytes(buffer, offset, syncableInts, encodingByte, mask);
//		hasWrittenAnything |= WriteRestBytes(buffer, offset, syncableLongs, encodingByte, mask);
//		hasWrittenAnything |= WriteRestBytes(buffer, offset, syncableHalfs, encodingByte, mask);
//		hasWrittenAnything |= WriteRestBytes(buffer, offset, syncableFloats, encodingByte, mask);
//		hasWrittenAnything |= WriteRestBytes(buffer, offset, syncableDoubles, encodingByte, mask);
//		hasWrittenAnything |= WriteRestBytes(buffer, offset, syncableVectors, encodingByte, mask);
//		hasWrittenAnything |= WriteRestBytes(buffer, offset, syncableQuats, encodingByte, mask);
//
//		if (mask != 1 || !hasWrittenAnything)
//		{
//			NetworkUtil::WriteToBuffer(buffer, offset, encodingByte);
//		}
//	}
//
//	void Serialize(TArray<uint8>& buffer, int& offset, bool anyJustStartedMoving)
//	{
//		int syncableCount = 0;
//		SerializeList(syncableChars, buffer, offset, anyJustStartedMoving);
//		SerializeList(syncableShorts, buffer, offset, anyJustStartedMoving);
//		SerializeList(syncableInts, buffer, offset, anyJustStartedMoving);
//		SerializeList(syncableLongs, buffer, offset, anyJustStartedMoving);
//		SerializeList(syncableHalfs, buffer, offset, anyJustStartedMoving);
//		SerializeList(syncableFloats, buffer, offset, anyJustStartedMoving);
//		SerializeList(syncableDoubles, buffer, offset, anyJustStartedMoving);
//		SerializeList(syncableVectors, buffer, offset, anyJustStartedMoving);
//		SerializeList(syncableQuats, buffer, offset, anyJustStartedMoving);
//	}
//
//	template<class T>
//	static bool All(TMap<FString, Syncable<T>*>& syncables, std::function<bool(Syncable<T>*)> fn)
//	{
//		for (auto kv : syncables) if (!fn(kv.Value)) return false;
//		return true;
//	}
//
//	template<class T>
//	static bool Any(TMap<FString, Syncable<T>*>& syncables, std::function<bool(Syncable<T>*)> fn)
//	{
//		for (auto kv : syncables) if (fn(kv.Value)) return true;
//		return false;
//	}
//
//	template<class T>
//	static bool IsAtRest(Syncable<T>* syncable)
//	{
//		return !syncable->canRest || syncable->restState == RestState::AT_REST;
//	}
//
//	template<class T>
//	static bool JustStartedMoving(Syncable<T>* syncable)
//	{
//		return syncable->restState == RestState::JUST_STARTED_MOVING;
//	}
//
//	template<class T>
//	static bool CanRest(Syncable<T>* syncable)
//	{
//		return syncable->canRest;
//	}
//
//	template<class T>
//	static void TeleportSyncable(Syncable<T>* syncable)
//	{
//		syncable->Teleport();
//	}
//
//	template<class T>
//	bool WriteShouldSyncBytes(TArray<uint8>& buffer, int& offset, TMap<FString, Syncable<T>*>& list, char& encodingByte, char& mask)
//	{
//		bool hasWrittenAnything = false;
//		for (auto kv : list)
//		{
//			if (kv.Value->ShouldSend())
//			{
//				encodingByte = (char)(encodingByte | mask);
//			}
//			mask *= 2;
//			if (mask == 256)
//			{
//				NetworkUtil::WriteToBuffer(buffer, offset, encodingByte);
//				hasWrittenAnything = true;
//				encodingByte = 0;
//				mask = 1;
//			}
//		}
//
//		return hasWrittenAnything;
//	}
//
//	template<class T>
//	bool WriteRestBytes(TArray<uint8>& buffer, int& offset, TMap<FString, Syncable<T>*>& list, char& encodingByte, char& mask)
//	{
//		bool hasWrittenAnything = false;
//		for (auto kv : list)
//		{
//			auto syncable = kv.Value;
//
//			if (!syncable->canRest || syncable->IsOverRestThreshold == nullptr) continue;
//			if (syncable->restValueThreshold == 0 && syncable->restTimeThreshold == 0) continue;
//
//			if (syncable->ShouldSendRestMessage())
//			{
//				encodingByte = (char)(encodingByte | mask);
//			}
//			mask *= 2;
//			if (mask == 256)
//			{
//				NetworkUtil::WriteToBuffer(buffer, offset, encodingByte);
//				hasWrittenAnything = true;
//				encodingByte = 0;
//				mask = 1;
//			}
//		}
//		return hasWrittenAnything;
//	}
//
//	template<class T>
//	void SerializeList(TMap<FString, Syncable<T>*>& list, TArray<uint8>& buffer, int& offset, bool anyJustStartedMoving)
//	{
//		for (auto kv : list)
//		{
//			auto syncable = kv.Value;
//
//			if (syncable->ShouldSend())
//			{
//				if (anyJustStartedMoving)
//				{
//					// Use previous value if anything just started moving
//					syncable->Serialize(buffer, offset, syncable->valueLastFrame);
//				}
//				else
//				{
//					// Use current value
//					syncable->Serialize(buffer, offset, syncable->GetValue());
//				}
//			}
//		}
//	}
//
//	void CleanUp()
//	{
//		for (auto kv : syncableChars) delete kv.Value;
//		for (auto kv : syncableShorts) delete kv.Value;
//		for (auto kv : syncableInts) delete kv.Value;
//		for (auto kv : syncableLongs) delete kv.Value;
//		for (auto kv : syncableHalfs) delete kv.Value;
//		for (auto kv : syncableFloats) delete kv.Value;
//		for (auto kv : syncableDoubles) delete kv.Value;
//		for (auto kv : syncableVectors) delete kv.Value;
//		for (auto kv : syncableQuats) delete kv.Value;
//
//		syncableChars.Empty();
//		syncableShorts.Empty();
//		syncableInts.Empty();
//		syncableLongs.Empty();
//		syncableHalfs.Empty();
//		syncableFloats.Empty();
//		syncableDoubles.Empty();
//		syncableVectors.Empty();
//		syncableQuats.Empty();
//	}
//};
//
//class SyncableValueCollection
//{
//public:
//
//	TMap<FString, SyncableValue<char>*> syncableCharValues;
//	TMap<FString, SyncableValue<short>*> syncableShortValues;
//	TMap<FString, SyncableValue<int>*> syncableIntValues;
//	TMap<FString, SyncableValue<long>*> syncableLongValues;
//	TMap<FString, SyncableValue<FFloat16>*> syncableHalfValues;
//	TMap<FString, SyncableValue<float>*> syncableFloatValues;
//	TMap<FString, SyncableValue<double>*> syncableDoubleValues;
//	TMap<FString, SyncableValue<FVector>*> syncableVectorValues;
//	TMap<FString, SyncableValue<FQuat>*> syncableQuatValues;
//
//	void Init(SyncableCollection& syncableCollection)
//	{
//		InitSyncableValues(syncableCollection.syncableChars, syncableCharValues);
//		InitSyncableValues(syncableCollection.syncableShorts, syncableShortValues);
//		InitSyncableValues(syncableCollection.syncableInts, syncableIntValues);
//		InitSyncableValues(syncableCollection.syncableLongs, syncableLongValues);
//		InitSyncableValues(syncableCollection.syncableHalfs, syncableHalfValues);
//		InitSyncableValues(syncableCollection.syncableFloats, syncableFloatValues);
//		InitSyncableValues(syncableCollection.syncableDoubles, syncableDoubleValues);
//		InitSyncableValues(syncableCollection.syncableVectors, syncableVectorValues);
//		InitSyncableValues(syncableCollection.syncableQuats, syncableQuatValues);
//	}
//
//	void ReadEncodingBytes(TArray<uint8>& buffer, int& offset, TArray<bool>& shouldSync)
//	{
//		char encodingByte;
//		NetworkUtil::ReadFromBuffer(buffer, offset, &encodingByte);
//		char mask = 1;
//
//		ReadShouldSyncBytes(buffer, offset, syncableCharValues, encodingByte, mask, shouldSync);
//		ReadShouldSyncBytes(buffer, offset, syncableShortValues, encodingByte, mask, shouldSync);
//		ReadShouldSyncBytes(buffer, offset, syncableIntValues, encodingByte, mask, shouldSync);
//		ReadShouldSyncBytes(buffer, offset, syncableLongValues, encodingByte, mask, shouldSync);
//		ReadShouldSyncBytes(buffer, offset, syncableHalfValues, encodingByte, mask, shouldSync);
//		ReadShouldSyncBytes(buffer, offset, syncableFloatValues, encodingByte, mask, shouldSync);
//		ReadShouldSyncBytes(buffer, offset, syncableDoubleValues, encodingByte, mask, shouldSync);
//		ReadShouldSyncBytes(buffer, offset, syncableVectorValues, encodingByte, mask, shouldSync);
//		ReadShouldSyncBytes(buffer, offset, syncableQuatValues, encodingByte, mask, shouldSync);
//
//		ReadRestBytes(buffer, offset, syncableCharValues, encodingByte, mask);
//		ReadRestBytes(buffer, offset, syncableShortValues, encodingByte, mask);
//		ReadRestBytes(buffer, offset, syncableIntValues, encodingByte, mask);
//		ReadRestBytes(buffer, offset, syncableLongValues, encodingByte, mask);
//		ReadRestBytes(buffer, offset, syncableHalfValues, encodingByte, mask);
//		ReadRestBytes(buffer, offset, syncableFloatValues, encodingByte, mask);
//		ReadRestBytes(buffer, offset, syncableDoubleValues, encodingByte, mask);
//		ReadRestBytes(buffer, offset, syncableVectorValues, encodingByte, mask);
//		ReadRestBytes(buffer, offset, syncableQuatValues, encodingByte, mask);
//	}
//
//	void Deserialize(TArray<uint8>& buffer, int& offset, TArray<bool>& shouldSync)
//	{
//		int syncableCount = 0;
//		DeserializeList(syncableCharValues, buffer, offset, syncableCount, shouldSync);
//		DeserializeList(syncableShortValues, buffer, offset, syncableCount, shouldSync);
//		DeserializeList(syncableIntValues, buffer, offset, syncableCount, shouldSync);
//		DeserializeList(syncableLongValues, buffer, offset, syncableCount, shouldSync);
//		DeserializeList(syncableHalfValues, buffer, offset, syncableCount, shouldSync);
//		DeserializeList(syncableFloatValues, buffer, offset, syncableCount, shouldSync);
//		DeserializeList(syncableDoubleValues, buffer, offset, syncableCount, shouldSync);
//		DeserializeList(syncableVectorValues, buffer, offset, syncableCount, shouldSync);
//		DeserializeList(syncableQuatValues, buffer, offset, syncableCount, shouldSync);
//	}
//
//	void ApplyValues(bool dontEase = false)
//	{
//		for (auto kv : syncableCharValues) kv.Value->Apply(dontEase);
//		for (auto kv : syncableShortValues) kv.Value->Apply(dontEase);
//		for (auto kv : syncableIntValues) kv.Value->Apply(dontEase);
//		for (auto kv : syncableLongValues) kv.Value->Apply(dontEase);
//		for (auto kv : syncableHalfValues) kv.Value->Apply(dontEase);
//		for (auto kv : syncableFloatValues) kv.Value->Apply(dontEase);
//		for (auto kv : syncableDoubleValues) kv.Value->Apply(dontEase);
//		for (auto kv : syncableVectorValues) kv.Value->Apply(dontEase);
//		for (auto kv : syncableQuatValues) kv.Value->Apply(dontEase);
//	}
//
//	bool AnyAtRest()
//	{
//		if (Any<char>(syncableCharValues, IsAtRest<char>)) return true;
//		if (Any<short>(syncableShortValues, IsAtRest<short>)) return true;
//		if (Any<int>(syncableIntValues, IsAtRest<int>)) return true;
//		if (Any<long>(syncableLongValues, IsAtRest<long>)) return true;
//		if (Any<FFloat16>(syncableHalfValues, IsAtRest<FFloat16>)) return true;
//		if (Any<float>(syncableFloatValues, IsAtRest<float>)) return true;
//		if (Any<double>(syncableDoubleValues, IsAtRest<double>)) return true;
//		if (Any<FVector>(syncableVectorValues, IsAtRest<FVector>)) return true;
//		if (Any<FQuat>(syncableQuatValues, IsAtRest<FQuat>)) return true;
//		return false;
//	}
//
//	bool AllAtRest()
//	{
//		if (!All<char>(syncableCharValues, IsAtRest<char>)) return false;
//		if (!All<short>(syncableShortValues, IsAtRest<short>)) return false;
//		if (!All<int>(syncableIntValues, IsAtRest<int>)) return false;
//		if (!All<long>(syncableLongValues, IsAtRest<long>)) return false;
//		if (!All<FFloat16>(syncableHalfValues, IsAtRest<FFloat16>)) return false;
//		if (!All<float>(syncableFloatValues, IsAtRest<float>)) return false;
//		if (!All<double>(syncableDoubleValues, IsAtRest<double>)) return false;
//		if (!All<FVector>(syncableVectorValues, IsAtRest<FVector>)) return false;
//		if (!All<FQuat>(syncableQuatValues, IsAtRest<FQuat>)) return false;;
//		return true;
//	}
//
//	void Interpolate(SyncableValueCollection& start, SyncableValueCollection& end, float t)
//	{
//		Interpolate(start.syncableCharValues, end.syncableCharValues, syncableCharValues, t);
//		Interpolate(start.syncableShortValues, end.syncableShortValues, syncableShortValues, t);
//		Interpolate(start.syncableIntValues, end.syncableIntValues, syncableIntValues, t);
//		Interpolate(start.syncableLongValues, end.syncableLongValues, syncableLongValues, t);
//		Interpolate(start.syncableHalfValues, end.syncableHalfValues, syncableHalfValues, t);
//		Interpolate(start.syncableFloatValues, end.syncableFloatValues, syncableFloatValues, t);
//		Interpolate(start.syncableDoubleValues, end.syncableDoubleValues, syncableDoubleValues, t);
//		Interpolate(start.syncableVectorValues, end.syncableVectorValues, syncableVectorValues, t);
//		Interpolate(start.syncableQuatValues, end.syncableQuatValues, syncableQuatValues, t);
//	}
//
//	void SetAllDefault()
//	{
//		for (auto kv : syncableCharValues) kv.Value->value = 0;
//		for (auto kv : syncableShortValues) kv.Value->value = 0;
//		for (auto kv : syncableIntValues) kv.Value->value = 0;
//		for (auto kv : syncableLongValues) kv.Value->value = 0;
//		for (auto kv : syncableHalfValues) kv.Value->value = 0;
//		for (auto kv : syncableFloatValues) kv.Value->value = 0;
//		for (auto kv : syncableDoubleValues) kv.Value->value = 0;
//		for (auto kv : syncableVectorValues) kv.Value->value = FVector::ZeroVector;
//		for (auto kv : syncableQuatValues) kv.Value->value = FQuat::Identity;
//	}
//
//	void CopyLocalValues()
//	{
//		CopyLocalValues(syncableCharValues);
//		CopyLocalValues(syncableShortValues);
//		CopyLocalValues(syncableIntValues);
//		CopyLocalValues(syncableLongValues);
//		CopyLocalValues(syncableHalfValues);
//		CopyLocalValues(syncableFloatValues);
//		CopyLocalValues(syncableDoubleValues);
//		CopyLocalValues(syncableVectorValues);
//		CopyLocalValues(syncableQuatValues);
//	}
//
//	void CopyFrom(SyncableValueCollection& source)
//	{
//		CopyValues(syncableCharValues, source.syncableCharValues);
//		CopyValues(syncableShortValues, source.syncableShortValues);
//		CopyValues(syncableIntValues, source.syncableIntValues);
//		CopyValues(syncableLongValues, source.syncableLongValues);
//		CopyValues(syncableHalfValues, source.syncableHalfValues);
//		CopyValues(syncableFloatValues, source.syncableFloatValues);
//		CopyValues(syncableDoubleValues, source.syncableDoubleValues);
//		CopyValues(syncableVectorValues, source.syncableVectorValues);
//		CopyValues(syncableQuatValues, source.syncableQuatValues);
//	}
//
//	template<class T> SyncableValue<T>* GetSyncableValue(FString name)
//	{
//		return nullptr;
//	}
//	template<> SyncableValue<char>* GetSyncableValue(FString name)
//	{
//		if (syncableCharValues.Contains(name)) { return (SyncableValue<char>*)syncableCharValues[name]; }
//		else { return nullptr; }
//	}
//	template<> SyncableValue<short>* GetSyncableValue(FString name)
//	{
//		if (syncableShortValues.Contains(name)) { return (SyncableValue<short>*)syncableShortValues[name]; }
//		else { return nullptr; }
//	}
//	template<> SyncableValue<int>* GetSyncableValue(FString name)
//	{
//		if (syncableIntValues.Contains(name)) { return (SyncableValue<int>*)syncableIntValues[name]; }
//		else { return nullptr; }
//	}
//	template<> SyncableValue<long>* GetSyncableValue(FString name)
//	{
//		if (syncableLongValues.Contains(name)) { return (SyncableValue<long>*)syncableLongValues[name]; }
//		else { return nullptr; }
//	}
//	template<> SyncableValue<FFloat16>* GetSyncableValue(FString name)
//	{
//		if (syncableHalfValues.Contains(name)) { return (SyncableValue<FFloat16>*)syncableHalfValues[name]; }
//		else { return nullptr; }
//	}
//	template<> SyncableValue<float>* GetSyncableValue(FString name)
//	{
//		if (syncableFloatValues.Contains(name)) { return (SyncableValue<float>*)syncableFloatValues[name]; }
//		else { return nullptr; }
//	}
//	template<> SyncableValue<double>* GetSyncableValue(FString name)
//	{
//		if (syncableDoubleValues.Contains(name)) { return (SyncableValue<double>*)syncableDoubleValues[name]; }
//		else { return nullptr; }
//	}
//	template<> SyncableValue<FVector>* GetSyncableValue(FString name)
//	{
//		if (syncableVectorValues.Contains(name)) { return (SyncableValue<FVector>*)syncableVectorValues[name]; }
//		else { return nullptr; }
//	}
//	template<> SyncableValue<FQuat>* GetSyncableValue(FString name)
//	{
//		if (syncableQuatValues.Contains(name)) { return (SyncableValue<FQuat>*)syncableQuatValues[name]; }
//		else { return nullptr; }
//	}
//
//	template<class T>
//	void CopyLocalValues(TMap<FString, SyncableValue<T>*>& list)
//	{
//		for (auto kv : list)
//		{
//			auto syncableValue = kv.Value;
//			auto syncable = syncableValue->syncable;
//			auto value = syncable->GetValue();
//			syncableValue->value = value;
//		}
//	}
//
//	template<class T>
//	void DeserializeList(TMap<FString, SyncableValue<T>*>& list, TArray<uint8>& buffer, int& offset, int& syncableCount, TArray<bool>& shouldSync)
//	{
//		for (auto kv : list)
//		{
//			auto syncableValue = kv.Value;
//			if (shouldSync[syncableCount])
//			{
//				syncableValue->Deserialize(buffer, offset);
//			}
//			else
//			{
//				auto syncable = syncableValue->syncable;
//				if (syncable->hasEverDeserializedValue)
//				{
//					// Copy the value from the most recent state
//					syncableValue->value = syncable->valueLastDeserialized;
//				}
//				else
//				{
//					// If no states to copy from, then get the value from the local object
//					syncableValue->value = syncable->GetValue();
//				}
//			}
//			syncableCount++;
//		}
//	}
//
//	template<class T>
//	void InitSyncableValues(TMap<FString, Syncable<T>*>& syncablesByName,
//		TMap<FString, SyncableValue<T>*>& syncableValues)
//	{
//		for (auto kv : syncablesByName)
//		{
//			auto syncableValue = new SyncableValue<T>(kv.Value);
//			syncableValues.Add(kv.Key, syncableValue);
//		}
//	}
//
//	template<class T>
//	void ReadShouldSyncBytes(TArray<uint8>& buffer, int& offset, TMap<FString, SyncableValue<T>*>& list, char& encodingByte, char& mask, TArray<bool>& shouldSync)
//	{
//		for (auto kv : list)
//		{
//			if (mask == 256)
//			{
//				NetworkUtil::ReadFromBuffer(buffer, offset, &encodingByte);
//				mask = 1;
//			}
//			if ((encodingByte & mask) == mask)
//			{
//				shouldSync.Add(true);
//			}
//			else
//			{
//				shouldSync.Add(false);
//			}
//			mask *= 2;
//		}
//	}
//
//	template<class T>
//	void ReadRestBytes(TArray<uint8>& buffer, int& offset, TMap<FString, SyncableValue<T>*>& list, char& encodingByte, char& mask)
//	{
//		for (auto kv : list)
//		{
//			auto syncableValue = kv.Value;
//			auto syncable = syncableValue->syncable;
//			if (!syncable->canRest || syncable->IsOverRestThreshold == nullptr) continue;
//			if (syncable->restValueThreshold == 0 && syncable->restTimeThreshold == 0) continue;
//
//			if (mask == 256)
//			{
//				NetworkUtil::ReadFromBuffer(buffer, offset, &encodingByte);
//				mask = 1;
//			}
//			if ((encodingByte & mask) == mask)
//			{
//				syncableValue->isAtRest = true;
//			}
//			mask *= 2;
//		}
//	}
//
//	template<class T>
//	static bool All(TMap<FString, SyncableValue<T>*>& syncables, std::function<bool(SyncableValue<T>*)> fn)
//	{
//		for (auto kv : syncables) if (!fn(kv.Value)) return false;
//		return true;
//	}
//	template<class T>
//	static bool Any(TMap<FString, SyncableValue<T>*>& syncables, std::function<bool(SyncableValue<T>*)> fn)
//	{
//		for (auto kv : syncables) if (fn(kv.Value)) return true;
//		return false;
//	}
//
//	template<class T>
//	static bool IsAtRest(SyncableValue<T>* syncableValue)
//	{
//		return syncableValue->isAtRest;
//	}
//
//	// Interpolate all SyncableValues in a TArray.
//	// Interpolation is performed between each start value and end value based on t
//	// The results are stored in the value property of each entry of the provided results TArray.
//	// This is used to interpolate between states in the state buffer.
//	template<class T>
//	void Interpolate(TMap<FString, SyncableValue<T>*>& startValues, TMap<FString, SyncableValue<T>*>& endValues, TMap<FString, SyncableValue<T>*>& results, float t)
//	{
//		auto startIterator = startValues.CreateIterator();
//		auto endIterator = endValues.CreateIterator();
//		auto resultsIterator = results.CreateIterator();
//		while (startIterator && endIterator && resultsIterator)
//		{
//			auto startSyncableValue = startIterator.Value();
//			auto endSyncableValue = endIterator.Value();
//			auto resultSyncableValue = resultsIterator.Value();
//
//			resultSyncableValue->value = startSyncableValue->syncable->Interpolate(startSyncableValue->value, endSyncableValue->value, t);
//
//			++startIterator;
//			++endIterator;
//			++resultsIterator;
//		}
//	}
//
//	template<class T>
//	void CopyValues(TMap<FString, SyncableValue<T>*>& destination, TMap<FString, SyncableValue<T>*>& source)
//	{
//		auto destinationIterator = destination.CreateIterator();
//		auto sourceIterator = source.CreateIterator();
//		while (destinationIterator && sourceIterator)
//		{
//			auto destinationSyncableValue = destinationIterator.Value();
//			auto sourceSyncableValue = sourceIterator.Value();
//
//			destinationSyncableValue->value = sourceSyncableValue->value;
//
//			++destinationIterator;
//			++sourceIterator;
//		}
//	}
//
//	void CleanUp()
//	{
//		for (auto kv : syncableCharValues) delete kv.Value;
//		for (auto kv : syncableShortValues) delete kv.Value;
//		for (auto kv : syncableIntValues) delete kv.Value;
//		for (auto kv : syncableLongValues) delete kv.Value;
//		for (auto kv : syncableHalfValues) delete kv.Value;
//		for (auto kv : syncableFloatValues) delete kv.Value;
//		for (auto kv : syncableDoubleValues) delete kv.Value;
//		for (auto kv : syncableVectorValues) delete kv.Value;
//		for (auto kv : syncableQuatValues) delete kv.Value;
//
//		syncableCharValues.Empty();
//		syncableShortValues.Empty();
//		syncableIntValues.Empty();
//		syncableLongValues.Empty();
//		syncableHalfValues.Empty();
//		syncableFloatValues.Empty();
//		syncableDoubleValues.Empty();
//		syncableVectorValues.Empty();
//		syncableQuatValues.Empty();
//	}
//};
