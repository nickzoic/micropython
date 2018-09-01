
#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/objexcept.h"

#include "generated/csr.h"

#ifndef CSR_CAS_BASE
static inline unsigned char cas_leds_out_read(void) {
	return 0;
}

static inline void cas_leds_out_write(unsigned char value) {
}
#endif

const mp_obj_type_t litex_led_type;

typedef struct _litex_led_obj_t {
    mp_obj_base_t base;
    unsigned int addr;
} litex_led_obj_t;

STATIC litex_led_obj_t litex_leds[CAS_LEDS_COUNT] = {
#ifdef CSR_CAS_LED0_DUTY_ADDR
       { { &litex_led_type }, CSR_CAS_LED0_DUTY_ADDR },
#endif
#ifdef CSR_CAS_LED1_DUTY_ADDR
       { { &litex_led_type }, CSR_CAS_LED1_DUTY_ADDR },
#endif
#ifdef CSR_CAS_LED2_DUTY_ADDR
       { { &litex_led_type }, CSR_CAS_LED2_DUTY_ADDR },
#endif
#ifdef CSR_CAS_LED3_DUTY_ADDR
       { { &litex_led_type }, CSR_CAS_LED3_DUTY_ADDR },
#endif
       };

STATIC mp_obj_t litex_led_make_new(const mp_obj_type_t *type_in,
		size_t n_args, size_t n_kw, const mp_obj_t *args) {
	mp_arg_check_num(n_args, n_kw, 1, 1, false);

	mp_uint_t led_num = mp_obj_get_int(args[0]);

	switch (led_num) {
	case 1 ... CAS_LEDS_COUNT:
		return &litex_leds[led_num - 1];
	default:
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
			"not a valid LED number: %d", led_num));
	}
}

void litex_led_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
	litex_led_obj_t *self = self_in;
	mp_printf(print, "LED(%p)", self->addr);
}

STATIC mp_obj_t litex_led_read(mp_obj_t self_in) {
	litex_led_obj_t *led = self_in;
	bool state = MMPTR(led->addr);

	return mp_obj_new_bool(state);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(litex_led_read_obj, litex_led_read);

STATIC mp_obj_t litex_led_on(mp_obj_t self_in) {
	litex_led_obj_t *led = self_in;
	MMPTR(led->addr) = 255;
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(litex_led_on_obj, litex_led_on);

STATIC mp_obj_t litex_led_off(mp_obj_t self_in) {
	litex_led_obj_t *led = self_in;
	MMPTR(led->addr) = 0;
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(litex_led_off_obj, litex_led_off);

STATIC mp_obj_t litex_led_duty(mp_obj_t self_in, mp_obj_t duty_in) {
	litex_led_obj_t *led = self_in;
	MMPTR(led->addr) = mp_obj_get_int(duty_in);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(litex_led_duty_obj, litex_led_duty);


STATIC const mp_map_elem_t litex_led_locals_dict_table[] = {
	{ MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&litex_led_read_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_on), (mp_obj_t)&litex_led_on_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_off), (mp_obj_t)&litex_led_off_obj },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_duty), (mp_obj_t)&litex_led_duty_obj },
};
STATIC MP_DEFINE_CONST_DICT(litex_led_locals_dict, litex_led_locals_dict_table);

const mp_obj_type_t litex_led_type = {
	{ &mp_type_type },
	.name = MP_QSTR_LED,
	.print = litex_led_print,
	.make_new = litex_led_make_new,
	.locals_dict = (mp_obj_t)&litex_led_locals_dict,
};
