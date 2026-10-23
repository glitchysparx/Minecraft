#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "VoxelResources.generated.h"

class UTexture2D;

/** Resource types that can be stored in a player's inventory. */
UENUM(BlueprintType)
enum class EResourceType : uint8
{
	None UMETA(DisplayName = "None"),
	Rock UMETA(DisplayName = "Rock"),
	Grass UMETA(DisplayName = "Grass"),
	Snow UMETA(DisplayName = "Snow")
};


/** Static resource information shared by all inventories. */
USTRUCT(BlueprintType)
struct MINECRAFT_API FResourceDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	EResourceType ResourceType = EResourceType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	TSoftObjectPtr<UTexture2D> Icon;
};

