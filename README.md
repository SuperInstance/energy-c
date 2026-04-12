# energy-c

Pure C11 ATP energy system for bare-metal agents. Zero dynamic allocation. Only external dependency: `-lm` for `cosf()`.

## Quick Start

```sh
make test
```

## API

See `energy.h` for full API. Key types:

- **AtpPool** — agent's metabolic energy store
- **EnergyCosts** — per-operation costs (perception, deliberation, evolution, etc.)
- **CircadianRhythm** — modulates generation rate with cosine interpolation
- **ApoptosisLevel** — health checks (NONE → LOW → STARVING → CRITICAL → terminate)

## Design

Energy models biological ATP: agents consume energy on every operation, generate it over time, and face circadian modulation. When energy drops critically low, the agent triggers apoptosis (graceful shutdown).
