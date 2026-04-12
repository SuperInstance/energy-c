#ifndef ENERGY_H
#define ENERGY_H

#include <stdint.h>

typedef struct {
    float atp;
    float max_atp;
    float generation_rate;
    float base_rate;
    float peak_rate;
} AtpPool;

typedef struct {
    float perception;
    float arithmetic;
    float deliberation;
    float communication;
    float memory_read;
    float memory_write;
    float instinct;
    float evolution;
} EnergyCosts;

typedef struct {
    uint8_t peak_start_hour;
    uint8_t peak_end_hour;
    float peak_multiplier;
    float trough_multiplier;
} CircadianRhythm;

typedef enum {
    ENERGY_OK = 0,
    ENERGY_INSUFFICIENT,
    ENERGY_DEPLETED,
    ENERGY_OVERFLOW
} EnergyResult;

void atp_init(AtpPool *p, float max_atp, float base_rate, float peak_rate);
void energy_costs_default(EnergyCosts *c);
void circadian_default(CircadianRhythm *r);

EnergyResult atp_consume(AtpPool *p, float cost);
EnergyResult atp_generate(AtpPool *p, CircadianRhythm *r, uint8_t current_hour);
float atp_remaining_fraction(const AtpPool *p);
int atp_can_afford(const AtpPool *p, const EnergyCosts *c, const char *operation);

float circadian_multiplier(const CircadianRhythm *r, uint8_t hour);
int circadian_is_peak(const CircadianRhythm *r, uint8_t hour);
int circadian_is_dreaming(const CircadianRhythm *r, uint8_t hour);

float energy_estimate_task(const EnergyCosts *c, int perceptions, int arith, int deliberations, int comms);

typedef enum {
    APOPTOSIS_NONE = 0,
    APOPTOSIS_LOW_ENERGY,
    APOPTOSIS_STARVING,
    APOPTOSIS_CRITICAL
} ApoptosisLevel;

ApoptosisLevel atp_apoptosis_check(const AtpPool *p, float starvation_threshold);
int atp_should_terminate(const AtpPool *p);

#endif
