/*
Simple keyboard layout indicator (very small window).
Thanks to everyone for their help and code examples.
Andrey Shashanov (2019)
gcc -O2 -s -lX11 -lXft `pkg-config --cflags freetype2` -o kblayout kblayout.c
*/

#include <ctype.h>
#include <X11/XKBlib.h>
#include <X11/Xft/Xft.h>

#define WIN_POSITION_X 600
#define WIN_POSITION_Y 70
#define WIN_WIDTH 32
#define WIN_HEIGHT 17
#define BG_COLOR "#191b1d" /* #e8e8e7 */
#define FG_COLOR "#dfdfdf" /* #2e3436 */
#define FONT "monospace:size=10"
#define NUM_CHARS 3
/*
#define CHARS_POS_X 4
#define CHARS_POS_Y 12
*/
int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    Display *dpy;
    int dummy;
    int scr;
    Drawable rwin;
    Visual *vsl;
    Colormap cmap;
    XftFont *xftfont;
    XSetWindowAttributes xwa;
    Window win;
    XftColor xftbgcolor, xftfgcolor;
    int chars_pos_x, chars_pos_y;
    ssize_t prevgroup = -1;

    if ((dpy = XOpenDisplay(NULL)) == NULL ||
        !XkbQueryExtension(dpy, &dummy, &dummy, &dummy, &dummy, &dummy))
        return EXIT_FAILURE;

    scr = DefaultScreen(dpy);
    rwin = RootWindow(dpy, scr);
    vsl = DefaultVisual(dpy, scr);
    cmap = DefaultColormap(dpy, scr);

    XftColorAllocName(dpy, vsl, cmap, BG_COLOR, &xftbgcolor);
    XftColorAllocName(dpy, vsl, cmap, FG_COLOR, &xftfgcolor);

    xftfont = XftFontOpenName(dpy, scr, FONT);

#if !defined(CHARS_POS_X)
    chars_pos_x = ((int)WIN_WIDTH - xftfont->max_advance_width * NUM_CHARS) / 2;
#else
    chars_pos_x = CHARS_POS_X;
#endif
#if !defined(CHARS_POS_Y)
    chars_pos_y = ((int)WIN_HEIGHT + xftfont->ascent) / 2 - 1;
#else
    chars_pos_y = CHARS_POS_Y;
#endif

    xwa.background_pixel = xftbgcolor.pixel;
    xwa.override_redirect = True;
    xwa.event_mask = ExposureMask | VisibilityChangeMask;
    win = XCreateWindow(dpy,
                        rwin,
                        WIN_POSITION_X, WIN_POSITION_Y,
                        WIN_WIDTH, WIN_HEIGHT,
                        0,
                        CopyFromParent,
                        InputOutput,
                        vsl,
                        CWBackPixel | CWOverrideRedirect | CWEventMask,
                        &xwa);
    XMapWindow(dpy, win);
    XFlush(dpy);

    XkbSelectEvents(dpy, XkbUseCoreKbd, XkbAllEventsMask, XkbAllEventsMask);

    for (;;)
    {
        XkbStateRec state;
        XEvent xev;

        XkbGetState(dpy, XkbUseCoreKbd, &state);
        while (XPending(dpy))
        {
            XNextEvent(dpy, &xev);
            if (state.group != prevgroup)
            {
                XkbDescPtr xkb;
                char *name;
                XftDraw *xftdraw;
                size_t i;

                xkb = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd);
                name = XGetAtomName(dpy, xkb->names->groups[state.group]);

                i = NUM_CHARS;
                do
                {
                    --i;
                    name[i] = (char)toupper(name[i]);
                } while (i);

                xftdraw = XftDrawCreate(dpy, win, vsl, cmap);
                XClearWindow(dpy, win);
                XftDrawString8(xftdraw, &xftfgcolor, xftfont,
                               chars_pos_x, chars_pos_y,
                               (const FcChar8 *)name, NUM_CHARS);
                XftDrawDestroy(xftdraw);
                XRaiseWindow(dpy, win);
                XFlush(dpy);

                XFree(name);
                XkbFreeKeyboard(xkb, XkbAllComponentsMask, True);
                prevgroup = state.group;
            }
        }
        XNextEvent(dpy, &xev);
    }

    /* code will never be executed */
    XkbSelectEvents(dpy, XkbUseCoreKbd, XkbAllEventsMask, 0);
    XDestroyWindow(dpy, win);
    XftColorFree(dpy, vsl, cmap, &xftfgcolor);
    XftColorFree(dpy, vsl, cmap, &xftbgcolor);
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
}
