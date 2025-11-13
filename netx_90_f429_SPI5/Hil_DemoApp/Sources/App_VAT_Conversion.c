#include "App_VAT_Conversion.h"
#include "App_VAT_Parameters.h"
#include <math.h>

/******************************************************************************
 * PRESSURE CONVERSION CONSTANTS
 ******************************************************************************/

/* Pressure conversion factors (all to/from Torr) */
#define PRESSURE_COUNTS_TO_TORR(counts, max_torr)  ((float)(counts) * (max_torr) / 32767.0f)
#define PRESSURE_TORR_TO_COUNTS(torr, max_torr)    ((int16_t)((torr) * 32767.0f / (max_torr)))

/* Standard conversion factors to Torr */
#define PSI_TO_TORR       51.71493f
#define BAR_TO_TORR       750.0616f
#define MBAR_TO_TORR      0.750062f
#define PA_TO_TORR        0.007501f
#define ATM_TO_TORR       760.0f
#define MTORR_TO_TORR     0.001f

/******************************************************************************
 * POSITION CONVERSION CONSTANTS
 ******************************************************************************/

#define POSITION_COUNTS_MAX     32767.0f
#define POSITION_PERCENT_MAX    100.0f
#define POSITION_DEGREES_MAX    90.0f   /* Valve rotation range */

/******************************************************************************
 * INT ↔ REAL CONVERSION FUNCTIONS
 ******************************************************************************/

/* Convert INT16 pressure to REAL (float) */
float VAT_ConvertPressureIntToReal(int16_t int_pressure, uint16_t pressure_units, float max_pressure_torr)
{
    float pressure_torr = PRESSURE_COUNTS_TO_TORR(int_pressure, max_pressure_torr);

    /* Convert from Torr to target units */
    switch (pressure_units) {
        case PRESSURE_UNIT_COUNTS:
            return (float)int_pressure;
        case PRESSURE_UNIT_PERCENT:
            return (pressure_torr / max_pressure_torr) * 100.0f;
        case PRESSURE_UNIT_TORR:
            return pressure_torr;
        case PRESSURE_UNIT_MTORR:
            return pressure_torr / MTORR_TO_TORR;
        case PRESSURE_UNIT_PSI:
            return pressure_torr / PSI_TO_TORR;
        case PRESSURE_UNIT_BAR:
            return pressure_torr / BAR_TO_TORR;
        case PRESSURE_UNIT_MBAR:
            return pressure_torr / MBAR_TO_TORR;
        case PRESSURE_UNIT_PA:
            return pressure_torr / PA_TO_TORR;
        case PRESSURE_UNIT_ATM:
            return pressure_torr / ATM_TO_TORR;
        default:
            return pressure_torr;
    }
}

/* Convert REAL (float) pressure to INT16 */
int16_t VAT_ConvertPressureRealToInt(float real_pressure, uint16_t pressure_units, float max_pressure_torr)
{
    float pressure_torr = 0.0f;

    /* Convert to Torr first */
    switch (pressure_units) {
        case PRESSURE_UNIT_COUNTS:
            return (int16_t)real_pressure;
        case PRESSURE_UNIT_PERCENT:
            pressure_torr = (real_pressure / 100.0f) * max_pressure_torr;
            break;
        case PRESSURE_UNIT_TORR:
            pressure_torr = real_pressure;
            break;
        case PRESSURE_UNIT_MTORR:
            pressure_torr = real_pressure * MTORR_TO_TORR;
            break;
        case PRESSURE_UNIT_PSI:
            pressure_torr = real_pressure * PSI_TO_TORR;
            break;
        case PRESSURE_UNIT_BAR:
            pressure_torr = real_pressure * BAR_TO_TORR;
            break;
        case PRESSURE_UNIT_MBAR:
            pressure_torr = real_pressure * MBAR_TO_TORR;
            break;
        case PRESSURE_UNIT_PA:
            pressure_torr = real_pressure * PA_TO_TORR;
            break;
        case PRESSURE_UNIT_ATM:
            pressure_torr = real_pressure * ATM_TO_TORR;
            break;
        default:
            pressure_torr = real_pressure;
    }

    return PRESSURE_TORR_TO_COUNTS(pressure_torr, max_pressure_torr);
}

/* Convert INT16 position to REAL (float) */
float VAT_ConvertPositionIntToReal(int16_t int_position, uint16_t position_units)
{
    float position_percent = ((float)int_position / POSITION_COUNTS_MAX) * 100.0f;

    switch (position_units) {
        case POSITION_UNIT_COUNTS:
            return (float)int_position;
        case POSITION_UNIT_PERCENT:
            return position_percent;
        case POSITION_UNIT_DEGREES:
            return (position_percent / 100.0f) * POSITION_DEGREES_MAX;
        default:
            return position_percent;
    }
}

/* Convert REAL (float) position to INT16 */
int16_t VAT_ConvertPositionRealToInt(float real_position, uint16_t position_units)
{
    float position_percent = 0.0f;

    switch (position_units) {
        case POSITION_UNIT_COUNTS:
            return (int16_t)real_position;
        case POSITION_UNIT_PERCENT:
            position_percent = real_position;
            break;
        case POSITION_UNIT_DEGREES:
            position_percent = (real_position / POSITION_DEGREES_MAX) * 100.0f;
            break;
        default:
            position_percent = real_position;
    }

    return (int16_t)((position_percent / 100.0f) * POSITION_COUNTS_MAX);
}

/******************************************************************************
 * UNITS CONVERSION FUNCTIONS
 ******************************************************************************/

/* Convert pressure between units (both REAL) */
float VAT_ConvertPressure(float value, uint16_t from_units, uint16_t to_units)
{
    if (from_units == to_units) {
        return value;
    }

    /* Convert to Torr first */
    float value_torr = 0.0f;
    switch (from_units) {
        case PRESSURE_UNIT_COUNTS:
            /* Cannot convert from counts without max_pressure_torr */
            return value;
        case PRESSURE_UNIT_PERCENT:
            /* Cannot convert from percent without max_pressure_torr */
            return value;
        case PRESSURE_UNIT_TORR:
            value_torr = value;
            break;
        case PRESSURE_UNIT_MTORR:
            value_torr = value * MTORR_TO_TORR;
            break;
        case PRESSURE_UNIT_PSI:
            value_torr = value * PSI_TO_TORR;
            break;
        case PRESSURE_UNIT_BAR:
            value_torr = value * BAR_TO_TORR;
            break;
        case PRESSURE_UNIT_MBAR:
            value_torr = value * MBAR_TO_TORR;
            break;
        case PRESSURE_UNIT_PA:
            value_torr = value * PA_TO_TORR;
            break;
        case PRESSURE_UNIT_ATM:
            value_torr = value * ATM_TO_TORR;
            break;
        default:
            return value;
    }

    /* Convert from Torr to target units */
    switch (to_units) {
        case PRESSURE_UNIT_TORR:
            return value_torr;
        case PRESSURE_UNIT_MTORR:
            return value_torr / MTORR_TO_TORR;
        case PRESSURE_UNIT_PSI:
            return value_torr / PSI_TO_TORR;
        case PRESSURE_UNIT_BAR:
            return value_torr / BAR_TO_TORR;
        case PRESSURE_UNIT_MBAR:
            return value_torr / MBAR_TO_TORR;
        case PRESSURE_UNIT_PA:
            return value_torr / PA_TO_TORR;
        case PRESSURE_UNIT_ATM:
            return value_torr / ATM_TO_TORR;
        default:
            return value_torr;
    }
}

/* Convert position between units (both REAL) */
float VAT_ConvertPosition(float value, uint16_t from_units, uint16_t to_units)
{
    if (from_units == to_units) {
        return value;
    }

    /* Convert to percent first */
    float value_percent = 0.0f;
    switch (from_units) {
        case POSITION_UNIT_COUNTS:
            value_percent = (value / POSITION_COUNTS_MAX) * 100.0f;
            break;
        case POSITION_UNIT_PERCENT:
            value_percent = value;
            break;
        case POSITION_UNIT_DEGREES:
            value_percent = (value / POSITION_DEGREES_MAX) * 100.0f;
            break;
        default:
            return value;
    }

    /* Convert from percent to target units */
    switch (to_units) {
        case POSITION_UNIT_COUNTS:
            return (value_percent / 100.0f) * POSITION_COUNTS_MAX;
        case POSITION_UNIT_PERCENT:
            return value_percent;
        case POSITION_UNIT_DEGREES:
            return (value_percent / 100.0f) * POSITION_DEGREES_MAX;
        default:
            return value_percent;
    }
}
