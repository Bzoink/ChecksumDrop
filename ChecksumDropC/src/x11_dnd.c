/*
 * x11_dnd.c — Linux/X11 XdndProxy setup
 * See x11_dnd.h for the rationale.
 */

#if defined(__linux__)

/* GLFW_EXPOSE_NATIVE_X11 must be defined before the native header. */
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

void xdnd_proxy_setup(void *glfw_win_handle)
{
    if (!glfw_win_handle) return;

    GLFWwindow *gwin    = (GLFWwindow *)glfw_win_handle;
    Display    *display = glfwGetX11Display();
    Window      app_win = glfwGetX11Window(gwin);

    if (!display || !app_win) return;

    /*
     * XSync flushes all pending requests to the X server and waits for the
     * server to process them.  This gives Mutter (or any reparenting WM) the
     * time it needs to reparent our window inside its frame before we query
     * the window tree below.
     */
    XSync(display, False);

    Window        root     = 0;
    Window        parent   = 0;
    Window       *children = NULL;
    unsigned int  nchildren = 0;

    XQueryTree(display, app_win, &root, &parent, &children, &nchildren);
    if (children) XFree(children);

    /*
     * If parent == root the WM has not (yet) reparented our window, or there
     * is no reparenting WM.  In either case XDND events are delivered
     * directly to app_win and no proxy is needed.
     */
    if (!parent || parent == root) return;

    /*
     * parent is the WM-managed frame window.  Advertise XDND support on it
     * and point back to our real app window via XdndProxy.
     *
     * XDND sources (GTK3/Nemo etc.) that find the frame will:
     *   1. See XdndProxy on the frame.
     *   2. Verify that the proxy window (app_win) has XdndAware.
     *   3. Send XdndEnter / XdndPosition / XdndDrop to app_win.
     *
     * GLFW's own ClientMessage handler in rcore_desktop.c then processes
     * those events exactly as it would without a compositing WM.
     */
    Atom xdnd_aware = XInternAtom(display, "XdndAware", False);
    Atom xdnd_proxy = XInternAtom(display, "XdndProxy", False);
    const long dnd_version = 5;

    /* XdndProxy on the frame → our app window */
    XChangeProperty(display, parent, xdnd_proxy, XA_WINDOW, 32,
                    PropModeReplace, (unsigned char *)&app_win, 1);

    /* XdndAware must also be present on the proxy window per XDND spec */
    XChangeProperty(display, parent, xdnd_aware, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)&dnd_version, 1);

    XFlush(display);
}

#endif /* __linux__ */
