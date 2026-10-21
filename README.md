# trainscii

<div align="center">

![Status](https://img.shields.io/badge/status-beta%20testing-yellow.svg)
![C++](https://img.shields.io/badge/C%2B%2B-11%2B-orange.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20WSL-lightgrey.svg)
![Dependencies](https://img.shields.io/badge/dependencies-0-brightgreen.svg)
![Type](https://img.shields.io/badge/type-foundation-blueviolet.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

**A train that runs on a single line of your terminal.**

*A foundation for animated terminal motion — one engine, five bogies, eighteen tracks, and a train that moves with the frame.*


</div>

---

> **Status:** This is a **foundation**, not a framework or a library. It is in **beta testing**. The API is small on purpose and will change as the foundation settles. Use it to learn, extend it to build. It is not ready to be depended on.

---

# Quick Demos:

<h3 align="center">View</h3>
<image src="demoGifs/1.gif">

<h3 align="center">Trains</h3>
<image src="demoGifs/2.gif">

<h3 align="center">Train Customizations</h3>
<image src="demoGifs/trainCustomizations.png">


---

## Important Note

`trainscii` is designed for **POSIX terminals** (Linux, macOS, BSD, WSL) and requires **ANSI escape codes** for cursor positioning.

### Zero-Dependency by Design

The foundation contains **no external dependencies**. The only POSIX-specific calls are `ioctl` + `TIOCGWINSZ` (to check the terminal width) and cursor escape sequences for positioning.

There is **no color** — not yet. `trainscii` renders in the terminal's default foreground. That's a deliberate constraint of the beta: get the motion right first, then color it later.

**trainscii handles the train. Where the track lives is up to you.**

---

## 📖 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [The Train Model](#-the-train-model)
- [The Motion Foundation](#-the-motion-foundation)
- [Architecture](#-architecture)
- [Quick Start](#-quick-start)
- [Use Cases](#-use-cases)
- [Why trainscii](#-why-trainscii)
- [Comparison](#-comparison)
- [Design Philosophy](#-design-philosophy)
- [API Reference](#-api-reference)
- [Known Limitations](#-known-limitations)
- [License](#-license)

---

## 🌟 Overview

`trainscii` is a **foundation for animated terminal motion**. It draws a horizontal track at a row of your choosing, places a small ASCII train on it, and moves the train left or right — one cell per frame.

The name is a join of **"train"** and **"ASCII"** — but the idea is bigger than the pun. `trainscii` is the **foundation** underneath a much larger intent: what if terminal objects could *move* with the same ease that text gets *printed*? A train is the smallest interesting thing that moves. Draw it, animate it, learn from it.

### Why a Train?

Most terminal animation is about **changing** something — text fades, colors shift, blocks pulse. Very little of it is about **moving** something — a single object with identity, position, and trajectory.

`trainscii` is about movement. A train is a **body in motion**:

- It has a **position** (row, col)
- It has a **direction** (right or left)
- It has a **shape** (engine + bogies)
- It has **bounds** (the track it runs on)

That's the whole vocabulary. Once you can animate a train, you can animate anything that **moves across a line**.

- **One train.** One track. One row. That's the foundation.
- **Five bogies.** Interchangeable carriages that define the train's body.
- **Eighteen tracks.** From solid to dotted to double, the ground beneath the wheels.
- **Frame-based motion.** `nextFrame()` moves forward. `prevFrame()` moves back.

`trainscii` doesn't try to be a scene graph. It doesn't try to be a game engine. It doesn't try to be a UI framework. It's the **smallest possible moving thing** — and that's exactly what makes it a foundation.

---

## ✨ Features

### 🚃 The Train
- **Five Bogies** — `□` `▢` `▤` `▥` `▭` as interchangeable carriages
- **Two Engines** — `›` (right-facing) and `‹` (left-facing)
- **Eighteen Tracks** — From `─` and `━` to `╼` and `╾`
- **Adjustable Length** — Change the bogie count at runtime

### 🌀 Motion
- **Frame-Based Movement** — `nextFrame()` advances one cell, `prevFrame()` reverses
- **Track-Clamped Motion** — The train stops at the edge of the track
- **Erase-and-Redraw** — Only the train and the vacated cell are redrawn per frame
- **Custom Frame Rate** — You call `nextFrame()`; you decide the timing

### 🧩 Foundation-Level Design
- **One Class** — Construct, tick, done
- **Zero Setup** — Track and train appear on construction
- **Bounds Checked** — Terminal width is verified before the track is drawn
- **Runtime Restyling** — Change bogies, engine, or track between frames

---

## 🚃 The Train Model

`trainscii` is built on a single abstraction: **the train on its track**.

```
                        A TRAIN

     ┌────────────────────────────────────────────────────┐
     │                                                    │
     │   engine  →  ›   (right)  or  ‹   (left)           │
     │   bogies  →  □ □ □ □ □   (five by default)         │
     │   track   →  ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─         │
     │   bounds  →  (row, col) to (row, col + length)     │
     │                                                    │
     └────────────────────────────────────────────────────┘

     A right-facing train:

         ─ ─ ─ ─ ─ ─ ─ ─ □ □ □ □ □ › ─ ─ ─ ─ ─ ─ ─

     A left-facing train:

         ─ ─ ─ ─ ─ ─ ‹ □ □ □ □ □ ─ ─ ─ ─ ─ ─ ─ ─ ─
```

The train has **three parts**:

- **The engine** is the head. It points in the direction of motion.
- **The bogies** are the body. Their count and style define the train's identity.
- **The track** is the surface. Its style defines the world the train moves through.

On every `nextFrame()`, the train:

```
1. moves one cell to the right
2. redraws itself at its new position
3. redraws the track cell it just vacated
```

That's it. That's the whole lifecycle. The `prevFrame()` method does the same in reverse.

---

## 🌀 The Motion Foundation

The **foundation** underneath `trainscii` is simple: **one object, one line, one direction, one step per frame.**

That's what makes it extendable. If you understand `trainscii`, you understand:

- **A moving object** is a position + a shape + a direction
- **A track** is the boundary the object respects
- **A frame** is one advance in the object's state, followed by a redraw
- **Erasing** is the difference between clean motion and trails

The train is small enough to see all of this in one class. Every animation of a moving terminal object — a cursor, a cursor that becomes a bullet, a snake, a scroller — is built on the same four ideas.

`trainscii` is the smallest useful embodiment of those ideas.

---

## 🏗 Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                        │
│   ┌──────────────┐  ┌────────────────┐  ┌───────────────┐   │
│   │ Constructor  │  │   nextFrame()  │  │  prevFrame()  │   │
│   └──────────────┘  └────────────────┘  └───────────────┘   │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                    Train State                              │
│   ┌────────────────────────────────────────────────────┐    │
│   │   position  •  direction  •  bogies  •  engine     │    │
│   │   track  •  length  •  trainArray                  │    │
│   └────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                    Track Layer                              │
│   ┌────────────────────────────────────────────────────┐    │
│   │   length + col ≤ terminal width  →  draw track     │    │
│   └────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                    Render Layer                             │
│   ┌────────────────────────────────────────────────────┐    │
│   │   Cursor positioning  •  Train draw  •  Erase      │    │
│   └────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                            ↓
                    Terminal Display
```

`trainscii` writes **directly** to the terminal. It positions the cursor, draws the train, positions the cursor again, and redraws the vacated cell. There's no framebuffer, no diffing, no scene graph — just cursor moves and character writes.

The train's shape is **precomputed** as `trainArray` on construction and whenever its style changes. Only the position moves during animation. That's the whole rendering strategy.

---

## 🚀 Quick Start

### Prerequisites

- **C++11 or higher** compiler
- **POSIX terminal** — Linux, macOS, BSD, WSL
- **ANSI escape code support** — For cursor positioning

### Installation

`trainscii` is a **single C++ class**. Drop it into your project.

```bash
# Clone or copy trainscii.cpp into your project

# Compile with optimizations
g++ -O3 -std=c++11 your_demo.cpp -o your_demo

# Run
./your_demo
```

### Minimal Example

```cpp
#include "trainscii.cpp"
#include <thread>
#include <chrono>

int main() {
    //           row  col  length
    trainscii t( 10,  2,   60 );

    for (int i = 0; i < 55; ++i) {
        t.nextFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
```

That's it. A train, a track, a loop. The train moves one cell every 100 milliseconds.

### Customizing the Train

```cpp
trainscii t(10, 2, 60);

t.chageBoggyCount(8);     // longer train
t.changeBoggyType(2);     // use ▤ bogies
t.changeTrackType(1);     // use ═ track
t.changeEngineType(1);    // face left

for (int i = 0; i < 20; ++i) {
    t.prevFrame();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

The train restyles itself. The next frame shows the new shape.

---

## 🎯 Use Cases

### 1. Loading Indicators
A train moving across the bottom of your terminal while a build runs. Slow, readable, and it tells the user something is happening.

### 2. Terminal Art & Demos
A train that runs across the screen, changes bogies, switches tracks. Small, self-contained, and complete.

### 3. Learning Tool
The foundation is small enough to read. A train is a position, a shape, and a direction. If you can animate a train, you can animate anything.

### 4. Foundation for Larger Things
A snake game. A scrolling news ticker. A status bar that *moves*. `trainscii` is the primitive — the thing you build on top of.

### 5. Teaching Motion
The clearest possible introduction to "a terminal object that moves": position, direction, erase, redraw. Four ideas, one class.

---

## 🌈 Why trainscii

Most terminal animation is about **changing**: fade, color, shade, pulse. Very little of it is about **moving**: an object that has a position and a trajectory.

Movement is harder than change. It requires you to track state, to erase what was there, to draw what's new, to respect boundaries. `trainscii` does all of that for **one object on one line** — and by doing it small, it teaches the shape of the problem.

Once you've animated a train, the animation of anything else — a cursor, a bullet, a snake, a scroller — becomes obvious. You already know the four steps: track state, check bounds, erase old, draw new.

`trainscii` is the smallest possible answer to **"what is a moving terminal object?"** That's why it's a foundation.

---

## ⚖️ Comparison

| | **trainscii** | Plain `printf` | `simulaii` | `swarmi` |
|---|---|---|---|---|
| **Status** | Beta (foundation) | Stable | v1.0 | v1.0 |
| **Dependencies** | None | None | None | None |
| **Unit of Work** | A moving train | A string | An animated tile | A character agent |
| **Motion** | Left/right on a line | None | Pattern-driven | Path-driven |
| **Color** | No | Manual ANSI | Yes | Yes |
| **Frame Model** | `nextFrame()` / `prevFrame()` | N/A | `nextFrame()` | `tick()` |
| **Erase** | Yes, vacated cell | N/A | Full tile redraw | Trail-based |
| **Scope** | Moving object | Text | Animated tiles | Animated text |

`trainscii` doesn't try to be `simulaii` or `swarmi`. It's the **foundation** for moving objects — the smallest class that answers *"how does a thing move on a terminal line?"*

---

## 🧬 Design Philosophy

`trainscii` is a foundation, so it is deliberately **narrower than a library**.

- It animates **one object**.
- It moves on **one line**.
- It has **one direction at a time**.
- It exposes **one method per axis** for advancing the frame: `nextFrame()` and `prevFrame()`.

There is no color, no shadow, no trail, no particle system, no scene graph. Those would make it a library. They would also hide the four ideas that make it useful: **state, bounds, erase, redraw.**

Because it's a foundation, it's meant to be **extended, not depended on**. Copy it into your project. Change the bogies. Change the engine. Add color later. Add multiple trains. Add a second track. Add vertical movement. The beta label exists to say: **this is a starting point, not a stable surface.**

The source is short enough to read in one sitting, and the abstraction is small enough to reshape in an afternoon. That's the whole point.

---

## 📚 API Reference

### Constructor

```cpp
trainscii(int row, int col, int length);
```

Creates a train at row `row`, starting at column `col`, with a track of `length` cells.

On construction, the terminal width is checked. If `length + col` exceeds the terminal width, the constructor prints an error and calls `exit(1)`. Otherwise, the track is drawn and the train is placed at its starting position.

### `nextFrame()`

```cpp
int nextFrame();
```

Moves the train **one cell to the right** and redraws. Restores the track cell vacated by the train.

Returns `0` on success, `-1` if the train has reached the end of the track.

### `prevFrame()`

```cpp
int prevFrame();
```

Moves the train **one cell to the left** and redraws. Restores the track cell vacated by the train.

Returns `0` on success, `-1` if the train is at the start of the track.

### `chageBoggyCount()`

```cpp
int chageBoggyCount(int newCount);
```

> **Note:** The name contains a typo (`chage`) in the current beta. Expect this to be renamed to `changeBoggyCount()` in a future release.

Changes the number of bogies. Rebuilds the train's `trainArray` immediately.

Returns `0` on success, `-1` if `newCount < 1`.

### `changeBoggyType()`

```cpp
int changeBoggyType(int newType);
```

Changes the bogie character. Accepts `0`–`4`, mapping to `□` `▢` `▤` `▥` `▭`. Rebuilds the train's `trainArray` immediately.

Returns `0` on success, `-1` if the type is out of range.

### `changeEngineType()`

```cpp
int changeEngineType(int newType);
```

Changes the engine direction. Accepts `0` (right, `›`) or `1` (left, `‹`). Rebuilds the train's `trainArray` immediately.

Returns `0` on success, `-1` if the type is out of range.

### `changeTrackType()`

```cpp
int changeTrackType(int newType);
```

Changes the track character. Accepts `0`–`17`, mapping to one of eighteen track glyphs. Redraws the track and re-places the train.

Returns `0` on success, `-1` if the type is out of range.

---

## ⚠️ Known Limitations

Because `trainscii` is a **beta foundation**, several things are **intentionally absent** or **known rough edges**:

- **No color.** The terminal's default foreground is used everywhere.
- **No automatic tick.** You call `nextFrame()` yourself; the class doesn't own a loop.
- **`changeTrackType()` redraws the track but not always cleanly.** Redrawing a longer track glyph over a shorter one can leave residue. Restyling between frames is best done at the start of a fresh scene.
- **`changeBoggyCount()` doesn't move the train.** Increasing the bogie count can push the train's tail backward past `col`. Length increases should be followed by a manual `nextFrame()` to reposition.
- **The constructor calls `exit(1)`** on out-of-bounds track length. This is a beta decision — future versions may return an error instead.
- **The method `chageBoggyCount()` has a typo** and will be renamed.
- **Only one train per object.** Multiple trains need multiple instances, and they won't know about each other's positions on the same track.
- **No UTF-8 awareness.** All bogies, engines, and tracks are single-byte strings in the current build. Multi-byte glyphs may render incorrectly.

These are the edges the beta is designed to explore. Expect them to move.

---

## 📄 License

Licensed under the MIT License. See [LICENSE](LICENSE) for details.

---

<div align="center">

**A train, a track, a line of motion.**

*"The smallest moving thing on a terminal — and the foundation for everything that moves."*

[⬆ Back to Top](#trainscii)

</div>