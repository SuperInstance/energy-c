#include "energy.h"
#include <math.h>
#include <string.h>

#define M_PI_F 3.14159265358979323846f

void atp_init(AtpPool *p, float max_atp, float base_rate, float peak_rate) {
    p->max_atp = max_atp;
    p->base_rate = base_rate;
    p->peak_rate = peak_rate;
    p->generation_rate = base_rate;
    p->atp = max_atp;
}

void energy_costs_default(EnergyCosts *c) {
    c->perception = 0.5f;
    c->arithmetic = 0.1f;
    c->deliberation = 2.0f;
    c->communication = 1.0f;
    c->memory_read = 0.3f;
    c->memory_write = 0.5f;
    c->instinct = 0.2f;
    c->evolution = 5.0f;
}

void circadian_default(CircadianRhythm *r) {
    r->peak_start_hour = 9;
    r->peak_end_hour = 22;
    r->peak_multiplier = 1.5f;
    r->trough_multiplier = 0.4f;
}

EnergyResult atp_consume(AtpPool *p, float cost) {
    if (cost <= 0.0f) return ENERGY_OK;
    if (p->atp < cost) {
        p->atp = 0.0f;
        return (p->atp <= 0.0f) ? ENERGY_DEPLETED : ENERGY_INSUFFICIENT;
    }
    p->atp -= cost;
    return ENERGY_OK;
}

EnergyResult atp_generate(AtpPool *p, CircadianRhythm *r, uint8_t current_hour) {
    float mult = circadian_multiplier(r, current_hour);
    p->generation_rate = p->base_rate * mult;
    p->atp += p->generation_rate;
    if (p->atp > p->max_atp) {
        p->atp = p->max_atp;
        return ENERGY_OVERFLOW;
    }
    return ENERGY_OK;
}

float atp_remaining_fraction(const AtpPool *p) {
    if (p->max_atp <= 0.0f) return 0.0f;
    return p->atp / p->max_atp;
}

int atp_can_afford(const AtpPool *p, const EnergyCosts *c, const char *operation) {
    float cost = 0.0f;
    if (strcmp(operation, "perception") == 0) cost = c->perception;
    else if (strcmp(operation, "arithmetic") == 0) cost = c->arithmetic;
    else if (strcmp(operation, "deliberation") == 0) cost = c->deliberation;
    else if (strcmp(operation, "communication") == 0) cost = c->communication;
    else if (strcmp(operation, "memory_read") == 0) cost = c->memory_read;
    else if (strcmp(operation, "memory_write") == 0) cost = c->memory_write;
    else if (strcmp(operation, "instinct") == 0) cost = c->instinct;
    else if (strcmp(operation, "evolution") == 0) cost = c->evolution;
    else return 0;
    return p->atp >= cost;
}

float circadian_multiplier(const CircadianRhythm *r, uint8_t hour) {
    if (circadian_is_peak(r, hour)) return r->peak_multiplier;

    uint8_t peak_center = 0;
    if (r->peak_start_hour <= r->peak_end_hour) {
        peak_center = (uint8_t)((r->peak_start_hour + r->peak_end_hour) / 2);
    } else {
        uint8_t span = (uint8_t)(24 - r->peak_start_hour + r->peak_end_hour);
        peak_center = (uint8_t)((r->peak_start_hour + span / 2) % 24);
    }

    uint8_t diff;
    if (hour >= peak_center) {
        diff = (uint8_t)(hour - peak_center);
    } else {
        diff = (uint8_t)(hour + 24 - peak_center);
    }
    if (diff > 12) diff = (uint8_t)(24 - diff);

    float t = diff / 12.0f;
    float cosine = cosf(t * M_PI_F);
    return r->trough_multiplier + (r->peak_multiplier - r->trough_multiplier) * (cosine * 0.5f + 0.5f);
}

int circadian_is_peak(const CircadianRhythm *r, uint8_t hour) {
    if (r->peak_start_hour <= r->peak_end_hour) {
        return hour >= r->peak_start_hour && hour < r->peak_end_hour;
    }
    return hour >= r->peak_start_hour || hour < r->peak_end_hour;
}

int circadian_is_dreaming(const CircadianRhythm *r, uint8_t hour) {
    uint8_t trough_end = r->peak_start_hour;
    uint8_t trough_start = r->peak_end_hour;
    if (trough_start <= trough_end) {
        return hour >= trough_start && hour < trough_end;
    }
    return hour >= trough_start || hour < trough_end;
}

float energy_estimate_task(const EnergyCosts *c, int perceptions, int arith, int deliberations, int comms) {
    return (float)perceptions * c->perception
         + (float)arith * c->arithmetic
         + (float)deliberations * c->deliberation
         + (float)comms * c->communication;
}

ApoptosisLevel atp_apoptosis_check(const AtpPool *p, float starvation_threshold) {
    float frac = atp_remaining_fraction(p);
    if (frac < starvation_threshold * 0.1f) return APOPTOSIS_CRITICAL;
    if (frac < starvation_threshold * 0.4f) return APOPTOSIS_STARVING;
    if (frac < starvation_threshold) return APOPTOSIS_LOW_ENERGY;
    return APOPTOSIS_NONE;
}

int atp_should_terminate(const AtpPool *p) {
    float frac = atp_remaining_fraction(p);
    return frac < 0.05f;
}
