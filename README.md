# Minecraft

A Minecraft-style voxel sandbox built with Unreal Engine 5.4 using C++ and Blueprints. Mine blocks to collect resources and select building blocks to craft and place. Resource costs are defined, but enforcing them is currently a known bug.

## Architecture

- **World management:** `UVoxelWorldSubsystem` manages chunks, schedules world generation in batches, and provides block placement, destruction, and coordinate lookups. `UVoxelWorldSettings` configures terrain and generation parameters.
- **Chunks:** `AVoxelChunk` defines the C++ interface and block storage. `BP_VoxelChunk` implements chunk generation and block visuals through Blueprint events.
- **Player:** `BP_Player` and `BP_PlayerController` handle player interaction through Enhanced Input. The player tracks the selected building block type.
- **Inventory:** `BP_InventoryComponent` stores resource counts separately from the selected building block. Inventory changes notify the UI through a dispatcher.
- **Definitions:** DataTables describe resources (names and icons) and blocks (names, icons, resource costs, and mining drops). `UVoxelDefinitionSettings` holds the table references, and `UVoxelDefinitionFunctionLibrary` exposes shared lookups to Blueprints.
- **HUD:** UMG widgets display resource counts and building choices, highlighting the selected block. Inventory and selection events keep the HUD updated.

## Controls

| Input | Action |
|---|---|
| WASD | Move |
| Mouse | Look around |
| Hold Left Mouse Button | Mine blocks and collect resources |
| Press Right Mouse Button | Craft and place the selected block |
| Mouse Wheel | Select the building block type |
| Space | Jump |

## Planned Improvements

- `UVoxelDefinitionFunctionLibrary` currently calls `LoadSynchronous()` for the relevant DataTable on every lookup. Move table ownership to a persistent runtime object, load the tables asynchronously once at game startup, and retain their references for subsequent lookups. Already loaded tables are reused by Unreal; the current calls do not necessarily reload them from disk each time.
