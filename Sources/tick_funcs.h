// Get ticks as a uint64_t
#define TICKS_VAL(ticks) (*(uint64_t*)ticks)

#define TICKS_TO_DOUBLE(ticks) ( \
		((double)TICKS_VAL(ticks.TICKS) * _time_get_hwticks_per_tick()) + ticks.HW_TICKS)
		