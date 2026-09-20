# AfterLife 0.2

AfterLife is an interactive Qt 6 / QML constructor and simulator for exploring how physical environmental states can become inputs to an evolving decision system.

The project contains two complete simulation modes:

- **Cosmological evolution** — clustered primary matter → particles → stars and non-intersecting planetary systems → galaxies / superstructure → abiogenesis → inherited sensor/action networks → colonies, reflective agents and interplanetary expeditions.
- **Crystal growth** — an evolutionary lattice-front model where orientation is inherited, mutated and selected by local geometry, anisotropy, temperature, supersaturation and diffusion.

The original dark blue visual language is retained, but the UI has been rebuilt around draggable/collapsible engineering panels, contextual help, several themes, particle typography, report playback and an interactive planet view.

## Build

Requirements:

- Qt **6.5+** (the uploaded project had previously been configured with Qt 6.11.2 MinGW)
- Qt modules: `Quick`, `QuickControls2`, `Network`
- CMake 3.16+
- C++17 compiler

Qt Creator: open `CMakeLists.txt`, select a Qt 6.5+ Desktop kit, configure and build.

Command-line example when Qt is on `CMAKE_PREFIX_PATH`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

This source package intentionally does not contain the stale generated `build/` directory from the uploaded archive.

## Controls

### Main simulation

- Hold **LMB** and drag: pan the simulation field.
- Mouse wheel: zoom.
- Click a planet: enter the interactive planet projection.
- **Space**: pause/resume.
- `+100`, `+1000`: fast-forward simulated time while retaining bounded integration steps.
- `След. эпоха`: advance until the next reachable evolutionary epoch or the safety budget is exhausted.
- `Орбиты`: toggle real parent-relative planetary and satellite orbital lanes.

### Planet view

- Drag the globe: rotate the 3D projection.
- Wheel: zoom the globe.
- Toggle colonies, satellites, ships and coordinate guides independently.
- Planet state is refreshed while the simulation continues.

### Crystal model

- Click anywhere: add a nucleation center.
- Tune temperature, supersaturation, anisotropy, mutation, branching and diffusion.
- Skip generations in large blocks.
- Export a sparse final lattice and its morphology history to JSON.

## Mathematical model

The interactive model uses **normalized units**. It is designed to preserve physically meaningful dependencies and stable causality rather than claim precision astrophysics.

Key relationships include:

- softened inverse-square Newtonian gravity;
- a smooth large-scale halo potential;
- Kepler-like circular angular velocities;
- disjoint orbital lanes, system-clearance checks and satellite spacing;
- luminosity scaling approximately as `M^3.5`;
- radiative equilibrium temperature approximately as `L^(1/4) / sqrt(r)`, including albedo and greenhouse retention;
- a configurable habitable-zone weighting;
- atmosphere retention, water, magnetic shielding, radiation and gravity terms in habitability;
- stochastic Poisson-style abiogenesis hazard;
- an energy budget for absorption, metabolism, motion, replication and spacecraft;
- inherited 3-sensor → 2-action networks with mutation, lifetime plasticity, memory and fitness-dependent selection;
- carrying capacity and crowding pressure;
- deterministic seeded initial conditions and reportable event history.

The strict non-collision baseline uses circular generated orbits. Eccentric multi-body orbital resonance is deliberately not faked: adding it correctly would require a substantially more expensive N-body / symplectic integration model.

## Local assistant

The UI always supports deterministic simulation commands such as:

- `пауза`
- `скорость x20`
- `пропусти 1000 секунд`
- `следующая эпоха`
- `экспорт отчёта`

For open-ended dialogue, AfterLife can connect to a **local OpenAI-compatible endpoint**. The default is:

`http://127.0.0.1:8080/v1/chat/completions`

This works well with a local `llama.cpp` server or another compatible local runtime. Change the endpoint directly in the assistant panel. No cloud service or model binary is bundled, so the application remains lightweight. If the endpoint is unavailable, the assistant explicitly switches to its built-in simulation-aware expert summary instead of pretending that a language model is running.

## Reproducibility and exports

Relative export names are saved in the user's `Documents/AfterLife` directory.

- `afterlife-scenario.json` — all constructor parameters and seed.
- `afterlife-metrics.csv` — metric timeline.
- `afterlife-metrics.json` — metric timeline in JSON.
- `afterlife-report.json` — scenario, event log, metric history, textual summary and final entities.
- `afterlife-crystal-report.json` — crystal parameters, morphology history and sparse final lattice.

A saved main-simulation report can be loaded back into the metrics panel for graphical timeline playback.

## Architecture

- `Scenario` — serializable parameter set.
- `Physics` — normalized physical helper equations.
- `Universe` — deterministic world state, evolution, hierarchy and reports.
- `SimulationWorker` — dedicated simulation thread / real-time stepping.
- `Simulator` — thread-safe QML-facing facade and report utilities.
- `UniverseRenderer` — 2D cosmic renderer, camera, picking and planet data projection.
- `CrystalField` — independent crystal evolutionary engine and renderer.
- `ParticleField` — interactive particles, including the AFTERLIFE text target.
- `LocalAssistant` — local LLM bridge plus safe deterministic simulation command parser.
- QML screens/panels — themed, movable/collapsible GUI.

See `ARCHITECTURE.md` for model flow and extension points.
