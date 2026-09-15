# Bluetooth-Controlled Soccer Robot

First-year team project for the VGU Soccer Bot competition (2023). An Arduino robot driven over
Bluetooth, with an H-bridge stage and TT gear motors. Chassis designed and assembled by the team.

**The firmware here is the original competition code, unmodified.** Known defects are documented
below rather than silently fixed — see *Review, two years on*.

## Demo

- [Match footage](PASTE_YOUTUBE_UNLISTED_LINK_HERE)
- [Drive test](PASTE_YOUTUBE_UNLISTED_LINK_HERE)

## Hardware

- Arduino Uno
- Bluetooth module on hardware `Serial` (pins 0/1)
- H-bridge motor driver
- TT gear motors
- Self-designed chassis

## Control protocol

Single ASCII characters over Bluetooth at 9600 baud.

| Command | Intended | Right wheel | Left wheel |
|---------|----------|-------------|------------|
| `F` | forward         | forward | forward |
| `B` | backward        | back    | back    |
| `R` | turn right      | stopped | forward |
| `L` | turn left       | forward | stopped |
| `I` | "forward right" | forward | **back** |
| `G` | "forward left"  | **back** | forward |
| `S` | stop            | stopped | stopped |

## My contribution

Firmware, wiring and soldering. The chassis was the hard part — nobody on the team had mechanical
design experience, so the frame went through several iterations before it held up in play.

## Review, two years on

Re-reading this code as a fourth-year student, four things are wrong with it:

**1. `motorLeftA` is never configured as an output.** `setup()` calls
`pinMode(motorRightB, OUTPUT)` twice; the second call should have been `motorLeftA`. Pin 11
therefore stays in INPUT mode, and `digitalWrite(11, HIGH)` only enables the internal pull-up
(~20-50 kOhm), which sources microamps rather than driving the H-bridge input. This affects
`F`, `R` and `G` — including forward motion itself. The failure is intermittent rather than
total, which makes it the worst kind: fine on the bench, unreliable under load or on a weak
battery. One-line fix.

**2. `I` and `G` are mislabelled.** The comments say "forward right" and "forward left", but both
drive the wheels in opposite directions, which is a spin in place. The manoeuvre is useful for a
soccer bot; the comment is what is wrong.

**3. No speed control.** Everything is `digitalWrite`, so the motors are full-on or off. There is
no enable-pin or PWM handling. Pin 8 is not PWM-capable on an Uno, so adding speed control would
also mean moving that pin.

**4. No failsafe.** If the Bluetooth link drops mid-command, the last command latches and the
robot keeps driving. A watchdog that stops the motors after ~300 ms without a new command is the
standard fix.

## What I would change now

Fix the `pinMode` line; move the H-bridge enable pins onto PWM outputs for proportional speed;
add a command timeout; move Bluetooth to `SoftwareSerial` so the module does not have to be
unplugged for every upload; and correct the comments to match the actual behaviour.# soccer-bot
