#include "Debug/DebugFunctionLibrary.h"

void UDebugFunctionLibrary::ValidateObject(UObject* Object, const FString& ObjectLabel, EObjectValidationResult& Result)
{
    if (IsValid(Object))
    {
        Result = EObjectValidationResult::Valid;
        return;
    }

    Result = EObjectValidationResult::Invalid;

#if !UE_BUILD_SHIPPING
    UE_LOG(LogTemp, Warning, TEXT("Object (%s) is invalid!"), *ObjectLabel);
#endif
}
