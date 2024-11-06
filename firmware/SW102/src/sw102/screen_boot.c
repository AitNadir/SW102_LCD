#include "gfx.h"
#include "ui.h"
#include "lcd.h"
#include "state.h"
#include <stdio.h>

const
#include "logo.xbm"
const
#include "logo_anim.xbm"
const
#include "logo_syklo.xbm"
const
#include "font_2nd.xbm"
DEFINE_FONT(2sd, "./0123456789Wehikmnr", 3, 11, 18, 22, 29, 37, 44, 51, 58, 65, 72, 79, 90, 96, 102, 105, 111, 120, 126);

DEFINE_IMAGE(logo);
DEFINE_IMAGE(logo_anim);
DEFINE_IMAGE(logo_syklo);


extern const struct screen screen_main;

static void boot_idle()
{
  ui_vars_t *ui = get_ui_vars();
	switch (g_motor_init_state) {
		case MOTOR_INIT_SIMULATING:
			if(tick < 100)
				break;

		case MOTOR_INIT_WAIT_GOT_CONFIGURATIONS_OK:
		case MOTOR_INIT_READY:
		  if(ui8_screenmain_ready_counter ||
		      //ui->ui8_error_states & 1 ||
		      ui->ui8_error_states & 4 ||
		      ui->ui8_error_states & 8 ||
		      ui->ui8_error_states & 32)
		    break;
		  else{
		    showScreen(&screen_main);
		          return;

		  }

		// any error state will block here and avoid leave the boot screen
		default:
			break;
	}

	if(tick&3)
		return;

	static int q=1;
	char buf[10] = "";
	clear_all();
	//if(ui->ui8_error_states & 1)
	  //sprintf(buf, "err%02d", 1);
	if(ui->ui8_error_states & 4)
	  sprintf(buf, "err%02d", 3);
	if(ui->ui8_error_states & 8)
	  sprintf(buf, "err%02d", 4);
	if(ui->ui8_error_states & 32)
	  sprintf(buf, "err%02d", 6);
	font_text(&font_2sd, 32, 102, buf, AlignCenter);
	img_draw(&img_logo, 16, 17);
	img_draw_clip(&img_logo_anim, 8, 29, 0, q*18, 18, 18, 0);
	q=(q+3)&3;
	img_draw_clip(&img_logo_anim, 38, 29, 0, q*18, 18, 18, 0);
	img_draw(&img_logo_syklo, 10, 64);
	lcd_refresh();
}

void ui_show_motor_status()
{
}

const struct screen screen_boot = {
	.idle = boot_idle
};
