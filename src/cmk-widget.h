/*
 * This file is part of graphene-desktop, the desktop environment of VeltOS.
 * Copyright (C) 2016 Velt Technologies, Aidan Shafran <zelbrium@gmail.com>
 * Licensed under the Apache License 2 <www.apache.org/licenses/LICENSE-2.0>.
 */

#ifndef __CMK_WIDGET_H__
#define __CMK_WIDGET_H__

#include <clutter/clutter.h>

G_BEGIN_DECLS

#define CMK_TYPE_WIDGET cmk_widget_get_type()
G_DECLARE_DERIVABLE_TYPE(CmkWidget, cmk_widget, CMK, WIDGET, ClutterActor);

typedef struct _CmkWidgetClass CmkWidgetClass;

struct _CmkWidgetClass
{
	ClutterActorClass parentClass;
	
	/*
	 * Emitted when a style property changes on the widget. This may include
	 * changes to the style of parent widgets. Always emitted during object
	 * construction (after init completes). Must chain up to parent class.
	 */
	void (*style_changed) (CmkWidget *self);

	/*
	 * Emitted when the value returned by cmk_widget_get_background_color
	 * changes. Must chain up to parent class.
	 */
	void (*background_changed) (CmkWidget *self);

	/*
	 * Emitted when a CmkWidget in the actor heirarchy below this widget
	 * gains key focus. Chain up to parent class if event goes unhandled.
	 */
	void (*key_focus_changed) (CmkWidget *self, ClutterActor *newfocus);
	
	/*
	 * Emitted by widget subclasses when the parent widget should replace
	 * them with a new widget. It is up to the parent widget to honor this.
	 * (This can also be used as a general 'forward' signal, with replacement
	 * being NULL and the parent having a planned list of screens.)
	 */
	void (*replace) (CmkWidget *self, CmkWidget *replacement);
	
	/*
	 * Emitted by widget subclasses when the parent should remove this widget
	 * and return to the previously shown widget (opposite of 'replace').
	 * It is up to the parent widget to honor this.
	 */
	void (*back) (CmkWidget *self);
};

typedef struct
{
	const gchar *name;
	ClutterColor color;
} CmkNamedColor;

/*
 * CmkWidget is not abstract. Creating a widget on its own is effectively
 * just a ClutterActor with styling properties.
 */
CmkWidget * cmk_widget_new();

/*
 * Exactly the same as clutter_actor_destroy, except returns G_SOURCE_REMOVE
 * so this can easily be called from timeouts. Safe to pass in a regular
 * ClutterActor* casted to a CmkWidget*.
 */
gboolean cmk_widget_destroy(CmkWidget *widget);

/*
 * This is a special instance of CmkWidget which is used for styling when no
 * inherited values can be found. You may set styles on it, but it should not
 * be parented/mapped. The return value should be unrefed after use, unless
 * its being used to set global styles, in which case you can leave it around
 * for the duration of your program.
 */
CmkWidget * cmk_widget_get_style_default();

/*
 * Instead of using the widget's actual parent, use a different widget to
 * inherit styles from. This should probably only be used if there is a
 * non-CmkWidget actor between two CmkWidgets, to connect the 3rd layer to
 * the 1st. Use NULL to unset.
 *
 * CmkWidget does not retain a reference to its style parent. If the parent
 * is destroyed, the "style parent" association will automatically be
 * removed and the widget will return to using its real parent or no parent.
 */
void cmk_widget_set_style_parent(CmkWidget *widget, CmkWidget *parent);

/*
 * Gets the current style parent. If cmk_widget_set_style_parent has been
 * called, this returns that value. Otherwise, this returns the widget's
 * current actual parent if it is a CmkWidget, or the default styling
 * CmkWidget otherwise. (If called on the default style CmkWidget, returns
 * NULL).
 */
CmkWidget * cmk_widget_get_style_parent(CmkWidget *widget);

/*
 * Gets a named color set with cmk_widget_set_style_color. If it is not
 * found, an inherited value is returned. If there is no inherited value,
 * NULL is returned.
 */
const ClutterColor * cmk_widget_style_get_color(CmkWidget *widget, const gchar *name);

/*
 * Sets a named color that can be used to draw the widget. This color will be
 * inherited by child CmkWidgets.
 */
void cmk_widget_style_set_color(CmkWidget *widget, const gchar *name, const ClutterColor *color);

/*
 * Takes an array of CmkNamedColor (last one must be a NULLed
 * struct) and calls cmk_widget_style_set_color on each.
 */
void cmk_widget_style_set_colors(CmkWidget *widget, const CmkNamedColor *colors);

/*
 * Getters and setters for other style properties. If these are set to invalid
 * values (ex. -1), an inherited value will be used when calling the getter.
 */
void cmk_widget_style_set_bevel_radius(CmkWidget *widget, float radius);
float cmk_widget_style_get_bevel_radius(CmkWidget *widget);
void cmk_widget_style_set_padding(CmkWidget *widget, float padding);
float cmk_widget_style_get_padding(CmkWidget *widget);
void cmk_widget_style_set_scale_factor(CmkWidget *widget, float scale);
float cmk_widget_style_get_scale_factor(CmkWidget *widget);

/*
 * Attempts to find a foreground color from the current background color.
 * This first tries to use the named color "<background name>-foreground",
 * then tries "foreground", and if both fail, returns solid black.
 */
const ClutterColor * cmk_widget_get_foreground_color(CmkWidget *widget);

/*
 * Similar to clutter_actor_set_background_color, except it uses the named
 * color from the current style. Set to NULL to have a fully transparent
 * background (the default).
 * See cmk_widget_set_draw_background_color.
 */
void cmk_widget_set_background_color_name(CmkWidget *widget, const gchar *namedColor);

/*
 * If TRUE, cmk_widget_set_background_color will actually call
 * clutter_actor_set_background_color to fill in the actor. Otherwise,
 * the specified background color is just used for child actors calling
 * cmk_widget_get_background_color. (If not TRUE, you should probably
 * be drawing a background of that color yourself.)
 * Defaults to FALSE.
 */
void cmk_widget_set_draw_background_color(CmkWidget *widget, gboolean draw);

/*
 * Gets the effective background color of the widget. If no background
 * color for this widget has been set, the parent widget's get_background_color
 * method will be called. Foreground actors should base their color on this
 * background color. If this method returns NULL, the background color is
 * unknown, and foreground actors should use a default color.
 */
const gchar * cmk_widget_get_background_color_name(CmkWidget *widget);

/*
 * Convenience method for calling cmk_widget_get_background_color_name
 * and cmk_widget_style_get_color, except returns solid white if no color
 * is found.
 */
const ClutterColor * cmk_widget_get_background_color(CmkWidget *widget);

/*
 * Fades out the actor and then hides it.
 * Set destroy to TRUE to destroy the actor after the fade completes.
 */
void cmk_widget_fade_out(CmkWidget *widget, gboolean destroy);

/*
 * Shows the actor and fades it in.
 */
void cmk_widget_fade_in(CmkWidget *widget);

/*
 * Convenience for ClutterColor -> RGBA -> cairo_set_source_rgba.
 */
void cairo_set_source_clutter_color(cairo_t *cr, const ClutterColor *color);

/*
 * Simple 1-topalpha blend.
 */
void clutter_color_blend(const ClutterColor *top, const ClutterColor *bottom, ClutterColor *out);

/*
 * Convenience for scaling a ClutterActorBox. Set move to TRUE to scale
 * its top left x,y coordinates as well as its size.
 */
void cmk_scale_actor_box(ClutterActorBox *b, gfloat scale, gboolean move);

void cmk_widget_push_tab_modal(CmkWidget *widget);
void cmk_widget_pop_tab_modal();

void cmk_widget_set_tabbable(CmkWidget *widget, gboolean tabbable);
gboolean cmk_widget_get_tabbable(CmkWidget *widget);

guint cmk_redirect_keyboard_focus(ClutterActor *actor, ClutterActor *redirection);
guint cmk_focus_on_mapped(ClutterActor *actor);
void cmk_focus_stack_push(CmkWidget *widget);
void cmk_focus_stack_pop(CmkWidget *widget);

/*
 * Convenience for
 * g_signal_emit_by_name(widget, "replace", replacement);
 */
void cmk_widget_replace(CmkWidget *widget, CmkWidget *replacement);

/*
 * Convenience for
 * g_signal_emit_by_name(widget, "back");
 */
void cmk_widget_back(CmkWidget *widget);

/*
 * Same as clutter_actor_add_child but casts for you.
 */
void cmk_widget_add_child(CmkWidget *widget, CmkWidget *child);

/*
 * Binds the CmkWidget to its parent so that it always
 * fills its allocation.
 * The bind can be removed by removing the clutter constraint
 * "cmk-widget-bind-fill"
 */
void cmk_widget_bind_fill(CmkWidget *widget);

G_END_DECLS

#endif