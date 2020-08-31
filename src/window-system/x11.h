/**
 * Copyright 2011 - 2020 José Expósito <jose.exposito89@gmail.com>
 *
 * This file is part of Touchégg.
 *
 * Touchégg is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License  as  published by  the  Free Software
 * Foundation,  either version 2 of the License,  or (at your option)  any later
 * version.
 *
 * Touchégg is distributed in the hope that it will be useful,  but  WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the  GNU General Public License  for more details.
 *
 * You should have received a copy of the  GNU General Public License along with
 * Touchégg. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef WINDOW_SYSTEM_X11_H_
#define WINDOW_SYSTEM_X11_H_

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "window-system/window-system.h"

/**
 * X11 WindowT implementation, just a Window as defined in <X11/X.h>.
 */
class X11WindowT : public WindowT {
 public:
  explicit X11WindowT(Window window) : window(window) {}
  Window window;
};

/**
 * X11 utilities.
 */
class X11 : public WindowSystem {
 public:
  X11();
  ~X11();

  std::unique_ptr<WindowT> getWindowUnderCursor() const override;

  std::string getWindowClassName(const WindowT &window) const override;
  Rectangle getWindowSize(const WindowT &window) const override;
  bool isWindowMaximized(const WindowT &window) const override;
  bool isSystemWindow(const WindowT &window) const override;
  void maximizeOrRestoreWindow(const WindowT &window) const override;
  void minimizeWindow(const WindowT &window) const override;
  Rectangle minimizeWindowIconSize(const WindowT &window) const override;

  Rectangle getDesktopWorkarea() const override;

  cairo_surface_t *createSurface() const override;
  int getSurfaceWidth(cairo_surface_t *cairoSurface) const override;
  int getSurfaceHeight(cairo_surface_t *cairoSurface) const override;
  void flushSurface(cairo_surface_t *cairoSurface) const override;
  void destroySurface(cairo_surface_t *cairoSurface) const override;

 private:
  /**
   * X11 connection.
   */
  Display *display;

  /**
   * A developer friendly wrapper around XGetWindowProperty.
   * https://tronche.com/gui/x/xlib/window-information/XGetWindowProperty.html
   * @param window The window to get the property from.
   * @param atomName Name of the Atom to get.
   * @param propType XA_WINDOW, XA_CARDINAL...
   * @returns A vector with the returned properties.
   */
  template <typename T>
  std::vector<T> getWindowProperty(Window window, const std::string &atomName,
                                   Atom atomType) const;

  /**
   * The top-level window contains useful attributes like WM_CLASS or WM_NAME
   * and it is also used for inter-client communication. For example, we can set
   * the _NET_WM_STATE atom in the top-level window to ask the window manager to
   * maximize it.
   *
   * ICCCM defines a top-level window like:
   * https://www.x.org/releases/X11R7.6/doc/xorg-docs/specs/ICCCM/icccm.html#creating_a_top_level_window
   * "A client's top-level window is a window whose override-redirect attribute
   * is False. It must either be a child of a root window, or it must have been
   * a child of a root window immediately prior to having been reparented by the
   * window manager."
   *
   * This means that we cannot presuppose that the top-level window is a direct
   * child of the root window. For example, run this command to get all the
   * children of the root window:
   *
   * $ xwininfo -tree -root
   *
   * You may notice that some direct children of the root window (for example
   * Visual Studio Code) don't have name or class:
   *
   * 0x18000a2 (has no name): () <--- Not the top-level window!!!
   *    1 child:
   *      0x3200001 "Visual Studio Code Title": ("code" "Code")
   *        1 child:
   *        [...]
   *
   * Luckily for us, EWMH defines _NET_CLIENT_LIST. This attribute of the root
   * window contains an array of top-level windows:
   * https://specifications.freedesktop.org/wm-spec/wm-spec-1.3.html#idm45805408060416
   * You can test it with:
   *
   * $ xprop -root
   *
   * TL;DR Get the _NET_CLIENT_LIST attribute and check all the children of the
   * window param to get the top-level window.
   */
  Window getTopLevelWindow(Window window) const;
  std::pair<bool, Window> findTopLevelWindowInChildren(
      Window window, const std::vector<Window> &topLevelWindows) const;
};

#endif  // WINDOW_SYSTEM_X11_H_
