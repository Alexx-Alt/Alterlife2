# AfterLife architecture and extension notes

## Main data flow

```text
QML controls
   ↓ scenario JSON / commands
Simulator (GUI thread)
   ↓ queued calls
SimulationWorker (worker thread)
   ↓
Universe ── Physics
   │        ├─ normalized gravity / orbits
   │        ├─ thermal / magnetic / tidal fields
   │        └─ habitability helpers
   ├─ Metrics history
   ├─ event history
   └─ RenderSnapshot (mutex protected)
             ↓
      UniverseRenderer (GUI thread)
```

The simulation state never depends on the frame renderer. This keeps reports reproducible and makes it possible to replace the current renderer with Qt Quick 3D or a GPU renderer later without changing the model.

## Cosmic hierarchy

The runtime entity hierarchy is intentionally explicit:

`Quant → Particle`

`Star → Planet → Moon`

`Planet → Cell → Colony → Mind`

`Colony/Mind → Ship → target Planet → Colony`

`Star groups → Galaxy → Supercluster`

Generated planetary and moon orbits are circular, parent-relative and radially separated. A star-system clearance test prevents a newly formed stellar body from appearing inside an existing orbital envelope. This is the conservative collision-safe mode required by the current GUI simulator.

## Decision-system mapping

Each biological agent receives three normalized environmental sensors:

1. thermal suitability;
2. magnetic shielding;
3. tidal cycle.

They feed a small inherited 3×2 action network. The output is interpreted as a movement/action direction. Selection is indirect through the energy budget and local habitability; successful agents replicate more often. Later stages add online plasticity, action memory, reduced response latency and technology accumulation.

This is intentionally a transparent evolutionary control model: every state can be inspected and exported. A future neural-policy implementation can be inserted behind the same sensor/action interface.

## Crystal evolution

`CrystalField` stores a sparse active frontier over a fixed lattice. Every new cell:

- competes with other frontier positions for growth;
- inherits neighboring orientation;
- mutates that orientation;
- is weighted by local geometry, sixfold anisotropy, temperature, supersaturation, branching and diffusion.

The frontier is maintained incrementally rather than rescanning the full grid on each growth event. Candidate indices are sorted before seeded selection to avoid hash-iteration order breaking reproducibility.

## Performance decisions

- Simulation runs off the GUI thread.
- Entity IDs are indexed with `QHash` rather than repeatedly scanning the full array.
- High playback speeds advance **simulated time with bounded substeps** instead of multiplying forces.
- Star-formation searches are candidate-limited.
- Crystal growth maintains an incremental frontier.
- Render data is copied into a compact snapshot under one mutex.
- Planet detail is a software 3D projection, avoiding a mandatory Qt Quick 3D dependency.

## Recommended future high-fidelity modules

If the prototype grows into a research simulator, keep the UI/API and replace individual layers rather than making the current normalized core enormous:

- Barnes–Hut / fast multipole gravity for large N;
- symplectic N-body integration for eccentric / resonant systems;
- SPH or grid hydrodynamics for gas;
- stellar evolution tracks and nucleosynthesis tables;
- climate energy-balance / atmospheric chemistry models;
- genome or learned-policy agents;
- Qt Quick 3D / Vulkan surface rendering;
- local quantized LLM model management and tool schema instead of only an external local endpoint.
