# Source implementation status

Included in the initial C++ graybox:

- Replicated match phase, crown holder, winner and timer progress.
- Server-side hitscan request, damage application and character elimination.
- Last-survivor winner selection.
- Crown claim/drop API and blood-moon continuous hold timer.
- Dropped-crown reset timer (state reset only; no physical pickup actor yet).
- Timeout placeholder: crown holder wins; otherwise highest remaining health, then lowest PlayerId.

Not yet included: altar proximity/channel validation, physical crown pickup, fire rate/ammo/reload, armor, death spectating, alliance, AI creatures, taming, awakening, contribution score, UI, input assets or a map. Blueprint-callable crown APIs must only be invoked through server-authoritative interactions with server-side validation in a production build.
