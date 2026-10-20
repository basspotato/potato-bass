# CLAUDE.md — Potato-Bass

Simple Windows 11 VST3 EDM bass synth. GitHub repo: `potato-bass`.

## Working Style (token efficiency)

- Do the task. Do not narrate each step or comment on what you're about to do.
- No recaps of completed work. Finish with at most 1-2 sentences on the outcome.
- Simple change = minimal action. Make the edit directly; do not re-read files you just edited, re-run builds, or add extra verification unless asked.
- Do not refactor, reformat, rename, or "improve" code outside the scope of the request.
- Do not add features, options, abstractions, or dependencies that weren't asked for.
- Do not add comments to code unless the logic is non-obvious or I ask for them.
- Do not create extra files (docs, READMEs, tests, scripts) unless asked.
- Do not explore or search the repo broadly when the target file is known.
- Ask a question only when blocked on a decision that changes the outcome; otherwise choose the obvious default.
- Keep answers short. No bullet-point summaries of what you changed.

## Git Rules

- NEVER push to the remote (`potato-bass`) without asking me first and getting a clear yes. This applies to every push, every time; one approval does not carry over.
- Local commits are fine when I ask for them. Do not force-push, rewrite history, or delete branches without asking.

## Project Basics

- Target: Windows 11, VST3 instrument (EDM bass synth).
- Framework: JUCE with CMake (confirm with me before changing toolchain).
- Keep the design simple: oscillator(s), filter, amp envelope, basic distortion/sub as requested. Add only what I instruct.
- GUI reference: match Potato-Kicker exactly (`Source/Look.h` palette/knob drawing, `Source/Hud.h` widgets, `theme` in `Source/Displays.h`). Black glossy knobs with orange-to-yellow light rings, all-caps labels, HUD tiles; the sequencer uses a deep blue/purple/magenta palette (`look::rampMode() = 1`).
- Real-time audio code: no allocations, locks, or file/IO on the audio thread.
- Keep the build clean and the plugin loadable in a DAW after each change.
