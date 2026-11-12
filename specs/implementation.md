# Sticker Label Gun: behavior and implementation

Describes the retained payload **0.2.1**. The behavior prose describes the retained payload; current candidate source and metadata are identified below. Gameplay qualification is a separate requirement.


<!-- source-only-sync -->
## Current source candidate

Source version **0.2.2** is saved from workshop revision `ca32bbf79ce0840d5166a232bdb3a9fdcd761254`. The retained installable payload remains **0.2.1** with its original bytes. This source update does not rebuild or requalify that payload. The current source and metadata define the candidate; gameplay, rendered behavior and co-op remain unqualified for rebuilt bytes.

The authored behavior and lifecycle descriptions below are retained from the published payload. They are not new qualification evidence for the source candidate. The integration table below follows the current candidate metadata.
<!-- /source-only-sync -->

## Behavior

Buy the $60 Sticker Label Gun, assign its Equipment slot and select it. Q cycles four designs, R places within eight meters, and Backspace removes the newest label. R avoids the vanilla E interaction. Collider-free sticker quads parent to the aimed surface, bounded at 128. The textured label dispenser appears only while equipped; native tool effects are suppressed through Equipment Framework.

## State and lifecycle

Ownership persists in the provider sidecar and quickslot IDs save natively. Placed labels, selected design and surface roots are local/session-only. Holster destroys the held instance while reusable mesh/texture resources remain; world teardown releases instances before shared resources. Partial model creation rolls back and retries with a bounded delay.

## Integration and configuration

Runtime dependencies: [shop-framework](https://github.com/swooping-mods/shop-framework), [equipment-framework](https://github.com/swooping-mods/equipment-framework), [hints-framework](https://github.com/swooping-mods/hints-framework).

The [module metadata](../source/mods/sticker-label-gun/mod.json) owns the version and declared integration contract. Native modules start at `MoonLab_ModStart`. Game/Unity integrations target the pinned Steam Windows x64 IL2CPP build.

| Service | ABI | Table size |
|---|---:|---|
| Requires `shop-framework.shop` | 1 | at least 56 bytes |
| Requires `equipment-framework.equipment` | 1 | at least 32 bytes |
| Requires `hints-framework.hints` | 1 | at least 24 bytes |

## Implementation sources

- [mod.c](../source/mods/sticker-label-gun/mod.c)
- [tool_runtime.h](../source/mods/sticker-label-gun/tool_runtime.h)

## Verification boundary

No saved/shared stickers, arbitrary image loading or guest replication. Surface attachment, undo, native shader fit and rendered placement require gameplay qualification; purchase/equip checks alone do not cover them.

`python3 build.py --check` verifies the exported source hashes. It does not execute the mod or qualify the behaviors above. Installation and independent build prerequisites are in the [README](../README.md).
