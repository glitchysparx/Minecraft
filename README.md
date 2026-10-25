# Minecraft

A Minecraft-style voxel sandbox built with Unreal Engine 5.4 using C++ and Blueprints. Mine blocks to collect resources and select building blocks to craft and place. Resource costs are defined, but enforcing them is currently a known bug.

## Controls

| Input | Action |
|---|---|
| WASD | Move |
| Mouse | Look around |
| Hold Left Mouse Button | Mine blocks and collect resources |
| Press Right Mouse Button | Craft and place the selected block |
| Mouse Wheel | Select the building block type |
| Space | Jump |

## Architecture

- **World management:** `UVoxelWorldSubsystem` owns chunk state for the active world. Generation proceeds outward from the configured center, processing one active chunk in configurable batches across frames. Scheduling is centralized in the subsystem, and generation ticking stops when the queue is complete. `UVoxelWorldSettings` exposes terrain parameters, chunk dimensions, generation batch size, and build limits through Developer Settings.
- **Chunks and C++ / Blueprint boundary:** C++ defines subsystem ownership, generation scheduling, coordinate conversion, and the `AVoxelChunk` interface and block storage. `BP_VoxelChunk` implements chunk generation and visuals through Blueprint events; chunks do not tick independently. Player interaction components and UI behavior are also implemented in Blueprints.
- **Player:** `BP_Player` and `BP_PlayerController` handle player interaction through Enhanced Input, with mining and building behavior organized into dedicated components.
- **Mining and building:** `BP_MiningComponent` handles mining blocks and collecting resources. `BP_BuildingComponent` handles building block selection and placement. Block edits pass through the world subsystem, which checks mining or building height limits and resolves global block coordinates to chunk-local positions. Chunk implementations apply block changes and refresh the affected visuals.
- **Inventory:** `BP_InventoryComponent` stores resource counts. Inventory changes notify the UI through a dispatcher.
- **Definitions:** `UVoxelDefinitionSubsystem` owns shared resource and block DataTables for the game instance, keeping loaded tables available across map changes. It loads them asynchronously, maintains building block order, and broadcasts `OnDefinitionsReady` when loading completes. DataTables define resource and block names and icons, block costs, and mining drops, allowing these values to change without editing gameplay code. `UVoxelDefinitionSettings` holds the table references, and `UVoxelDefinitionFunctionLibrary` exposes shared lookups to Blueprints.
- **HUD:** `WBP_HUD` presents resource counts through `WBP_InventoryPanel` and building choices through `WBP_CraftingPanel`, highlighting the selected block. Inventory and selection events keep the panels updated.
- **Loading screen:** `WB_LoadingScreen` covers startup while definitions load and the world generates, keeping gameplay gated until the player is positioned at a valid spawn location.

## Planned Improvements

- Add new block types, such as trees and water.
- Load new chunks as the player approaches world boundaries and unload distant chunks to support an infinite world while keeping performance optimized.
