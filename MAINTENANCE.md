# MAINTENANCE.md

## Coding Standards

- C11, `-Wall -Wextra`, zero warnings
- No `malloc`/`calloc`/`free` — stack-only
- Only external dep: `-lm` for `cosf()`
- All public API in `energy.h`

## Running Tests

```sh
make clean && make test
```

## Adding Operations

1. Add field to `EnergyCosts` in `energy.h`
2. Set default in `energy_costs_default()` in `energy.c`
3. Add lookup in `atp_can_afford()` in `energy.c`
4. Add test in `test_energy.c`
