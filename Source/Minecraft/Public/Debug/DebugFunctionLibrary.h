#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DebugFunctionLibrary.generated.h"



UENUM(BlueprintType)
enum class EObjectValidationResult : uint8
{
	Valid,
	Invalid
};



UCLASS()
class MINECRAFT_API UDebugFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Debug|Log", meta = (ExpandEnumAsExecs = "Result"))
	static void ValidateObject (	UObject* Object, const FString& ObjectLabel, EObjectValidationResult& Result );
};
