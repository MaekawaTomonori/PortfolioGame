# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

---

# Purpose

This project is a **game built on top of the Engine framework**.

Dependency direction:

Game → Engine

The Game layer is responsible for:

- Gameplay logic
- Game state management
- Game-specific systems
- Content integration

The Engine provides reusable infrastructure such as:

- Rendering
- Resource management
- Input
- Audio
- Core utilities

The Game should focus on **gameplay and content**, not engine-level functionality.

---

# Dependency Rules (Critical)

- Game may depend on Engine.
- Game must **not modify Engine behavior directly**.
- Game must **not rely on Engine internal implementation**.
- Game must **only use public Engine APIs**.

Never:

- Access internal Engine headers
- Depend on Engine private structures
- Modify Engine source to solve gameplay problems

If Engine functionality is insufficient, discuss whether the change belongs in the Engine.

---

# Architectural Boundaries

The Game layer should remain clearly separated from Engine systems.

Game systems should include:

- Gameplay logic
- Game objects
- Player / enemy behaviors
- Stage logic
- UI logic
- Game-specific resource usage

The Game should **never reimplement Engine functionality** such as:

- Rendering systems
- Resource loaders
- Low-level graphics management

Engine systems must be used through their public interfaces.

---

# Game Architecture Guidelines

Game code should remain organized around **gameplay responsibilities**, not engine internals.

Typical layers may include:

Game Layer

- Game
- Stage / Scene
- GameObject
- Gameplay systems
- UI logic

Engine Layer

- Rendering
- Resource management
- Audio
- Input
- Utilities

GameObject classes should represent **gameplay entities**, not low-level rendering objects.

Avoid mixing gameplay logic with rendering or engine internals.

---

# Core Design Principles

The following principles guide all design decisions.

---

## Simplicity (KISS)

Prefer simple, clear gameplay implementations.

Avoid unnecessary abstraction or complex systems.

Gameplay code should be easy to understand and modify.

---

## Avoid Unnecessary Features (YAGNI)

Do not implement systems before they are needed.

Gameplay systems should evolve based on actual requirements.

Avoid speculative gameplay frameworks.

---

## Don't Repeat Yourself (DRY)

Avoid duplicated gameplay logic.

Reusable behaviors should be centralized where appropriate.

However, readability and clarity are more important than aggressive abstraction.

---

## Single Responsibility (SRP)

Each class should have a clear purpose.

Examples:

- PlayerController
- EnemySpawner
- SkillSystem
- StageManager

Avoid classes that manage multiple unrelated systems.

---

## Separation of Concerns (SoC)

Different gameplay responsibilities should be separated.

Examples:

- Input handling
- Game state management
- Skill systems
- AI behaviors
- UI logic

Avoid tightly coupling unrelated gameplay systems.

---

## Defensive Programming

The game should remain stable even when unexpected situations occur.

Examples:

- Missing assets should fall back to defaults
- Invalid handles should not crash the game
- Gameplay systems should validate assumptions when possible

The goal is **robust gameplay behavior**, even during development.

---

## Meaningful Naming

Names are extremely important.

Classes, functions, and variables should clearly describe their purpose.

Avoid unclear abbreviations or ambiguous names.

---

## Boy Scout Rule

Whenever modifying code:

Leave it cleaner than you found it.

Examples:

- Improve naming
- Remove dead code
- Simplify logic
- Improve readability

Only perform safe improvements.

---

## Avoid Premature Optimization

Do not introduce performance complexity unless profiling shows a real bottleneck.

Gameplay code should prioritize:

- clarity
- maintainability
- correctness

Optimization should be data-driven.

---

# Error Handling Policy

- Exceptions are not used.
- Errors should be handled through return values or state checks.
- Gameplay systems should fail safely when possible.

Critical failures should provide clear diagnostics.

---

# AI Behavior Guidelines

When modifying this repository:

- Do not modify Engine code.
- Do not redesign core architecture unless explicitly instructed.
- Preserve existing gameplay patterns.
- Prefer small, incremental changes.
- Avoid introducing speculative gameplay systems.

Follow the Boy Scout Rule when safe.

---

# Priority Order

When making decisions, follow this priority:

1. Maintain dependency direction (Game → Engine)
2. Respect Engine API boundaries
3. Maintain clear gameplay architecture
4. Ensure correctness and stability
5. Keep implementations simple
6. Follow coding style rules
7. Optimize performance (only when required)

---

# Coding Rules

Follow the rules defined in:

- docs/Coderule.md
- docs/design-philosophy.md

---

# Build / Execution

Do not attempt to build or run the project.

The user is responsible for compilation and validation.