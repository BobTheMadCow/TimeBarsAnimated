#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x07, 0xCE, 0xF0, 0x49, 0x59, 0xB8, 0x47, 0x40, 0x8F, 0x96, 0xF4, 0x1F, 0x8C, 0x26, 0x0A, 0xD3 }
PBL_APP_INFO(MY_UUID,
             "TimeBars", "BobTheMadCow",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

/********************
	 vHOUR_LOC
	+--------+ 
	| 12  59 | NUM_LABEL_OFFSET
	|,--.,--.| BAR_MAX_LOC
	||  ||  || 
	||  ||  || 
	|+--++--+| BAR_MIN_LOC 
	|HOURMINS| TEXT_LABEL_MAX_LOC
	+--------+ TEXT_LABEL_MIN_LOC
	     ^MINUTE_LOC
*********************/
#define CORNER_MASK GCornersTop
#define CORNER_SIZE 12
#define BAR_MAX_LOC 22
#define BAR_MIN_LOC 150
#define MAX_HEIGHT (BAR_MIN_LOC - BAR_MAX_LOC)
#define HOUR_LOC 4
#define MINUTE_LOC 74
#define HOUR_WIDTH 66
#define MINUTE_WIDTH 66
#define HOUR_UNIT_HEIGHT (MAX_HEIGHT/12.0f)
#define MINUTE_UNIT_HEIGHT (MAX_HEIGHT/60.0f)
	
#define TEXT_LABEL_MAX_LOC 142
#define TEXT_LABEL_MIN_LOC 168
#define TEXT_LABEL_HEIGHT (TEXT_LABEL_MIN_LOC - TEXT_LABEL_MAX_LOC)
#define NUM_LABEL_OFFSET (BAR_MAX_LOC + 14)
#define NUM_LABEL_HEIGHT 46

#define HOUR_LABEL_TEXT "HOUR"
#define MINUTE_LABEL_TEXT "MINS"
	
#define BACKGROUND_COLOR GColorWhite
#define FOREGROUND_COLOR GColorBlack
	
Window window;
Layer hour_bar_layer;
Layer minute_bar_layer;

GFont text_font;
GFont num_font;

// Need to be static because it's used by the system later.
static char hour_text[] = "00";
static char minute_text[] = "00";

void update_hour_bar_callback(Layer *me, GContext* ctx)
{
	PblTm pblTime;
	get_time(&pblTime);
	int hour = pblTime.tm_hour; 
	float adjusted_hour_unit_height = HOUR_UNIT_HEIGHT;
	int x, y, w, h;
	char *time_format;
	
	if(clock_is_24h_style())
	{
		adjusted_hour_unit_height = HOUR_UNIT_HEIGHT / 2.0f;
		//	%H 	Two digit representation of the hour in 24-hour format 	00 through 23
		//	%k 	Two digit representation of the hour in 24-hour format, with a space preceding single digits 	0 through 23
		time_format = "%H";
	}
	else
	{
		if(hour > 12)
		{
			hour -= 12;
		}		
		else if(hour == 0)
		{
			hour = 12;	//for correct hieght bar in 12 hour mode
		}
		//	%I 	Two digit representation of the hour in 12-hour format 	01 through 12
		//	%l (lower-case 'L') 	Hour in 12-hour format, with a space preceding single digits 	1 through 12
		time_format = "%l";
	}
	string_format_time(hour_text, sizeof(hour_text), time_format, &pblTime);

	graphics_context_set_stroke_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_text_color(ctx, FOREGROUND_COLOR);

	x = HOUR_LOC;
	y = BAR_MIN_LOC - (int)(hour * adjusted_hour_unit_height);
	w = HOUR_WIDTH;
	h = (int)(hour * adjusted_hour_unit_height);

	graphics_fill_rect(ctx, GRect( x, y, w, h ), CORNER_SIZE, CORNER_MASK);
  	graphics_text_draw(ctx, hour_text, num_font, GRect(0, (y - NUM_LABEL_OFFSET), 72, NUM_LABEL_HEIGHT), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
	graphics_text_draw(ctx, HOUR_LABEL_TEXT, text_font, GRect(0, TEXT_LABEL_MAX_LOC, 72, TEXT_LABEL_HEIGHT), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

void update_minute_bar_callback(Layer *me, GContext* ctx)
{
	PblTm pblTime;
	get_time(&pblTime);
	int minute  = pblTime.tm_min;
	int16_t x, y, w, h;

	string_format_time(minute_text, sizeof(minute_text), "%M", &pblTime);

	graphics_context_set_stroke_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);
	graphics_context_set_text_color(ctx, FOREGROUND_COLOR);

	x = MINUTE_LOC;
	y = BAR_MIN_LOC - (int)(minute * MINUTE_UNIT_HEIGHT);
	w = MINUTE_WIDTH;
	h = (int)(minute * MINUTE_UNIT_HEIGHT);

	graphics_fill_rect(ctx, GRect( x, y, w, h ), CORNER_SIZE, CORNER_MASK);
	graphics_text_draw(ctx, minute_text, num_font, GRect(72, (y - NUM_LABEL_OFFSET), 72, NUM_LABEL_HEIGHT), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
	graphics_text_draw(ctx, MINUTE_LABEL_TEXT, text_font, GRect(72, TEXT_LABEL_MAX_LOC, 72, TEXT_LABEL_HEIGHT), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t)
{
	PblTm pblTime;
	get_time(&pblTime);
	
	if( pblTime.tm_min == 0 )
	{
		layer_mark_dirty(&hour_bar_layer);
	}
	layer_mark_dirty(&minute_bar_layer);
}

void handle_init(AppContextRef ctx) 
{
	resource_init_current_app(&APP_RESOURCES);
	
	window_init(&window, "TimeBars");
	window_stack_push(&window, true /* Animated */);
	window_set_background_color(&window, BACKGROUND_COLOR);
	
	text_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TYPEONE_24));
	num_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TYPEONE_34));
		
	layer_init(&hour_bar_layer, window.layer.frame);
	hour_bar_layer.update_proc = update_hour_bar_callback;
	layer_add_child(&window.layer, &hour_bar_layer);

	layer_init(&minute_bar_layer, window.layer.frame);
	minute_bar_layer.update_proc = update_minute_bar_callback;
	layer_add_child(&window.layer, &minute_bar_layer);
}


void handle_deinit(AppContextRef ctx) {
	fonts_unload_custom_font(text_font);
	fonts_unload_custom_font(num_font);
}

void pbl_main(void *params) 
{
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
	.deinit_handler = &handle_deinit,
    .tick_info = {
		.tick_handler = &handle_minute_tick,
		.tick_units = MINUTE_UNIT
    },
  };
  app_event_loop(params, &handlers);
}