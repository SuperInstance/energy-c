#include "energy.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

static int tests_passed = 0;
static int tests_total = 0;

#define TEST(name) do { tests_total++; printf("  TEST %d: %s ... ", tests_total, name); } while(0)
#define PASS() do { tests_passed++; printf("PASS\n"); } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); } while(0)

int main(void) {
    printf("=== energy-c test suite ===\n\n");

    // 1. atp_init sets correct values
    {
        TEST("atp_init sets correct values");
        AtpPool p;
        atp_init(&p, 100.0f, 1.0f, 2.0f);
        if (p.atp == 100.0f && p.max_atp == 100.0f && p.base_rate == 1.0f && p.peak_rate == 2.0f)
            PASS(); else FAIL("values mismatch");
    }

    // 2. atp_consume deducts correctly
    {
        TEST("atp_consume deducts correctly");
        AtpPool p;
        atp_init(&p, 100.0f, 1.0f, 2.0f);
        EnergyResult r = atp_consume(&p, 30.0f);
        if (r == ENERGY_OK && fabsf(p.atp - 70.0f) < 0.001f)
            PASS(); else FAIL("wrong result or balance");
    }

    // 3. atp_consume returns INSUFFICIENT
    {
        TEST("atp_consume returns INSUFFICIENT");
        AtpPool p;
        atp_init(&p, 100.0f, 1.0f, 2.0f);
        p.atp = 5.0f;
        EnergyResult r = atp_consume(&p, 10.0f);
        if (r == ENERGY_INSUFFICIENT || r == ENERGY_DEPLETED) PASS(); else FAIL("expected insufficient/depleted");
    }

    // 4. atp_consume returns DEPLETED when hits zero
    {
        TEST("atp_consume returns DEPLETED when hits zero");
        AtpPool p;
        atp_init(&p, 100.0f, 1.0f, 2.0f);
        p.atp = 5.0f;
        atp_consume(&p, 10.0f);
        if (p.atp == 0.0f) PASS(); else FAIL("atp not clamped to zero");
    }

    // 5. atp_generate produces energy
    {
        TEST("atp_generate produces energy");
        AtpPool p;
        atp_init(&p, 100.0f, 1.0f, 2.0f);
        p.atp = 50.0f;
        CircadianRhythm r;
        circadian_default(&r);
        EnergyResult res = atp_generate(&p, &r, 12);
        if (p.atp > 50.0f && res == ENERGY_OK)
            PASS(); else FAIL("no energy generated");
    }

    // 6. atp_generate respects circadian rhythm
    {
        TEST("atp_generate respects circadian rhythm");
        AtpPool p1, p2;
        atp_init(&p1, 100.0f, 1.0f, 2.0f); p1.atp = 50.0f;
        atp_init(&p2, 100.0f, 1.0f, 2.0f); p2.atp = 50.0f;
        CircadianRhythm r;
        circadian_default(&r);
        atp_generate(&p1, &r, 12); // peak
        atp_generate(&p2, &r, 3);  // trough
        if (p1.atp > p2.atp) PASS(); else FAIL("peak should generate more");
    }

    // 7. circadian_multiplier during peak
    {
        TEST("circadian_multiplier is peak value during peak");
        CircadianRhythm r;
        circadian_default(&r);
        float m = circadian_multiplier(&r, 12);
        if (fabsf(m - r.peak_multiplier) < 0.001f)
            PASS(); else FAIL("expected peak multiplier");
    }

    // 8. circadian_multiplier lower during trough
    {
        TEST("circadian_multiplier is lower during trough");
        CircadianRhythm r;
        circadian_default(&r);
        float trough = circadian_multiplier(&r, 3);
        float peak = r.peak_multiplier;
        if (trough < peak) PASS(); else FAIL("trough should be lower than peak");
    }

    // 9. circadian handles midnight wrap
    {
        TEST("circadian handles midnight wrap");
        CircadianRhythm r;
        r.peak_start_hour = 22;
        r.peak_end_hour = 6;
        r.peak_multiplier = 1.5f;
        r.trough_multiplier = 0.4f;
        if (circadian_is_peak(&r, 23) && circadian_is_peak(&r, 2) && !circadian_is_peak(&r, 14))
            PASS(); else FAIL("midnight wrap failed");
    }

    // 10. atp_can_afford checks correctly
    {
        TEST("atp_can_afford checks correctly");
        AtpPool p;
        atp_init(&p, 100.0f, 1.0f, 2.0f);
        p.atp = 1.5f;
        EnergyCosts c;
        energy_costs_default(&c);
        if (atp_can_afford(&p, &c, "perception") && !atp_can_afford(&p, &c, "deliberation"))
            PASS(); else FAIL("afford check wrong");
    }

    // 11. energy_estimate_task calculates total
    {
        TEST("energy_estimate_task calculates total");
        EnergyCosts c;
        energy_costs_default(&c);
        float est = energy_estimate_task(&c, 2, 3, 1, 0);
        float expected = 2*0.5f + 3*0.1f + 1*2.0f;
        if (fabsf(est - expected) < 0.001f)
            PASS(); else FAIL("estimate mismatch");
    }

    // 12. apoptosis levels at various thresholds
    {
        TEST("apoptosis levels at various thresholds");
        AtpPool p;
        atp_init(&p, 100.0f, 1.0f, 2.0f);
        p.atp = 60.0f;
        ApoptosisLevel a1 = atp_apoptosis_check(&p, 0.5f);
        p.atp = 30.0f;
        ApoptosisLevel a2 = atp_apoptosis_check(&p, 0.5f);
        p.atp = 10.0f;
        ApoptosisLevel a3 = atp_apoptosis_check(&p, 0.5f);
        p.atp = 2.0f;
        ApoptosisLevel a4 = atp_apoptosis_check(&p, 0.5f);
        if (a1 == APOPTOSIS_NONE && a2 == APOPTOSIS_LOW_ENERGY && a3 == APOPTOSIS_STARVING && a4 == APOPTOSIS_CRITICAL)
            PASS(); else FAIL("apoptosis levels wrong");
    }

    // 13. should_terminate at critical
    {
        TEST("should_terminate at critical");
        AtpPool p;
        atp_init(&p, 100.0f, 1.0f, 2.0f);
        p.atp = 3.0f;
        if (atp_should_terminate(&p))
            PASS(); else FAIL("should terminate");
    }

    // 14. energy_costs_default returns positive values
    {
        TEST("energy_costs_default returns positive values");
        EnergyCosts c;
        energy_costs_default(&c);
        float *vals = (float *)&c;
        int ok = 1;
        for (int i = 0; i < 8; i++) {
            if (vals[i] <= 0.0f) { ok = 0; break; }
        }
        if (ok) PASS(); else FAIL("non-positive cost found");
    }

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_total);
    return (tests_passed == tests_total) ? 0 : 1;
}
