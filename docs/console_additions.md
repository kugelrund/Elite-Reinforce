
# Console Additions

## Gameplay Modifications

Variables:

- `g_fixFreezeBorg1` (0 or 1)
  Allows to avoid the freze that happens in `borg1` when dropping down from the vents.
  Default: `1`.

- `g_skipDialogs` (0, 1 or 2)

  Allows to skip dialogs by never waiting for sounds to be done playing.
  Value `0` means skipping is disabled, `1` means all sounds are skipped, `2` means all sounds with a filepath that is listed in textfile `speedrun/sounds_to_remove.txt` are skipped.
  Default: `0`.

- `g_snapVelocity` (0 or 1)

  Allows to enable or disable [velocity snapping](velocity_snapping.md).
  **Cheat protected**.
  Default: `1`.

- `g_teamKnockback` (0 or 1)

  Allow players to damage boost themselves with explosive weapons.
  **Cheat protected**.
  Default: `0`.

Commands:

- `saveholodeck <filename>`

  Allows to do quicksaves on holodeck maps.
  Otherwise works just like the `save` command.

## Strafe Helper

Variables:

- `cg_drawStrafeHelper` (0 or 1)

  Draw a strafe helper.
  Default: `0`.

- `cg_strafeHelperCenter` (0 or 1)

  Put the strafe helper in the center of the screen.
  Default: `1`.

- `cg_strafeHelperCenterMarker` (0 or 1)

  Draw a marker in the center of the strafe helper.
  Default: `1`.

- `cg_strafeHelperHeight` (#)

  Height of strafe helper bar.
  Default: `20.0`.

- `cg_strafeHelperScale` (#)

  Horizontal scale of the strafe helper.
  Angles correspond directly to the field of view if this scale is set to `360/cg_fov`.
  Default: `4.0`.

- `cg_strafeHelperSpeedScale` (0, 1, 2 or 3)

  Scale of the text for speed display tied to strafe helper.
  Note that this shows the speed after friction but prior to acceleration, as this is what is relevant for optimal strafing.
  It is not the intuitive current player speed.
  Default: `2`.

- `cg_strafeHelperSpeedY` (#)

  Vertical offset of the text for speed display, relative to the strafe helper.
  Default: `5.0` (directly under the strafe helper).

- `cg_strafeHelperY` (#)

  Vertical position of the strafe helper.
  Default: `50.0`.

## Scripted Behavior Information

Variables:

- `cg_highlightDeathScripts` (scriptname)

  Highlights all entities whose death script have a name that contains the given scriptname.
  Many sections where you have to kill a set amount of enemies use a counting script which can be highlighted by setting this cvar to `count`.
  Default: `NONE`.

- `g_showPaths` (teamname)

  Visualizes the currently planned path for all NPCs whose team matches the value of this cvar.
  Most useful is `starfleet` for showing teammate pathing.
  To show pathing for all NPCs, use `all`.
  Default: empty (no paths are shown).
