#ifndef APP_VAT_CONVERSION_H_
#define APP_VAT_CONVERSION_H_

#include <stdint.h>

/******************************************************************************
 * INT ↔ REAL CONVERSION FUNCTIONS
 ******************************************************************************/

/* Convert INT16 pressure to REAL (float) */
float VAT_ConvertPressureIntToReal(int16_t int_pressure, uint16_t pressure_units, float max_pressure_torr);

/* Convert REAL (float) pressure to INT16 */
int16_t VAT_ConvertPressureRealToInt(float real_pressure, uint16_t pressure_units, float max_pressure_torr);

/* Convert INT16 position to REAL (float) */
float VAT_ConvertPositionIntToReal(int16_t int_position, uint16_t position_units);

/* Convert REAL (float) position to INT16 */
int16_t VAT_ConvertPositionRealToInt(float real_position, uint16_t position_units);

/******************************************************************************
 * UNITS CONVERSION FUNCTIONS
 ******************************************************************************/

/* Convert pressure between units (both REAL) */
float VAT_ConvertPressure(float value, uint16_t from_units, uint16_t to_units);

/* Convert position between units (both REAL) */
float VAT_ConvertPosition(float value, uint16_t from_units, uint16_t to_units);

#endif /* APP_VAT_CONVERSION_H_ */
