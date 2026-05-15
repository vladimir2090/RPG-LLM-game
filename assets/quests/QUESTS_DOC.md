# Quest JSON examples

The game loads quests from `assets/quests/quests.json`.

JSON does not support comments, so keep notes here and keep `quests.json` clean.

## Open chests

```json
{
  "id": "open_chests_3",
  "title": "Old sign",
  "complete_title": "Quest complete",
  "objective": "Open chests",
  "hint": "Search old chests for weapons.",
  "complete_hint": "The wilds are a little less empty now.",
  "type": "open_chests",
  "required": 3
}
```

## Kill slimes

```json
{
  "id": "kill_slimes_3",
  "title": "Slime trouble",
  "complete_title": "Slimes cleared",
  "objective": "Kill slimes",
  "hint": "Thin out the nearby slime pack.",
  "complete_hint": "The grass is quieter now.",
  "type": "kill_slimes",
  "required": 3
}
```

## Manual quest sign

```json
{
  "quest_id": "kill_slimes_3",
  "x": 420,
  "y": -120
}
```

Supported `type` values now:

- `open_chests`
- `kill_slimes`

Up to 3 quests can be active at once. Completed quests show as complete for a short time, then shrink out of the HUD.
